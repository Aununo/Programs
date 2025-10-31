#include "codegen.h"

int reg_count = 1;      
int label_count = 0;      
pSymbol symbol_table = NULL;      

typedef struct {
    int value;          
    int is_immediate;   
} ExprResult;

ExprResult genExpr(past node, char* result);

pSymbol createSymbol(char* name, int reg_num) {
    pSymbol sym = (pSymbol)malloc(sizeof(Symbol));
    sym->name = (char*)malloc(strlen(name) + 1);
    strcpy(sym->name, name);
    sym->reg_num = reg_num;
    sym->is_global = 0;
    sym->is_const = 0;
    sym->const_value = 0;
    sym->has_const_value = 0;
    sym->array_type = NULL;
    sym->next = NULL;
    return sym;
}

void addSymbol(char* name, int reg_num) {
    pSymbol sym = createSymbol(name, reg_num);
    sym->next = symbol_table;
    symbol_table = sym;
}

void addConstSymbol(char* name, int reg_num, int const_value) {
    pSymbol sym = createSymbol(name, reg_num);
    sym->is_const = 1;
    sym->const_value = const_value;
    sym->has_const_value = 1;
    sym->next = symbol_table;
    symbol_table = sym;
}

void addGlobalSymbol(char* name) {
    pSymbol sym = createSymbol(name, 0);
    sym->is_global = 1;
    sym->next = symbol_table;
    symbol_table = sym;
}

void addGlobalSymbolWithType(char* name, char* array_type) {
    pSymbol sym = createSymbol(name, 0);
    sym->is_global = 1;
    if (array_type) {
        sym->array_type = (char*)malloc(strlen(array_type) + 1);
        strcpy(sym->array_type, array_type);
    }
    sym->next = symbol_table;
    symbol_table = sym;
}

void addGlobalConstSymbol(char* name) {
    pSymbol sym = createSymbol(name, 0);
    sym->is_global = 1;
    sym->is_const = 1;
    sym->next = symbol_table;
    symbol_table = sym;
}

void addGlobalConstSymbolWithValue(char* name, int const_value) {
    pSymbol sym = createSymbol(name, 0);
    sym->is_global = 1;
    sym->is_const = 1;
    sym->const_value = const_value;
    sym->has_const_value = 1;
    sym->next = symbol_table;
    symbol_table = sym;
}

void addGlobalConstSymbolWithType(char* name, char* array_type) {
    pSymbol sym = createSymbol(name, 0);
    sym->is_global = 1;
    sym->is_const = 1;
    if (array_type) {
        sym->array_type = (char*)malloc(strlen(array_type) + 1);
        strcpy(sym->array_type, array_type);
    }
    sym->next = symbol_table;
    symbol_table = sym;
}

int findSymbol(char* name) {
    pSymbol current = symbol_table;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current->reg_num;
        }
        current = current->next;
    }
    return -1;  
}

pSymbol findSymbolPtr(char* name) {
    pSymbol current = symbol_table;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void freeSymbols() {
    pSymbol current = symbol_table;
    while (current != NULL) {
        pSymbol temp = current;
        current = current->next;
        free(temp->name);
        free(temp);
    }
    symbol_table = NULL;
}

int newRegister() {
    return reg_count++;
}

int newLabel() {
    // 使用寄存器计数器来生成标签，保持一致性
    return reg_count++;
}

ExprResult genExpr(past node, char* result) {
    ExprResult res;
    char temp[1024];
    
    if (node == NULL) {
        res.is_immediate = 1;
        res.value = 0;
        return res;
    }
    
    if (node->nodeType == INTEGER_LITERAL) {
        res.is_immediate = 1;
        res.value = node->ivalue;
        return res;
    }
    
    if (node->nodeType == FLOATING_LITERAL) {
        int reg = newRegister();
        sprintf(temp, "  %%%d = fadd float 0.0, %f\n", reg, node->fvalue);
        strcat(result, temp);
        res.is_immediate = 0;
        res.value = reg;
        return res;
    }
    
    if (node->nodeType == CALL_EXPR) {
        // 函数调用 - 首先计算所有参数
        ExprResult arg_results[10];
        int arg_count = 0;
        past arg = node->left;
        while (arg != NULL && arg_count < 10) {
            arg_results[arg_count++] = genExpr(arg, result);
            arg = arg->right;
        }
        
        // 然后生成函数调用
        int call_reg = newRegister();
        sprintf(temp, "  %%%d = call i32 @%s(", call_reg, node->svalue);
        strcat(result, temp);
        
        for (int i = 0; i < arg_count; i++) {
            if (i > 0) {
                strcat(result, ", ");
            }
            if (arg_results[i].is_immediate) {
                sprintf(temp, "i32 %d", arg_results[i].value);
            } else {
                sprintf(temp, "i32 %%%d", arg_results[i].value);
            }
            strcat(result, temp);
        }
        
        strcat(result, ")\n");
        res.is_immediate = 0;
        res.value = call_reg;
        return res;
    }
    
    if (node->nodeType == DECL_REF_EXPR) {
        pSymbol sym = findSymbolPtr(node->svalue);
        if (sym != NULL) {
            // 如果是 const 变量且有常量值，直接返回常量值（常量传播优化）
            if (sym->is_const && sym->has_const_value && node->left == NULL) {
                res.is_immediate = 1;
                res.value = sym->const_value;
                return res;
            }
            
            // 检查是否有数组下标访问
            if (node->left != NULL && sym->is_global) {
                // 数组下标访问
                // 收集所有下标
                int indices[10];
                int index_regs[10];
                int index_count = 0;
                
                past subscript = node->left;
                while (subscript != NULL && index_count < 10) {
                    if (subscript->nodeType == ARRAY_SUBSCRIPT_EXPR) {
                        // 多维数组
                        ExprResult idx_res = genExpr(subscript->left, result);
                        if (idx_res.is_immediate) {
                            indices[index_count] = idx_res.value;
                            index_regs[index_count] = -1;
                        } else {
                            index_regs[index_count] = idx_res.value;
                            indices[index_count] = -1;
                        }
                        index_count++;
                        subscript = subscript->right;
                    } else {
                        // 单维数组或最后一维
                        ExprResult idx_res = genExpr(subscript, result);
                        if (idx_res.is_immediate) {
                            indices[index_count] = idx_res.value;
                            index_regs[index_count] = -1;
                        } else {
                            index_regs[index_count] = idx_res.value;
                            indices[index_count] = -1;
                        }
                        index_count++;
                        break;
                    }
                }
                
                // 生成 getelementptr 内联在 load 中
                // 格式：load i32, i32* getelementptr inbounds ([type], [type]* @var, i64 0, i64 idx)
                
                int load_reg = newRegister();
                if (sym->array_type) {
                    sprintf(temp, "  %%%d = load i32, i32* getelementptr inbounds (%s, %s* @%s, i64 0, i64 %d), align 16\n", 
                            load_reg, sym->array_type, sym->array_type, node->svalue, indices[0]);
                } else {
                    // 默认为简单数组
                    sprintf(temp, "  %%%d = load i32, i32* getelementptr inbounds ([5 x i32], [5 x i32]* @%s, i64 0, i64 %d), align 16\n", 
                            load_reg, node->svalue, indices[0]);
                }
                strcat(result, temp);
                
                res.is_immediate = 0;
                res.value = load_reg;
                return res;
            } else {
                // 普通变量访问
                int load_reg = newRegister();
                if (sym->is_global) {
                    sprintf(temp, "  %%%d = load i32, i32* @%s, align 4\n", load_reg, node->svalue);
                } else {
                    sprintf(temp, "  %%%d = load i32, i32* %%%d, align 4\n", load_reg, sym->reg_num);
                }
                strcat(result, temp);
                res.is_immediate = 0;
                res.value = load_reg;
                return res;
            }
        }
        res.is_immediate = 1;
        res.value = 0;
        return res;
    }
    
    if (node->nodeType == BINARY_OPERATOR) {
        int op = node->ivalue;
        
        if (node->left == NULL && node->right != NULL) {
            ExprResult right_res = genExpr(node->right, result);
            
            if (op == Y_SUB) {  
                // 一元负号：如果操作数是立即数，直接返回负值（常量折叠）
                if (right_res.is_immediate) {
                    res.is_immediate = 1;
                    res.value = -right_res.value;
                    return res;
                } else {
                    int result_reg = newRegister();
                    sprintf(temp, "  %%%d = sub nsw i32 0, %%%d\n", result_reg, right_res.value);
                    strcat(result, temp);
                    res.is_immediate = 0;
                    res.value = result_reg;
                    return res;
                }
            } else if (op == Y_ADD) {  
                return right_res;
            }
        }
        
        ExprResult left_res = genExpr(node->left, result);
        ExprResult right_res = genExpr(node->right, result);
        
        // 常量折叠：如果两个操作数都是立即数，直接计算结果
        if (left_res.is_immediate && right_res.is_immediate) {
            int left_val = left_res.value;
            int right_val = right_res.value;
            
            switch (op) {
                case Y_ADD:
                    res.is_immediate = 1;
                    res.value = left_val + right_val;
                    return res;
                case Y_SUB:
                    res.is_immediate = 1;
                    res.value = left_val - right_val;
                    return res;
                case Y_MUL:
                    res.is_immediate = 1;
                    res.value = left_val * right_val;
                    return res;
                case Y_DIV:
                    if (right_val != 0) {
                        res.is_immediate = 1;
                        res.value = left_val / right_val;
                        return res;
                    }
                    break;
                case Y_MODULO:
                    if (right_val != 0) {
                        res.is_immediate = 1;
                        res.value = left_val % right_val;
                        return res;
                    }
                    break;
                case Y_LESS:
                    res.is_immediate = 1;
                    res.value = left_val < right_val;
                    return res;
                case Y_GREAT:
                    res.is_immediate = 1;
                    res.value = left_val > right_val;
                    return res;
                case Y_LESSEQ:
                    res.is_immediate = 1;
                    res.value = left_val <= right_val;
                    return res;
                case Y_GREATEQ:
                    res.is_immediate = 1;
                    res.value = left_val >= right_val;
                    return res;
                case Y_EQ:
                    res.is_immediate = 1;
                    res.value = left_val == right_val;
                    return res;
                case Y_NOTEQ:
                    res.is_immediate = 1;
                    res.value = left_val != right_val;
                    return res;
            }
        }
        
        int result_reg = newRegister();
        
        if (op == Y_LESS || op == Y_GREAT || op == Y_LESSEQ || op == Y_GREATEQ ||
            op == Y_EQ || op == Y_NOTEQ) {
            int cmp_reg = result_reg;
            result_reg = newRegister();
            
            char* cmp_op = "";
            switch(op) {
                case Y_LESS: cmp_op = "slt"; break;
                case Y_GREAT: cmp_op = "sgt"; break;
                case Y_LESSEQ: cmp_op = "sle"; break;
                case Y_GREATEQ: cmp_op = "sge"; break;
                case Y_EQ: cmp_op = "eq"; break;
                case Y_NOTEQ: cmp_op = "ne"; break;
            }
            
            if (left_res.is_immediate && right_res.is_immediate) {
                sprintf(temp, "  %%%d = icmp %s i32 %d, %d\n", cmp_reg, cmp_op, left_res.value, right_res.value);
            } else if (left_res.is_immediate) {
                sprintf(temp, "  %%%d = icmp %s i32 %d, %%%d\n", cmp_reg, cmp_op, left_res.value, right_res.value);
            } else if (right_res.is_immediate) {
                sprintf(temp, "  %%%d = icmp %s i32 %%%d, %d\n", cmp_reg, cmp_op, left_res.value, right_res.value);
            } else {
                sprintf(temp, "  %%%d = icmp %s i32 %%%d, %%%d\n", cmp_reg, cmp_op, left_res.value, right_res.value);
            }
            strcat(result, temp);
            sprintf(temp, "  %%%d = zext i1 %%%d to i32\n", result_reg, cmp_reg);
            strcat(result, temp);
            res.is_immediate = 0;
            res.value = result_reg;
            return res;
        }
        
        if (op == Y_AND || op == Y_OR) {
            int left_bool = newRegister();
            int right_bool = newRegister();
            int and_or_reg = newRegister();
            result_reg = newRegister();
            
            if (left_res.is_immediate) {
                sprintf(temp, "  %%%d = icmp ne i32 %d, 0\n", left_bool, left_res.value);
            } else {
                sprintf(temp, "  %%%d = icmp ne i32 %%%d, 0\n", left_bool, left_res.value);
            }
            strcat(result, temp);
            
            if (right_res.is_immediate) {
                sprintf(temp, "  %%%d = icmp ne i32 %d, 0\n", right_bool, right_res.value);
            } else {
                sprintf(temp, "  %%%d = icmp ne i32 %%%d, 0\n", right_bool, right_res.value);
            }
            strcat(result, temp);
            
            sprintf(temp, "  %%%d = %s i1 %%%d, %%%d\n", and_or_reg, 
                    op == Y_AND ? "and" : "or", left_bool, right_bool);
            strcat(result, temp);
            sprintf(temp, "  %%%d = zext i1 %%%d to i32\n", result_reg, and_or_reg);
            strcat(result, temp);
            res.is_immediate = 0;
            res.value = result_reg;
            return res;
        }
        
        char* op_str = "";
        int use_nsw = 0;  
        switch (op) {
            case Y_ADD: 
                op_str = "add"; 
                use_nsw = 1;
                break;
            case Y_SUB: 
                op_str = "sub"; 
                use_nsw = 1;
                break;
            case Y_MUL: 
                op_str = "mul"; 
                use_nsw = 1;
                break;
            case Y_DIV: 
                op_str = "sdiv"; 
                break;
            case Y_MODULO: 
                op_str = "srem"; 
                break;
            default: 
                res.is_immediate = 1;
                res.value = 0;
                return res;
        }
        
        char nsw_flag[8] = "";
        if (use_nsw) {
            strcpy(nsw_flag, " nsw");
        }
        
        if (left_res.is_immediate && right_res.is_immediate) {
            sprintf(temp, "  %%%d = %s%s i32 %d, %d\n", result_reg, op_str, nsw_flag, left_res.value, right_res.value);
        } else if (left_res.is_immediate) {
            sprintf(temp, "  %%%d = %s%s i32 %d, %%%d\n", result_reg, op_str, nsw_flag, left_res.value, right_res.value);
        } else if (right_res.is_immediate) {
            sprintf(temp, "  %%%d = %s%s i32 %%%d, %d\n", result_reg, op_str, nsw_flag, left_res.value, right_res.value);
        } else {
            sprintf(temp, "  %%%d = %s%s i32 %%%d, %%%d\n", result_reg, op_str, nsw_flag, left_res.value, right_res.value);
        }
        strcat(result, temp);
        res.is_immediate = 0;
        res.value = result_reg;
        return res;
    }
    
    res.is_immediate = 1;
    res.value = 0;
    return res;
}

void genArithmeticExpr(past node, char* result) {
    genExpr(node, result);
}

void genLogicExpr(past node, char* result) {
    genExpr(node, result);
}

void genAssignStmt(past node, char* result) {
    if (node == NULL || node->nodeType != BINARY_OPERATOR || node->ivalue != Y_ASSIGN) {
        return;
    }
    
    char temp[1024];
    
    if (node->left->nodeType == DECL_REF_EXPR) {
        char* var_name = node->left->svalue;
        pSymbol sym = findSymbolPtr(var_name);
        
        if (sym == NULL) {
            // 如果符号不存在，创建局部变量
            int var_reg = newRegister();
            sprintf(temp, "  %%%d = alloca i32, align 4\n", var_reg);
            strcat(result, temp);
            addSymbol(var_name, var_reg);
            
            ExprResult right_res = genExpr(node->right, result);
            
            if (right_res.is_immediate) {
                sprintf(temp, "  store i32 %d, i32* %%%d, align 4\n", right_res.value, var_reg);
            } else {
                sprintf(temp, "  store i32 %%%d, i32* %%%d, align 4\n", right_res.value, var_reg);
            }
            strcat(result, temp);
        } else {
            // 符号已存在
            ExprResult right_res = genExpr(node->right, result);
            
            if (sym->is_global) {
                // 全局变量
                if (right_res.is_immediate) {
                    sprintf(temp, "  store i32 %d, i32* @%s, align 4\n", right_res.value, var_name);
                } else {
                    sprintf(temp, "  store i32 %%%d, i32* @%s, align 4\n", right_res.value, var_name);
                }
            } else {
                // 局部变量
                if (right_res.is_immediate) {
                    sprintf(temp, "  store i32 %d, i32* %%%d, align 4\n", right_res.value, sym->reg_num);
                } else {
                    sprintf(temp, "  store i32 %%%d, i32* %%%d, align 4\n", right_res.value, sym->reg_num);
                }
            }
            strcat(result, temp);
        }
    }
}

// 生成短路求值的条件表达式，返回用于 br 的 i1 寄存器
int genCondExpr(past node, char* result, int true_label, int false_label) {
    char temp[1024];
    
    if (node == NULL) {
        return -1;
    }
    
    // 处理 || 运算符（短路求值）
    if (node->nodeType == BINARY_OPERATOR && node->ivalue == Y_OR) {
        int mid_label = newLabel();
        
        // 对于 A || B：如果 A 为真，跳转到 true_label；否则继续检查 B
        genCondExpr(node->left, result, true_label, mid_label);
        
        sprintf(temp, "%d:\n", mid_label);
        strcat(result, temp);
        
        genCondExpr(node->right, result, true_label, false_label);
        return -1;
    }
    
    // 处理 && 运算符（短路求值）
    if (node->nodeType == BINARY_OPERATOR && node->ivalue == Y_AND) {
        int mid_label = newLabel();
        
        // 对于 A && B：如果 A 为假，跳转到 false_label；否则继续检查 B
        genCondExpr(node->left, result, mid_label, false_label);
        
        sprintf(temp, "%d:\n", mid_label);
        strcat(result, temp);
        
        genCondExpr(node->right, result, true_label, false_label);
        return -1;
    }
    
    // 对于比较运算符，直接生成 icmp 和 br，不使用 zext
    if (node->nodeType == BINARY_OPERATOR) {
        int op = node->ivalue;
        
        if (op == Y_LESS || op == Y_GREAT || op == Y_LESSEQ || op == Y_GREATEQ ||
            op == Y_EQ || op == Y_NOTEQ) {
            ExprResult left_res = genExpr(node->left, result);
            ExprResult right_res = genExpr(node->right, result);
            
            int cmp_reg = newRegister();
            
            char* cmp_op = "";
            switch(op) {
                case Y_LESS: cmp_op = "slt"; break;
                case Y_GREAT: cmp_op = "sgt"; break;
                case Y_LESSEQ: cmp_op = "sle"; break;
                case Y_GREATEQ: cmp_op = "sge"; break;
                case Y_EQ: cmp_op = "eq"; break;
                case Y_NOTEQ: cmp_op = "ne"; break;
            }
            
            if (left_res.is_immediate && right_res.is_immediate) {
                sprintf(temp, "  %%%d = icmp %s i32 %d, %d\n", cmp_reg, cmp_op, left_res.value, right_res.value);
            } else if (left_res.is_immediate) {
                sprintf(temp, "  %%%d = icmp %s i32 %d, %%%d\n", cmp_reg, cmp_op, left_res.value, right_res.value);
            } else if (right_res.is_immediate) {
                sprintf(temp, "  %%%d = icmp %s i32 %%%d, %d\n", cmp_reg, cmp_op, left_res.value, right_res.value);
            } else {
                sprintf(temp, "  %%%d = icmp %s i32 %%%d, %%%d\n", cmp_reg, cmp_op, left_res.value, right_res.value);
            }
            strcat(result, temp);
            
            sprintf(temp, "  br i1 %%%d, label %%%d, label %%%d\n", cmp_reg, true_label, false_label);
            strcat(result, temp);
            
            return cmp_reg;
        }
    }
    
    // 其他表达式：计算结果并生成条件跳转
    ExprResult cond_res = genExpr(node, result);
    
    int cond_bool_reg = newRegister();
    if (cond_res.is_immediate) {
        sprintf(temp, "  br i1 %d, label %%%d, label %%%d\n", 
                cond_res.value != 0, true_label, false_label);
        strcat(result, temp);
    } else {
        sprintf(temp, "  %%%d = icmp ne i32 %%%d, 0\n", cond_bool_reg, cond_res.value);
        strcat(result, temp);
        
        sprintf(temp, "  br i1 %%%d, label %%%d, label %%%d\n", 
                cond_bool_reg, true_label, false_label);
        strcat(result, temp);
    }
    
    return cond_bool_reg;
}

void genIfStmt(past node, char* result) {
    if (node == NULL || node->nodeType != IF_STMT) {
        return;
    }
    
    char temp[1024];
    
    int then_label = newLabel();
    int else_label = newLabel();
    int end_label = newLabel();
    
    if (node->right != NULL) {  
        genCondExpr(node->if_cond, result, then_label, else_label);
    } else {  
        genCondExpr(node->if_cond, result, then_label, end_label);
    }
    
    sprintf(temp, "%d:\n", then_label);
    strcat(result, temp);
    
    // 处理 then 分支
    if (node->left != NULL) {
        if (node->left->nodeType == COMPOUND_STMT) {
            // 检查是包装的单个语句还是 BlockItems 链表
            if (node->left->left == NULL && node->left->right != NULL) {
                // 包装的单个语句：newCompoundStmt(NULL, stmt)
                genStmt(node->left->right, result);
            } else {
                // BlockItems 链表
                past stmt = node->left->right;
                while (stmt != NULL) {
                    genStmt(stmt->left, result);
                    stmt = stmt->right;
                }
            }
        } else {
            genStmt(node->left, result);
        }
    }
    
    sprintf(temp, "  br label %%%d\n", end_label);
    strcat(result, temp);
    
    // 处理 else 分支
    if (node->right != NULL) {
        sprintf(temp, "%d:\n", else_label);
        strcat(result, temp);
        
        if (node->right->nodeType == COMPOUND_STMT) {
            // 检查是包装的单个语句还是 BlockItems 链表
            if (node->right->left == NULL && node->right->right != NULL) {
                // 包装的单个语句：newCompoundStmt(NULL, stmt)
                genStmt(node->right->right, result);
            } else {
                // BlockItems 链表
                past stmt = node->right->right;
                while (stmt != NULL) {
                    genStmt(stmt->left, result);
                    stmt = stmt->right;
                }
            }
        } else if (node->right->nodeType == IF_STMT) {
            // else if 情况
            genStmt(node->right, result);
        } else {
            genStmt(node->right, result);
        }
        
        sprintf(temp, "  br label %%%d\n", end_label);
        strcat(result, temp);
    }

    sprintf(temp, "%d:\n", end_label);
    strcat(result, temp);
}


void genWhileStmt(past node, char* result) {
    if (node == NULL || node->nodeType != WHILE_STMT) {
        return;
    }
    
    char temp[1024];

    int cond_label = newLabel();
    int body_label = newLabel();
    int end_label = newLabel();

    sprintf(temp, "  br label %%%d\n", cond_label);
    strcat(result, temp);

    sprintf(temp, "%d:\n", cond_label);
    strcat(result, temp);

    genCondExpr(node->left, result, body_label, end_label);

    sprintf(temp, "%d:\n", body_label);
    strcat(result, temp);

    // 处理循环体
    if (node->right != NULL) {
        if (node->right->nodeType == COMPOUND_STMT) {
            // 检查是包装的单个语句还是 BlockItems 链表
            if (node->right->left == NULL && node->right->right != NULL) {
                // 包装的单个语句：newCompoundStmt(NULL, stmt)
                genStmt(node->right->right, result);
            } else {
                // BlockItems 链表
                past stmt = node->right->right;
                while (stmt != NULL) {
                    genStmt(stmt->left, result);
                    stmt = stmt->right;
                }
            }
        } else {
            genStmt(node->right, result);
        }
    }

    sprintf(temp, "  br label %%%d\n", cond_label);
    strcat(result, temp);

    sprintf(temp, "%d:\n", end_label);
    strcat(result, temp);
}


void genStmt(past node, char* result) {
    if (node == NULL) {
        return;
    }
    
    char temp[1024];
    
    switch (node->nodeType) {
        case BINARY_OPERATOR:
            if (node->ivalue == Y_ASSIGN) {
                genAssignStmt(node, result);
            } else {
                genExpr(node, result);
            }
            break;
        case CALL_EXPR:
            // 处理单独的函数调用语句（如 putint(x)）
            genExpr(node, result);
            break;
        case IF_STMT:
            genIfStmt(node, result);
            break;
        case WHILE_STMT:
            genWhileStmt(node, result);
            break;
        case DECL_STMT:
            if (node->left != NULL) {
                genStmt(node->left, result);
            }
            if (node->right != NULL) {
                genStmt(node->right, result);
            }
            break;
        case VAR_DECL:
            {
                if (node->right != NULL) {
                    past assign_node = newBinaryOper("=", Y_ASSIGN, 
                                                     newDeclRefExp(node->svalue, NULL, NULL), 
                                                     node->right);
                    genAssignStmt(assign_node, result);
                }
            }
            break;
        case RETURN_STMT:
            {
                if (node->left != NULL) {
                    ExprResult ret_res = genExpr(node->left, result);
                    if (ret_res.is_immediate) {
                        sprintf(temp, "  ret i32 %d\n", ret_res.value);
                    } else {
                        sprintf(temp, "  ret i32 %%%d\n", ret_res.value);
                    }
                } else {
                    sprintf(temp, "  ret i32 0\n");
                }
                strcat(result, temp);
            }
            break;
        case COMPOUND_STMT:
            {
                past stmt = node->right;
                while (stmt != NULL) {
                    genStmt(stmt->left, result);
                    stmt = stmt->right;
                }
            }
            break;
        default:
            break;
    }
}
