#include "traffic_sort.h"

// 快速排序比较函数：按总流量降序
static int cmpTotalSize(const void *a, const void *b) {
    SortedNode *sa = (SortedNode *)a;
    SortedNode *sb = (SortedNode *)b;
    return (sa->total_size < sb->total_size) ? 1 : (sa->total_size > sb->total_size) ? -1 : 0;
}

// 筛选HTTPS节点：遍历边，判断是否为TCP6+DstPort443
static int isHttpsNode(Node *node) {
    if (node == NULL) return FALSE;
    Edge *e = node->edge_head;
    while (e != NULL) {
        ProtocolStat *ps = findProtoStat(e->proto_head, 6); // TCP协议编号6
        if (ps != NULL && e->dst_port == 443) {
            return TRUE;
        }
        e = e->next;
    }
    return FALSE;
}

// 计算节点发送流量占比：send/(send+recv)*100%
static double calcSendRatio(Node *node) {
    if (node == NULL || (node->send_size + node->recv_size) == 0) return 0.0;
    return (double)node->send_size / (node->send_size + node->recv_size) * 100.0;
}

// 流量排序核心实现
int trafficSort(Graph *graph, int sort_type, SortedNode **result, int *count) {
    if (graph == NULL || result == NULL || count == NULL || graph->node_count == 0) {
        return FAIL;
    }

    // 初始化结果数组，先分配最大空间（节点总数）
    *count = 0;
    *result = (SortedNode *)malloc(sizeof(SortedNode) * graph->node_count);
    if (*result == NULL) {
        perror("malloc SortedNode failed");
        return FAIL;
    }

    Node *n = graph->node_head;
    while (n != NULL) {
        long long total = n->send_size + n->recv_size;
        if (total == 0) { // 无流量节点跳过
            n = n->next;
            continue;
        }

        // 根据排序类型筛选
        int is_valid = TRUE;
        double send_ratio = 0.0;
        if (sort_type == SORT_HTTPS) {
            is_valid = isHttpsNode(n);
        } else if (sort_type == SORT_SEND_RATIO) {
            send_ratio = calcSendRatio(n);
            is_valid = (send_ratio > 80.0);
        }

        if (is_valid) {
            strcpy((*result)[*count].ip, n->ip);
            (*result)[*count].total_size = total;
            (*result)[*count].send_ratio = send_ratio;
            (*count)++;
        }
        n = n->next;
    }

    if (*count == 0) {
        printf("No node match the sort condition!\n");
        free(*result);
        *result = NULL;
        return FAIL;
    }

    // 快速排序
    qsort(*result, *count, sizeof(SortedNode), cmpTotalSize);
    return SUCCESS;
}

// 打印排序结果，按类型格式化
void printSortedResult(SortedNode *result, int count, int sort_type) {
    if (result == NULL || count == 0) return;

    printf("\n===== Traffic Sort Result (type: %d) =====\n", sort_type);
    printf("Rank | IP Address       | Total Traffic\n");
    printf("-----|------------------|--------------\n");
    for (int i = 0; i < count; i++) {
        printf("%-4d | %-16s | %-12lld", i+1, result[i].ip, result[i].total_size);
        // 占比>80%时额外打印占比
        if (sort_type == SORT_SEND_RATIO) {
            printf(" | Send Ratio: %.2f%%", result[i].send_ratio);
        }
        printf("\n");
    }
    printf("=========================================\n");
}