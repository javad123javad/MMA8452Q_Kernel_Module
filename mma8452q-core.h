#ifndef _MMA845Q_CORE_H_
#define _MMA845Q_CORE_H_
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
/* For iio interface */
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/iio/events.h>
#include <linux/iio/buffer.h>

#define DRIVER_AUTHOR "Javad Rahimi <javad321javad@gmail.com>"
#define DRIVER_DESC "MMA8452Q Sensor Driver"
/* Device registers and constants */
#define MMA8452_STATUS				0x00
#define MMA8452_STATUS_DRDY			(BIT(2) | BIT(1) | BIT(0))
#define MMA8452_OUT_X				0x01 /* MSB first */
#define MMA8452_OUT_Y				0x03
#define MMA8452_OUT_Z				0x05
#define MMA8452_SYS_MOD				0x0b
#define MMA8452_INT_SRC				0x0c
#define MMA8452_WHO_AM_I			0x0d
#define MMA8452_DATA_CFG			0x0e
#define MMA8452_DATA_CFG_FS_MASK		GENMASK(1, 0)
#define MMA8452_DATA_CFG_FS_2G			0
#define MMA8452_DATA_CFG_FS_4G			1
#define MMA8452_DATA_CFG_FS_8G			2
#define MMA8452_DATA_CFG_HPF_MASK		BIT(4)
#define MMA8452_HP_FILTER_CUTOFF		0x0f
#define MMA8452_HP_FILTER_CUTOFF_SEL_MASK	GENMASK(1, 0)
#define MMA8452_FF_MT_CFG			0x15
#define MMA8452_FF_MT_CFG_OAE			BIT(6)
#define MMA8452_FF_MT_CFG_ELE			BIT(7)
#define MMA8452_FF_MT_SRC			0x16
#define MMA8452_FF_MT_SRC_XHE			BIT(1)
#define MMA8452_FF_MT_SRC_YHE			BIT(3)
#define MMA8452_FF_MT_SRC_ZHE			BIT(5)
#define MMA8452_FF_MT_THS			0x17
#define MMA8452_FF_MT_THS_MASK			0x7f
#define MMA8452_FF_MT_COUNT			0x18
#define MMA8452_FF_MT_CHAN_SHIFT		3
#define MMA8452_TRANSIENT_CFG			0x1d
#define MMA8452_TRANSIENT_CFG_CHAN(chan)	BIT(chan + 1)
#define MMA8452_TRANSIENT_CFG_HPF_BYP		BIT(0)
#define MMA8452_TRANSIENT_CFG_ELE		BIT(4)
#define MMA8452_TRANSIENT_SRC			0x1e
#define MMA8452_TRANSIENT_SRC_XTRANSE		BIT(1)
#define MMA8452_TRANSIENT_SRC_YTRANSE		BIT(3)
#define MMA8452_TRANSIENT_SRC_ZTRANSE		BIT(5)
#define MMA8452_TRANSIENT_THS			0x1f
#define MMA8452_TRANSIENT_THS_MASK		GENMASK(6, 0)
#define MMA8452_TRANSIENT_COUNT			0x20
#define MMA8452_TRANSIENT_CHAN_SHIFT		1
#define MMA8452_CTRL_REG1			0x2a
#define MMA8452_CTRL_ACTIVE			BIT(0)
#define MMA8452_CTRL_DR_MASK			GENMASK(5, 3)
#define MMA8452_CTRL_DR_SHIFT			3
#define MMA8452_CTRL_DR_DEFAULT		0x4 /* 50 Hz sample frequency */
#define MMA8452_CTRL_REG2			0x2b
#define MMA8452_CTRL_REG2_RST			BIT(6)
#define MMA8452_CTRL_REG2_MODS_SHIFT		3
#define MMA8452_CTRL_REG2_MODS_MASK		0x1b
#define MMA8452_CTRL_REG4			0x2d
#define MMA8452_CTRL_REG5			0x2e
#define MMA8452_OFF_X				0x2f
#define MMA8452_OFF_Y				0x30
#define MMA8452_OFF_Z				0x31

#define MMA8452_MAX_REG				0x31

#define MMA8452_INT_DRDY			BIT(0)
#define MMA8452_INT_FF_MT			BIT(2)
#define MMA8452_INT_TRANS			BIT(5)

#define MMA8451_DEVICE_ID			0x1a
#define MMA8452_DEVICE_ID			0x2a
#define MMA8453_DEVICE_ID			0x3a
#define MMA8652_DEVICE_ID			0x4a
#define MMA8653_DEVICE_ID			0x5a
#define FXLS8471_DEVICE_ID			0x6a

#define MMA8452_AUTO_SUSPEND_DELAY_MS		2000

struct mma8452q_dev
{
	struct i2c_client *client;
	
};

//u16 MMA845x_Standby(struct mma8452q_dev * mma_dev);
//u16 MMA845x_Active(struct mma8452q_dev * mma_dev);
//u8 MMA845x_Is_Active(struct mma8452q_dev * mma_dev);

#endif
