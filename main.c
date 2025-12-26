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

    InitWindow(800, 600, "图书管理系统");

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
    char* fileText = LoadFileText("chinese.txt");
    int codepointCount = 0;
    int* codepoints = LoadCodepoints(fileText, &codepointCount);
    Font globalFont = LoadFontEx("SourceHanSansSC-Regular.otf", 48, codepoints, codepointCount);

    // 设置 GUI 样式
    GuiSetFont(globalFont);
    // 按钮文字改小一点，48太大了，按钮会爆
    GuiSetStyle(DEFAULT, TEXT_SIZE, 24);
    GuiSetStyle(DEFAULT, TEXT_SPACING, 2);

    SetTargetFPS(60);

    // 编辑框的状态初始化
    char idBuf[32] = "", priceBuf[32] = "", amountBuf[32] = "", catBuf[32] = "";
    char nameBuf[128] = "", authorBuf[128] = "", pubBuf[128] = "", isbnBuf[64] = "";

    bool editId = false, editName = false, editAuthor = false, editPub = false;
    bool editPrice = false, editAmount = false, editISBN = false, editCat = false;

    bool showSuccessPopup = false; // 成功弹窗开关
    bool showFailPopup = false;    // 失败弹窗开关
    char failMessage[128] = "";    // 用来存具体的报错内容 (比如: "ID不能为空")


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
            DrawTextEx(globalFont, title, (Vector2) { (800 - MeasureTextEx(globalFont, title, 40, 2).x) / 2, 80 }, 40, 2, DARKGREEN);

            float btnX = (800 - 300) / 2;

            // 点击查询 -> 切换页面
            if (GuiButton((Rectangle) { btnX, 180, 300, 50 }, "查询图书")) {
                currentPage = PAGE_SEARCH;
                // 【注意】这里不要写 LoadTexture 和 DrawTexture！
                // 这里只负责切换状态
            }

            if (GuiButton((Rectangle) { btnX, 260, 300, 50 }, "图书列表")) {
                currentPage = PAGE_LIST;
            }

            if (GuiButton((Rectangle) { btnX, 340, 300, 50 }, "添加图书")) {
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
            if (GuiButton((Rectangle) { 580, 20, 200, 50 }, "返回主页")) {
                currentPage = PAGE_HOME;
            }
        }
        // ======================= 列表页面 =======================
        else if (currentPage == PAGE_LIST) {
            DrawTextEx(globalFont, "图书列表 - 暂无数据", (Vector2) { 100, 100 }, 30, 2, GRAY);
            if (GuiButton((Rectangle) { 580, 20, 200, 50 }, "返回主页")) currentPage = PAGE_HOME;
        }


        // ======================= 添加页面 =======================
        else if (currentPage == PAGE_ADD) {

            if (showSuccessPopup || showFailPopup) GuiLock();

            DrawTextEx(globalFont, "添加新书", (Vector2) { 180, 50 }, 30, 2, BLACK);

            const char* labels[8] = { "ID", "书名", "作者", "出版社", "价格", "库存", "ISBN", "分类ID" };

            if (GuiTextBox((Rectangle) { 100, 100, 300, 40 }, idBuf, 31, editId)) {
                editId = !editId;
                if (editId) { editName = false; editAuthor = false; editPub = false; editPrice = false; editAmount = false; editISBN = false; editCat = false; }
            }

            if (GuiTextBox((Rectangle) { 100, 160, 300, 40 }, nameBuf, 127, editName)) {
                editName = !editName;
                if (editName) { editId = false; editAuthor = false; editPub = false; editPrice = false; editAmount = false; editISBN = false; editCat = false; }
            }

            if (GuiTextBox((Rectangle) { 100, 220, 300, 40 }, authorBuf, 127, editAuthor)) {
                editAuthor = !editAuthor;
                if (editAuthor) { editId = false; editName = false; editPub = false; editPrice = false; editAmount = false; editISBN = false; editCat = false; }
            }

            if (GuiTextBox((Rectangle) { 100, 280, 300, 40 }, pubBuf, 127, editPub)) {
                editPub = !editPub;
                if (editPub) { editId = false; editName = false; editAuthor = false; editPrice = false; editAmount = false; editISBN = false; editCat = false; }
            }

            if (GuiTextBox((Rectangle) { 100, 340, 300, 40 }, priceBuf, 31, editPrice)) {
                editPrice = !editPrice;
                if (editPrice) { editId = false; editName = false; editAuthor = false; editPub = false; editAmount = false; editISBN = false; editCat = false; }
            }

            if (GuiTextBox((Rectangle) { 100, 400, 300, 40 }, amountBuf, 31, editAmount)) {
                editAmount = !editAmount;
                if (editAmount) { editId = false; editName = false; editAuthor = false; editPub = false; editPrice = false; editISBN = false; editCat = false; }
            }

            if (GuiTextBox((Rectangle) { 100, 460, 300, 40 }, isbnBuf, 63, editISBN)) {
                editISBN = !editISBN;
                if (editISBN) { editId = false; editName = false; editAuthor = false; editPub = false; editPrice = false; editAmount = false; editCat = false; }
            }

            if (GuiTextBox((Rectangle) { 100, 520, 300, 40 }, catBuf, 31, editCat)) {
                editCat = !editCat;
                if (editCat) { editId = false; editName = false; editAuthor = false; editPub = false; editPrice = false; editAmount = false; editISBN = false; }
            }

            for (int i = 0; i < 8; i++) {
                GuiLabel((Rectangle) { 20, 105 + i * 60, 80, 30 }, labels[i]);
            }

            if (!showSuccessPopup && GuiButton((Rectangle) { 450, 520, 120, 40 }, "提交")) {


                if (atoi(idBuf) == 0) {
                    strcpy(failMessage, "添加失败：ID 不能为 0！");
                    showFailPopup = true; 
                }
                else if (strlen(nameBuf) == 0) {
                    strcpy(failMessage, "添加失败：书名不能为空！");
                    showFailPopup = true;
                }
                else {

                    Book* book = createBook(
                        atoi(idBuf), nameBuf, authorBuf, pubBuf,
                        atoi(priceBuf), atoi(amountBuf), isbnBuf, atoi(catBuf)
                    );

                    if (book != NULL) {

                        free(book->name);
                        free(book->author);
                        free(book->publisher);
                        free(book->isbn);
                        free(book);

                        memset(idBuf, 0, sizeof(idBuf)); memset(nameBuf, 0, sizeof(nameBuf));
                        memset(authorBuf, 0, sizeof(authorBuf)); memset(pubBuf, 0, sizeof(pubBuf));
                        memset(priceBuf, 0, sizeof(priceBuf)); memset(amountBuf, 0, sizeof(amountBuf));
                        memset(isbnBuf, 0, sizeof(isbnBuf)); memset(catBuf, 0, sizeof(catBuf));

                        editId = editName = editAuthor = editPub = editPrice = editAmount = editISBN = editCat = false;

                        showSuccessPopup = true;
                    }
                    else {
                        strcpy(failMessage, "添加失败：请查看后台记录！");
                        showFailPopup = true;
                    }
                }
            }

            GuiUnlock();

            // 3. 【绘制】成功弹窗
            if (showSuccessPopup)
            {
                DrawRectangle(0, 0, 800, 600, Fade(BLACK, 0.5f));
                int result = GuiMessageBox((Rectangle) { 800 / 2 - 150, 600 / 2 - 60, 300, 120 },
                    "提示", "图书添加成功！", "确定");
                if (result >= 0) showSuccessPopup = false;
            }

            // 4. 【新增】失败弹窗
            if (showFailPopup)
            {
                // 画半透明背景
                DrawRectangle(0, 0, 800, 600, Fade(BLACK, 0.5f));

                // 绘制红色边框或标题的弹窗 (这里用标准样式，标题写"错误")
                // 使用 failMessage 变量显示具体的错误原因
                int result = GuiMessageBox((Rectangle) { 800 / 2 - 150, 600 / 2 - 60, 300, 120 },
                    "错误", failMessage, "确定");

                if (result >= 0) showFailPopup = false; // 点击确定关闭
            }

            if (GuiButton((Rectangle) { 580, 20, 200, 50 }, "返回主页")) currentPage = PAGE_HOME;
            }

        EndDrawing();
    }

    // -----------------------------------------------------------
    // 3. 资源释放区
    // -----------------------------------------------------------
    UnloadTexture(bookTex); 
    UnloadCodepoints(codepoints);
    UnloadFont(globalFont);
    CloseWindow();

    return 0;
}