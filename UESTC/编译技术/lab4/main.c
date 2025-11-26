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
        sprintf(result + strlen(result), "  %%%d = alloca i32, align 4\n", var_reg);
        
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
                    sprintf(result + strlen(result), "@%s = %s %s ", node->svalue, keyword, array_type);
                    
                    if (node->right->nodeType == INTEGER_LITERAL) {
                        sprintf(result + strlen(result), "[");
                        past init = node->right;
                        int first = 1;
                        while (init != NULL) {
                            if (init->nodeType == INTEGER_LITERAL) {
                                if (!first) strcat(result, ", ");
                                sprintf(result + strlen(result), "i32 %d", init->ivalue);
                                first = 0;
                                init = init->next;
                            } else {
                                // 跳过非整数字面量（嵌套列表等）
                                break;
                            }
                        }
                        sprintf(result + strlen(result), "], align 16\n");
                    } else {
                        // 复杂初始化，暂时用0填充
                        sprintf(result + strlen(result), "zeroinitializer, align 16\n");
                    }
                } else {
                    sprintf(result + strlen(result), "@%s = %s %s zeroinitializer, align 16\n", 
                            node->svalue, keyword, array_type);
                }
            } else {
                // 无初始化值
                sprintf(result + strlen(result), "@%s = %s %s zeroinitializer, align 16\n", 
                        node->svalue, keyword, array_type);
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
                sprintf(result + strlen(result), "@%s = %s i32 %d, align 4\n", node->svalue, keyword, node->right->ivalue);
            } else {
                sprintf(result + strlen(result), "@%s = %s i32 0, align 4\n", node->svalue, keyword);
            }
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
    char* llvm_type = "void";
        if (node->stype) {
            if (strcmp(node->stype, "int") == 0) {
                llvm_type = "i32";
            } else if (strcmp(node->stype, "float") == 0) {
                llvm_type = "float";
            }
        }
        sprintf(result + strlen(result), "\ndefine %s @%s(", llvm_type, node->svalue);
    
        past param = node->left;
        int first = 1;
        int head_param_idx = 0;
        while (param != NULL) {
            if (!first) {
                strcat(result, ", ");
            }
            char* p_type = "i32";
            if (param->stype && strcmp(param->stype, "float") == 0) p_type = "float";
            sprintf(result + strlen(result), "%s %%%d", p_type, head_param_idx++);
            first = 0;
            param = param->left;
        }
        
        strcat(result, ") {\n");
        
        extern int reg_count;
        reg_count = head_param_idx + 1; // 参数个数 + 1 (Entry Block 占用一个编号)
        
        // 处理参数：为每个参数生成 alloca 和 store，并添加到符号表
        param = node->left;
        int param_idx = 0;
        while (param != NULL) {
            int param_reg = reg_count++;
            // 转换类型
            char* param_llvm_type = "i32";
            if (param->stype && strcmp(param->stype, "float") == 0) {
                param_llvm_type = "float";
            }
            
            // alloca
            sprintf(result + strlen(result), "  %%%d = alloca %s, align 4\n", param_reg, param_llvm_type);
            
            // store: 使用匿名参数寄存器 %param_idx
            sprintf(result + strlen(result), "  store %s %%%d, %s* %%%d, align 4\n", 
                    param_llvm_type, param_idx, param_llvm_type, param_reg);
            
            // 添加到符号表
            addSymbol(param->svalue, param_reg);
            
            param = param->left;
            param_idx++;
        }
        
        int main_ret_var = -1;

        if (strcmp(node->svalue, "main") == 0 && strcmp(llvm_type, "i32") == 0) {
            main_ret_var = reg_count++;
            sprintf(result + strlen(result), "  %%%d = alloca i32, align 4\n", main_ret_var);
        }

        if (node->right != NULL && node->right->nodeType == COMPOUND_STMT) {
            past body = node->right->right;
            while (body != NULL) {
                generateAlloca(body->left, result);
                body = body->right;
            }
        }

        if (main_ret_var != -1) {
            sprintf(result + strlen(result), "  store i32 0, i32* %%%d, align 4\n", main_ret_var);
        }

        if (node->right != NULL && node->right->nodeType == COMPOUND_STMT) {
            past body = node->right->right;
            while (body != NULL) {
                genStmt(body->left, result);
                body = body->right;
            }
        }

        if (strcmp(llvm_type, "void") == 0) {
            sprintf(result + strlen(result), "  ret void\n");
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

    char* result = (char*)malloc(64 * 1024); 
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

