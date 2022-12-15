#ifndef LEDS_TM1681_H
#define LEDS_TM1681_H

//OPRs
#define TM1681_READ			0xC0
#define TM1681_WRITE			0xA0
#define TM1681_READ_MODIFY_WRITE	0xA0
#define TM1681_COMMAND			0x80


//COMMANDs
#define TM1681_SYSDIS		0x00	//关闭系统时钟和 LED 循环
#define TM1681_SYSEN		0x01	//打开系统振荡器

#define TM1681_LEDOFF		0x02	//关闭 LED 循环
#define TM1681_LEDON		0x03	//开启 LED 循环

#define TM1681_BLINK_OFF	0x08	//关闭闪烁功能
#define TM1681_BLINK_2Hz	0x09	//LED 按 2Hz 的频率闪烁
#define TM1681_BLINK_1Hz	0x0A	//LED 按 1Hz 的频率闪烁
#define TM1681_BLINK_0_5Hz	0x0B	//LED 按 0.5Hz 的频率闪烁

#define TM1681_SLAVE		0x10	//外置振荡，时钟由 OSC 引脚输入，同步信号由 SYN 引脚输入
#define TM1681_RC_MODE0		0x18	//内置振荡， OSC 保持低电平，同步信号在 SYN 引脚保持高电平，只应用于单芯片
#define TM1681_RC_MODE1		0x1A	//内置振荡，内部频率在 OSC 输出，同步信号在 SYN 引脚输出
#define TM1681_EXT_MODE0	0x1C	//外置振荡，时钟由 OSC 引脚输入，同步信号由 SYN 引脚保持高电平，只引用于单芯片
#define TM1681_EXT_MODE1	0x1E	//外置振荡，时钟由 OSC 引脚输入，同步信号由 SYN 引脚输出

#define TM1681_N_MOS_8		0x20	//8COM Nmos
#define TM1681_N_MOS_16		0x24	//16COM Nmos
#define TM1681_P_MOS_8		0x28	//8COM Pmos
#define TM1681_P_MOS_16		0x2C	//16COM Pmos

#define TM1681_BRIGHTNESS(x)	(0xA0 | ((x) & 0x0F))	//从 0-F 变化分别对应1/16--16/16 的 LED 的 16 阶灰度调节

#define TM1681_ADDRs_8	64
#define TM1681_ADDRs_16	96

struct leds_tm1681_platform_data {
	unsigned int gpio_cs;
	unsigned int gpio_wr;
	unsigned int gpio_rd;
	unsigned int gpio_data;

	unsigned char com_mode;
	unsigned char clk_mode;
	unsigned char brightness;   //0~15
	unsigned char blink;        //TM1681_BLINK_OFF、TM1681_BLINK_2Hz、TM1681_BLINK_1Hz、TM1681_BLINK_0_5Hz

	/**
	 * 64*4显示RAM(32ROW*8COM)
	 * 96*4显示RAM(24ROW*16COM)
	 * +-------------+-------------+
	 * | bit7 - bit4 | bit3 - bit0 |
	 * +-------------+-------------+
	 * | Addr1       | Addr0       |
	 * +-------------+-------------+
	 * | Addr3       | Addr2       |
	 * +-------------+-------------+
	 */
	long unsigned int led_ram[TM1681_ADDRs_16 / 2 / 4];
};


int tm1681_brightness_set(struct platform_device *pdev, 
		int brightness);

int tm1681_blink_set(struct platform_device *pdev, 
		int blink);

int tm1681_set_led(struct platform_device *pdev, 
		const int offset, 
		const int value);

int tm1681_init(struct platform_device *pdev);

#endif //LEDS_TM1681_H

