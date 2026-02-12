#include "game.h"
#include "bgm.h"
#include "car.h"
#include "enemy.h"
#include "rendering.h"
#include "transition.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <SDL2/SDL_image.h>

// 声明外部全局变量
extern BGMState bgm_state;
extern CarState car_state;
extern EnemyCar enemies[MAX_ENEMIES];
extern Transition transition_state;
extern UIState ui_state;

// 静态函数声明
static void handle_player_input(GameState *game);
static void check_collisions(GameState *game);
static void load_explosion_texture(GameState *game);
static int init_game_components(GameState *game);

// 初始化游戏
int init_game(GameState *game)
{
    // 初始化随机种子
    srand(getpid());

    // 初始化SDL视频和音频子系统
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        printf("SDL初始化失败: %s\n", SDL_GetError());
        return 0;
    }

    // 初始化SDL扩展库（SDL_image）
    if (!init_sdl_extensions())
    {
        SDL_Quit();
        return 0;
    }

    // 创建游戏窗口
    game->window = SDL_CreateWindow(
        "F1 Racing Car",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_W,
        WINDOW_H,
        SDL_WINDOW_SHOWN
    );

    if (!game->window)
    {
        printf("创建窗口失败: %s\n", SDL_GetError());
        cleanup_sdl_extensions();
        SDL_Quit();
        return 0;
    }

    // 创建渲染器（使用硬件加速和垂直同步）
    game->renderer = SDL_CreateRenderer(
        game->window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!game->renderer)
    {
        printf("创建渲染器失败: %s\n", SDL_GetError());
        SDL_DestroyWindow(game->window);
        cleanup_sdl_extensions();
        SDL_Quit();
        return 0;
    }

    // 创建双缓冲纹理，用于平滑渲染
    game->buffer_texture = SDL_CreateTexture(
        game->renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        WINDOW_W,
        WINDOW_H
    );

    // 初始化游戏状态
    game->running = 1;           // 游戏运行标志
    game->game_over = 0;         // 游戏结束标志
    game->score = 0;             // 初始分数
    game->lives = 5;             // 初始生命值

    // 初始化玩家赛车位置和速度
    game->car_x = WINDOW_W / 2 - CAR_WIDTH / 2;   // 水平居中
    game->car_y = WINDOW_H - CAR_HEIGHT - 20;     // 靠近底部
    game->base_car_speed = 5;                     // 基础速度
    game->current_car_speed = game->base_car_speed; // 当前速度

    // 初始化地图滚动相关变量
    game->scroll_position = 0.0f;      // 地图滚动位置
    game->scroll_speed = game->current_car_speed;  // 滚动速度
    game->enemy_scroll_factor = 0.0f;  // 敌方车辆滚动因子

    // 获取键盘状态，用于持续按键检测
    game->key_state = SDL_GetKeyboardState(NULL);

    // 初始化敌方车辆生成计时器
    game->enemy_spawn_timer = 0;      // 计时器
    game->enemy_spawn_delay = 60;     // 初始生成间隔（60帧≈1秒）

    // 初始化爆炸特效
    game->explosion_texture = NULL;   // 爆炸纹理
    game->explosion_alpha = 0.0f;     // 透明度
    game->explosion_duration = 0;     // 持续时间
    game->explosion_x = 0;            // X坐标
    game->explosion_y = 0;            // Y坐标

    // 加载爆炸特效纹理
    load_explosion_texture(game);

    // 初始化各子系统
    if (!init_game_components(game))
    {
        printf("游戏初始化失败！\n");
        cleanup_game(game);
        return 0;
    }

    printf("游戏初始化成功！\n");
    printf("控制说明：\n");
    printf("  WASD/方向键 - 控制赛车移动\n");
    printf("  P - 暂停/继续音乐\n");
    printf("  N/B - 切换下一首/上一首音乐\n");
    printf("  M - 切换地图\n");
    printf("  C - 切换赛车皮肤\n");

    return 1;
}

// 初始化游戏组件
static int init_game_components(GameState *game)
{
    // 初始化背景音乐系统
    if (!init_bgm())
    {
        printf("背景音乐初始化失败\n");
        return 0;
    }

    // 初始化赛车系统
    if (!init_car(game->renderer))
    {
        printf("赛车系统初始化失败\n");
        return 0;
    }

    // 初始化敌方车辆系统
    if (!init_enemy(game->renderer))
    {
        printf("敌方车辆系统初始化失败\n");
        return 0;
    }

    // 初始化过场动画系统
    if (!init_transition(game->renderer))
    {
        printf("过场动画系统初始化失败\n");
        return 0;
    }

    // 初始化UI系统
    if (!init_ui(game->renderer))
    {
        printf("UI系统初始化失败\n");
        return 0;
    }

    // 初始化渲染系统
    if (!init_rendering(game->renderer))
    {
        printf("渲染系统初始化失败\n");
        return 0;
    }

    return 1;
}

// 处理游戏事件
void handle_events(GameState *game)
{
    SDL_Event event;

    // 处理所有待处理的事件
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                // 窗口关闭事件
                game->running = 0;
                break;

            case SDL_KEYDOWN:
                // 键盘按下事件
                handle_keydown(event.key.keysym.sym, game->game_over);
                break;
        }
    }
}

// 更新游戏逻辑
void update_game(GameState *game)
{
    // 如果游戏结束，不更新逻辑
    if (game->game_over)
        return;

    // 如果正在播放过场动画，只更新动画
    if (transition_state.state != TRANSITION_NONE)
    {
        update_transition();
        return;
    }

    // 根据当前分数计算速度倍率
    float speed_multiplier = get_speed_multiplier(game->score);

    // 更新玩家速度
    game->current_car_speed = (int)(game->base_car_speed * speed_multiplier);
    game->scroll_speed = game->current_car_speed;

    // 处理玩家输入
    handle_player_input(game);

    // 更新敌方车辆
    update_enemies(game, speed_multiplier);

    // 检测碰撞
    check_collisions(game);

    // 更新爆炸特效
    if (game->explosion_duration > 0)
    {
        game->explosion_duration--;
        game->explosion_alpha = game->explosion_duration / 30.0f;

        // 添加轻微的随机偏移，模拟震动效果
        game->explosion_x += rand() % 5 - 2;
        game->explosion_y += rand() % 5 - 2;
    }
}

// 渲染游戏画面
void render_game(GameState *game)
{
    // 清屏（设置为黑色背景）
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderClear(game->renderer);

    if (game->game_over)
    {
        // 渲染游戏结束画面
        render_game_over(game->renderer);
    }
    else
    {
        // 渲染游戏场景
        render_map(game);                              // 渲染地图
        render_enemies(game->renderer);               // 渲染敌方车辆
        render_car(game->renderer, game->car_x, game->car_y); // 渲染玩家赛车

        // 渲染爆炸特效（如果有）
        if (game->explosion_duration > 0 && game->explosion_texture)
        {
            render_explosion(game);
        }

        // 渲染UI元素
        render_ui(game);
    }

    // 渲染过场动画（覆盖在所有内容之上）
    render_transition(game->renderer);

    // 更新屏幕显示
    SDL_RenderPresent(game->renderer);
}

// 清理游戏资源
void cleanup_game(GameState *game)
{
    printf("正在清理游戏资源...\n");

    // 清理各子系统资源
    cleanup_ui();
    cleanup_transition();
    cleanup_enemy();
    cleanup_car();
    cleanup_bgm();
    cleanup_rendering();

    // 清理爆炸纹理
    if (game->explosion_texture)
    {
        SDL_DestroyTexture(game->explosion_texture);
        game->explosion_texture = NULL;
    }

    // 清理双缓冲纹理
    if (game->buffer_texture)
    {
        SDL_DestroyTexture(game->buffer_texture);
        game->buffer_texture = NULL;
    }

    // 清理SDL渲染器和窗口
    if (game->renderer)
    {
        SDL_DestroyRenderer(game->renderer);
        game->renderer = NULL;
    }

    if (game->window)
    {
        SDL_DestroyWindow(game->window);
        game->window = NULL;
    }

    // 清理SDL扩展库并退出SDL
    cleanup_sdl_extensions();
    SDL_Quit();

    printf("游戏资源清理完成！\n");
}

// 计算速度倍率
float get_speed_multiplier(int score)
{
    // 基础速度为 1.0
    // 每 50 分增加 0.2 倍速度
    float multiplier = 1.0f + (score / 50) * 0.2f;

    // 限制最大速度为3.0倍
    if (multiplier > 3.0f)
        multiplier = 3.0f;

    return multiplier;
}

// 处理玩家输入
static void handle_player_input(GameState *game)
{
    // 上下移动：控制地图滚动
    if (game->key_state[SDL_SCANCODE_UP] || game->key_state[SDL_SCANCODE_W])
    {
        // 向上滚动，敌方车辆需要更快
        game->scroll_position -= game->scroll_speed;
        game->enemy_scroll_factor = 0.8f;
    }
    else if (game->key_state[SDL_SCANCODE_DOWN] || game->key_state[SDL_SCANCODE_S])
    {
        // 向下滚动，敌方车辆减慢
        game->scroll_position += game->scroll_speed;
        game->enemy_scroll_factor = -0.3f;
    }
    else
    {
        // 没有滚动，敌方车辆正常速度
        game->enemy_scroll_factor = 0.0f;
    }

    // 左右移动：控制赛车横向位置
    if (game->key_state[SDL_SCANCODE_LEFT] || game->key_state[SDL_SCANCODE_A])
    {
        game->car_x -= game->current_car_speed;
    }

    if (game->key_state[SDL_SCANCODE_RIGHT] || game->key_state[SDL_SCANCODE_D])
    {
        game->car_x += game->current_car_speed;
    }

    // 赛车边界检查，防止移出道路
    int left_boundary = WINDOW_W / 2 - CAR_WIDTH - 30;
    int right_boundary = WINDOW_W / 2 + CAR_WIDTH - 30;

    if (game->car_x < left_boundary)
        game->car_x = left_boundary;

    if (game->car_x > right_boundary)
        game->car_x = right_boundary;
}

// 检测碰撞
static void check_collisions(GameState *game)
{
    // 玩家赛车的碰撞矩形
    SDL_Rect player_rect = {
        game->car_x,
        game->car_y,
        CAR_WIDTH,
        CAR_HEIGHT
    };

    // 检查与所有敌方车辆的碰撞
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].active)
            continue;

        // 敌方车辆的碰撞矩形
        SDL_Rect enemy_rect = {
            enemies[i].x,
            (int)enemies[i].y,
            CAR_WIDTH,
            CAR_HEIGHT
        };

        // 使用SDL的碰撞检测函数
        if (SDL_HasIntersection(&player_rect, &enemy_rect))
        {
            printf("发生碰撞, 生命值-1\n");

            // 设置爆炸特效位置（在玩家赛车中心）
            game->explosion_x = game->car_x + CAR_WIDTH / 2 - 40;
            game->explosion_y = game->car_y + CAR_HEIGHT / 2 - 40;
            game->explosion_alpha = 1.0f;
            game->explosion_duration = 30;  // 持续30帧（约0.5秒）

            // 减少生命值
            game->lives--;

            // 禁用被碰撞的敌方车辆
            enemies[i].active = 0;

            // 检查游戏是否结束
            if (game->lives <= 0)
            {
                game->game_over = 1;
                stop_bgm();  // 停止背景音乐
                printf("游戏结束！最终分数: %d\n", game->score);
            }
            else
            {
                printf("剩余生命值: %d\n", game->lives);
            }

            break;  // 一次只处理一个碰撞
        }
    }
}

// 加载爆炸特效纹理
static void load_explosion_texture(GameState *game)
{
    SDL_Surface *surface = IMG_Load("assets/ui/explosion.png");

    if (surface)
    {
        game->explosion_texture = SDL_CreateTextureFromSurface(game->renderer, surface);
        SDL_FreeSurface(surface);

        if (game->explosion_texture)
        {
            printf("爆炸特效纹理加载成功\n");
        }
        else
        {
            printf("创建爆炸特效纹理失败\n");
        }
    }
    else
    {
        printf("无法加载爆炸特效图片: %s\n", IMG_GetError());
    }
}