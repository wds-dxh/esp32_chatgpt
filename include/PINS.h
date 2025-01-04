/*
 * @Author: wds-Ubuntu22-cqu wdsnpshy@163.com
 * @Date: 2024-11-13 22:20:59
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2025-01-04 19:22:27
 * @FilePath: /arduino-esp32/include/PINS.h
 * @Description: 定义使用的引脚
 * 微信: 15310638214 
 * 邮箱：wdsnpshy@163.com 
 * Copyright (c) 2024 by ${wds-Ubuntu22-cqu}, All Rights Reserved. 
 */
#pragma once

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
#define WS2812_PIN 2
#define LED_COUNT 1 //定义灯的数量


/**************************************esp32 s3引脚定义*************************************************** */
// MicRecorder 引脚定义
// #define MicRecorder_DEFAULT_BCK_PIN         45       //用于同步时钟
// #define MicRecorder_DEFAULT_WS_PIN          12      //片选引脚
// #define MicRecorder_DEFAULT_DATA_IN_PIN     9      //数据输入引脚


// // Megaphone 引脚定义
// #define Megaphone_DEFAULT_BCK_PIN              40
// #define Megaphone_DEFAULT_WS_PIN               42
// #define Megaphone_DEFAULT_DATA_OUT_PIN         1




/**************************************esp32 引脚定义*************************************************** */
// MicRecorder 引脚定义
#define MicRecorder_DEFAULT_BCK_PIN         4       //用于同步时钟
#define MicRecorder_DEFAULT_WS_PIN          15      //片选引脚
#define MicRecorder_DEFAULT_DATA_IN_PIN     22      //数据输入引脚


// Megaphone 引脚定义
#define Megaphone_DEFAULT_BCK_PIN              26
#define Megaphone_DEFAULT_WS_PIN               27
#define Megaphone_DEFAULT_DATA_OUT_PIN         25
 




