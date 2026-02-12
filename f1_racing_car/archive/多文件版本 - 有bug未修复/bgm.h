#ifndef BGM_H
#define BGM_H

#include <SDL2/SDL_mixer.h>

// 背景音乐状态
typedef struct
{
    Mix_Music *current_bgm;
    const char **bgm_list;
    int bgm_count;
    int current_index;
    int is_playing;
} BGMState;

// 全局BGM状态声明
extern BGMState bgm_state;

// BGM管理函数
int init_bgm(void);
void handle_keydown(int key, int game_over);
void stop_bgm(void);
void cleanup_bgm(void);
int init_sdl_extensions(void);
void cleanup_sdl_extensions(void);
void change_car_skin(void);
void start_transition(int next_map);

#endif