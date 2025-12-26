#pragma once
#include "books.h"
#include <stdio.h>

// 存储所有书的动态数组
extern Book* globalBookList;
// 当前书的数量
extern int globalBookCount;
// 数组的容量
extern int globalBookCapacity;


// 添加图书到库
int add_book(Book* book);

// 检查图书库文件是否存在，若不存在则创建
void check_file();

// 启动时加载数据
void InitBookSystem();     

// 退出时释放数据
void CloseBookSystem();          

// 极速内存查询
Book** SearchBooksInMemory(int fieldType, const char* keyword, int* outCount);

// 写入字符串到文件
void write_string(FILE* fp, const char* str);

// 读取字符串从文件
char* read_string(FILE* fp);
