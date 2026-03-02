#include "data_structure.h"

// 创建协议统计节点
ProtocolStat *createProtoStat(int proto_id, long long data_size, double duration) {
    ProtocolStat *ps = (ProtocolStat *)malloc(sizeof(ProtocolStat));
    if (ps == NULL) {
        perror("malloc ProtoStat failed");
        return NULL;
    }
    ps->proto_id = proto_id;
    ps->data_size = data_size;
    ps->duration = duration;
    ps->next = NULL;
    return ps;
}

// 释放协议统计链表
void freeProtoStat(ProtocolStat *head) {
    ProtocolStat *p = head, *q;
    while (p != NULL) {
        q = p->next;
        free(p);
        p = q;
    }
}

// 在协议链表中查找指定协议编号的节点
ProtocolStat *findProtoStat(ProtocolStat *head, int proto_id) {
    ProtocolStat *p = head;
    while (p != NULL) {
        if (p->proto_id == proto_id) return p;
        p = p->next;
    }
    return NULL;
}

// 创建边节点：初始化并创建首个协议统计
Edge *createEdge(const char *dst_ip, int src_port, int dst_port, int proto_id, long long data_size, double duration) {
    if (!checkIpFormat(dst_ip)) return NULL;
    Edge *e = (Edge *)malloc(sizeof(Edge));
    if (e == NULL) {
        perror("malloc Edge failed");
        return NULL;
    }
    strcpy(e->dst_ip, dst_ip);
    e->src_port = src_port;
    e->dst_port = dst_port;
    e->total_size = data_size;
    e->total_dur = duration;
    e->proto_head = createProtoStat(proto_id, data_size, duration);
    e->next = NULL;
    return e;
}

// 释放边链表（含协议统计）
void freeEdge(Edge *head) {
    Edge *p = head, *q;
    while (p != NULL) {
        q = p->next;
        freeProtoStat(p->proto_head);
        free(p);
        p = q;
    }
}

// 在节点的邻接表中查找指定目的IP的边
Edge *findEdge(Node *node, const char *dst_ip) {
    if (node == NULL || !checkIpFormat(dst_ip)) return NULL;
    Edge *e = node->edge_head;
    while (e != NULL) {
        if (strcmp(e->dst_ip, dst_ip) == 0) return e;
        e = e->next;
    }
    return NULL;
}

// 创建IP节点
Node *createNode(const char *ip) {
    if (!checkIpFormat(ip)) return NULL;
    Node *n = (Node *)malloc(sizeof(Node));
    if (n == NULL) {
        perror("malloc Node failed");
        return NULL;
    }
    strcpy(n->ip, ip);
    n->send_size = 0;
    n->recv_size = 0;
    n->edge_head = NULL;
    n->next = NULL;
    return n;
}

// 释放节点链表（含边、协议统计）
void freeNode(Node *head) {
    Node *p = head, *q;
    while (p != NULL) {
        q = p->next;
        freeEdge(p->edge_head);
        free(p);
        p = q;
    }
}

// 在图中查找指定IP的节点
Node *findNode(Graph *graph, const char *ip) {
    if (graph == NULL || !checkIpFormat(ip)) return NULL;
    Node *n = graph->node_head;
    while (n != NULL) {
        if (strcmp(n->ip, ip) == 0) return n;
        n = n->next;
    }
    return NULL;
}

// 创建空图
Graph *createGraph() {
    Graph *g = (Graph *)malloc(sizeof(Graph));
    if (g == NULL) {
        perror("malloc Graph failed");
        return NULL;
    }
    g->node_head = NULL;
    g->node_count = 0;
    g->edge_count = 0;
    return g;
}

// 释放整个图
void freeGraph(Graph *graph) {
    if (graph == NULL) return;
    freeNode(graph->node_head);
    free(graph);
}

// 打印图信息（调试用：节点数、边数、各节点流量）
void printGraph(Graph *graph) {
    if (graph == NULL) {
        printf("Graph is NULL!\n");
        return;
    }
    printf("===== Graph Info =====\n");
    printf("Node count: %d\nEdge count: %d\n", graph->node_count, graph->edge_count);
    Node *n = graph->node_head;
    while (n != NULL) {
        printf("IP: %-15s | Send: %-10lld | Recv: %-10lld | Total: %lld\n",
               n->ip, n->send_size, n->recv_size, n->send_size + n->recv_size);
        n = n->next;
    }
    printf("======================\n");
}