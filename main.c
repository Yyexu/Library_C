#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h" 
#include "files.h"
#include "books.h" 
#include "utils.h"


// 状态机
typedef enum Page {
    PAGE_HOME,        // 主页
    PAGE_SEARCH,      // 查询页面
    PAGE_LIST,        // 列表页面
    PAGE_ADD,         // 新增页面
    PAGE_DETAIL,
} Page;

// 当前页面
Page currentPage = PAGE_HOME;



// 全局变量：用于存储查询结果
Book** searchResults = NULL;
// 当前选中的图书
Book* selectedBook = NULL;
// 查询结果数量
int searchResultCount = 0;
// 列表页面滚动位置
static int listScrollIndex = 0;

int main() {
    // 检查文件是否完全
    check_file();
    // 初始化数据，将图书加载进内存
    InitBookSystem();
    // 窗口初始化
    InitWindow(800, 600, "图书管理系统");

	// 设置图标
    Image iconImg = LoadImage("icon.png");
    SetWindowIcon(iconImg);
    UnloadImage(iconImg);
    // 绿书图片预加载
    Texture2D bookTex = LoadTexture("book.png");
    SetTextureFilter(bookTex, TEXTURE_FILTER_POINT);

	// 中文字体加载，使用chinese.txt作为字符集，可以手动加识别为？的字符
    char* fileText = LoadFileText("chinese.txt");
    int codepointCount = 0;
    int* codepoints = LoadCodepoints(fileText, &codepointCount);
    Font globalFont = LoadFontEx("SourceHanSansSC-Regular.otf", 48, codepoints, codepointCount);
    // 字体风格大小
    GuiSetFont(globalFont);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 24);
    GuiSetStyle(DEFAULT, TEXT_SPACING, 2);
	// 设置帧率
    SetTargetFPS(60);

	// 输入框缓冲区和编辑状态
	// -----------------------------------------------------------
    char idBuf[32] = "", priceBuf[32] = "", amountBuf[32] = "", catBuf[32] = "";
    char nameBuf[128] = "", authorBuf[128] = "", pubBuf[128] = "", isbnBuf[64] = "";
    // 修改的一些状态
    bool editId = false, editName = false, editAuthor = false, editPub = false;
    bool editPrice = false, editAmount = false, editISBN = false, editCat = false;
	// 操作结果弹窗状态
    bool showSuccessPopup = false;
    bool showFailPopup = false;
    char failMessage[128] = "";
	// 下拉框选择的字段
    static int selectedField = 0;
	// 下拉框激活状态
    static bool dropdownActive = false;
	// 输入框编辑状态
    static bool editQuery = false;
	// 查询输入缓冲区
    static char queryBuf[128] = "";
    // 是否处于编辑模式
    bool isDetailEditing = false; 
    // 是否显示删除确认框
    bool showDeleteConfirm = false;
    // -----------------------------------------------------------



    // 整个系统主循环
    while (!WindowShouldClose()) {
		// 绘制开始
        BeginDrawing();
        // 清屏
        ClearBackground(RAYWHITE);

        // ======================= 主页逻辑 =======================
        if (currentPage == PAGE_HOME) {
            // 绘制标题
            const char* title = "< 图书管理系统 >";
            DrawTextEx(globalFont, title, (Vector2) { (800 - MeasureTextEx(globalFont, title, 40, 2).x) / 2, 80 }, 40, 2, DARKGREEN);
            
            float btnX = (800 - 300) / 2;
            // 绘制按钮
            if (GuiButton((Rectangle) { btnX, 180, 300, 50 }, "查询图书")) currentPage = PAGE_SEARCH;
            if (GuiButton((Rectangle) { btnX, 260, 300, 50 }, "图书列表")) currentPage = PAGE_LIST;
            if (GuiButton((Rectangle) { btnX, 340, 300, 50 }, "添加图书")) currentPage = PAGE_ADD;
        }



        // ======================= 查询页面 =======================
        else if (currentPage == PAGE_SEARCH) {
            // 滚动位置
            static int searchScrollIndex = 0;
			// 绘制标题
            DrawTextEx(globalFont, "查询页面", (Vector2) { 50, 40 }, 60, 2, DARKGREEN);

            // 左侧的图书显示
            if (bookTex.id != 0) {
                DrawTexturePro(bookTex,
                    (Rectangle) {
                    0, 0, (float)bookTex.width, (float)bookTex.height
                },
                    (Rectangle) {
                    30, 110, 160, 160
                },
                    (Vector2) {
                    0, 0
                }, 0, WHITE);
            }

            // ----------------------------------------------------------------
            // 1. 搜索控制区
            // ----------------------------------------------------------------

            // 输入框 (防止下拉框遮挡逻辑)
            if (GuiTextBox((Rectangle) { 300, 200, 300, 40 }, queryBuf, 127, editQuery)) {
                if (!dropdownActive) editQuery = !editQuery;
            }

			// 如果正在编辑输入框，关闭下拉框
            if (editQuery) dropdownActive = false;

            // 下拉框 (放在输入框上方)
            if (!editQuery) {
                if (GuiDropdownBox((Rectangle) { 300, 120, 200, 40 },
                    "ID;书名;ISBN;作者;出版社;分类ID",
                    & selectedField, dropdownActive))
                {
                    dropdownActive = !dropdownActive;
                }
            }

            // 显示当前选择
            const char* fieldNames[6] = { "ID", "书名", "ISBN", "作者", "出版社", "分类ID" };
            int safeIndex = (selectedField >= 0 && selectedField < 6) ? selectedField : 0;
            DrawTextEx(globalFont, TextFormat("当前搜索按: %s", fieldNames[safeIndex]),
                (Vector2) {
                550, 130
            }, 24, 2, BLACK);

            // 查询按钮
            if (!editQuery && GuiButton((Rectangle) { 620, 200, 100, 40 }, "查询")) {
                // 1. 释放旧结果
                if (searchResults) free(searchResults);

                // 2. 内存极速查询
                searchResults = SearchBooksInMemory(selectedField, queryBuf, &searchResultCount);

                // 3. 重置滚动条
                searchScrollIndex = 0;

                printf("[查询] 找到 %d 条结果\n", searchResultCount);
            }

            // ----------------------------------------------------------------
            // 2. 搜索结果列表区 
            // ----------------------------------------------------------------

            int listX = 220;    // 列表起始X
            int listY = 280;    // 列表起始Y
            int listW = 550;    // 列表宽度
            int listH = 280;    // 列表高度
            int rowH = 35;      // 每一行的高度
            int itemsPerPage = 7; // 一页能显示几行

            // 画背景框
            DrawRectangleLines(listX - 1, listY - 1, listW + 2, listH + 2, LIGHTGRAY);
            DrawRectangle(listX, listY, listW, listH, Fade(WHITE, 0.8f));

            // 画表头
            DrawRectangle(listX, listY, listW, 30, LIGHTGRAY);
            DrawTextEx(globalFont, "ID", (Vector2) { listX + 10, listY + 5 }, 20, 1, DARKGRAY);
            DrawTextEx(globalFont, "书名", (Vector2) { listX + 60, listY + 5 }, 20, 1, DARKGRAY);
            DrawTextEx(globalFont, "作者", (Vector2) { listX + 300, listY + 5 }, 20, 1, DARKGRAY);
            DrawTextEx(globalFont, "价格", (Vector2) { listX + 460, listY + 5 }, 20, 1, DARKGRAY);

            // --- 列表内容绘制 ---
            if (searchResultCount == 0) {
                DrawTextEx(globalFont, "没有找到相关图书", (Vector2) { listX + 180, listY + 100 }, 24, 1, GRAY);
            }
            else {
                // 1. 处理鼠标滚轮
                if (CheckCollisionPointRec(GetMousePosition(), (Rectangle) { (float)listX, (float)listY, (float)listW, (float)listH })) {
                    int wheel = GetMouseWheelMove();
                    if (wheel != 0) searchScrollIndex -= wheel;
                }

                // 2. 限制滚动范围
                int maxIndex = searchResultCount - itemsPerPage;
                if (maxIndex < 0) maxIndex = 0;
                if (searchScrollIndex < 0) searchScrollIndex = 0;
                if (searchScrollIndex > maxIndex) searchScrollIndex = maxIndex;

                // 3. 循环绘制可见行
                int startDrawY = listY + 30; // 跳过表头

                for (int i = 0; i < itemsPerPage; i++) {
                    // 计算当前行的真实索引
                    int actualIndex = searchScrollIndex + i;
                    if (actualIndex >= searchResultCount) break;

                    // 获取当前书的指针
                    Book* b = searchResults[actualIndex];

                    // 计算当前行的 Y 坐标
                    int currentY = startDrawY + (i * rowH);

                    // 定义这一行的矩形区域
                    Rectangle rowRect = { (float)listX, (float)currentY, (float)listW, (float)rowH };

                    // 隐形按钮逻辑 (点击跳转)
                    if (GuiButton(rowRect, "")) {
                        selectedBook = b;           // 记录当前选中的书
                        currentPage = PAGE_DETAIL;  // 跳转到详情页
                        isDetailEditing = false;    // 默认进入查看模式

                        memset(nameBuf, 0, sizeof(nameBuf));
                    }

                    // --- 绘制行内容 (画在按钮上面) ---

                    // 斑马纹背景
                    if (i % 2 == 0) DrawRectangleRec(rowRect, Fade(SKYBLUE, 0.1f));
                    else DrawRectangleRec(rowRect, WHITE);

                    // 鼠标悬停高亮
                    if (CheckCollisionPointRec(GetMousePosition(), rowRect)) {
                        DrawRectangleRec(rowRect, Fade(ORANGE, 0.2f));
                    }

                    // 绘制文字
                    char idStr[16], priceStr[32];
                    sprintf(idStr, "%d", b->id);
                    sprintf(priceStr, "CNY %d", b->price);

                    DrawTextEx(globalFont, idStr, (Vector2) { listX + 10, currentY + 8 }, 20, 1, BLACK);

                    // 书名截断
                    if (MeasureTextEx(globalFont, b->name, 20, 1).x > 230) {
                        DrawTextEx(globalFont, TextFormat("%.14s...", b->name), (Vector2) { listX + 60, currentY + 8 }, 20, 1, DARKBLUE);
                    }
                    else {
                        DrawTextEx(globalFont, b->name, (Vector2) { listX + 60, currentY + 8 }, 20, 1, DARKBLUE);
                    }

                    DrawTextEx(globalFont, b->author, (Vector2) { listX + 300, currentY + 8 }, 20, 1, BLACK);
                    DrawTextEx(globalFont, priceStr, (Vector2) { listX + 460, currentY + 8 }, 20, 1, MAROON);
                }

                // 4. 滚动条指示器
                if (searchResultCount > itemsPerPage) {
                    float scrollPercent = (float)searchScrollIndex / (float)maxIndex;
                    int barAreaH = listH - 30;
                    int barH = 40;
                    int barY = startDrawY + (int)(scrollPercent * (barAreaH - barH));
                    DrawRectangle(listX + listW - 6, barY, 4, barH, Fade(GRAY, 0.6f));
                }
            }

            // 返回按钮
            if (GuiButton((Rectangle) { 620, 20, 150, 40 }, "返回主页")) {
                currentPage = PAGE_HOME;
                dropdownActive = false;
                editQuery = false;
                if (searchResults) { free(searchResults); searchResults = NULL; searchResultCount = 0; }
            }
        }


        // ======================= 列表页面 =======================
        else if (currentPage == PAGE_LIST) {

            // 状态变量
            static int sortSelected = 0;
            static bool sortEdit = false;
            static bool sortAscending = true;
            static bool needSort = false;

            // -------------------------------------------------------------
            // 第一层：顶部栏 (标题 + 返回按钮)
            // -------------------------------------------------------------
            // 标题
            DrawTextEx(globalFont, "图书总览", (Vector2) { 40, 30 }, 40, 2, DARKGREEN);

            // 返回按钮 
            if (GuiButton((Rectangle) { 640, 30, 120, 40 }, "返回主页")) {
                currentPage = PAGE_HOME;
                sortEdit = false;
            }

            // -------------------------------------------------------------
            // 第二层：工具栏 (统计信息 + 排序控件)
            // -------------------------------------------------------------
            float toolBarY = 85; // 工具栏高度基准

            // 1. 统计信息 (左侧)
            char countText[64];
            sprintf(countText, "共 %d 本藏书", globalBookCount);
            DrawTextEx(globalFont, countText, (Vector2) { 40, toolBarY + 5 }, 24, 1, GRAY);

            // 2. 排序控件 (右侧)
            float sortControlsRightX = 760; 

            // 升序/降序 按钮
            const char* dirText = sortAscending ? "升序 #" : "降序 #"; // #图标
            if (GuiButton((Rectangle) { sortControlsRightX - 80, toolBarY, 80, 30 }, dirText)) {
                sortAscending = !sortAscending;
                needSort = true;
            }

            // 排序标签
            DrawTextEx(globalFont, "排序:", (Vector2) { sortControlsRightX - 260, toolBarY + 5 }, 20, 1, BLACK);

            // 下拉框的位置
            Rectangle dropdownRect = { sortControlsRightX - 210, toolBarY, 120, 30 };

            // -------------------------------------------------------------
            // 第三层：表格区域
            // -------------------------------------------------------------
            int startY = 130; // 表格起始Y坐标下移，给上面留空间

            // 表头背景
            DrawRectangle(40, startY, 720, 30, LIGHTGRAY);

            // 表头文字 (调整了一下间距)
            DrawTextEx(globalFont, "ID", (Vector2) { 50, startY + 5 }, 20, 1, DARKGRAY);
            DrawTextEx(globalFont, "书名", (Vector2) { 120, startY + 5 }, 20, 1, DARKGRAY);
            DrawTextEx(globalFont, "作者", (Vector2) { 400, startY + 5 }, 20, 1, DARKGRAY);
            DrawTextEx(globalFont, "价格", (Vector2) { 580, startY + 5 }, 20, 1, DARKGRAY);
            DrawTextEx(globalFont, "库存", (Vector2) { 680, startY + 5 }, 20, 1, DARKGRAY);

            // --- 列表内容循环 ---
            // 滚轮逻辑
            int wheel = GetMouseWheelMove();
            if (wheel != 0) listScrollIndex -= wheel;

            if (listScrollIndex < 0) listScrollIndex = 0;
            int maxIndex = globalBookCount - 8;
            if (maxIndex < 0) maxIndex = 0;
            if (listScrollIndex > maxIndex) listScrollIndex = maxIndex;

            int itemHeight = 45;

            int itemsPerPage = 8; // 一页显示数量

            for (int i = 0; i < itemsPerPage; i++) {
                int actualIndex = listScrollIndex + i;
                if (actualIndex >= globalBookCount) break;

                Book* b = &globalBookList[actualIndex];

                int yPos = startY + 35 + (i * itemHeight); // +35 是跳过表头
                Rectangle rowRect = { 40, (float)yPos, 720, (float)itemHeight - 5 };

                // 隐形按钮
                if (GuiButton(rowRect, "")) {
                    selectedBook = b;
                    currentPage = PAGE_DETAIL;
                    isDetailEditing = false;
                    memset(nameBuf, 0, sizeof(nameBuf));
                }

                // 背景
                if (i % 2 == 0) DrawRectangleRec(rowRect, Fade(SKYBLUE, 0.1f));
                else DrawRectangleRec(rowRect, Fade(WHITE, 0.5f));

                // 悬停高亮
                if (CheckCollisionPointRec(GetMousePosition(), rowRect)) {
                    DrawRectangleRec(rowRect, Fade(ORANGE, 0.2f));
                }

                // 文字
                char idStr[16], priceStr[32], amtStr[16];
                sprintf(idStr, "%d", b->id);
                sprintf(priceStr, "CNY %d", b->price);
                sprintf(amtStr, "%d", b->amount);

                DrawTextEx(globalFont, idStr, (Vector2) { 50, yPos + 10 }, 20, 1, BLACK);

                // 书名截断
                if (MeasureTextEx(globalFont, b->name, 20, 1).x > 250) {
                    DrawTextEx(globalFont, TextFormat("%.12s...", b->name), (Vector2) { 120, yPos + 10 }, 20, 1, DARKBLUE);
                }
                else {
                    DrawTextEx(globalFont, b->name, (Vector2) { 120, yPos + 10 }, 20, 1, DARKBLUE);
                }

                DrawTextEx(globalFont, b->author, (Vector2) { 400, yPos + 10 }, 20, 1, BLACK);
                DrawTextEx(globalFont, priceStr, (Vector2) { 580, yPos + 10 }, 20, 1, MAROON);
                DrawTextEx(globalFont, amtStr, (Vector2) { 680, yPos + 10 }, 20, 1, BLACK);
            }

            // 滚动条
            if (globalBookCount > itemsPerPage) {
                float scrollPercent = (float)listScrollIndex / (float)maxIndex;
                // 滚动条高度要根据列表区域调整
                int barTotalHeight = itemsPerPage * itemHeight;
                DrawRectangle(770, startY + 35, 6, barTotalHeight, Fade(GRAY, 0.3f));
                DrawRectangle(770, startY + 35 + (int)(scrollPercent * (barTotalHeight - 40)), 6, 40, DARKGRAY);
            }

            // -------------------------------------------------------------
            // 最后绘制下拉框 
            // -------------------------------------------------------------
            int oldSelection = sortSelected;
            if (GuiDropdownBox(dropdownRect, "按 ID;按 价格;按 库存", &sortSelected, sortEdit)) {
                sortEdit = !sortEdit;
            }

            // 触发排序
            if (oldSelection != sortSelected) needSort = true;
            if (needSort) {
                SortGlobalBooks(sortSelected, sortAscending);
                needSort = false;
            }
        }


        // ======================= 添加页面 =======================
        else if (currentPage == PAGE_ADD) {

            if (showSuccessPopup || showFailPopup) GuiLock();

            DrawTextEx(globalFont, "添加新书", (Vector2) { 180, 50 }, 30, 2, BLACK);

			// 输入框标签
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

            // 绘制标签
            for (int i = 0; i < 8; i++) {
                GuiLabel((Rectangle) { 20, 105 + i * 60, 80, 30 }, labels[i]);
            }

            // --- 提交按钮逻辑 ---
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
                    // 1. 构造一个临时的 Book 对象 (在栈上，不需要 malloc)
                    Book tempBook;
                    tempBook.id = atoi(idBuf);
                    tempBook.price = atoi(priceBuf);
                    tempBook.amount = atoi(amountBuf);
                    tempBook.category_id = atoi(catBuf);

                    // 2.指向缓冲区即可，AddBook 内部会进行深拷贝 (strdup)
                    tempBook.name = nameBuf;
                    tempBook.author = authorBuf;
                    tempBook.publisher = pubBuf;
                    tempBook.isbn = isbnBuf;

                    // 3. 调用add_book,同时写入文件和内存
                    if (add_book(&tempBook) == 0) {
                        // 清空输入框
                        memset(idBuf, 0, sizeof(idBuf)); memset(nameBuf, 0, sizeof(nameBuf));
                        memset(authorBuf, 0, sizeof(authorBuf)); memset(pubBuf, 0, sizeof(pubBuf));
                        memset(priceBuf, 0, sizeof(priceBuf)); memset(amountBuf, 0, sizeof(amountBuf));
                        memset(isbnBuf, 0, sizeof(isbnBuf)); memset(catBuf, 0, sizeof(catBuf));
                        editId = editName = editAuthor = editPub = editPrice = editAmount = editISBN = editCat = false;

                        showSuccessPopup = true;
                    }
                    else {
                        strcpy(failMessage, "添加失败：写入错误！");
                        showFailPopup = true;
                    }
                }
            }

			// 解锁背景
            GuiUnlock();

            // 弹窗逻辑
            if (showSuccessPopup) {
                DrawRectangle(0, 0, 800, 600, Fade(BLACK, 0.5f));
                if (GuiMessageBox((Rectangle) { 800 / 2 - 150, 600 / 2 - 60, 300, 120 }, "提示", "图书添加成功！", "确定") >= 0)
                    showSuccessPopup = false;
            }
            if (showFailPopup) {
                DrawRectangle(0, 0, 800, 600, Fade(BLACK, 0.5f));
                if (GuiMessageBox((Rectangle) { 800 / 2 - 150, 600 / 2 - 60, 300, 120 }, "错误", failMessage, "确定") >= 0)
                    showFailPopup = false;
            }

            if (GuiButton((Rectangle) { 580, 20, 200, 50 }, "返回主页")) currentPage = PAGE_HOME;
        }


		// ======================= 书的详情页面 =======================
        else if (currentPage == PAGE_DETAIL) {

            // 异常保护：如果没有选中的书，直接回首页
            if (selectedBook == NULL) {
                currentPage = PAGE_LIST;
            }
            else {
                // 如果有弹窗，锁定背景
                if (showDeleteConfirm) GuiLock();

                DrawTextEx(globalFont, "图书详情", (Vector2) { 50, 40 }, 40, 2, DARKGREEN);

                // 左侧封面
                if (bookTex.id != 0) {
                    DrawTexturePro(bookTex, (Rectangle) { 0, 0, (float)bookTex.width, (float)bookTex.height },
                        (Rectangle) {
                        50, 110, 200, 200
                    }, (Vector2) { 0, 0 }, 0, WHITE);
                }

                // 定义布局坐标
                int startX = 300;
                int startY = 110;
                int gap = 50;

                // -------------------------------------------------------------
                // 模式 A: 查看模式 (View Mode)
                // -------------------------------------------------------------
                if (!isDetailEditing) {
                    // 显示信息文本
                    DrawTextEx(globalFont, TextFormat("ID: %d", selectedBook->id), (Vector2) { startX, startY }, 30, 2, GRAY);

                    DrawTextEx(globalFont, TextFormat("书名: %s", selectedBook->name), (Vector2) { startX, startY + gap * 1 }, 24, 2, BLACK);
                    DrawTextEx(globalFont, TextFormat("作者: %s", selectedBook->author), (Vector2) { startX, startY + gap * 2 }, 24, 2, BLACK);
                    DrawTextEx(globalFont, TextFormat("出版社: %s", selectedBook->publisher), (Vector2) { startX, startY + gap * 3 }, 24, 2, BLACK);
                    DrawTextEx(globalFont, TextFormat("ISBN: %s", selectedBook->isbn), (Vector2) { startX, startY + gap * 4 }, 24, 2, BLACK);
                    DrawTextEx(globalFont, TextFormat("价格: CNY %d", selectedBook->price), (Vector2) { startX, startY + gap * 5 }, 24, 2, MAROON);
                    DrawTextEx(globalFont, TextFormat("库存: %d", selectedBook->amount), (Vector2) { startX, startY + gap * 6 }, 24, 2, BLACK);

                    // --- 按钮组 ---

                    // [修改按钮]
                    if (GuiButton((Rectangle) { startX, 500, 120, 40 }, "修改信息")) {
                        isDetailEditing = true; // 进入编辑模式

                        // 把当前书的数据 复制到 输入缓冲区
                        strcpy(nameBuf, selectedBook->name);
                        strcpy(authorBuf, selectedBook->author);
                        strcpy(pubBuf, selectedBook->publisher);
                        strcpy(isbnBuf, selectedBook->isbn);
                        sprintf(priceBuf, "%d", selectedBook->price);
                        sprintf(amountBuf, "%d", selectedBook->amount);

                        // 重置编辑状态
                        editName = editAuthor = editPub = editISBN = editPrice = editAmount = false;
                    }

                    // [删除按钮] (红色)
                    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0xffadadff);
                    if (GuiButton((Rectangle) { startX + 140, 500, 120, 40 }, "删除图书")) {
                        showDeleteConfirm = true; // 弹出确认框
                    }
                    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0xf5f5f5ff); // 恢复颜色

                    // [返回按钮]
                    if (GuiButton((Rectangle) { 580, 20, 200, 50 }, "返回列表")) {
                        currentPage = PAGE_LIST;
                        selectedBook = NULL;
                    }
                }
                // -------------------------------------------------------------
                // 模式 B: 编辑模式 (Edit Mode)
                // -------------------------------------------------------------
                else {
                    DrawTextEx(globalFont, TextFormat("ID: %d (不可修改)", selectedBook->id), (Vector2) { startX, startY }, 30, 2, LIGHTGRAY);

                    // 绘制输入框
                    GuiLabel((Rectangle) { startX - 60, startY + gap * 1, 80, 30 }, "书名");
                    if (GuiTextBox((Rectangle) { startX + 20, startY + gap * 1, 300, 35 }, nameBuf, 127, editName)) editName = !editName;

                    GuiLabel((Rectangle) { startX - 60, startY + gap * 2, 80, 30 }, "作者");
                    if (GuiTextBox((Rectangle) { startX + 20, startY + gap * 2, 300, 35 }, authorBuf, 127, editAuthor)) editAuthor = !editAuthor;

                    GuiLabel((Rectangle) { startX - 60, startY + gap * 3, 80, 30 }, "出版社");
                    if (GuiTextBox((Rectangle) { startX + 20, startY + gap * 3, 300, 35 }, pubBuf, 127, editPub)) editPub = !editPub;

                    GuiLabel((Rectangle) { startX - 60, startY + gap * 4, 80, 30 }, "ISBN");
                    if (GuiTextBox((Rectangle) { startX + 20, startY + gap * 4, 300, 35 }, isbnBuf, 127, editISBN)) editISBN = !editISBN;

                    GuiLabel((Rectangle) { startX - 60, startY + gap * 5, 80, 30 }, "价格");
                    if (GuiTextBox((Rectangle) { startX + 20, startY + gap * 5, 100, 35 }, priceBuf, 31, editPrice)) editPrice = !editPrice;

                    GuiLabel((Rectangle) { startX - 60, startY + gap * 6, 80, 30 }, "库存");
                    if (GuiTextBox((Rectangle) { startX + 20, startY + gap * 6, 100, 35 }, amountBuf, 31, editAmount)) editAmount = !editAmount;

                    // [保存按钮]
                    if (GuiButton((Rectangle) { startX, 500, 120, 40 }, "保存修改")) {
                        // 1. 更新内存 (先释放旧字符串，再分配新的)
                        if (selectedBook->name) free(selectedBook->name);
                        selectedBook->name = (char*)malloc(strlen(nameBuf) + 1);
                        strcpy(selectedBook->name, nameBuf);

                        if (selectedBook->author) free(selectedBook->author);
                        selectedBook->author = (char*)malloc(strlen(authorBuf) + 1);
                        strcpy(selectedBook->author, authorBuf);

                        if (selectedBook->publisher) free(selectedBook->publisher);
                        selectedBook->publisher = (char*)malloc(strlen(pubBuf) + 1);
                        strcpy(selectedBook->publisher, pubBuf);

                        if (selectedBook->isbn) free(selectedBook->isbn);
                        selectedBook->isbn = (char*)malloc(strlen(isbnBuf) + 1);
                        strcpy(selectedBook->isbn, isbnBuf);

                        selectedBook->price = atoi(priceBuf);
                        selectedBook->amount = atoi(amountBuf);

                        // 2. 把整个内存数组重新覆盖写入文件
                        SaveBooksToFile();

                        isDetailEditing = false; // 退出编辑模式
                    }

                    // [取消按钮]
                    if (GuiButton((Rectangle) { startX + 140, 500, 120, 40 }, "取消")) {
                        isDetailEditing = false; // 直接退出，不保存
                    }
                }

                GuiUnlock();

                // -------------------------------------------------------------
                // 删除确认弹窗
                // -------------------------------------------------------------
                if (showDeleteConfirm) {
                    DrawRectangle(0, 0, 800, 600, Fade(BLACK, 0.5f));
                    int result = GuiMessageBox((Rectangle) { 800 / 2 - 150, 600 / 2 - 60, 300, 120 },
                        "警告", "确定要永久删除这本书吗？", "确定;取消");

                    if (result == 1) { // 点击了确定
                        DeleteBook(selectedBook); // 调用 books.c 里的删除函数

                        selectedBook = NULL;       // 既然删了，指针就无效了
                        showDeleteConfirm = false;

                        // 删完后返回列表页
                        currentPage = PAGE_LIST;

                        // 如果之前有搜索结果，最好清空，防止里面有野指针
                        if (searchResults) { 
                            free(searchResults); 
                            searchResults = NULL; searchResultCount = 0; 
                        }
                    }
                    else if (result == 0 || result == 2) { // 点击了关闭或取消
                        showDeleteConfirm = false;
                    }
                }
            }
            }

        EndDrawing();
    }

    // -----------------------------------------------------------
    // 3. 资源释放区
    // -----------------------------------------------------------
    // 释放查询结果数组
    if (searchResults) free(searchResults);

    // 系统关闭，释放全局内存库
    CloseBookSystem();
    // 卸载资源
    UnloadTexture(bookTex);
    UnloadCodepoints(codepoints);
    UnloadFont(globalFont);

    // 关闭窗口
    CloseWindow();

    return 0;
}