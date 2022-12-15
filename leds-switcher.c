#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/leds.h>

#include "leds-tm1681.h"
#include "leds-switcher.h"


struct led_t {
	uint8_t led;
	int8_t offset[3];
};

struct led2_t {
	uint8_t led;
	int8_t offset[2][3];
};

const struct led_t g_led[] = {
	{KEY_LED_MIC1, {7, -1, -1}},
	{KEY_LED_MIC2, {6, -1, -1}},
	{KEY_LED_HDMI1, {15, -1, -1}},
	{KEY_LED_HDMI2, {14, -1, -1}},
	{KEY_LED_HDMI3, {23, -1, -1}},
	{KEY_LED_HDMI4, {22, -1, -1}},
	{KEY_LED_AUX, {31, -1, -1}},
	{KEY_LED_PGM, {30, -1, -1}},
	{KEY_LED_AUDIO_ON, {29, -1, -1}},
	{KEY_LED_MEM1, {5, -1, -1}},
	{KEY_LED_MEM2, {4, -1, -1}},
	{KEY_LED_MEM3, {13, -1, -1}},
	{KEY_LED_MEM4, {12, -1, -1}},
	{KEY_LED_MEM5, {21, -1, -1}},
	{KEY_LED_MEM6, {20, -1, -1}},
	{KEY_LED_MEM7, {28, -1, -1}},
	{KEY_LED_MEM8, {27, -1, -1}},
	{KEY_LED_MEMU, {19, -1, -1}},
	{KEY_LED_REC, {43, -1, -1}},
	{KEY_LED_PLAY, {112, -1, -1}},
	{KEY_LED_STOP, {0, -1, -1}},
	{KEY_LED_PREV_1, {3, -1, -1}},
	{KEY_LED_NEXT, {1, -1, -1}},
	{KEY_LED_LIVE, {40, -1, -1}},
	{KEY_LED_ON_AIR_1, {42, -1, -1}},
	{KEY_LED_ON_AIR_2, {41, -1, -1}},
	{KEY_LED_BKGD, {11, -1, -1}},
	{KEY_LED_KEY, {2, -1, -1}},
	{KEY_LED_DSK, {9, -1, -1}},
	{KEY_LED_FTB, {32, -1, -1}},
	{KEY_LED_MIX, {10, -1, -1}},
	{KEY_LED_DIP, {18, -1, -1}},
	{KEY_LED_WIPE, {26, -1, -1}},
	{KEY_LED_PREV_2, {72, -1, -1}},
	{KEY_LED_CUT, {17, 25, 33}},
};

const struct led2_t g_led_wr[] = {
	{KEY_LED_AUTO, {{8, 16, 24}, {56, 48, 64}}},
};

const struct led2_t g_led_rg[] = {
	{KEY_LED_AUX1, {{50, -1, -1}, {81, -1, -1}}},
	{KEY_LED_AUX2, {{66, -1, -1}, {97, -1, -1}}},
	{KEY_LED_STLL1, {{49, -1, -1}, {80, -1, -1}}},
	{KEY_LED_STLL2, {{57, -1, -1}, {88, -1, -1}}},
	{KEY_LED_STLL3, {{65, -1, -1}, {96, -1, -1}}},
	{KEY_LED_STLL4, {{73, -1, -1}, {104, -1, -1}}},
	{KEY_LED_BLACK1, {{58, -1, -1}, {89, -1, -1}}},
	{KEY_LED_BLACK2, {{74, -1, -1}, {105, -1, -1}}},
	{PUSH_LED_MAX, {{34, -1, -1}, {113, -1, -1}}},
	{PUSH_LED_MIN, {{35, -1, -1}, {114, -1, -1}}},
	{KEY_LED_PVW_1, {{47, 54, 62}, {86, 87, 95}}},
	{KEY_LED_PVW_2, {{46, 53, 61}, {85, 93, 94}}},
	{KEY_LED_PVW_3, {{45, 52, 60}, {83, 84, 92}}},
	{KEY_LED_PVW_4, {{44, 51, 59}, {82, 90, 91}}},
	{KEY_LED_PGM_1, {{70, 71, 79}, {102, 103, 111}}},
	{KEY_LED_PGM_2, {{69, 77, 78}, {101, 109, 110}}},
	{KEY_LED_PGM_3, {{67, 68, 76}, {99, 100, 108}}},
	{KEY_LED_PGM_4, {{55, 63, 75}, {98, 106, 107}}},
};

#define DEV_NAME	"leds-switcher"

#ifndef LED_MAJOR
#define LED_MAJOR	76
#endif

int dev_count = 1;

int dev_major = LED_MAJOR;
int dev_minor = 0;

static struct cdev      *led_cdev;

static struct platform_device *g_leds_device = NULL;


bool isValidLed(const int led)
{
	switch(led)
	{
		case KEY_Encoder1:
		case KNOB_Encoder1:
		case KEY_Encoder2:
		case KNOB_Encoder2:
		case PUSH_ROD:
			return false;
			break;
		default:
			break;
	}

	switch (led & 0xF0) {
		case KEYS_TYPE_Audio:
			if ((led & 0x0F) >= KEYS_Audio_NUM)
				return false;
			break;
		case KEYS_TYPE_MACRO:
			if ((led & 0x0F) >= KEYS_MACRO_NUM)
				return false;
			break;
		case KEYS_TYPE_Menu:
			if ((led & 0x0F) >= KEYS_Menu_NUM)
				return false;
			break;
		case KEYS_TYPE_Recorder:
			if ((led & 0x0F) >= KEYS_Recorder_NUM)
				return false;
			break;
		case KEYS_TYPE_Streaming:
			if ((led & 0x0F) >= KEYS_Streaming_NUM)
				return false;
			break;
		case KEYS_TYPE_Next:
			if ((led & 0x0F) >= KEYS_Next_NUM)
				return false;
			break;
		case KEYS_TYPE_PVM:
			if ((led & 0x0F) >= KEYS_PVM_NUM)
				return false;
			break;
		case KEYS_TYPE_PGM:
			if ((led & 0x0F) >= KEYS_PGM_NUM)
				return false;
			break;
		case KEYS_TYPE_Transition:
			if ((led & 0x0F) >= KEYS_Transition_NUM)
				return false;
			break;
		case KEYS_TYPE_PUSH:
			if ((led & 0x0F) >= KEYS_PUSH_NUM)
				return false;
			break;
		default:
			return false;
			break;
	}

	return true;
}

bool isValidColor(const int led, const int color)
{
	switch(led)
	{
		case KEY_LED_AUTO:
			switch (color) {
				case SWITCHER_LED_W:
				case SWITCHER_LED_R:
				case SWITCHER_LED_OFF:
					return true;
					break;
				default:
					return false;
					break;
			}
			break;
		case KEY_LED_AUX1:
		case KEY_LED_AUX2:
		case KEY_LED_STLL1:
		case KEY_LED_STLL2:
		case KEY_LED_STLL3:
		case KEY_LED_STLL4:
		case KEY_LED_BLACK1:
		case KEY_LED_BLACK2:
		case KEY_LED_PVW_1:
		case KEY_LED_PVW_2:
		case KEY_LED_PVW_3:
		case KEY_LED_PVW_4:
		case KEY_LED_PGM_1:
		case KEY_LED_PGM_2:
		case KEY_LED_PGM_3:
		case KEY_LED_PGM_4:
		case PUSH_LED_MAX:
		case PUSH_LED_MIN:
			switch (color) {
				case SWITCHER_LED_R:
				case SWITCHER_LED_G:
				case SWITCHER_LED_OFF:
					return true;
					break;
				default:
					return false;
					break;
			}
			break;
		default:
			switch (color) {
				case SWITCHER_LED_ON:
				case SWITCHER_LED_OFF:
					return true;
					break;
				default:
					return false;
					break;
			}
			break;
	}

	return false;
}

int switcher_set_led(struct platform_device *pdev,
		const int led,
		const int color)
{
	struct leds_switcher_private_data *priv = platform_get_drvdata(pdev);
	struct leds_tm1681_platform_data *pdata = &priv->pdata;

	int i, j;
	int num;

	if (! isValidLed(led))
		return -ENOENT;

	if (! isValidColor(led, color))
		return -EINVAL;

	switch(led)
	{
		case KEY_LED_AUTO:
			switch (color) {
				case SWITCHER_LED_W:
					for(j = 0; j < 3; j++)
					{
						if (g_led_wr[0].offset[0][j] >= 0)
							tm1681_set_led(pdev, g_led_wr[0].offset[0][j], 1);
						if (g_led_wr[0].offset[1][j] >= 0)
							tm1681_set_led(pdev, g_led_wr[0].offset[1][j], 0);
					}
					break;
				case SWITCHER_LED_R:
					for(j = 0; j < 3; j++)
					{
						if (g_led_wr[0].offset[0][j] >= 0)
							tm1681_set_led(pdev, g_led_wr[0].offset[0][j], 0);
						if (g_led_wr[0].offset[1][j] >= 0)
							tm1681_set_led(pdev, g_led_wr[0].offset[1][j], 1);
					}
					break;
				case SWITCHER_LED_OFF:
					for(j = 0; j < 3; j++)
					{
						if (g_led_wr[0].offset[0][j] >= 0)
							tm1681_set_led(pdev, g_led_wr[0].offset[0][j], 0);
						if (g_led_wr[0].offset[1][j] >= 0)
							tm1681_set_led(pdev, g_led_wr[0].offset[1][j], 0);
					}
					break;
				default:
					break;
			}

			break;
		case KEY_LED_AUX1:
		case KEY_LED_AUX2:
		case KEY_LED_STLL1:
		case KEY_LED_STLL2:
		case KEY_LED_STLL3:
		case KEY_LED_STLL4:
		case KEY_LED_BLACK1:
		case KEY_LED_BLACK2:
		case KEY_LED_PVW_1:
		case KEY_LED_PVW_2:
		case KEY_LED_PVW_3:
		case KEY_LED_PVW_4:
		case KEY_LED_PGM_1:
		case KEY_LED_PGM_2:
		case KEY_LED_PGM_3:
		case KEY_LED_PGM_4:
		case PUSH_LED_MAX:
		case PUSH_LED_MIN:
			num = sizeof(g_led_rg) / sizeof(struct led2_t);

			for(i = 0; i < num; i++)
			{
				if (g_led_rg[i].led == led)
				{
					switch (color) {
						case SWITCHER_LED_R:
							for(j = 0; j < 3; j++)
							{
								if (g_led_rg[i].offset[0][j] >= 0)
									tm1681_set_led(pdev, g_led_rg[i].offset[0][j], 1);
								if (g_led_rg[i].offset[1][j] >= 0)
									tm1681_set_led(pdev, g_led_rg[i].offset[1][j], 0);
							}
							break;
						case SWITCHER_LED_G:
							for(j = 0; j < 3; j++)
							{
								if (g_led_rg[i].offset[0][j] >= 0)
									tm1681_set_led(pdev, g_led_rg[i].offset[0][j], 0);
								if (g_led_rg[i].offset[1][j] >= 0)
									tm1681_set_led(pdev, g_led_rg[i].offset[1][j], 1);
							}
							break;
						case SWITCHER_LED_OFF:
							for(j = 0; j < 3; j++)
							{
								if (g_led_rg[i].offset[0][j] >= 0)
									tm1681_set_led(pdev, g_led_rg[i].offset[0][j], 0);
								if (g_led_rg[i].offset[1][j] >= 0)
									tm1681_set_led(pdev, g_led_rg[i].offset[1][j], 0);
							}
							break;
						default:
							break;
					}

					break;
				}
			}
			break;
		default:
			num = sizeof(g_led) / sizeof(struct led_t);

			for(i = 0; i < num; i++)
			{
				if (g_led[i].led == led)
				{
					switch(color)
					{
						case SWITCHER_LED_ON:
							for(j = 0; j < 3; j++)
							{
								if (g_led[i].offset[j] >= 0)
									tm1681_set_led(pdev, g_led[i].offset[j], 1);
							}
							break;
						case SWITCHER_LED_OFF:
							for(j = 0; j < 3; j++)
							{
								if (g_led[i].offset[j] >= 0)
									tm1681_set_led(pdev, g_led[i].offset[j], 0);
							}
							break;
						default:
							break;
					}

					break;
				}
			}
			break;
	}

	return 0;
}

static int leds_switcher_gpio_alloc(const unsigned int pin,
		const char *label,
		bool is_in)
{
	int ret = 0;

	ret = gpio_request(pin, label);
	if (ret == 0) {
		if (is_in)
			ret = gpio_direction_input(pin);
		else
			ret = gpio_direction_output(pin, 1);
	}

	return ret;
}

static int leds_switcher_probe(struct platform_device *pdev)
{
	struct leds_switcher_private_data *priv;
	struct leds_tm1681_platform_data *pdata;

	int ret;
	const char *s;
	u32 tmp;

	struct device_node *np = pdev->dev.of_node;
	const char *label = dev_name(&pdev->dev);

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	pdata = &priv->pdata;

	//gpio-cs
	ret = of_get_named_gpio(np, "gpio-cs", 0);
	if (ret < 0) {
		dev_err(&pdev->dev, "gpio-cs property not found\n");
		goto error_free;
	}
	pdata->gpio_cs = ret;

	ret = leds_switcher_gpio_alloc(pdata->gpio_cs, label, false);
	if (ret)
	{
		printk("\t%u (gpio_cs) error\n", pdata->gpio_cs);
		goto free_cs;
	}

	//gpio-wr
	ret = of_get_named_gpio(np, "gpio-wr", 0);
	if (ret < 0) {
		dev_err(&pdev->dev, "gpio-wr property not found\n");
		goto error_free;
	}
	pdata->gpio_wr = ret;

	ret = leds_switcher_gpio_alloc(pdata->gpio_wr, label, false);
	if (ret)
	{
		printk("\t%u (gpio_wr) error\n", pdata->gpio_wr);
		goto free_wr;
	}

	//gpio-rd
	ret = of_get_named_gpio(np, "gpio-rd", 0);
	if (ret < 0) {
		dev_err(&pdev->dev, "gpio-rd property not found\n");
		goto error_free;
	}
	pdata->gpio_rd = ret;

	ret = leds_switcher_gpio_alloc(pdata->gpio_rd, label, false);
	if (ret)
	{
		printk("\t%u (gpio_rd) error\n", pdata->gpio_rd);
		goto free_rd;
	}

	//gpio-data
	ret = of_get_named_gpio(np, "gpio-data", 0);
	if (ret < 0) {
		dev_err(&pdev->dev, "gpio-data property not found\n");
		goto error_free;
	}
	pdata->gpio_data = ret;

	ret = leds_switcher_gpio_alloc(pdata->gpio_data, label, false);
	if (ret)
	{
		printk("\t%u (gpio_data) error\n", pdata->gpio_data);
		goto free_data;
	}

	//brightness
	ret = of_property_read_u32(np, "brightness ", &tmp);
	if (ret < 0) {
		dev_err(&pdev->dev, "brightness property not found\n");
		goto error_free;
	}

	if (tmp > 15)
		tmp = 15;

	pdata->brightness = tmp;

	//com-mode
	ret = of_property_read_string(np, "com-mode", &s);
	if (strcmp(s, "N8") == 0)
	{
		pdata->com_mode = TM1681_N_MOS_8;
	}
	else if (strcmp(s, "N16") == 0)
	{
		pdata->com_mode = TM1681_N_MOS_16;
	}
	else if (strcmp(s, "P8") == 0)
	{
		pdata->com_mode = TM1681_P_MOS_8;
	}
	else if (strcmp(s, "P16") == 0)
	{
		pdata->com_mode = TM1681_P_MOS_16;
	}
	else
	{
		pdata->com_mode = TM1681_N_MOS_8;
	}

	//clk-mode
	ret = of_property_read_string(np, "clk-mode", &s);
	if (strcmp(s, "slave") == 0)
	{
		pdata->clk_mode = TM1681_SLAVE;
	}
	else if (strcmp(s, "rc0") == 0)
	{
		pdata->clk_mode = TM1681_RC_MODE0;
	}
	else if (strcmp(s, "rc1") == 0)
	{
		pdata->clk_mode = TM1681_RC_MODE1;
	}
	else if (strcmp(s, "ext0") == 0)
	{
		pdata->clk_mode = TM1681_EXT_MODE0;
	}
	else if (strcmp(s, "ext1") == 0)
	{
		pdata->clk_mode = TM1681_EXT_MODE1;
	}
	else
	{
		pdata->clk_mode = TM1681_RC_MODE1;
	}

	pdev->dev.platform_data = pdata;

	g_leds_device = pdev;

	tm1681_init(pdev);

	platform_set_drvdata(pdev, priv);

	return 0;

free_data:
	gpio_free(pdata->gpio_data);
free_rd:
	gpio_free(pdata->gpio_rd);
free_wr:
	gpio_free(pdata->gpio_wr);
free_cs:
	gpio_free(pdata->gpio_cs);
error_free:
	devm_kfree(&pdev->dev, pdata);

	return ret;
}

static int leds_switcher_remove(struct platform_device *pdev)
{
	struct leds_switcher_private_data *priv = platform_get_drvdata(pdev);
	struct leds_tm1681_platform_data *pdata = &priv->pdata;

	gpio_free(pdata->gpio_data);
	gpio_free(pdata->gpio_rd);
	gpio_free(pdata->gpio_wr);
	gpio_free(pdata->gpio_cs);

	devm_kfree(&pdev->dev, pdata);

	return 0;
}

static const struct of_device_id of_leds_switcher_match[] = {
	{ .compatible = "leds-switcher", },
	{},
};

MODULE_DEVICE_TABLE(of, of_leds_switcher_match);

static struct platform_driver leds_switcher_driver = {
	.driver		= {
		.name	= "leds-switcher",
		.owner	= THIS_MODULE,
		.of_match_table	= of_match_ptr(of_leds_switcher_match),
	},
	.probe		= leds_switcher_probe,
	.remove		= leds_switcher_remove,
};

//module_platform_driver(leds_switcher_driver);

static int leds_switcher_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int leds_switcher_release(struct inode *inode, struct file *file)
{
	return 0;
}

static long leds_switcher_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int blink = 0;

	switch(cmd)
	{
		case IOCTL_LED_OFF:
		case IOCTL_LED_ON:
		case IOCTL_LED_W:
		case IOCTL_LED_R:
		case IOCTL_LED_G:
			switcher_set_led(g_leds_device, arg, cmd);
			break;
		case IOCTL_BRIGHTNESS:
			tm1681_brightness_set(g_leds_device, arg);
			break;
		case IOCTL_BLINK:
			switch(arg)
			{
				case LEDS_BLINK_OFF:
					blink = TM1681_BLINK_OFF;
					break;
				case LEDS_BLINK_2Hz:
					blink = TM1681_BLINK_2Hz;
					break;
				case LEDS_BLINK_1Hz:
					blink = TM1681_BLINK_1Hz;
					break;
				case LEDS_BLINK_0_5Hz:
					blink = TM1681_BLINK_0_5Hz;
					break;
				default:
					return -EINVAL;
					break;
			}

			tm1681_blink_set(g_leds_device, blink);
			break;
		default:
			printk(KERN_ERR "%s driver don't support ioctl command=%d\n", DEV_NAME, cmd);
			return -ENOTTY;
			break;
	}

	return 0;
}

static struct file_operations led_switcher_fops =
{
	.owner = THIS_MODULE,
	.open = leds_switcher_open,
	.release = leds_switcher_release,
	.unlocked_ioctl = leds_switcher_ioctl,
};

static int __init leds_switcher_init(void)
{
	int                    result;
	dev_t                  devno;

	result = platform_driver_register(&leds_switcher_driver);
	if (result)
		printk(KERN_ERR "leds-switcher: register failed: %d\n", result);

	/*  Alloc the device for driver */
	if (0 != dev_major) /*  Static */
	{
		//分配设备号
		devno = MKDEV(dev_major, 0);
		result = register_chrdev_region (devno, dev_count, DEV_NAME);
	}
	else
	{
		result = alloc_chrdev_region(&devno, dev_minor, dev_count, DEV_NAME);
		dev_major = MAJOR(devno);
	}

	/*  Alloc for device major failure */
	if (result < 0)
	{
		printk(KERN_ERR "%s driver can't use major %d\n", DEV_NAME, dev_major);
		return -ENODEV;
	} 
	printk(KERN_DEBUG "%s driver use major %d\n", DEV_NAME, dev_major);

	//注册字符设备结构体
	if(NULL == (led_cdev = cdev_alloc()) )
	{
		printk(KERN_ERR "%s driver can't alloc for the cdev.\n", DEV_NAME);
		unregister_chrdev_region(devno, dev_count);
		return -ENOMEM;
	}

	led_cdev->owner = THIS_MODULE;
	cdev_init(led_cdev, &led_switcher_fops);//结构体初始化

	result = cdev_add(led_cdev, devno, dev_count);//结构体注册到内核
	if (0 != result)
	{
		printk(KERN_INFO "%s driver can't reigster cdev: result=%d\n", DEV_NAME, result);
		goto ERROR;
	}


	printk(KERN_ERR "%s driver[major=%d] version 1.0.0 installed successfully!\n", DEV_NAME, dev_major);
	return 0;

ERROR:
	printk(KERN_ERR "%s driver installed failure.\n", DEV_NAME);
	cdev_del(led_cdev);
	unregister_chrdev_region(devno, dev_count);
	return result;
}

static void __exit leds_switcher_exit(void)
{
	dev_t devno = MKDEV(dev_major, dev_minor);

	cdev_del(led_cdev);
	unregister_chrdev_region(devno, dev_count);

	platform_driver_unregister(&leds_switcher_driver);

	printk(KERN_ERR "%s driver version 1.0.0 removed!\n", DEV_NAME);

	return ;
}

module_init(leds_switcher_init);
module_exit(leds_switcher_exit);

module_param(dev_major, int, S_IRUGO);

MODULE_AUTHOR("Jackie Lee(OSEE)");
MODULE_DESCRIPTION("les-switcher driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:leds-switcher");
