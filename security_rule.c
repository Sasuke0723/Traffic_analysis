#include "security_rule.h"
// 注释/删除Linux专属头文件
// #include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

// 【新增】Windows下手动实现IP转无符号整数（替代inet_aton+ntohl）
unsigned int ipToUint(const char *ip) {
    if (ip == NULL || strlen(ip) == 0) return 0;
    
    unsigned int a, b, c, d;
    // 解析IP地址的四个段（如192.168.0.1 → a=192, b=168, c=0, d=1）
    if (sscanf(ip, "%u.%u.%u.%u", &a, &b, &c, &d) != 4) {
        return 0; // 非法IP格式
    }
    
    // 校验每个段的范围（0-255）
    if (a > 255 || b > 255 || c > 255 || d > 255) {
        return 0;
    }
    
    // 转换为无符号整数（大端序）
    return (a << 24) | (b << 16) | (c << 8) | d;
}

// 检查IP是否在指定范围内
static int isIpInRange(const char *ip, IpRange *range) {
    if (!checkIpFormat(ip) || !checkIpFormat(range->start_ip) || !checkIpFormat(range->end_ip)) {
        return FALSE;
    }
    unsigned int ip_uint = ipToUint(ip);
    unsigned int start_uint = ipToUint(range->start_ip);
    unsigned int end_uint = ipToUint(range->end_ip);
    return (ip_uint >= start_uint && ip_uint <= end_uint) ? TRUE : FALSE;
}

// 以下代码保持不变（无需修改）
// 添加安全规则
void addSecurityRule(SecurityRule *rules, int *rule_count, SecurityRule new_rule) {
    if (rules == NULL || rule_count == NULL) return;
    rules[*rule_count] = new_rule;
    (*rule_count)++;
}

// 创建违规会话节点
static ViolationSession *createViolationSession(const char *src_ip, const char *dst_ip, int proto_id,
                                                int src_port, int dst_port, long long data_size) {
    ViolationSession *vs = (ViolationSession *)malloc(sizeof(ViolationSession));
    if (vs == NULL) {
        perror("malloc ViolationSession failed");
        return NULL;
    }
    strcpy(vs->src_ip, src_ip);
    strcpy(vs->dst_ip, dst_ip);
    vs->proto_id = proto_id;
    vs->src_port = src_port;
    vs->dst_port = dst_port;
    vs->data_size = data_size;
    vs->next = NULL;
    return vs;
}

// 检测违规会话核心函数
int detectViolationSessions(Graph *graph, SecurityRule *rules, int rule_count, ViolationSession **violations) {
    if (graph == NULL || rules == NULL || rule_count <= 0 || violations == NULL) return FAIL;
    
    *violations = NULL;
    ViolationSession *vs_tail = NULL;
    Node *node = graph->node_head;

    // 遍历所有节点和边，匹配规则
    while (node != NULL) {
        Edge *e = node->edge_head;
        while (e != NULL) {
            ProtocolStat *ps = e->proto_head;
            while (ps != NULL) {
                // 遍历所有规则
                for (int i = 0; i < rule_count; i++) {
                    int is_violation = FALSE;
                    switch (rules[i].type) {
                        case RULE_IP_RANGE_BLOCK:
                            // 源IP或目的IP在阻断范围则违规
                            if (isIpInRange(node->ip, &rules[i].ip_range) || 
                                isIpInRange(e->dst_ip, &rules[i].ip_range)) {
                                is_violation = TRUE;
                            }
                            break;
                        case RULE_PROTOCOL_LIMIT:
                            // 指定协议流量超过阈值则违规
                            if (ps->proto_id == rules[i].proto_id && ps->data_size > rules[i].threshold) {
                                is_violation = TRUE;
                            }
                            break;
                        case RULE_PORT_BLOCK:
                            // 目的端口匹配阻断端口则违规
                            if (e->dst_port == rules[i].port) {
                                is_violation = TRUE;
                            }
                            break;
                        default:
                            break;
                    }

                    // 记录违规会话
                    if (is_violation) {
                        ViolationSession *vs = createViolationSession(
                            node->ip, e->dst_ip, ps->proto_id,
                            e->src_port, e->dst_port, ps->data_size
                        );
                        if (vs != NULL) {
                            if (*violations == NULL) {
                                *violations = vs;
                                vs_tail = vs;
                            } else {
                                vs_tail->next = vs;
                                vs_tail = vs;
                            }
                        }
                    }
                }
                ps = ps->next;
            }
            e = e->next;
        }
        node = node->next;
    }

    return (*violations != NULL) ? SUCCESS : FAIL;
}

// 释放违规会话内存
void freeViolationSessions(ViolationSession *violations) {
    ViolationSession *vs = violations, *vs_next;
    while (vs != NULL) {
        vs_next = vs->next;
        free(vs);
        vs = vs_next;
    }
}

// 打印违规会话结果
void printViolationSessions(ViolationSession *violations) {
    printf("\n===== Security Rule Violation Result =====\n");
    if (violations == NULL) {
        printf("No violation sessions found!\n");
        printf("===========================================\n");
        return;
    }

    int vs_idx = 1;
    ViolationSession *vs = violations;
    while (vs != NULL) {
        printf("Violation %d:\n", vs_idx);
        printf("  Source IP: %s\n", vs->src_ip);
        printf("  Destination IP: %s\n", vs->dst_ip);
        printf("  Protocol: %d\n", vs->proto_id);
        printf("  Source Port: %d\n", vs->src_port);
        printf("  Destination Port: %d\n", vs->dst_port);
        printf("  Data Size: %lld\n", vs->data_size);
        printf("-------------------------------------------\n");
        vs = vs->next;
        vs_idx++;
    }
    printf("===========================================\n");
}