#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define MAX_ITEMS 40000 // �����Ʒ����
#define MAX_CAPACITY 50000 // ��󱳰�����
#define MAX_BRUTE_FORCE 100// ����������
#define MAX_N_TEST 20 // ���ݷ�����Ʒ��������

// ��Ʒ�ṹ��
typedef struct {
    int index; // ��Ʒ���
    double weight; // ��Ʒ����
    double value; // ��Ʒ��ֵ
} Item;

// ��������
void generate_items(int n, Item items[], unsigned int seed); // ���������Ʒ
double brute_force(int n, Item items[], int capacity, int selected[], int *total_weight); // ������
double dynamic_programming(int n, Item items[], int capacity, int selected[], int *total_weight); // ��̬�滮��
double greedy(int n, Item items[], int capacity, int selected[], int *total_weight); // ̰�ķ�
double backtracking(int n, Item items[], int capacity, int selected[], int *total_weight); // ���ݷ�
void print_results(int n, Item items[], int selected[], int total_weight, double total_value, double time_ms, const char *method); // ��ӡ���
void save_to_csv(int n, Item items[], int selected[], int total_weight, double total_value, const char *filename, int capacity); // �������� CSV

// ����̰�ķ��� qsort �ȽϺ���
int compare(const void *a, const void *b) {
    Item *item1 = (Item *)a;
    Item *item2 = (Item *)b;
    double ratio1 = item1->value / item1->weight; // ��Ʒ1�ĵ�λ��ֵ
    double ratio2 = item2->value / item2->weight; // ��Ʒ2�ĵ�λ��ֵ
    return ratio2 > ratio1 ? 1 : -1; // ����λ��ֵ��������
}

// ���������Ʒ
void generate_items(int n, Item items[], unsigned int seed) {
    srand(seed); // �����������
    for (int i = 0; i < n; i++) {
        items[i].index = i; // ������Ʒ���
        items[i].weight = (rand() % 100) + 1; // ������1��100
        items[i].value = 100.00 + (rand() % 90001) / 100.0; // ��ֵ��100.00��1000.00
    }
}

// ������
double brute_force(int n, Item items[], int capacity, int selected[], int *total_weight) {
    if (n > MAX_BRUTE_FORCE) return -1.0; // ��Ʒ�����������ƣ�����
    double max_value = 0.0; // ����ֵ
    *total_weight = 0; // ��������ʼ��
    unsigned int max_subsets = 1u << n; // �����Ӽ�����

    for (unsigned int i = 0; i < max_subsets; i++) {
        double curr_weight = 0.0; // ��ǰ����
        double curr_value = 0.0; // ��ǰ��ֵ
        int temp_selected[MAX_N_TEST] = {0}; // ��ʱѡ������

        for (int j = 0; j < n; j++) {
            if (i & (1u << j)) { // ����Ƿ�ѡ���j����Ʒ
                curr_weight += items[j].weight;
                curr_value += items[j].value;
                temp_selected[j] = 1;
            }
        }

        if (curr_weight <= capacity && curr_value > max_value) { // �������Ž�
            max_value = curr_value;
            *total_weight = (int)curr_weight;
            for (int j = 0; j < n; j++) {
                selected[j] = temp_selected[j];
            }
        }
    }
    return max_value;
}

// ��̬�滮�����Ż��ڴ棬ȥ�� keep ���飩
double dynamic_programming(int n, Item items[], int capacity, int selected[], int *total_weight) {
    int *dp = (int *)calloc(capacity + 1, sizeof(int)); // �������洢
    int **keep = (int **)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        keep[i] = (int *)calloc(capacity + 1, sizeof(int));
    }
    if (!dp || !keep) {
        printf("��̬�滮�ڴ����ʧ��\n");
        exit(1);
    }

    // ��䶯̬�滮��
    for (int i = 0; i < n; i++) {
        int wgt = (int)items[i].weight;
        int val = (int)round(items[i].value * 100); // ��ֵתΪ����
        for (int w = capacity; w >= wgt; w--) {
            if (dp[w] < dp[w - wgt] + val) {
                dp[w] = dp[w - wgt] + val;
                keep[i][w] = 1;
            }
        }
    }

    // ����ѡ����Ʒ
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

// ̰�ķ�������ֵ/�����ȣ�
double greedy(int n, Item items[], int capacity, int selected[], int *total_weight) {
    Item *sorted_items = (Item *)malloc(n * sizeof(Item)); // ��������Ʒ����
    if (!sorted_items) {
        printf("������Ʒ�ڴ����ʧ��\n");
        exit(1);
    }
    memcpy(sorted_items, items, n * sizeof(Item));
    qsort(sorted_items, n, sizeof(Item), compare); // ����λ��ֵ����

    double total_value = 0.0; // �ܼ�ֵ
    *total_weight = 0; // ������
    for (int i = 0; i < n; i++) {
        if (*total_weight + (int)sorted_items[i].weight <= capacity) {
            selected[sorted_items[i].index] = 1;
            *total_weight += (int)sorted_items[i].weight;
            total_value += sorted_items[i].value;
        }
    }

    free(sorted_items); // �ͷ��ڴ�
    return total_value;
}

// ���ݷ�
int cmp_value_per_weight(const void *a, const void *b) {
    Item *ia = (Item*)a, *ib = (Item*)b;
    double r1 = ia->value / ia->weight, r2 = ib->value / ib->weight;
    return r2 > r1 ? 1 : (r2 < r1 ? -1 : 0);
}
void backtrack(int i, int n, Item items[], int capacity, int curr_weight, double curr_value,
               int selected[], int *best_selected, double *max_value, int *best_weight, double remain_value) {
    if (curr_weight > capacity) return; // ���ؼ�֦
    if (i == n) {
        if (curr_value > *max_value) {
            *max_value = curr_value;
            *best_weight = curr_weight;
            memcpy(best_selected, selected, n * sizeof(int));
        }
        return;
    }
    // ��֦��ʣ�������ܼ�ֵ�����統ǰ����
    if (curr_value + remain_value <= *max_value) return;

    // ��ѡ��i��
    selected[i] = 0;
    backtrack(i + 1, n, items, capacity, curr_weight, curr_value, selected, best_selected, max_value, best_weight, remain_value - items[i].value);

    // ѡ��i��
    selected[i] = 1;
    backtrack(i + 1, n, items, capacity, curr_weight + (int)items[i].weight,
              curr_value + items[i].value, selected, best_selected, max_value, best_weight, remain_value - items[i].value);
    selected[i] = 0;
}

double backtracking(int n, Item items[], int capacity, int selected[], int *total_weight) {
    if (n > MAX_N_TEST) return -1.0;
    // ��ѡ���Ȱ���λ��ֵ����������֦Ч��
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

    // ��ԭ��ԭ˳��
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

// ��ӡ���
void print_results(int n, Item items[], int selected[], int total_weight, double total_value, double time_ms, const char *method) {
    if (total_value < 0) {
        printf("\n%s ������ (n=%d)\n", method, n);
        return;
    }
    printf("\n%s ��� (n=%d):\n", method, n);
    printf("ѡ�����Ʒ (���, ����, ��ֵ):\n");
    int count = 0;
    for (int i = 0; i < n && count < 10; i++) {
        if (selected[i]) {
            printf("��Ʒ %d: ����=%.0f, ��ֵ=%.2f\n", items[i].index, items[i].weight, items[i].value);
            count++;
        }
    }
    if (count >= 10) printf("... (������Ʒ��ѡ��)\n");
    printf("������: %d\n", total_weight);
    printf("�ܼ�ֵ: %.2f\n", total_value);
    printf("ִ��ʱ��: %.2f ����\n", time_ms);
}

// �������� CSV �ļ� (n=1000)
void save_to_csv(int n, Item items[], int selected[], int total_weight, double total_value, const char *filename, int capacity) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        printf("�޷����ļ� %s\n", filename);
        return;
    }
    fprintf(fp, "��Ʒ���,����,��ֵ,�Ƿ�ѡ��\n");
    for (int i = 0; i < n; i++) {
        fprintf(fp, "%d,%.0f,%.2f,%d\n", items[i].index, items[i].weight, items[i].value, selected[i]);
    }
    fprintf(fp, "\n������,%d\n", total_weight);
    fprintf(fp, "�ܼ�ֵ,%.2f\n", total_value);
    fprintf(fp, "��������,%d\n", capacity);
    fclose(fp);
}

int main() {
    // ͳһ��������
    int item_counts[] = {5, 8, 10, 15, 20, 100, 500, 1000, 2000, 3000, 4000, 5000, 10000, 20000, 30000, 40000}; // ��Ʒ���� 45000 �ѵ����� 
    int capacities[] = { 500, 1000, 10000, 50000}; // ��������
    int num_items = sizeof(item_counts) / sizeof(item_counts[0]);
    int num_capacities = sizeof(capacities) / sizeof(capacities[0]);

    Item *items = (Item *)malloc(MAX_ITEMS * sizeof(Item)); // ��Ʒ����
    int *selected = (int *)calloc(MAX_ITEMS, sizeof(int)); // ѡ������
    if (!items || !selected) {
        printf("��Ʒ��ѡ�������ڴ����ʧ��\n");
        exit(1);
    }
    int total_weight; // ������
    double total_value; // �ܼ�ֵ
    clock_t start, end; // ʱ���¼
    double time_ms; // ִ��ʱ�䣨���룩

    // ִ��ʱ���ܽ�����
    double bf_times[16][4]; // ������
    double dp_times[16][4]; // ��̬�滮��
    double greedy_times[16][4]; // ̰�ķ�
    double bt_times[16][4]; // ���ݷ�
    for (int i = 0; i < 16; i++) {
        for (int c = 0; c < 4; c++) {
            bf_times[i][c] = dp_times[i][c] = greedy_times[i][c] = bt_times[i][c] = -1.0;
        }
    }

    printf("���� 0-1 ������������㷨...\n");

    // ���������㷨
    for (int c = 0; c < num_capacities; c++) {
        printf("\n--- ��������: %d ---\n", capacities[c]);
        for (int i = 0; i < num_items; i++) {
            int n = item_counts[i];
            printf("\n���� n=%d\n", n);
            generate_items(n, items, 12345); // ���������Ʒ���̶�����

            // ������
            if (n <= MAX_BRUTE_FORCE) {
                memset(selected, 0, n * sizeof(int));
                start = clock();
                total_value = brute_force(n, items, capacities[c], selected, &total_weight);
                end = clock();
                time_ms = ((double)(end - start) * 1000) / CLOCKS_PER_SEC;
                bf_times[i][c] = time_ms;
                print_results(n, items, selected, total_weight, total_value, time_ms, "������");
            }

            // ��̬�滮��
            if (n >= 500 ||n <= MAX_N_TEST ) {
                memset(selected, 0, n * sizeof(int));
                start = clock();
                total_value = dynamic_programming(n, items, capacities[c], selected, &total_weight);
                end = clock();
                time_ms = ((double)(end - start) * 1000) / CLOCKS_PER_SEC;
                dp_times[i][c] = time_ms;
                print_results(n, items, selected, total_weight, total_value, time_ms, "��̬�滮��");
                if (n == 1000) {
                    char filename[50];
                    snprintf(filename, sizeof(filename), "dp_1000_capacity_%d.csv", capacities[c]);
                    save_to_csv(n, items, selected, total_weight, total_value, filename, capacities[c]);
                }
            }

            // ̰�ķ�
            if (n >= 500 || n <= MAX_N_TEST) {
                memset(selected, 0, n * sizeof(int));
                start = clock();
                total_value = greedy(n, items, capacities[c], selected, &total_weight);
                end = clock();
                time_ms = ((double)(end - start) * 1000) / CLOCKS_PER_SEC;
                greedy_times[i][c] = time_ms;
                print_results(n, items, selected, total_weight, total_value, time_ms, "̰�ķ�");
                if (n == 1000) {
                    char filename[50];
                    snprintf(filename, sizeof(filename), "greedy_1000_capacity_%d.csv", capacities[c]);
                    save_to_csv(n, items, selected, total_weight, total_value, filename, capacities[c]);
                }
            }

            // ���ݷ�
            if (n <= MAX_N_TEST) {
                memset(selected, 0, n * sizeof(int));
                start = clock();
                total_value = backtracking(n, items, capacities[c], selected, &total_weight);
                end = clock();
                time_ms = ((double)(end - start) * 1000) / CLOCKS_PER_SEC;
                bt_times[i][c] = time_ms;
                print_results(n, items, selected, total_weight, total_value, time_ms, "���ݷ�");
            }
        }
    }

    // ��ӡִ��ʱ���ܽ�
    printf("\n--- ִ��ʱ���ܽ� (����) ---\n");
    for (int c = 0; c < num_capacities; c++) {
        printf("\n��������: %d\n", capacities[c]);
        printf("��Ʒ����   | ������        | ��̬�滮��    | ̰�ķ�      | ���ݷ�        \n");
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

    // �ռ临�Ӷȷ���
    printf("\n--- �ռ临�Ӷȷ��� ---\n");
    printf("������: O(n) ����ѡ������\n");
    printf("��̬�滮��: O(capacity) ���ڶ�̬�滮����\n");
    printf("̰�ķ�: O(n) ����������Ʒ����\n");
    printf("���ݷ�: O(n) ���ڵݹ�ջ��ѡ������\n");

    free(items); // �ͷ���Ʒ����
    free(selected); // �ͷ�ѡ������
    return 0;
}
