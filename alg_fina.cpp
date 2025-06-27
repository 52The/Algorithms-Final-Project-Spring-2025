#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define MAX_ITEMS 40000 // 最大物品数量
#define MAX_CAPACITY 50000 // 最大背包容量
#define MAX_BRUTE_FORCE 100// 蛮力法极限
#define MAX_N_TEST 20 // 回溯法的物品数量限制

// 物品结构体
typedef struct {
    int index; // 物品编号
    double weight; // 物品重量
    double value; // 物品价值
} Item;

// 函数声明
void generate_items(int n, Item items[], unsigned int seed); // 生成随机物品
double brute_force(int n, Item items[], int capacity, int selected[], int *total_weight); // 蛮力法
double dynamic_programming(int n, Item items[], int capacity, int selected[], int *total_weight); // 动态规划法
double greedy(int n, Item items[], int capacity, int selected[], int *total_weight); // 贪心法
double backtracking(int n, Item items[], int capacity, int selected[], int *total_weight); // 回溯法
void print_results(int n, Item items[], int selected[], int total_weight, double total_value, double time_ms, const char *method); // 打印结果
void save_to_csv(int n, Item items[], int selected[], int total_weight, double total_value, const char *filename, int capacity); // 保存结果到 CSV

// 用于贪心法的 qsort 比较函数
int compare(const void *a, const void *b) {
    Item *item1 = (Item *)a;
    Item *item2 = (Item *)b;
    double ratio1 = item1->value / item1->weight; // 物品1的单位价值
    double ratio2 = item2->value / item2->weight; // 物品2的单位价值
    return ratio2 > ratio1 ? 1 : -1; // 按单位价值降序排序
}

// 生成随机物品
void generate_items(int n, Item items[], unsigned int seed) {
    srand(seed); // 设置随机种子
    for (int i = 0; i < n; i++) {
        items[i].index = i; // 设置物品编号
        items[i].weight = (rand() % 100) + 1; // 重量：1到100
        items[i].value = 100.00 + (rand() % 90001) / 100.0; // 价值：100.00到1000.00
    }
}

// 蛮力法
double brute_force(int n, Item items[], int capacity, int selected[], int *total_weight) {
    if (n > MAX_BRUTE_FORCE) return -1.0; // 物品数量超过限制，跳过
    double max_value = 0.0; // 最大价值
    *total_weight = 0; // 总重量初始化
    unsigned int max_subsets = 1u << n; // 所有子集数量

    for (unsigned int i = 0; i < max_subsets; i++) {
        double curr_weight = 0.0; // 当前重量
        double curr_value = 0.0; // 当前价值
        int temp_selected[MAX_N_TEST] = {0}; // 临时选择数组

        for (int j = 0; j < n; j++) {
            if (i & (1u << j)) { // 检查是否选择第j个物品
                curr_weight += items[j].weight;
                curr_value += items[j].value;
                temp_selected[j] = 1;
            }
        }

        if (curr_weight <= capacity && curr_value > max_value) { // 更新最优解
            max_value = curr_value;
            *total_weight = (int)curr_weight;
            for (int j = 0; j < n; j++) {
                selected[j] = temp_selected[j];
            }
        }
    }
    return max_value;
}

// 动态规划法（优化内存，去除 keep 数组）
double dynamic_programming(int n, Item items[], int capacity, int selected[], int *total_weight) {
    int *dp = (int *)calloc(capacity + 1, sizeof(int)); // 用整数存储
    int **keep = (int **)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        keep[i] = (int *)calloc(capacity + 1, sizeof(int));
    }
    if (!dp || !keep) {
        printf("动态规划内存分配失败\n");
        exit(1);
    }

    // 填充动态规划表
    for (int i = 0; i < n; i++) {
        int wgt = (int)items[i].weight;
        int val = (int)round(items[i].value * 100); // 价值转为整数
        for (int w = capacity; w >= wgt; w--) {
            if (dp[w] < dp[w - wgt] + val) {
                dp[w] = dp[w - wgt] + val;
                keep[i][w] = 1;
            }
        }
    }

    // 回溯选择物品
    int w = capacity;
    *total_weight = 0;
    memset(selected, 0, n * sizeof(int));
    for (int i = n - 1; i >= 0; i--) {
        if (keep[i][w]) {
            selected[i] = 1;
            w -= (int)items[i].weight;
            *total_weight += (int)items[i].weight;
        }
    }

    double max_value = dp[capacity] / 100.0;

    for (int i = 0; i < n; i++) free(keep[i]);
    free(keep);
    free(dp);
    return max_value;
}

// 贪心法（按价值/重量比）
double greedy(int n, Item items[], int capacity, int selected[], int *total_weight) {
    Item *sorted_items = (Item *)malloc(n * sizeof(Item)); // 排序后的物品数组
    if (!sorted_items) {
        printf("排序物品内存分配失败\n");
        exit(1);
    }
    memcpy(sorted_items, items, n * sizeof(Item));
    qsort(sorted_items, n, sizeof(Item), compare); // 按单位价值排序

    double total_value = 0.0; // 总价值
    *total_weight = 0; // 总重量
    for (int i = 0; i < n; i++) {
        if (*total_weight + (int)sorted_items[i].weight <= capacity) {
            selected[sorted_items[i].index] = 1;
            *total_weight += (int)sorted_items[i].weight;
            total_value += sorted_items[i].value;
        }
    }

    free(sorted_items); // 释放内存
    return total_value;
}

// 回溯法
int cmp_value_per_weight(const void *a, const void *b) {
    Item *ia = (Item*)a, *ib = (Item*)b;
    double r1 = ia->value / ia->weight, r2 = ib->value / ib->weight;
    return r2 > r1 ? 1 : (r2 < r1 ? -1 : 0);
}
void backtrack(int i, int n, Item items[], int capacity, int curr_weight, double curr_value,
               int selected[], int *best_selected, double *max_value, int *best_weight, double remain_value) {
    if (curr_weight > capacity) return; // 超重剪枝
    if (i == n) {
        if (curr_value > *max_value) {
            *max_value = curr_value;
            *best_weight = curr_weight;
            memcpy(best_selected, selected, n * sizeof(int));
        }
        return;
    }
    // 剪枝：剩余最大可能价值都不如当前最优
    if (curr_value + remain_value <= *max_value) return;

    // 不选第i个
    selected[i] = 0;
    backtrack(i + 1, n, items, capacity, curr_weight, curr_value, selected, best_selected, max_value, best_weight, remain_value - items[i].value);

    // 选第i个
    selected[i] = 1;
    backtrack(i + 1, n, items, capacity, curr_weight + (int)items[i].weight,
              curr_value + items[i].value, selected, best_selected, max_value, best_weight, remain_value - items[i].value);
    selected[i] = 0;
}

double backtracking(int n, Item items[], int capacity, int selected[], int *total_weight) {
    if (n > MAX_N_TEST) return -1.0;
    // 可选：先按单位价值排序，提升剪枝效果
    Item *sorted = (Item*)malloc(n * sizeof(Item));
    memcpy(sorted, items, n * sizeof(Item));
    qsort(sorted, n, sizeof(Item), cmp_value_per_weight);

    int *best_selected = (int *)calloc(n, sizeof(int));
    double max_value = 0.0;
    *total_weight = 0;
    int temp_selected[MAX_N_TEST] = {0};
    double remain_value = 0.0;
    for (int i = 0; i < n; i++) remain_value += sorted[i].value;

    backtrack(0, n, sorted, capacity, 0, 0.0, temp_selected, best_selected, &max_value, total_weight, remain_value);

    // 还原到原顺序
    memset(selected, 0, n * sizeof(int));
    for (int i = 0; i < n; i++) {
        if (best_selected[i]) {
            selected[sorted[i].index] = 1;
        }
    }

    free(best_selected);
    free(sorted);
    return max_value;
}

// 打印结果
void print_results(int n, Item items[], int selected[], int total_weight, double total_value, double time_ms, const char *method) {
    if (total_value < 0) {
        printf("\n%s 已跳过 (n=%d)\n", method, n);
        return;
    }
    printf("\n%s 结果 (n=%d):\n", method, n);
    printf("选择的物品 (编号, 重量, 价值):\n");
    int count = 0;
    for (int i = 0; i < n && count < 10; i++) {
        if (selected[i]) {
            printf("物品 %d: 重量=%.0f, 价值=%.2f\n", items[i].index, items[i].weight, items[i].value);
            count++;
        }
    }
    if (count >= 10) printf("... (更多物品被选择)\n");
    printf("总重量: %d\n", total_weight);
    printf("总价值: %.2f\n", total_value);
    printf("执行时间: %.2f 毫秒\n", time_ms);
}

// 保存结果到 CSV 文件 (n=1000)
void save_to_csv(int n, Item items[], int selected[], int total_weight, double total_value, const char *filename, int capacity) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        printf("无法打开文件 %s\n", filename);
        return;
    }
    fprintf(fp, "物品编号,重量,价值,是否选择\n");
    for (int i = 0; i < n; i++) {
        fprintf(fp, "%d,%.0f,%.2f,%d\n", items[i].index, items[i].weight, items[i].value, selected[i]);
    }
    fprintf(fp, "\n总重量,%d\n", total_weight);
    fprintf(fp, "总价值,%.2f\n", total_value);
    fprintf(fp, "背包容量,%d\n", capacity);
    fclose(fp);
}

int main() {
    // 统一测试用例
    int item_counts[] = {5, 8, 10, 15, 20, 100, 500, 1000, 2000, 3000, 4000, 5000, 10000, 20000, 30000, 40000}; // 物品数量 45000 已到极限 
    int capacities[] = { 500, 1000, 10000, 50000}; // 背包容量
    int num_items = sizeof(item_counts) / sizeof(item_counts[0]);
    int num_capacities = sizeof(capacities) / sizeof(capacities[0]);

    Item *items = (Item *)malloc(MAX_ITEMS * sizeof(Item)); // 物品数组
    int *selected = (int *)calloc(MAX_ITEMS, sizeof(int)); // 选择数组
    if (!items || !selected) {
        printf("物品或选择数组内存分配失败\n");
        exit(1);
    }
    int total_weight; // 总重量
    double total_value; // 总价值
    clock_t start, end; // 时间记录
    double time_ms; // 执行时间（毫秒）

    // 执行时间总结数组
    double bf_times[16][4]; // 蛮力法
    double dp_times[16][4]; // 动态规划法
    double greedy_times[16][4]; // 贪心法
    double bt_times[16][4]; // 回溯法
    for (int i = 0; i < 16; i++) {
        for (int c = 0; c < 4; c++) {
            bf_times[i][c] = dp_times[i][c] = greedy_times[i][c] = bt_times[i][c] = -1.0;
        }
    }

    printf("运行 0-1 背包问题多种算法...\n");

    // 测试所有算法
    for (int c = 0; c < num_capacities; c++) {
        printf("\n--- 背包容量: %d ---\n", capacities[c]);
        for (int i = 0; i < num_items; i++) {
            int n = item_counts[i];
            printf("\n测试 n=%d\n", n);
            generate_items(n, items, 12345); // 生成随机物品，固定种子

            // 蛮力法
            if (n <= MAX_BRUTE_FORCE) {
                memset(selected, 0, n * sizeof(int));
                start = clock();
                total_value = brute_force(n, items, capacities[c], selected, &total_weight);
                end = clock();
                time_ms = ((double)(end - start) * 1000) / CLOCKS_PER_SEC;
                bf_times[i][c] = time_ms;
                print_results(n, items, selected, total_weight, total_value, time_ms, "蛮力法");
            }

            // 动态规划法
            if (n >= 500 ||n <= MAX_N_TEST ) {
                memset(selected, 0, n * sizeof(int));
                start = clock();
                total_value = dynamic_programming(n, items, capacities[c], selected, &total_weight);
                end = clock();
                time_ms = ((double)(end - start) * 1000) / CLOCKS_PER_SEC;
                dp_times[i][c] = time_ms;
                print_results(n, items, selected, total_weight, total_value, time_ms, "动态规划法");
                if (n == 1000) {
                    char filename[50];
                    snprintf(filename, sizeof(filename), "dp_1000_capacity_%d.csv", capacities[c]);
                    save_to_csv(n, items, selected, total_weight, total_value, filename, capacities[c]);
                }
            }

            // 贪心法
            if (n >= 500 || n <= MAX_N_TEST) {
                memset(selected, 0, n * sizeof(int));
                start = clock();
                total_value = greedy(n, items, capacities[c], selected, &total_weight);
                end = clock();
                time_ms = ((double)(end - start) * 1000) / CLOCKS_PER_SEC;
                greedy_times[i][c] = time_ms;
                print_results(n, items, selected, total_weight, total_value, time_ms, "贪心法");
                if (n == 1000) {
                    char filename[50];
                    snprintf(filename, sizeof(filename), "greedy_1000_capacity_%d.csv", capacities[c]);
                    save_to_csv(n, items, selected, total_weight, total_value, filename, capacities[c]);
                }
            }

            // 回溯法
            if (n <= MAX_N_TEST) {
                memset(selected, 0, n * sizeof(int));
                start = clock();
                total_value = backtracking(n, items, capacities[c], selected, &total_weight);
                end = clock();
                time_ms = ((double)(end - start) * 1000) / CLOCKS_PER_SEC;
                bt_times[i][c] = time_ms;
                print_results(n, items, selected, total_weight, total_value, time_ms, "回溯法");
            }
        }
    }

    // 打印执行时间总结
    printf("\n--- 执行时间总结 (毫秒) ---\n");
    for (int c = 0; c < num_capacities; c++) {
        printf("\n背包容量: %d\n", capacities[c]);
        printf("物品数量   | 蛮力法        | 动态规划法    | 贪心法      | 回溯法        \n");
        printf("-----------|---------------|---------------|-------------|---------------\n");
                for (int i = 0; i < num_items; i++) {
            char buf[16];
            printf("%10d |", item_counts[i]);
            if (bf_times[i][c] >= 0) {
                sprintf(buf, "%12.2f", bf_times[i][c]);
                printf(" %12s |", buf);
            } else {
                printf(" %12s |", "N/A");
            }
            if (dp_times[i][c] >= 0) {
                sprintf(buf, "%12.2f", dp_times[i][c]);
                printf(" %12s |", buf);
            } else {
                printf(" %12s |", "N/A");
            }
            if (greedy_times[i][c] >= 0) {
                sprintf(buf, "%10.2f", greedy_times[i][c]);
                printf(" %10s |", buf);
            } else {
                printf(" %10s |", "N/A");
            }
            if (bt_times[i][c] >= 0) {
                sprintf(buf, "%12.2f", bt_times[i][c]);
                printf(" %12s |", buf);
            } else {
                printf(" %12s |", "N/A");
            }
            printf("\n");
        }
    }

    // 空间复杂度分析
    printf("\n--- 空间复杂度分析 ---\n");
    printf("蛮力法: O(n) 用于选择数组\n");
    printf("动态规划法: O(capacity) 用于动态规划数组\n");
    printf("贪心法: O(n) 用于排序物品数组\n");
    printf("回溯法: O(n) 用于递归栈和选择数组\n");

    free(items); // 释放物品数组
    free(selected); // 释放选择数组
    return 0;
}
