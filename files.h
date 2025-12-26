#pragma once
#include "books.h"
#include <stdio.h>

// 添加图书到库
int add_book(Book* book);

// 检查图书库文件是否存在，若不存在则创建
void check_file();
