#include "car.h"
#include <SDL2/SDL_image.h>  // 添加这一行
#include <stdio.h>
#include <stdlib.h>

// 定义全局赛车状态
CarState car_state = { 0 };

// 赛车皮肤文件列表
static const char *car_files[] = {
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

// 初始化赛车系统
int init_car(SDL_Renderer *renderer)
{
    car_state.texture_count = sizeof(car_files) / sizeof(car_files[0]);
    car_state.current_index = 0;

    // 分配纹理数组内存
    car_state.textures = malloc(sizeof(SDL_Texture *) * car_state.texture_count);
    if (!car_state.textures)
    {
        printf("分配赛车纹理内存失败\n");
        return 0;
    }

    // 加载所有赛车纹理
    for (int i = 0; i < car_state.texture_count; i++)
    {
        SDL_Surface *surface = IMG_Load(car_files[i]);
        if (surface)
        {
            car_state.textures[i] = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);

            if (!car_state.textures[i])
            {
                printf("创建赛车纹理 %d 失败: %s\n", i + 1, car_files[i]);
            }
        }
        else
        {
            car_state.textures[i] = NULL;
            printf("无法加载赛车图片 %d: %s - %s\n", i + 1, car_files[i], IMG_GetError());
        }
    }

    // 设置当前纹理
    car_state.current_texture = car_state.textures[0];

    if (!car_state.current_texture)
    {
        printf("警告：默认赛车纹理加载失败\n");
    }

    printf("赛车系统初始化完成，加载了 %d 个赛车皮肤\n", car_state.texture_count);
    return 1;
}

// 切换赛车皮肤
void change_car_skin(void)
{
    if (car_state.texture_count == 0)
    {
        printf("没有可用的赛车皮肤\n");
        return;
    }
    car_state.current_index = (car_state.current_index + 1) % car_state.texture_count;
    car_state.current_texture = car_state.textures[car_state.current_index];
}

// 渲染赛车
void render_car(SDL_Renderer *renderer, int x, int y)
{
    if (car_state.current_texture)
    {
        SDL_Rect dest = { x, y, 70, 130 };
        SDL_RenderCopy(renderer, car_state.current_texture, NULL, &dest);
    }
}

// 清理赛车资源
void cleanup_car(void)
{
    if (car_state.textures)
    {
        for (int i = 0; i < car_state.texture_count; i++)
        {
            if (car_state.textures[i])
            {
                SDL_DestroyTexture(car_state.textures[i]);
            }
        }
        free(car_state.textures);
        car_state.textures = NULL;
    }

    car_state.current_texture = NULL;
    car_state.texture_count = 0;
    car_state.current_index = 0;
}