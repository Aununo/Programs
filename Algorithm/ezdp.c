/*
坐标系与移动规则:

我们定义一个 n 行 m 列的棋盘，左上角为 (0, 0)。

起点: 左下角 (n-1, 0)。

终点: 右上角 (0, m-1)。

移动: 只能 向上 ((r, c) -> (r-1, c)) 或 向右 ((r, c) -> (r, c+1))。

动态规划 (DP) 思想:

状态定义: 我们创建一个 dp 表，dp[i][j] 表示从起点 (n-1, 0) 走到格子 (i, j) 的总方案数。

状态转移: 要到达 (i, j)，只能从其下方格子 (i+1, j) 或左方格子 (i, j-1) 走来。因此，dp[i][j] = dp[i+1][j] + dp[i][j-1]。

障碍物: 如果格子 (i, j) 是障碍物，则 dp[i][j] = 0。

边界条件 (起点): dp[n-1][0] = 1，前提是起点不是障碍物。

遍历顺序: 为了保证计算 dp[i][j] 时 dp[i+1][j] 和 dp[i][j-1] 已经被计算过，我们需要从下到上、从左到右遍历棋盘。

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


long long count_paths(int n, int m, int obstacles[][2], int num_obstacles) {
    long long dp[n][m];
    memset(dp, 0, sizeof(dp));

    //创建一个网格来标记障碍物，用1表示
    int grid[n][m];
    memset(grid, 0, sizeof(grid));

    for (int i = 0; i < num_obstacles; i++) {
        int r = obstacles[i][0];
        int c = obstacles[i][1];

        if (r >= 0 && r < n && c >= 0 && c < m) {
            grid[r][c] = 1;
        }
    }

    int start_r = n - 1;
    int start_c = 0;

    if (grid[start_r][start_c] == 1) {
        return 0; 
    }
    

    dp[start_r][start_c] = 1;

    // 遍历棋盘填充 dp 表
    // 循环顺序：从下到上 (r--), 从左到右 (c++)
    for (int r = n - 1; r >= 0; r--) {
        for (int c = 0; c < m; c++) {
            // 如果当前格子是障碍物，则到达此处的路径数为 0，跳过
            if (grid[r][c] == 1) {
                continue;
            }

            // 从下方格子来的路径
            if (r + 1 < n) {
                dp[r][c] += dp[r + 1][c];
            }
            
            // 从左方格子来的路径
            if (c - 1 >= 0) {
                dp[r][c] += dp[r][c - 1];
            }
        }
    }

    return dp[0][m - 1];
}


int main() {
    // --- 示例 ---
    // 一个 3x4 的棋盘
    int n_rows = 3;
    int m_cols = 4;

    // 障碍物在 (1, 1) 和 (0, 2)
    // 坐标系:
    // (0,0) (0,1) (0,2)X (0,3) <- 终点
    // (1,0) (1,1)X (1,2) (1,3)
    // (2,0) (2,1) (2,2) (2,3)
    //   ^
    //  起点
    int obstacles[][2] = {{1, 1}, {0, 2}};
    int num_obs = sizeof(obstacles) / sizeof(obstacles[0]);

    long long total_ways = count_paths(n_rows, m_cols, obstacles, num_obs);
    // 打印结果时，long long 使用 %lld 格式说明符
    printf("在一个 %dx%d 的棋盘上，从左下角到右上角的方案有: %lld 种\n", n_rows, m_cols, total_ways);

    // --- 另一个例子: 无障碍 ---
    int n2 = 3, m2 = 3;
    long long total_ways_no_obs = count_paths(n2, m2, NULL, 0);
    printf("在一个 %dx%d 的无障碍棋盘上，方案有: %lld 种\n", n2, m2, total_ways_no_obs);

    return 0;
}
