#ifndef __FONT_H
#define __FONT_H

#include "stdint.h"
#define OLED_CHN_CHAR_WIDTH	3   // UTF-8编码格式给3，GB2312编码格式给2
typedef struct {
    const uint8_t width;
    uint8_t height;
    const uint16_t *data;
} FontDef;


typedef struct
{
    char Index[OLED_CHN_CHAR_WIDTH + 1];	//汉字索引
    uint8_t Data[32];						//字模数据
} ChineseCell_t;

extern const ChineseCell_t OLED_CF16x16[];

//Font lib.
extern FontDef Font_7x10;
extern FontDef Font_11x18;
extern FontDef Font_16x26;

//16-bit(RGB565) Image lib.
/*******************************************
 *             CAUTION:
 *   If the MCU onchip flash cannot
 *  store such huge image data,please
 *           do not use it.
 * These pics are for test purpose only.
 *******************************************/

/* 128x128 pixel RGB565 image */
extern const uint16_t saber[][128];
extern const unsigned char gImage_test[112320];
/* 240x240 pixel RGB565 image 
extern const uint16_t knky[][240];
extern const uint16_t tek[][240];
extern const uint16_t adi1[][240];
*/
#endif
