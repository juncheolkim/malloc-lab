// 예제 2

#include <unistd.h>
// #include <stdlib.h>
// #include <stdio.h>

int main()
{
    void *sbrk(intptr_t incr);
    // Returns: 성공한다면 이전의 brk 값을 리턴, 아니라면 -1 리턴 후 ENOMEM으로 errno 설정
}
