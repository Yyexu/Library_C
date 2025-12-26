#pragma once
#include <stdio.h>

typedef struct {
	// 图书结构体 必须要有id作为主键
	int id;
	char* name;
	char* author;
	char* publisher;
	int price;
	int amount;
	char* isbn;
	int category_id;
}Book;


Book* createBook(int id, const char* name, const char* author, const char* publisher, int price, int amount, const char* isbn, int category_id);
