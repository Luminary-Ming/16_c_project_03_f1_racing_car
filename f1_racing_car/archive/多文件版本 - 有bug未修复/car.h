#ifndef CAR_H
#define CAR_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// 赛车状态
typedef struct
{
    SDL_Texture *current_texture;
    SDL_Texture **textures;
    int texture_count;
    int current_index;
} CarState;

// 全局赛车状态声明
extern CarState car_state;

// 赛车管理函数
int init_car(SDL_Renderer *renderer);
void change_car_skin(void);
void render_car(SDL_Renderer *renderer, int x, int y);
void cleanup_car(void);

#endif