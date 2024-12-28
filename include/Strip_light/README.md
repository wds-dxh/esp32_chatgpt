# StripLight 

## 概述

`StripLight` 类是一个用于控制和操作 WS2812/NeoPixel LED 灯带的类。它提供了一系列的功能，包括设置单个 LED 的颜色、调节亮度、显示固定颜色、关闭灯带、实现闪烁效果、呼吸灯效果和扫描效果。

## 功能

- **构造函数**：初始化灯带，设置引脚和 LED 数量。
- **析构函数**：清除灯带显示。
- **设置颜色**：为单个 LED 设置颜色。
- **调节亮度**：调节整个灯带的亮度。
- **显示固定颜色**：将整个灯带设置为固定颜色。
- **关闭灯带**：关闭所有 LED。
- **闪烁效果**：实现灯带的闪烁效果。
- **呼吸灯效果**：实现灯带的呼吸灯效果。
- **扫描效果**：实现灯带的扫描效果。

## 构造函数

```cpp
StripLight(uint8_t pin, uint8_t num_leds);
```

- **参数**：
  - `pin`：连接到 LED 灯带的 Arduino 引脚。
  - `num_leds`：灯带上的 LED 数量。

## 析构函数

```cpp
~StripLight();
```

- 清除灯带上的所有 LED 显示。

## 设置颜色

```cpp
void setColor(uint16_t index, Color color);
```

- **参数**：
  - `index`：要设置颜色的 LED 索引（从 0 开始）。
  - `color`：要设置的颜色。

## 调节亮度

```cpp
void setBrightness(uint8_t brightness);
```

- **参数**：
  - `brightness`：亮度值（0 到 255）。

## 显示固定颜色

```cpp
void show_color(Color color);
```

- **参数**：
  - `color`：要显示的固定颜色。

## 关闭灯带

```cpp
void show_off();
```

- 关闭所有 LED。

## 闪烁效果

```cpp
void show_flash(uint8_t flash_time, Color color);
```

- **参数**：
  - `flash_time`：闪烁时间（毫秒）。
  - `color`：闪烁颜色。

## 呼吸灯效果

```cpp
void show_breath(uint8_t wait, Color color_start, Color color_end);
```

- **参数**：
  - `wait`：呼吸效果的时间间隔（毫秒）。
  - `color_start`：呼吸效果起始颜色。
  - `color_end`：呼吸效果结束颜色。

## 扫描效果

```cpp
void show_scan(uint8_t interval, Color color);
```

- **参数**：
  - `interval`：扫描效果的时间间隔（毫秒）。
  - `color`：扫描效果的颜色。

## 获取灯带任务句柄

```cpp
auto get_stripTaskHandle() -> TaskHandle_t;
```

- 返回灯带任务句柄，用于删除任务。

## 示例代码

```cpp
#include "Strip_light.hpp"

void setup() {
    StripLight strip(6, 30); // 初始化灯带，引脚 6，30 个 LED
    strip.show_color({255, 0, 0}); // 显示红色
    strip.show_flash(500, {0, 255, 0}); // 绿色闪烁
}

void loop() {
    // 循环中不需要添加代码
}
```

## 注意事项

- 在使用闪烁、呼吸和扫描效果时，这些效果会在单独的 FreeRTOS 任务中运行，因此不会阻塞主循环。
- 使用 `show_off()` 可以停止当前运行的效果，并关闭所有 LED。（包括rtos任务）
- 确保在使用这些效果时，不会同时运行多个效果，以避免冲突。
