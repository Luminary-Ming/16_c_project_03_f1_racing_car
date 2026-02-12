#include "transition.h"
#include <stdio.h>
#include <SDL2/SDL_image.h>
#include "rendering.h"

// 全局过场动画状态
Transition transition_state = { 0 };

// 静态变量
static int pending_map_change = -1;

// 过场图片文件列表
static const char *transition_files[] = {
    "assets/maps/map1_page.png",
    "assets/maps/map2_page.png",
    "assets/maps/map3_page.png",
    "assets/maps/map4_page.png",
    "assets/maps/map5_page.png"
};

// 静态函数声明
static void load_transition_texture(SDL_Renderer *renderer);

// 初始化过场动画系统
int init_transition(SDL_Renderer *renderer)
{
    // 初始化状态
    transition_state.state = TRANSITION_NONE;
    transition_state.alpha = 0.0f;
    transition_state.speed = 0.02f;      // 过渡速度
    transition_state.duration = 500;     // 持续时间（帧）
    transition_state.timer = 0;          // 计时器
    transition_state.texture = NULL;     // 纹理指针

    // 加载过场动画纹理
    load_transition_texture(renderer);

    return 1;
}

// 开始过场动画
void start_transition(SDL_Renderer *renderer, int next_map)
{
    // 设置过场动画状态
    transition_state.state = TRANSITION_FADE_OUT;  // 开始淡出
    transition_state.alpha = 0.0f;                 // 透明度从0开始
    transition_state.timer = 0;                    // 重置计时器
    pending_map_change = next_map;                 // 设置要切换的地图

    // 切换到对应地图的过场图片（添加这一行）
    change_transition_texture(renderer, next_map);

    printf("开始过场动画，切换到地图: %d\n", next_map + 1);
}

// 更新过场动画
void update_transition(SDL_Renderer *renderer)
{
    // 如果没有过场动画，直接返回
    if (transition_state.state == TRANSITION_NONE)
    {
        return;
    }

    // 增加计时器
    transition_state.timer++;

    // 根据状态更新动画
    switch (transition_state.state)
    {
        case TRANSITION_FADE_OUT:
            // 淡出阶段：透明度增加
            transition_state.alpha += transition_state.speed;

            // 检查是否达到完全透明
            if (transition_state.alpha >= 1.0f)
            {
                transition_state.alpha = 1.0f;           // 确保透明度不超过1.0
                transition_state.state = TRANSITION_FADE_IN; // 切换到淡入阶段
                transition_state.timer = 0;             // 重置计时器

                // 切换地图（在完全淡出时执行）
                if (pending_map_change != -1)
                {
                    // 更新当前地图索引
                    current_map_index = pending_map_change;
                    // 切换到对应地图的过场图片
                    change_transition_texture(renderer, pending_map_change);
                    printf("地图已切换到: %d\n", current_map_index + 1);
                }
            }
            break;

        case TRANSITION_FADE_IN:
            // 淡入阶段：透明度减少
            transition_state.alpha -= transition_state.speed;

            // 检查是否完全淡入（透明度为0）
            if (transition_state.alpha <= 0.0f)
            {
                transition_state.alpha = 0.0f;           // 确保透明度不低于0.0
                transition_state.state = TRANSITION_NONE; // 动画结束
                transition_state.timer = 0;             // 重置计时器
                pending_map_change = -1;                // 清除待切换的地图

                printf("过场动画结束\n");
            }
            break;

        default:
            // 未知状态，重置为无动画
            transition_state.state = TRANSITION_NONE;
            break;
    }
}

// 渲染过场动画
void render_transition(SDL_Renderer *renderer)
{
    // 如果没有过场动画或透明度为0，不渲染
    if (transition_state.state == TRANSITION_NONE || transition_state.alpha <= 0.0f)
    {
        return;
    }

    // 如果有纹理，使用纹理渲染
    if (transition_state.texture)
    {
        // 设置纹理透明度
        SDL_SetTextureAlphaMod(transition_state.texture, (Uint8)(transition_state.alpha * 255));

        // 渲染到整个屏幕
        SDL_Rect dest = {
            0,                    // x坐标
            0,                    // y坐标
            500,                  // 宽度（WINDOW_W）
            800                   // 高度（WINDOW_H）
        };

        SDL_RenderCopy(renderer, transition_state.texture, NULL, &dest);

        // 重置纹理透明度
        SDL_SetTextureAlphaMod(transition_state.texture, 255);
    }
}

// 清理过场动画资源
void cleanup_transition(void)
{
    // 清理纹理资源
    if (transition_state.texture)
    {
        SDL_DestroyTexture(transition_state.texture);
        transition_state.texture = NULL;
        printf("清理过场动画纹理\n");
    }

    // 重置状态
    transition_state.state = TRANSITION_NONE;
    transition_state.alpha = 0.0f;
    transition_state.timer = 0;
    pending_map_change = -1;

    printf("过场动画资源清理完成\n");
}

// 加载过场动画纹理
static void load_transition_texture(SDL_Renderer *renderer)
{
    // 检查是否有过场图片文件列表
    if (sizeof(transition_files) / sizeof(transition_files[0]) == 0)
    {
        printf("没有过场图片文件列表\n");
        return;
    }

    // 尝试加载第一张过场图片作为默认纹理
    SDL_Surface *surface = IMG_Load(transition_files[0]);
    if (surface)
    {
        // 创建纹理
        transition_state.texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (transition_state.texture)
        {
            printf("过场动画纹理加载成功: %s\n", transition_files[0]);
        }
        else
        {
            printf("创建过场动画纹理失败\n");
        }
    }
    else
    {
        printf("无法加载过场动画图片: %s\n", IMG_GetError());
        printf("将使用纯色遮罩作为过场动画\n");
    }
}

// 切换到指定地图的过场图片
void change_transition_texture(SDL_Renderer *renderer, int map_index)
{
    // 检查地图索引是否有效
    if (map_index < 0 || map_index >= (int)(sizeof(transition_files) / sizeof(transition_files[0])))
    {
        printf("无效的地图索引: %d\n", map_index);
        return;
    }

    // 清理旧的纹理
    if (transition_state.texture)
    {
        SDL_DestroyTexture(transition_state.texture);
        transition_state.texture = NULL;
    }

    // 加载新的过场图片
    SDL_Surface *surface = IMG_Load(transition_files[map_index]);
    if (surface)
    {
        transition_state.texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (transition_state.texture)
        {
            printf("切换到过场图片: %s\n", transition_files[map_index]);
        }
        else
        {
            printf("创建过场图片纹理失败\n");
        }
    }
    else
    {
        printf("无法加载过场图片: %s\n", IMG_GetError());
    }
}

// 获取当前过场动画状态
TransitionState get_transition_state(void)
{
    return transition_state.state;
}

// 检查是否正在播放过场动画
int is_transition_active(void)
{
    return (transition_state.state != TRANSITION_NONE);
}