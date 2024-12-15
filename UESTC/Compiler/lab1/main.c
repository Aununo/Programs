#include <stdio.h>
#include <stdlib.h>
#include "token.h"

extern int yylex();
extern _YYLVAL yylval;

int main(int argc, char **argv) {
    int token;
    while((token = yylex()) != 0) {
		if(token <= 260){
			switch (token) {
				case Y_ID:
					printf("<%d, %s>\n", token, yylval.id_name);
                    free(yylval.id_name);                    
					break;
				case num_INT:
					printf("<%d, %d>\n", token, yylval.int_value);
					break;
				case num_FLOAT:
					printf("<%d, %f>\n", token, yylval.float_value);
					break;
				default:
                    printf("<UNKNOWN>\n");
                    break;					
            }	
        } else {
            if(token <= 269 || token == 293) {
                printf("<%d, KEYWORD>\n", token);
            } else if(token <= 284) {
                printf("<%d, OPERATOR>\n", token);
            } else if(token <= 292) {
                printf("<%d, SYMBOL>\n", token);
            } else {
                printf("<UNKNOWN>\n");
            }            
        }       
    }
    return 0;
}