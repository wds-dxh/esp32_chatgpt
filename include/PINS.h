/*
 * @Author: wds-Ubuntu22-cqu wdsnpshy@163.com
 * @Date: 2024-11-13 22:20:59
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2024-12-10 17:32:30
 * @FilePath: /arduino-esp32/include/PINS.h
 * @Description: 定义使用的引脚
 * 微信: 15310638214 
 * 邮箱：wdsnpshy@163.com 
 * Copyright (c) 2024 by ${wds-Ubuntu22-cqu}, All Rights Reserved. 
 */
#ifndef PINS_H
#define PINS_H
#include<Arduino.h>


// // 定义i2c引脚
// #define scl_pin 14
// #define sda_pin 21

// //定义按键引脚 IO38 IO39
// #define KEY1_left 38
// #define KEY2_right 39

// //定义串口引脚为默认引脚：RX TX
// #define UART_TX RX
// #define UART_RX TX

// //定义ws2812引脚
// #define WS2812_PIN 2


// MicRecorder 引脚定义
#define DEFAULT_BCK_PIN         4       //用于同步时钟
#define DEFAULT_WS_PIN          15      //片选引脚
#define DEFAULT_DATA_IN_PIN     22      //数据输入引脚

#endif // PINS_H



