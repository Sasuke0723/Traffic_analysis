#include "path_finder.h"
#include <limits.h>
#include <float.h>

// 创建路径节点
static PathNode *createPathNode(const char *ip) {
    if (!checkIpFormat(ip)) return NULL;
    PathNode *pn = (PathNode *)malloc(sizeof(PathNode));
    if (pn == NULL) {
        perror("malloc PathNode failed");
        return NULL;
    }
    strcpy(pn->ip, ip);
    pn->next = NULL;
    return pn;
}

// 释放路径节点链表
static void freePathNode(PathNode *head) {
    PathNode *p = head, *q;
    while (p != NULL) {
        q = p->next;
        free(p);
        p = q;
    }
}

// 计算边的拥塞值：total_size / total_dur（流量/时长）
static double calcEdgeCongestion(Edge *edge) {
    if (edge == NULL || edge->total_dur <= 0) return DBL_MAX;
    return (double)edge->total_size / edge->total_dur;
}

// 辅助函数：根据IP查找节点在node_list中的索引
static int findNodeIndex(Node **node_list, int node_count, const char *ip) {
    for (int i = 0; i < node_count; i++) {
        if (strcmp(node_list[i]->ip, ip) == 0) {
            return i;
        }
    }
    return -1;
}

// BFS算法：查找跳数最小路径（修复内存越界+逻辑漏洞）
static int bfsFindMinHop(Graph *graph, const char *start_ip, const char *end_ip, PathResult *result) {
    // 初始化路径结果
    result->path_head = NULL;
    result->hop_count = 0;
    result->congestion = 0.0;

    // 基础校验
    if (graph == NULL || !checkIpFormat(start_ip) || !checkIpFormat(end_ip)) return FAIL;
    Node *start_node = findNode(graph, start_ip);
    Node *end_node = findNode(graph, end_ip);
    if (start_node == NULL || end_node == NULL) return FAIL;

    // 边界场景：起点=终点
    if (strcmp(start_ip, end_ip) == 0) {
        PathNode *pn = createPathNode(start_ip);
        result->path_head = pn;
        result->hop_count = 0;
        result->congestion = 0.0;
        return SUCCESS;
    }

    int node_count = graph->node_count;
    if (node_count == 0) return FAIL;

    // 1. 构建节点列表（固定内存，避免越界）
    Node **node_list = (Node **)malloc(sizeof(Node *) * node_count);
    if (node_list == NULL) {
        perror("malloc node_list failed");
        return FAIL;
    }
    int idx = 0;
    Node *n = graph->node_head;
    while (n != NULL && idx < node_count) {
        node_list[idx++] = n;
        n = n->next;
    }

    // 2. 初始化访问标记（改用int数组，避免字符串字面量问题）
    int *visited = (int *)calloc(node_count, sizeof(int)); // 初始化为0
    // 初始化前驱数组（存储IP字符串，动态分配）
    char **prev = (char **)calloc(node_count, sizeof(char *));
    if (visited == NULL || prev == NULL) {
        perror("malloc visited/prev failed");
        free(node_list);
        free(visited);
        free(prev);
        return FAIL;
    }

    // 3. 初始化队列（动态扩容，避免越界）
    char **queue = (char **)malloc(sizeof(char *) * node_count * 2); // 预留双倍空间
    int front = 0, rear = 0;
    if (queue == NULL) {
        perror("malloc queue failed");
        free(node_list);
        free(visited);
        free(prev);
        return FAIL;
    }

    // 起点入队
    int start_idx = findNodeIndex(node_list, node_count, start_ip);
    if (start_idx == -1) {
        free(node_list);
        free(visited);
        free(prev);
        free(queue);
        return FAIL;
    }
    visited[start_idx] = 1;
    queue[rear++] = strdup(start_ip);

    // 4. BFS核心循环（增加队列边界校验）
    int found = 0;
    while (front < rear && !found && rear < node_count * 2) {
        char *cur_ip = queue[front++];
        int cur_idx = findNodeIndex(node_list, node_count, cur_ip);
        if (cur_idx == -1) {
            free(cur_ip);
            continue;
        }

        Node *cur_node = node_list[cur_idx];
        Edge *e = cur_node->edge_head;
        while (e != NULL && !found) {
            int dst_idx = findNodeIndex(node_list, node_count, e->dst_ip);
            if (dst_idx == -1 || visited[dst_idx] == 1) {
                e = e->next;
                continue;
            }

            // 标记访问、记录前驱
            visited[dst_idx] = 1;
            prev[dst_idx] = strdup(cur_ip);

            // 找到终点，终止循环
            if (strcmp(e->dst_ip, end_ip) == 0) {
                found = 1;
                break;
            }

            // 目的节点入队（校验队列边界）
            if (rear < node_count * 2) {
                queue[rear++] = strdup(e->dst_ip);
            }
            e = e->next;
        }
        free(cur_ip); // 释放当前节点的IP内存
    }

    // 5. 回溯构建路径（修复跳数计算）
    if (found) {
        PathNode *path_tail = NULL;
        char *cur_ip = strdup(end_ip);
        int hop_count = 0;

        while (cur_ip != NULL) {
            PathNode *pn = createPathNode(cur_ip);
            if (pn == NULL) break;
            pn->next = path_tail;
            path_tail = pn;
            hop_count++;

            // 查找前驱IP
            int cur_idx = findNodeIndex(node_list, node_count, cur_ip);
            char *prev_ip = (cur_idx != -1) ? prev[cur_idx] : NULL;
            free(cur_ip);

            // 无前置节点（回到起点），终止回溯
            if (prev_ip == NULL || strcmp(prev_ip, start_ip) == 0) {
                // 添加起点到路径头部
                PathNode *start_pn = createPathNode(start_ip);
                if (start_pn != NULL) {
                    start_pn->next = path_tail;
                    path_tail = start_pn;
                    hop_count++;
                }
                cur_ip = NULL;
            } else {
                cur_ip = strdup(prev_ip);
            }
        }

        result->path_head = path_tail;
        result->hop_count = (hop_count > 0) ? hop_count - 1 : 0; // 跳数=节点数-1
        // 计算路径总拥塞（可选）
        PathNode *pn = result->path_head;
        double total_cong = 0.0;
        while (pn != NULL && pn->next != NULL) {
            Node *from_node = findNode(graph, pn->ip);
            Edge *e = findEdge(from_node, pn->next->ip);
            if (e != NULL) {
                total_cong += calcEdgeCongestion(e);
            }
            pn = pn->next;
        }
        result->congestion = total_cong;
    }

    // 6. 释放所有临时内存（避免泄漏）
    for (int i = 0; i < node_count; i++) {
        if (prev[i] != NULL) free(prev[i]);
    }
    for (int i = 0; i < rear; i++) {
        if (queue[i] != NULL) free(queue[i]);
    }
    free(node_list);
    free(visited);
    free(prev);
    free(queue);

    return found ? SUCCESS : FAIL;
}

// 迪杰斯特拉算法：查找拥塞程度最小路径（同步修复内存问题）
static int dijkstraFindMinCong(Graph *graph, const char *start_ip, const char *end_ip, PathResult *result) {
    // 初始化路径结果
    result->path_head = NULL;
    result->hop_count = 0;
    result->congestion = 0.0;

    // 基础校验
    if (graph == NULL || !checkIpFormat(start_ip) || !checkIpFormat(end_ip)) return FAIL;
    Node *start_node = findNode(graph, start_ip);
    Node *end_node = findNode(graph, end_ip);
    if (start_node == NULL || end_node == NULL) return FAIL;

    // 边界场景：起点=终点
    if (strcmp(start_ip, end_ip) == 0) {
        PathNode *pn = createPathNode(start_ip);
        result->path_head = pn;
        result->hop_count = 0;
        result->congestion = 0.0;
        return SUCCESS;
    }

    int node_count = graph->node_count;
    if (node_count == 0) return FAIL;

    // 1. 构建节点列表
    Node **node_list = (Node **)malloc(sizeof(Node *) * node_count);
    int idx = 0;
    Node *n = graph->node_head;
    while (n != NULL && idx < node_count) {
        node_list[idx++] = n;
        n = n->next;
    }

    // 2. 初始化距离/访问/前驱数组
    double *dist = (double *)malloc(sizeof(double) * node_count);
    int *visited = (int *)calloc(node_count, sizeof(int));
    char **prev = (char **)calloc(node_count, sizeof(char *));
    if (dist == NULL || visited == NULL || prev == NULL) {
        perror("malloc dijkstra failed");
        free(node_list);
        free(dist);
        free(visited);
        free(prev);
        return FAIL;
    }

    // 初始化距离数组（起点为0，其他为无穷大）
    int start_idx = findNodeIndex(node_list, node_count, start_ip);
    if (start_idx == -1) {
        free(node_list);
        free(dist);
        free(visited);
        free(prev);
        return FAIL;
    }
    for (int i = 0; i < node_count; i++) {
        dist[i] = (i == start_idx) ? 0.0 : DBL_MAX;
    }

    // 3. 迪杰斯特拉核心循环
    for (int i = 0; i < node_count; i++) {
        // 找未访问的最小距离节点
        int min_idx = -1;
        double min_dist = DBL_MAX;
        for (int j = 0; j < node_count; j++) {
            if (visited[j] == 0 && dist[j] < min_dist) {
                min_dist = dist[j];
                min_idx = j;
            }
        }
        if (min_idx == -1) break; // 无可达节点
        visited[min_idx] = 1;

        // 找到终点，提前退出
        if (strcmp(node_list[min_idx]->ip, end_ip) == 0) break;

        // 更新邻接节点距离
        Node *cur_node = node_list[min_idx];
        Edge *e = cur_node->edge_head;
        while (e != NULL) {
            int dst_idx = findNodeIndex(node_list, node_count, e->dst_ip);
            if (dst_idx == -1 || visited[dst_idx] == 1) {
                e = e->next;
                continue;
            }

            // 计算新距离
            double edge_cong = calcEdgeCongestion(e);
            if (dist[min_idx] != DBL_MAX && (dist[min_idx] + edge_cong) < dist[dst_idx]) {
                dist[dst_idx] = dist[min_idx] + edge_cong;
                if (prev[dst_idx] != NULL) free(prev[dst_idx]);
                prev[dst_idx] = strdup(cur_node->ip);
            }
            e = e->next;
        }
    }

    // 4. 构建路径结果
    int found = (dist[findNodeIndex(node_list, node_count, end_ip)] != DBL_MAX) ? 1 : 0;
    if (found) {
        PathNode *path_tail = NULL;
        char *cur_ip = strdup(end_ip);
        int hop_count = 0;
        double total_cong = 0.0;

        while (cur_ip != NULL) {
            PathNode *pn = createPathNode(cur_ip);
            if (pn == NULL) break;
            pn->next = path_tail;
            path_tail = pn;
            hop_count++;

            // 累加拥塞值
            int cur_idx = findNodeIndex(node_list, node_count, cur_ip);
            char *prev_ip = (cur_idx != -1) ? prev[cur_idx] : NULL;
            if (prev_ip != NULL) {
                Node *prev_node = findNode(graph, prev_ip);
                Edge *e = findEdge(prev_node, cur_ip);
                if (e != NULL) total_cong += calcEdgeCongestion(e);
            }

            // 回溯前驱
            free(cur_ip);
            if (prev_ip == NULL || strcmp(prev_ip, start_ip) == 0) {
                PathNode *start_pn = createPathNode(start_ip);
                if (start_pn != NULL) {
                    start_pn->next = path_tail;
                    path_tail = start_pn;
                    hop_count++;
                }
                cur_ip = NULL;
            } else {
                cur_ip = strdup(prev_ip);
            }
        }

        result->path_head = path_tail;
        result->hop_count = (hop_count > 0) ? hop_count - 1 : 0;
        result->congestion = total_cong;
    }

    // 5. 释放临时内存
    for (int i = 0; i < node_count; i++) {
        if (prev[i] != NULL) free(prev[i]);
    }
    free(node_list);
    free(dist);
    free(visited);
    free(prev);

    return found ? SUCCESS : FAIL;
}

// 核心：查找两条路径
int findPaths(Graph *graph, const char *start_ip, const char *end_ip, PathResult *min_hop, PathResult *min_cong) {
    if (graph == NULL || min_hop == NULL || min_cong == NULL) return FAIL;

    // 初始化路径结果
    min_hop->path_head = NULL;
    min_hop->hop_count = 0;
    min_hop->congestion = 0.0;
    min_cong->path_head = NULL;
    min_cong->hop_count = 0;
    min_cong->congestion = 0.0;
    // 查找跳数最小路径
    int hop_ok = bfsFindMinHop(graph, start_ip, end_ip, min_hop);
    // 查找拥塞最小路径
    int cong_ok = dijkstraFindMinCong(graph, start_ip, end_ip, min_cong);

    return (hop_ok == SUCCESS || cong_ok == SUCCESS) ? SUCCESS : FAIL;
}

// 释放路径结果内存
void freePathResult(PathResult *result) {
    if (result == NULL) return;
    freePathNode(result->path_head);
    result->path_head = NULL;
    result->hop_count = 0;
    result->congestion = 0.0;
}

// 打印路径结果
void printPathResult(const char *start_ip, const char *end_ip, PathResult *min_hop, PathResult *min_cong) {
    printf("\n===== Path Find Result (%s -> %s) =====\n", start_ip, end_ip);

    // 打印跳数最小路径
    printf("1. Min Hop Path:\n");
    if (min_hop->path_head == NULL) {
        printf("   No path found!\n");
    } else {
        printf("   Hop Count: %d\n   Path: ", min_hop->hop_count);
        PathNode *pn = min_hop->path_head;
        while (pn != NULL) {
            printf("%s", pn->ip);
            if (pn->next != NULL) printf(" -> ");
            pn = pn->next;
        }
        printf("\n   Total Congestion: %.2f\n", min_hop->congestion);
    }

    // 打印拥塞最小路径
    printf("2. Min Congestion Path:\n");
    if (min_cong->path_head == NULL) {
        printf("   No path found!\n");
    } else {
        printf("   Hop Count: %d\n   Path: ", min_cong->hop_count);
        PathNode *pn = min_cong->path_head;
        while (pn != NULL) {
            printf("%s", pn->ip);
            if (pn->next != NULL) printf(" -> ");
            pn = pn->next;
        }
        printf("\n   Total Congestion: %.2f\n", min_cong->congestion);
    }
    printf("=========================================\n");
}