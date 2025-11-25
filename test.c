#include <stdio.h>
#include <stdlib.h>


int main(){

    int* x = malloc(sizeof(int));
    //x = malloc(sizeof(int));
    //x = malloc(sizeof(int));
    *x = 20;
    printf("%d \n", *x);
    free(x);
    free(x);
    return 0;

}