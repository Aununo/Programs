typedef enum{
    HASH_OK,
    HASH_ERROR,
	HASH_ADDED,
	HASH_REPLACED_VALUE,
	HASH_ALREADY_ADDED,
	HASH_DELETED,
	HASH_NOT_FOUND,
} HASH_RESULT; //相关函数的返回类型

typedef enum{
    HASH_PTR,
	HASH_NUMERIC,
	HASH_STRING,
} HASH_VAL_TAG; //value的类型

typedef struct __HashEntry HashEntry;
struct __HashEntry{
	union{
		char *str_value;
		double dbl_value;
		int int_value;
	} key;
	HASH_VAL_TAG val_tag;
	union{
		char *str_value;
		double dbl_value;
		int int_value;
		long long_value;
		void *ptr_value;
	} value;
	HashEntry *next;
}; /* HashEntry 键值对的类型*/



struct __HashTable{
	HashEntry **bucket;
	int size;
	HASH_RESULT last_error;
};
typedef struct __HashTable HashTable;


//哈希表采用链地址法处理冲突，在对有冲突的值进行插入时，采用“头”插入法，即新元素总在链首

// 向哈希表中添加元素，其中键类型为char*， 元素类型为char*。
HASH_RESULT hash_add_str(HashTable* table, const char *key, char *value );
// 从哈希表中删除元素，其中键类型为char*。
HASH_RESULT hash_del_str(HashTable *table, char *key );

