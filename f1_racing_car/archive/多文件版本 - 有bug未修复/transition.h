#ifndef TRANSITION_H
#define TRANSITION_H

#include <SDL2/SDL.h>

// 过场动画状态枚举
typedef enum
{
    TRANSITION_NONE,       // 无过场动画
    TRANSITION_FADE_OUT,   // 淡出阶段
    TRANSITION_FADE_IN     // 淡入阶段
} TransitionState;

// 过场动画结构体
typedef struct
{
    TransitionState state;    // 当前状态
    float alpha;             // 透明度 (0.0-1.0)
    float speed;             // 过渡速度
    int duration;            // 持续时间（帧）
    int timer;               // 计时器
    SDL_Texture *texture;    // 纹理指针
} Transition;

// 全局过场动画状态声明
extern Transition transition_state;

// 过场动画管理函数
int init_transition(SDL_Renderer *renderer);
void start_transition(SDL_Renderer *renderer, int next_map);
void update_transition(SDL_Renderer *renderer);
void render_transition(SDL_Renderer *renderer);
void cleanup_transition(void);
void change_transition_texture(SDL_Renderer *renderer, int map_index);
TransitionState get_transition_state(void);
int is_transition_active(void);

#endif