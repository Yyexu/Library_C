#pragma once
#include <stdbool.h>


// 图书结构体 必须要有id作为主键
typedef struct {
	int id;
	char* name;
	char* author;
	char* publisher;
	int price;
	int amount;
	char* isbn;
	int category_id;
}Book;

// 排序类型
typedef enum SortType {
	SORT_ID = 0, // 默认按 ID 排序
	SORT_PRICE,  // 价格排序
	SORT_AMOUNT  // 库存排序
} SortType;


// 创建一本新书并添加到库中
Book* createBook(int id, const char* name, const char* author, const char* publisher, int price, int amount, const char* isbn, int category_id);

// 保存所有书到文件
void SaveBooksToFile(); 

// 删除一本书
void DeleteBook(Book* book); 

// 对全局书籍数组进行排序
void SortGlobalBooks(SortType type, bool ascending);