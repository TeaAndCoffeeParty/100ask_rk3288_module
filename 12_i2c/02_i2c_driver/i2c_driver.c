#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/i2c.h>

static const struct of_device_id of_match_ids_example[] = {
	{ .compatible = "com_name,chip_name",	.data = NULL },
	{ /* END OF LIST */ }
};

static const struct i2c_device_id example_ids[] = {
	{ "chip_name", (kernel_ulong_t)NULL },
	{ /* END OF LIST */ }
};

int i2c_driver_example_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	return 0;
}

int i2c_driver_example_remove(struct i2c_client *client)
{
	return 0;
}

static struct i2c_driver i2c_driver_example = {
	.driver = {
		.name = "example",
		.of_match_table = of_match_ids_example,
	},
	.probe    = i2c_driver_example_probe,
	.remove   = i2c_driver_example_remove,
	.id_table = example_ids,
};

static int __init i2c_driver_example_init(void)
{
	int ret = i2c_add_driver(&i2c_driver_example);
	return ret;
}

static void __exit i2c_driver_example_exit(void)
{
	i2c_del_driver(&i2c_driver_example);
}

module_init(i2c_driver_example_init);
module_exit(i2c_driver_example_exit);

MODULE_AUTHOR("chentuo");
MODULE_LICENSE("GPL");

