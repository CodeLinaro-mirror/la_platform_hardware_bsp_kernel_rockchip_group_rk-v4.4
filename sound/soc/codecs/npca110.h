/*
 * NPCA110.h    --  MaxAudio driver for NPCA110
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __NPCA110_H__
#define __NPCA110_H__

/*
 * Register values.
 */
#define NPCA110_SOFT_MUTE		0xC7
#define NPCA110_MAXX_VOLUME		0xD0

/* codec private data */
struct npca110_priv {
	struct i2c_client *npca110_i2c;
	struct clk *mclk;
	int gpio_det;
	int debounce_time;
	int linein_det_invert;
	bool linein_inserted;
	struct delayed_work work;
	bool playback_active;
};

#endif  /* __NPCA110_H__ */
