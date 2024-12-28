#ifndef COLOR_LIGHT_CONTROL_H
#define COLOR_LIGHT_CONTROL_H

#include <Arduino.h>
/**
  * @brief    灯光控制类
  */
class Color_light_control{

private:

public:
    void color_light_control_init(uint8_t luminance);  // 初始化灯光控制
    void color_control(uint16_t Num,char color);  // 控制单个灯光灯光
    void color_off(uint16_t Num);  // 关闭单个灯光
    void color_light_all_every(int R,int G,int B);  // 全部灯光亮度调节
    void color_control_all(char color);  // 控制全部灯光

    void color_flash(uint8_t quantity_1,char color);  // 闪烁灯光
    void color_riot_of_colours(uint8_t quantity);  // 七彩灯

    void colr_light_all_off(uint8_t quantity);  // 关闭全部灯光
    





};






#endif