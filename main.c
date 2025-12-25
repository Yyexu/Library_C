#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// 假设这些头文件你都有，没有就注释掉
#include "files.h"
#include "books.h"
#include "utils.h"

typedef enum Page {
    PAGE_HOME,        // 主页
    PAGE_SEARCH,      // 查询页面
    PAGE_LIST,        // 列表页面
    PAGE_ADD,         // 新增页面
} Page;

Page currentPage = PAGE_HOME;

int main() {
    check_file(); // 你的文件检查函数

    InitWindow(500, 600, "图书管理系统");

    // -----------------------------------------------------------
    // 1. 资源加载区 (必须在 while 循环之前！)
    // -----------------------------------------------------------

    // 设置图标 (注意：LoadImage 返回的数据也需要释放)
    Image iconImg = LoadImage("icon.png");
    SetWindowIcon(iconImg);
    UnloadImage(iconImg); // 设置完图标后，内存里的 image 就可以释放了

    // 【关键修改】加载书籍图片 (只加载一次)
    Texture2D bookTex = LoadTexture("book.png");
    // 建议设置为邻近采样，防止放大模糊
    SetTextureFilter(bookTex, TEXTURE_FILTER_POINT);

    // 加载字体
    const char* allChineseChars = "图书管理系统查询列表添加返回主页功能开发中暂无数据确认提交<>";
    int codepointCount = 0;
    int* codepoints = LoadCodepoints(allChineseChars, &codepointCount);
    Font globalFont = LoadFontEx("SourceHanSansSC-Regular.otf", 48, codepoints, codepointCount);

    // 设置 GUI 样式
    GuiSetFont(globalFont);
    // 按钮文字改小一点，48太大了，按钮会爆
    GuiSetStyle(DEFAULT, TEXT_SIZE, 24);
    GuiSetStyle(DEFAULT, TEXT_SPACING, 2);

    SetTargetFPS(60);

    // -----------------------------------------------------------
    // 2. 主循环
    // -----------------------------------------------------------
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // ======================= 主页逻辑 =======================
        if (currentPage == PAGE_HOME) {
            const char* title = "< 图书管理系统 >";
            // 标题用大字号 40
            DrawTextEx(globalFont, title, (Vector2) { (500 - MeasureTextEx(globalFont, title, 40, 2).x) / 2, 80 }, 40, 2, DARKGREEN);

            float btnX = (500 - 200) / 2;

            // 点击查询 -> 切换页面
            if (GuiButton((Rectangle) { btnX, 180, 200, 50 }, "查询图书")) {
                currentPage = PAGE_SEARCH;
                // 【注意】这里不要写 LoadTexture 和 DrawTexture！
                // 这里只负责切换状态
            }

            if (GuiButton((Rectangle) { btnX, 260, 200, 50 }, "图书列表")) {
                currentPage = PAGE_LIST;
            }

            if (GuiButton((Rectangle) { btnX, 340, 200, 50 }, "添加图书")) {
                currentPage = PAGE_ADD;
            }
        }
        // ======================= 查询页面 =======================
        else if (currentPage == PAGE_SEARCH) {
            DrawTextEx(globalFont, "查询页面 - 功能开发中", (Vector2) { 50, 50 }, 30, 2, GRAY);

            // -----------------------------------------------------------
            // 【关键修改】在这里持续绘制图片
            // 只要 currentPage 是 SEARCH，这一段每秒都会执行 60 次，图片才会一直显示
            // -----------------------------------------------------------
            if (bookTex.id != 0) { // 简单检查图片是否加载成功
                Rectangle sourceRec = { 0.0f, 0.0f, (float)bookTex.width, (float)bookTex.height };
                // 把图片画在中间偏下的位置
                Rectangle destRec = { 150.0f, 120.0f, 200.0f, 200.0f };
                Vector2 origin = { 0.0f, 0.0f };

                DrawTexturePro(bookTex, sourceRec, destRec, origin, 0.0f, WHITE);
            }

            // 返回按钮
            if (GuiButton((Rectangle) { 150, 400, 200, 50 }, "返回主页")) {
                currentPage = PAGE_HOME;
            }
        }
        // ======================= 列表页面 =======================
        else if (currentPage == PAGE_LIST) {
            DrawTextEx(globalFont, "图书列表 - 暂无数据", (Vector2) { 100, 100 }, 30, 2, GRAY);
            if (GuiButton((Rectangle) { 150, 400, 200, 50 }, "返回主页")) currentPage = PAGE_HOME;
        }
        // ======================= 添加页面 =======================
        else if (currentPage == PAGE_ADD) {
            DrawTextEx(globalFont, "添加新书", (Vector2) { 180, 50 }, 30, 2, BLACK);
            GuiLabel((Rectangle) { 100, 150, 100, 30 }, "确认提交?");
            if (GuiButton((Rectangle) { 150, 400, 200, 50 }, "返回主页")) currentPage = PAGE_HOME;
        }

        EndDrawing();
    }

    // -----------------------------------------------------------
    // 3. 资源释放区
    // -----------------------------------------------------------
    UnloadTexture(bookTex); // 别忘了卸载图片！
    UnloadCodepoints(codepoints);
    UnloadFont(globalFont);
    CloseWindow();

    return 0;
}