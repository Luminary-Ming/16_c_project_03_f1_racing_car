/*
    F1 Racing Car - F1 赛车
*/
#include "game.h"
#include "bgm.h"
#include "car.h"
#include "enemy.h"
#include "rendering.h"
#include "transition.h"
#include "ui.h"

int main(void)
{
    GameState game = { 0 };

    // 初始化SDL和游戏
    if (!init_game(&game))
        return -1;

    // 主游戏循环
    while (game.running)
    {
        handle_events(&game);  // 处理输入事件
        update_game(&game);  // 更新游戏逻辑
        render_game(&game);  // 渲染游戏画面
        SDL_Delay(16);  // 60FPS
    }

    // 清理资源并退出
    cleanup_game(&game);

    printf("游戏结束! 最终分数: %d\n", game.score);

    return 0;
}