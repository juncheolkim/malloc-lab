#include <stdio.h>

int main(int argc, char const *argv[])
{
    int arr[10];
    size_t arr_size = sizeof(arr) / sizeof(int);
    printf("%d", arr_size);
    return 0;
}
