// 图的结构体，包含顶点数、边数和邻接矩阵
typedef struct GraphStruct {
    int vertex_num;  // 顶点数
    int edge_num;    // 边数
    int **edges;     // 邻接矩阵
} GraphStruct;

typedef struct GraphStruct* Graph;
typedef unsigned int Vertex;

// 边的结构体，包含两个端点和权值
typedef struct {
    Vertex v;        // 起点
    Vertex w;        // 终点
    int weight;      // 权值
} Edge;