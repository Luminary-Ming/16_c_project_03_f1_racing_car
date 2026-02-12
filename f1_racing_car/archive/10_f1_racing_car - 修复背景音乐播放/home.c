#include "home.h"
#include <stdio.h>
#include <stdlib.h>

// 全局变量
static SDL_Window *homeWindow = NULL;
static SDL_Renderer *homeRenderer = NULL;
static SDL_Texture *bgTexture = NULL, *titleTexture = NULL;
static Button startButton;  // 单个开始按钮
static int selectedButton = 0;

// 缩放动画相关
static float titleScale = 0.3f, scaleDir = 0.002f;

// 主页 bgm
static Mix_Music *home_bgm = NULL;
static const char *home_bgm_path = "assets/bgm/本兮,阿悄 - 无限速.mp3";

// 主页 bgm 函数
static void play_home_bgm(void)
{
    if (home_bgm)
    {
        Mix_FreeMusic(home_bgm);
        home_bgm = NULL;
    }

    home_bgm = Mix_LoadMUS(home_bgm_path);
    if (home_bgm)
    {
        if (Mix_PlayMusic(home_bgm, -1) != -1)
        {
            Mix_VolumeMusic(80);
            printf("主页背景音乐开始播放\n");
        }
        else
        {
            printf("播放音乐失败: %s\n", Mix_GetError());
        }
    }
    else
    {
        printf("加载音乐失败: %s\n", Mix_GetError());
    }
}

// 初始化按钮（使用图片纹理）
static int initButton(SDL_Renderer *r)
{
    // 按钮图片文件路径
    const char *path = "assets/home/start_game.png";  // 改为相对路径

    SDL_Surface *s = IMG_Load(path);

    if (!s)
    {
        // 如果加载失败，创建默认按钮
        s = SDL_CreateRGBSurface(0, 200, 40, 32, 0, 0, 0, 0);
        SDL_FillRect(s, NULL, SDL_MapRGB(s->format, 30, 30, 30));
        startButton.w = 200;
        startButton.h = 40;
    }
    else
    {
        // 计算缩放比例
        float scale = ((float)HOME_WINDOW_W / s->w) * 0.3f;
        if (scale < 0.1f) scale = 0.1f;
        if (scale > 0.2f) scale = 0.2f;
        startButton.w = s->w * scale;
        startButton.h = s->h * scale;
    }

    startButton.tex = SDL_CreateTextureFromSurface(r, s);
    SDL_FreeSurface(s);

    // 设置按钮位置
    startButton.x = HOME_WINDOW_W / 2;
    startButton.y = HOME_WINDOW_H * 3 / 4;
    startButton.hover = 0;
    startButton.clicked = 0;

    return startButton.tex != NULL;
}

// 更新标题动画
static void updateTitleAnimation(void)
{
    titleScale += scaleDir;
    if (titleScale >= 0.32f)
    {
        titleScale = 0.32f;
        scaleDir = -0.002f;
    }
    if (titleScale <= 0.28f)
    {
        titleScale = 0.28f;
        scaleDir = 0.002f;
    }
}

// 绘制黑金标题
static void drawGoldBlackTitle(SDL_Renderer *r)
{
    if (!titleTexture) return;

    int texW, texH;
    SDL_QueryTexture(titleTexture, NULL, NULL, &texW, &texH);

    int w = texW * titleScale, h = texH * titleScale;
    int x = HOME_WINDOW_W / 2 - w / 2, y = HOME_WINDOW_H / 5 - h / 2;

    SDL_Rect rect = { x, y, w, h };

    // 黑色底层
    SDL_SetTextureColorMod(titleTexture, 20, 20, 20);
    SDL_RenderCopy(r, titleTexture, NULL, &rect);

    // 金色上层（立体效果）
    SDL_SetTextureColorMod(titleTexture, 255, 218, 0);
    rect.x++; rect.y--;
    SDL_RenderCopy(r, titleTexture, NULL, &rect);

    // 恢复纹理设置
    SDL_SetTextureColorMod(titleTexture, 255, 255, 255);
}

// 绘制按钮（带光晕和波纹效果）
static void drawButton(SDL_Renderer *r)
{
    if (!startButton.tex) return;

    SDL_Rect rect = {
        startButton.x - startButton.w / 2,
        startButton.y - startButton.h / 2,
        startButton.w,
        startButton.h
    };

    if (startButton.hover)
    {
        // 悬停状态
        SDL_SetTextureColorMod(startButton.tex, 255, 255, 255);
        SDL_Rect hoverRect = { rect.x - 2, rect.y - 2, rect.w + 4, rect.h + 4 };
        SDL_RenderCopy(r, startButton.tex, NULL, &hoverRect);

        // 光晕效果
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_ADD);
        SDL_SetRenderDrawColor(r, 255, 255, 150, 20);
        for (int i = 0; i < 2; i++)
        {
            SDL_Rect glowRect = { hoverRect.x - i * 3, hoverRect.y - i * 3,
                               hoverRect.w + i * 6, hoverRect.h + i * 6 };
            SDL_RenderDrawRect(r, &glowRect);
        }
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    }
    else
    {
        // 正常状态
        SDL_SetTextureColorMod(startButton.tex, 220, 220, 220);
        SDL_RenderCopy(r, startButton.tex, NULL, &rect);
    }

    if (startButton.clicked)
    {
        // 点击状态
        SDL_SetTextureColorMod(startButton.tex, 180, 180, 180);
        SDL_RenderCopy(r, startButton.tex, NULL, &rect);

        // 波纹效果
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_ADD);
        SDL_SetRenderDrawColor(r, 255, 200, 50, 80);
        for (int i = 0; i < 3; i++)
        {
            SDL_Rect rippleRect = { rect.x - i * 3, rect.y - i * 3,
                                 rect.w + i * 6, rect.h + i * 6 };
            SDL_RenderDrawRect(r, &rippleRect);
        }
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    }

    SDL_SetTextureColorMod(startButton.tex, 255, 255, 255);
}

// 绘制主页
static void drawHomePage(SDL_Renderer *r)
{
    // 清屏
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);

    // 绘制背景
    if (bgTexture)
    {
        SDL_RenderCopy(r, bgTexture, NULL, NULL);
    }

    // 更新并绘制标题
    updateTitleAnimation();
    drawGoldBlackTitle(r);

    // 绘制按钮
    drawButton(r);
}

// 检查鼠标悬停
static void checkButtonHover(int mouseX, int mouseY)
{
    if (!startButton.tex) return;

    int left = startButton.x - startButton.w / 2;
    int right = startButton.x + startButton.w / 2;
    int top = startButton.y - startButton.h / 2;
    int bottom = startButton.y + startButton.h / 2;

    startButton.hover = (mouseX >= left && mouseX <= right &&
                         mouseY >= top && mouseY <= bottom) ? 1 : 0;
}

// 处理按钮点击
static int handleButtonClick(int mouseX, int mouseY)
{
    if (!startButton.tex) return 0;

    int left = startButton.x - startButton.w / 2;
    int right = startButton.x + startButton.w / 2;
    int top = startButton.y - startButton.h / 2;
    int bottom = startButton.y + startButton.h / 2;

    if (mouseX >= left && mouseX <= right &&
        mouseY >= top && mouseY <= bottom)
    {
        printf("点击开始游戏按钮\n");
        startButton.clicked = 1;
        return BUTTON_START_GAME;
    }
    return 0;
}

// 显示点击动画
static void showClickAnimation(void)
{
    for (int i = 0; i < 15; i++)
    {
        drawHomePage(homeRenderer);
        SDL_RenderPresent(homeRenderer);
        SDL_Delay(10);
    }
    startButton.clicked = 0;
}

// 初始化主页
void init_home_page(void)
{
    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL_Init() failed: %s\n", SDL_GetError());
        return;
    }

    // 初始化SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("Mix_OpenAudio() failed: %s\n", Mix_GetError());
    }

    // 初始化SDL_image
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        printf("IMG_Init() failed: %s\n", IMG_GetError());
        return;
    }

    // 创建窗口
    homeWindow = SDL_CreateWindow(
        "F1 Racing Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        HOME_WINDOW_W,
        HOME_WINDOW_H,
        SDL_WINDOW_SHOWN
    );

    if (!homeWindow)
    {
        printf("SDL_CreateWindow() failed: %s\n", SDL_GetError());
        return;
    }

    // 创建渲染器
    homeRenderer = SDL_CreateRenderer(
        homeWindow,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!homeRenderer)
    {
        printf("SDL_CreateRenderer() failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(homeWindow);
        return;
    }

    // 启用混合模式
    SDL_SetRenderDrawBlendMode(homeRenderer, SDL_BLENDMODE_BLEND);

    // 加载背景图片
    const char *bgPath = "assets/home/home.png";

    SDL_Surface *bgSurface = IMG_Load(bgPath);
    if (bgSurface)
    {
        bgTexture = SDL_CreateTextureFromSurface(homeRenderer, bgSurface);
        SDL_FreeSurface(bgSurface);
    }
    else
    {
        printf("加载背景图片失败: %s\n", IMG_GetError());
    }

    // 加载标题图片
    const char *titlePath = "assets/home/car.png";

    SDL_Surface *titleSurface = IMG_Load(titlePath);
    if (titleSurface)
    {
        titleTexture = SDL_CreateTextureFromSurface(homeRenderer, titleSurface);
        SDL_FreeSurface(titleSurface);
    }
    else
    {
        printf("加载标题图片失败: %s\n", IMG_GetError());
    }

    // 初始化按钮
    initButton(homeRenderer);

    // 播放主页背景音乐
    play_home_bgm();
}

// 运行主页
int run_home_page(void)
{
    SDL_Event event;
    int mouseX, mouseY;
    selectedButton = 0;

    while (selectedButton == 0)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    selectedButton = BUTTON_EXIT;
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        selectedButton = BUTTON_EXIT;
                    }
                    break;

                case SDL_MOUSEMOTION:
                    mouseX = event.motion.x;
                    mouseY = event.motion.y;
                    checkButtonHover(mouseX, mouseY);
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT)
                    {
                        mouseX = event.button.x;
                        mouseY = event.button.y;
                        int clicked = handleButtonClick(mouseX, mouseY);
                        if (clicked == BUTTON_START_GAME)
                        {
                            showClickAnimation();
                            selectedButton = BUTTON_START_GAME;
                        }
                    }
                    break;
            }
        }

        drawHomePage(homeRenderer);
        SDL_RenderPresent(homeRenderer);
        SDL_Delay(16);
    }

    return selectedButton;
}

// 清理主页资源
void cleanup_home_page(void)
{
    // 停止并释放音乐资源
    if (home_bgm)
    {
        Mix_HaltMusic();
        Mix_FreeMusic(home_bgm);
        home_bgm = NULL;
    }

    if (Mix_GetNumMusicDecoders() > 0)
        Mix_CloseAudio();

    // 释放纹理
    if (startButton.tex) SDL_DestroyTexture(startButton.tex);
    if (bgTexture) SDL_DestroyTexture(bgTexture);
    if (titleTexture) SDL_DestroyTexture(titleTexture);

    // 释放渲染器和窗口
    if (homeRenderer) SDL_DestroyRenderer(homeRenderer);
    if (homeWindow) SDL_DestroyWindow(homeWindow);

    // 退出SDL
    IMG_Quit();
    SDL_Quit();
}