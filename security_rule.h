#ifndef SECURITY_RULE_H
#define SECURITY_RULE_H

#include "data_structure.h"

// 安全规则类型枚举
typedef enum {
    RULE_IP_RANGE_BLOCK,    // IP范围阻断规则
    RULE_PROTOCOL_LIMIT,    // 协议限流规则
    RULE_PORT_BLOCK         // 端口阻断规则
} RuleType;

// IP范围结构体
typedef struct _IpRange {
    char start_ip[MAX_IP_LEN];
    char end_ip[MAX_IP_LEN];
} IpRange;

// 安全规则结构体
typedef struct _SecurityRule {
    RuleType type;          // 规则类型
    char rule_name[64];     // 规则名称
    IpRange ip_range;       // IP范围（RULE_IP_RANGE_BLOCK）
    int proto_id;           // 协议编号（RULE_PROTOCOL_LIMIT）
    int port;               // 端口号（RULE_PORT_BLOCK）
    long long threshold;    // 阈值（协议限流：流量阈值）
} SecurityRule;

// 违规会话结构体
typedef struct _ViolationSession {
    char src_ip[MAX_IP_LEN];
    char dst_ip[MAX_IP_LEN];
    int proto_id;
    int src_port;
    int dst_port;
    long long data_size;
    struct _ViolationSession *next;
} ViolationSession;

// 添加安全规则（支持动态扩展）
void addSecurityRule(SecurityRule *rules, int *rule_count, SecurityRule new_rule);

// 检测违规会话
// graph: 图指针, rules: 规则数组, rule_count: 规则数量
// violations: 输出违规会话链表（需外部释放）
// 返回：SUCCESS/FAIL
int detectViolationSessions(Graph *graph, SecurityRule *rules, int rule_count, ViolationSession **violations);

// 释放违规会话内存
void freeViolationSessions(ViolationSession *violations);

// 打印违规会话结果
void printViolationSessions(ViolationSession *violations);

// IP地址转整数（用于范围比较）
unsigned int ipToUint(const char *ip);

#endif