#ifndef __HOME_H
#define __HOME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

// 窗口尺寸
#define HOME_WINDOW_W 800
#define HOME_WINDOW_H 500

// 按钮结构体
typedef struct
{
    int x, y;  // 按钮中心位置
    char text[50];  // 按钮文字
    int width, height;  // 按钮大小
    int hover;  // 鼠标悬停状态
} Button;

// 主页相关函数声明
int run_home_page(void);  // 运行主页，返回选择的按钮 ID
void init_home_page(void);  // 初始化主页
void cleanup_home_page(void);  // 清理主页资源

// 按钮 ID 定义
#define BUTTON_START_GAME 1
#define BUTTON_CAR_SELECT 2
#define BUTTON_TRACK_SELECT 3
#define BUTTON_EXIT 4

#endif