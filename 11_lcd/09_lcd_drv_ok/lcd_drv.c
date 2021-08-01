#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fb.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <video/display_timing.h>
#include <linux/platform_device.h>
#include <video/of_display_timing.h>
#include <linux/of_gpio.h>
#include <linux/clk.h>
#include "lcd_drv.h"

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

static void lcd_controller_enable(struct imx6ull_lcdif *lcdif)
{
	lcdif->CTRL |= (1<<0);
}

static int lcd_controller_init(struct imx6ull_lcdif *lcdif, 
		struct display_timing *dt, int lcd_bpp, int fb_bpp, 
		unsigned int fb_phy)
{
	int lcd_data_bus_width;
	int fb_width;
	int vsync_pol = 0;
	int hsync_pol = 0;
	int dotclk_pol = 0;
	int de_pol = 0;

	switch(lcd_bpp) {
	case 16:
		lcd_data_bus_width = 0x0;
		break;
	case 8:
		lcd_data_bus_width = 0x1;
		break;
	case 18:
		lcd_data_bus_width = 0x2;
		break;
	case 24:
		lcd_data_bus_width = 0x3;
		break;
	default:
		lcd_data_bus_width = 0x0;
		break;
	}

	switch(fb_bpp) {
	case 16:
		fb_width = 0x0;
		break;
	case 8:
		fb_width = 0x1;
		break;
	case 18:
		fb_width = 0x2;
		break;
	case 24:
	case 32:
		fb_width = 0x3;
		break;
	default:
		fb_width = 0x0;
		break;
	}

	/*
	lcdif->CTRL = (0<<31) | (0<<30) | (0<<29) | (0<<28) | (1<<27) |
		(0<<20) | (1<<19) | (1<<18) | (1<<17) | (0x0<<14) | (0x0<<12) |
		(lcd_data_bus_width<<10) | (fb_width<<8) | (1<<5);
	*/

	lcdif->CTRL = (0<<30) | (0<<29) | (0<<28) | (1<<19) | (1<<17) | (lcd_data_bus_width << 10) |\
			  (fb_width << 8) | (1<<5);

	if(fb_bpp == 24 || fb_bpp == 32) {
		lcdif->CTRL1 &= ~(0xf << 16);
		lcdif->CTRL1 |= (0x7 << 16);
	} else {
		lcdif->CTRL1 |= (0xf << 16);
	}


	lcdif->TRANSFER_COUNT = (dt->vactive.typ << 16 )| (dt->hactive.typ<<0);


	if(dt->flags & DISPLAY_FLAGS_HSYNC_HIGH)
		hsync_pol = 1;
	if(dt->flags & DISPLAY_FLAGS_VSYNC_HIGH)
		vsync_pol = 1;
	if(dt->flags & DISPLAY_FLAGS_DE_HIGH)
		de_pol = 1;
	if(dt->flags & DISPLAY_FLAGS_PIXDATA_POSEDGE)
		dotclk_pol = 1;


	lcdif->VDCTRL0 = (1 << 28) | (vsync_pol << 27) | (hsync_pol << 26) | (dotclk_pol << 25) | (de_pol << 24) | (1<<21) |
		(1<<20) | (dt->vsync_len.typ << 0);

	lcdif->VDCTRL1 = dt->vback_porch.typ + dt->vsync_len.typ + dt->vactive.typ + dt->vfront_porch.typ;

	lcdif->VDCTRL2 = (dt->hsync_len.typ << 18)| (dt->hback_porch.typ + dt->hsync_len.typ + dt->hactive.typ + dt->hfront_porch.typ) << 0;

	lcdif->VDCTRL3 = (dt->hback_porch.typ + dt->hsync_len.typ) << 16 |
		((dt->vback_porch.typ + dt->vsync_len.typ) << 0);

	lcdif->VDCTRL4 = (1 << 18) | (dt->hactive.typ);

	lcdif->CUR_BUF = fb_phy;
	lcdif->NEXT_BUF = fb_phy;

	return 0;
}

static int mylcd_probe(struct platform_device *pdev)
{
	struct device_node *display_np;
	int ret;
	dma_addr_t phy_addr;	
	int width, bits_per_pixel; 
	struct display_timings *timings = NULL;
	struct display_timing *dt = NULL;
	struct imx6ull_lcdif *lcdif;
	struct resource *res;

	display_np = of_parse_phandle(pdev->dev.of_node, "display", 0);
	if(!display_np) {
		dev_err(&pdev->dev, "missing property display \n");
		return -EINVAL;
	}

	/* get common info */
	ret = of_property_read_u32(display_np, "bus-width", &width);
	if(ret) {
		dev_err(&pdev->dev, "Not found bus-width\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(display_np, "bits-per-pixel", &bits_per_pixel);
	if(ret) {
		dev_err(&pdev->dev, "Not found bits-per-pixel\n");
		return -EINVAL;
	}

	timings = of_get_display_timings(display_np);
	if (!timings) {
		dev_err(&pdev->dev, "failed to get display timings\n");
		return -EINVAL;
	}
	dt = timings->timings[timings->native_mode];

	/* get gpio from devicetree */
	bl_gpio = gpiod_get(&pdev->dev, "backlight", 0);
	if(IS_ERR(bl_gpio)) {
		dev_err(&pdev->dev, "failed to get gpio backlight \n");
		return -1;
	}
	
	/* config bl_gpio output */
	gpiod_direction_output(bl_gpio, 1);


	/* set  val: gpiod_set_value(bl_gpio, status); */


	/* get clock from devicetree */
	clk_pix = devm_clk_get(&pdev->dev, "pix");
	if(IS_ERR(clk_pix)) {
		dev_err(&pdev->dev, "failed to get pix \n");
		return -1;
	}

	clk_axi = devm_clk_get(&pdev->dev, "axi");
	if(IS_ERR(clk_axi)) {
		dev_err(&pdev->dev, "failed to get axi \n");
		return -1;
	}

	/* set clock rate */
	clk_set_rate(clk_pix, dt->pixelclock.typ); 

	clk_prepare_enable(clk_pix);
	clk_prepare_enable(clk_axi);

	/* 1. crate */
	myfb_info = framebuffer_alloc(0, NULL);
	if(!myfb_info)
		return -ENOMEM;

	/* 2. set */
	/* 2.1 var resolution  color formate */
	myfb_info->var.xres = dt->hactive.typ;
	myfb_info->var.yres = dt->vactive.typ;
	myfb_info->var.bits_per_pixel = 16;		/* rgb565 */

	myfb_info->var.red.offset   = 11;
	myfb_info->var.red.length   = 5;
	myfb_info->var.green.offset = 5;
	myfb_info->var.green.length = 6;
	myfb_info->var.blue.offset  = 0;
	myfb_info->var.blue.length  = 5;
	myfb_info->var.xres_virtual = dt->hactive.typ;
	myfb_info->var.yres_virtual = dt->vactive.typ;

	/* 2.2 fix */
	strcpy(myfb_info->fix.id, "mylcd");
	myfb_info->fix.smem_len = myfb_info->var.xres * myfb_info->var.yres * myfb_info->var.bits_per_pixel / 8;
	if(myfb_info->var.bits_per_pixel == 24) {
		myfb_info->fix.smem_len = myfb_info->var.xres * myfb_info->var.yres * 4;
	}

	/* 2.3 fb virtual address */
	myfb_info->screen_base =  dma_alloc_wc(NULL, myfb_info->fix.smem_len,	&phy_addr, GFP_KERNEL);	 	/* virtual address */
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
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	lcdif = devm_ioremap_resource(&pdev->dev, res);

	lcd_controller_init(lcdif, dt, bits_per_pixel, 16,  phy_addr);

	lcd_controller_enable(lcdif);

	gpiod_set_value(bl_gpio, 1);

	return 0;
}

static int mylcd_remove(struct platform_device *pdev)
{
	iounmap(mylcd_regs);

	unregister_framebuffer(myfb_info);

	framebuffer_release(myfb_info);

	return 0;
}

static const struct of_device_id mylcd_of_match[] = {
	{ .compatible = "chen,mylcd", },
	{ },
};

static struct platform_driver mylcd_driver = {
	.probe = mylcd_probe,
	.remove = mylcd_remove,
	.driver = {
		.name = "drv,mylcd",
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
