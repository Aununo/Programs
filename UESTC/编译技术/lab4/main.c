#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"

void yyerror(char *s)
{
    printf("%s\n", s);
}

extern past root;
int yyparse();

void generateAlloca(past node, char* result) {
    if (node == NULL) {
        return;
    }
    
    char temp[1024];
    
    if (node->nodeType == DECL_STMT) {
        if (node->left != NULL) {
            generateAlloca(node->left, result);
        }
        if (node->right != NULL) {
            generateAlloca(node->right, result);
        }
    } else if (node->nodeType == VAR_DECL) {
        extern int reg_count;
        int var_reg = reg_count++;
        sprintf(temp, "  %%%d = alloca i32, align 4\n", var_reg);
        strcat(result, temp);
        
        if (node->if_const && node->right != NULL && node->right->nodeType == INTEGER_LITERAL) {
            // const 变量，记录常量值
            addConstSymbol(node->svalue, var_reg, node->right->ivalue);
        } else {
            addSymbol(node->svalue, var_reg);
        }
    } else if (node->nodeType == COMPOUND_STMT) {
        past stmt = node->right;
        while (stmt != NULL) {
            generateAlloca(stmt->left, result);
            stmt = stmt->right;
        }
    }
}

void generateGlobalDecl(past node, char* result) {
    if (node == NULL) {
        return;
    }
    
    char temp[1024];
    
    if (node->nodeType == VAR_DECL) {
        int is_const = node->if_const;
        char* keyword = is_const ? "constant" : "global";
        
        // 检查是否是数组
        if (node->left != NULL && node->left->nodeType == INTEGER_LITERAL) {
            // 收集所有维度
            int dims[10];
            int dim_count = 0;
            past dim_node = node->left;

            while (dim_node != NULL && dim_node->nodeType == INTEGER_LITERAL && dim_count < 10) {
                dims[dim_count++] = dim_node->ivalue;
                dim_node = dim_node->next;
            }
            
            // 生成数组类型字符串
            char array_type[256];
            char* type_ptr = array_type;
            for (int i = dim_count - 1; i >= 0; i--) {
                type_ptr += sprintf(type_ptr, "[%d x ", dims[i]);
            }
            type_ptr += sprintf(type_ptr, "i32");
            for (int i = 0; i < dim_count; i++) {
                type_ptr += sprintf(type_ptr, "]");
            }
            
            // 添加符号到符号表
            if (is_const) {
                addGlobalConstSymbolWithType(node->svalue, array_type);
            } else {
                addGlobalSymbolWithType(node->svalue, array_type);
            }
            
            // 处理数组初始化
            if (node->right != NULL) {
                // 有初始化值 - 需要处理初始化列表
                if (is_const) {
                    // const 数组需要显式初始化
                    sprintf(temp, "@%s = %s %s ", node->svalue, keyword, array_type);
                    strcat(result, temp);
                    
                    if (node->right->nodeType == INTEGER_LITERAL) {
                        sprintf(temp, "[");
                        strcat(result, temp);
                        past init = node->right;
                        int first = 1;
                        while (init != NULL) {
                            if (init->nodeType == INTEGER_LITERAL) {
                                if (!first) strcat(result, ", ");
                                sprintf(temp, "i32 %d", init->ivalue);
                                strcat(result, temp);
                                first = 0;
                                init = init->next;
                            } else {
                                // 跳过非整数字面量（嵌套列表等）
                                break;
                            }
                        }
                        sprintf(temp, "], align 16\n");
                        strcat(result, temp);
                    } else {
                        // 复杂初始化，暂时用0填充
                        sprintf(temp, "zeroinitializer, align 16\n");
                        strcat(result, temp);
                    }
                } else {
                    sprintf(temp, "@%s = %s %s zeroinitializer, align 16\n", 
                            node->svalue, keyword, array_type);
                    strcat(result, temp);
                }
            } else {
                // 无初始化值
                sprintf(temp, "@%s = %s %s zeroinitializer, align 16\n", 
                        node->svalue, keyword, array_type);
                strcat(result, temp);
            }
        } else {
            // 普通变量
            if (is_const) {
                // const 变量，如果有初始值，记录常量值
                if (node->right != NULL && node->right->nodeType == INTEGER_LITERAL) {
                    addGlobalConstSymbolWithValue(node->svalue, node->right->ivalue);
                } else {
                    addGlobalConstSymbol(node->svalue);
                }
            } else {
                addGlobalSymbol(node->svalue);
            }
            
            if (node->right != NULL && node->right->nodeType == INTEGER_LITERAL) {
                sprintf(temp, "@%s = %s i32 %d, align 4\n", node->svalue, keyword, node->right->ivalue);
            } else {
                sprintf(temp, "@%s = %s i32 0, align 4\n", node->svalue, keyword);
            }
            strcat(result, temp);
        }
        return;
    }
    
    if (node->nodeType == DECL_STMT) {
        if (node->left != NULL) {
            generateGlobalDecl(node->left, result);
        }
        if (node->right != NULL) {
            generateGlobalDecl(node->right, result);
        }
        return;
    }
}

void generateCode(past node, char* result) {
    if (node == NULL) {
        return;
    }
    
    if (node->nodeType == TRANSLATION_UNIT) {
        past current = node;
        while (current != NULL) {
            if (current->left != NULL && current->left->nodeType != FUNCTION_DECL) {
                generateGlobalDecl(current->left, result);
            }
            current = current->right;
        }
        
        current = node;
        while (current != NULL) {
            if (current->left != NULL && current->left->nodeType == FUNCTION_DECL) {
                generateCode(current->left, result);
            }
            current = current->right;
        }
        return;
    }
    
    if (node->nodeType == FUNCTION_DECL) {
        char temp[1024];
        char* llvm_type = "void";
        if (node->stype) {
            if (strcmp(node->stype, "int") == 0) {
                llvm_type = "i32";
            } else if (strcmp(node->stype, "float") == 0) {
                llvm_type = "float";
            }
        }
        sprintf(temp, "\ndefine %s @%s(", llvm_type, node->svalue);
        strcat(result, temp);
    
        past param = node->left;
        int first = 1;
        while (param != NULL) {
            if (!first) {
                strcat(result, ", ");
            }
            sprintf(temp, "%s %%p%s", param->stype, param->svalue);
            strcat(result, temp);
            first = 0;
            param = param->left;
        }
        
        strcat(result, ") {\n");
        
        extern int reg_count;
        int main_ret_var = -1;

        if (strcmp(node->svalue, "main") == 0 && strcmp(llvm_type, "i32") == 0) {
            main_ret_var = reg_count++;
            sprintf(temp, "  %%%d = alloca i32, align 4\n", main_ret_var);
            strcat(result, temp);
        }

        if (node->right != NULL && node->right->nodeType == COMPOUND_STMT) {
            past body = node->right->right;
            while (body != NULL) {
                generateAlloca(body->left, result);
                body = body->right;
            }
        }

        if (main_ret_var != -1) {
            sprintf(temp, "  store i32 0, i32* %%%d, align 4\n", main_ret_var);
            strcat(result, temp);
        }

        if (node->right != NULL && node->right->nodeType == COMPOUND_STMT) {
            past body = node->right->right;
            while (body != NULL) {
                genStmt(body->left, result);
                body = body->right;
            }
        }

        if (strcmp(llvm_type, "void") == 0) {
            sprintf(temp, "  ret void\n");
            strcat(result, temp);
        }
        
        strcat(result, "}\n");
        return;
    }

    genStmt(node, result);
}

int main(int argc, char* argv[]) {

    if (argc > 1) {
        freopen(argv[1], "r", stdin);
    }

    yyparse();

    char* result = (char*)malloc(1024 * 1024); 
    if (result == NULL) {
        fprintf(stderr, "内存分配失败\n");
        return 1;
    }
    result[0] = '\0';

    extern past root;
    generateCode(root, result);

    printf("%s", result);

    free(result);
    freeSymbols();
    
    return 0;
}

