#ifndef TRAFFIC_SORT_H
#define TRAFFIC_SORT_H

#include "data_structure.h"

// 流量排序类型
#define SORT_TOTAL 1    // 节点总流量排序
#define SORT_HTTPS 2    // HTTPS节点筛选排序（TCP6+DstPort443）
#define SORT_SEND_RATIO 3 // 单向发送占比>80%筛选排序

// 流量排序核心函数
// graph: 图指针, sort_type: 排序类型, result: 排序结果数组（需外部释放）, count: 结果节点数
// 返回：SUCCESS/FAIL
int trafficSort(Graph *graph, int sort_type, SortedNode **result, int *count);

// 打印排序结果
void printSortedResult(SortedNode *result, int count, int sort_type);

#endif