#include <stdio.h>
#include <stdlib.h>
#include "graph.h"


Graph createGraph(int vertex_num) {
    Graph g = (Graph)malloc(sizeof(GraphStruct));
    g->vertex_num = vertex_num;
    g->edge_num = 0;
    g->edges = (int **)malloc(vertex_num * sizeof(int *));
    for (int i = 0; i < vertex_num; i++) {
        g->edges[i] = (int *)calloc(vertex_num, sizeof(int)); 
    }
    return g;
}


void addEdge(Graph g, Vertex v, Vertex w, int weight) {
    g->edges[v][w] = weight;
    g->edges[w][v] = weight;
    g->edge_num++;
}


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


int dfs(Graph g, int v, int visited[], int parent) {
    visited[v] = 1; 
    for (int i = 0; i < g->vertex_num; i++) {
        if (g->edges[v][i] != 0) { 
            if (!visited[i]) {     
                if (dfs(g, i, visited, v)) {
                    return 1;      
                }
            } else if (i != parent) { 
                return 1;
            }
        }
    }
    return 0; 
}


int hasCycle(Graph g) {
    int *visited = (int *)calloc(g->vertex_num, sizeof(int));
    for (int i = 0; i < g->vertex_num; i++) {
        if (!visited[i]) {
            if (dfs(g, i, visited, -1)) {
                free(visited);
                return 1; 
            }
        }
    }
    free(visited);
    return 0; 
}

int main() {
    Graph g = readGraph();
    if (hasCycle(g)) {
        printf("yes\n"); 
    } else {
        printf("no\n");  
    }
    return 0;
}