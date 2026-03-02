#include "csv_reader.h"
#include "graph_builder.h"

// 分割CSV行（按逗号分割），返回字段数
static int splitCsvLine(char *line, char **fields, int max_fields) {
    if (line == NULL || fields == NULL) return 0;
    int count = 0;
    char *token = strtok(line, ",");
    while (token != NULL && count < max_fields) {
        // 去除字段前后空格
        int start = 0, end = strlen(token) - 1;
        while (isspace(token[start])) start++;
        while (end >= start && isspace(token[end])) end--;
        token[end + 1] = '\0';
        fields[count++] = token + start;
        token = strtok(NULL, ",");
    }
    return count;
}

// 读取CSV文件核心函数
int readCsv(const char *file_path, Graph *graph) {
    if (file_path == NULL || graph == NULL) return FAIL;
    FILE *fp = fopen(file_path, "r");
    if (fp == NULL) {
        perror("fopen CSV file failed");
        return FAIL;
    }

    char line[MAX_LINE_LEN];
    char *fields[7];  // CSV有7个字段：Source,Destination,Protocol,SrcPort,DstPort,DataSize,Duration
    // 跳过首行标题
    fgets(line, MAX_LINE_LEN, fp);

    while (fgets(line, MAX_LINE_LEN, fp) != NULL) {
        // 去除换行符
        line[strcspn(line, "\n\r")] = '\0';
        if (strlen(line) == 0) continue;

        // 分割行，要求7个字段
        int field_num = splitCsvLine(line, fields, 7);
        if (field_num != 7) {
            printf("Warning: invalid line (fields: %d) -> %s\n", field_num, line);
            continue;
        }

        // 解析字段并类型转换
        char *src_ip = fields[0];
        char *dst_ip = fields[1];
        int proto = atoi(fields[2]);
        int src_port = atoi(fields[3]);
        int dst_port = atoi(fields[4]);
        long long data_size = str2ll(fields[5]);
        double duration = str2d(fields[6]);

        // 校验核心字段
        if (!checkIpFormat(src_ip) || !checkIpFormat(dst_ip) || data_size <= 0 || duration <= 0) {
            printf("Warning: invalid data -> %s\n", line);
            continue;
        }

        // 调用图构建函数，添加边（会话合并在该函数中实现）
        if (addEdge2Graph(graph, src_ip, dst_ip, proto, src_port, dst_port, data_size, duration) == FAIL) {
            printf("Warning: add edge failed -> %s -> %s\n", src_ip, dst_ip);
            continue;
        }
    }

    fclose(fp);
    printf("CSV read success! Total nodes: %d, total edges: %d\n", graph->node_count, graph->edge_count);
    return SUCCESS;
}