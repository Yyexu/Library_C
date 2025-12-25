#include "books.h"
#include "files.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DATA_FILE "book_library.data"

Book* createBook(int id, const char* name, const char* author, const char* publisher, int price, int amount, const char* isbn, int category_id) {
	// 如果ID小于0或者空就退出
	if (!id || id < 0) {
		return NULL;
	}
	// 检测图书的ID是否重复
	FILE* fp = fopen(DATA_FILE, "rb");
	if (fp != NULL) {
		Book temp;
		while (fread(&temp, sizeof(Book), 1, fp)) {
			if (temp.id == id) { // 找到重复ID
				fclose(fp);
				printf("[系统] 图书ID重复，创建图书失败！\n");
				return NULL;
			}
		}
		fclose(fp);
	}

	// 新建图书
	Book* book = (Book*)malloc(sizeof(Book));
	if (book == NULL) { // 修复C6011空指针引用
		printf("[系统] 内存分配失败，创建图书失败！\n");
		return NULL;
	}
	memset(book, 0, sizeof(Book));

	book->id = id;
	// 修复C4996，使用strcpy_s
	strcpy_s(book->name, sizeof(book->name), name);
	strcpy_s(book->author, sizeof(book->author), author);
	strcpy_s(book->publisher, sizeof(book->publisher), publisher);
	book->price = price;
	book->amount = amount;
	strcpy_s(book->isbn, sizeof(book->isbn), isbn);
	book->category_id = category_id;

	// 添加图书到数据文件data
	add_book(*book); 

	return book;

}

void free_book_ptr(Book* p_book) {
	if (p_book != NULL) { 
		free(p_book);
		p_book = NULL; 
	}
}