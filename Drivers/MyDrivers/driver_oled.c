#include "driver_oled.h"
#include "i2c.h"
#include <string.h>
#include <stdio.h>

#define OLED_DEV_ADDRESS                0x78
#define FOLLOW_DATA_TYPE_IS_COMMAND     0x00
#define FOLLOW_DATA_TYPE_IS_DATA        0x40
#define WIDTH                           128
#define HEIGHT                          64
#define PAGES                           (HEIGHT / 8)  // = 8


char g_xStateList[][10] = {{"Stopped!"}, {"Running!"}, {"Ready!"}};


/**
 * @brief 写数据
 * @param data 数据字节
 */
static void OLED_Write_Data(uint8_t data)
{
    HAL_I2C_Mem_Write(&hi2c1, OLED_DEV_ADDRESS, FOLLOW_DATA_TYPE_IS_DATA, I2C_MEMADD_SIZE_8BIT, &data, 1, 0x10);
}

/**
 * @brief 写命令
 * @param command 命令字节
 */
static void OLED_Write_Command(uint8_t command)
{
    HAL_I2C_Mem_Write(&hi2c1, OLED_DEV_ADDRESS, FOLLOW_DATA_TYPE_IS_COMMAND, I2C_MEMADD_SIZE_8BIT, &command, 1, 0x10);
}

/**
 * @brief 初始化
 */
void OLED_Init(void)
{
    //等待上电稳定
    HAL_Delay(200);

    //关闭显示
    OLED_Write_Command(0xAE);

    //Set MUX Ratio A8h, 3Fh
    OLED_Write_Command(0xA8);
    OLED_Write_Command(0x3F);

    //Set Display Offset D3h, 00h
    OLED_Write_Command(0xD3);
    OLED_Write_Command(0x00);

    //Set Display Start Line 40h
    OLED_Write_Command(0x40);

    //Set Segment re-map A0h/A1h
    OLED_Write_Command(0xA1);

    //Set COM Output Scan Direction C0h/C8h
    OLED_Write_Command(0xC8);

    //Set COM Pins hardware configuration DAh, 02/12
    OLED_Write_Command(0xDA);
    OLED_Write_Command(0x12);

    //Set Contrast Control 81h, 7Fh/CFh
    OLED_Write_Command(0x81); 
    OLED_Write_Command(0xCF); 

    //预充电周期
    OLED_Write_Command(0xD9); 
    OLED_Write_Command(0xF1); 

    //设置VCOMH取消选择级别
    OLED_Write_Command(0xDB); 
    OLED_Write_Command(0x30); 


    //设置充电泵
    OLED_Write_Command(0x8D); 
    OLED_Write_Command(0x14); 

    //Disable Entire Display On A4h
    OLED_Write_Command(0xA4);

    //Set Normal Display A6h
    OLED_Write_Command(0xA6);

    //Set Osc Frequency  D5h, 80h
    OLED_Write_Command(0xD5);
    OLED_Write_Command(0x80);

    //Enable charge pump regulator 8Dh, 14h
    OLED_Write_Command(0x8D);
    OLED_Write_Command(0x14);
    
    //Display On AFh
    OLED_Write_Command(0xAF);

    OLED_Clear();
}


/**
 * @brief 设置像素点页地址和起始列地址，连续写多个字节，列地址自动自增
 * @param x 像素点起始列坐标(0~127)
 * @param y 像素点坐标(0~63)
 */
static void OLED_SetCursor(uint8_t x, uint8_t y)
{
    //根据行坐标计算页地址
    uint8_t page = y / 8;

    //设置页地址，设置列低地址，设置列的高地址
    OLED_Write_Command(0xB0 + page);
    OLED_Write_Command(0x00 | (x & 0x0F)); //低四位
    OLED_Write_Command(0x10 | (x & 0xF0)>>4); //高四位
}


/**
 * @brief 清屏
 */
 void OLED_Clear(void)
 {
    uint8_t i,j;
    uint8_t data = 0;

    for(i = 0; i < PAGES; i++)
    {
        OLED_SetCursor(0, i*8); //按页擦
        for(j = 0; j < WIDTH; j++)
        {
            OLED_Write_Data(data);
        }
    }
 }
 
#if 0
/**
 * @brief 画点
 * @param x 像素点起始列坐标(0~127)
 * @param y 像素点坐标(0~63)
 */
void OLED_DrawPoint(uint8_t x, uint8_t y)
{
    uint8_t data;

    //检查像素点坐标是否合理
    if((x >= WIDTH) || (y >= HEIGHT))
        return;

    data = (1 << y % 8);
    OLED_SetCursor(x, y);
    OLED_Write_Data(data);
}
#endif

/**
 * @brief 在 OLED 上显示字符串（仅支持 ASCII 字符和部分自定义符号）
 * @param cur_x     起始列坐标（0~127）
 * @param cur_x     起始行坐标（0~63，以像素为单位）
 * @param str       要显示的字符串（以 '\0' 结尾）
 */
void OLED_ShowString(uint8_t x, uint8_t y, char *str) 
{
    for(uint8_t i = 0; *str != '\0'; i++) //字符宽度为8
    {
        OLED_ShowChar(x + i*8, y, *str++); //字符串
    }
}


/**
 * @brief 在 OLED 上显示字符（仅支持 ASCII 字符和部分自定义符号）
 * @param x     起始列坐标（0~127）
 * @param y     起始行坐标（0~63，以像素为单位）
 * @param ch    要显示的字符，°非ascii标准字符使用宏SYM_DEGREE代表
 */
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t ch)
{
    uint8_t i;
    uint8_t rowId = (ch == SYM_DEGREE) ? SYM_DEGREE : (ch - ' '); //获取目标字符在字符库所在的行

    if (x + 8 > WIDTH || y + 16 > HEIGHT) 
		return;

    //设置上半部分字符在OLED的页地址和起始列地址
    OLED_SetCursor(x, y);

    //设置上半部分字符数据写入OLED的GDDRAM,字符的像素大小为8x16，占据两页，8列
    for(i = 0; i < 8; i++)
        OLED_Write_Data(ascii_8x16[rowId][i]);
	

    //设置上半部分字符在OLED的页地址和起始列地址
    OLED_SetCursor(x, y + 8);

	//设置下半部分字符
    for(i = 8; i < 16; i++)
		 OLED_Write_Data(ascii_8x16[rowId][i]);
}

/**
 * @brief 在 OLED 上显示一个整数（十进制）
 * @param x     起始列坐标（0~127）
 * @param y     起始行坐标（0~63）
 * @param num   要显示的整数（支持负数）
 */
void OLED_ShowNum(uint8_t x, uint8_t y, int32_t num) 
{
    char buf[16];               // 足够容纳 int32_t 的十进制字符串
    sprintf(buf, "%ld", num);   // 转换为十进制字符串
    OLED_ShowString(x, y, buf);
}

/**
 * @brief 在 OLED 上显示一个浮点数（默认保留一位小数）
 * @param x     起始列坐标（0~127）
 * @param y     起始行坐标（0~63）
 * @param num   要显示的浮点数
 * @param decimals 小数位数（0~3 为宜）
 */
void OLED_ShowFloat(uint8_t x, uint8_t y, float num, uint8_t decimals) 
{
    char buf[16];   // 缓冲区足够容纳带符号、小数点和最多3位小数的浮点数
    // 使用 sprintf 格式化，例如 "%.1f" 保留一位小数
    sprintf(buf, "%.*f", decimals, num);
    OLED_ShowString(x, y, buf);
}




void OLED_Test(void)
{
	OLED_Clear();
    OLED_ShowString(0, 0, "Waiting!");
	OLED_ShowChar(0, 16, 'X');
	OLED_ShowNum(0, 32, 25);
	OLED_ShowFloat(0, 48, 27.63, 1);
	OLED_ShowChar(32, 48, SYM_DEGREE);
	OLED_ShowChar(40, 48, '%');
	HAL_Delay(2000);
}







