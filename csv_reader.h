#ifndef CSV_READER_H
#define CSV_READER_H

#include "data_structure.h"

// 从CSV文件读取流量数据并初始化图
// file_path: CSV文件路径, graph: 空图指针
// 返回：SUCCESS/FAIL
int readCsv(const char *file_path, Graph *graph);

#endif