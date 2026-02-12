/*
	F1 Racing Car - F1 赛车
*/
#include "game.h"
#include <stdio.h>
#include <SDL2/SDL.h>  // sudo apt install libsdl2-dev

#define WINDOW_W 500  // 设置窗口宽度
#define WINDOW_H 800  // 设置窗口高度

int main(void)
{
	// 初始化 SDL 视频子系统
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL_Init() is failed: %s\n", SDL_GetError());
		return -1;
	}

	// 创建窗口
	SDL_Window *window = SDL_CreateWindow(
		"F1 Racing Car",  // 窗口标题
		SDL_WINDOWPOS_CENTERED,  // 窗口初始 x 坐标 ( 居中 )
		SDL_WINDOWPOS_CENTERED,  // 窗口初始 y 坐标 ( 居中 )
		WINDOW_W,  // 窗口宽度
		WINDOW_H,  // 窗口高度
		SDL_WINDOW_SHOWN  // 窗口标志, 创建后立即显示
	);

	if (!window)  // 判断窗口是否创建成功
	{
		printf("SDL_CreateWindow() is failed: %s\n", SDL_GetError());
		SDL_Quit();  // 释放所有 SDL 分配的资源
		return -2;
	}


	SDL_Event event;  // SDL 事件结构体, event 存储事件信息
	int running = 1;  // 游戏运行标志, 1运行 0退出
	while (running)  // 事件循环
	{
		// 循环处理事件, 从事件队列中取出每一个事件
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)  // 判断事件类型是否为窗口关闭事件( 点击 x )
				running = 0;  // 退出游戏
			else if (event.type == SDL_KEYDOWN)  // 键盘按键按下
			{
				// event.key : 按键事件的数据结构
				// event.key.keysym : 按键的符号表示
				// event.key.keysym.sym : SDL_Keycode 类型, 表示具体的按键代码
				// SDL_GetKeyName() : 将 SDL_Keycode 转换为人类可读的字符串
				// 例如：SDLK_a(97) -> "A"，SDLK_SPACE(32) -> "Space"
				printf("按键 : %s\n", SDL_GetKeyName(event.key.keysym.sym));
			}
		}

		// 添加延迟，减少 CPU 占用
		// 让程序暂停 10 毫秒
		// 减少 CPU 占用率 (否则 while 循环会全速运行，占用大量 CPU)
		SDL_Delay(10);
	}

	printf("游戏结束\n");

	SDL_DestroyWindow(window);  // 销毁窗口，释放窗口资源
	SDL_Quit();  // 释放所有 SDL 分配的资源

	return 0;
}