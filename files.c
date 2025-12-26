#include "files.h"
#include "books.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#define DATA_FILE "book_library.data"

// 全局变量定义
Book* globalBookList = NULL;
int globalBookCount = 0;
int globalBookCapacity = 0;



// 文件检测
void check_file() {
    FILE* fp = fopen(DATA_FILE, "rb");
    if (fp) {
        fclose(fp);
        printf("[系统] 已检测到图书库数据文件\n");
        return;
    }

    fp = fopen(DATA_FILE, "wb");
    if (!fp) {
        printf("[系统] 创建空白数据文件失败！！\n");
        exit(1); 
    }
    fclose(fp);
    printf("[系统] 未检测到图书库文件，已新建空数据文件:%s\n", DATA_FILE);
}

// 读取字符串从文件
char* read_string(FILE* fp) {
    int len = 0;
    if (fread(&len, sizeof(int), 1, fp) != 1) return NULL;
    char* str = (char*)malloc(len + 1);
    if (!str) return NULL;
    fread(str, 1, len, fp);
    str[len] = '\0';
    return str;
}

// 写入字符串到文件
void write_string(FILE* fp, const char* str) {
    int len = strlen(str);
    fwrite(&len, sizeof(int), 1, fp);
    fwrite(str, 1, len, fp);
}

// 初始化系统：把文件一次性读入内存
void InitBookSystem() {
    FILE* fp = fopen(DATA_FILE, "rb");

    // 初始化动态数组
    globalBookCapacity = 100;
    globalBookList = (Book*)malloc(sizeof(Book) * globalBookCapacity);
    globalBookCount = 0;

    if (!fp) return; 

    while (1) {
        Book t;
        if (fread(&t.id, sizeof(int), 1, fp) != 1) break;
        fread(&t.price, sizeof(int), 1, fp);
        fread(&t.amount, sizeof(int), 1, fp);
        fread(&t.category_id, sizeof(int), 1, fp);

        t.name = read_string(fp);
        t.author = read_string(fp);
        t.publisher = read_string(fp);
        t.isbn = read_string(fp);

        if (!t.name || !t.author || !t.publisher || !t.isbn) break;

        // 扩容检测
        if (globalBookCount >= globalBookCapacity) {
            globalBookCapacity *= 2;
            globalBookList = (Book*)realloc(globalBookList, sizeof(Book) * globalBookCapacity);
        }

        // 存入内存
        globalBookList[globalBookCount++] = t;
    }

    fclose(fp);
    printf("[系统] 初始化完成，已加载 %d 本书到内存。\n", globalBookCount);
}

// 关闭系统：释放内存
void CloseBookSystem() {
    if (globalBookList) {
        for (int i = 0; i < globalBookCount; i++) {
            free(globalBookList[i].name);
            free(globalBookList[i].author);
            free(globalBookList[i].publisher);
            free(globalBookList[i].isbn);
        }
        free(globalBookList);
        globalBookList = NULL;
    }
}

int add_book(Book* book) {
    // 写入硬盘 (这步你已经成功了)
    FILE* fp = fopen("book_library.data", "ab"); 
    if (!fp) return -1;

    fwrite(&book->id, sizeof(int), 1, fp);
    fwrite(&book->price, sizeof(int), 1, fp);
    fwrite(&book->amount, sizeof(int), 1, fp);
    fwrite(&book->category_id, sizeof(int), 1, fp);

    int len;
    len = strlen(book->name); fwrite(&len, sizeof(int), 1, fp); fwrite(book->name, 1, len, fp);
    len = strlen(book->author); fwrite(&len, sizeof(int), 1, fp); fwrite(book->author, 1, len, fp);
    len = strlen(book->publisher); fwrite(&len, sizeof(int), 1, fp); fwrite(book->publisher, 1, len, fp);
    len = strlen(book->isbn); fwrite(&len, sizeof(int), 1, fp); fwrite(book->isbn, 1, len, fp);

    fclose(fp);

    // 写入内存（易崩溃）

    // 1. 检查容量并扩容
    if (globalBookCount >= globalBookCapacity) {
        // 如果容量是0，初始化为10；否则翻倍
        int newCapacity = (globalBookCapacity == 0) ? 10 : globalBookCapacity * 2;

        Book* tempList = (Book*)realloc(globalBookList, sizeof(Book) * newCapacity);
        if (tempList == NULL) {
            printf("内存不足，扩容失败！\n");
            return -2; // 内存耗尽，防止崩溃
        }
        globalBookList = tempList;
        globalBookCapacity = newCapacity;
    }

    // 2. 深拷贝数据到全局列表
    // 不要直接赋值指针，必须 malloc + strcpy
    Book* target = &globalBookList[globalBookCount];

    target->id = book->id;
    target->price = book->price;
    target->amount = book->amount;
    target->category_id = book->category_id;

    // 安全复制字符串函数 
    // +1 是为了放 '\0'
    target->name = (char*)malloc(strlen(book->name) + 1);
    if (target->name) strcpy(target->name, book->name);

    target->author = (char*)malloc(strlen(book->author) + 1);
    if (target->author) strcpy(target->author, book->author);

    target->publisher = (char*)malloc(strlen(book->publisher) + 1);
    if (target->publisher) strcpy(target->publisher, book->publisher);

    target->isbn = (char*)malloc(strlen(book->isbn) + 1);
    if (target->isbn) strcpy(target->isbn, book->isbn);

    // 3. 计数器 +1
    globalBookCount++;

    printf("[系统] 数据已更新. 书一共有: %d\n", globalBookCount);

    return 0;
}

// 极速内存查询
Book** SearchBooksInMemory(int fieldType, const char* keyword, int* outCount) {
    // 分配一个指针数组，最大可能也就是 globalBookCount 这么多
    // 注意：这里存的是 Book* (指向全局数组的指针)，而不是 Book 结构体本身
    // 查询速度更快，节省内存
    Book** results = (Book**)malloc(sizeof(Book*) * globalBookCount);
    int count = 0;

    // 如果关键字为空，通常逻辑是返回所有
    bool returnAll = (keyword == NULL || strlen(keyword) == 0);

    for (int i = 0; i < globalBookCount; i++) {
        bool match = false;
        Book* b = &globalBookList[i]; // 获取指针

        if (returnAll) {
            match = true;
        }
        else {
            // 下拉框顺序: 0:ID, 1:书名, 2:ISBN, 3:作者, 4:出版社, 5:分类ID
            switch (fieldType) {
            case 0: // ID (精确)
                if (b->id == atoi(keyword)) match = true;
                break;
            case 1: // 书名 (模糊 strstr)
                if (strstr(b->name, keyword)) match = true;
                break;
            case 2: // ISBN (模糊或精确)
                if (strstr(b->isbn, keyword)) match = true;
                break;
            case 3: // 作者
                if (strstr(b->author, keyword)) match = true;
                break;
            case 4: // 出版社
                if (strstr(b->publisher, keyword)) match = true;
                break;
            case 5: // 分类ID
                if (b->category_id == atoi(keyword)) match = true;
                break;
            }
        }

        if (match) {
            results[count++] = b; // 只是把地址存进去
        }
    }

    *outCount = count;
    // 如果一个都没找到，还是返回分配的指针(count=0)，或者 realloc 缩小
    return results;
}