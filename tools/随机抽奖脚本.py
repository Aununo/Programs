"""
这是一个抽奖脚本。它会生成一个随机的编号布局，并从中抽取指定数量的获奖号码。
"""


import random
import time
import os


def run_numbered_lottery(total_slots, num_winners, min_rows=5, max_rows=15, min_cols=5, max_cols=15):
    if total_slots <= 0:
        print("错误：总席位数必须大于0！")
        return
    
    if num_winners > total_slots:
        print(f"提示：获奖人数 ({num_winners}) 大于总席位数 ({total_slots})。将抽取所有席位。")
        num_winners = total_slots
    
    if num_winners <= 0:
        print("错误：获奖人数必须大于0。")
        return

    grid_size = 0
    while grid_size < total_slots:
        num_rows = random.randint(min_rows, max_rows)
        num_cols = random.randint(min_cols, max_cols)
        grid_size = num_rows * num_cols

    print(f"抽奖系统启动！将为 {total_slots} 个编号生成一个 {num_rows} 行 x {num_cols} 列的随机布局...")
    print("-" * 30)
    time.sleep(2)

    slot_numbers = list(range(1, total_slots + 1))
    
    grid_slots = slot_numbers + ['--'] * (grid_size - total_slots)
    
    random.shuffle(grid_slots)
    
    grid = []
    for i in range(num_rows):
        row_items = [str(item) for item in grid_slots[i * num_cols : (i + 1) * num_cols]]
        grid.append(row_items)

    print("随机布局已生成！如下所示：\n")
    max_len = len(str(total_slots))
    for r in range(num_rows):
        print("  ".join(item.center(max_len) for item in grid[r]))
    print("\n" + "=" * 30)

    print(f"\n准备就绪！现在开始从 {total_slots} 个编号中抽取 {num_winners} 个幸运号码...")
    input("按 Enter 键开始抽奖...")

    valid_coords = []
    for r in range(num_rows):
        for c in range(num_cols):
            if grid[r][c] != '--':
                valid_coords.append((r, c))
    
    winning_coords = random.sample(valid_coords, num_winners)
    
    winners = []
    for r, c in winning_coords:
        winners.append((grid[r][c], r + 1, c + 1)) 
    
    winners.sort(key=lambda x: int(x[0]))

    os.system('cls' if os.name == 'nt' else 'clear')
    print("🎉🎉🎉 抽奖结果揭晓！ 🎉🎉🎉\n")
    print(f"本次中奖的幸运号码是：\n")
    for i, (number, r, c) in enumerate(winners):
        print(f"  第 {i+1} 位: 号码 {number} (位于第 {r} 行, 第 {c} 列)")
        time.sleep(1)

    print("\n\n恭喜所有中奖号码！抽奖结束。")

if __name__ == "__main__":
    TOTAL_SEATS = 121
    NUMBER_OF_WINNERS = 5
    
    run_numbered_lottery(TOTAL_SEATS, NUMBER_OF_WINNERS)