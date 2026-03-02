#ifndef GRAPH_BUILDER_H
#define GRAPH_BUILDER_H

#include "data_structure.h"

// 向图中添加边（核心：自动会话合并、更新节点流量）
// 参数：图、源IP、目的IP、协议、源端口、目的端口、流量、时长
// 返回：SUCCESS/FAIL
int addEdge2Graph(Graph *graph, const char *src_ip, const char *dst_ip, int proto, int src_port, int dst_port, long long data_size, double duration);

#endif