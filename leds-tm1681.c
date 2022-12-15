#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include "leds-tm1681.h"
#include "leds-switcher.h"


/***
 * 函数功能： 写数据+写命令
 ***/
static inline void WriteCmdBits(struct platform_device *pdev,
		uint8_t sData,
		uint8_t cnt)  //高位先写
{
	struct leds_switcher_private_data *priv = platform_get_drvdata(pdev);
	struct leds_tm1681_platform_data *pdata = &priv->pdata;

	uint8_t i = 0;

	for(i = 0; i < cnt; i++)
	{
		gpio_set_value(pdata->gpio_wr, 0);
		udelay(4);
		if(sData & 0x80)
		{
			gpio_set_value(pdata->gpio_data, 1);
		}
		else
		{
			gpio_set_value(pdata->gpio_data, 0);
		}
		udelay(5);  //高电平的时间大于4us
		gpio_set_value(pdata->gpio_wr, 1);
		udelay(6);
		sData <<= 1;
	}
}

static inline void WriteDataBits(struct platform_device *pdev,
		uint8_t sData,
		uint8_t cnt)   //低位先写
{
	struct leds_switcher_private_data *priv = platform_get_drvdata(pdev);
	struct leds_tm1681_platform_data *pdata = &priv->pdata;

	uint8_t i = 0;

	for(i = 0; i < cnt; i++)
	{
		gpio_set_value(pdata->gpio_wr, 0);
		udelay(4);
		if(sData & 0x01)
		{
			gpio_set_value(pdata->gpio_data, 1);
		}
		else
		{
			gpio_set_value(pdata->gpio_data, 0);
		}
		udelay(5);
		gpio_set_value(pdata->gpio_wr, 1);
		udelay(6);
		gpio_set_value(pdata->gpio_data, 0);                             //***往地址写数据必须DATA先进行置低处理，然后再进行CLK置低，确保数据信号写入***//
		udelay(3);
		gpio_set_value(pdata->gpio_wr, 0);
		sData>>=1;
	}
}

static void WriteCmd(struct platform_device *pdev,
		uint8_t cmd)
{
	struct leds_switcher_private_data *priv = platform_get_drvdata(pdev);
	struct leds_tm1681_platform_data *pdata = &priv->pdata;

	gpio_set_value(pdata->gpio_cs, 0);
	udelay(10);

	WriteCmdBits(pdev, TM1681_COMMAND, 3);	// 写入100
	WriteCmdBits(pdev, cmd, 8);		// 写入命令
	WriteCmdBits(pdev, 0x00, 1);		// X

	gpio_set_value(pdata->gpio_wr, 0);
	udelay(5);
	gpio_set_value(pdata->gpio_data, 0);
	udelay(10);
	gpio_set_value(pdata->gpio_cs, 1);
}

static void WriteEnCmd(struct platform_device *pdev,
		uint8_t cmd)
{
	struct leds_switcher_private_data *priv = platform_get_drvdata(pdev);
	struct leds_tm1681_platform_data *pdata = &priv->pdata;

	WriteCmdBits(pdev, cmd, 8);		//写入命令
	WriteCmdBits(pdev, 0x00, 1);
	gpio_set_value(pdata->gpio_wr, 0);
	udelay(5);
	gpio_set_value(pdata->gpio_data, 0);
	udelay(20);
}

/***
  写一个字节数据：		
 ***/
static void WriteOneData(struct platform_device *pdev,
		uint8_t saddr,
		uint8_t sData)
{
	struct leds_switcher_private_data *priv = platform_get_drvdata(pdev);
	struct leds_tm1681_platform_data *pdata = &priv->pdata;

	gpio_set_value(pdata->gpio_cs, 0);
	udelay(10);

	WriteCmdBits(pdev, TM1681_WRITE, 3);	//写入101
	WriteCmdBits(pdev, saddr << 1, 7);		//写入地址
	WriteDataBits(pdev, sData, 4);		//写入数据

	gpio_set_value(pdata->gpio_cs, 1);
	gpio_set_value(pdata->gpio_wr, 1);
	udelay(5);
	gpio_set_value(pdata->gpio_data, 1);                     //***通讯结束必须CS先置高，发送STOP信号，后将通讯口全置高***//
	udelay(10);

}

/***
  函数功能：页操作
 ***/
static int WriteDatas(struct platform_device *pdev,
		uint8_t saddr,
		void *sData,
		int cnt)
{	
	struct leds_switcher_private_data *priv = platform_get_drvdata(pdev);
	struct leds_tm1681_platform_data *pdata = &priv->pdata;

	int i = 0;
	int offset = 0;

	uint8_t *pbuf = (uint8_t *)sData;

	if (sData == NULL)
		return -1;

	gpio_set_value(pdata->gpio_cs, 0);
	udelay(10);
	WriteCmdBits(pdev, TM1681_WRITE, 3);   	//写入101
	WriteCmdBits(pdev, saddr << 1, 7);	 	//写入地址

	for(i = 0; i < cnt; i++)
	{
		if (i & 1)
		{
			WriteDataBits(pdev, pbuf[offset] >> 4, 4);
			offset++;
		}
		else
			WriteDataBits(pdev, pbuf[offset], 4);

		gpio_set_value(pdata->gpio_wr, 0);
		udelay(5);
		gpio_set_value(pdata->gpio_data, 0);
	}
	udelay(10);
	gpio_set_value(pdata->gpio_cs, 1);
	gpio_set_value(pdata->gpio_wr, 1);
	udelay(5);
	gpio_set_value(pdata->gpio_data, 1);
	udelay(10);

	return i;
}

//0 ~ 15
int tm1681_brightness_set(struct platform_device *pdev,
		int brightness)
{
	struct leds_switcher_private_data *priv = platform_get_drvdata(pdev);
	struct leds_tm1681_platform_data *pdata = &priv->pdata;

	if (brightness >= 16)
		brightness = 15;
	else if (brightness < 0)
		brightness = 0;

	WriteCmd(pdev, TM1681_BRIGHTNESS(brightness));

	pdata->brightness = brightness;

	return 0;
}

int tm1681_blink_set(struct platform_device *pdev,
		int blink)
{
	struct leds_switcher_private_data *priv = platform_get_drvdata(pdev);
	struct leds_tm1681_platform_data *pdata = &priv->pdata;

	int ret = 0;

	switch (blink) {
		case TM1681_BLINK_OFF:
		case TM1681_BLINK_2Hz:
		case TM1681_BLINK_1Hz:
		case TM1681_BLINK_0_5Hz:
			break;
		default:
			return -1;
			break;
	}

	WriteCmd(pdev, blink);

	pdata->blink = blink;

	return ret;
}

int tm1681_set_led(struct platform_device *pdev,
		const int offset,
		const int value)
{
	struct leds_switcher_private_data *priv = platform_get_drvdata(pdev);
	struct leds_tm1681_platform_data *pdata = &priv->pdata;

	int addr;
	uint8_t data;
	int count = 0;

	if (offset >= 0)
	{
		if (value)
			set_bit(offset, pdata->led_ram);
		else
			clear_bit(offset, pdata->led_ram);

		addr = offset / 4;

		data = pdata->led_ram[addr / 2];
		if (addr & 1)
			data >>= 4;

		data &= 0x0F;

		WriteOneData(pdev, addr, data);

		count++;
	}

	return count;
}

int tm1681_init(struct platform_device *pdev)
{
	struct leds_switcher_private_data *priv = platform_get_drvdata(pdev);
	struct leds_tm1681_platform_data *pdata = &priv->pdata;

	int count = 0;

	gpio_set_value(pdata->gpio_cs, 1);
	gpio_set_value(pdata->gpio_wr, 0);
	gpio_set_value(pdata->gpio_data, 1);
	udelay(10);

	WriteCmd(pdev, TM1681_LEDOFF);   //LEDOFF
	WriteCmd(pdev, TM1681_SYSDIS);   //SYSDIS
	WriteCmd(pdev, pdata->com_mode);
	WriteCmd(pdev, pdata->clk_mode);
	tm1681_brightness_set(pdev, pdata->brightness);
	WriteCmd(pdev, TM1681_LEDON);   //LEDON
	WriteCmd(pdev, TM1681_SYSEN);   //SYSEN

	switch(pdata->com_mode)
	{
		case TM1681_N_MOS_8:
		case TM1681_P_MOS_8:
			count = TM1681_ADDRs_8 / 2;
			break;
		case TM1681_N_MOS_16:
		case TM1681_P_MOS_16:
			count = TM1681_ADDRs_16 / 2;
			break;
		default:
			return -EINVAL;
			break;
	}

	//pdata->led_ram = kmemdup(pdata->led_ram, count, GFP_KERNEL);

	memset((void*)pdata->led_ram, 0, count);
	WriteDatas(pdev, 0x00, pdata->led_ram, count * 2);

	return 0;
}
