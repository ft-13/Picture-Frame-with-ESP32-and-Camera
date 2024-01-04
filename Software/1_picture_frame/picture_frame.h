#ifndef _PICTURE_FRAME_H_
#define _PICTURE_FRAME_H_

#define STAT_OK 1
#define STAT_ERR 0

#define BATT_IP5310_ADDRESS      0x75   // IP5310 slave address
#define BATT_IP5310_REG          0x02   // register where permanent ON bit is located
#define BATT_IP5310_VALUE        0xE0   // default value in 0x02 is 01100000; set 11100000 -> 0xE0

#define BATT_SCL                 15
#define BATT_SDA                 17

#define EN_PWR_HALL              38
#define EN_PWR_DISP              9
#define EN_PWR_SD                0

#define LED_SHOT                 44     // red LED in front panel to indicate taking picture

#define SW_DISP_SD               43     // analog switch IN pin to switch SPI signals between SD card and epaper display

// three hall sensors to
// wake up ESP32
#define HALL_TOP                  3     // take picture and send to display and save to sd card
#define HALL_LEFT                18     // get previous picture from sd card and send to display
#define HALL_RIGHT               16     // get next picture from sd card and send to display
/*#define DEEP_SLEEP_BITMASK \
((2^HALL_TOP) + \
(2^HALL_LEFT) + \
(2^HALL_RIGHT))*/
// Generate deep sleep wake up bitmask:
// Bitmask = 2^3 + 2^16 + 2^18 (3 -> GPIO3, 16 -> GPIO16, 18 -> GPIO18)
// https://randomnerdtutorials.com/esp32-external-wake-up-deep-sleep/
#define DEEP_SLEEP_BITMASK      ((1<<HALL_TOP) | (1<<HALL_LEFT) | (1<<HALL_RIGHT))

// SPI signals (µSD and epaper)
// and control signals (epaper)
#define RESET                    11
#define HRDY                     10
#define MISO                     21
#define MOSI                     13
#define SCK                      12
#define CS                       14

// 9.7 inch epaper 1200x825
// camer XGA is 1024x768
// To set smaller pic (camera XGA) to the center of epaper
#define DISP_OFFSET_X            88   // (1200 - 1024) / 2 = 88
#define DISP_OFFSET_Y            28   // (825 - 768) / 2 = 28.5


// ov2640 camera signals
#define PWDN_GPIO_NUM            -1
#define RESET_GPIO_NUM           -1
#define XCLK_GPIO_NUM            45
#define SIOD_GPIO_NUM             1
#define SIOC_GPIO_NUM             2
#define Y9_GPIO_NUM              48
#define Y8_GPIO_NUM              46
#define Y7_GPIO_NUM               8
#define Y6_GPIO_NUM               7
#define Y5_GPIO_NUM               4
#define Y4_GPIO_NUM              41
#define Y3_GPIO_NUM              40
#define Y2_GPIO_NUM              39
#define VSYNC_GPIO_NUM            6
#define HREF_GPIO_NUM            42
#define PCLK_GPIO_NUM             5


extern SPIClass spi;

#endif
