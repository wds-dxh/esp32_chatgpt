#include <Arduino.h>

// 定义引脚
const int inputPin = 0;

void setup() {
  // 初始化串口通信
  Serial.begin(115200);

  // 设置引脚模式为上拉输入
  pinMode(inputPin, INPUT_PULLUP);

  // 打印初始化信息
  Serial.println("ESP32 GPIO 0 State Reader Initialized");
}

void loop() {
  // 读取引脚状态
  int state = digitalRead(inputPin);

  // 输出引脚状态
  if (state == HIGH) {
    Serial.println("GPIO 0 is HIGH");
  } else {
    Serial.println("GPIO 0 is LOW");
  }

  // 添加延时以避免过多输出
  delay(500);
}