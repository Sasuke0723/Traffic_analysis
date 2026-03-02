#ifndef DATA_STRUCTURE_H
#define DATA_STRUCTURE_H

#include "utils.h"

// 协议统计结构体：单协议的流量和时长
typedef struct _ProtocolStat {
    int proto_id;               // 协议编号（6=TCP，17=UDP，1=ICMP）
    long long data_size;        // 该协议总流量
    double duration;            // 该协议总时长
    struct _ProtocolStat *next; // 协议链表节点
} ProtocolStat;

// 边结构体：有向边，源IP->目的IP的通信关系
typedef struct _Edge {
    char dst_ip[MAX_IP_LEN];    // 目的IP
    int src_port;               // 源端口
    int dst_port;               // 目的端口
    long long total_size;       // 该边总流量（所有协议）
    double total_dur;           // 该边总时长（所有协议）
    ProtocolStat *proto_head;   // 协议统计链表头
    struct _Edge *next;         // 邻接表下一条边
} Edge;

// 节点结构体：IP节点，存储发送/接收流量
typedef struct _Node {
    char ip[MAX_IP_LEN];        // 节点IP
    long long send_size;        // 发出总流量（所有边）
    long long recv_size;        // 接收总流量（所有入边）
    Edge *edge_head;            // 邻接表边链表头
    struct _Node *next;         // 节点链表下一个节点
} Node;

// 图结构体：整个网络流量有向图
typedef struct _Graph {
    Node *node_head;            // 节点链表头
    int node_count;             // 节点总数
    int edge_count;             // 边总数
} Graph;

// 排序后的节点结构体：用于流量排序结果存储
typedef struct _SortedNode {
    char ip[MAX_IP_LEN];
    long long total_size;       // 总流量（send+recv）
    double send_ratio;          // 发出流量占比（仅筛选占比>80%时用）
} SortedNode;

// 协议统计操作
ProtocolStat *createProtoStat(int proto_id, long long data_size, double duration);
void freeProtoStat(ProtocolStat *head);
ProtocolStat *findProtoStat(ProtocolStat *head, int proto_id);

// 边操作
Edge *createEdge(const char *dst_ip, int src_port, int dst_port, int proto_id, long long data_size, double duration);
void freeEdge(Edge *head);
Edge *findEdge(Node *node, const char *dst_ip);

// 节点操作
Node *createNode(const char *ip);
void freeNode(Node *head);
Node *findNode(Graph *graph, const char *ip);

// 图操作
Graph *createGraph();
void freeGraph(Graph *graph);
void printGraph(Graph *graph); // 打印图信息（调试用）

#endif