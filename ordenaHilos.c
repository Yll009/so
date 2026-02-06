#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

typedef struct {
    int *array;
    int start;
    int end;
} ThreadData;

void bubble_sort(int *array, int array_size);
void merge_arrays(int *A, int *L, int leftCount, int *R, int rightCount);
void* sort_thread(void *arg);

void bubble_sort(int *array, int array_size) {
    int sortFlag = 0;
    int tempHolder = 0;

    do {
        sortFlag = 1;
        for (int i = 1; i < array_size; i++) {
            if (array[i-1] > array[i]) {
                tempHolder = array[i-1];
                array[i-1] = array[i];
                array[i] = tempHolder;
            } else {
                sortFlag++;
            }
        }
    } while (sortFlag < array_size);
}

void merge_arrays(int *original_array, int *left, int left_count, int *right, int right_count) {
    int i = 0;
    int j = 0;
    int k = 0;

    while(i < left_count && j < right_count) {
        if(left[i] < right[j]) 
            original_array[k++] = left[i++];
        else 
            original_array[k++] = right[j++];
    }
    while(i < left_count) 
        original_array[k++] = left[i++];
    while(j < right_count) 
        original_array[k++] = right[j++];
}

void* sort_thread(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    bubble_sort(data->array + data->start, data->end - data->start);
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <tamaño_del_arreglo>\n", argv[0]);
        return -1;
    }

    int array_size = atoi(argv[1]);
    if (array_size <= 0) {
        fprintf(stderr, "El tamaño del arreglo debe ser positivo\n");
        return -1;
    }

    printf("Tamaño del arreglo: %d elementos\n", array_size);

    // Crear y llenar arreglos
    int *array = malloc(array_size * sizeof(int));
    int *array_seq = malloc(array_size * sizeof(int));
    int *array_threads = malloc(array_size * sizeof(int));
    
    srand(time(NULL));
    for (int i = 0; i < array_size; i++) {
        array[i] = rand() % 10000;
        array_seq[i] = array[i];
        array_threads[i] = array[i];
    }

    // ========== ORDENAMIENTO SECUENCIAL ==========
    clock_t start_seq = clock();
    bubble_sort(array_seq, array_size);
    clock_t end_seq = clock();
    double time_seq = (double)(end_seq - start_seq) / CLOCKS_PER_SEC;

    // ========== ORDENAMIENTO CON HILOS ==========
    clock_t start_threads = clock();

    pthread_t threads[2];
    ThreadData thread_data[2];
    
    // Dividir el trabajo entre dos hilos
    int mid = array_size / 2;
    
    // Primer hilo - primera mitad
    thread_data[0].array = array_threads;
    thread_data[0].start = 0;
    thread_data[0].end = mid;
    
    // Segundo hilo - segunda mitad
    thread_data[1].array = array_threads;
    thread_data[1].start = mid;
    thread_data[1].end = array_size;
    
    // Crear hilos
    pthread_create(&threads[0], NULL, sort_thread, (void *)&thread_data[0]);
    pthread_create(&threads[1], NULL, sort_thread, (void *)&thread_data[1]);
    
    // Esperar a que los hilos terminen
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    
    // Fusionar los resultados
    int *left = malloc(mid * sizeof(int));
    int *right = malloc((array_size - mid) * sizeof(int));
    
    memcpy(left, array_threads, mid * sizeof(int));
    memcpy(right, array_threads + mid, (array_size - mid) * sizeof(int));
    
    merge_arrays(array_threads, left, mid, right, array_size - mid);
    
    free(left);
    free(right);
    
    clock_t end_threads = clock();
    double time_threads = (double)(end_threads - start_threads) / CLOCKS_PER_SEC;

    // Verificar que esté ordenado
    int sorted = 1;
    for (int i = 1; i < array_size; i++) {
        if (array_threads[i-1] > array_threads[i]) {
            sorted = 0;
            break;
        }
    }

    printf("\n=== RESULTADOS ===\n");
    printf("Ordenamiento secuencial: %.6f segundos\n", time_seq);
    printf("Ordenamiento con hilos: %.6f segundos\n", time_threads);
    printf("Arreglo ordenado correctamente: %s\n", sorted ? "Sí" : "No");

    // Liberar memoria
    free(array);
    free(array_seq);
    free(array_threads);
    
    return 0;
}