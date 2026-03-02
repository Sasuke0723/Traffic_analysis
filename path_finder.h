#ifndef PATH_FINDER_H
#define PATH_FINDER_H

#include "data_structure.h"

// 路径节点结构体：存储路径中的单个IP
typedef struct _PathNode {
    char ip[MAX_IP_LEN];
    struct _PathNode *next;
} PathNode;

// 路径结果结构体：存储单条路径的完整信息
typedef struct _PathResult {
    PathNode *path_head;  // 路径节点链表头
    int hop_count;        // 跳数
    double congestion;    // 路径总拥塞程度（流量/时长）
} PathResult;

// 查找指定起点/终点的两条路径：跳数最小 + 拥塞程度最小
// graph: 图指针, start_ip/end_ip: 起点/终点IP
// min_hop: 输出跳数最小路径, min_cong: 输出拥塞最小路径
// 返回：SUCCESS/FAIL（无路径时返回FAIL）
int findPaths(Graph *graph, const char *start_ip, const char *end_ip, PathResult *min_hop, PathResult *min_cong);

// 释放路径结果内存
void freePathResult(PathResult *result);

// 打印路径结果
void printPathResult(const char *start_ip, const char *end_ip, PathResult *min_hop, PathResult *min_cong);

#endif