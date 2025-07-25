## 哈希表添加

哈希表（Hash Table，也叫散列表），是根据键（Key）而直接访问在内存存储位置的数据结构。也就是说，它通过计算一个关于键值的函数，将所需查询的数据映射到表中一个位置来访问记录，这加快了查找速度。这个映射函数称做哈希函数，存放记录的数组称做哈希表。哈希表相关定义如下：./hash.h

哈希表相关说明：

- HASH_RESULT 类型为相关函数的返回类型

- HashEntry 为哈希表所保存元素（即键值对 《key, value》）类型

- HASH_VAL_TAG 用来表示键值对中value的类型，value为字符指针时val_tag的值为HASH_STRING

- HashTable 为哈希表，其中 bucket 指向大小为size的、元素类型为 HashEntry*的指针数组

- 哈希表采用链地址法处理冲突

- 在对有冲突的值进行插入时，采用“头”插入法，即新元素总在链首

1. 请实现 hash_add_str 函数，向哈希表中添加元素，其中键和元素类型都为char*。在添加过程中，如果要添加的键值key已在哈希表中，且对应的值value也已存在，则函数返回HASH_ALREADY_ADDED；如果要添加的键值key已在哈希表中，但对应的值value不同，则函数将value值更新到哈希表中，之后返回 HASH_REPLACED_VALUE；如果要添加的键值key不在哈希表中，则函数创建 HashEntry 类型，并将其加入到哈希表中，且函数返回HASH_OK。

2. 请实现 hash_del_str 函数，从哈希表中删除元素，其中键类型为char*。在删除过程中，如果要删除的键值key在哈希表中，则函数返回 HASH_DELETED；如果要删除的键值key不在哈希表中，则函数返回 HASH_NOT_FOUND。

本题所用的哈希函数如下：

```c
long hash_string(const char* str) {
    long hash = 5381;
    int c;
    
    while (c == *str++) 
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    if (hash < 0)
        hash *= -1;
    return hash;
}
```

该函数将键值对中的 key 映射为内部存储的下标。

测试用例输入及输出说明如下：



```
//第一行为正整数 n，表示键值对的个数，之后为 n 行数据，每行为一个键值对，key在前，value在后，之间由空格隔开
//此类输入的含义是将 n 个键值对添加到哈希表中。示例如下：
2
www eee
www1 eee1
// 对此类输入，程序首先输出 hash_add_str 的返回值，之后是哈希表中的 size 和val_tag，最后是各键值对，如上例的正确输出为
0_0_9_2(www,eee)(www1,eee1)
```

程序的输入还有另一种形式：

```
// 第一行为负整数 -n，表示要从已建立好的含有 n 个元素的哈希表中删除 k （伪随机数）个元素，删除的 key 不一定在哈希表中存在；
// 对此类输入，程序首先输出 hash_del_str 的返回值，之后是以del开始的三元组del(i,j,k)，
// 其含义是：从哈希表中的 i 个元素中删除 j 个元素，并尝试删除 k 个不存在的元素；
// 之后是哈希表中的 size 和 val_tag，最后是各键值对
// 例如当输入为 -4 程序的输出为：
5_6_del(4,1,1)9_2(key1,value1)(key2,value2)(key3,value3)
```

## 半精度浮点数

在Ｃ语言中，单精度浮点数float由32位组成，其格式为IEEE-754标准定义。请用Ｃ语言实现半精度浮点数（即IEEE-754标准定义的16位浮点数）相关运算。因C语言的short类型也为16位，所以可以用short类型存储一个半精度浮点数，但其对应二进制串格式仍需遵从IEEE-754标准。相关函数定义如下：

```c
short float16_add( short op1, short op2); //16位浮点数加法
short float16_sub( short op1, short op2); //16位浮点数减法
short float16_mul( short op1, short op2); //16位浮点数乘法
```

在上述函数中，我们用short类型存储半精度浮点数，请根据IEEE-754标准解析该二进制串并进行相应运算，最后将运算结果保存到16位的short类型变量中返回。

IEEE-754浮点数相关工具可参考如下页面：https://numeral-systems.com/ieee-754-converter/


## 大整数乘法

C语言整数最多64位，在很多场景下不够用，现采用如下结构表示一个整数：

```c
typedef struct _Bigint {
    int mem_len;
    int digit_len;
    unsigned char* p;
} Bigint;
```

其中 mem_len 表示该整数占用内存的字节数，digit_len表示整数的字符位数（如12的字符位数为2，152的字符位数为3），p则指向该整数的实际存储空间，为方便使用，将其定义为unsigned char*类型，其大小为 mem_len

存储方式要求如下：

- 用4位二进制串表示1位十进制数，如十进制数6的二进制串为 0110

- 保存顺序为由十进制的高位到低位依次保存，如十进制数152，p所指向的二进制串为0001 0101 0010

请实现整数的乘法函数

```c
Bigint multi(Bigint m, Bigint k);
```

该函数返回整数 m 与 k 的积。

## PDF 文件解析

PDF文件格式说明可从下列链接中获取：

https://zxyle.github.io/PDF-Explained/

https://opensource.adobe.com/dc-acrobat-sdk-docs/pdfstandards/pdfreference1.0.pdf

请阅读并理解以上资料内容实现`pdf_get_page_num`函数：

```c
int pdf_get_page_num(FILE* pdf) ; //获取pdf文档的页数
```

其中，pdf指向已通过`fopen("xxx", "r")`打开的文件, 函数返回pdf文档的页数。

此外，还需实现下列函数：

```c
bool pdf_get_creator(FILE* pdf, char* creator) ; //获取文档的创建人，即由/Creator 指定的信息
bool pdf_get_author(FILE* pdf, char* author) ; //获取文档的作者，即由/Author 指定的信息
bool pdf_get_producer(FILE* pdf, char* producer) ; //获取文档的出版商，即由/Producer 指定的信息
bool pdf_get_file_title(FILE* pdf, char* file_title) ; //获取文档的标题
bool pdf_get_keywords(FILE* pdf, char* keywords) ; //获取文档的关键字信息
bool pdf_get_page_txt(FILE* pdf, int page_num, char* page_txt);//获取文档中指定页面(page_num)的文本内容(仅包含ASCII码)
```

上述函数中，pdf指向已通过`fopen("xxx", "r")`打开的文件, 各函数需将获取的信息保存到其最后一个参数中；如果文件中有指定信息，且函数已成功获取则返回true，否则返回false。

`pdf_get_page_txt`函数中的`page_num`从０开始计数，即当`page_num`为０时，获取文档第１页的文本内容。

注：以上函数所要设置或返回的信息，都需通过解析文件内容获取，如通过硬编码设置或返回答案，本题成绩将按０分处理！

```c
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>

int pdf_get_page_num(FILE* pdf) {
    return 0;
}
bool pdf_get_creator(FILE* pdf, char* creator) {
	return false;
}
bool pdf_get_author(FILE* pdf, char* author){
 	return false;
}
bool pdf_get_producer(FILE* pdf, char* producer) {
 	return false;
}
bool pdf_get_file_title(FILE* pdf, char* file_title) {
 	return true;
}
bool pdf_get_keywords(FILE* pdf, char* keywords) {
 	return false;
}
bool pdf_get_page_txt(FILE* pdf, int page_num, char* page_txt){
 	return false;
}
```

