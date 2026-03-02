#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

// 全局常量定义
#define MAX_IP_LEN 16        // IP地址最大长度（点分十进制）
#define MAX_PROTO_NUM 256    // 最大协议编号
#define MAX_LINE_LEN 1024    // CSV行最大长度
#define SUCCESS 0
#define FAIL -1
#define TRUE 1
#define FALSE 0

// IP格式校验：返回TRUE/FALSE
int checkIpFormat(const char *ip);
// 字符串转长整型：处理流量大小
long long str2ll(const char *str);
// 字符串转浮点型：处理会话时长
double str2d(const char *str);
// 快速排序辅助：交换两个节点指针
void swap(void **a, void **b);

#endif