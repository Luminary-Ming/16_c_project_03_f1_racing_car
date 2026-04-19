**F1 赛车游戏（C/SDL2 图形化客户端）**

技术栈：C语言 / SDL2 / SDL_image / SDL_mixer / SDL_ttf

- 基于 SDL2 多媒体库开发 2D 赛车游戏，实现窗口管理、渲染管线、事件循环与音效播放，支持键盘实时操控（WASD/方向键）。
- 设计并实现动态地图滚动系统：通过滚动位置偏移与纹理循环拼接，实现无限道路效果；根据玩家得分动态调整滚动速度与敌方车辆生成频率，提升游戏难度曲线。
- 实现敌方车辆对象池（最多 5 辆同时存在），包含碰撞检测（矩形 AABB）、生命值系统（10 点血条，红色方块 UI）、爆炸特效（透明度渐变与随机抖动）与得分累计逻辑。
- 开发过场动画模块（淡入淡出），支持地图切换（共 5 张地图）时展示过渡图片，通过状态机管理 `FADE_OUT` / `FADE_IN` 流程，保证画面平滑切换。
- 实现多首背景音乐循环播放与切换功能（N 键下一首 / B 键上一首），集成 SDL_mixer 管理音乐资源；支持游戏暂停（P 键）时同步暂停音乐，恢复时继续播放。
- 搭建游戏 UI 系统：使用数字纹理图片显示分数，动态绘制生命值方块，利用 TTF 字体库渲染速度倍率文字（随得分提升至 3 倍速），提升信息可读性。
- 优化渲染性能：采用双缓冲纹理（`SDL_TEXTUREACCESS_TARGET`）减少画面撕裂；通过 `SDL_RENDERER_PRESENTVSYNC` 同步帧率，保证 60 FPS 稳定运行。
- 模块化代码设计：分离主页模块（`home.c`）与游戏核心模块（`game.c`），使用全局状态枚举（运行/暂停/结束）管理游戏流程，支持 ESC 返回主页、R 键重新开始等功能。

<img width="1827" height="1046" alt="image" src="https://github.com/user-attachments/assets/5c76132e-b9ea-474a-8057-84037df1ed52" />
<img width="1827" height="1046" alt="F1 赛车截图 2" src="https://github.com/user-attachments/assets/92e8078b-8ba5-4a2e-a9e5-4c8676778b18" />
<img width="1827" height="1046" alt="F1 赛车截图 3" src="https://github.com/user-attachments/assets/0468eb3e-0054-466d-babf-a802130e115d" />
<img width="1827" height="1046" alt="F1 赛车截图 4 png" src="https://github.com/user-attachments/assets/905e3862-0237-4629-a211-e59e9217afac" />
<img width="1252" height="250" alt="bgm" src="https://github.com/user-attachments/assets/f0536f68-8043-4daf-be95-20198aab21af" />
<img width="1399" height="375" alt="car" src="https://github.com/user-attachments/assets/a1661844-7d16-4c73-94cc-92a85e6cac44" />
<img width="1424" height="227" alt="map" src="https://github.com/user-attachments/assets/4864ef3c-147d-4453-a94a-724c09cf2a4e" />
<img width="1020" height="196" alt="home" src="https://github.com/user-attachments/assets/bb573347-d340-4aed-af4f-d883ca3e05cf" />
<img width="1399" height="353" alt="ui" src="https://github.com/user-attachments/assets/13c089ab-6fc5-4513-8ea4-4acf83668541" />
