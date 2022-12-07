# malloc lab 개발일지 (김준철)

---

## 22.12.04

팀원들과 동적 메모리 할당에 대한 팀 스터디 진행.

## 22.12.05

CS:APP 챕터9에 나와있는 C언어 코드를 공부하면서 코드 작성.

#### 1. 기본 상수와 매크로 선언

-   1word 2word 규격 설정
-   자주 사용되는 함수 선언

#### 2. static void \*extend_heap(size_t words) 함수 작성

heap 영역 확장하는 함수 선언

#### 3. static void \*coalesce(void \*bp) 함수 작성

ptr 포인터에 위치한 할당된 블럭 반환(가용 블럭으로 만들기)
case 2, PUT(FTRP(bp), PACK(size, 0)); 이 가능한 이유 :
바로 직전 코드에서 헤드의 정보를 바꿨기 때문에, 옳게 설정될 수 있다.

#### 4. void \*mm_malloc(size_t size) 함수 작성

기존의 mm_malloc 함수는 여러 조건을 필터링 할 수 없었다.
수정 및 hand-out에 맞게 작성

#### 5. static void \*find_fit(size_t asize) 함수 작성

first-fit 방식으로 find_fit 함수 작성

#### 6. static void place(void \*bp, size_t asize) 함수 작성

가용 블럭에 데이터를 할당할 때,가용 블럭을 나눠주는 함수 작성

#### 7. void *mm_realloc(void *ptr, size_t size) 함수 작성

블럭의 메모리를 변경하는 reaaloc 함수 작성

## 22.12.06

명시적 가용 리스트 만들기 = mm.c
기존 묵시적 가용 리스트 코드 = mm_implicit.c

-   root_free 선언

#### 1. static void \*extend_heap(size_t words) 수정

-   가용 블럭의 prev, next 값 갱신하는 코드 작성

#### 2. void mm_free(void \*ptr) 수정

-   새로 생기는 가용 블럭으로의 root_free 값 수정

#### 3. static void \*coalesce(void \*bp) 수정

-   case에 따라서 두 블럭의 prev, next 값 갱신하는 코드 작성

#### 4. static void place(void \*bp, size_t asize) 수정

-   가용 블럭의 최소 단위는 4\*DSIZE로 변경

#### 5. 작업 도중에 root_free가 NULL값이 된다면? (가용블럭이 하나도 없는 상태)

-   free와 extend_heap 함수에서 조건문으로 검사해야한다.
-   free : if (root_free != NULL) 를 통해서 root_free값이 존재할 때만 새로운 가용 블럭과 연결한다.
-   extend_heap : init을 고려하여 미리 설정한 조건문이 확인해준다.

#### 6. static void \*find_fit(size_t asize) 수정

-   for문의 조건을 수정하여 root_free부터 탐색하도록 했다.

#### 7. segmentation fault 발생 (오마이갓 비상사태 큰일이다)

-   형식자 지정을 옳게 지정하지 못해 발생하는 것으로 가정

## 22.12.07

#### 1. 익일 발생한 오류로 인해 다시 작업 시작

-   익일 place 함수 수정중에 root_free가 없어지는 상황을 어떻게 처리할 것인지
-   case로 나누어 작동

#### 2. static void \*find_fit(size_t asize) 수정

-   [1] 1658 segmentation fault ./mdriver 발생

#### 3. root_free 를 char\* 로 지정하여 다시 작업 시작

-   NULL 포인터와 int 0 은 서로 형 변환이 가능하다!!! (세상에;)

#### 4. 발견 허점들

-   coalesce 함수에서 합쳐지는 블럭이 마지막 가용블럭이었을 경우 계산을 놓침.
-   place 함수에서 최소 가용블럭 크기는 이전과 동일하게 2\*DSIZE 이다.
    왜냐하면, 헤더(1word) + 풋터(1word) + 후임자(1word) + 선임자(1word) = 4\*word = 2\*double word 이기 떄문이다.
-   free에서 root_free 최신화를 안했다.
-   find_fit 함수에서 for문이 아닌 while문으로 마지막 블럭 검색도 포함시킨다.
-   [1] 5360 segmentation fault ./mdriver 발생
