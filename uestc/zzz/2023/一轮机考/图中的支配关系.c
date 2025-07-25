#include <stdio.h>
#include <stdlib.h>
#define MAX_N 50

// 图的结构体，包含顶点数、边数和邻接矩阵
typedef struct GraphStruct {
    int vertex_num;  // 顶点数
    int edge_num;    // 边数
    int **edges;     // 邻接矩阵
} GraphStruct;

typedef struct GraphStruct *Graph;
typedef unsigned int Vertex;

// 边的结构体，包含起点、终点和权值
typedef struct {
    Vertex v;        // 起点
    Vertex w;        // 终点
    int weight;      // 权值
} Edge;

// 创建带权有向图
Graph createGraph(int vertex_num, int edge_num) {
    Graph g = (Graph)malloc(sizeof(GraphStruct));
    g->vertex_num = vertex_num;
    g->edge_num = edge_num;
    // 分配邻接矩阵内存
    g->edges = (int **)malloc(vertex_num * sizeof(int *));
    for (int i = 0; i < vertex_num; i++) {
        g->edges[i] = (int *)malloc(vertex_num * sizeof(int));
        for (int j = 0; j < vertex_num; j++) {
            g->edges[i][j] = 0; // 初始化为0
        }
    }
    return g;
}

// 添加有向边
void addEdge(Graph g, Edge e) {
    g->edges[e.v][e.w] = e.weight;
}

// DFS遍历，检测删除节点del后从u开始的可达性
int n; // 全局变量：节点总数，编号为0到n-1
int vis[MAX_N]; // 访问标记数组
Graph g; // 全局图
int dom[MAX_N][MAX_N]; // 支配关系矩阵

void dfs(int u, int del) {
    vis[u] = 1; // 标记当前节点为已访问
    for (int v = 0; v < n; v++) {
        if (g->edges[u][v] != 0 && v != del && !vis[v]) {
            dfs(v, del); // 递归访问未访问的邻接节点
        }
    }
}

// 计算支配关系
void getdom() {
    for (int i = 1; i < n; i++) { // 遍历每个待删除的节点
        for (int k = 0; k < n; k++) {
            vis[k] = 0; // 重置访问标记
        }
        dfs(0, i); // 从节点0开始DFS，删除节点i
        for (int j = 0; j < n; j++) {
            if (!vis[j]) {
                dom[j][i] = 1; // 节点j不可达，说明i支配j
            } else {
                dom[j][i] = 0;
            }
        }
    }
}

// 主函数：输入图数据，计算支配关系并输出
int main() {
    int vertex_num, edge_num;
    scanf("%d,%d", &vertex_num, &edge_num);
    n = vertex_num;
    g = createGraph(vertex_num, edge_num);

    // 读取边数据
    for (int i = 0; i < edge_num; i++) {
        Edge e;
        scanf("%u,%u,%d", &e.v, &e.w, &e.weight);
        addEdge(g, e);
    }

    // 初始化支配关系矩阵
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            dom[i][j] = 0;
        }
    }

    // 计算支配关系
    getdom();

    // 输出支配关系
    int flag = 1;
    printf("(");
    for (int j = 0; j < n; j++) {
        for (int i = 1; i < n; i++) {
            if (dom[j][i]) {
                if (!flag) {
                    printf(")(");
                }
                flag = 0;
                printf("%d#%d", j, i); // 输出支配对 j#i
            }
        }
    }
    printf(")\n");

    // 释放内存
    for (int i = 0; i < n; i++) {
        free(g->edges[i]);
    }
    free(g->edges);
    free(g);

    return 0;
}