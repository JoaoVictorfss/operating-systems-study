
#include "statistics.h"

void swap(int *a, int *b) {
    int t = *a;

    *a = *b;
    *b = t;
}

int partition(int *list, int low, int high) {
    int pivot = list[high];
    int i = (low - 1);
    int j;

    for (j = low; j <= high - 1; j++) {
        if (list[j] < pivot) {
            i++;
            swap(&list[i], &list[j]);
        }
    }

    swap(&list[i + 1], &list[high]);

    return (i + 1);
}

void rol(int *list, int low, int high) {
    if (low >= high)
        return;
    else {
        int j = partition(list, low, high);
        
        rol(list, low, j - 1);
        rol(list, j + 1, high);
    }
}


Fashion c_fashion(int *list, int size) {
    Fashion fashionData;
    int fashion = -1, fashionCount = 0, cont = 0;

    for (int i = 0; i < size; i++) {
        cont++;
        if (i == size - 1 || list[i] != list[i + 1]) {
            if (cont > fashionCount) {
                fashionCount = cont;
                fashion = list[i];
            }
            cont = 0;
        }
    }

    fashionData.value = fashion;
    fashionData.amount = fashionCount;

    return fashionData;
}