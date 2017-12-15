/*
 * Driver for virtual codec
 *
 * Copyright (C) 2017 Rockchip Electronics Co., Ltd.
 * Author: Li Dongqiang <David.li@rock-chips.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#include <sound/soc.h>

static const struct snd_soc_dapm_widget rockchip_dummy_codec_widgets[] = {
	SND_SOC_DAPM_INPUT("RX"),
	SND_SOC_DAPM_OUTPUT("TX"),
};

static const struct snd_soc_dapm_route rockchip_dummy_codec_routes[] = {
	{ "Capture", NULL, "RX" },
	{ "TX", NULL, "Playback" },
};

#define rockchip_dummy_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | \
			SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE)

static struct snd_soc_dai_driver rockchip_dummy_codec_dai = {
	.name = "dummy_codec",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_192000,
		.formats = rockchip_dummy_FORMATS,
	},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 8,
		.rates = SNDRV_PCM_RATE_8000_192000,
		.formats = rockchip_dummy_FORMATS,
	},
};

static struct snd_soc_codec_driver soc_codec_dev_rockchip_dummy_codec = {
	.dapm_widgets = rockchip_dummy_codec_widgets,
	.num_dapm_widgets = ARRAY_SIZE(rockchip_dummy_codec_widgets),
	.dapm_routes = rockchip_dummy_codec_routes,
	.num_dapm_routes = ARRAY_SIZE(rockchip_dummy_codec_routes),
};

static int rockchip_dummy_codec_probe(struct platform_device *pdev)
{
	return snd_soc_register_codec(&pdev->dev, &soc_codec_dev_rockchip_dummy_codec,
			&rockchip_dummy_codec_dai, 1);
}

static int rockchip_dummy_codec_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);

	return 0;
}

static const struct of_device_id rockchip_dummy_codec_of_match[] = {
	{ .compatible = "rockchip,dummy-codec", },
	{},
};
MODULE_DEVICE_TABLE(of, rockchip_dummy_codec_of_match);

static struct platform_driver rockchip_dummy_codec_driver = {
	.driver = {
		.name = "dummy_codec",
		.of_match_table = of_match_ptr(rockchip_dummy_codec_of_match),
	},
	.probe = rockchip_dummy_codec_probe,
	.remove = rockchip_dummy_codec_remove,
};

module_platform_driver(rockchip_dummy_codec_driver);

MODULE_AUTHOR("Li Dongqiang <David.li@rock-chips.com>");
MODULE_DESCRIPTION("ASoC rockchip dummy virtual driver");
MODULE_LICENSE("GPL");
