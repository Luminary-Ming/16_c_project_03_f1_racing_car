#include "home.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// 全局变量
static SDL_Window *homeWindow = NULL;  // 主页窗口指针，指向主页的SDL窗口对象，NULL表示未创建
static SDL_Renderer *homeRenderer = NULL;  // 主页渲染器指针，用于在主页窗口上绘制图形和纹理
static SDL_Texture *bgTexture = NULL;  // 背景纹理指针，存储主页背景图片的纹理资源
static SDL_Texture *titleTexture = NULL;  // 标题纹理指针，存储主页标题图片（如"car.png"）的纹理资源
static Button buttons[4];  // 按钮数组，存储主页上的所有按钮对象，大小为4对应4个功能按钮
static int buttonCount = 0;  // 按钮数量计数器，记录当前实际创建的按钮数量（通常为4）
static int selectedButton = 0;  // 选中的按钮ID，0表示未选中，1-4对应BUTTON_START_GAME等按钮常量

// 缩放动画相关
static float titleScale = 0.3f;  // 标题当前缩放比例，初始为30%大小
static float scaleDir = 0.002f;  // 缩放方向增量，正数表示放大，负数表示缩小

// 主页 bgm
static Mix_Music *home_bgm = NULL;  // 主页背景音乐指针
static const char *home_bgm_path = "assets/bgm/本兮,阿悄 - 无限速.mp3";

// 主页 bgm 函数
static void play_home_bgm(void)
{
    // 如果已经有音乐在播放，先停止
    if (home_bgm)
    {
        Mix_FreeMusic(home_bgm);
        home_bgm = NULL;
    }

    // 加载并播放音乐
    home_bgm = Mix_LoadMUS(home_bgm_path);
    if (home_bgm)
    {
        if (Mix_PlayMusic(home_bgm, -1) != -1)  // -1表示循环播放
        {
            Mix_VolumeMusic(80);  // 设置中等音量（0-128）
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

// 绘制带透明度的矩形
static void drawTransparentRect(SDL_Renderer *renderer, SDL_Rect rect, SDL_Color color, int alpha)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

// 绘制单个字符
static void drawChar(SDL_Renderer *renderer, char c, int x, int y, int size, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    switch (c)
    {
        case 'F':
            SDL_RenderDrawLine(renderer, x, y, x, y + 7 * size);
            SDL_RenderDrawLine(renderer, x, y, x + 4 * size, y);
            SDL_RenderDrawLine(renderer, x, y + 3 * size, x + 3 * size, y + 3 * size);
            break;
        case '1':
            SDL_RenderDrawLine(renderer, x + 2 * size, y, x + 2 * size, y + 7 * size);
            SDL_RenderDrawLine(renderer, x + 1 * size, y + 6 * size, x + 3 * size, y + 6 * size);
            SDL_RenderDrawLine(renderer, x, y + 1 * size, x + 1 * size, y);
            break;
        case 'G':
            SDL_RenderDrawLine(renderer, x + 4 * size, y, x, y);
            SDL_RenderDrawLine(renderer, x, y, x, y + 7 * size);
            SDL_RenderDrawLine(renderer, x, y + 7 * size, x + 4 * size, y + 7 * size);
            SDL_RenderDrawLine(renderer, x + 4 * size, y + 7 * size, x + 4 * size, y + 4 * size);
            SDL_RenderDrawLine(renderer, x + 2 * size, y + 4 * size, x + 4 * size, y + 4 * size);
            break;
        case 'A':
        case 'a':
            SDL_RenderDrawLine(renderer, x + 2 * size, y, x, y + 7 * size);
            SDL_RenderDrawLine(renderer, x + 2 * size, y, x + 4 * size, y + 7 * size);
            SDL_RenderDrawLine(renderer, x + 1 * size, y + 4 * size, x + 3 * size, y + 4 * size);
            break;
        case 'M':
        case 'm':
            SDL_RenderDrawLine(renderer, x, y, x, y + 7 * size);
            SDL_RenderDrawLine(renderer, x + 4 * size, y, x + 4 * size, y + 7 * size);
            SDL_RenderDrawLine(renderer, x, y, x + 2 * size, y + 3 * size);
            SDL_RenderDrawLine(renderer, x + 4 * size, y, x + 2 * size, y + 3 * size);
            break;
        case 'E':
        case 'e':
            SDL_RenderDrawLine(renderer, x, y, x, y + 7 * size);
            SDL_RenderDrawLine(renderer, x, y, x + 4 * size, y);
            SDL_RenderDrawLine(renderer, x, y + 3 * size, x + 3 * size, y + 3 * size);
            SDL_RenderDrawLine(renderer, x, y + 6 * size, x + 4 * size, y + 6 * size);
            break;
        case 'R':
        case 'r':
            SDL_RenderDrawLine(renderer, x, y, x, y + 7 * size);
            SDL_RenderDrawLine(renderer, x, y, x + 4 * size, y);
            SDL_RenderDrawLine(renderer, x + 4 * size, y, x + 4 * size, y + 3 * size);
            SDL_RenderDrawLine(renderer, x, y + 3 * size, x + 4 * size, y + 3 * size);
            SDL_RenderDrawLine(renderer, x, y + 3 * size, x + 4 * size, y + 7 * size);
            break;
        case 'S':
        case 's':
            SDL_RenderDrawLine(renderer, x + 4 * size, y, x, y);
            SDL_RenderDrawLine(renderer, x, y, x, y + 3 * size);
            SDL_RenderDrawLine(renderer, x, y + 3 * size, x + 4 * size, y + 3 * size);
            SDL_RenderDrawLine(renderer, x + 4 * size, y + 3 * size, x + 4 * size, y + 7 * size);
            SDL_RenderDrawLine(renderer, x + 4 * size, y + 7 * size, x, y + 7 * size);
            break;
        case 'T':
        case 't':
            SDL_RenderDrawLine(renderer, x, y, x + 4 * size, y);
            SDL_RenderDrawLine(renderer, x + 2 * size, y, x + 2 * size, y + 7 * size);
            break;
        case 'C':
        case 'c':
            SDL_RenderDrawLine(renderer, x + 4 * size, y, x, y);
            SDL_RenderDrawLine(renderer, x, y, x, y + 7 * size);
            SDL_RenderDrawLine(renderer, x, y + 7 * size, x + 4 * size, y + 7 * size);
            break;
        case 'L':
        case 'l':
            SDL_RenderDrawLine(renderer, x, y, x, y + 7 * size);
            SDL_RenderDrawLine(renderer, x, y + 7 * size, x + 4 * size, y + 7 * size);
            break;
        case ' ':
            break;
        default:
            SDL_Rect rect = { x, y, 4 * size, 7 * size };
            SDL_RenderDrawRect(renderer, &rect);
            break;
    }
}

// 绘制艺术字
static void drawArtText(SDL_Renderer *renderer, const char *text, int centerX, int centerY, int size, SDL_Color color, float scale)
{
    int len = strlen(text);
    int charWidth = 6 * size;
    float scaledSize = size * scale;
    int totalWidth = len * charWidth * scale;
    int startX = centerX - totalWidth / 2;
    int startY = centerY - 3 * scaledSize;

    for (int i = 0; i < len; i++)
    {
        drawChar(renderer, text[i], startX + i * (charWidth * scale), startY, scaledSize, color);
    }
}

// 绘制按钮
static void drawButton(SDL_Renderer *renderer, Button *btn)
{
    SDL_Color textColor;
    SDL_Color bgColor;
    SDL_Color borderColor;

    if (btn->hover)
    {
        textColor.r = 255; textColor.g = 0; textColor.b = 0; textColor.a = 255;
        bgColor.r = 255; bgColor.g = 255; bgColor.b = 0; bgColor.a = 200;
        borderColor.r = 255; borderColor.g = 200; borderColor.b = 0; borderColor.a = 255;
    }
    else
    {
        textColor.r = 255; textColor.g = 255; textColor.b = 255; textColor.a = 255;
        bgColor.r = 30; bgColor.g = 30; bgColor.b = 30; bgColor.a = 180;
        borderColor.r = 200; borderColor.g = 200; borderColor.b = 200; borderColor.a = 200;
    }

    SDL_Rect bgRect = {
        btn->x - btn->width / 2,
        btn->y - btn->height / 2,
        btn->width,
        btn->height
    };

    drawTransparentRect(renderer, bgRect, bgColor, bgColor.a);
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderDrawRect(renderer, &bgRect);
    drawArtText(renderer, btn->text, btn->x, btn->y, 4, textColor, 1.0f);
}

// 初始化按钮
static void initButtons(void)
{
    buttonCount = 0;

    int startY = HOME_WINDOW_H / 2 - 30;
    int buttonSpacing = 70;

    buttons[buttonCount++] = (Button){ HOME_WINDOW_W / 2, startY, "START GAME", 220, 50, 0 };
    buttons[buttonCount++] = (Button){ HOME_WINDOW_W / 2, startY + buttonSpacing, "CAR SELECT", 220, 50, 0 };
    buttons[buttonCount++] = (Button){ HOME_WINDOW_W / 2, startY + buttonSpacing * 2, "TRACK SELECT", 220, 50, 0 };
    buttons[buttonCount++] = (Button){ HOME_WINDOW_W / 2, startY + buttonSpacing * 3, "EXIT", 220, 50, 0 };
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
    else if (titleScale <= 0.28f)
    {
        titleScale = 0.28f;
        scaleDir = 0.002f;
    }
}

// 绘制黑金标题
static void drawGoldBlackTitle(SDL_Renderer *renderer)
{
    if (!titleTexture) return;

    int texW, texH;
    SDL_QueryTexture(titleTexture, NULL, NULL, &texW, &texH);

    float scaledW = texW * titleScale;
    float scaledH = texH * titleScale;

    int posX = HOME_WINDOW_W / 2 - scaledW / 2;
    int posY = HOME_WINDOW_H / 4 - scaledH / 2;

    SDL_Rect titleRect = { posX, posY, scaledW, scaledH };

    SDL_SetTextureColorMod(titleTexture, 20, 20, 20);
    SDL_SetTextureAlphaMod(titleTexture, 255);
    SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);

    SDL_SetTextureColorMod(titleTexture, 255, 218, 0);
    SDL_SetTextureAlphaMod(titleTexture, 230);
    SDL_Rect goldRect = { posX + 1, posY - 1, scaledW, scaledH };
    SDL_RenderCopy(renderer, titleTexture, NULL, &goldRect);

    SDL_SetTextureColorMod(titleTexture, 255, 255, 255);
    SDL_SetTextureAlphaMod(titleTexture, 255);
}

// 绘制初始化页面
static void drawHomePage(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (bgTexture)
    {
        SDL_RenderCopy(renderer, bgTexture, NULL, NULL);
    }

    SDL_Color maskColor = { 0, 0, 0, 80 };
    for (int i = 0; i < buttonCount; i++)
    {
        Button *btn = &buttons[i];
        SDL_Rect buttonMask = {
            btn->x - btn->width / 2 - 10,
            btn->y - btn->height / 2 - 5,
            btn->width + 20,
            btn->height + 10
        };
        drawTransparentRect(renderer, buttonMask, maskColor, 80);
    }

    updateTitleAnimation();
    drawGoldBlackTitle(renderer);

    for (int i = 0; i < buttonCount; i++)
    {
        drawButton(renderer, &buttons[i]);
    }
}

// 检查鼠标悬停
static void checkButtonHover(int mouseX, int mouseY)
{
    for (int i = 0; i < buttonCount; i++)
    {
        Button *btn = &buttons[i];
        int left = btn->x - btn->width / 2;
        int right = btn->x + btn->width / 2;
        int top = btn->y - btn->height / 2;
        int bottom = btn->y + btn->height / 2;

        btn->hover = (mouseX >= left && mouseX <= right &&
                      mouseY >= top && mouseY <= bottom) ? 1 : 0;
    }
}

// 处理按钮点击
static int handleButtonClick(int mouseX, int mouseY)
{
    for (int i = 0; i < buttonCount; i++)
    {
        Button *btn = &buttons[i];
        int left = btn->x - btn->width / 2;
        int right = btn->x + btn->width / 2;
        int top = btn->y - btn->height / 2;
        int bottom = btn->y + btn->height / 2;

        if (mouseX >= left && mouseX <= right &&
            mouseY >= top && mouseY <= bottom)
        {
            printf("点击按钮: %s\n", btn->text);
            return i + 1; // 返回按钮ID（1-4）
        }
    }
    return 0;
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
        // 继续运行，没有音乐也可以
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
        "F1 Racing Game - Home",
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
    printf("加载背景图片: %s\n", bgPath);

    SDL_Surface *bgSurface = IMG_Load(bgPath);
    if (bgSurface)
    {
        bgTexture = SDL_CreateTextureFromSurface(homeRenderer, bgSurface);
        SDL_FreeSurface(bgSurface);
    }

    // 加载标题图片
    const char *titlePath = "assets/home/car.png";
    printf("加载标题图片: %s\n", titlePath);

    SDL_Surface *titleSurface = IMG_Load(titlePath);
    if (titleSurface)
    {
        titleTexture = SDL_CreateTextureFromSurface(homeRenderer, titleSurface);
        SDL_FreeSurface(titleSurface);
    }

    // 初始化按钮
    initButtons();

    // 播放主页背景音乐（只播放不控制）
    play_home_bgm();

    printf("\n主页初始化完成！\n");
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
                        selectedButton = handleButtonClick(mouseX, mouseY);
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
        Mix_HaltMusic();  // 停止音乐播放
        Mix_FreeMusic(home_bgm);
        home_bgm = NULL;
    }

    if (Mix_GetNumMusicDecoders() > 0)  // 检查是否初始化了音频
        Mix_CloseAudio();


    if (bgTexture) SDL_DestroyTexture(bgTexture);
    if (titleTexture) SDL_DestroyTexture(titleTexture);
    if (homeRenderer) SDL_DestroyRenderer(homeRenderer);
    if (homeWindow) SDL_DestroyWindow(homeWindow);
    IMG_Quit();
    SDL_Quit();
}