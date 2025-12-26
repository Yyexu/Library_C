#include <stdio.h>
#include <string.h>
#include "books.h"
#include "files.h"
#define DATA_FILE "book_library.data"


// 创建新书并保存到文件
Book* createBook(int id, const char* name, const char* author, const char* publisher,
    int price, int amount, const char* isbn, int category_id) {

	// 参数合法性检查
    if (id <= 0) {
        printf("[系统] ID非法，创建失败\n");
        return NULL;
    }
        
    FILE* fp = fopen(DATA_FILE, "rb");

	// 检查ID重复性
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

	// 分配内存并初始化
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

	// 调用add_book，添加到库中并保存到文件
    if (add_book(book) != 0) {
        free(book->name); free(book->author);
        free(book->publisher); free(book->isbn);
        free(book);
        return NULL;
    }

	// 返回新创建的书指针
    return book;
}

// 全量保存：把内存数组覆盖写入文件 (修改和删除后调用)
void SaveBooksToFile() {
    FILE* fp = fopen(DATA_FILE, "wb"); 
    if (!fp) return;

    for (int i = 0; i < globalBookCount; i++) {
        Book* b = &globalBookList[i];

        fwrite(&b->id, sizeof(int), 1, fp);
        fwrite(&b->price, sizeof(int), 1, fp);
        fwrite(&b->amount, sizeof(int), 1, fp);
        fwrite(&b->category_id, sizeof(int), 1, fp);

        write_string(fp, b->name);
        write_string(fp, b->author);
        write_string(fp, b->publisher);
        write_string(fp, b->isbn);
    }
    fclose(fp);
    printf("[系统] 数据已同步到文件。\n");
}

// 删除书籍
void DeleteBook(Book* bookToDelete) {
    if (!globalBookList || globalBookCount == 0) return;
    
    int index = -1;
    // 找到这本书在数组里的下标
    for (int i = 0; i < globalBookCount; i++) {
        if (&globalBookList[i] == bookToDelete) {
            index = i;
            break;
        }
    }

    if (index == -1) return; // 没找到

    // 释放这本书占用的内存字符串
    free(globalBookList[index].name);
    free(globalBookList[index].author);
    free(globalBookList[index].publisher);
    free(globalBookList[index].isbn);

    // 数组移位：把后面的书往前挪
    for (int i = index; i < globalBookCount - 1; i++) {
        globalBookList[i] = globalBookList[i + 1];
    }

    globalBookCount--;

    // 保存到文件
    SaveBooksToFile();
}


// 排序功能实现
// 内部使用的比较辅助变量
static int _sort_direction = 1; // 1为升序，-1为降序

// 具体的比较函数 (供 qsort 使用) 可继续添加更多比较方式
// 
// 1. 按 ID 比较
int CompareID(const void* a, const void* b) {
    Book* b1 = (Book*)a;
    Book* b2 = (Book*)b;
    return (b1->id - b2->id) * _sort_direction;
}

// 2. 按 价格 比较
int ComparePrice(const void* a, const void* b) {
    Book* b1 = (Book*)a;
    Book* b2 = (Book*)b;
    return (b1->price - b2->price) * _sort_direction;
}

// 3. 按 库存 比较
int CompareAmount(const void* a, const void* b) {
    Book* b1 = (Book*)a;
    Book* b2 = (Book*)b;
    return (b1->amount - b2->amount) * _sort_direction;
}

// 排序主函数
void SortGlobalBooks(SortType type, bool ascending) {
    if (globalBookCount < 2) return; // 没书或者只有一本，不用排

    // 设置方向 (升序乘1，降序乘-1)
    _sort_direction = ascending ? 1 : -1;

    // 根据类型选择比较函数
    int (*compar)(const void*, const void*) = NULL;

    switch (type) {
    case SORT_ID:    compar = CompareID; break;
    case SORT_PRICE: compar = ComparePrice; break;
    case SORT_AMOUNT: compar = CompareAmount; break;
    default: return;
    }

    // 执行快速排序
    // base: 数组首地址
    // num: 元素个数
    // size: 每个元素的大小
    // compar: 比较函数
    qsort(globalBookList, globalBookCount, sizeof(Book), compar);

    printf("[系统] 图书已排序 (模式: %d, 升序: %d)\n", type, ascending);
}


