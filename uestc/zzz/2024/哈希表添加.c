#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

// 这就是个哈希函数，将输入的字符串映射为一个long类型的hash值
long hash_string(const char* str) {
    long hash = 5381;
    int c;
    
    while (c = *str++) 
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    if (hash < 0)
        hash *= -1;
    return hash;
}


HASH_RESULT hash_add_str(HashTable* table, const char *key, char *value ) {
    //TODO: 你的代码
    int index = hash_string(key) % table->size; //将键值对中的 key 映射为内部存储的下标。
    HashEntry *entry = table->bucket[index];
    HashEntry *prev = NULL;
    while (entry != NULL) {
        if (strcmp(entry->key.str_value, key) == 0) {
            // 键已存在，检查值是否相同
            if (strcmp(entry->value.str_value, value) == 0) {
                table->last_error = HASH_ALREADY_ADDED;
                return HASH_ALREADY_ADDED;
            } else {
                // 值不同，替换旧值
                free(entry->value.str_value);
                entry->value.str_value = strdup(value);
                if (entry->value.str_value == NULL) {
                    table->last_error = HASH_ERROR;
                    return HASH_ERROR;
                }
                table->last_error = HASH_REPLACED_VALUE;
                return HASH_REPLACED_VALUE;
            }
        }
        prev = entry;
        entry = entry->next;
    }

    // 键不存在，创建新节点
    HashEntry *new_entry = (HashEntry *)malloc(sizeof(HashEntry));
    if (new_entry == NULL) {
        table->last_error = HASH_ERROR;
        return HASH_ERROR;
    }

    // 复制键和值
    new_entry->key.str_value = strdup(key);
    new_entry->value.str_value = strdup(value);
    if (new_entry->key.str_value == NULL || new_entry->value.str_value == NULL) {
        free(new_entry->key.str_value);
        free(new_entry->value.str_value);
        free(new_entry);
        table->last_error = HASH_ERROR;
        return HASH_ERROR;
    }

    // 设置节点属性并插入链表头部
    new_entry->val_tag = HASH_STRING;
    new_entry->next = table->bucket[index];
    table->bucket[index] = new_entry;
    table->last_error = HASH_OK;
    return HASH_OK;
}



HASH_RESULT hash_del_str( HashTable *table, char *key ) {
    //TODO: 你的代码
    unsigned int hash = hash_string(key);
    int index = hash % table->size;

    // 遍历桶中的链表，查找并删除键
    HashEntry *entry = table->bucket[index];
    HashEntry *prev = NULL;
    while (entry != NULL) {
        if (strcmp(entry->key.str_value, key) == 0) {
            // 找到键，删除节点
            if (prev == NULL) {
                table->bucket[index] = entry->next;
            } else {
                prev->next = entry->next;
            }
            free(entry->key.str_value);
            free(entry->value.str_value);
            free(entry);
            table->last_error = HASH_DELETED;
            return HASH_DELETED;
        }
        prev = entry;
        entry = entry->next;
    }

    // 键不存在
    table->last_error = HASH_NOT_FOUND;
    return HASH_NOT_FOUND;
}

