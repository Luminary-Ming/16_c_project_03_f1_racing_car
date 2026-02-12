#include <stdio.h>
#include "home.h"
#include "game.h"

// 简单的游戏菜单
void show_game_menu(void)
{
    printf("========================================\n");
    printf("        F1 Racing Game - F1 赛车\n");
    printf("========================================\n");
    printf("控制说明:\n");
    printf("  • 点击按钮开始游戏\n");
    printf("  • ESC键退出\n");
    printf("游戏控制:\n");
    printf("  • WASD / 方向键: 控制车辆移动\n");
    printf("  • M: 切换地图（有过场动画）\n");
    printf("  • C: 切换车辆\n");
    printf("  • N: 切换下一首BGM\n");
    printf("  • B: 切换上一首BGM\n");
    printf("  • ESC: 返回主页\n");
    printf("========================================\n\n");
}

int main(void)
{
    int choice = 0;
    int final_score = 0;

    // 显示欢迎信息
    show_game_menu();

    // 初始化主页
    init_home_page();

    // 运行主页循环
    while (1)
    {
        choice = run_home_page();

        switch (choice)
        {
            case BUTTON_START_GAME:
                printf("开始游戏！\n");

                cleanup_home_page(); // 清理主页资源
                final_score = run_game(); // 运行游戏
                printf("游戏结束！最终分数: %d\n", final_score);

                // 游戏结束后重新初始化主页
                init_home_page();
                break;

            case BUTTON_CAR_SELECT:
                printf("车辆选择功能（待实现）\n");
                // TODO: 实现车辆选择界面
                break;

            case BUTTON_TRACK_SELECT:
                printf("赛道选择功能（待实现）\n");
                // TODO: 实现赛道选择界面
                break;

            case BUTTON_EXIT:
                printf("退出游戏\n");
                cleanup_home_page();
                return 0;

            default:
                break;
        }
    }

    return 0;
}