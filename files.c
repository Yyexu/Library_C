#include "files.h"
#include "books.h"
#include <stdio.h>
#include <stdlib.h>
#define DATA_FILE "book_library.data"

// 添加图书
int add_book(const Book* book) {
    FILE* fp = fopen(DATA_FILE, "ab");
    if (!fp) return -1;

    fwrite(&book->id, sizeof(int), 1, fp);
    fwrite(&book->price, sizeof(int), 1, fp);
    fwrite(&book->amount, sizeof(int), 1, fp);
    fwrite(&book->category_id, sizeof(int), 1, fp);

    int len;

    len = strlen(book->name);
    fwrite(&len, sizeof(int), 1, fp);
    fwrite(book->name, 1, len, fp);

    len = strlen(book->author);
    fwrite(&len, sizeof(int), 1, fp);
    fwrite(book->author, 1, len, fp);

    len = strlen(book->publisher);
    fwrite(&len, sizeof(int), 1, fp);
    fwrite(book->publisher, 1, len, fp);

    len = strlen(book->isbn);
    fwrite(&len, sizeof(int), 1, fp);
    fwrite(book->isbn, 1, len, fp);

    fclose(fp);
    printf("[系统] 成功添加图书：%s（ID：%d）\n", book->name, book->id);
    return 0;
}




/*
int query_book(int id, Book* result) {
    FILE* fp = fopen(DATA_FILE, "rb");
    if (!fp) {
        printf("错误：图书库文件不存在！\n");
        return -1;
    }

    Book temp;
    // 循环读取每个结构体，直到找到目标ID
    while (fread(&temp, sizeof(Book), 1, fp)) {
        if (temp.id == id) {
            *result = temp;
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    return -1;
}
*/


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
