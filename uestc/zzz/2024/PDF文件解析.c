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