/*
	F1 Racing Car - F1 赛车
*/
#include <stdio.h>
#include <SDL2/SDL.h>        // sudo apt install libsdl2-dev -y
#include <SDL2/SDL_image.h>  // sudo apt install libsdl2-image-dev -y
#include <SDL2/SDL_mixer.h>  // sudo apt install libsdl2-mixer-dev -y
#include <math.h>

#define WINDOW_W 500  // 设置窗口宽度
#define WINDOW_H 800  // 设置窗口高度

#define CAR_WIDTH 70    // 赛车宽
#define CAR_HEIGHT 130  // 赛车高

#define MAPS_COUNT 5  // 地图数量
#define CARS_COUNT 7  // 赛车数量

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
		Mix_CloseAudio();
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

	// 地图
	const char *maps[] = {
		"assets/maps/map1.png",
		"assets/maps/map2.png",
		"assets/maps/map3.png",
		"assets/maps/map4.png",
		"assets/maps/map5.png",
	};
	int map_total = sizeof(maps) / sizeof(maps[0]);
	int current_map = 0;
	SDL_Texture *map_textures[MAPS_COUNT] = { NULL };  // 初始化所有地图纹理
	int map_heights[MAPS_COUNT] = { 0 };  // 初始化所有地图的高度


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

	// 加载地图纹理
	for (int i = 0; i < map_total; i++)
	{
		SDL_Surface *map_surface = IMG_Load(maps[i]);
		if (map_surface)
		{
			map_textures[i] = SDL_CreateTextureFromSurface(renderer, map_surface);
			if (map_textures[i])
			{
				int tex_width, tex_height;
				SDL_QueryTexture(map_textures[i], NULL, NULL, &tex_width, &tex_height);
				map_heights[i] = tex_height;
				printf("加载地图 %d: %s (%d x %d)\n", i + 1, maps[i], tex_width, tex_height);
			}
			SDL_FreeSurface(map_surface);
		}
	}

	// 设置当前地图纹理
	SDL_Texture *current_map_texture = map_textures[current_map];
	int current_map_height = map_heights[current_map];

	// 赛车皮肤
	const char *cars[] = {
		"assets/cars/red_up.png",
		"assets/cars/black_up.png",
		"assets/cars/bumblebee_up.png",
		"assets/cars/taxi_up.png",
		"assets/cars/tractor_up.png",
		"assets/cars/white_up.png"
	};
	int car_total = sizeof(cars) / sizeof(cars[0]);
	int current_car = 0;
	SDL_Texture *car_textures[CARS_COUNT] = { NULL };


	// 加载汽车皮肤纹理
	for (int i = 0; i < car_total; i++)
	{
		SDL_Surface *skin_surface = IMG_Load(cars[i]);
		if (skin_surface)
		{
			car_textures[i] = SDL_CreateTextureFromSurface(renderer, skin_surface);
			if (car_textures[i])
			{
				printf("加载汽车皮肤 %d: %s\n", i + 1, cars[i]);
			}
			SDL_FreeSurface(skin_surface);
		}
	}

	// 设置当前汽车皮肤
	SDL_Texture *current_car_texture = car_textures[current_car];

	// 播放背景音乐
	if (bgm && Mix_PlayMusic(bgm, -1) != -1)
	{
		printf("背景音乐开始播放\n");
		printf("音乐控制: P=暂停/继续, N=下一首, B=上一首, M=切换地图\n");
		Mix_VolumeMusic(128);
	}

	// 赛车的初始位置
	int car_x = WINDOW_W / 2;
	int car_y = WINDOW_H - CAR_HEIGHT - 20;
	int car_speed = 10;

	// 双缓冲渲染变量
	float scroll_position = 0.0f;      // 使用浮点数实现平滑滚动
	int scroll_speed = car_speed;

	// 双缓冲纹理：用于绘制完整的地图循环
	SDL_Texture *buffer_texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET,
		WINDOW_W,
		WINDOW_H
	);

	if (!buffer_texture)
	{
		printf("双缓冲纹理创建失败\n");
		// 继续运行，使用单缓冲
	}

	SDL_Event event;  // SDL 事件结构体, event 存储事件信息
	int running = 1;  // 游戏运行标志, 1运行 0退出

	// 持续按键检测
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
				//printf("按键 : %s\n", SDL_GetKeyName(event.key.keysym.sym));

				switch (event.key.keysym.sym)
				{
					case SDLK_p:
						if (bgm)
						{
							if (Mix_PausedMusic())
								Mix_ResumeMusic();
							else if (Mix_PlayingMusic())
								Mix_PauseMusic();
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

					case SDLK_m:  // M键：切换下一张地图
						if (map_total > 0)
						{
							current_map = (current_map + 1) % map_total;
							scroll_position = 0;
							current_map_texture = map_textures[current_map];
							current_map_height = map_heights[current_map];
						}
						break;

					case SDLK_c:  // C键：切换赛车皮肤
						if (car_total > 0)
						{
							current_car = (current_car + 1) % car_total;
							current_car_texture = car_textures[current_car];
						}
						break;
				
				}
			}
		}

		// 处理持续按键
		if (keyState[SDL_SCANCODE_UP] || keyState[SDL_SCANCODE_W])
			scroll_position -= scroll_speed;

		if (keyState[SDL_SCANCODE_DOWN] || keyState[SDL_SCANCODE_S])
			scroll_position += scroll_speed;

		if (keyState[SDL_SCANCODE_LEFT] || keyState[SDL_SCANCODE_A])
			car_x -= car_speed;

		if (keyState[SDL_SCANCODE_RIGHT] || keyState[SDL_SCANCODE_D])
			car_x += car_speed;

		// 赛车边界检查
		if (car_x < WINDOW_W / 2 - CAR_WIDTH - 30) car_x = WINDOW_W / 2 - CAR_WIDTH - 30;
		if (car_x > WINDOW_W / 2 + CAR_WIDTH - 30) car_x = WINDOW_W / 2 + CAR_WIDTH - 30;

		// 清屏
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		// ====== 双缓冲渲染：绘制完整的地图循环 ======
		if (current_map_texture && current_map_height > 0)
		{
			// 方法1：使用双缓冲纹理（推荐，最流畅）
			if (buffer_texture)
			{
				// 切换到缓冲纹理进行绘制
				SDL_SetRenderTarget(renderer, buffer_texture);
				SDL_RenderClear(renderer);

				// 计算实际滚动位置（循环）
				float actual_scroll = fmod(scroll_position, current_map_height);
				if (actual_scroll < 0) actual_scroll += current_map_height;

				// 绘制地图第一部分
				int src_y1 = (int)actual_scroll;
				int dest_y1 = 0;
				int height1 = WINDOW_H;

				// 如果地图高度不够，需要绘制第二部分
				if (src_y1 + WINDOW_H > current_map_height)
				{
					height1 = current_map_height - src_y1;

					// 绘制第二部分（从地图顶部开始）
					SDL_Rect src_rect2 = { 0, 0, WINDOW_W, WINDOW_H - height1 };
					SDL_Rect dest_rect2 = { 0, height1, WINDOW_W, WINDOW_H - height1 };
					SDL_RenderCopy(renderer, current_map_texture, &src_rect2, &dest_rect2);
				}

				// 绘制第一部分
				SDL_Rect src_rect1 = { 0, src_y1, WINDOW_W, height1 };
				SDL_Rect dest_rect1 = { 0, dest_y1, WINDOW_W, height1 };
				SDL_RenderCopy(renderer, current_map_texture, &src_rect1, &dest_rect1);

				// 切换回默认渲染目标
				SDL_SetRenderTarget(renderer, NULL);

				// 将缓冲纹理绘制到屏幕
				SDL_RenderCopy(renderer, buffer_texture, NULL, NULL);
			}
			else
			{
				// 方法2：备用方法（无缓冲纹理）
				float actual_scroll = fmod(scroll_position, current_map_height);
				if (actual_scroll < 0) actual_scroll += current_map_height;

				SDL_Rect src_rect = { 0, (int)actual_scroll, WINDOW_W, WINDOW_H };
				SDL_Rect dest_rect = { 0, 0, WINDOW_W, WINDOW_H };
				SDL_RenderCopy(renderer, current_map_texture, &src_rect, &dest_rect);

				// 如果需要，绘制循环部分
				if ((int)actual_scroll + WINDOW_H > current_map_height)
				{
					int remaining = current_map_height - (int)actual_scroll;
					int part2_height = WINDOW_H - remaining;

					SDL_Rect src_rect2 = { 0, 0, WINDOW_W, part2_height };
					SDL_Rect dest_rect2 = { 0, remaining, WINDOW_W, part2_height };


					SDL_RenderCopy(renderer, current_map_texture, &src_rect2, &dest_rect2);
				}
			}
		}

		// 绘制赛车
		SDL_Rect carRect = { car_x, car_y, CAR_WIDTH, CAR_HEIGHT };
		SDL_RenderCopy(renderer, current_car_texture, NULL, &carRect);

		// 更新屏幕
		SDL_RenderPresent(renderer); 

		// 添加延迟，减少 CPU 占用
		// 让程序暂停 10 毫秒
		// 减少 CPU 占用率 (否则 while 循环会全速运行，占用大量 CPU)
		SDL_Delay(16);  // 约60FPS
	}

	printf("game over!\n");

	if (buffer_texture)
		SDL_DestroyTexture(buffer_texture);

	for (int i = 0; i < map_total; i++)
		if (map_textures[i]) 
			SDL_DestroyTexture(map_textures[i]);

	for (int i = 0; i < car_total; i++)
		if (car_textures[i])
			SDL_DestroyTexture(car_textures[i]);

	if (bgm) Mix_FreeMusic(bgm);
	if (renderer) SDL_DestroyRenderer(renderer);
	if (window) SDL_DestroyWindow(window);
	
	Mix_CloseAudio();
	IMG_Quit();  // 释放 SDL_image
	SDL_Quit();  // 释放所有 SDL 分配的资源

	return 0;
}