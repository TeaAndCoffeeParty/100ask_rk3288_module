#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>


struct i2c_client *ap3216_client;
struct i2c_board_info ap3216_info = {
	.type = "ap3216c",
	.addr = 0x1e,
};
const unsigned short addr_list[] = {
	0x1e,
	I2C_CLIENT_END
};

static int i2c_client_ap3216_init(void)
{
	struct i2c_adapter *adapter;

	adapter = i2c_get_adapter(0);
	if(!adapter) {
		printk(KERN_ERR"i2c_get_adapter(0) failed\n");
		return -EINVAL;
	}

#if 1
	ap3216_client = i2c_new_device(adapter, &ap3216_info);
#else
	ap3216_client = i2c_new_probed_device(adapter, &ap3216_info, addr_list, NULL);
#endif
	if(!ap3216_client) {
		printk(KERN_ERR"can't create i2c device %s\n", ap3216_info.type);
		return -EINVAL;
	}

	i2c_put_adapter(adapter);
	return 0;
}

static void i2c_client_ap3216_exit(void)
{
	i2c_unregister_device(ap3216_client);
}

module_init(i2c_client_ap3216_init);
module_exit(i2c_client_ap3216_exit);

MODULE_AUTHOR("chentuo");
MODULE_LICENSE("GPL");
