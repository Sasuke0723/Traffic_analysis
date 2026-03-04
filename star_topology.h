#ifndef STAR_TOPOLOGY_H
#define STAR_TOPOLOGY_H

#include "data_structure.h"

// 星型拓扑结果结构体
typedef struct _StarTopology {
    char center_ip[MAX_IP_LEN];       // 中心节点IP
    char **edge_ips;                  // 边缘节点IP数组
    int edge_count;                   // 边缘节点数量
    struct _StarTopology *next;       // 下一个星型拓扑
} StarTopology;

// 检测星型拓扑（中心节点：边数≥20；边缘节点：仅与中心节点相连）
// graph: 图指针, min_edge_count: 中心节点最小边数（默认20）
// result: 输出星型拓扑链表（需外部释放）
// 返回：SUCCESS/FAIL
int detectStarTopology(Graph *graph, int min_edge_count, StarTopology **result);

// 释放星型拓扑结果内存
void freeStarTopology(StarTopology *result);

// 打印星型拓扑检测结果
void printStarTopology(StarTopology *result);

#endif