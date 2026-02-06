#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <math.h>

void bubble_sort(int *array, int array_size);
void merge_arrays(int *A, int *L, int leftCount, int *R, int rightCount);

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

    // Crear y llenar arreglo con números aleatorios
    int *array = malloc(array_size * sizeof(int));
    int *array_seq = malloc(array_size * sizeof(int)); // Para ordenamiento secuencial
    
    srand(time(NULL));
    for (int i = 0; i < array_size; i++) {
        array[i] = rand() % 10000;
        array_seq[i] = array[i];
    }

    // ========== ORDENAMIENTO SECUENCIAL ==========
    clock_t start_seq = clock();
    bubble_sort(array_seq, array_size);
    clock_t end_seq = clock();
    double time_seq = (double)(end_seq - start_seq) / CLOCKS_PER_SEC;

    // ========== ORDENAMIENTO CON FORK ==========
    clock_t start_fork = clock();

    // Crear memoria compartida
    int *shared_array = mmap(NULL, array_size * sizeof(int), 
                           PROT_READ | PROT_WRITE, 
                           MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    // Copiar datos a memoria compartida
    memcpy(shared_array, array, array_size * sizeof(int));

    // Dividir el arreglo en dos partes
    int mid = array_size / 2;
    
    pid_t pid1 = fork();
    if (pid1 == 0) {
        // Proceso hijo 1 - ordena primera mitad
        bubble_sort(shared_array, mid);
        exit(0);
    } else if (pid1 > 0) {
        // Proceso padre
        pid_t pid2 = fork();
        
        if (pid2 == 0) {
            // Proceso hijo 2 - ordena segunda mitad
            bubble_sort(shared_array + mid, array_size - mid);
            exit(0);
        } else if (pid2 > 0) {
            // Esperar a que ambos hijos terminen
            wait(NULL);
            wait(NULL);
            
            // Fusionar los resultados
            int *left = malloc(mid * sizeof(int));
            int *right = malloc((array_size - mid) * sizeof(int));
            
            memcpy(left, shared_array, mid * sizeof(int));
            memcpy(right, shared_array + mid, (array_size - mid) * sizeof(int));
            
            merge_arrays(shared_array, left, mid, right, array_size - mid);
            
            free(left);
            free(right);
            
            clock_t end_fork = clock();
            double time_fork = (double)(end_fork - start_fork) / CLOCKS_PER_SEC;

            // Verificar que esté ordenado
            int sorted = 1;
            for (int i = 1; i < array_size; i++) {
                if (shared_array[i-1] > shared_array[i]) {
                    sorted = 0;
                    break;
                }
            }

            printf("\n=== RESULTADOS ===\n");
            printf("Ordenamiento secuencial: %.6f segundos\n", time_seq);
            printf("Ordenamiento con fork(): %.6f segundos\n", time_fork);
            printf("Arreglo ordenado correctamente: %s\n", sorted ? "Sí" : "No");

            // Liberar memoria
            munmap(shared_array, array_size * sizeof(int));
        }
    }

    free(array);
    free(array_seq);
    return 0;
}
