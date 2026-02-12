#include "game.h"
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL_ttf.h>  // sudo apt install libsdl2-ttf-dev -y

// 游戏配置
#define WINDOW_W 500  // 窗口宽
#define WINDOW_H 800  // 窗口高
#define CAR_WIDTH 70  // 赛车宽
#define CAR_HEIGHT 130  // 赛车高
#define MAPS_COUNT 5  // 地图数量
#define CARS_COUNT 9  // 赛车皮肤数量
#define ENEMY_CARS_COUNT 9  // 敌方赛车皮肤数量
#define MAX_ENEMIES 5  // 敌方赛车最大数量, 最多同时存在MAX_ENEMIES辆
#define LIVES 5  // 生命值
#define BASE_CAR_SPEED 5  // 初始速度

// 游戏状态相关
static GameState gameState = GAME_RUNNING;  // 游戏状态
static SDL_Texture *pause_texture = NULL;   // 暂停界面纹理

// 游戏音乐库
static const char *bgms[] = {
    "assets/bgm/Insomnia - Craig David.mp3",
    "assets/bgm/Lady Gaga - Poker Face.mp3",
    "assets/bgm/right now-akon.mp3",
    "assets/bgm/本兮,阿悄 - 无限速.mp3",
    "assets/bgm/死一样痛过(MC梦版)-MC몽.mp3"
};
static int bgm_total = sizeof(bgms) / sizeof(bgms[0]);  // bgm 数量

// 过场动画状态
typedef enum
{
    TRANSITION_NONE,  // 没有过场动画，正常游戏状态
    TRANSITION_FADE_OUT,  // 淡出状态：屏幕逐渐变白/显示过场图片
    TRANSITION_FADE_IN  // 淡入状态：逐渐恢复游戏画面
} TransitionState;

// 过场动画结构体
typedef struct
{
    TransitionState state;  // 当前过场动画状态
    float alpha;  // 透明度 0.0-1.0
    float speed;  // 过渡速度
    int duration;  // 持续时间（帧）
    int timer;  // 计时器
    SDL_Texture *transition_texture;  // 过场纹理
} Transition;

// 敌方车辆结构体
typedef struct
{
    int x;   // x坐标
    float y;  // y坐标（使用浮点数实现平滑移动）
    int width;  // 宽度
    int height;  // 高度
    float speed; // 速度（像素/帧）
    int active;  // 是否运动（1=活动状态，0=非活动状态）
    int type;  // 车辆类型
    SDL_Texture *texture;  // 纹理指针，指向车辆图片资源
} EnemyCar;

// 全局变量
static SDL_Window *gameWindow = NULL;  // 游戏窗口指针
static SDL_Renderer *gameRenderer = NULL;  // 游戏渲染器指针
static int gameRunning = 1;  // 游戏运行标志（1=运行中，0=结束）
static float enemy_scroll_factor = 0.0f;  // 敌方车辆滚动因子，用于调整敌人速度随地图滚动变化
static TTF_Font *speed_font = NULL;  // 字体指针，用于显示速度倍率文字

// 游戏资源
static Mix_Music *bgm = NULL;  // 背景音乐指针
static SDL_Texture *map_textures[MAPS_COUNT] = { NULL };  // 地图纹理数组，存储所有地图图片
static SDL_Texture *car_textures[CARS_COUNT] = { NULL };  // 玩家车辆纹理数组
static SDL_Texture *enemy_textures[ENEMY_CARS_COUNT] = { NULL };  // 敌方车辆纹理数组
static SDL_Texture *current_map_texture = NULL;  // 当前使用的地图纹理
static SDL_Texture *current_car_texture = NULL;  // 当前使用的玩家车辆纹理
static SDL_Texture *game_over_texture = NULL;  // 游戏结束画面纹理
static SDL_Texture *explosion_texture = NULL;  // 爆炸特效纹理
static SDL_Texture *number_textures[10] = { NULL };  // 数字纹理数组（0-9），用于显示分数
static SDL_Texture *buffer_texture = NULL;  // 双缓冲纹理，用于平滑渲染

// 敌方车辆数组
static EnemyCar enemies[MAX_ENEMIES];

// 游戏状态
static int car_x;  // 玩家车辆x坐标
static int car_y;  // 玩家车辆y坐标（固定位置）
static int current_car_speed;  // 当前玩家车辆速度（像素/帧）
static float scroll_position;  // 地图滚动位置，控制地图显示的偏移量
static int scroll_speed;  // 地图滚动速度
static Transition map_transition;  // 地图切换的过场动画对象
static int pending_map_change = -1;  // 待切换的地图索引（-1表示没有待切换的地图） 

static int score = 0;  // 当前游戏分数
static int lives = LIVES;  // 当前生命值
static int game_over = 0;  // 游戏结束标志（1=游戏结束，0=游戏进行中）
static int enemy_spawn_timer = 0;  // 敌方车辆生成计时器（帧数）
static int enemy_spawn_delay = 60; // 敌方车辆生成间隔（帧数）

// 爆炸特效相关
static float explosion_alpha = 0.0f;  // 爆炸特效透明度
static int explosion_duration = 0;  // 爆炸特效持续时间（帧数）
static int explosion_x = 0;  // 爆炸特效x坐标
static int explosion_y = 0;  // 爆炸特效y坐标

// 当前选择
static int current_map = 0;  // 当前选择的地图索引（0-MAPS_COUNT-1）
static int current_car = 0;  // 当前选择的玩家车辆索引（0-CARS_COUNT-1）
static int current_bgm = 0;  // 当前播放的背景音乐索引

// 函数声明
static void init_game(void);  // 初始化游戏
static void cleanup_game(void);  // 清理游戏资源
static SDL_Texture *load_transition_texture(const char *path);  // 加载过场动画纹理
static void init_transition(void);  // 初始化过场动画
static void start_transition(void);  // 开始过场动画
static int update_transition(void);  // 更新过场动画状态
static void render_transition(void);  // 渲染过场动画
static float get_speed_multiplier(int score);  // 根据分数计算速度倍率
static void handle_events(void);  // 处理输入事件
static void update_game(void);  // 更新游戏逻辑
static void render_game(void);  // 渲染游戏画面
static void load_game_resources(void);  // 加载游戏资源
static void cleanup_game_resources(void);  // 清理游戏资源
static void spawn_enemy(void);   // 生成新的敌方车辆
static void update_enemies(void);   // 更新所有敌方车辆状态
static void check_collisions(void);  // 检测碰撞
static void render_ui(void);   // 渲染用户界面（分数、血条等）


// 渲染暂停界面
static void render_pause_menu(void)
{
    if (pause_texture)
    {
        // 绘制半透明黑色背景
        SDL_RenderCopy(gameRenderer, pause_texture, NULL, NULL);
    }
    else
    {
        // 备用：绘制半透明黑色矩形
        SDL_SetRenderDrawBlendMode(gameRenderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(gameRenderer, 0, 0, 0, 180);
        SDL_Rect bg_rect = { 0, 0, WINDOW_W, WINDOW_H };
        SDL_RenderFillRect(gameRenderer, &bg_rect);
        SDL_SetRenderDrawBlendMode(gameRenderer, SDL_BLENDMODE_NONE);
    }
}

// 加载过场图片
static SDL_Texture *load_transition_texture(const char *path)
{
    SDL_Surface *surface = IMG_Load(path);
    if (!surface)
    {
        printf("无法加载过场图片: %s\n", IMG_GetError());
        return NULL;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(gameRenderer, surface);
    SDL_FreeSurface(surface);

    if (!texture)
    {
        printf("无法创建过场纹理: %s\n", SDL_GetError());
    }

    return texture;
}

// 初始化过场动画
static void init_transition(void)
{
    map_transition.state = TRANSITION_NONE;
    map_transition.alpha = 0.0f;
    map_transition.speed = 0.02f;  // 减慢过渡速度，让动画更平滑
    map_transition.duration = 500;  // 增加持续时间到80帧（约1.3秒）
    map_transition.timer = 0;

    // 加载过场图片
    const char *transitions[] = {
        "assets/maps/map1_page.png",
        "assets/maps/map2_page.png",
        "assets/maps/map3_page.png",
        "assets/maps/map4_page.png",
        "assets/maps/map5_page.png",
    };

    const char *transition_path = transitions[current_map];
    map_transition.transition_texture = load_transition_texture(transition_path);

    // 如果加载失败，创建一个纯色纹理作为备用
    if (!map_transition.transition_texture)
    {
        SDL_Surface *surface = SDL_CreateRGBSurface(0, WINDOW_W, WINDOW_H, 32,
                                                    0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
        if (surface)
        {
            SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 255, 255, 255, 255));
            map_transition.transition_texture = SDL_CreateTextureFromSurface(gameRenderer, surface);
            SDL_FreeSurface(surface);
        }
    }
}

// 开始过场动画
static void start_transition(void)
{
    map_transition.state = TRANSITION_FADE_OUT;
    map_transition.alpha = 0.0f;
    map_transition.timer = 0;
}

// 更新过场动画
static int update_transition(void)
{
    if (map_transition.state == TRANSITION_NONE)
    {
        return 0;
    }

    map_transition.timer++;

    switch (map_transition.state)
    {
        case TRANSITION_FADE_OUT:
            map_transition.alpha += map_transition.speed;
            if (map_transition.alpha >= 1.0f)
            {
                map_transition.alpha = 1.0f;
                map_transition.state = TRANSITION_FADE_IN;
                map_transition.timer = 0;
            }
            break;

        case TRANSITION_FADE_IN:
            map_transition.alpha -= map_transition.speed;
            if (map_transition.alpha <= 0.0f)
            {
                map_transition.alpha = 0.0f;
                map_transition.state = TRANSITION_NONE;
                map_transition.timer = 0;
                return 1; // 动画结束
            }
            break;

        default:
            break;
    }

    return 0;
}

// 渲染过场动画
static void render_transition(void)
{
    if (map_transition.state == TRANSITION_NONE || map_transition.alpha <= 0.0f)
    {
        return;
    }

    if (map_transition.transition_texture)
    {
        SDL_SetTextureAlphaMod(map_transition.transition_texture, (Uint8)(map_transition.alpha * 255));
        SDL_Rect dest = { 0, 0, WINDOW_W, WINDOW_H };
        SDL_RenderCopy(gameRenderer, map_transition.transition_texture, NULL, &dest);
        SDL_SetTextureAlphaMod(map_transition.transition_texture, 255);
    }
    else
    {
        SDL_SetRenderDrawColor(gameRenderer, 255, 255, 255, (Uint8)(map_transition.alpha * 255));
        SDL_Rect fullScreen = { 0, 0, WINDOW_W, WINDOW_H };
        SDL_RenderFillRect(gameRenderer, &fullScreen);
    }
}

// 根据分数计算速度倍率
static float get_speed_multiplier(int score)
{
    float multiplier = 1.0f + (score / 50) * 0.2f;
    if (multiplier > 3.0f)
    {
        multiplier = 3.0f;
    }
    return multiplier;
}

// 加载游戏资源
static void load_game_resources(void)
{
    // 地图
    const char *maps[] = {
        "assets/maps/map1.png",
        "assets/maps/map2.png",
        "assets/maps/map3.png",
        "assets/maps/map4.png",
        "assets/maps/map5.png",
    };

    // 加载地图纹理
    for (int i = 0; i < MAPS_COUNT; i++)
    {
        SDL_Surface *map_surface = IMG_Load(maps[i]);
        if (map_surface)
        {
            map_textures[i] = SDL_CreateTextureFromSurface(gameRenderer, map_surface);
            SDL_FreeSurface(map_surface);
        }
    }


    current_map_texture = map_textures[current_map];

    // 赛车皮肤
    const char *cars[] = {
        "assets/cars/red_up.png",
        "assets/cars/black_up.png",
        "assets/cars/bumblebee_up.png",
        "assets/cars/taxi_up.png",
        "assets/cars/tractor_up.png",
        "assets/cars/grey_up.png",
        "assets/cars/white_up.png",
        "assets/cars/motorcycle_up.png",
        "assets/cars/optimus_prime_up.png"
    };

    // 加载赛车皮肤纹理
    for (int i = 0; i < CARS_COUNT; i++)
    {
        SDL_Surface *car_surface = IMG_Load(cars[i]);
        if (car_surface)
        {
            car_textures[i] = SDL_CreateTextureFromSurface(gameRenderer, car_surface);
            SDL_FreeSurface(car_surface);
        }
    }

    // 使用配置中选择的车辆
    current_car_texture = car_textures[current_car];

    // 敌方赛车
    const char *enemy_cars[] = {
        "assets/cars/red_down.png",
        "assets/cars/black_down.png",
        "assets/cars/bumblebee_down.png",
        "assets/cars/taxi_down.png",
        "assets/cars/tractor_down.png",
        "assets/cars/grey_down.png",
        "assets/cars/white_down.png",
        "assets/cars/motorcycle_down.png",
        "assets/cars/optimus_prime_down.png"
    };

    // 加载敌方车辆纹理
    for (int i = 0; i < ENEMY_CARS_COUNT; i++)
    {
        SDL_Surface *enemy_surface = IMG_Load(enemy_cars[i]);
        if (enemy_surface)
        {
            enemy_textures[i] = SDL_CreateTextureFromSurface(gameRenderer, enemy_surface);
            SDL_FreeSurface(enemy_surface);
        }
    }

    // 初始化敌方车辆数组
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        enemies[i].x = 0;
        enemies[i].y = -CAR_HEIGHT;
        enemies[i].width = CAR_WIDTH;
        enemies[i].height = CAR_HEIGHT;
        enemies[i].speed = 3.0f + (rand() % 10) / 5.0f;
        enemies[i].active = 0;
        enemies[i].type = rand() % ENEMY_CARS_COUNT;
        enemies[i].texture = enemy_textures[enemies[i].type];
    }

    // 加载game over图片
    const char *game_over_path = "assets/ui/game_over.png";
    SDL_Surface *game_over_surface = IMG_Load(game_over_path);
    if (game_over_surface)
    {
        game_over_texture = SDL_CreateTextureFromSurface(gameRenderer, game_over_surface);
        SDL_FreeSurface(game_over_surface);
    }

    // 加载爆炸特效图片
    const char *explosion_path = "assets/ui/explosion.png";
    SDL_Surface *explosion_surface = IMG_Load(explosion_path);
    if (explosion_surface)
    {
        explosion_texture = SDL_CreateTextureFromSurface(gameRenderer, explosion_surface);
        SDL_FreeSurface(explosion_surface);
    }

    // 加载数字纹理
    const char *number_paths[] = {
        "assets/ui/0.jpg",
        "assets/ui/1.jpg",
        "assets/ui/2.jpg",
        "assets/ui/3.jpg",
        "assets/ui/4.jpg",
        "assets/ui/5.jpg",
        "assets/ui/6.jpg",
        "assets/ui/7.jpg",
        "assets/ui/8.jpg",
        "assets/ui/9.jpg"
    };

    for (int i = 0; i < 10; i++)
    {
        SDL_Surface *num_surface = IMG_Load(number_paths[i]);
        if (num_surface)
        {
            number_textures[i] = SDL_CreateTextureFromSurface(gameRenderer, num_surface);
            SDL_FreeSurface(num_surface);
        }
    }

    // 创建双缓冲纹理
    buffer_texture = SDL_CreateTexture(
        gameRenderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        WINDOW_W,
        WINDOW_H
    );

    // 游戏音乐
    const char *bgms[] = {
        "assets/bgm/Insomnia - Craig David.mp3",
        "assets/bgm/Lady Gaga - Poker Face.mp3",
        "assets/bgm/right now-akon.mp3",
        "assets/bgm/本兮,阿悄 - 无限速.mp3",
        "assets/bgm/死一样痛过(MC梦版)-MC몽.mp3"
    };

    int bgm_count = sizeof(bgms) / sizeof(bgms[0]);
    if (bgm_count > 0)
    {
        bgm = Mix_LoadMUS(bgms[current_bgm % bgm_count]);
        if (bgm)
        {
            Mix_PlayMusic(bgm, -1);
            Mix_VolumeMusic(128);
        }
    }

    // 加载暂停界面纹理
    const char *pause_path = "assets/home/pause.png";
    SDL_Surface *pause_surface = IMG_Load(pause_path);
    if (pause_surface)
    {
        pause_texture = SDL_CreateTextureFromSurface(gameRenderer, pause_surface);
        SDL_FreeSurface(pause_surface);
    }
}

// 清理游戏资源
static void cleanup_game_resources(void)
{
    if (bgm)
    {
        Mix_FreeMusic(bgm);
        bgm = NULL;
    }

    for (int i = 0; i < MAPS_COUNT; i++)
    {
        if (map_textures[i])
        {
            SDL_DestroyTexture(map_textures[i]);
            map_textures[i] = NULL;
        }
    }

    for (int i = 0; i < CARS_COUNT; i++)
    {
        if (car_textures[i])
        {
            SDL_DestroyTexture(car_textures[i]);
            car_textures[i] = NULL;
        }
    }

    for (int i = 0; i < ENEMY_CARS_COUNT; i++)
    {
        if (enemy_textures[i])
        {
            SDL_DestroyTexture(enemy_textures[i]);
            enemy_textures[i] = NULL;
        }
    }

    if (game_over_texture)
    {
        SDL_DestroyTexture(game_over_texture);
        game_over_texture = NULL;
    }

    if (explosion_texture)
    {
        SDL_DestroyTexture(explosion_texture);
        explosion_texture = NULL;
    }

    if (buffer_texture)
    {
        SDL_DestroyTexture(buffer_texture);
        buffer_texture = NULL;
    }

    for (int i = 0; i < 10; i++)
    {
        if (number_textures[i])
        {
            SDL_DestroyTexture(number_textures[i]);
            number_textures[i] = NULL;
        }
    }

    if (map_transition.transition_texture)
    {
        SDL_DestroyTexture(map_transition.transition_texture);
        map_transition.transition_texture = NULL;
    }

    // 清理暂停界面资源
    if (pause_texture)
    {
        SDL_DestroyTexture(pause_texture);
        pause_texture = NULL;
    }
}

// 清理游戏
static void cleanup_game(void)
{
    cleanup_game_resources();

    if (gameRenderer)
        SDL_DestroyRenderer(gameRenderer);
    if (gameWindow)
        SDL_DestroyWindow(gameWindow);

    if (speed_font)
    {
        TTF_CloseFont(speed_font);
        speed_font = NULL;
    }
    TTF_Quit();

    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();

    printf("游戏资源已清理\n");
}

// 初始化游戏
static void init_game(void)
{
    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        printf("SDL_Init() failed: %s\n", SDL_GetError());
        return;
    }

    // 初始化SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("Mix_OpenAudio() failed: %s\n", Mix_GetError());
        SDL_Quit();
        return;
    }

    // 初始化SDL_image
    if (IMG_Init(IMG_INIT_PNG) == 0)
    {
        printf("IMG_Init() failed: %s\n", IMG_GetError());
        Mix_CloseAudio();
        SDL_Quit();
        return;
    }

    // 初始化SDL_ttf
    if (TTF_Init() == -1)
    {
        printf("TTF_Init() failed: %s\n", TTF_GetError());
        // 继续运行，没有字体也可以
    }
    else
    {
        // 加载字体文件（确保你有这个字体文件）
        speed_font = TTF_OpenFont("assets/fonts/Ubuntu-B-2.ttf", 18);  // 18像素大小
        if (!speed_font)
        {
            printf("无法加载字体: %s\n", TTF_GetError());
        }
    }

    // 设置随机种子
    srand(getpid());

    // 创建窗口
    gameWindow = SDL_CreateWindow(
        "F1 Racing Car",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_W,
        WINDOW_H,
        SDL_WINDOW_SHOWN
    );

    if (!gameWindow)
    {
        printf("SDL_CreateWindow() failed: %s\n", SDL_GetError());
        return;
    }

    // 创建渲染器
    gameRenderer = SDL_CreateRenderer(
        gameWindow,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!gameRenderer)
    {
        printf("SDL_CreateRenderer() failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(gameWindow);
        return;
    }

    // 初始化游戏状态
    gameState = GAME_RUNNING;
    score = 0;
    lives = LIVES;
    game_over = 0;
    gameRunning = 1;
    enemy_spawn_timer = 0;
    enemy_spawn_delay = 60;
    explosion_duration = 0;

    // 初始化玩家位置和速度
    car_x = WINDOW_W / 2;
    car_y = WINDOW_H - CAR_HEIGHT - 20;
    current_car_speed = BASE_CAR_SPEED;
    scroll_position = 0.0f;
    scroll_speed = current_car_speed;

    current_car = 0;  // 使用第一辆车
    current_map = 0;  // 使用第一个地图

    // 加载游戏资源
    load_game_resources();

    // 初始化过场动画
    init_transition();

    printf("游戏初始化完成！\n");
}

// 生成敌方车辆
static void spawn_enemy(void)
{
    enemy_spawn_timer = 0;

    // 寻找可用的敌方车辆槽位
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].active)
        {
            // 激活敌方车辆
            enemies[i].active = 1;

            // 随机选择车道
            int lane = rand() % 2;  // 0或1

            if (lane == 0)
                enemies[i].x = WINDOW_W / 2 - CAR_WIDTH - 30;  // 左侧车道
            else
                enemies[i].x = WINDOW_W / 2 + CAR_WIDTH / 2; // 右侧车道

            // 从屏幕顶部开始
            enemies[i].y = -CAR_HEIGHT;

            // 基础速度 + 分数加成
            float speed_multiplier = get_speed_multiplier(score);
            float base_speed = 3.0f + (rand() % 10) / 5.0f;  // 3.0-5.0
            enemies[i].speed = base_speed * speed_multiplier;

            // 随机车辆类型
            enemies[i].type = rand() % ENEMY_CARS_COUNT;
            enemies[i].texture = enemy_textures[enemies[i].type];

            // 随着分数增加，生成间隔减少（游戏变难）
            int base_delay = 60;
            int min_delay = 20;
            int adjusted_delay = base_delay - (score / 50) * 5;
            if (adjusted_delay < min_delay) adjusted_delay = min_delay;
            enemy_spawn_delay = adjusted_delay + (rand() % 20);

            break;
        }
    }
}

// 更新敌方车辆
static void update_enemies(void)
{
    enemy_spawn_timer++;

    // 生成新的敌方车辆
    if (enemy_spawn_timer >= enemy_spawn_delay)
    {
        spawn_enemy();
    }

    // 更新敌方车辆位置
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            // 基础速度 + 滚动因子影响
            // 当 enemy_scroll_factor > 0 时（玩家向前），敌方车辆移动更快
            // 当 enemy_scroll_factor < 0 时（玩家向后），敌方车辆移动更慢
            float effective_speed = enemies[i].speed * (1.0f + enemy_scroll_factor);
            enemies[i].y += effective_speed;

            // 如果车辆超出屏幕底部，重置并增加分数
            if (enemies[i].y > WINDOW_H)
            {
                enemies[i].active = 0;
                enemies[i].y = -CAR_HEIGHT;
                score += 10;  // 每过一辆车加10分
                printf("得分！当前分数: %d\n", score);
            }
        }
    }
}

// 检查碰撞
static void check_collisions(void)
{
    SDL_Rect playerRect = { car_x, car_y, CAR_WIDTH, CAR_HEIGHT };

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            SDL_Rect enemyRect = {
                enemies[i].x,
                (int)enemies[i].y,
                CAR_WIDTH,
                CAR_HEIGHT
            };

            // 简单的矩形碰撞检测
            if (playerRect.x < enemyRect.x + enemyRect.w &&
                playerRect.x + playerRect.w > enemyRect.x &&
                playerRect.y < enemyRect.y + enemyRect.h &&
                playerRect.y + playerRect.h > enemyRect.y)
            {
                printf("碰撞！生命值-1\n");

                // 触发爆炸特效
                explosion_x = car_x + CAR_WIDTH / 2 - 40;  // 居中显示
                explosion_y = car_y + CAR_HEIGHT / 2 - 40; // 居中显示
                explosion_alpha = 1.0f;  // 完全不透明开始
                explosion_duration = 30; // 持续30帧（约0.5秒）

                // 减少生命值
                lives--;

                // 重置碰撞的敌方车辆
                enemies[i].active = 0;
                enemies[i].y = -CAR_HEIGHT;

                // 如果生命值为0，游戏结束
                if (lives <= 0)
                {
                    game_over = 1;

                    // 游戏结束时暂停音乐
                    Mix_HaltMusic();

                    printf("游戏结束！最终分数: %d\n", score);
                }
                else
                {
                    printf("剩余生命值: %d\n", lives);
                }
            }
        }
    }
}

// 处理事件
static void handle_events(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            gameRunning = 0;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            // ESC键退出游戏
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                gameRunning = 0;
            }

            // P键暂停/继续游戏和音乐
            else if (event.key.keysym.sym == SDLK_p)
            {
                if (gameState == GAME_RUNNING)
                {
                    // 暂停游戏和音乐
                    gameState = GAME_PAUSED;
                    if (bgm && Mix_PlayingMusic())
                    {
                        Mix_PauseMusic();
                    }
                    printf("游戏已暂停\n");
                }
                else if (gameState == GAME_PAUSED)
                {
                    // 继续游戏和音乐
                    gameState = GAME_RUNNING;
                    if (bgm && Mix_PausedMusic())
                    {
                        Mix_ResumeMusic();
                    }
                    printf("游戏已继续\n");
                }
            }
            // R键重新开始游戏（在暂停或游戏结束时）
            else if (event.key.keysym.sym == SDLK_r &&
                     (gameState == GAME_PAUSED || gameState == GAME_OVER))
            {
                printf("重新开始游戏\n");

                // 重新初始化游戏
                cleanup_game_resources();
                init_game();

                // 重置状态
                gameState = GAME_RUNNING;
                score = 0;
                lives = LIVES;
                game_over = 0;
                enemy_spawn_timer = 0;
                explosion_duration = 0;

                // 重新加载资源
                load_game_resources();
                init_transition();

                // 重新开始音乐
                if (bgm)
                {
                    Mix_PlayMusic(bgm, -1);
                    Mix_VolumeMusic(128);
                }
            }


            // 原有功能键（只在游戏运行或暂停时可用）
            else if (gameState != GAME_OVER)
            {
                // M键切换地图
                if (event.key.keysym.sym == SDLK_m && map_transition.state == TRANSITION_NONE)
                {
                    int next_map = (current_map + 1) % MAPS_COUNT;
                    pending_map_change = next_map;
                    start_transition();

                    const char *transitions[] = {
                        "assets/maps/map1_page.png",
                        "assets/maps/map2_page.png",
                        "assets/maps/map3_page.png",
                        "assets/maps/map4_page.png",
                        "assets/maps/map5_page.png",
                    };

                    if (next_map < 5)
                    {
                        if (map_transition.transition_texture)
                        {
                            SDL_DestroyTexture(map_transition.transition_texture);
                        }
                        map_transition.transition_texture = load_transition_texture(transitions[next_map]);
                    }
                }
                // C键切换车辆
                else if (event.key.keysym.sym == SDLK_c)
                {
                    current_car = (current_car + 1) % CARS_COUNT;
                    current_car_texture = car_textures[current_car];
                }
                // N键切换下一首音乐
                else if (event.key.keysym.sym == SDLK_n)
                {
                    const char *bgms[] = {
                        "assets/bgm/Insomnia - Craig David.mp3",
                        "assets/bgm/Lady Gaga - Poker Face.mp3",
                        "assets/bgm/right now-akon.mp3",
                        "assets/bgm/本兮,阿俏 - 无限速.mp3",
                        "assets/bgm/死一样痛过(MC梦版)-MC몽.mp3"
                    };
                    int bgm_count = sizeof(bgms) / sizeof(bgms[0]);

                    if (bgm_count > 0 && bgm)
                    {
                        Mix_HaltMusic();
                        Mix_FreeMusic(bgm);

                        current_bgm = (current_bgm + 1) % bgm_count;
                        bgm = Mix_LoadMUS(bgms[current_bgm]);
                        if (bgm && Mix_PlayMusic(bgm, -1) == 0)
                        {
                            Mix_VolumeMusic(128);
                            printf("切换到下一首音乐: %d\n", current_bgm + 1);
                        }
                    }
                }
                // B键切换上一首音乐
                else if (event.key.keysym.sym == SDLK_b)
                {
                    const char *bgms[] = {
                        "assets/bgm/Insomnia - Craig David.mp3",
                        "assets/bgm/Lady Gaga - Poker Face.mp3",
                        "assets/bgm/right now-akon.mp3",
                        "assets/bgm/本兮,阿俏 - 无限速.mp3",
                        "assets/bgm/死一样痛过(MC梦版)-MC몽.mp3"
                    };
                    int bgm_count = sizeof(bgms) / sizeof(bgms[0]);

                    if (bgm_count > 0 && bgm)
                    {
                        Mix_HaltMusic();
                        Mix_FreeMusic(bgm);

                        current_bgm = (current_bgm - 1 + bgm_count) % bgm_count;
                        bgm = Mix_LoadMUS(bgms[current_bgm]);
                        if (bgm && Mix_PlayMusic(bgm, -1) == 0)
                        {
                            Mix_VolumeMusic(128);
                            printf("切换到上一首音乐: %d\n", current_bgm + 1);
                        }
                    }
                }
            }
        }
    }
}

// 更新游戏逻辑
static void update_game(void)
{
    if (game_over)
        return;

    // 如果游戏暂停或结束，不更新游戏逻辑
    if (gameState == GAME_PAUSED || gameState == GAME_OVER)
        return;

    // 获取键盘状态
    const Uint8 *keyState = SDL_GetKeyboardState(NULL);

    // 根据当前分数计算速度倍率
    float speed_multiplier = get_speed_multiplier(score);

    // 更新当前玩家速度
    current_car_speed = (int)(BASE_CAR_SPEED * speed_multiplier);
    scroll_speed = current_car_speed;

    // 重置滚动因子
    enemy_scroll_factor = 0.0f;

    // 处理持续按键
    if (keyState[SDL_SCANCODE_UP] || keyState[SDL_SCANCODE_W])
    {
        scroll_position -= scroll_speed;
        enemy_scroll_factor = 0.8f;  // 当地图向上滚动时，敌方车辆应该移动更快（看起来像后退）
    }
    else if (keyState[SDL_SCANCODE_DOWN] || keyState[SDL_SCANCODE_S])
    {
        scroll_position += scroll_speed;
        enemy_scroll_factor = -0.5f; // 当地图向下滚动时，敌方车辆应该移动更慢（看起来像前进）
    }

    if (keyState[SDL_SCANCODE_LEFT] || keyState[SDL_SCANCODE_A])
        car_x -= current_car_speed;

    if (keyState[SDL_SCANCODE_RIGHT] || keyState[SDL_SCANCODE_D])
        car_x += current_car_speed;

    // 赛车边界检查
    if (car_x < WINDOW_W / 2 - CAR_WIDTH - 30)
        car_x = WINDOW_W / 2 - CAR_WIDTH - 30;
    if (car_x > WINDOW_W / 2 + CAR_WIDTH - 30)
        car_x = WINDOW_W / 2 + CAR_WIDTH - 30;

    // 更新过场动画
    if (map_transition.state != TRANSITION_NONE)
    {
        int transition_finished = update_transition();

        // 如果淡出完成且淡入开始，切换到新地图
        if (map_transition.state == TRANSITION_FADE_IN)
        {
            if (pending_map_change != -1 && transition_finished == 0 && map_transition.timer == 1)
            {
                current_map = pending_map_change;
                scroll_position = 0;
                current_map_texture = map_textures[current_map];
                printf("切换到地图 %d\n", current_map + 1);
            }

            if (transition_finished)
            {
                pending_map_change = -1;
            }
        }
    }

    // 更新敌方车辆
    update_enemies();

    // 检查碰撞
    if (map_transition.state == TRANSITION_NONE)
    {
        check_collisions();
    }

    // 更新爆炸特效
    if (explosion_duration > 0)
    {
        explosion_duration--;
        explosion_alpha = explosion_duration / 30.0f;
        explosion_x += rand() % 5 - 2;
        explosion_y += rand() % 5 - 2;
    }

    // 在碰撞检测后更新游戏状态
    if (lives <= 0)
    {
        game_over = 1;
        gameState = GAME_OVER;  // 设置游戏状态为结束
        Mix_HaltMusic();
        printf("游戏结束！最终分数: %d\n", score);
    }

}

// 渲染UI
static void render_ui(void)
{
    // 渲染分数（左上角）
    SDL_SetRenderDrawColor(gameRenderer, 0, 0, 0, 180);
    SDL_Rect score_bg = { 10, 10, 120, 50 };
    SDL_RenderFillRect(gameRenderer, &score_bg);
    SDL_SetRenderDrawColor(gameRenderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(gameRenderer, &score_bg);

    // 显示分数
    char score_str[20];
    sprintf(score_str, "%d", score);
    int score_x = 20;
    int score_y = 20;

    // 尝试使用数字纹理
    int has_numbers = 0;
    for (int i = 0; i < 10; i++)
    {
        if (number_textures[i])
        {
            has_numbers = 1;
            break;
        }
    }

    if (has_numbers)
    {
        for (int i = 0; score_str[i]; i++)
        {
            int digit = score_str[i] - '0';
            if (digit >= 0 && digit <= 9 && number_textures[digit])
            {
                SDL_Rect digit_rect = { score_x, score_y, 20, 30 };
                SDL_RenderCopy(gameRenderer, number_textures[digit], NULL, &digit_rect);
                score_x += 22;
            }
        }
    }
    else
    {
        SDL_SetRenderDrawColor(gameRenderer, 255, 255, 255, 255);
        for (int i = 0; score_str[i]; i++)
        {
            SDL_Rect digit_rect = { score_x, score_y, 20, 30 };
            SDL_RenderDrawRect(gameRenderer, &digit_rect);
            score_x += 22;
        }
    }

    /*------------------ 血条背景框 ------------------*/
    // 血条背景框（右上角）
    SDL_SetRenderDrawColor(gameRenderer, 0, 0, 0, 180);
    SDL_Rect life_bg = { WINDOW_W - 130, 10, 120, 50 };
    SDL_RenderFillRect(gameRenderer, &life_bg);

    // 血条边框
    SDL_SetRenderDrawColor(gameRenderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(gameRenderer, &life_bg);

    // 绘制红色小方块
    SDL_SetRenderDrawColor(gameRenderer, 255, 0, 0, 255);

    // 紧凑布局：小方块，小间距
    int life_block_size = 16;     // 更小的方块
    int life_block_spacing = 3;   // 更小的间距

    // 计算总宽度，居中显示
    int total_life_width = lives * life_block_size + (lives - 1) * life_block_spacing;
    int life_start_x = life_bg.x + (life_bg.w - total_life_width) / 2;
    int life_y = life_bg.y + (life_bg.h - life_block_size) / 2;

    for (int i = 0; i < lives; i++)
    {
        SDL_Rect life_rect = {
            life_start_x + i * (life_block_size + life_block_spacing),
            life_y,
            life_block_size,
            life_block_size
        };
        SDL_RenderFillRect(gameRenderer, &life_rect);

        // 可选：添加白色边框让方块更清晰
        SDL_SetRenderDrawColor(gameRenderer, 255, 255, 255, 100);  // 半透明白色
        SDL_RenderDrawRect(gameRenderer, &life_rect);
        SDL_SetRenderDrawColor(gameRenderer, 255, 0, 0, 255);      // 恢复红色
    }

    /*------------------ 渲染速度倍率 ------------------*/
    // 渲染速度倍率显示（右上角，放在血条下面）
     // 渲染速度倍率显示（使用TTF字体）
    float speed_multiplier = get_speed_multiplier(score);
    char speed_str[30];
    sprintf(speed_str, "SPEED: x%.1f", speed_multiplier);

    if (speed_font)
    {
        // 使用TTF字体渲染文字
        SDL_Color text_color = { 255, 255, 255, 255 };  // 白色
        SDL_Surface *text_surface = TTF_RenderText_Solid(speed_font, speed_str, text_color);
        if (text_surface)
        {
            SDL_Texture *text_texture = SDL_CreateTextureFromSurface(gameRenderer, text_surface);
            if (text_texture)
            {
                // 计算位置
                int text_x = WINDOW_W - text_surface->w - 20;
                int text_y = 70;

                // 绘制背景
                SDL_SetRenderDrawColor(gameRenderer, 0, 0, 0, 180);
                SDL_Rect speed_bg = { text_x - 5, text_y - 2, text_surface->w + 10, text_surface->h + 4 };
                SDL_RenderFillRect(gameRenderer, &speed_bg);
                SDL_SetRenderDrawColor(gameRenderer, 255, 255, 255, 255);
                SDL_RenderDrawRect(gameRenderer, &speed_bg);

                // 绘制文字
                SDL_Rect text_rect = { text_x, text_y, text_surface->w, text_surface->h };
                SDL_RenderCopy(gameRenderer, text_texture, NULL, &text_rect);

                SDL_DestroyTexture(text_texture);
            }
            SDL_FreeSurface(text_surface);
        }
    }
    else
    {
        // 备用方案：简单绘制矩形
        int speed_x = WINDOW_W - 80;
        int speed_y = 70;

        SDL_SetRenderDrawColor(gameRenderer, 0, 0, 0, 180);
        SDL_Rect speed_bg = { speed_x - 10, speed_y - 5, 70, 25 };
        SDL_RenderFillRect(gameRenderer, &speed_bg);
        SDL_SetRenderDrawColor(gameRenderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(gameRenderer, &speed_bg);

        // 简单绘制字符框
        for (int i = 0; i < 5; i++)  // 假设显示"x1.5"
        {
            SDL_Rect char_rect = { speed_x + i * 14, speed_y, 12, 20 };
            SDL_RenderDrawRect(gameRenderer, &char_rect);
        }
    }
}

// 渲染游戏
static void render_game(void)
{
    // 清屏
    SDL_SetRenderDrawColor(gameRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gameRenderer);

    if (game_over && game_over_texture)
    {
        // 绘制game over图片
        SDL_Rect dest = { 0, 0, WINDOW_W, WINDOW_H };
        SDL_RenderCopy(gameRenderer, game_over_texture, NULL, &dest);

        SDL_RenderPresent(gameRenderer);
        return;
    }

    // 渲染地图（双缓冲版本）
    if (current_map_texture)
    {
        // 获取地图纹理的尺寸
        int tex_w, tex_h;
        SDL_QueryTexture(current_map_texture, NULL, NULL, &tex_w, &tex_h);

        // 计算实际滚动位置（循环）
        float actual_scroll = fmod(scroll_position, tex_h);
        if (actual_scroll < 0) actual_scroll += tex_h;

        // 绘制第一部分
        int src_y1 = (int)actual_scroll;
        int dest_y1 = 0;
        int height1 = WINDOW_H;

        // 如果地图高度不够，需要绘制第二部分
        if (src_y1 + WINDOW_H > tex_h)
        {
            height1 = tex_h - src_y1;

            // 绘制第二部分（从地图顶部开始）
            SDL_Rect src_rect2 = { 0, 0, WINDOW_W, WINDOW_H - height1 };
            SDL_Rect dest_rect2 = { 0, height1, WINDOW_W, WINDOW_H - height1 };
            SDL_RenderCopy(gameRenderer, current_map_texture, &src_rect2, &dest_rect2);
        }

        // 绘制第一部分
        SDL_Rect src_rect1 = { 0, src_y1, WINDOW_W, height1 };
        SDL_Rect dest_rect1 = { 0, dest_y1, WINDOW_W, height1 };
        SDL_RenderCopy(gameRenderer, current_map_texture, &src_rect1, &dest_rect1);
    }

    // 渲染敌方车辆
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active && enemies[i].texture)
        {
            SDL_Rect enemyRect = {
                enemies[i].x,
                (int)enemies[i].y,
                enemies[i].width,
                enemies[i].height
            };
            SDL_RenderCopy(gameRenderer, enemies[i].texture, NULL, &enemyRect);
        }
    }

    // 渲染玩家车辆
    if (current_car_texture)
    {
        SDL_Rect carRect = { car_x, car_y, CAR_WIDTH, CAR_HEIGHT };
        SDL_RenderCopy(gameRenderer, current_car_texture, NULL, &carRect);
    }

    // 渲染爆炸特效
    if (explosion_duration > 0 && explosion_texture)
    {
        SDL_SetTextureAlphaMod(explosion_texture, (Uint8)(explosion_alpha * 255));
        SDL_Rect explosionRect = {
            explosion_x,
            explosion_y,
            80,
            80
        };
        SDL_RenderCopy(gameRenderer, explosion_texture, NULL, &explosionRect);
        SDL_SetTextureAlphaMod(explosion_texture, 255);
    }

    // 渲染UI
    render_ui();

    // 渲染过场动画
    render_transition();

    // 如果游戏暂停，渲染暂停界面
    if (gameState == GAME_PAUSED)
    {
        render_pause_menu();
    }

    // 更新屏幕
    SDL_RenderPresent(gameRenderer);
}

// 运行游戏
int run_game(void)
{
    init_game();

    while (gameRunning)
    {
        handle_events();

        if (!game_over)
        {
            update_game();
        }

        render_game();
        SDL_Delay(16);
    }

    cleanup_game();
    return score;
}