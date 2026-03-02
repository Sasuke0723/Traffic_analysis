#include "utils.h"
#include "data_structure.h"
#include "csv_reader.h"
#include "graph_builder.h"
#include "traffic_sort.h"
#include "path_finder.h"

void showMenu() {
    printf("\n===== Network Traffic Analysis System =====\n");
    printf("1. Read CSV Traffic Data\n");
    printf("2. View Graph Info\n");
    printf("3. Sort Nodes by Total Traffic\n");
    printf("4. Sort HTTPS Nodes\n");
    printf("5. Sort Nodes with Send Ratio >80%%\n");
    // 新增路径查找选项
    printf("6. Find Min Hop/Min Congestion Path\n");
    printf("0. Exit System\n");
    printf("===========================================\n");
    printf("Please enter function number: ");
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
                printf("Enter CSV file path (default: test_data.csv): ");
                char input[256];
                fgets(input, 256, stdin);
                input[strcspn(input, "\n\r")] = '\0';
                if (strlen(input) > 0) strcpy(csv_path, input);
                if (readCsv(csv_path, graph) == SUCCESS) {
                    printf("Data read successfully!\n");
                } else {
                    printf("Data read failed!\n");
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
                    printf("Total traffic sort failed!\n");
                }
                break;

            case 4:
                if (trafficSort(graph, SORT_HTTPS, &sorted_result, &result_count) == SUCCESS) {
                    printSortedResult(sorted_result, result_count, SORT_HTTPS);
                    free(sorted_result);
                    sorted_result = NULL;
                } else {
                    printf("HTTPS node sort failed!\n");
                }
                break;

            case 5:
                if (trafficSort(graph, SORT_SEND_RATIO, &sorted_result, &result_count) == SUCCESS) {
                    printSortedResult(sorted_result, result_count, SORT_SEND_RATIO);
                    free(sorted_result);
                    sorted_result = NULL;
                } else {
                    printf("Send ratio sort failed!\n");
                }
                break;

            case 6:
                char start_ip[MAX_IP_LEN], end_ip[MAX_IP_LEN];
                printf("Enter start IP address: ");
                scanf("%s", start_ip);
                printf("Enter end IP address: ");
                scanf("%s", end_ip);
                getchar(); // 吸收换行符

                PathResult min_hop, min_cong;
                if (findPaths(graph, start_ip, end_ip, &min_hop, &min_cong) == SUCCESS) {
                    printPathResult(start_ip, end_ip, &min_hop, &min_cong);
                    freePathResult(&min_hop);
                    freePathResult(&min_cong);
                } else {
                    printf("No path found between %s and %s!\n", start_ip, end_ip);
                }
                break;

            case 0:
                printf("Exiting system...\n");
                break;

            default:
                printf("Invalid function number, please re-enter!\n");
                break;
        }
    }

    // 释放资源
    freeGraph(graph);
    printf("System exited successfully!\n");
    return 0;
}