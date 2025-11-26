#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "lrparser.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int reg_count;
extern int label_count;


typedef struct _symbol {
    char* name;
    int reg_num;
    int is_global;
    int is_const;
    int const_value;   
    int has_const_value; 
    char* array_type;  
    struct _symbol* next;
} Symbol;

typedef struct _symbol* pSymbol;


pSymbol createSymbol(char* name, int reg_num);
void addSymbol(char* name, int reg_num);
void addConstSymbol(char* name, int reg_num, int const_value);
void addGlobalSymbol(char* name);
void addGlobalSymbolWithType(char* name, char* array_type);
void addGlobalConstSymbol(char* name);
void addGlobalConstSymbolWithValue(char* name, int const_value);
void addGlobalConstSymbolWithType(char* name, char* array_type);
int findSymbol(char* name);
pSymbol findSymbolPtr(char* name);
void freeSymbols();


void genArithmeticExpr(past node, char* result);
void genLogicExpr(past node, char* result);
void genAssignStmt(past node, char* result);
void genIfStmt(past node, char* result);
void genWhileStmt(past node, char* result);


int newRegister();
int newLabel();
void genExpression(past node, char* result);
void genStmt(past node, char* result);

#endif

