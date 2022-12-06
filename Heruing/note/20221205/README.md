# 2022-12-05

## Malloc Lab 구현하기

### 1. 교재 내용 구현하기

- 구현 필요한 내용
  - `realloc`
    - 구현 자체가 안 되어있음
  - `find_fit`
    - 현재 `first-fit`으로 구현된 내용을 `next-fit`, `best-fit`으로 구현해보기
    - `list` 방식에 대해서 구현해보기(`best-fit`)

- 구현해보기

  - `realloc`

    ```c
    void *mm_realloc(void *ptr, size_t size)
    {
    	void *oldptr = ptr;
    	void *newptr;
    	size_t copySize;
    	if (size <= 0)
    		return NULL;
    
        // size should be allocated(align and add heder, footer size)
    	size = ALIGN(size) + DSIZE;     
    	newptr = mm_malloc(size);		//	new pointer to allocate
    	if (newptr == NULL)
    		return NULL;
    
    	copySize = GET_SIZE(HDRP(oldptr)) - DSIZE;
        //copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    	if (size < copySize)
    		copySize = size;
    
    	memcpy(newptr, oldptr, copySize);
    	mm_free(oldptr);
    	return newptr;
    }
    ```

    - 기본적으로 Header와 Footer를 사용하는 형태로 이루어져 있으므로
      bp와 header, footer의 블럭을 고려하여 사이즈를 조절
    - copySize의 DSIZE를 빼도 정상적으로 돌아가는데, 무엇이 옳은가에 대한 고민 필요?
  
    

