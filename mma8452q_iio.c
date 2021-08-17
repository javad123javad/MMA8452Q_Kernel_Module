/*
 * mma8452q.c − Yet another kernel driver for MMA8452Q Accelerator Sensor.
 */
#include "mma8452q-core.h"
#include <linux/of_device.h>
#include <linux/of_irq.h>
//struct mma8452q_dev *mma_dev;
#define MMA8452_WHO_AM_I			0x0d

static struct i2c_device_id mma8452q_idtable[] = {
    {"mma8452q1", 0x1C}, {"mma8452q2", 0x1D}, {}};

MODULE_DEVICE_TABLE(i2c, mma8452q_idtable);

static const struct of_device_id mma8452q_of_match[] = {
    {.compatible = "jrp,mma8452q"}, {}};
MODULE_DEVICE_TABLE(of, mma8452q_of_match);


/**
 * struct mma_chip_info - chip specific data
 * @chip_id:			WHO_AM_I register's value
 * @channels:			struct iio_chan_spec matching the device's
 *				capabilities
 * @num_channels:		number of channels
 * @mma_scales:			scale factors for converting register values
 *				to m/s^2; 3 modes: 2g, 4g, 8g; 2 integers
 *				per mode: m/s^2 and micro m/s^2
 * @all_events:			all events supported by this chip
 * @enabled_events:		event flags enabled and handled by this driver
 */
struct mma_chip_info {
	u8 chip_id;
	const struct iio_chan_spec *channels;
	int num_channels;
	const int mma_scales[3][2];
	int all_events;
	int enabled_events;
};
enum {
	mod_standby,
	mod_wake,
	mod_sleep,
};
enum {
	idx_x,
	idx_y,
	idx_z,
	idx_ts,
};
enum {
	mma8451,
	mma8452,
	mma8453,
	mma8652,
	mma8653,
	fxls8471,
};
static const struct iio_event_spec mma8452_transient_event[] = {
	{
		.type = IIO_EV_TYPE_MAG,
		.dir = IIO_EV_DIR_RISING,
		.mask_separate = BIT(IIO_EV_INFO_ENABLE),
		.mask_shared_by_type = BIT(IIO_EV_INFO_VALUE) |
					BIT(IIO_EV_INFO_PERIOD) |
					BIT(IIO_EV_INFO_HIGH_PASS_FILTER_3DB)
	},
};
static const struct iio_event_spec mma8452_freefall_event[] = {
	{
		.type = IIO_EV_TYPE_MAG,
		.dir = IIO_EV_DIR_FALLING,
		.mask_separate = BIT(IIO_EV_INFO_ENABLE),
		.mask_shared_by_type = BIT(IIO_EV_INFO_VALUE) |
					BIT(IIO_EV_INFO_PERIOD) |
					BIT(IIO_EV_INFO_HIGH_PASS_FILTER_3DB)
	},
};
#define MMA8452_FREEFALL_CHANNEL(modifier) { \
	.type = IIO_ACCEL, \
	.modified = 1, \
	.channel2 = modifier, \
	.scan_index = -1, \
	.event_spec = mma8452_freefall_event, \
	.num_event_specs = ARRAY_SIZE(mma8452_freefall_event), \
}

#define MMA8452_CHANNEL(axis, idx, bits) { \
	.type = IIO_ACCEL, \
	.modified = 1, \
	.channel2 = IIO_MOD_##axis, \
	.info_mask_separate = BIT(IIO_CHAN_INFO_RAW) | \
			      BIT(IIO_CHAN_INFO_CALIBBIAS), \
	.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SAMP_FREQ) | \
			BIT(IIO_CHAN_INFO_SCALE) | \
			BIT(IIO_CHAN_INFO_HIGH_PASS_FILTER_3DB_FREQUENCY) | \
			BIT(IIO_CHAN_INFO_OVERSAMPLING_RATIO), \
	.scan_index = idx, \
	.scan_type = { \
		.sign = 's', \
		.realbits = (bits), \
		.storagebits = 16, \
		.shift = 16 - (bits), \
		.endianness = IIO_BE, \
	}, \
	.event_spec = mma8452_transient_event, \
	.num_event_specs = ARRAY_SIZE(mma8452_transient_event), \
}
static const struct iio_chan_spec mma8452_channels[] = {
	MMA8452_CHANNEL(X, idx_x, 12),
	MMA8452_CHANNEL(Y, idx_y, 12),
	MMA8452_CHANNEL(Z, idx_z, 12),
	IIO_CHAN_SOFT_TIMESTAMP(idx_ts),
	MMA8452_FREEFALL_CHANNEL(IIO_MOD_X_AND_Y_AND_Z),
};

struct mma8452_data {
	struct i2c_client *client;
	struct mutex lock;
	u8 ctrl_reg1;
	u8 data_cfg;
	const struct mma_chip_info *chip_info;
	int sleep_val;
	struct regulator *vdd_reg;
	struct regulator *vddio_reg;

	/* Ensure correct alignment of time stamp when present */
	struct {
		__be16 channels[3];
		s64 ts __aligned(8);
	} buffer;
};

static const struct mma_chip_info mma_chip_info_table[] = {
	[mma8452] = {
		.chip_id = MMA8452_DEVICE_ID,
		.channels = mma8452_channels,
		.num_channels = ARRAY_SIZE(mma8452_channels),
		.mma_scales = { {0, 9577}, {0, 19154}, {0, 38307} },
		/*
		 * Although we enable the interrupt sources once and for
		 * all here the event detection itself is not enabled until
		 * userspace asks for it by mma8452_write_event_config()
		 */
		.all_events = MMA8452_INT_DRDY |
					MMA8452_INT_TRANS |
					MMA8452_INT_FF_MT,
		.enabled_events = MMA8452_INT_TRANS |
					MMA8452_INT_FF_MT,
	},
};

static const struct of_device_id mma8452_dt_ids[] = {
        { .compatible = "jrp,mma8452q", .data = &mma_chip_info_table[mma8452] },
        { }
};
MODULE_DEVICE_TABLE(of, mma8452_dt_ids);

static const unsigned long mma8452_scan_masks[] = {0x7, 0};//User can enable all or disable all

/* Methods for IIO Dev */
static int mma8452_set_mode(struct mma8452_data *data, const int mode)
{
    int ret = 0;
    
    // Check current mode from 0x0B
    ret = i2c_smbus_read_byte_data(data->client, MMA8452_SYS_MOD);
    if(mod_standby != ret)
    {
        pr_info("Sysmode:0x%02x\n", ret);
	return -EIO;
    }

    ret = i2c_smbus_read_byte_data(data->client, MMA8452_CTRL_REG1);
    switch(mode)
    {
    	case(0)://Standby Mode
		ret = i2c_smbus_write_byte_data(data->client, MMA8452_CTRL_REG1, (ret | mod_standby));
		
	break;
	case(1)://Wake mode
 		ret = i2c_smbus_write_byte_data(data->client, MMA8452_CTRL_REG1, (ret | mod_wake));
	break;
	
	case(2):
		ret = ret = i2c_smbus_write_byte_data(data->client, MMA8452_CTRL_REG1, (ret | mod_sleep));
	break;

	default:
		ret = -1;
	break;
    }

	pr_info("Sysmode[after]: 0x%02x\n", i2c_smbus_read_byte_data(data->client, MMA8452_SYS_MOD));
	

    return ret;
}

static int mma8452_rd(struct mma8452_data *data, __be16 buf[])
{
	int ret = 0;
	ret = i2c_smbus_read_i2c_block_data(data->client, MMA8452_OUT_X, 3 * sizeof(__be16),(u8*) buf);
	
	return ret ;
	
	
}
static int mma8452_read_raw(struct iio_dev *indio_dev,
			    struct iio_chan_spec const *chan,
			    int *val, int *val2, long mask)
{
  struct mma8452_data *data = iio_priv(indio_dev);
  int ret = 0;
  __be16 buf[4] = {0};

  mutex_lock(&data->lock);
  pr_info("Calling Read Raw.chan->scan_type.shift:%di\tchan->scan_type.realbits:%d\n",chan->scan_type.shift, chan->scan_type.realbits);
  
  switch (mask) {
	case IIO_CHAN_INFO_RAW:
	  ret = mma8452_rd(data, buf);
	  if(ret < 0)
	  {
		pr_err("Error in Reading Axes: %d.", ret);
		return ret;	  
	  }
	  *val = sign_extend32(be16_to_cpu(buf[chan->scan_index]) >> chan->scan_type.shift, chan->scan_type.realbits - 1);
	  ret = IIO_VAL_INT;
	break;
  default:
    ret = -EINVAL;//*/
  }

  mutex_unlock(&data->lock);
  return ret;
}

static int mma8452_write_raw(struct iio_dev *indio_dev,
			     struct iio_chan_spec const *chan,
			     int val, int val2, long mask)
{
  int ret = 0;
  pr_info("Calling Write Raw.\n");
  return ret;

}

static const struct iio_info mma8452_info = {
  .read_raw = &mma8452_read_raw,
  .write_raw = &mma8452_write_raw,

};

static int mma8452q_probe(struct i2c_client *client,
                          const struct i2c_device_id *id) {
  int ret = 0;
  
  const struct of_device_id *match;
  struct iio_dev *indio_dev;
  struct mma8452_data *data;
  __be16 buf[6] = {0};
  match = of_match_device(mma8452_dt_ids, &client->dev);
  if(!match)
  {
    dev_err(&client->dev,"No Match Device Found.\n");

  }	
  pr_info("Device Found: %s\n", match->compatible);
  
  indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*data));
  if(!indio_dev)
    return -ENOMEM;

  data = iio_priv(indio_dev);
  data->client = client;
  mutex_init(&data->lock);
  data->chip_info = match->data;
  
  ret = i2c_smbus_read_byte_data(client, MMA8452_WHO_AM_I);
  if(ret < 0)
    goto nodevfound;
  pr_info("My ID: 0x%02X\n", ret);
  mma8452_set_mode(data, mod_wake);  
  mma8452_rd(data, buf);
  i2c_set_clientdata(client, indio_dev);
  indio_dev->info = &mma8452_info;
  indio_dev->name = id->name;
  indio_dev->modes = INDIO_DIRECT_MODE;
  indio_dev->channels = data->chip_info->channels;
  indio_dev->num_channels = data->chip_info->num_channels;
  indio_dev->available_scan_masks= mma8452_scan_masks;
  
  ret = iio_device_register(indio_dev);
  if(ret < 0)
  {
    dev_err(&client->dev, "Unable to register iio dev:%d\n", ret);
    
  }
  return 0;

nodevfound:
   iio_device_free(indio_dev);
   return 0;
}

static int mma8452q_remove(struct i2c_client *client) {

  int ret = 0;
  struct iio_dev *indio_dev = i2c_get_clientdata(client);
  //struct mma8452_data *data = iio_priv(indio_dev);

  iio_device_unregister(indio_dev); 
  //if (mma_dev) {
//    i2c_unregister_device(client);
    //kfree(mma_dev);
  //}
  return ret;
}


static struct i2c_driver mma8452q_driver = {
    .driver =
        {
            .name = "mma8452q",
            .of_match_table = of_match_ptr(mma8452_dt_ids),
        },

    .id_table = mma8452q_idtable,
    .probe = mma8452q_probe,
    .remove = mma8452q_remove,
};
module_i2c_driver(mma8452q_driver);

/*
 * You can use strings, like this:
 */
/*
 * Get rid of taint message by declaring code as GPL.
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);    /* Who wrote this module? */
MODULE_DESCRIPTION(DRIVER_DESC); /* What does this module do */
