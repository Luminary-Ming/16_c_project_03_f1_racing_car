/*
	F1 Racing Car - F1 赛车
*/
#include <stdio.h>
#include <SDL2/SDL.h>        // sudo apt install libsdl2-dev -y
#include <SDL2/SDL_image.h>  // sudo apt install libsdl2-image-dev -y
#include <SDL2/SDL_mixer.h>  // sudo apt install libsdl2-mixer-dev -y
#include "game.h"
#include "car.h"

#define WINDOW_W 500  // 设置窗口宽度
#define WINDOW_H 800  // 设置窗口高度

#define CAR_WIDTH 70    // 赛车宽
#define CAR_HEIGHT 130  // 赛车高

int main(void)
{
	// 初始化 SDL 视频子系统
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		printf("SDL_Init() failed: %s\n", SDL_GetError());
		return -1;
	}

	// 初始化 SDL_mixer 音频库
	// 44100 CD音质, 数值越高，音质越好, 但 CPU 占用越高
	// 2  声道数, 双声道, 立体声 ( 1单声道  4环绕声) 
	// 2048 块大小, 每次从音频缓冲区读取多少字节, 影响延迟和稳定性
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		printf("Mix_OpenAudio() failed: %s\n", Mix_GetError());
		SDL_Quit();
		return -2;
	}

	// 初始化 SDL_image 库，支持 PNG 格式
	if (IMG_Init(IMG_INIT_PNG) == 0)
	{
		printf("IMG_Init() failed: %s\n", IMG_GetError());
		SDL_Quit();
		return -3;
	}

	// 加载背景音乐
	Mix_Music *bgm = Mix_LoadMUS("assets/bgm/Insomnia - Craig David.mp3");
	if (!bgm)
	{
		printf("无法加载背景音乐: %s\n", Mix_GetError());
		// 继续运行，没有音乐也可以玩
	}

	// 游戏音乐库
	const char *bgms[] = {
		"assets/bgm/Insomnia - Craig David.mp3",
		"assets/bgm/Lady Gaga - Poker Face.mp3",
		"assets/bgm/right now-akon.mp3",
		"assets/bgm/本兮,阿悄 - 无限速.mp3",
		"assets/bgm/死一样痛过(MC梦版)-MC몽.mp3"
	};
	int bgm_total = sizeof(bgms) / sizeof(bgms[0]);  // bgm 数量
	int current_bgm = 0;

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
		printf("SDL_CreateWindow() failed: %s\n", SDL_GetError());
		if (bgm) Mix_FreeMusic(bgm);
		Mix_CloseAudio();
		IMG_Quit();
		SDL_Quit();  // 释放所有 SDL 分配的资源
		return -4;
	}

	// 创建渲染器
	SDL_Renderer *renderer = SDL_CreateRenderer(
		window,
		-1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
	);
	if (!renderer)
	{
		printf("SDL_CreateRenderer() failed: %s\n", SDL_GetError());
		if (bgm) Mix_FreeMusic(bgm);
		SDL_DestroyWindow(window);
		Mix_CloseAudio();
		IMG_Quit();
		SDL_Quit();
		return -5;
	}

	SDL_Surface *bgSurface = IMG_Load("assets/maps/map.png");
	// 添加背景图片
	if (!bgSurface)
	{
		printf("map.png load failed: %s\n", SDL_GetError());
		if (bgm) Mix_FreeMusic(bgm);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		Mix_CloseAudio();
		IMG_Quit();
		SDL_Quit();
		return -6;
	}

	// 创建纹理, 将地图背景的 Surface 转换为 GPU 可用的纹理 bgTexture
	SDL_Texture *bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
	SDL_FreeSurface(bgSurface);  // 释放 Surface，不再需要
	if (!bgTexture)
	{
		printf("map.png rendering failed: %s\n", SDL_GetError());
		if (bgm) Mix_FreeMusic(bgm);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		Mix_CloseAudio();
		IMG_Quit();
		SDL_Quit();
		return -7;
	}

	// 加载赛车背景图
	SDL_Surface *carSurface = IMG_Load("assets/cars/red_up.png");
	if (!carSurface)
	{
		printf("red_up car load failed: %s\n", IMG_GetError());
		if (bgm) Mix_FreeMusic(bgm);
		SDL_DestroyTexture(bgTexture);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		Mix_CloseAudio();
		IMG_Quit();
		SDL_Quit();
		return -8;
	}
	SDL_Texture *carTexture = SDL_CreateTextureFromSurface(renderer, carSurface);
	SDL_FreeSurface(carSurface);
	if (!carTexture)
	{
		printf("red_up car rendering failed: %s\n", SDL_GetError());
		if (bgm) Mix_FreeMusic(bgm);
		SDL_DestroyTexture(bgTexture);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		Mix_CloseAudio();
		IMG_Quit();
		SDL_Quit();
		return -9;
	}

	// 开始播放背景音乐
	if (bgm)
	{
		// 循环播放背景音乐（-1表示无限循环）
		if (Mix_PlayMusic(bgm, -1) == -1)
		{
			printf("无法播放背景音乐: %s\n", Mix_GetError());
		}
		else
		{
			printf("背景音乐开始播放\n");
			printf("音乐控制: P=暂停/继续, N=下一首, B=上一首\n");
			// 设置音乐音量（0-128）
			Mix_VolumeMusic(128);  // 中等音量
		}
	}

	// 赛车的初始位置
	int carX = WINDOW_W / 2 - CAR_WIDTH / 2;
	int carY = WINDOW_H - CAR_HEIGHT - 20;
	int carSpeed = 5;


	SDL_Event event;  // SDL 事件结构体, event 存储事件信息
	int running = 1;  // 游戏运行标志, 1运行 0退出

	// 为了更流畅的控制，添加持续按键检测
	const Uint8 *keyState = SDL_GetKeyboardState(NULL);

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

				switch (event.key.keysym.sym)
				{
					case SDLK_p:
						if (bgm)
						{
							if (Mix_PausedMusic())
							{
								Mix_ResumeMusic();
								printf("bgm continue play\n");
							}
							else if (Mix_PlayingMusic())
							{
								Mix_PauseMusic();
								printf("bgm pause\n");
							}
						}
						break;

					case SDLK_n:  // 切换下一首 bgm
						if (bgm_total > 0 && bgm)
						{
							// 停止当前音乐
							Mix_HaltMusic();
							Mix_FreeMusic(bgm);

							// 计算下一首索引
							current_bgm = (current_bgm + 1) % bgm_total;

							bgm = Mix_LoadMUS(bgms[current_bgm]);
							if (bgm && (Mix_PlayMusic(bgm, -1) == 0))
								Mix_VolumeMusic(128);
						}
						break;

					case SDLK_b:  //  切换上一首 bgm
						if (bgm_total > 0 && bgm)
						{
							// 停止当前音乐
							Mix_HaltMusic();
							Mix_FreeMusic(bgm);

							// 计算下一首索引
							current_bgm = (current_bgm - 1 + bgm_total) % bgm_total;

							bgm = Mix_LoadMUS(bgms[current_bgm]);
							if (bgm && (Mix_PlayMusic(bgm, -1) == 0))
								Mix_VolumeMusic(128);
						}
						break;


					case SDLK_UP:
					case SDLK_w:
						carY -= carSpeed;
						break;
					case SDLK_DOWN:
					case SDLK_s:
						carY += carSpeed;
						break;
					case SDLK_LEFT:
					case SDLK_a:
						carX -= carSpeed;
						break;
					case SDLK_RIGHT:
					case SDLK_d:
						carX += carSpeed;
						break;
				}

				// 持续按键处理（更流畅）
				if (keyState[SDL_SCANCODE_UP] || keyState[SDL_SCANCODE_W])
					carY -= carSpeed;
				if (keyState[SDL_SCANCODE_DOWN] || keyState[SDL_SCANCODE_S])
					carY += carSpeed;
				if (keyState[SDL_SCANCODE_LEFT] || keyState[SDL_SCANCODE_A])
					carX -= carSpeed;
				if (keyState[SDL_SCANCODE_RIGHT] || keyState[SDL_SCANCODE_D])
					carX += carSpeed;

				// 边界检查
				if (carX < 0) carX = 0;
				if (carX > WINDOW_W - CAR_WIDTH) carX = WINDOW_W - CAR_WIDTH;
				if (carY < 0) carY = 0;
				if (carY > WINDOW_H - CAR_HEIGHT) carY = WINDOW_H - CAR_HEIGHT;
			}
		}

		// 清屏
		SDL_RenderClear(renderer);  

		// 绘制背景图
		SDL_RenderCopy(renderer, bgTexture, NULL, NULL);  

		// 绘制赛车
		SDL_Rect carRect = { carX, carY, CAR_WIDTH, CAR_HEIGHT };  
		SDL_RenderCopy(renderer, carTexture, NULL, &carRect);

		// 更新屏幕
		SDL_RenderPresent(renderer); 

		// 添加延迟，减少 CPU 占用
		// 让程序暂停 10 毫秒
		// 减少 CPU 占用率 (否则 while 循环会全速运行，占用大量 CPU)
		SDL_Delay(10);
	}

	printf("game over!\n");

	// 在程序结束前释放音乐资源
	if (bgm)
	{
		Mix_FreeMusic(bgm);
	}

	SDL_DestroyTexture(carTexture);
	SDL_DestroyTexture(bgTexture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);  // 销毁窗口，释放窗口资源
	IMG_Quit();  // 释放 SDL_image
	SDL_Quit();  // 释放所有 SDL 分配的资源

	return 0;
}