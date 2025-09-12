# 模拟

import copy

def print_sudoku(board):
    print("+" + "-------+" * 3)
    for i, row in enumerate(board):
        print(f"| {' '.join(str(num) if num != 0 else '.' for num in row[:3])} | "
              f"{' '.join(str(num) if num != 0 else '.' for num in row[3:6])} | "
              f"{' '.join(str(num) if num != 0 else '.' for num in row[6:])} |")
        if (i + 1) % 3 == 0:
            print("+" + "-------+" * 3)

def get_candidates(board, row, col):
    if board[row][col] != 0:
        return set()

    candidates = set(range(1, 10))
    
    # 检查行
    candidates -= set(board[row])
    
    # 检查列
    candidates -= {board[i][col] for i in range(9)}
    
    # 检查九宫格
    start_row, start_col = 3 * (row // 3), 3 * (col // 3)
    box_nums = {
        board[r][c]
        for r in range(start_row, start_row + 3)
        for c in range(start_col, start_col + 3)
    }
    candidates -= box_nums
    
    return candidates

def solve_with_strategies(board):
    """
    使用指定的两种策略填充数独。
    """
    # 深拷贝一份棋盘以进行修改
    solved_board = copy.deepcopy(board)
    
    while True:
        made_progress = False
        
        # --- 策略 1: 唯一候选数法 (Sole Candidate) ---
        for r in range(9):
            for c in range(9):
                if solved_board[r][c] == 0:
                    candidates = get_candidates(solved_board, r, c)
                    if len(candidates) == 1:
                        num = candidates.pop()
                        solved_board[r][c] = num
                        # print(f"策略1: 在 ({r},{c}) 填入 {num}")
                        made_progress = True

        # --- 策略 2: 唯一位置法 (Unique Candidate) ---
        # 检查所有行
        for r in range(9):
            for num in range(1, 10):
                if num in solved_board[r]:
                    continue
                
                possible_cols = []
                for c in range(9):
                    if solved_board[r][c] == 0:
                        if num in get_candidates(solved_board, r, c):
                            possible_cols.append(c)
                
                if len(possible_cols) == 1:
                    c = possible_cols[0]
                    solved_board[r][c] = num
                    # print(f"策略2 (行): 在 ({r},{c}) 填入 {num}")
                    made_progress = True

        # 检查所有列
        for c in range(9):
            for num in range(1, 10):
                col_nums = {solved_board[i][c] for i in range(9)}
                if num in col_nums:
                    continue
                
                possible_rows = []
                for r in range(9):
                    if solved_board[r][c] == 0:
                        if num in get_candidates(solved_board, r, c):
                            possible_rows.append(r)
                
                if len(possible_rows) == 1:
                    r = possible_rows[0]
                    solved_board[r][c] = num
                    # print(f"策略2 (列): 在 ({r},{c}) 填入 {num}")
                    made_progress = True

        # 检查所有九宫格
        for box_r in range(3):
            for box_c in range(3):
                start_row, start_col = 3 * box_r, 3 * box_c
                for num in range(1, 10):
                    box_nums = {
                        solved_board[r][c]
                        for r in range(start_row, start_row + 3)
                        for c in range(start_col, start_col + 3)
                    }
                    if num in box_nums:
                        continue
                    
                    possible_cells = []
                    for r_offset in range(3):
                        for c_offset in range(3):
                            r, c = start_row + r_offset, start_col + c_offset
                            if solved_board[r][c] == 0:
                                if num in get_candidates(solved_board, r, c):
                                    possible_cells.append((r, c))
                    
                    if len(possible_cells) == 1:
                        r, c = possible_cells[0]
                        solved_board[r][c] = num
                        # print(f"策略2 (宫): 在 ({r},{c}) 填入 {num}")
                        made_progress = True
        
        # 如果一整轮下来没有任何进展，则结束循环
        if not made_progress:
            break
            
    return solved_board

# --- 示例：一个未完成的数独 ---
# 0 代表空格
initial_board = [
    [5, 3, 0, 0, 7, 0, 0, 0, 0],
    [6, 0, 0, 1, 9, 5, 0, 0, 0],
    [0, 9, 8, 0, 0, 0, 0, 6, 0],
    [8, 0, 0, 0, 6, 0, 0, 0, 3],
    [4, 0, 0, 8, 0, 3, 0, 0, 1],
    [7, 0, 0, 0, 2, 0, 0, 0, 6],
    [0, 6, 0, 0, 0, 0, 2, 8, 0],
    [0, 0, 0, 4, 1, 9, 0, 0, 5],
    [0, 0, 0, 0, 8, 0, 0, 7, 9]
]

print("初始数独:")
print_sudoku(initial_board)

# 使用指定策略求解
final_board = solve_with_strategies(initial_board)

print("\n应用策略后完成的数独:")
print_sudoku(final_board)