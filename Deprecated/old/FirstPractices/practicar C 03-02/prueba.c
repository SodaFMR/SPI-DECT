#include <stdio.h>

int main() {
    
    // Different variables for testing
    int a = 12;
    float b = 3.1415;
    char c = 'C';

    // pointers associated to each variable
    int *ptr_a = &a;
    float *ptr_b = &b;
    char *ptr_c = &c;

    // Print the values and memory directions of each variable
    printf("Valor de a: %d, Dirección de a: %p\n", a, ptr_a);
    printf("Valor de b: %.2f, Dirección de b: %p\n", b, ptr_b);
    printf("Valor de c: %c, Dirección de c: %p\n\n", c, (void*)ptr_c); // Another way to print the directionº

    // Print the size of each variable
    printf("Tamaño de int: %lu bytes\n", sizeof(int));
    printf("Tamaño de float: %lu bytes\n", sizeof(float));
    printf("Tamaño de char: %lu bytes\n\n", sizeof(char));
    
    
    // Create an int array and check each value with a pointer
    int array[] = {10, 20, 30, 40, 50};
    int *ptr_array = array; // Pointer to the first element of the array

    printf("Recorriendo el array con for:\n");
    for (int i = 0; i < 5; i++) {
        printf("Elemento %d: %d (Dirección: %p)\n", i, *(ptr_array + i), (void*)(ptr_array + i));
    }

    printf("\nRecorriendo el array con while:\n");
    int *ptr = array; // Reiniciar puntero
    int i = 0;
    while (i < 5) {
        printf("Elemento %d: %d (Dirección: %p)\n", i, *ptr, (void*)ptr);
        ptr++;
        i++;
    }

    return 0;
}