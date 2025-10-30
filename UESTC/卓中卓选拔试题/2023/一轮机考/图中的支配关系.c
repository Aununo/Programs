#include <stdio.h>
#include <stdlib.h>
#define MAX_N 50

typedef struct GraphStruct {
    int vertex_num; 
    int edge_num;   
    int **edges; 
} GraphStruct;

typedef struct GraphStruct *Graph;
typedef unsigned int Vertex;


typedef struct {
    Vertex v;       
    Vertex w;      
    int weight;      
} Edge;


Graph createGraph(int vertex_num, int edge_num) {
    Graph g = (Graph)malloc(sizeof(GraphStruct));
    g->vertex_num = vertex_num;
    g->edge_num = edge_num;
    g->edges = (int **)malloc(vertex_num * sizeof(int *));
    for (int i = 0; i < vertex_num; i++) {
        g->edges[i] = (int *)malloc(vertex_num * sizeof(int));
        for (int j = 0; j < vertex_num; j++) {
            g->edges[i][j] = 0; 
        }
    }
    return g;
}

void addEdge(Graph g, Edge e) {
    g->edges[e.v][e.w] = e.weight;
}

int n; 
int vis[MAX_N]; 
Graph g; 
int dom[MAX_N][MAX_N]; 

void dfs(int u, int del) {
    vis[u] = 1; 
    for (int v = 0; v < n; v++) {
        if (g->edges[u][v] != 0 && v != del && !vis[v]) {
            dfs(v, del);
        }
    }
}


void getdom() {
    for (int i = 1; i < n; i++) { 
        for (int k = 0; k < n; k++) {
            vis[k] = 0; 
        }
        dfs(0, i);
        for (int j = 0; j < n; j++) {
            if (!vis[j]) {
                dom[j][i] = 1; 
            } else {
                dom[j][i] = 0;
            }
        }
    }
}


int main() {
    int vertex_num, edge_num;
    scanf("%d,%d", &vertex_num, &edge_num);
    n = vertex_num;
    g = createGraph(vertex_num, edge_num);

    for (int i = 0; i < edge_num; i++) {
        Edge e;
        scanf("%u,%u,%d", &e.v, &e.w, &e.weight);
        addEdge(g, e);
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            dom[i][j] = 0;
        }
    }

    getdom();

    int flag = 1;
    printf("(");
    for (int j = 0; j < n; j++) {
        for (int i = 1; i < n; i++) {
            if (dom[j][i]) {
                if (!flag) {
                    printf(")(");
                }
                flag = 0;
                printf("%d#%d", j, i); 
            }
        }
    }
    printf(")\n");

    for (int i = 0; i < n; i++) {
        free(g->edges[i]);
    }
    free(g->edges);
    free(g);

    return 0;
}