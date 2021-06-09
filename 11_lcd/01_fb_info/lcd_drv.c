#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fb.h>

struct fb_info *myfb_info;

static struct fb_ops myfb_ops = {
	.owner		= THIS_MODULE,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

static int lcd_drv_init(void) 
{
	int ret;
	dma_ddr_t phy_addr;	
	/* 1. crate */
	myfb_info = framebuffer_alloc(0, NULL);
	if(!fb_info)
		return -ENOMEN;

	/* 2. set */
	/* 2.1 var resolution  color formate */
	myfb_info->var.xres = 1024;
	myfb_info->var.yres = 600;
	myfb_info->var.bits_per_pixel = 16;		/* rgb565 */

	myfb_info->var.red.offset   = 11;
	myfb_info->var.red.length   = 5;
	myfb_info->var.green.offset = 5;
	myfb_info->var.green.length = 6;
	myfb_info->var.blue.offset  = 0;
	myfb_info->var.blue.length  = 5;

	/* 2.2 fix */
	myfb_info->fix.smem_len= myfb_info->var.xres * myfb_info->var.yres * myfb_info->var.bits_per_pixel / 8;
	if(myfb_info->var.bits_per_pixel == 24) {
		myfb_info->fix.smem_len= myfb_info->var.xres * myfb_info->var.yres * 4;
	}
	myfb_info->fix.type = FB_TYPE_PACKED_PIXELS;
	myfb_info->fix.visual = FB_VISUAL_TRUECOLOR;

	myfb_info->screen_base =  dma_alloc_writecombine(NULL, myfb_info->fix.smem_len,	
			&phy_addr, GFP_KERNEL);	 	/* virtual address */
	myfb_info->fix.smem_start = phy_addr;		/* physicsal address */

	/* 2.3 fops */
	myfb_info->fbops = &myfb_ops;

	/* 3. register */
	ret = register_framebuffer(myfb_info);

	/* 4. hardware opeation */

	return 0;
}

static void lcd_drv_exit(void)
{
	unregister_framebuffer(myfb_info);

	dma_free_writecombine(NULL, PAGE_ALIGN(myfb_info->fix.smem_len),
			myfb_info->screen_base, myfb_info->fix.smem_start);

	framebuffer_release(myfb_info);
}

module_init(lcd_drv_init);
module_exit(lcd_drv_exit);

MODULE_AUTHOR("chen");
MODULE_DESCRIPTION("Framebuffer driver for the linux");
MODULE_LICENSE("GPL");
