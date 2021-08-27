#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>

static struct i2c_adapter *g_adapter;

static int i2c_bus_virtual_xfer(struct i2c_adapter *i2c_adap,
		struct i2c_msg msgs[], int num)
{
	int i;
	for(i=0;i<num;i++) {
		// do transfer msgs[i]
	}
	return num;
}

static u32 i2c_bus_virtual_func(struct i2c_adapter *adap)
{
	return (I2C_FUNC_I2C | I2C_FUNC_SMBUS_QUICK | I2C_FUNC_SMBUS_BYTE |
			I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_WORD_DATA);
}

const struct i2c_algorithm i2c_bus_virtual_algo = {
	.master_xfer = ,
	.functionality = i2c_bus_virtual_func,
};

static int i2c_bus_virtual_probe(struct platform_device *pdev)
{
	/* get info from device tree, to set i2c_adapter or hardware  */

	/* alloc, set, register i2c_adapter */
	g_adapter = kzalloc(sizeof(*g_adapter), GFP_KERNEL);

	g_adapter->owner = THIS_MODULE;
	g_adapter->class = I2C_CLASS_HWMON | I2C_CLASS_SPD;
	g_adapter->nr = -1;
	g_adapter->algo = &i2c_bus_virtual_algo;
	snprintf(g_adapter->name, sizeof(g_adapter->name), "i2c-bus-virtual");

	i2c_add_adapter(g_adapter);		//i2c_add_numerd_adapter(g_adapter);

	return 0;
}

static int i2c_bus_virtual_remove(struct platform_device *dev)
{
	i2c_del_adapter(g_adapter);
	return 0;
}

static const struct of_device_id i2c_bus_virtual_ids[] = {
	{	.compatible = "100ask,i2c-bus-virtual",	},
	{	/* end of list */ }
};

struct platform_driver i2c_bus_virtual_driver = {
	.driver = {
		.name = "i2c-bus-virtual",
		.of_match_table = i2c_bus_virtual_ids,
	},
	.probe = i2c_bus_virtual_probe,
	.remove = i2c_bus_virtual_remove,
};

static int i2c_bus_virtual_init(void)
{
	int ret;

	ret = platform_driver_register(&i2c_bus_virtual_driver);
	if(ret)
		printk(KERN_ERR"i2c_driver register failed\n");

	return 0;
}

static void i2c_bus_virtual_exit(void)
{
	platform_driver_unregister(&i2c_bus_virtual_driver);
}

moudle_init(i2c_bus_virtual_init);
module_exit(i2c_bus_virtual_exit);

MOUDLE_AUTHOR("tuo");
MODULE_LINCESE("GPL");
