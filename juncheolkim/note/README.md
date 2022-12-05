# malloc lab 개발일지 (김준철)

---

### 22.12.04

팀원들과 동적 메모리 할당에 대한 팀 스터디 진행.

### 22.12.05

CS:APP 챕터9에 나와있는 C언어 코드를 공부하면서 코드 작성.

##### 기본 상수와 매크로 선언

1. 1word 2word 규격 설정
2. 자주 사용되는 함수 선언

##### static void \*extend_heap(size_t words) 함수 작성

heap 영역 확장하는 함수 선언

##### static void \*coalesce(void \*bp) 함수 작성

ptr 포인터에 위치한 할당된 블럭 반환(가용 블럭으로 만들기)
case 2, PUT(FTRP(bp), PACK(size, 0)); 이 가능한 이유 :
바로 직전 코드에서 헤드의 정보를 바꿨기 때문에, 옳게 설정될 수 있다.

##### void \*mm_malloc(size_t size) 함수 작성

기존의 mm_malloc 함수는 여러 조건을 필터링 할 수 없었다.
수정 및 hand-out에 맞게 작성