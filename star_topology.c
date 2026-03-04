#include "star_topology.h"
#include <string.h>

// 创建星型拓扑节点
static StarTopology *createStarTopology(const char *center_ip) {
    if (!checkIpFormat(center_ip)) return NULL;
    StarTopology *st = (StarTopology *)malloc(sizeof(StarTopology));
    if (st == NULL) {
        perror("malloc StarTopology failed");
        return NULL;
    }
    strcpy(st->center_ip, center_ip);
    st->edge_ips = NULL;
    st->edge_count = 0;
    st->next = NULL;
    return st;
}

// 检查节点是否仅与指定中心节点相连
static int isEdgeNodeOnlyConnectCenter(Node *node, const char *center_ip) {
    if (node == NULL || !checkIpFormat(center_ip)) return FALSE;
    
    // 遍历节点所有边，检查是否仅连接中心节点
    Edge *e = node->edge_head;
    int connect_center = FALSE;
    while (e != NULL) {
        if (strcmp(e->dst_ip, center_ip) == 0) {
            connect_center = TRUE;
        } else {
            // 连接了非中心节点，不是边缘节点
            return FALSE;
        }
        e = e->next;
    }
    return connect_center;
}

// 统计节点的出边数量
static int countNodeOutEdges(Node *node) {
    if (node == NULL) return 0;
    int count = 0;
    Edge *e = node->edge_head;
    while (e != NULL) {
        count++;
        e = e->next;
    }
    return count;
}

// 检测星型拓扑核心函数
int detectStarTopology(Graph *graph, int min_edge_count, StarTopology **result) {
    if (graph == NULL || result == NULL || min_edge_count <= 0) return FAIL;
    
    *result = NULL;
    StarTopology *st_tail = NULL;
    Node *center_candidate = graph->node_head;

    // 遍历所有节点，筛选中心节点候选（边数≥min_edge_count）
    while (center_candidate != NULL) {
        int out_edge_count = countNodeOutEdges(center_candidate);
        if (out_edge_count < min_edge_count) {
            center_candidate = center_candidate->next;
            continue;
        }

        // 收集候选中心节点的所有目的IP（潜在边缘节点）
        char **candidate_edges = (char **)malloc(sizeof(char *) * out_edge_count);
        if (candidate_edges == NULL) {
            perror("malloc candidate_edges failed");
            continue;
        }
        int edge_idx = 0;
        Edge *e = center_candidate->edge_head;
        while (e != NULL && edge_idx < out_edge_count) {
            candidate_edges[edge_idx] = strdup(e->dst_ip);
            edge_idx++;
            e = e->next;
        }

        // 验证潜在边缘节点是否仅连接中心节点
        int valid_edge_count = 0;
        char **valid_edges = (char **)malloc(sizeof(char *) * out_edge_count);
        for (int i = 0; i < edge_idx; i++) {
            Node *edge_node = findNode(graph, candidate_edges[i]);
            if (edge_node != NULL && isEdgeNodeOnlyConnectCenter(edge_node, center_candidate->ip)) {
                valid_edges[valid_edge_count] = strdup(candidate_edges[i]);
                valid_edge_count++;
            }
            free(candidate_edges[i]);
        }
        free(candidate_edges);

        // 有效边缘节点数>0，判定为星型拓扑
        if (valid_edge_count > 0) {
            StarTopology *st = createStarTopology(center_candidate->ip);
            st->edge_ips = (char **)malloc(sizeof(char *) * valid_edge_count);
            st->edge_count = valid_edge_count;
            for (int i = 0; i < valid_edge_count; i++) {
                st->edge_ips[i] = valid_edges[i];
            }

            // 加入结果链表
            if (*result == NULL) {
                *result = st;
                st_tail = st;
            } else {
                st_tail->next = st;
                st_tail = st;
            }
        }
        free(valid_edges);
        center_candidate = center_candidate->next;
    }

    return (*result != NULL) ? SUCCESS : FAIL;
}

// 释放星型拓扑结果内存
void freeStarTopology(StarTopology *result) {
    StarTopology *st = result, *st_next;
    while (st != NULL) {
        st_next = st->next;
        if (st->edge_ips != NULL) {
            for (int i = 0; i < st->edge_count; i++) {
                free(st->edge_ips[i]);
            }
            free(st->edge_ips);
        }
        free(st);
        st = st_next;
    }
}

// 打印星型拓扑检测结果
void printStarTopology(StarTopology *result) {
    printf("\n===== Star Topology Detection Result =====\n");
    if (result == NULL) {
        printf("No star topology found!\n");
        printf("==========================================\n");
        return;
    }

    int topology_idx = 1;
    StarTopology *st = result;
    while (st != NULL) {
        printf("Topology %d:\n", topology_idx);
        printf("  Center IP: %s\n", st->center_ip);
        printf("  Edge Nodes (%d): ", st->edge_count);
        for (int i = 0; i < st->edge_count; i++) {
            printf("%s", st->edge_ips[i]);
            if (i < st->edge_count - 1) printf(", ");
        }
        printf("\n");
        st = st->next;
        topology_idx++;
    }
    printf("==========================================\n");
}