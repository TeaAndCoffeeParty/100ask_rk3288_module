#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fb.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>

static unsigned int pseudo_palette[16];

struct lcd_regs {
	volatile unsigned int fb_base_phys;
	volatile unsigned int fb_xres;
	volatile unsigned int fb_yres;
	volatile unsigned int fb_bpp;
};
static struct lcd_regs *mylcd_regs;

static struct fb_info *myfb_info;

static struct gpio_desc *bl_gpio;


static struct clk* clk_pix;
static struct clk* clk_axi;

static inline unsigned int chan_to_field(unsigned int chan, struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}

static int s3c_lcdfb_setcolreg(unsigned int regno, unsigned int red,
			     unsigned int green, unsigned int blue,
			     unsigned int transp, struct fb_info *info)
{
	unsigned int val;

	/* dprintk("setcol: regno=%d, rgb=%d,%d,%d\n",
		   regno, red, green, blue); */
	switch (info->fix.visual) {
	case FB_VISUAL_TRUECOLOR:
		/* true-colour, use pseudo-palette */

		if (regno < 16) {
			u32 *pal = info->pseudo_palette;

			val  = chan_to_field(red,   &info->var.red);
			val |= chan_to_field(green, &info->var.green);
			val |= chan_to_field(blue,  &info->var.blue);

			pal[regno] = val;
		}
		break;

	default:
		return 1;	/* unknown type */
	}

	return 0;
}

static struct fb_ops myfb_ops = {
	.owner		= THIS_MODULE,
	.fb_setcolreg   = s3c_lcdfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

static int mylcd_probe(struct platform_device *pdev)
{
	int ret;
	dma_addr_t phy_addr;	


	/* get gpio from devicetree */
	bl_gpio = gpiod_get(&pdev->dev, "backlight", 0);
	
	/* config bl_gpio output */
	gpiod_direction_output(bl_gpio, 1);


	/* set  val: gpiod_set_value(bl_gpio, status); */


	/* get clock from devicetree */
	clk_pix = devm_clk_get(pdev->dev, "pix");

	clk_axi = devm_clk_get(pdev->dev, "axi");

	/* set clock rate */
	clk_set_rate(clk_pix, 50000000); //PICOS2KHZ(fb_info->var.pixclock) * 1000U);

	clk_prepare_enable(clk_pix);
	clk_prepare_enable(clk_axi);

	/* 1. crate */
	myfb_info = framebuffer_alloc(0, NULL);
	if(!myfb_info)
		return -ENOMEM;

	/* 2. set */
	/* 2.1 var resolution  color formate */
	myfb_info->var.xres = 500;
	myfb_info->var.yres = 300;
	myfb_info->var.bits_per_pixel = 16;		/* rgb565 */

	myfb_info->var.red.offset   = 11;
	myfb_info->var.red.length   = 5;
	myfb_info->var.green.offset = 5;
	myfb_info->var.green.length = 6;
	myfb_info->var.blue.offset  = 0;
	myfb_info->var.blue.length  = 5;
	myfb_info->var.xres_virtual = 500;
	myfb_info->var.yres_virtual = 300;

	/* 2.2 fix */
	strcmp(myfb_info->fix.id, "mylcd");
	myfb_info->fix.smem_len = myfb_info->var.xres * myfb_info->var.yres * myfb_info->var.bits_per_pixel / 8;
	if(myfb_info->var.bits_per_pixel == 24) {
		myfb_info->fix.smem_len = myfb_info->var.xres * myfb_info->var.yres * 4;
	}

	/* 2.3 fb virtual address */
	myfb_info->screen_base =  dma_alloc_writecombine(NULL, myfb_info->fix.smem_len,	
			&phy_addr, GFP_KERNEL);	 	/* virtual address */
	myfb_info->fix.smem_start = phy_addr;		/* physicsal address */
	myfb_info->fix.type = FB_TYPE_PACKED_PIXELS;
	myfb_info->fix.visual = FB_VISUAL_TRUECOLOR;

	myfb_info->fix.line_length = (myfb_info->var.xres_virtual * myfb_info->var.bits_per_pixel / 8);
	if(myfb_info->var.bits_per_pixel == 24) {
		myfb_info->fix.line_length = myfb_info->var.xres_virtual * 4;
	}

	/* 2.3 fops */
	myfb_info->fbops = &myfb_ops;
	myfb_info->pseudo_palette = pseudo_palette;

	/* 3. register */
	ret = register_framebuffer(myfb_info);

	/* 4. hardware opeation */
	mylcd_regs = ioremap(0x21c8000, sizeof(struct lcd_regs));
	mylcd_regs->fb_base_phys = phy_addr;
	mylcd_regs->fb_xres = 500;
	mylcd_regs->fb_yres = 300;
	mylcd_regs->fb_bpp = 16;

	return 0;
}

static int mylcd_remove(struct platform_device *pdev)
{
	iounmap(mylcd_regs);

	unregister_framebuffer(myfb_info);

	dma_free_writecombine(NULL, PAGE_ALIGN(myfb_info->fix.smem_len),
			myfb_info->screen_base, myfb_info->fix.smem_start);

	framebuffer_release(myfb_info);

	return 0;
}

static const struct of_device_id mylcd_of_match[] = {
	{ .compatible = "mylcd", },
	{ },
};

static struct platform_driver mylcd_driver = {
	.probe = mylcd_probe,
	.remove = mylcd_remove,
	.driver = {
		.name = "mylcd",
		.of_match_table = mylcd_of_match,
	},
};

static int lcd_drv_init(void)
{
	int ret = platform_driver_register(&mylcd_driver);
	if(ret)
		return ret;

	return ret;
}

static void lcd_drv_exit(void)
{
	platform_driver_unregister(&mylcd_driver);
}

module_init(lcd_drv_init);
module_exit(lcd_drv_exit);

MODULE_AUTHOR("chen");
MODULE_DESCRIPTION("Framebuffer driver for the linux");
MODULE_LICENSE("GPL");
