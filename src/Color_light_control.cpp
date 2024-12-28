/**
 * @file ws2812b控制程序
 * @author wds
 * @date 2023/9/19
 * 邮箱：wdsnpshy@163.com
 * 博客：https://blog.csdn.net/weixin_63211230
 * qq:3412363587
*/

#include "Color_light_control.h"    
#include "Adafruit_NeoPixel.h"    //包含头文件
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define LED_PIN   2 //定义引脚
#define LED_COUNT 1 //定义灯的数量
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


/**
 * @brief   灯光控制初始化
 * @param   luminance 灯光的亮度
 * @retval  none
*/
void Color_light_control::color_light_control_init(uint8_t luminance) {
  strip.begin();  //初始化
  strip.show();   //初始化后关闭所有灯
  strip.setBrightness(luminance);  //设置灯光亮度
}

/**
 * @brief   控制灯光
 * @param   Num  灯的编号 0-29 
 * @param   color  灯的颜色 0-2 ——————RGB
 * @retval  none
*/
void Color_light_control::color_control(uint16_t Num,char color) {
  switch (color)
  {
    case 'R':
      strip.setPixelColor(Num, 255, 0, 0);
      break;
    case 'G':
      strip.setPixelColor(Num, 0, 255, 0);
      break;
    case 'B':
      strip.setPixelColor(Num, 0, 0, 255);
      break;
  }
  strip.show();

}


/**
 * @brief   关闭灯光
 * @param   Num  灯的编号 0-29
 * @retval  none
*/
void Color_light_control::color_off(uint16_t Num) {
  strip.setPixelColor(Num, 0, 0, 0);
  strip.show();
}


/**
 * @brief   七彩灯
 * @param   quantity  灯的数量
 * @retval  none
 *    注意事项：此功能需要放到一个线程中才可以实现，不能只运行一次
*/
void Color_light_control::color_riot_of_colours(uint8_t quantity) {
  for (int i = 0; i < quantity; i++) {
    strip.setPixelColor(i, random(0, 255), random(0, 255), random(0, 255));
    strip.show();
  }
  
}


/**
 * @brief   控制全部灯光
 * @param   color  灯的颜色 0-2 ——————RGB
 * @retval  none
*/
void Color_light_control::color_control_all(char color) {
  for (int i = 0; i < LED_COUNT; i++) {
    switch (color)
    {
      case 'R':
        strip.setPixelColor(i, 255, 0, 0);
        break;
      case 'G':
        strip.setPixelColor(i, 0, 255, 0);
        break;
      case 'B':
        strip.setPixelColor(i, 0, 0, 255);
        break;
    }
  }
  strip.show();
}


/**
 * @brief   闪烁灯光
 * @param   quantity_1  灯的数量
 * @param   color  灯的颜色 0-2 ——————RGB
 * @retval  none
 * 注意事项：此功能需要放到一个线程中才可以实现，不能只运行一次
*/
void Color_light_control::color_flash(uint8_t quantity_1,char color) {

  this->color_off('R');

  for (int i = 0; i < quantity_1; i++) {
    switch (color)
    {
      case 'R':
        strip.setPixelColor(i, 255, 0, 0);
        break;
      case 'G':
        strip.setPixelColor(i, 0, 255, 0);
        break;
      case 'B':
        strip.setPixelColor(i, 0, 0, 255);
        break;
    }
    strip.show();
    delay(10);
  }
  
}




/**
 * @brief   全部亮任意颜色
 * @param   R  红色亮度
 * @param   G  绿色亮度
 * @param   B  蓝色亮度
 * @retval  none
*/
void Color_light_control::color_light_all_every(int R,int G,int B) {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, R, G, B);
    strip.show();
  }
  
}


/**
 * @brief   关闭全部灯光
 * @param   quantity  灯的数量
 * @retval  none
*/
void Color_light_control::colr_light_all_off(uint8_t quantity) {
  for (int i = 0; i < quantity; i++) {
    strip.setPixelColor(i, 0, 0, 0);
    strip.show();
    delay(10);
  }
  
}
