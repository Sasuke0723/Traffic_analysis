#include "traffic_sort.h"

// 基础菜单交互
void showMenu() {
    printf("\n===== 网络流量分析与异常检测系统 =====\n");
    printf("1. 读取CSV流量数据\n");
    printf("2. 查看图结构信息\n");
    printf("3. 节点总流量排序\n");
    printf("4. HTTPS节点流量排序\n");
    printf("5. 单向发送占比>80%节点排序\n");
    printf("0. 退出系统\n");
    printf("=======================================\n");
    printf("请输入功能编号：");
}

int main() {
    Graph *graph = createGraph();
    if (graph == NULL) {
        printf("Create graph failed!\n");
        return -1;
    }

    int choice = -1;
    char csv_path[256] = "test_data.csv"; // 默认测试用例路径
    SortedNode *sorted_result = NULL;
    int result_count = 0;

    while (choice != 0) {
        showMenu();
        scanf("%d", &choice);
        getchar(); // 吸收换行符

        switch (choice) {
            case 1:
                printf("请输入CSV文件路径（默认：test_data.csv）：");
                char input[256];
                fgets(input, 256, stdin);
                input[strcspn(input, "\n\r")] = '\0';
                if (strlen(input) > 0) strcpy(csv_path, input);
                if (readCsv(csv_path, graph) == SUCCESS) {
                    printf("数据读取成功！\n");
                } else {
                    printf("数据读取失败！\n");
                }
                break;

            case 2:
                printGraph(graph);
                break;

            case 3:
                if (trafficSort(graph, SORT_TOTAL, &sorted_result, &result_count) == SUCCESS) {
                    printSortedResult(sorted_result, result_count, SORT_TOTAL);
                    free(sorted_result);
                    sorted_result = NULL;
                } else {
                    printf("总流量排序失败！\n");
                }
                break;

            case 4:
                if (trafficSort(graph, SORT_HTTPS, &sorted_result, &result_count) == SUCCESS) {
                    printSortedResult(sorted_result, result_count, SORT_HTTPS);
                    free(sorted_result);
                    sorted_result = NULL;
                } else {
                    printf("HTTPS节点排序失败！\n");
                }
                break;

            case 5:
                if (trafficSort(graph, SORT_SEND_RATIO, &sorted_result, &result_count) == SUCCESS) {
                    printSortedResult(sorted_result, result_count, SORT_SEND_RATIO);
                    free(sorted_result);
                    sorted_result = NULL;
                } else {
                    printf("单向发送占比排序失败！\n");
                }
                break;

            case 0:
                printf("正在退出系统...\n");
                break;

            default:
                printf("无效的功能编号，请重新输入！\n");
                break;
        }
    }

    // 释放资源
    freeGraph(graph);
    printf("系统退出成功！\n");
    return 0;
}