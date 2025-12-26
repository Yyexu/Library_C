#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "books.h"
#include "files.h"
#define DATA_FILE "book_library.data"

Book* createBook(int id, const char* name, const char* author, const char* publisher,
    int price, int amount, const char* isbn, int category_id) {

    if (id <= 0) {
        printf("[系统] ID非法，创建失败\n");
        return NULL;
    }
        
    FILE* fp = fopen(DATA_FILE, "rb");
    if (fp) {
        int tempId;
        while (fread(&tempId, sizeof(int), 1, fp) == 1) {
            if (tempId == id) {
                fclose(fp);
                printf("[系统] 图书ID重复，创建失败\n");
                return NULL;
            }
            fseek(fp, sizeof(Book) - sizeof(int), SEEK_CUR);
        }
        fclose(fp);
    }

    Book* book = malloc(sizeof(Book));
    if (!book) return NULL;
    memset(book, 0, sizeof(Book));

    book->id = id;
    book->price = price;
    book->amount = amount;
    book->category_id = category_id;

    book->name = _strdup(name);
    book->author = _strdup(author);
    book->publisher = _strdup(publisher);
    book->isbn = _strdup(isbn);

    if (!book->name || !book->author || !book->publisher || !book->isbn) {
        printf("[系统] 字符串内存分配失败\n");
        free(book);
        return NULL;
    }

    if (add_book(book) != 0) {
        free(book->name); free(book->author);
        free(book->publisher); free(book->isbn);
        free(book);
        return NULL;
    }

    return book;
}
