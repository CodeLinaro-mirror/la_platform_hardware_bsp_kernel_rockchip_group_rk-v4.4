/*
 * ntp8810.c  --  NTP8810_C Digital Audio Amplifier driver
 *
 * Copyright NeoFidelity, Inc.
 * Author: Yeevan <showy.zhang@rock-chips.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/regmap.h>

#include <linux/mfd/syscon.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <sound/pcm_params.h>
#include <sound/dmaengine_pcm.h>
#include <linux/rockchip/grf.h>



#if 1
static unsigned char reg_default[] = {
	0x02,0x01,  // Modulation Reset Control
	0x00,0x00,  // Master Clock Frequency Control
	0x40,0x0E,  // DBTL MLP
	0x45,0x23,  // Modulation index :
	0x3A,0x02,  // Miscellaneous PWM Control DBTL
	0x2A,0xA8,  // 1 Band DRC setting
	0x2B,0x01,  // Attack _ Release Setting
	0x08,0x7D,  // Prescaler 10Watt@6ohm/ PVDD 13V
	0x06,0xB7,  // Channel 1  volume
	0x07,0xB7,  // Channel 2  volume
#if 1
	0x32,0x04,  // PWM MASK ON
	0x31,0x00,  // PWM Switching ON
	0x30,0x00,   // Soft-mute OFF
	0x04,0xFF,   //Master volume Setting
#endif
};
#endif
#if 0
static unsigned char reg_playback_on[] = {
	0x32,0x04,  // PWM MASK ON
	0x31,0x00,  // PWM Switching ON
	0x30,0x00,  // Soft-mute OFF
	0x04,0xFF,  //Master volume Setting
};

static unsigned char reg_playback_off[] = {
	0x30,0x03,  // Soft-mute ON
	0x31,0x03,  // PWM Switching OFF
	0x32,0x06,  // PWM MASK OFF
};
#endif
int ntp8810_reset_gpio;
int ntp8810_power_en_gpio;
struct i2c_client *client;

int i2c_8bit_write(struct i2c_client *i2c_client, unsigned char *RegList, int size)
{
	int index;
	int ret = 0;

	for (index = 0; index < (size / 2); index++)
	{
		ret = i2c_master_send(i2c_client, RegList, 2);
		printk("### %s, %d, [%0x]:[%0x]\n", __func__, index, RegList[0], RegList[1]);
		RegList += 2;

		if (ret < 0)
		{
			printk("### %s,%d,ret =%d,index=%d\n\n",__func__,__LINE__,ret ,index);
			return -1;
		}
	}

	return 0;
}

int ntp8810_playback(int mute)
{
	int ret = 0;
	if (!mute) {
		//ret = i2c_8bit_write(client, reg_playback_on, ARRAY_SIZE(reg_playback_on));
	} else {
		//ret = i2c_8bit_write(client, reg_playback_off, ARRAY_SIZE(reg_playback_off));
	}

	return ret;
}
EXPORT_SYMBOL_GPL(ntp8810_playback);

static int ntp8810_i2c_probe(struct i2c_client *i2c_client,
		const struct i2c_device_id *id)
{
	int ret = -1;
	struct device_node *np = i2c_client->dev.of_node;
	enum of_gpio_flags flags;
	int ntp8810_reset_gpio;
	struct regmap *grf;

	grf = syscon_regmap_lookup_by_phandle(np,
			"rockchip,grf");
	if (IS_ERR(grf)) {
		pr_err("missing 'rockchip,grf'\n");
		return PTR_ERR(grf);
	}

	pr_err(">>>>>>>>>>>>>> %s \n",  __func__);
	/*power enable gpio*/
	ntp8810_power_en_gpio = of_get_named_gpio_flags(np, "pwd-en-gpio",
			0, &flags);
	if (ntp8810_power_en_gpio < 0) {
		pr_err("%s Can not read property ntp8810 pwd-en-gpio\n",
				__func__);
		ntp8810_power_en_gpio = -1;
	} else {
		ret = gpio_request(ntp8810_power_en_gpio, NULL);
		if (!ret) {
			printk("requset ntp8810-power-en-gpio success!\n");
			gpio_direction_output(ntp8810_power_en_gpio, 1);
		} else {
			pr_err("requset pwd-en-gpio failed:%d\n", ret);
			return ret;
		}
	}

	/*power reset gpio*/
	ntp8810_reset_gpio = of_get_named_gpio_flags(np, "pwd-reset-gpio",
			0, &flags);

	if (ntp8810_reset_gpio < 0) {
		pr_err("%s Can not read property ntp8810 pwd-reset-gpio\n",
				__func__);
		ntp8810_reset_gpio = -1;
	} else {
		printk("ntp8810 pwd-reset-gpio = %d\n", ntp8810_reset_gpio);
		ret = gpio_request(ntp8810_reset_gpio, NULL);
		if (!ret) {
			printk("requset ntp8810-reset-gpio success!\n");

			regmap_write(grf, RK3228_GRF_SOC_CON6, (1<<(8+16)));
			gpio_direction_output(ntp8810_reset_gpio, 1);
			printk(">>>%d\n",gpio_get_value(ntp8810_reset_gpio));
			msleep(10);
			//while(1) {
			gpio_direction_output(ntp8810_reset_gpio, 0);
			printk(">>>%d\n",gpio_get_value(ntp8810_reset_gpio));
			msleep(10);
			gpio_direction_output(ntp8810_reset_gpio, 1);
			printk(">>>%d\n",gpio_get_value(ntp8810_reset_gpio));
			msleep(2000);
			//}
			msleep(10);
			//gpio_direction_output(ntp8810_reset_gpio, 1);
			msleep(10);
			//gpio_free(ntp8810_reset_gpio);
		} else {
			pr_err("requset pwd-con-gpio failed:%d\n", ret);
			return ret;
		}
	}
	client = i2c_client;

	ret = i2c_8bit_write(i2c_client, reg_default, ARRAY_SIZE(reg_default));

	if (ret != 0) {
		pr_err("ntp8810 probe error\n");
		return ret;
	}

	return 0;
}

static const struct i2c_device_id ntp8810_i2c_id[] = {
	{"ntp8810_c", 0},
	{ }
};
MODULE_DEVICE_TABLE(i2c, ntp8810_i2c_id);

static struct i2c_driver ntp8810_i2c_driver = {
	.driver = {
		.name = "ntp8810",
		.owner = THIS_MODULE,
	},
	.probe = ntp8810_i2c_probe,
	.id_table = ntp8810_i2c_id,
};

static int __init ntp8810_init(void)
{
	return i2c_add_driver(&ntp8810_i2c_driver);
}

static void __exit ntp8810_exit(void)
{
	return i2c_del_driver(&ntp8810_i2c_driver);
}

late_initcall_sync(ntp8810_init);
//module_init(ntp8810_init);
module_exit(ntp8810_exit);

MODULE_DESCRIPTION("Digital Audio Amplifier ntp8810 driver");
MODULE_AUTHOR("Yeevan <showy.zhang@rock-chips.com>");
MODULE_LICENSE("GPL");
