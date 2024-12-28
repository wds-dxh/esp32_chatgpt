#include "Strip_light/Strip_light.hpp"



Adafruit_NeoPixel strip(LED_COUNT, WS2812_PIN, NEO_GRB + NEO_KHZ800); // 创建对象
// 实现构造函数
StripLight::StripLight(){

    strip.begin();
    strip.setBrightness(20);
    strip.show();
}

StripLight::~StripLight() {
    strip.clear();
    strip.show();
}

/**
 * @description: 设置灯珠颜色
 */
void StripLight::setColor(uint16_t index, Color color) {
    strip.setPixelColor(index, strip.Color(color.r, color.g, color.b));
    strip.show();
}

/**
 * @description: 设置亮度
 */
void StripLight::setBrightness(uint8_t brightness) {
    strip.setBrightness(brightness);
    strip.show();
}

/**
 * @description: 显示固定颜色
 */
void StripLight::show_color(Color color) {
    strip.fill(strip.Color(color.r, color.g, color.b), 0, num_leds);
    strip.show();
}

/**
 * @description: 关闭灯带
 */
void StripLight::show_off() {
    // 删除其他任务
    if (stripTaskHandle != NULL) {
        vTaskDelete(stripTaskHandle);
        stripTaskHandle = NULL;
    }
    strip.clear();
    strip.show();
}

/**
 * @description: 闪烁效果
 */
void StripLight::show_flash(uint8_t flash_time, Color color) {
    // 删除其他任务
    if (stripTaskHandle != NULL) {
        vTaskDelete(stripTaskHandle);
        stripTaskHandle = NULL;
    }
    // 定义任务函数
    auto flashTask = [this, flash_time, color]() {
        while (true) {
            strip.fill(strip.Color(color.r, color.g, color.b), 0, num_leds);
            strip.show();
            vTaskDelay(pdMS_TO_TICKS(flash_time));
            strip.fill(strip.Color(0, 0, 0), 0, num_leds);
            strip.show();
            vTaskDelay(pdMS_TO_TICKS(flash_time));
        }
    };
    // 创建任务
    xTaskCreate([](void* param) {
        auto task = static_cast<decltype(flashTask)*>(param);
        (*task)();
        delete task; // 这一步执行不到，因为任务函数中有 while(true)
        vTaskDelete(NULL); // 删除任务
    }, "flashTask", 
    2048, 
    new decltype(flashTask)(flashTask), // 传递参数
    1, 
    &stripTaskHandle);
}

/**
 * @description: 呼吸灯效果
 */
void StripLight::show_breath(uint8_t wait, Color color_start, Color color_end) {
    // 删除其他任务
    if (stripTaskHandle != NULL) {
        vTaskDelete(stripTaskHandle);
        stripTaskHandle = NULL;
    }
    // 定义任务函数
    auto breathTask = [this, wait, color_start, color_end]() {
        while (true) {
            for (int i = 0; i <= 255; i++) {
                Color color;
                color.r = map(i, 0, 255, color_start.r, color_end.r); // 映射
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
    // 创建任务
    xTaskCreate([](void* param) {
        auto task = static_cast<decltype(breathTask)*>(param);
        (*task)();
        delete task; // 这一步执行不到，因为任务函数中有 while(true)
        vTaskDelete(NULL); // 删除任务
    }, "breathTask", 
    2048, 
    new decltype(breathTask)(breathTask), // 传递参数
    1, 
    &stripTaskHandle);
}

/**
 * @description: 扫描效果 
 */
void StripLight::show_scan(uint8_t interval, Color color) {
    // 删除其他任务
    if (stripTaskHandle != NULL) {
        vTaskDelete(stripTaskHandle);
        stripTaskHandle = NULL;
    }
    // 定义任务函数
    auto scanTask = [this, interval, color]() {
        while (true) {
            for (uint16_t i = 0; i < num_leds; i++) {
                strip.setPixelColor(i, strip.Color(color.r, color.g, color.b));
                strip.show();
                vTaskDelay(pdMS_TO_TICKS(interval));
                strip.setPixelColor(i, strip.Color(0, 0, 0));
                strip.show();
            }
            for (int16_t i = num_leds - 1; i >= 0; i--) { // 修改为 int16_t 避免无符号下溢
                strip.setPixelColor(i, strip.Color(color.r, color.g, color.b));
                strip.show();
                vTaskDelay(pdMS_TO_TICKS(interval));
                strip.setPixelColor(i, strip.Color(0, 0, 0));
                strip.show();
            }
        }
    };
    // 创建任务
    xTaskCreate([](void* param) {
        auto task = static_cast<decltype(scanTask)*>(param);
        (*task)();
        delete task; // 这一步执行不到，因为任务函数中有 while(true)
        vTaskDelete(NULL); // 删除任务
    }, "scanTask", 
    2048, 
    new decltype(scanTask)(scanTask), // 传递参数
    1, 
    &stripTaskHandle);
}
