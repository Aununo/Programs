#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "lrparser.tab.h"


past newAstNode(){
	past node = malloc(sizeof(ast));
	if(node == NULL)
	{
		printf("run out of memory.\n");
		exit(0);
	}
	memset(node, 0, sizeof(ast));
	return node;
}

void showToken(int Token){
	switch (Token)	{
	case Y_INT:
		printf("%s", "INTEGER");
		break;
	case Y_FLOAT:
		printf("%s", "FLOAT");
		break;
	case Y_CONST:
		printf("%s", "CONST");
		break;
	case Y_ADD:
	case '+':
		printf("'%c'", '+');
		break;
	case Y_SUB:
	case '-':
		printf("'%c'", '-');
		break;
	case Y_MUL:
	case '*':
		printf("'%c'", '*');
		break;
	case Y_DIV:
		printf("'%c'", '/');
		break;
	case Y_MODULO:
	case '%':
		printf("'%c'", '%');
		break;
	case Y_LESS:
	case '<':
		printf("'%c'", '<');
		break;
	case Y_LESSEQ:
		printf("'%s'", "<=");
		break;
	case Y_GREAT:
	case '>':
		printf("'%c'", '>');
		break;
	case Y_GREATEQ:
		printf("'%s'", ">=");
		break;
	case Y_NOTEQ:
		printf("'%s'", "!=");
		break;
	case Y_EQ:
		printf("'%s'", "==");
		break;
	case Y_NOT:
	case '!':
		printf("'%c'", '!');
		break;
	case Y_AND:
		printf("'%s'", "&&");
		break;
	case Y_OR:
		printf("'%s'", "||");
		break;
	case Y_ASSIGN:
	case '=':
		printf("'%c'", '=');
		break;
	default:
		break;
	}
}

void showAst(past node, int nest, int use_blank){
	if(node == NULL)
		return;
	
	// 特殊处理 TRANSLATION_UNIT：不输出节点本身，只遍历子节点
	if(node->nodeType == TRANSLATION_UNIT){
		showAst(node->left, nest, use_blank);
		showAst(node->right, nest, use_blank);
		showAst(node->next, nest, use_blank);
		return;
	}
	
	if(use_blank) {
		for (int i = 0; i < nest; i++)
			printf("    ");
	}
	
	switch (node->nodeType){
		case INTEGER_LITERAL:
			printf("%s %d\n", "INTEGER_LITERAL", node->ivalue);
			break;
		case FLOATING_LITERAL:
			printf("%s %f\n", "FLOATING_LITERAL", node->fvalue);
			break;
		case BINARY_OPERATOR:
			printf("%s ", "BINARY_OPERATOR");
			showToken(node->ivalue);
			printf("\n");
			break;
		case UNARY_OPERATOR:
			printf("%s ", "UNARY_OPERATOR");
			showToken(node->ivalue);
			printf("\n");
			break;
		case FUNCTION_DECL:
			printf("%s '%s'\n", "FUNCTION_DECL", node->svalue);
			break;
		case CALL_EXPR:
			printf("%s\n", "CALL_EXPR");
			break;
		case COMPOUND_STMT:
			printf("%s\n", "COMPOUND_STMT");
			break;
		case DECL_STMT:
			printf("%s\n", "DECL_STMT");
			break;
		case DECL_REF_EXPR:
			printf("%s '%s'\n", "DECL_REF_EXPR", node->svalue);
			break;
		case PARM_DECL:
			printf("%s '%s'\n", "PARM_DECL", node->svalue);
			break;
		case VAR_DECL:
			printf("%s '%s'\n", "VAR_DECL", node->svalue);
			break;
		case RETURN_STMT:
			printf("%s\n", "RETURN_STMT");
			break;
		case WHILE_STMT:
			printf("%s\n", "WHILE_STMT");
			break;
		case BREAK_STMT:
			printf("%s\n", "BREAK_STMT");
			break;
		case CONTINUE_STMT:
			printf("%s\n", "CONTINUE_STMT");
			break;
		case IF_STMT:
			printf("%s\n", "IF_STMT");
			showAst(node->if_cond, nest+1, use_blank);
			break;
		case INIT_LIST_EXPR:
			printf("%s\n", "INIT_LIST_EXPR");
			break;
		case ARRAY_SUBSCRIPT_EXPR:
			printf("%s\n", "ARRAY_SUBSCRIPT_EXPR");
			break;
		case PAREN_EXPR:
			printf("%s\n", "PAREN_EXPR");
			break;
		default:
			printf("%s\n", "NOT_IMPLEMENTED");
			break;
	}
	showAst(node->left, nest+1, use_blank);
	showAst(node->right, nest+1, use_blank);
	showAst(node->next, nest, use_blank);
}


char* get_id(char* id){
	int i = 0;
	while(id[i] != '\0'){
		i++;
	}
	char* s = malloc(sizeof(char) * (i + 1));
	memcpy(s, id, i + 1);
	return s;
}

char* get_stype(int type){
	char* stype;
	if(type == 0){
		return NULL;
	} else if(type == Y_INT){
		stype = "int";
	} else if(type == Y_FLOAT){
		stype = "float";
	} else {
		stype = "void";
	}
	return stype;
}

char* get_conststype(int type){
	char* stype;
	if(type == 0){
		return NULL;
	} else if(type == Y_INT){
		stype = "const int";
	} else if(type == Y_FLOAT){
		stype = "const float";
	} else {
		stype = "const void";
	}
	return stype;
}

past newCompUnit(past left, past right){
	past node = newAstNode();
	node->nodeType = TRANSLATION_UNIT;
	node->snodeType = "TRANSLATION_UNIT";
	node->left = left;
	node->right = right;
	return node;
}

past newDeclStmt(past left, past right){  
	past node = newAstNode();
	node->nodeType = DECL_STMT;
	node->snodeType = "DECL_STMT";
	node->left = left;
	node->right = right; 
	return node;
}

past newDeclRefExp(char* name, past left, past right){ 	
	past node = newAstNode();
	node->nodeType = DECL_REF_EXPR;
	node->snodeType = "DECL_REF_EXPR";
	node->svalue = name;
	node->left = left;
	node->right = right;
	return node;
}

past newFuncDecl(char* stype, int type, char* name, past left, past right){ 
	past node = newAstNode();
	node->nodeType = FUNCTION_DECL;
	node->snodeType = "FUNCTION_DECL";
	node->svalue = name;
	node->left = left;
	node->right = right;
	node->ivalue = type;
	node->stype = stype;
	return node;	
}

past newVarDecl(char* stype, int type, int if_const, char *name, past left, past right){ 
	past node = newAstNode();
	node->nodeType = VAR_DECL;
	node->snodeType = "VAR_DECL";
	node->stype = stype;
	node->left = left;
	node->right = right;
	node->ivalue = type;
	node->svalue = name;
	node->if_const = if_const;
	return node;	
}


past newCompoundStmt(past left, past right){  
	past node = newAstNode();
	node->nodeType = COMPOUND_STMT;
	node->snodeType = "COMPOUND_STMT";
	node->left = left;
	node->right = right;
	return node;
}


past newArraySubscriptsExp(past left, past right){  
	past node = newAstNode();
	node->nodeType = ARRAY_SUBSCRIPT_EXPR;
	node->snodeType = "ARRAY_SUBSCRIPT_EXPR";
	node->left = left;
	node->right = right;
	return node;
}

past newBinaryOper(char* soper, int oper, past left, past right){  
	past node = newAstNode();
	node->nodeType = BINARY_OPERATOR;
	node->snodeType = "BINARY_OPERATOR";
	node->ivalue = oper;
	node->svalue = soper;
	node->left = left;
	node->right = right;
	return node;
}


past newCallExp(char* stype, int type, char* name, past left, past right){  
	past node = newAstNode();
	node->nodeType = CALL_EXPR;
	node->snodeType = "CALL_EXPR";
	node->stype = stype;
	node->ivalue = type;
	node->svalue = name;
	node->left = left;
	node->right = right;
	return node;
}

past newParaDecl(char* stype,char* name,  past left, past right){  
	past node = newAstNode();
	node->nodeType = PARM_DECL;
	node->snodeType = "PARM_DECL";
	node->stype = stype;
	node->svalue = name;
	node->left = left;
	node->right = right;
	return node;
}

past newIntVal(int ival){
	past node = newAstNode();
	node->nodeType = INTEGER_LITERAL;
	node->snodeType = "INTEGER_LITERAL";
	node->ivalue = ival;
	return node;
}

past newFloatVal(float fval){
	past node = newAstNode();
	node->nodeType = FLOATING_LITERAL;
	node->snodeType = "FLOATING_LITERAL";
	node->fvalue = fval;
	return node;
}

past newIfStmt(past if_cond, past left, past right){ 
	past node = newAstNode();
	node->nodeType = IF_STMT;
	node->snodeType = "IF_STMT";
	node->if_cond = if_cond;
	node->left = left;
	node->right = right;
	if(right != NULL){
		node->svalue = "has else";
	} else {
		node->svalue = "no else";
	}
	return node;
}

past newWhileStmt(past left, past right){  
	past node = newAstNode();
	node->nodeType = WHILE_STMT;
	node->snodeType = "WHILE_STMT";
	node->left = left;
	node->right = right;
	return node;
}

past newContinueStmt(){
	past node = newAstNode();
	node->nodeType = CONTINUE_STMT;
	node->snodeType = "CONTINUE_STMT";
	return node;
}

past newBreakStmt(){
	past node = newAstNode();
	node->nodeType = BREAK_STMT;
	node->snodeType = "BREAK_STMT";
	return node;
}

past newReturnStmt(past left, past right){   
	past node = newAstNode();
	node->nodeType = RETURN_STMT;
	node->snodeType = "RETURN_STMT";
	node->left = left;
	node->right = right;
	return node;
}

past newType(int oper){
	past node = newAstNode();
	node->ivalue = oper;
	return node;
}