/*
 * memlib.c - a module that simulates the memory system.  Needed because it 
 *            allows us to interleave calls from the student's malloc package 
 *            with the system's malloc package in libc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

#include "memlib.h"
#include "config.h"

/* private variables */
static char *mem_start_brk;  /* 힙의 첫번째 바이트를 가리키는 포인터 */
static char *mem_brk;        /* 힙의 마지막 바이트를 가리키는 포인터 */
static char *mem_max_addr;   /* 가장 큰 legal한 힙 주소 */ 

/* 
 * mem_init - 메모리 시스템 모델을 초기화
 */
void mem_init(void)
{
    /* 힙으로 사용 가능한 가상 메모리를 만들어 줌, 바이트의 더블 워드 정렬 형태로. 
    사용가능한 가상 메모리를 모델링하기 위해 사용할 저장 공간을 할당 */

    // 20MB 메모리를 힙으로 쓰게끔 말록으로 할당 요청 (=20971520 bytes, 5342880개 워드)
    // 에러 처리 구문 
    if ((mem_start_brk = (char *)malloc(MAX_HEAP)) == NULL) {
	fprintf(stderr, "mem_init_vm: malloc error\n");
	exit(1); // 초기화 실패인 경우 1 리턴 (책에서는 -1)
    }

    mem_max_addr = mem_start_brk + MAX_HEAP;  /* 최대로 legal하게 사용할 수 있는 힙 주소 */
    mem_brk = mem_start_brk;                  /* 힙은 초기에는 empty 상태  */
}

/* 
 * mem_deinit - free the storage used by the memory system model
 */
void mem_deinit(void)
{
    free(mem_start_brk);
}

/*
 * mem_reset_brk - brk 포인터를 리셋하여 비어있는 힙을 만듦
 */
void mem_reset_brk()
{
    mem_brk = mem_start_brk;
}

/* 
 * mem_sbrk - 힙을 incr 바이트 만큼 늘려주고, 새로운 구역의 시작 주소를 돌려줌
 * 이 모델에서는 힙이 줄어들지 않음! 
 */
void *mem_sbrk(int incr) 
{
    char *old_brk = mem_brk; 

    // mem_brk와 증가값을 더한 게 최대 힙 영역을 넘어가면 error
    if ( (incr < 0) || ((mem_brk + incr) > mem_max_addr)) {
	errno = ENOMEM;
	fprintf(stderr, "ERROR: mem_sbrk failed. Ran out of memory...\n");
	return (void *)-1;
    }
    mem_brk += incr;  // 현재 brk에 증가값을 추가하여 가상 메모리 더 받아냄 
    return (void *)old_brk; // old_brk는 새로운 힙 구역을 가리키는 포인터가 됨. 
}

/*
 * mem_heap_lo - 처음 heap byte의 주소 반환
 */
void *mem_heap_lo()
{
    return (void *)mem_start_brk;
}

/* 
 * mem_heap_hi - 마지막 heap byte의 주소 반환
 */
void *mem_heap_hi()
{
    return (void *)(mem_brk - 1);
}

/*
 * mem_heapsize() - 힙 사이즈를 바이트로 반환
 */
size_t mem_heapsize() 
{
    return (size_t)(mem_brk - mem_start_brk);
}

/*
 * mem_pagesize() - 시스템의 페이지 사이즈를 반환함 ==이게뭐람==
 */
size_t mem_pagesize()
{
    return (size_t)getpagesize();
}
