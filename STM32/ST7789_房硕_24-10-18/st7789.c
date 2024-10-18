#include "st7789.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
static void ST7789_Write_Cmd(uint8_t cmd)
{
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&ST7789_SPI_PORT, &cmd, sizeof(cmd), HAL_MAX_DELAY);
}
static void ST7789_Write_Data(uint8_t data)
{
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);
    HAL_SPI_Transmit(&ST7789_SPI_PORT, &data, sizeof(data), HAL_MAX_DELAY);
}
static void ST7789_Write_Datas(uint8_t *buff, size_t buff_size)
{
    HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);
    // 这个循环用于解决发送超过2^16个数据
    while (buff_size > 0) {
        uint16_t chunk_size = buff_size > 65535 ? 65535 : buff_size;
        HAL_SPI_Transmit(&ST7789_SPI_PORT, buff, chunk_size, HAL_MAX_DELAY);
        buff += chunk_size;     // 记录当前发送的位置，用于下次发送
        buff_size -= chunk_size;
    }
}
void ST7789_Init()
{
    HAL_GPIO_WritePin(ST7789_RST_PORT, ST7789_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(ST7789_RST_PORT, ST7789_RST_PIN, GPIO_PIN_SET);

    ST7789_Write_Cmd(0x36);		
    ST7789_Write_Data(0x00);
    ST7789_Write_Cmd(0x3A);
    ST7789_Write_Data(0x05);
    ST7789_Write_Cmd(0xB2);		
    ST7789_Write_Data(0x0C);
    ST7789_Write_Data(0x0C);
    ST7789_Write_Data(0x00);
    ST7789_Write_Data(0x33);
    ST7789_Write_Data(0x33);

    ST7789_Write_Cmd(0XB7);		
    ST7789_Write_Data(0x35);	
    ST7789_Write_Cmd(0xBB);		
    ST7789_Write_Data(0x19);	
    ST7789_Write_Cmd(0xC0);		
    ST7789_Write_Data (0x2C);	
    ST7789_Write_Cmd (0xC2);	
    ST7789_Write_Data (0x01);	
    ST7789_Write_Cmd (0xC3);	
    ST7789_Write_Data (0x12);	
    ST7789_Write_Cmd (0xC4);	
    ST7789_Write_Data (0x20);	
    ST7789_Write_Cmd (0xC6);	
    ST7789_Write_Data (0x0F);	
    ST7789_Write_Cmd (0xD0);	
    ST7789_Write_Data (0xA4);	
    ST7789_Write_Data (0xA1);

    ST7789_Write_Cmd(0xE0);
    {
        uint8_t data[] = {0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23};
        ST7789_Write_Datas(data, sizeof(data));
    }

    ST7789_Write_Cmd(0xE1);
    {
        uint8_t data[] = {0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23};
        ST7789_Write_Datas(data, sizeof(data));
    }
    ST7789_Write_Cmd (0x21);	
    ST7789_Write_Cmd (0x11);	   
    ST7789_Write_Cmd (0x29);	

    HAL_Delay(50);
    ST7789_Clear(0xFFFF);
}
static void ST7789_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint16_t x_start = x0, x_end = x1;
    uint16_t y_start = y0, y_end = y1;

    /* Column Address set */
    ST7789_Write_Cmd(0x2A);
    {
        uint8_t data[] = {x_start >> 8, x_start & 0xFF, x_end >> 8, x_end & 0xFF};
        ST7789_Write_Datas(data, sizeof(data));
    }

    /* Row Address set */
    ST7789_Write_Cmd(0x2B);
    {
        uint8_t data[] = {y_start >> 8, y_start & 0xFF, y_end >> 8, y_end & 0xFF};
        ST7789_Write_Datas(data, sizeof(data));
    }
    /* Write to RAM */
    ST7789_Write_Cmd(0x2C);

}
void ST7789_Clear(uint16_t color)
{
    uint16_t i;
    ST7789_SetAddressWindow(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);

    uint16_t j;
    for (i = 0; i < ST7789_WIDTH; i++)
        for (j = 0; j < ST7789_HEIGHT; j++) {
            uint8_t data[] = {color >> 8, color & 0xFF};
            ST7789_Write_Datas(data, sizeof(data));
        }
}
void OLED_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    if ((x < 0) || (x >= ST7789_WIDTH) ||(y < 0) || (y >= ST7789_HEIGHT))	return;

    ST7789_SetAddressWindow(x, y, x, y);
    uint8_t data[] = {color >> 8, color & 0xFF};    //高位和低位
    ST7789_Write_Datas(data, sizeof(data));
}
void ST7789_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,uint16_t color)
{
    uint16_t swap;
    uint16_t steep = ABS(y1 - y0) > ABS(x1 - x0);
    if (steep) {
        swap = x0;
        x0 = y0;
        y0 = swap;

        swap = x1;
        x1 = y1;
        y1 = swap;
    }

    if (x0 > x1) {
        swap = x0;
        x0 = x1;
        x1 = swap;

        swap = y0;
        y0 = y1;
        y1 = swap;
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = ABS(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0<=x1; x0++) {
        if (steep) {
            OLED_DrawPoint(y0, x0, color);
        } else {
            OLED_DrawPoint(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}
void ST7789_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color,uint8_t isFilled)
{
    if (isFilled==0)
    {
        ST7789_DrawLine(x1, y1, x2, y1, color);
        ST7789_DrawLine(x1, y1, x1, y2, color);
        ST7789_DrawLine(x1, y2, x2, y2, color);
        ST7789_DrawLine(x2, y1, x2, y2, color);
    }else
    {
        uint16_t i, j;
        ST7789_SetAddressWindow(x1, y1, x2, y2);
        for (i = y1; i <= y2; i++)
            for (j = x1; j <= x2; j++) {
                uint8_t data[] = {color >> 8, color & 0xFF};
                ST7789_Write_Datas(data, sizeof(data));
            }
    }
    
}

void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data)
{
    if ((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT))
        return;
    if ((x + w - 1) >= ST7789_WIDTH)
        return;
    if ((y + h - 1) >= ST7789_HEIGHT)
        return;

    ST7789_SetAddressWindow(x, y, x + w - 1, y + h - 1);
    ST7789_Write_Datas((uint8_t *)data, sizeof(uint16_t) * w * h);
}

void ST7789_ShowChar(uint16_t x, uint16_t y, uint16_t color, uint16_t bgcolor,char ch, FontDef font)
{
    uint32_t i, b, j;
    ST7789_SetAddressWindow(x, y, x + font.width - 1, y + font.height - 1);

    for (i = 0; i < font.height; i++) {
        b = font.data[(ch - 32) * font.height + i];
        for (j = 0; j < font.width; j++) {
            if ((b << j) & 0x8000) {
                uint8_t data[] = {color >> 8, color & 0xFF};
                ST7789_Write_Datas(data, sizeof(data));
            }
            else {
                uint8_t data[] = {bgcolor >> 8, bgcolor & 0xFF};
                ST7789_Write_Datas(data, sizeof(data));
            }
        }
    }
}
void ST7789_ShowString(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor)
{
    while (*str) {
        if (x + font.width >= ST7789_WIDTH) {
            x = 0;
            y += font.height;
            if (y + font.height >= ST7789_HEIGHT) {
                break;
            }

            if (*str == ' ') {
                // skip spaces in the beginning of the new line
                str++;
                continue;
            }
        }
        ST7789_ShowChar(x, y, color, bgcolor,*str, font);
        x += font.width;
        str++;
    }
}

void ST7789_ShowChinese(uint16_t x, uint16_t y, uint16_t color,uint16_t bgcolor,const uint8_t *Chinese)
{
    uint32_t i,j;
    uint8_t  byte;
    ST7789_SetAddressWindow(x, y, x + 16- 1, y + 16 - 1);
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 2; j++) {
            byte = Chinese[i*2+j];
            for(int b=0;b<8;b++)
            {
                if ((byte>>b)&0x01)
                {
                    uint8_t data[] = {color >> 8, color & 0xFF};
                    ST7789_Write_Datas(data, sizeof(data));
                }
                else {
                    uint8_t data[] = {bgcolor >> 8, bgcolor & 0xFF};
                    ST7789_Write_Datas(data, sizeof(data));
                }
            }
        }
    }
}
void ST7789_ShowChineses(int16_t X, int16_t Y, uint16_t color,uint16_t bgcolor,char *Chinese)
{
    uint8_t pChinese = 0;
    uint8_t pIndex;
    uint8_t i=0;
    char SingleChinese[OLED_CHN_CHAR_WIDTH + 1] = {0};

    for (i = 0; Chinese[i] != '\0'; i ++)		//遍历汉字串
    {
        SingleChinese[pChinese] = Chinese[i];    //提取汉字串数据到单个汉字数组
        pChinese++;                            //计次自增

        /*当提取次数到达OLED_CHN_CHAR_WIDTH时，即代表提取到了一个完整的汉字*/
        if (pChinese >= OLED_CHN_CHAR_WIDTH) {
            pChinese = 0;        //计次归零

            /*遍历整个汉字字模库，寻找匹配的汉字*/
            /*如果找到最后一个汉字（定义为空字符串），则表示汉字未在字模库定义，停止寻找*/
            for (pIndex = 0; strcmp(OLED_CF16x16[pIndex].Index, "") != 0; pIndex++) {
                /*找到匹配的汉字*/
                if (strcmp(OLED_CF16x16[pIndex].Index, SingleChinese) == 0) {
                    break;        //跳出循环，此时pIndex的值为指定汉字的索引
                }
            }

            /*将汉字字模库OLED_CF16x16的指定数据以16*16的图像格式显示*/
            // ST7789_ShowChar(X + ((i + 1) / OLED_CHN_CHAR_WIDTH - 1) * 16, Y, 16, 16, OLED_CF16x16[pIndex].Data);
            ST7789_ShowChinese(X + ((i + 1) / OLED_CHN_CHAR_WIDTH - 1) * 16, Y,color,bgcolor,OLED_CF16x16[pIndex].Data);
        }
    }
}
static uint32_t ST7789_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;	//结果默认为1
    while (Y --)			//累乘Y次
    {
        Result *= X;		//每次把X累乘到结果上
    }
    return Result;
}
void ST7789_ShowBinNum(int16_t X, int16_t Y,uint16_t color,uint16_t bgcolor, uint32_t Number, uint8_t Length, FontDef font)
{
    uint8_t i;
    for (i = 0; i < Length; i++)		//遍历数字的每一位
    {
        /*调用OLED_ShowChar函数，依次显示每个数字*/
        /*Number / OLED_Pow(2, Length - i - 1) % 2 可以二进制提取数字的每一位*/
        /*+ '0' 可将数字转换为字符格式*/
        ST7789_ShowChar(X + i * font.width, Y, color,bgcolor,Number / ST7789_Pow(2, Length - i - 1) % 2 + '0',font);
    }
}void ST7789_Printf(int16_t X, int16_t Y, uint16_t color,uint16_t bgcolor,FontDef font, char *format, ...)
{
    char String[256];						//定义字符数组
    va_list arg;							//定义可变参数列表数据类型的变量arg
    va_start(arg, format);					//从format开始，接收参数列表到arg变量
    vsprintf(String, format, arg);			//使用vsprintf打印格式化字符串和参数列表到字符数组中
    va_end(arg);							//结束变量arg
    ST7789_ShowString(X, Y, String, font,color,bgcolor);//OLED显示字符数组（字符串）
}




