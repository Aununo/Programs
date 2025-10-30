#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node_type.h"
#include "token.h"

extern int yylex();
extern char* yytext;
extern int yyleng;
_YYLVAL yylval;

int paranum;
int callParaNum;
int compoundStmtNum;
int if_DeclStmt;
int tok;

typedef struct _ast ast;
typedef struct _ast *past;
struct _ast {
	char* stype;
	int ivalue;
	float fvalue;
	char* svalue;
	node_type nodeType;
	char* snodeType;
	int if_const;
	past left;
	past right;
	past if_cond;
	past next;
};

past newAstNode();
void showAst(char* sym, past node, int nest);
void showParaDecl(past node);
void showCompoundStmt(past node, int nest);
void showCallExp(past node, int nest);
void showTranstion(past node, int nest);
int match(int x);
void advance();

past astDecl();
past astConstDecl();
past astConstDefs(char* stype, int type);
past astConstDef(char* stype, int type);
past astConstInitVal();
past astVarDecl();
past astVarDecls();
past astVarDef(char* stype, int type);
past astInitVal();
past astFuncDef();
past astFuncParams();
past astFuncParam();
past astBlock();
int astType();
past astCompUnit();
past astStmt();
past astBlockItems();
past astBlockItem();
past astLOrExp();
past astLAndExp();
past astEqExp();
past astRelExp();
past astExp();
past astAddExp();
past astMulExp();
past astUnaryExp();
past astCallParams();
past astPrimaryExp();
past astArraySubscriptsExp();
past newDeclStmt(past left, past right);
past newCompUnit(past left, past right);
past newDeclRefExp(char* name, past left, past right);
past newFuncDecl(char* stype, int type,char* svalue, past left, past right);
past newBinaryOper(char* soper, int oper, past left, past right);
past newArraySubscriptsExp(past left, past right);
past newCallExp(char* stype, int type, char* name, past left, past right);
past newParaDecl(char* stype, char* name, past left, past right);
past newCompoundStmt(past left, past right);
past newIntVal(int ival);
past newFloatVal(float fval);
past newVarDecl(char* stype, int type, int if_cond, char* s, past left, past right);
past newIfStmt(past if_cond, past left, past right);
past newWhileStmt(past left, past right);
past newContinueStmt();
past newBreakStmt();
past newReturnStmt(past left, past right);

int main(int argc, char **argv) {
	advance();
	past node = astCompUnit();
	showAst("|", node, 0);
	return 0;
}

int match(int x) {
	if(tok == x) {
		return 1;
	}
	return 0;
}

void advance() {
	tok = yylex();
	// printf("tok: %s\n", yytext);  // 调试用，可以注释掉
}

past newAstNode() {
	past node = malloc(sizeof(ast));
	if(node == NULL) {
		printf("run out of memory.\n");
		exit(0);
	}
	memset(node, 0, sizeof(ast));
	return node;
}

void showAst(char* sym, past node, int nest) {
	if(node == NULL)
		return;
	int i = 0;
	for(i = 0; i < nest; i++)
		printf("  ");
	if(node->nodeType == TRANSLATION_UNIT) {
		printf("%s-%s\n", sym, node->snodeType);
		showTranstion(node, nest + 1);
		return;
	} else if(node->nodeType == INTEGER_LITERAL) {
		printf("%s-%s  %d\n", sym, node->snodeType, node->ivalue);
	} else if(node->nodeType == FLOATING_LITERAL) {
		printf("%s-%s  %f\n", sym, node->snodeType, node->fvalue);
	} else if(node->nodeType == IF_STMT) {
		printf("%s-%s  %s\n", sym, node->snodeType, node->svalue);
		showAst("|", node->if_cond, nest+1);
	} else if(node->nodeType == FUNCTION_DECL) {
		printf("%s-%s  %s '%s'\n", sym, node->snodeType, node->svalue, node->stype);
	} else if(node->nodeType == PARM_DECL) {
		showParaDecl(node);
		return;
	} else if(node->nodeType == COMPOUND_STMT) {
		printf("%s-%s\n", sym, node->snodeType);
		node = node->right;
		showCompoundStmt(node, nest + 1);
		return;
	} else if(node->nodeType == VAR_DECL) {
		printf("%s-%s  %s '%s'\n", sym, node->snodeType, node->svalue, node->stype);
	} else if(node->nodeType == CALL_EXPR) {
		printf("%s-%s  %s\n", sym, node->snodeType, node->svalue);
		node = node->left;
		showCallExp(node, nest + 1);
		return;
	} else if(node->svalue != NULL) {
		printf("%s-%s  '%s'\n", sym, node->snodeType, node->svalue);
	} else {
		printf("%s-%s\n", sym, node->snodeType);
	}
	showAst("|", node->left, nest+1);
	showAst("`", node->right, nest+1);
}

void showTranstion(past node, int nest) {
	if(node == NULL) {
		return;
	}
	while(node->right != NULL) {
		showAst("|", node->left, nest);
		node = node->right;
	}
	showAst("`", node->left, nest);
}

void showCallExp(past node, int nest) {
	if(node == NULL) {
		return;
	}
	while(node->right != NULL) {
		showAst("|", node->left, nest);
		node = node->right;
	}
	showAst("`", node->left, nest);
}

void showCompoundStmt(past node, int nest) {
	if(node == NULL) {
		return;
	}
	while(node->right != NULL) {
		showAst("|", node->left, nest);
		node = node->right;
	}
	showAst("`", node->left, nest);
}

void showParaDecl(past node) {
	if(node == NULL) {
		return;
	}
	int i = paranum;
	past stack[paranum];
	int top = 0;
    while(top || node) {
		if(node != NULL) {
			stack[top++] = node;
            node = node->left;
        } else if(top != 0) {
			node = stack[--top];
			if(i == paranum) {
				printf("|-%s  used %s '%s'\n", node->snodeType, node->svalue, node->stype);
				i--;
			} else {
				printf("    |-%s  used %s '%s'\n", node->snodeType, node->svalue, node->stype);
			}
            node = node->right;
        }
    }
}

past astCompUnit() {
	past l = astDecl();
	if(l == NULL) {
		l = astFuncDef();
	}
	l = newCompUnit(l, NULL);
	past res = l;
	past r = astDecl();
	if(r == NULL) {
		r = astFuncDef();
	}
	while(r != NULL) {
		r = newCompUnit(r, NULL);
		l->right = r;
		l = r;
		r = astDecl();
		if(r == NULL) {
			r = astFuncDef();
		}
	}
	return res;
}

past astDecl() {
	past l;
	if((l = astConstDecl())) {
		return l;
	} else if((l = astVarDecl())) {
		return l;
	} else {
		return NULL;
	}
}

past astConstDecl() {
	if(!match(Y_CONST)) {
		return NULL;
	}
	advance();
	int type = astType();
	char* stype;
	if(type == 0) {
		return NULL;
	} else if(type == Y_INT) {
		stype = "const int";
	} else if(type == Y_FLOAT) {
		stype = "const float";
	} else {
		stype = "const void";
	}
	past r = astConstDefs(stype, type);
	if(!match(Y_SEMICOLON)) {
		return NULL;
	}
	advance();
	return newDeclStmt(NULL, r);
}

past astConstDefs(char* stype, int type) {
	past l = astConstDef(stype, type);
	while(match(Y_COMMA)) {
		advance();
		past r = astConstDef(stype, type);
		l = newDeclStmt(l, r);
	}
	return l;
}

past astConstDef(char* stype, int type) {
	if(!match(Y_ID)) {
		return NULL;
	}
	char* s = malloc(sizeof(char) * (yyleng + 1));
    strcpy(s, yytext);
	advance();
	if(match(Y_ASSIGN)) {
		advance();
		past l = astConstInitVal();
		return newVarDecl(stype, type, 1, s, NULL, l);
	} else if(match(Y_LSQUARE)) {
		advance();
		past l = astAddExp();
		if(!match(Y_RSQUARE)) {
			return NULL;
		}
		advance();
		while(match(Y_LSQUARE)) {
			advance();
			past r = astAddExp();
			if(!match(Y_RSQUARE)) {
				return NULL;
			}
			advance();
		}
		if(!match(Y_ASSIGN)) {
			return NULL;
		}
		advance();
		past r = astConstInitVal();
		return newVarDecl(stype, type, 1, s, NULL, r);
	} else {
		return NULL;
	}
}

past astConstInitVal() {
	past l = astAddExp();
	if(l != NULL) {
		return l;
	} else if(match(Y_LBRACKET)) {
		advance();
		if(match(Y_RBRACKET)) {
			advance();
			return NULL;  
		} else {
			past l = astConstInitVal();
			while(match(Y_COMMA)) {
				advance();
				past r = astConstInitVal();
			}
			if(!match(Y_RBRACKET)) {
				return NULL;
			}
			advance();
			return l;
		}
	} else {
		return NULL;
	}
}

past astVarDecl() {
	int type = astType();
	char* stype;
	if(type == 0) {
		return NULL;
	} else if(type == Y_INT) {
		stype = "int";
	} else if(type == Y_FLOAT) {
		stype = "float";
	} else {
		stype = "void";
	}
	past r = astVarDecls(stype, type);
	if(r->nodeType == FUNCTION_DECL) {
		return r;
	}
	if(!match(Y_SEMICOLON)) {
		return NULL;
	}
	advance();
	return newDeclStmt(NULL, r);
}

past astVarDecls(char* stype, int type) {
	past l = astVarDef(stype, type);
	if(l->nodeType == FUNCTION_DECL) {
		return l;
	}
	while(match(Y_COMMA)) {
		advance();
		past r = astVarDef(stype, type);
		l = newDeclStmt(l, r);
	}
	return l;
}

past astVarDef(char* stype, int type) {
	if(!match(Y_ID)) {
		return NULL;
	}
	char* s = malloc(sizeof(char) * (yyleng + 1));
    strcpy(s, yytext);
	advance();
	if(match(Y_LPAR)) {
		advance();
		if(match(Y_RPAR)) {
			advance();
			past l = astBlock();
			l = newCompoundStmt(NULL, l);
			return newFuncDecl(stype, type, s, NULL, l);
		} else {
			past r = astFuncParams();
			if(!match(Y_RPAR)) {
				return NULL;
			}
			advance();
			past b = astBlock();
			b = newCompoundStmt(NULL, b);
			return newFuncDecl(stype, type, s, r, b);
		}
	} else if(match(Y_ASSIGN)) {
		advance();
		past l = astInitVal();
		return newVarDecl(stype, type, 0, s, NULL, l);
	} else if(match(Y_LSQUARE)) {
		advance();
		past l = astAddExp();
		if(!match(Y_RSQUARE)) {
			return NULL;
		}
		advance();
		while(match(Y_LSQUARE)) {
			advance();
			past r = astAddExp();
			if(!match(Y_RSQUARE)) {
				return NULL;
			}
			advance();
		}
		if(match(Y_ASSIGN)) {
			advance();
			past r = astInitVal();
			return newVarDecl(stype, type, 0, s, NULL, r);
		} else {
			return newVarDecl(stype, type, 0, s, NULL, NULL);
		}
	} else {
		return newVarDecl(stype, type, 0, s, NULL, NULL);
	}
}

past astInitVal() {
	past l = astExp();
	if(l != NULL) {
		return l;
	} else if(match(Y_LBRACKET)) {
		advance();
		if(match(Y_RBRACKET)) {
			advance();
			return NULL;  
		} else {
			past l = astInitVal();
			while(match(Y_COMMA)) {
				advance();
				past r = astInitVal();
			}
			if(!match(Y_RBRACKET)) {
				return NULL;
			}
			advance();
			return l;
		}
	} else {
		return NULL;
	}
}

past astFuncDef() {
	int type = astType();
	char* stype;
	if(type == 0) {
		return NULL;
	} else if(type == Y_INT) {
		stype = "int";
	} else if(type == Y_FLOAT) {
		stype = "float";
	} else {
		stype = "void";
	}
	if(!match(Y_ID)) {
		return NULL;
	}
	char* s = malloc(sizeof(char) * (yyleng + 1));
    strcpy(s, yytext);
	advance();
	if(!match(Y_LPAR)) {
		return NULL;
	}
	advance();
	if(match(Y_RPAR)) {
		advance();
		past l = astBlock();
		l = newCompoundStmt(NULL, l);
		return newFuncDecl(stype, type, s, NULL, l);
	} else {
		past r = astFuncParams();
		if(!match(Y_RPAR)) {
			return NULL;
		}
		advance();
		past b = astBlock();
		b = newCompoundStmt(NULL, b);
		return newFuncDecl(stype, type, s, r, b);
	}
}

past astFuncParams() {
	paranum = 1;
	past l = astFuncParam();
	while(match(Y_COMMA)) {
		advance();
		past r = astFuncParam();
		r->left = l;
		l = r;
		paranum++;
	}
	return l;
}

past astFuncParam() {
	int type = astType();
	char* stype;
	if(type == 0) {
		return NULL;
	} else if(type == Y_INT) {
		stype = "int";
	} else if(type == Y_FLOAT) {
		stype = "float";
	} else {
		stype = "void";
	}
	if(!match(Y_ID)) {
		return NULL;
	}
	char* s = malloc(sizeof(char) * (yyleng + 1));
    strcpy(s, yytext);
	advance();
	past l = NULL;
	if(match(Y_LSQUARE)) {
		advance();
		if(!match(Y_RSQUARE)) {
			return NULL;
		}
		advance();
		l = astArraySubscriptsExp();
		return newParaDecl(stype, s, NULL, NULL);
	} else {
		l = astArraySubscriptsExp();
		return newParaDecl(stype, s, NULL, NULL);
	}
}

past astStmt() {
	past t;
	if(match(Y_ID)) {
		char* s = malloc(sizeof(char) * (yyleng + 1));
        strcpy(s, yytext);
		advance();
		past n = astArraySubscriptsExp();
		if(n == NULL) {
			if(!match(Y_ASSIGN)) {
				return NULL;
			}
			advance();
			past l = astExp();
			if(!match(Y_SEMICOLON)) {
				return NULL;
			}
			advance();
			past r = newDeclRefExp(s, NULL, NULL);
			return newBinaryOper("=", Y_ASSIGN, r, l);
		} else {
			if(!match(Y_ASSIGN)) {
				return NULL;
			}
			advance();
			past l = astExp();
			if(!match(Y_SEMICOLON)) {
				return NULL;
			}
			advance();
			past r = newDeclRefExp(s, NULL, NULL);
			n = newArraySubscriptsExp(r, n);
			return newBinaryOper("=", Y_ASSIGN, n, l);
		}
	} else if(match(Y_SEMICOLON)) {
		advance();
		return NULL;
	} else if((t = astExp())) {
		if(!match(Y_SEMICOLON)) {
			return NULL;
		}
		advance();
		return t;
	} else if(match(Y_LBRACKET)) {
		advance();
		past l = astBlockItems();
		if(!match(Y_RBRACKET)) {
			return NULL;
		}
		advance();
		return l;
	} else if(match(Y_WHILE)) {
		advance();
		if(!match(Y_LPAR)) {
			return NULL;
		}
		advance();
		past l = astLOrExp();
		if(!match(Y_RPAR)) {
			return NULL;
		}
		advance();
		past r = astStmt();
		r = newCompoundStmt(NULL, r);
		return newWhileStmt(l, r);
	} else if(match(Y_IF)) {
		advance();
		if(!match(Y_LPAR)) {
			return NULL;
		}
		advance();
		past l1 = astLOrExp();
		if(!match(Y_RPAR)) {
			return NULL;
		}
		advance();
		past l2 = astStmt();
		l2 = newCompoundStmt(NULL, l2);
		if(!match(Y_ELSE)) {
			return newIfStmt(l1, l2, NULL);
		}
		advance();
		past l3 = astStmt();
		l3 = newCompoundStmt(NULL, l3);
		return newIfStmt(l1, l2, l3);
	} else if(match(Y_BREAK)) {
		advance();
		if(!match(Y_SEMICOLON)) {
			return NULL;
		}
		advance();
		return newBreakStmt();
	} else if(match(Y_CONTINUE)) {
		advance();
		if(!match(Y_SEMICOLON)) {
			return NULL;
		}
		advance();
		return newContinueStmt();
	} else if(match(Y_RETURN)) {
		advance();
		past l = astExp();
		if(!match(Y_SEMICOLON)) {
			return NULL;
		}
		advance();
		return newReturnStmt(l, NULL);
	} else {
		return NULL;
	}
}

past astBlock() {
	if(!match(Y_LBRACKET)) {
		return NULL;
	}
	advance();
	past l = astBlockItems();
	if(!match(Y_RBRACKET)) {
		return NULL;
	}
	advance();
	return l;
}

past astBlockItems() {
	compoundStmtNum = 1;
	past temp;
	past l = astBlockItem();
	l = newCompoundStmt(l, NULL);
	temp = l;
	past r = astBlockItem();
	while(r != NULL) {
		compoundStmtNum++;
		r = newCompoundStmt(r, NULL);
		l->right = r;
		l = r;
		r = astBlockItem();
	}
	return temp;
}

past astBlockItem() {
	past l = astDecl();
	if(l) {
		return l;
	}
	return astStmt();
}

past astLOrExp() {
	past l = astLAndExp();
	while(match(Y_OR)) {
		int oper = tok;
		advance();
		past r = astLAndExp();
		l = newBinaryOper("||", oper, l, r);
	}
	return l;
}

past astLAndExp() {
	past l = astEqExp();
	while(match(Y_AND)) {
		int oper = tok;
		advance();
		past r = astEqExp();
		l = newBinaryOper("&&", oper, l, r);
	}
	return l;
}

past astEqExp() {
	past l = astRelExp();
	while(match(Y_EQ) || match(Y_NOTEQ)) {
		int oper = tok;
		advance();
		past r = astRelExp();
		if(oper == Y_EQ) {
			l = newBinaryOper("==", oper, l, r);
		} else {
			l = newBinaryOper("!=", oper, l, r);
		}
	}
	return l;
}

past astRelExp() {
	past l = astAddExp();
	while(match(Y_LESS) || match(Y_LESSEQ) || match(Y_GREAT) || match(Y_GREATEQ)) {
		int oper = tok;
		advance();
		past r = astAddExp();
		if(oper == Y_LESS) {
			l = newBinaryOper("<", oper, l, r);
		} else if(oper == Y_GREAT) {
			l = newBinaryOper(">", oper, l, r);
		} else if(oper == Y_LESSEQ) {
			l = newBinaryOper("<=", oper, l, r);
		} else {
			l = newBinaryOper(">=", oper, l, r);
		}
	}
	return l;
}

past astExp() {
	return astAddExp();
}

past astAddExp() {
	past l = astMulExp();
	while(match(Y_ADD) || match(Y_SUB)) {
		int oper = tok;
		advance();
		past r = astMulExp();
		if(oper == Y_ADD) {
			l = newBinaryOper("+", oper, l, r);
		} else {
			l = newBinaryOper("-", oper, l, r);
		}
	}
	return l;
}

past astMulExp() {
	past l = astUnaryExp();
	while(match(Y_MUL) || match(Y_DIV) || match(Y_MODULO)) {
		int oper = tok;
		advance();
		past r = astUnaryExp();
		if(oper == Y_MUL) {
			l = newBinaryOper("*", oper, l, r);
		} else if(oper == Y_DIV) {
			l = newBinaryOper("/", oper, l, r);
		} else {
			l = newBinaryOper("%", oper, l, r);
		}
	}
	return l;
}

past astUnaryExp() {
	past l = astPrimaryExp();
	if(l != NULL) {
		return l;
	} else if(match(Y_ID)) {
		char* s = malloc(sizeof(char) * (yyleng + 1));
        strcpy(s, yytext);
		advance();
		if(!match(Y_LPAR)) {
			return NULL;
		}
		advance();
		if(match(Y_RPAR)) {
			advance();
			return newCallExp(NULL,0, s, NULL, NULL);
		} else {
			past l = astCallParams();
			if(!match(Y_RPAR)) {
				return NULL;
			}
			advance();
			return newCallExp(NULL, 0, s, l, NULL);
		}
	} else if(match(Y_ADD)) {
		advance();
		past l = astUnaryExp();
		return newBinaryOper("+", Y_ADD, NULL, l);
	} else if(match(Y_SUB)) {
		advance();
		past l = astUnaryExp();
		return newBinaryOper("-", Y_SUB, NULL, l);
	} else if(match(Y_NOT)) {
		advance();
		past l = astUnaryExp();
		return newBinaryOper("!", Y_NOT, NULL, l);
	} else {
		return NULL;
	}
}

past astCallParams() {
	callParaNum = 1;
	past l = astExp();
	past t = newParaDecl(NULL, NULL, l, NULL);
	l = t;
	while(match(Y_COMMA)) {
		advance();
		callParaNum++;
		past r = astExp();
		l->right = newParaDecl(NULL, NULL, r, NULL);
		l = l->right;
	}
	return t;
}

past astPrimaryExp() {
	if(match(Y_LPAR)) {
		advance();
		past n1 = astExp();
		if(!match(Y_RPAR)) {
			return NULL;
		}
		return n1;
	} else if(match(Y_ID)) {
		char* s = malloc(sizeof(char) * (yyleng + 1));
        strcpy(s, yytext);
		past l = newDeclRefExp(s, NULL, NULL);
		advance();
		if(match(Y_LPAR)) {
			advance();
			if(match(Y_RPAR)) {
				advance();
				return newCallExp(NULL,0, s, NULL, NULL);
			} else {
				past l = astCallParams();
				if(!match(Y_RPAR)) {
					return NULL;
				}
				advance();
				return newCallExp(NULL, 0, s, l, NULL);
			}
		}
		past n = astArraySubscriptsExp();
		if(n != NULL) {
			return newArraySubscriptsExp(l, n);
		} else {
			return l;
		}
	} else if(match(num_INT)) {
		past n2 = newIntVal(atoi(yytext));
		advance();
		return n2;
	} else if(match(num_FLOAT)) {
		past n3 = newFloatVal(atof(yytext));
		advance();
		return n3;
	} else {
		return NULL;
	}
}

past astArraySubscriptsExp() {
	if(!match(Y_LSQUARE)) {
		return NULL;
	}
	advance();
	past l = astExp();
	if(!match(Y_RSQUARE)) {
		return NULL;
	}
	advance();
	while(match(Y_LSQUARE)) {
		advance();
		past r = astExp();
		if(!match(Y_RSQUARE)) {
			return NULL;
		}
		l = newArraySubscriptsExp(l,r);
	}
	return l;
}

int astType() {
	if(match(Y_INT)) {
		advance();
		return Y_INT;
	} else if(match(Y_FLOAT)) {
		advance();
		return Y_FLOAT;
	} else if(match(Y_VOID)) {
		advance();
		return Y_VOID;
	} else {
		return 0;
	}
}

past newCompUnit(past left, past right) {
	past node = newAstNode();
	node->nodeType = TRANSLATION_UNIT;
	node->snodeType = "TRANSLATION_UNIT";
	node->left = left;
	node->right = right;
	return node;
}

past newDeclStmt(past left, past right) {
	past node = newAstNode();
	node->nodeType = DECL_STMT;
	node->snodeType = "DECL_STMT";
	node->left = left;
	node->right = right;
	return node;
}

past newDeclRefExp(char* name, past left, past right) {
	past node = newAstNode();
	node->nodeType = DECL_REF_EXPR;
	node->snodeType = "DECL_REF_EXPR";
	node->svalue = name;
	node->left = left;
	node->right = right;
	return node;
}

past newFuncDecl(char* stype, int type, char* name, past left, past right) {
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

past newVarDecl(char* stype, int type, int if_const, char *name, past left, past right) {
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

past newCompoundStmt(past left, past right) {
	past node = newAstNode();
	node->nodeType = COMPOUND_STMT;
	node->snodeType = "COMPOUND_STMT";
	node->left = left;
	node->right = right;
	return node;
}

past newArraySubscriptsExp(past left, past right) {
	past node = newAstNode();
	node->nodeType = ARRAY_SUBSCRIPT_EXPR;
	node->snodeType = "ARRAY_SUBSCRIPT_EXPR";
	node->left = left;
	node->right = right;
	return node;
}

past newBinaryOper(char* soper, int oper, past left, past right) {
	past node = newAstNode();
	node->nodeType = BINARY_OPERATOR;
	node->snodeType = "BINARY_OPERATOR";
	node->ivalue = oper;
	node->svalue = soper;
	node->left = left;
	node->right = right;
	return node;
}

past newCallExp(char* stype, int type, char* name, past left, past right) {
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

past newParaDecl(char* stype,char* name, past left, past right) {
	past node = newAstNode();
	node->nodeType = PARM_DECL;
	node->snodeType = "PARM_DECL";
	node->stype = stype;
	node->svalue = name;
	node->left = left;
	node->right = right;
	return node;
}

past newIntVal(int ival) {
	past node = newAstNode();
	node->nodeType = INTEGER_LITERAL;
	node->snodeType = "INTEGER_LITERAL";
	node->ivalue = ival;
	return node;
}

past newFloatVal(float fval) {
	past node = newAstNode();
	node->nodeType = FLOATING_LITERAL;
	node->snodeType = "FLOATING_LITERAL";
	node->fvalue = fval;
	return node;
}

past newIfStmt(past if_cond, past left, past right) {
	past node = newAstNode();
	node->nodeType = IF_STMT;
	node->snodeType = "IF_STMT";
	node->if_cond = if_cond;
	node->left = left;
	node->right = right;
	if(right != NULL) {
		node->svalue = "has else";
	} else {
		node->svalue = "no else";
	}
	return node;
}

past newWhileStmt(past left, past right) {
	past node = newAstNode();
	node->nodeType = WHILE_STMT;
	node->snodeType = "WHILE_STMT";
	node->left = left;
	node->right = right;
	return node;
}

past newContinueStmt() {
	past node = newAstNode();
	node->nodeType = CONTINUE_STMT;
	node->snodeType = "CONTINUE_STMT";
	return node;
}

past newBreakStmt() {
	past node = newAstNode();
	node->nodeType = BREAK_STMT;
	node->snodeType = "BREAK_STMT";
	return node;
}

past newReturnStmt(past left, past right) {
	past node = newAstNode();
	node->nodeType = RETURN_STMT;
	node->snodeType = "RETURN_STMT";
	node->left = left;
	node->right = right;
	return node;
}