// 예제 1

#include <stdlib.h>
#include <stdio.h>

int main()
{
    void *p = malloc(4);
    printf("%p\n", p);
    // Returns: 성공적이라면 할당된 블럭에 대한 포인터, 아니라면 에러로써 NULL
}
