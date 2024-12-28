/*
 * @Author: wds-Ubuntu22-cqu wdsnpshy@163.com
 * @Date: 2024-11-20 18:36:30
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2024-12-29 00:07:15
 * @FilePath: /Wireless_transmission/include/Strip_light.hpp
 * @Description: 灯带驱动
 * 微信: 15310638214
 * 邮箱：wdsnpshy@163.com
 * Copyright (c) 2024 by ${wds-Ubuntu22-cqu}, All Rights Reserved.
 */
#ifndef STRIP_LIGHT_HPP
#define STRIP_LIGHT_HPP
#include <Arduino.h>
#include "Adafruit_NeoPixel.h" //包含头文件
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "PINS.h"       //引脚定义--->传递默认引脚
class StripLight    
{
public:
    struct Color {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };
private:
    uint8_t pin;        // 引脚
    uint8_t num_leds;   // 灯珠数量
    TaskHandle_t stripTaskHandle;       // 灯带任务句柄，把需要循环执行的函数放到任务中
    Adafruit_NeoPixel strip; // 灯带实例

public:
    StripLight(uint8_t pin, uint8_t num_leds = WS2812_PIN); // 构造函数,默认引脚为WS2812_PIN
    ~StripLight();//析构函数
    void setColor(uint16_t index, Color color); // 设置灯珠颜色
    void setBrightness(uint8_t brightness);//亮度调节
    void show_color(Color color);//显示固定颜色
    void show_off();//关闭灯带
    void show_flash(uint8_t flash_time, Color color);//闪烁效果---->需要放到loop中
    void show_breath(uint8_t wait, Color color_start, Color color_end);//呼吸灯-->需要放到loop中
    void show_scan(uint8_t interval, Color color);//扫描效果-->需要放到loop中
    
    auto get_stripTaskHandle() -> TaskHandle_t { return stripTaskHandle; }  // 获取灯带任务句柄,用于删除任务
    
};

//实现构造函数
StripLight::StripLight(uint8_t pin, uint8_t num_leds) : pin(pin), num_leds(num_leds), strip(num_leds, pin, NEO_GRB + NEO_KHZ800) {
    strip.begin();
    strip.setPixelColor(0, strip.Color(255, 0, 0));
    strip.setBrightness(100);
    strip.show();
}

StripLight::~StripLight() {
    strip.clear();
    strip.show();
}
/**
 * @description: 设置灯珠颜色
 * @param {uint16_t} index 灯珠索引
 * @param {Color} color 颜色
 * @return {*}
 */
void StripLight::setColor(uint16_t index, Color color) {
    strip.setPixelColor(index, strip.Color(color.r, color.g, color.b));
    
    strip.show();
}

/**
 * @description: 设置亮度
 * @param {uint8_t} brightness 亮度
 * @return {*}
 */
void StripLight::setBrightness(uint8_t brightness) {
    strip.setBrightness(brightness);
    strip.show();
}

/**
 * @description: 显示固定颜色
 * @param {Color} color
 * @return {*}
 */
void StripLight::show_color(Color color) {
    strip.fill(strip.Color(color.r, color.g, color.b), 0, num_leds);
    strip.show();
}

/**
 * @description: 关闭灯带
 * @param {*}
 * @return {*}
 */
void StripLight::show_off() {
    //删除其他任务
    if (stripTaskHandle != NULL) {
        vTaskDelete(stripTaskHandle);
        stripTaskHandle = NULL;
    }
    strip.clear();
    strip.show();
}

/**
 * @description: 闪烁效果
 * @param {uint8_t} flash_time  闪烁时间,单位ms
 * @param {Color} color 闪烁颜色
 * @return {*}
 */
void StripLight::show_flash(uint8_t flash_time, Color color) {
    //删除其他任务
    if (stripTaskHandle != NULL) {
        vTaskDelete(stripTaskHandle);
        stripTaskHandle = NULL;
    }
    //定义任务函数-->捕获外部变量：flash_time, color
    auto flashTask = [this, flash_time, color]() {
        while (true) {
            strip.fill(strip.Color(color.r, color.g, color.b), 0, num_leds);
            strip.show();
            vTaskDelay(pdMS_TO_TICKS(flash_time));
            this->show_off();
            strip.show();
            vTaskDelay(pdMS_TO_TICKS(flash_time));
        }
    };
    //创建任务
    xTaskCreate([](void* param) {
        auto task = static_cast<decltype(flashTask)*>(param);
        (*task)();
        delete task;       //释放内存---->执行不到这一步，因为任务函数中有while(true)
        vTaskDelete(NULL);  //删除任务
    }, "flashTask", 
    2048, 
    new decltype(flashTask)(flashTask),    //传递参数
    1, 
    &stripTaskHandle);
}


/**
 * @description: 呼吸灯效果
 * @param {uint8_t} wait    呼吸时间间隔,单位ms
 * @param {Color} color_start   呼吸起始颜色
 * @param {Color} color_end     呼吸结束颜色
 * @return {*}
 */
void StripLight :: show_breath(uint8_t wait, Color color_start, Color color_end) {
    //删除其他任务
    if (stripTaskHandle != NULL) {
        vTaskDelete(stripTaskHandle);
        stripTaskHandle = NULL;
    }
    //定义任务函数-->捕获外部变量：wait, color_start, color_end
    auto breathTask = [this, wait, color_start, color_end]() {
        while (true) {
            for (int i = 0; i <= 255; i++) {
                Color color;
                color.r = map(i, 0, 255, color_start.r, color_end.r);   //映射
                color.g = map(i, 0, 255, color_start.g, color_end.g);
                color.b = map(i, 0, 255, color_start.b, color_end.b);
                strip.fill(strip.Color(color.r, color.g, color.b), 0, num_leds);
                strip.show();
                vTaskDelay(pdMS_TO_TICKS(wait));
            }
            for (int i = 255; i >= 0; i--) {
                Color color;
                color.r = map(i, 0, 255, color_start.r, color_end.r);
                color.g = map(i, 0, 255, color_start.g, color_end.g);
                color.b = map(i, 0, 255, color_start.b, color_end.b);
                strip.fill(strip.Color(color.r, color.g, color.b), 0, num_leds);
                strip.show();
                vTaskDelay(pdMS_TO_TICKS(wait));
            }
        }
    };
    //创建任务
    xTaskCreate([](void* param) {
        auto task = static_cast<decltype(breathTask)*>(param);
        (*task)();
        delete task;       //释放内存---->执行不到这一步，因为任务函数中有while(true)
        vTaskDelete(NULL);  //删除任务
    }, "breathTask", 
    2048, 
    new decltype(breathTask)(breathTask),    //传递参数
    1, 
    &stripTaskHandle);
}

/**
 * @description: 扫描效果 
 * @param {uint8_t} interval    扫描间隔,单位ms
 * @param {Color} color    扫描颜色
 * @return {*}
 */
void StripLight :: show_scan(uint8_t interval, Color color) {
    //删除其他任务
    if (stripTaskHandle != NULL) {
        vTaskDelete(stripTaskHandle);
        stripTaskHandle = NULL;
    }
    //定义任务函数-->捕获外部变量：interval, color
    auto scanTask = [this, interval, color]() {
        while (true) {
            for (uint16_t i = 0; i < num_leds; i++) {
                strip.setPixelColor(i, strip.Color(color.r, color.g, color.b));
                strip.show();
                vTaskDelay(pdMS_TO_TICKS(interval));
                strip.setPixelColor(i, strip.Color(0, 0, 0));
                strip.show();
            }
            for (uint16_t i = num_leds - 1; i >= 0; i--) {
                strip.setPixelColor(i, strip.Color(color.r, color.g, color.b));
                strip.show();
                vTaskDelay(pdMS_TO_TICKS(interval));
                strip.setPixelColor(i, strip.Color(0, 0, 0));
                strip.show();
            }
        }
    };
    //创建任务
    xTaskCreate([](void* param) {
        auto task = static_cast<decltype(scanTask)*>(param);
        (*task)();
        delete task;       //释放内存---->执行不到这一步，因为任务函数中有while(true)
        vTaskDelete(NULL);  //删除任务
    }, "scanTask", 
    2048, 
    new decltype(scanTask)(scanTask),    //传递参数
    1, 
    &stripTaskHandle);
}

#endif // STRIP_LIGHT_HPP
