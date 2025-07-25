#include "rdlab2.h"
#include <stdbool.h>

// Function declarations
past newBinaryOper(int oper, past left, past right);
past newDeclRefExp(char *name, past left, past right);
past newWhileStmt(past condition, past body);
past newIfStmt(past condition, past ifBody, past elseBody);
past newBreakStmt();
past newContinueStmt();
past newReturnStmt(past left, past right);

past rd_add_exp();
past rd_mul_exp();
past rd_unary_exp();
past rd_primary_exp();
past rd_l_or_exp();
past rd_l_and_exp();
past rd_eq_exp();
past rd_call_paras();
past rd_relexp();
past rd_stmt();

// Function definitions

past newBinaryOper(int oper, past left, past right) {
    past node = newAstNode();
    if (node) {
        node->nodeType = BINARY_OPERATOR;
        node->ivalue = oper;
        node->left = left;
        node->right = right;
    }
    return node;
}

past newDeclRefExp(char *name, past left, past right) {  
    past node = newAstNode();
    if (node) {
        node->nodeType = DECL_REF_EXPR;
        node->svalue = name;
        node->left = left;
        node->right = right;
    }
    return node;
}

past newWhileStmt(past condition, past body) {
    past node = newAstNode();
    if (node) {
        node->nodeType = WHILE_STMT;
        node->left = condition;
        node->right = body;
    }
    return node;
}

past newIfStmt(past condition, past ifBody, past elseBody) {
    past node = newAstNode();
    if (node) {
        node->nodeType = IF_STMT;
        node->if_cond = condition;
        node->left = ifBody;
        node->right = elseBody;
    }
    return node;
}

past newBreakStmt() {   
    past node = newAstNode();
    if (node) {
        node->nodeType = BREAK_STMT;
    }
    return node;
}

past newContinueStmt() {
    past node = newAstNode();
    if (node) {
        node->nodeType = CONTINUE_STMT;
    }
    return node;
}

past newReturnStmt(past left, past right) {
    past node = newAstNode();
    if (node) {
        node->nodeType = RETURN_STMT;
        node->left = left;
        node->right = right;
    }
    return node;
}

past rd_primary_exp() {
    past node = NULL;

    if (cur_token.token == Y_LPAR) {
        advance(); 
        node = rd_add_exp();
        if (cur_token.token != Y_RPAR) {
            return NULL; 
        }
        advance(); 
    } else if (cur_token.token == Y_ID) {
        char *s = cur_token.attr.svalue;
        past Arr = rd_array_subscripts();
        node = newDeclRefExp(s, Arr, NULL);
        advance();
    } else if (cur_token.token == num_INT) { 
        node = newInt(cur_token.attr.ivalue);
        advance(); 
    } else if (cur_token.token == num_FLOAT) { 
        node = newAstNode();
        advance(); 
        node->fvalue = cur_token.attr.fvalue;
    }

    return node;
}

past rd_unary_exp() {
    past node = rd_primary_exp();

    while (node == NULL) {
        if (cur_token.token == Y_ID) {
            char *s = cur_token.attr.svalue;
            advance();
            if (cur_token.token == Y_LPAR) {
                advance();
                past params;
                if (cur_token.token != Y_RPAR) {
                    params = rd_call_paras();
                }
                node = newDeclRefExp(s, params, NULL);
            }
        } else if (cur_token.token == Y_ADD || cur_token.token == Y_SUB || cur_token.token == Y_NOT) {
            int oper = cur_token.token;
            advance(); 
            past operand = rd_unary_exp();
            node = newBinaryOper(oper, NULL, operand);
        } else {
            return NULL;
        }
    }

    return node;
}

past rd_add_exp() {
    past left = rd_mul_exp();

    while (cur_token.token == Y_ADD || cur_token.token == Y_SUB) {
        int oper = cur_token.token;
        advance(); 
        past right = rd_mul_exp();
        left = newBinaryOper(oper, left, right);
    }

    return left;
}

past rd_mul_exp() {
    past left = rd_unary_exp();

    while (cur_token.token == Y_MUL || cur_token.token == Y_DIV || cur_token.token == Y_MODULO) {
        int oper = cur_token.token;
        advance(); 
        past right = rd_unary_exp();
        left = newBinaryOper(oper, left, right);
    }

    return left;
}

past rd_eq_exp() {
    past left = rd_relexp();

    while (cur_token.token == Y_EQ || cur_token.token == Y_NOTEQ) {
        int oper = cur_token.token;
        advance(); 
        past right = rd_relexp();
        left = newBinaryOper(oper, left, right);
    }

    return left;
}

past rd_relexp() {
    past left = rd_add_exp();

    while (true) {
        switch (cur_token.token) {
            case Y_LESS:
            case Y_LESSEQ:
            case Y_GREAT:
            case Y_GREATEQ: {
                int oper = cur_token.token;
                advance(); 
                past right = rd_add_exp();
                left = newBinaryOper(oper, left, right);
                break;
            }
            default:
                return left;
        }
    }
}

past rd_l_and_exp() {
    past left = rd_eq_exp();

    while (cur_token.token == Y_AND) {
        int oper = cur_token.token;
        advance(); 
        past right = rd_eq_exp();
        left = newBinaryOper(oper, left, right);
    }

    return left;
}

past rd_l_or_exp() {
    past left = rd_l_and_exp();

    while (cur_token.token == Y_OR) {
        int oper = cur_token.token;
        advance(); 
        past right = rd_l_and_exp();
        left = newBinaryOper(oper, left, right);
    }

    return left;
}

past rd_call_paras() {
    past head = rd_add_exp();
    past current = head;

    while (cur_token.token) {
        if (cur_token.token != Y_COMMA) {
            break;
        }
        advance(); 
        past new_node = rd_add_exp();
        current->next = new_node;
        current = current->next;
    }

    return head;
}

past rd_stmt() {
    switch (cur_token.token) {
        case Y_ID: {
            char *s = cur_token.attr.svalue;
            past Arr = rd_array_subscripts();
            past lval = newDeclRefExp(s, Arr, NULL);
            advance(); 
            if (cur_token.token != Y_ASSIGN) {
                return NULL; 
            }
            advance(); 
            past left = rd_add_exp();
            if (cur_token.token != Y_SEMICOLON) {
                return NULL; 
            }
            advance(); 
            return newBinaryOper(Y_ASSIGN, lval, left);
        }
        case Y_SEMICOLON: {
            advance(); 
            return NULL; 
        }
        case Y_LBRACKET: {
            advance(); 
            past block = rd_block();
            if (cur_token.token != Y_RBRACKET) {
                return NULL; 
            }
            advance(); 
            return block;
        }
        case Y_WHILE: {
            advance(); 
            if (cur_token.token != Y_LPAR) {
                return NULL; 
            }
            advance(); 
            past condition = rd_l_or_exp();
            if (cur_token.token != Y_RPAR) {
                return NULL; 
            }
            advance(); 
            past stmt = rd_stmt();
            return newWhileStmt(condition, stmt);
        }
        case Y_IF: {
            advance(); 
            if (cur_token.token != Y_LPAR) {
                return NULL;
            }
            advance(); 
            past condition = rd_l_or_exp();
            if (cur_token.token != Y_RPAR) {
                return NULL; 
            }
            advance(); 
            past if_stmt = rd_stmt();
            if (cur_token.token != Y_ELSE) {
                return newIfStmt(condition, if_stmt, NULL);
            }
            advance(); 
            past else_stmt = rd_stmt();
            return newIfStmt(condition, if_stmt, else_stmt);
        }
        case Y_BREAK: {
            advance(); 
            if (cur_token.token != Y_SEMICOLON) {
                return NULL; 
            }
            advance(); 
            return newBreakStmt();
        }
        case Y_CONTINUE: {
            advance(); 
            if (cur_token.token != Y_SEMICOLON) {
                return NULL; 
            }
            advance(); 
            return newContinueStmt();
        }
        case Y_RETURN: {
            advance(); 
            past left = rd_add_exp();
            if (cur_token.token != Y_SEMICOLON) {
                return NULL;
            }
            advance();
            return newReturnStmt(left, NULL);
        }
        default:
            return NULL;
    }
}