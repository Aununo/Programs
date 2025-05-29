#include <stdio.h>
#include <stdlib.h>
#include "graph.h"


// 创建一个零图（无边）
Graph createGraph(int vertex_num) {
    Graph g = (Graph)malloc(sizeof(GraphStruct));
    g->vertex_num = vertex_num;
    g->edge_num = 0;
    // 分配邻接矩阵内存
    g->edges = (int **)malloc(vertex_num * sizeof(int *));
    for (int i = 0; i < vertex_num; i++) {
        g->edges[i] = (int *)calloc(vertex_num, sizeof(int)); // 初始化为0
    }
    return g;
}

// 添加边（无向图，双向添加）
void addEdge(Graph g, Vertex v, Vertex w, int weight) {
    g->edges[v][w] = weight;
    g->edges[w][v] = weight;
    g->edge_num++;
}

// 读取图数据
Graph readGraph() {
    int vertex_num, edge_num;
    scanf("%d,%d", &vertex_num, &edge_num);
    Graph g = createGraph(vertex_num);
    for (int i = 0; i < edge_num; i++) {
        int v, w, weight;
        scanf("%d,%d,%d", &v, &w, &weight);
        addEdge(g, v, w, weight);
    }
    return g;
}

// DFS检测图中是否存在环，存在环返回1
int dfs(Graph g, int v, int visited[], int parent) {
    visited[v] = 1; // 标记当前顶点为已访问
    for (int i = 0; i < g->vertex_num; i++) {
        if (g->edges[v][i] != 0) { // 如果存在边 (v, i)
            if (!visited[i]) {     // 未访问的邻接点
                if (dfs(g, i, visited, v)) {
                    return 1;      // 递归发现环
                }
            } else if (i != parent) { // 已访问且不是父节点，说明有环
                return 1;
            }
        }
    }
    return 0; // 无环
}

// 判断图是否包含环
int hasCycle(Graph g) {
    int *visited = (int *)calloc(g->vertex_num, sizeof(int));
    for (int i = 0; i < g->vertex_num; i++) {
        if (!visited[i]) {
            if (dfs(g, i, visited, -1)) {
                free(visited);
                return 1; // 发现环
            }
        }
    }
    free(visited);
    return 0; // 无环
}

int main() {
    Graph g = readGraph();
    if (hasCycle(g)) {
        printf("yes\n"); // 图中存在环
    } else {
        printf("no\n");  // 图中无环
    }
    return 0;
}