#ifndef STRIP_LIGHT_HPP
#define STRIP_LIGHT_HPP

#include <Arduino.h>
#include "Adafruit_NeoPixel.h" // 包含头文件
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "PINS.h"       // 引脚定义--->传递默认引脚

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


public:
    StripLight(); // 构造函数,默认引脚为WS2812_PIN
    ~StripLight(); // 析构函数
    void setColor(uint16_t index, Color color); // 设置灯珠颜色
    void setBrightness(uint8_t brightness); // 亮度调节
    void show_color(Color color); // 显示固定颜色
    void show_off(); // 关闭灯带
    void show_flash(uint8_t flash_time, Color color); // 闪烁效果---->需要放到loop中
    void show_breath(uint8_t wait, Color color_start, Color color_end); // 呼吸灯-->需要放到loop中
    void show_scan(uint8_t interval, Color color); // 扫描效果-->需要放到loop中
    
    auto get_stripTaskHandle() -> TaskHandle_t { return stripTaskHandle; }  // 获取灯带任务句柄,用于删除任务
};


#endif // STRIP_LIGHT_HPP
