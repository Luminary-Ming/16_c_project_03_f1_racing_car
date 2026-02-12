#include "bgm.h"
#include "transition.h"
#include "rendering.h"
#include "car.h"
#include <stdio.h>
#include <stdlib.h>

// 定义全局BGM状态
BGMState bgm_state = { 0 };

// BGM文件列表
static const char *bgm_files[] = {
    "assets/bgm/Insomnia - Craig David.mp3",
    "assets/bgm/Lady Gaga - Poker Face.mp3",
    "assets/bgm/right now-akon.mp3",
    "assets/bgm/本兮,阿悄 - 无限速.mp3",
    "assets/bgm/死一样痛过(MC梦版)-MC몽.mp3"
};

// 静态函数声明
static void change_bgm(int direction);

// 初始化BGM系统
int init_bgm(void)
{
    // 初始化SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("初始化音频失败: %s\n", Mix_GetError());
        return 0;
    }

    // 设置BGM状态
    bgm_state.bgm_list = bgm_files;
    bgm_state.bgm_count = sizeof(bgm_files) / sizeof(bgm_files[0]);
    bgm_state.current_index = 0;
    bgm_state.is_playing = 0;

    // 加载第一首BGM
    bgm_state.current_bgm = Mix_LoadMUS(bgm_files[0]);
    if (!bgm_state.current_bgm)
    {
        printf("加载BGM失败: %s\n", Mix_GetError());
        return 0;
    }

    // 开始播放
    if (Mix_PlayMusic(bgm_state.current_bgm, -1) != -1)
    {
        Mix_VolumeMusic(128);
        bgm_state.is_playing = 1;
    }

    return 1;
}

// 处理键盘事件
void handle_keydown(int key, int game_over)
{
    if (game_over) return;

    switch (key)
    {
        case SDLK_p:  // 暂停/继续
            if (Mix_PausedMusic())
            {
                Mix_ResumeMusic();
            }
            else if (Mix_PlayingMusic())
            {
                Mix_PauseMusic();
            }
            break;

        case SDLK_n:  // 下一首
            change_bgm(1);
            break;

        case SDLK_b:  // 上一首
            change_bgm(-1);
            break;

        case SDLK_m:  // 切换地图
            if (transition_state.state == TRANSITION_NONE)
            {
                int next_map = (current_map_index + 1) % 5;
                start_transition(next_map);
            }
            break;

        case SDLK_c:  // 切换赛车皮肤
            change_car_skin();
            break;
    }
}

// 切换BGM
static void change_bgm(int direction)
{
    if (bgm_state.bgm_count == 0 || !bgm_state.current_bgm) return;

    // 停止当前音乐
    Mix_HaltMusic();
    Mix_FreeMusic(bgm_state.current_bgm);

    // 计算新索引
    bgm_state.current_index = (bgm_state.current_index + direction + bgm_state.bgm_count) % bgm_state.bgm_count;

    // 加载新音乐
    bgm_state.current_bgm = Mix_LoadMUS(bgm_state.bgm_list[bgm_state.current_index]);
    if (bgm_state.current_bgm)
    {
        Mix_PlayMusic(bgm_state.current_bgm, -1);
        Mix_VolumeMusic(128);
    }
}

// 停止BGM
void stop_bgm(void)
{
    if (bgm_state.is_playing)
    {
        Mix_HaltMusic();
        bgm_state.is_playing = 0;
    }
}

// 清理BGM资源
void cleanup_bgm(void)
{
    if (bgm_state.current_bgm)
    {
        Mix_FreeMusic(bgm_state.current_bgm);
        bgm_state.current_bgm = NULL;
    }
    Mix_CloseAudio();
}

// 初始化SDL扩展库
int init_sdl_extensions(void)
{
    // 初始化SDL_image
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG)
    {
        printf("初始化SDL_image失败: %s\n", IMG_GetError());
        return 0;
    }
    return 1;
}

// 清理SDL扩展库
void cleanup_sdl_extensions(void)
{
    IMG_Quit();
}