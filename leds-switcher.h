#ifndef LEDS_SWITCHER_H
#define LEDS_SWITCHER_H

#include "leds-tm1681.h"


//Audio
#define KEYS_TYPE_Audio     0x00
#define KEYS_Audio_NUM      0x0B

#define KEY_LED_MIC1        0x00
#define KEY_LED_MIC2        0x01
#define KEY_LED_HDMI1       0x02
#define KEY_LED_HDMI2       0x03
#define KEY_LED_HDMI3       0x04
#define KEY_LED_HDMI4       0x05
#define KEY_LED_AUX         0x06
#define KEY_LED_PGM         0x07
#define KEY_LED_AUDIO_ON    0x08
#define KEY_Encoder1        0x09
#define KNOB_Encoder1       0x0A

//MACRO
#define KEYS_TYPE_MACRO     0x10
#define KEYS_MACRO_NUM      0x08

#define KEY_LED_MEM1        0x10
#define KEY_LED_MEM2        0x11
#define KEY_LED_MEM3        0x12
#define KEY_LED_MEM4        0x13
#define KEY_LED_MEM5        0x14
#define KEY_LED_MEM6        0x15
#define KEY_LED_MEM7        0x16
#define KEY_LED_MEM8        0x17

//Menu
#define KEYS_TYPE_Menu      0x20
#define KEYS_Menu_NUM       0x03

#define KEY_LED_MEMU        0x20
#define KEY_Encoder2        0x21
#define KNOB_Encoder2       0x22

//Media Recorder
#define KEYS_TYPE_Recorder  0x30
#define KEYS_Recorder_NUM   0x05

#define KEY_LED_REC         0x30
#define KEY_LED_PLAY        0x31
#define KEY_LED_STOP        0x32
#define KEY_LED_PREV_1      0x33
#define KEY_LED_NEXT        0x34

//Streaming
#define KEYS_TYPE_Streaming 0x40
#define KEYS_Streaming_NUM  0x01

#define KEY_LED_LIVE        0x40

//Next Transition
#define KEYS_TYPE_Next 0x50
#define KEYS_Next_NUM  0x06

#define KEY_LED_ON_AIR_1    0x50
#define KEY_LED_ON_AIR_2    0x51
#define KEY_LED_BKGD        0x52
#define KEY_LED_KEY         0x53
#define KEY_LED_DSK         0x54
#define KEY_LED_FTB         0x55

//PVM
#define KEYS_TYPE_PVM       0x60
#define KEYS_PVM_NUM        0x08

#define KEY_LED_PVW_1       0x60
#define KEY_LED_PVW_2       0x61
#define KEY_LED_PVW_3       0x62
#define KEY_LED_PVW_4       0x63
#define KEY_LED_AUX1        0x64
#define KEY_LED_BLACK1      0x65
#define KEY_LED_STLL1       0x66
#define KEY_LED_STLL2       0x67

//PGM
#define KEYS_TYPE_PGM       0x70
#define KEYS_PGM_NUM        0x08

#define KEY_LED_PGM_1       0x70
#define KEY_LED_PGM_2       0x71
#define KEY_LED_PGM_3       0x72
#define KEY_LED_PGM_4       0x73
#define KEY_LED_AUX2        0x74
#define KEY_LED_BLACK2      0x75
#define KEY_LED_STLL3       0x76
#define KEY_LED_STLL4       0x77

//Transition Style
#define KEYS_TYPE_Transition 0x80
#define KEYS_Transition_NUM  0x06

#define KEY_LED_MIX         0x80
#define KEY_LED_DIP         0x81
#define KEY_LED_WIPE        0x82
#define KEY_LED_PREV_2      0x83
#define KEY_LED_CUT         0x84
#define KEY_LED_AUTO        0x85

//Push Rod
#define KEYS_TYPE_PUSH      0x90
#define KEYS_PUSH_NUM       0x03

#define PUSH_ROD            0x90	//推杆
#define PUSH_LED_MAX        0x91
#define PUSH_LED_MIN        0x92


enum switcher_led_color_t {
	SWITCHER_LED_OFF = 0,
	SWITCHER_LED_ON,
	SWITCHER_LED_W,
	SWITCHER_LED_R,
	SWITCHER_LED_G,
};

#define IOCTL_LED_OFF		0
#define IOCTL_LED_ON		1
#define IOCTL_LED_W		2
#define IOCTL_LED_R		3
#define IOCTL_LED_G		4
#define IOCTL_BRIGHTNESS	5
#define IOCTL_BLINK		6

#define LEDS_BLINK_OFF        0    //关闭闪烁功能
#define LEDS_BLINK_2Hz        1    //LED 按 2Hz 的频率闪烁
#define LEDS_BLINK_1Hz        2    //LED 按 1Hz 的频率闪烁
#define LEDS_BLINK_0_5Hz      3    //LED 按 0.5Hz 的频率闪烁


struct leds_switcher_private_data {
	struct leds_tm1681_platform_data pdata;
};

#endif //LEDS_SWITCHER_H

