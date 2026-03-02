#include "utils.h"

// IP格式校验：xxx.xxx.xxx.xxx，每位0-255
int checkIpFormat(const char *ip) {
    if (ip == NULL || strlen(ip) > MAX_IP_LEN - 1) return FALSE;
    int dot_count = 0;
    int num = 0;
    for (int i = 0; ip[i] != '\0'; i++) {
        if (ip[i] == '.') {
            dot_count++;
            if (dot_count > 3 || num < 0 || num > 255) return FALSE;
            num = 0;
            continue;
        }
        if (!isdigit(ip[i])) return FALSE;
        num = num * 10 + (ip[i] - '0');
        if (num > 255) return FALSE;
    }
    return (dot_count == 3 && num >= 0 && num <= 255) ? TRUE : FALSE;
}

// 字符串转长整型，容错返回0
long long str2ll(const char *str) {
    if (str == NULL || strlen(str) == 0) return 0;
    long long res = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (isdigit(str[i])) res = res * 10 + (str[i] - '0');
        else break;
    }
    return res;
}

// 字符串转浮点型，容错返回0.0
double str2d(const char *str) {
    if (str == NULL || strlen(str) == 0) return 0.0;
    return atof(str);
}

// 交换两个void指针（快速排序用）
void swap(void **a, void **b) {
    void *temp = *a;
    *a = *b;
    *b = temp;
}