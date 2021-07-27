/*
 * mma8452q.c − Yet another kernel driver for MMA8452Q Accelerator Sensor.
 */
#include <linux/module.h> /* Needed by all modules */
#include <linux/kernel.h> /* Needed for KERN_INFO */
#include <linux/i2c.h>
#define DRIVER_AUTHOR "Javad Rahimi <javad321javad@gmail.com>"
#define DRIVER_DESC "MMA8452Q Sensor Driver"


static int mma8452q_probe(struct i2c_client *client, const struct i2c_device_id
*id)
{
	int ret = 0;

	return ret;
}

static int mma8452q_remove(struct i2c_client *client)
{

	int ret = 0;

	return ret;
}

static struct i2c_device_id mma8452q_idtable[] = {
      { "mma8452q1", 0x1C },
      { "mma8452q2", 0x1D },
      { }
};

MODULE_DEVICE_TABLE(i2c, mma8452q_idtable);

static const struct of_device_id mma8452q_of_match[] = {
{ .compatible = "jrp,mma8452q" },
{}
};    
MODULE_DEVICE_TABLE(of, mma8452q_of_match);


static struct i2c_driver mma8452q_driver = {
      .driver = {
              	.name   = "mma8452q",
		.of_match_table = of_match_ptr(mma8452q_of_match),
      },

      .id_table       = mma8452q_idtable,
      .probe          = mma8452q_probe,
      .remove         = mma8452q_remove,
};
module_i2c_driver(mma8452q_driver);

/*
 * You can use strings, like this:
 */
/*
 * Get rid of taint message by declaring code as GPL.
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR); /* Who wrote this module? */
MODULE_DESCRIPTION(DRIVER_DESC); /* What does this module do */
