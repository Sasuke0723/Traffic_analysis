#include "graph_builder.h"

// 向节点中添加/合并边
static int addEdge2Node(Node *src_node, const char *dst_ip, int proto, int src_port, int dst_port, long long data_size, double duration) {
    if (src_node == NULL || !checkIpFormat(dst_ip)) return FAIL;

    // 查找是否已存在该目的IP的边：存在则合并，不存在则新建
    Edge *e = findEdge(src_node, dst_ip);
    if (e != NULL) {
        // 合并边：更新总流量、总时长
        e->total_size += data_size;
        e->total_dur += duration;
        // 查找该边的协议统计：存在则累加，不存在则新建
        ProtocolStat *ps = findProtoStat(e->proto_head, proto);
        if (ps != NULL) {
            ps->data_size += data_size;
            ps->duration += duration;
        } else {
            ProtocolStat *new_ps = createProtoStat(proto, data_size, duration);
            new_ps->next = e->proto_head;
            e->proto_head = new_ps;
        }
        return SUCCESS;
    }

    // 新建边并插入邻接表头部
    Edge *new_e = createEdge(dst_ip, src_port, dst_port, proto, data_size, duration);
    if (new_e == NULL) return FAIL;
    new_e->next = src_node->edge_head;
    src_node->edge_head = new_e;
    return SUCCESS;
}

// 向图中添加节点（不存在则新建）
static int addNode2Graph(Graph *graph, const char *ip) {
    if (graph == NULL || !checkIpFormat(ip)) return FAIL;
    if (findNode(graph, ip) != NULL) return SUCCESS; // 节点已存在

    Node *new_n = createNode(ip);
    if (new_n == NULL) return FAIL;
    // 插入节点链表头部
    new_n->next = graph->node_head;
    graph->node_head = new_n;
    graph->node_count++;
    return SUCCESS;
}

// 核心：添加边并自动处理会话合并、节点流量更新
int addEdge2Graph(Graph *graph, const char *src_ip, const char *dst_ip, int proto, int src_port, int dst_port, long long data_size, double duration) {
    if (graph == NULL || !checkIpFormat(src_ip) || !checkIpFormat(dst_ip)) return FAIL;

    // 确保源、目的节点存在于图中
    if (addNode2Graph(graph, src_ip) == FAIL || addNode2Graph(graph, dst_ip) == FAIL) {
        return FAIL;
    }

    Node *src_node = findNode(graph, src_ip);
    Node *dst_node = findNode(graph, dst_ip);

    // 向源节点添加边，并更新源节点发送流量、目的节点接收流量
    if (addEdge2Node(src_node, dst_ip, proto, src_port, dst_port, data_size, duration) == FAIL) {
        return FAIL;
    }
    src_node->send_size += data_size;
    dst_node->recv_size += data_size;
    graph->edge_count++;

    return SUCCESS;
}