#include "list.h"
#include "hash.h"
#include "bitmap.h"
#include "hex_dump.h"
#include "round.h"
#include "debug.h"
#include "limits.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_SIZE 10
#define hash_entry(PTR, TYPE, MEMBER) \
    ((TYPE *)((char *)(PTR) - (unsigned long)(&((TYPE *)0)->MEMBER)))

struct my_struct
{
    struct hash_elem elem; // 해시 테이블 요소
    int data;              // 실제 저장할 데이터
};

struct my_data
{
    struct list_elem elem; // 리스트 요소
    int data;              // 실제 데이터// 요소가 속한 리스트를 가리키는 포인터
    // 여기에 추가 필드가 있을 수 있습니다.
};

struct bitmap *bitmap_list[MAX_SIZE];
struct list *list_list[MAX_SIZE];
struct hash *hash_tables[MAX_SIZE] = {NULL};

bool less(const struct list_elem *elem_a, const struct list_elem *elem_b, void *aux)
{
    // list_elem을 포함하는 my_data 구조체로 변환합니다.
    const struct my_data *da = list_entry(elem_a, struct my_data, elem);
    const struct my_data *db = list_entry(elem_b, struct my_data, elem);

    // data 필드를 비교하여 결과를 반환합니다.
    return da->data < db->data;
}

// 자료 구조 목록을 관리할 배열과 카운터를 정의합니다.
#define MAX_STRUCTURES 100

// bitmap_create 함수와 필요한 헤더 파일이 포함되어야 합니다.
/*만들기*/
unsigned hash_my_struct(const struct hash_elem *e, void *aux)
{
    const struct my_struct *p = hash_entry(e, struct my_struct, elem);
    return hash_int(p->data);
}

bool hash_less_my_struct(const struct hash_elem *a, const struct hash_elem *b, void *aux)
{
    const struct my_struct *a_ = hash_entry(a, struct my_struct, elem);
    const struct my_struct *b_ = hash_entry(b, struct my_struct, elem);
    return a_->data < b_->data;
}

void create_hash(const char *name)
{
    int index = -1;
    if (sscanf(name, "hash%d", &index) == 1 && index >= 0 && index < MAX_SIZE)
    {
        if (hash_tables[index] == NULL)
        {
            // 해당 인덱스의 해시 테이블이 아직 할당되지 않았다면 해시 테이블에 메모리 할당 및 초기화
            hash_tables[index] = malloc(sizeof(struct hash));
            if (hash_tables[index] != NULL)
            {
                // 해시 테이블 초기화 시 사용자 정의 해시 함수와 비교 함수 전달
                if (hash_init(hash_tables[index], hash_my_struct, hash_less_my_struct, NULL))
                {
                }
                else
                {
                    // 초기화 실패 시 메모리 해제
                    printf("Failed to initialize hash table %s.\n", name);
                    free(hash_tables[index]);
                    hash_tables[index] = NULL;
                }
            }
            else
            {
                // 메모리 할당 실패 메시지
                printf("Memory allocation failed for hash table %s.\n", name);
            }
        }
        else
        {
            // 이미 초기화된 해시 테이블이 있을 경우
            printf("Hash table %s is already initialized.\n", name);
        }
    }
    else
    {
        // 유효하지 않은 해시 테이블 이름 또는 인덱스 범위 오류 메시지
        printf("Invalid hash table name or index out of range: %s\n", name);
    }
}

void createBitmap(const char *name, size_t bit_cnt)
{
    // Check for valid name format "bmX" and calculate index
    int index = -1;
    if (sscanf(name, "bm%d", &index) == 1 && index >= 0 && index < MAX_SIZE)
    {
        // Check if bitmap already exists at the index
        if (bitmap_list[index] != NULL)
        {
            printf("A bitmap already exists at index %d. Please delete it before creating a new one.\n", index);
            return;
        }

        // Create the bitmap
        struct bitmap *newBitmap = bitmap_create(bit_cnt);
        if (newBitmap == NULL)
        {
            printf("Failed to create bitmap %s.\n", name);
            return;
        }

        // Store the pointer in the global array
        bitmap_list[index] = newBitmap;
    }
    else
    {
        printf("Invalid bitmap name: %s. Expected format is 'bmX'.\n", name);
    }
}

void create_list(const char *name)
{
    int index = -1;
    // 이름에서 숫자를 추출
    if (sscanf(name, "list%d", &index) == 1 && index >= 0 && index < MAX_SIZE)
    {
        // 이미 리스트가 생성되었는지 확인
        if (list_list[index] == NULL)
        {
            // 메모리 할당 및 초기화
            list_list[index] = malloc(sizeof(struct list));
            if (list_list[index] != NULL)
            {
                list_init(list_list[index]);
            }
            else
            {
                printf("Memory allocation failed for list '%s'.", name);
            }
        }
        else
        {
            printf("List '%s' at index %d is already initialized.", name, index);
        }
    }
    else
    {
        printf("Error: Invalid list name or index out of bounds.");
    }
}

// bitmap_list 배열 초기화를 위한 함수가 필요할 수 있습니다.
void initializeBitmapList()
{
    for (int i = 0; i < MAX_SIZE; ++i)
    {
        bitmap_list[i] = NULL; // 모든 포인터를 NULL로 초기화
    }
}

void initialize_all_lists()
{
    for (int i = 0; i < MAX_SIZE; i++)
    {
        list_init(&list_list[i]);
    }
}

/*없애기*/
/*완성안됨*/
void delete_list(struct list *my_list)
{
    struct list_elem *e = my_list->head.next; // 리스트의 첫 번째 요소부터 시작

    while (e != &my_list->tail)
    {                                                               // tail 요소에 도달할 때까지 반복
        struct my_data *data = list_entry(e, struct my_data, elem); // 현재 요소를 my_data 구조체로 변환
        struct list_elem *next = e->next;                           // 다음 요소를 저장

        free(data); // 현재 my_data 구조체 메모리 해제
        e = next;   // 다음 요소로 이동
    }
}

/*내보내기*/

void dumpdata_bitmap_binary(const struct bitmap *b)
{
    bool data_printed = false; // 데이터가 출력되었는지 여부를 추적

    for (size_t i = 0; i < b->bit_cnt; ++i)
    {
        printf("%d", bitmap_test(b, i) ? 1 : 0);
        data_printed = true; // 데이터 출력
    }

    if (data_printed)
    {
        printf("\n"); // 데이터가 출력되었다면 개행 문자 출력
    }
}

void dumpdata_list(const struct list *list)
{
    struct list_elem *e;
    bool data_printed = false; // 데이터가 출력되었는지 여부를 추적

    for (e = list_begin(list); e != list_end(list); e = list_next(e))
    {
        // list_entry를 사용하여 struct list_elem에서 struct my_data로 변환합니다.
        struct my_data *data = list_entry(e, struct my_data, elem);
        printf("%d ", data->data); // 실제 데이터 출력
        data_printed = true;       // 데이터 출력
    }

    if (data_printed)
    {
        printf("\n"); // 데이터가 출력되었다면 개행 문자 출력
    }
}

void dumpdata_hash(const struct hash *h)
{
    bool data_printed = false; // 데이터가 출력되었는지 여부를 추적하는 플래그

    for (size_t i = 0; i < h->bucket_cnt; i++)
    { // 모든 버킷 순회
        struct list *bucket = &h->buckets[i];
        struct list_elem *e;

        for (e = list_begin(bucket); e != list_end(bucket); e = list_next(e))
        {
            // 현재 요소를 구조체로 변환
            struct hash_elem *hash_e = list_entry(e, struct hash_elem, list_elem);
            struct my_struct *item = hash_entry(hash_e, struct my_struct, elem);

            // 데이터 출력
            printf("%d ", item->data);
            data_printed = true; // 데이터를 출력했으므로 플래그를 true로 설정
        }
    }

    if (data_printed)
    {
        printf("\n"); // 데이터가 한 번이라도 출력되었다면, 개행을 출력
    }
}

/*apply에 쓰이는 함수*/

/*hash apply에 쓰이는 함수*/
void square(struct hash_elem *e, void *aux)
{
    struct my_struct *entry = hash_entry(e, struct my_struct, elem);
    entry->data = entry->data * entry->data; // 데이터를 제곱
}

void triple(struct hash_elem *e, void *aux)
{
    struct my_struct *entry = hash_entry(e, struct my_struct, elem);
    int temp = entry->data;
    entry->data = temp * temp * temp; // 데이터를 세제곱
}
/*hash_delete에 쓰이는 함수*/
// 해시 테이블에서 data_value 값을 가진 요소를 찾아서 해당 hash_elem을 반환합니다.
// 찾지 못한 경우 NULL을 반환합니다.
struct hash_elem *find_hash_elem_by_value(struct hash *h, int data_value)
{
    // 해시 테이블의 모든 버킷을 순회
    for (size_t i = 0; i < h->bucket_cnt; ++i)
    {
        struct list *bucket = &h->buckets[i];
        // 각 버킷 내의 모든 요소를 순회
        for (struct list_elem *e = list_begin(bucket); e != list_end(bucket); e = list_next(e))
        {
            struct hash_elem *he = list_entry(e, struct hash_elem, list_elem);
            struct my_struct *item = hash_entry(he, struct my_struct, elem);
            // data_value와 일치하는 요소를 찾으면 반환
            if (item->data == data_value)
            {
                return he;
            }
        }
    }
    // 찾지 못한 경우
    return NULL;
}
/*insert*/
void execute_list_insert_command(struct list *list, int insert_position, int insert_value)
{
    struct list_elem *e = list_begin(list);
    struct my_data *new_data = malloc(sizeof(struct my_data));
    if (new_data == NULL)
    {
        // 메모리 할당 실패 처리
        return;
    }
    new_data->data = insert_value; // 새 데이터 설정

    for (int i = 0; e != list_end(list) && i < insert_position; e = list_next(e), i++)
        ;

    // 'e'는 이제 'insert_position'에 삽입하려는 위치를 가리킵니다.
    // 'new_data->elem'을 'e' 앞에 삽입합니다.
    list_insert(e, &new_data->elem);
}
/*list splice*/
struct list_elem *list_nth_elem(struct list *list, int n)
{
    struct list_elem *e = list_begin(list);
    for (int i = 0; i < n; i++)
    {
        if (e == list_end(list))
        {
            // n이 리스트의 범위를 벗어났을 때
            return NULL;
        }
        e = list_next(e);
    }
    return e; // n번째 요소의 포인터 반환
}

/*list_unique*/

int main()
{
    // initializeBitmapList();
    // initialize_all_lists();
    char line[1024];
    while (fgets(line, sizeof(line), stdin) != NULL)
    {
        line[strcspn(line, "\n")] = 0;

        char command[100];
        size_t size, start, cnt, bit_cnt, idx, block_size;
        void *a, *b, *block;
        void *aux = NULL;
        bool value;
        char type[10]; // 자료구조의 타입을 저장할 변수 (예: "bitmap")
        char structName[20];
        char value_str[6]; // 자료구조의 이름을 저장할 변수 (예: "bm0")
        struct hash_elem *e;
        struct hash *h;
        struct list *list;
        struct list_elem *before, *first, *last, *elem;
        struct list_elem *elem_a, *elem_b;

        int position, bit_index, hash_index, int_value, list_index, list_index2, data_value;

        int numArgs = sscanf(line, "%s", command);
        /*여기까지*/

        if (strcmp(command, "quit") == 0)
        {
            break;
        }

        else if (strcmp(command, "create") == 0 && sscanf(line, "%*s %s %s %zu", type, structName, &bit_cnt) == 3)
        {
            createBitmap(structName, bit_cnt);
        }

        else if (strcmp(command, "create") == 0 && sscanf(line, "%*s %s %s", type, structName) == 2)
        {
            if (strcmp(type, "list") == 0)
            {
                create_list(structName);
            }
            else if (strcmp(type, "hashtable") == 0)
            {
                create_hash(structName);
            }
        }
        else if (strcmp(command, "delete") == 0 && sscanf(line, "%*s bm%d", &bit_index) == 1)
        {
            if (bit_index >= 0 && bit_index < MAX_SIZE) //
            {
                if (bitmap_list[bit_index] != NULL)
                {
                    bitmap_destroy(bitmap_list[bit_index]);
                    bitmap_list[bit_index] = NULL; // 해제된 메모리를 다시 사용하지 않도록 방지
                }
                else
                {
                    printf("Error: No bitmap at index %d to delete.\n", bit_index);
                }
            }
            else
            {
                printf("Error: Bitmap index %d is out of bounds.\n", bit_index);
            }
        }
        else if (strcmp(command, "delete") == 0 && sscanf(line, "%*s list%d", &list_index) == 1)
        {
            // list_index의 유효성 검사
            if (list_index >= 0 && list_index < MAX_SIZE)
            {
                // 리스트가 초기화되었는지 확인
                if (list_list[list_index] != NULL)
                {
                    // 리스트가 비어 있지 않은지 추가로 확인할 수도 있습니다.
                    if (!list_empty(list_list[list_index]))
                    {
                        delete_list(list_list[list_index]);
                    }
                }
                else
                {
                    printf("List %d has not been initialized.\n", list_index);
                }
            }
            else
            {
                printf("Invalid list index: %d.\n", list_index);
            }
        }

        else if (strcmp(command, "dumpdata") == 0 && sscanf(line, "%*s bm%d", &bit_index) == 1)
        {
            dumpdata_bitmap_binary(bitmap_list[bit_index]);
        }
        else if (strcmp(command, "dumpdata") == 0 && sscanf(line, "%*s list%d", &list_index) == 1)
        {
            dumpdata_list(list_list[list_index]);
        }
        else if (strcmp(command, "dumpdata") == 0 && sscanf(line, "%*s hash%d", &hash_index) == 1)
        {
            dumpdata_hash(hash_tables[hash_index]);
        }
        else if (strcmp(command, "bitmap_dump") == 0 && sscanf(line, "%*s bm%d", &bit_index) == 1)
        {
            bitmap_dump(bitmap_list[bit_index]);
        }
        else if (strcmp(command, "dumpdata") == 0 && sscanf(line, "%*s list%d", structName) == 1)
        {
        }
        else if (strcmp(command, "dumpdata") == 0 && sscanf(line, "%*s bm%d", structName) == 1)
        {
        }
        else if (strcmp(command, "bitmap_destroy") == 0 && sscanf(line, "%*s %p", &b) == 1)
        {
            bitmap_destroy(b);
        }
        else if (strcmp(command, "bitmap_destroy") == 0 && sscanf(line, "%*s %p", &b) == 1)
        {
            bitmap_destroy(b);
        }
        else if (strcmp(command, "bitmap_set") == 0)
        {
            char value_str[6]; // "true" 또는 "false" 문자열을 저장할 배열
            if (sscanf(line, "%*s bm%d %zu %5s", &bit_index, &idx, value_str) == 3)
            {
                bool bool_value;
                // 문자열을 비교하여 bool 값 설정
                if (strcmp(value_str, "true") == 0)
                {
                    bool_value = true;
                }
                else if (strcmp(value_str, "false") == 0)
                {
                    bool_value = false;
                }
                else
                {
                    printf("Invalid value. Use 'true' or 'false'.\n");
                    return; // 값을 올바르게 변환하지 못한 경우 더 이상 진행하지 않음
                }

                // 변환된 bool_value를 사용하여 bitmap_set 함수 호출
                bitmap_set(bitmap_list[bit_index], idx, bool_value);
            }
            else
            {
                printf("Invalid command format. Correct format: bitmap_set <bm_index> <index> <true|false>\n");
            }
        }

        else if (strcmp(command, "bitmap_mark") == 0 && sscanf(line, "%*s bm%d %zu", &b, &idx) == 2)
        {

            bitmap_mark(bitmap_list[(int)b], idx);
        }
        else if (strcmp(command, "bitmap_reset") == 0 && sscanf(line, "%*s bm%d %zu", &bit_index, &idx) == 2)
        {
            bitmap_reset(bitmap_list[bit_index], idx);
        }
        else if (strcmp(command, "bitmap_flip") == 0 && sscanf(line, "%*s bm%d %zu", &bit_index, &idx) == 2)
        {
            bitmap_flip(bitmap_list[bit_index], idx);
        }
        else if (strcmp(command, "bitmap_test") == 0 && sscanf(line, "%*s bm%d %zu", &bit_index, &idx) == 2)
        {
            bool result = bitmap_test(bitmap_list[bit_index], idx);
            printf("%s\n", result ? "true" : "false");
        }

        else if (strcmp(command, "bitmap_set_all") == 0)
        {
            char value_str[6]; // "true" 또는 "false" 문자열을 저장할 배열
            if (sscanf(line, "%*s bm%d %5s", &bit_index, value_str) == 2)
            {
                bool bool_value;
                // 문자열을 비교하여 bool 값 설정
                if (strcmp(value_str, "true") == 0)
                {
                    bool_value = true;
                }
                else if (strcmp(value_str, "false") == 0)
                {
                    bool_value = false;
                }
                else
                {
                    printf("Invalid value. Use 'true' or 'false'.\n");
                    return; // 값을 올바르게 변환하지 못한 경우 더 이상 진행하지 않음
                }

                // 변환된 bool_value를 사용하여 bitmap_set_all 함수 호출
                bitmap_set_all(bitmap_list[bit_index], bool_value);
            }
            else
            {
                // 사용자에게 명령어를 올바르게 입력하도록 구체적인 지침 제공
                printf("Usage: bitmap_set_all <bitmap_index> <true|false>\n");
            }
        }

        else if (strcmp(command, "bitmap_set_multiple") == 0)
        {
            char value_str[6]; // "true" 또는 "false" 문자열을 저장할 배열
            if (sscanf(line, "%*s bm%d %zu %zu %5s", &bit_index, &start, &cnt, value_str) == 4)
            {
                bool bool_value;
                // 문자열을 비교하여 bool 값 설정
                if (strcmp(value_str, "true") == 0)
                {
                    bool_value = true;
                }
                else if (strcmp(value_str, "false") == 0)
                {
                    bool_value = false;
                }
                else
                {
                    printf("Invalid value. Use 'true' or 'false'.\n");
                    return; // 값을 올바르게 변환하지 못한 경우 더 이상 진행하지 않음
                }

                // 변환된 bool_value를 사용하여 bitmap_set_multiple 함수 호출
                bitmap_set_multiple(bitmap_list[bit_index], start, cnt, bool_value);
            }
        }

        else if (strcmp(command, "bitmap_count") == 0)
        {
            char value_str[6]; // "true" 또는 "false" 문자열을 저장할 배열
            if (sscanf(line, "%*s bm%d %zu %zu %5s", &bit_index, &start, &cnt, value_str) == 4)
            {
                bool value;
                // 문자열을 비교하여 bool 값 설정
                if (strcmp(value_str, "true") == 0)
                {
                    value = true;
                }
                else if (strcmp(value_str, "false") == 0)
                {
                    value = false;
                }
                else
                {
                    printf("Invalid value. Use 'true' or 'false'.\n");
                    return; // 값을 올바르게 변환하지 못한 경우 더 이상 진행하지 않음
                }

                size_t result = bitmap_count(bitmap_list[bit_index], start, cnt, value); // 비트맵 카운트 함수 호출
                printf("%zu\n", result);                                                 // 결과 출력
            }
        }

        else if (strcmp(command, "bitmap_contains") == 0)
        {
            // "true" 또는 "false" 문자열을 저장할 공간
            if (sscanf(line, "%*s bm%d %zu %zu %5s", &bit_index, &start, &cnt, value_str) == 4)
            {
                bool value = false; // 기본값을 false로 설정

                if (strcmp(value_str, "true") == 0)
                {
                    value = true;
                }
                else if (strcmp(value_str, "false") == 0)
                {
                    value = false;
                }
                else
                {
                    printf("Invalid value. Please enter 'true' or 'false'.\n");
                    return; // 또는 적절한 오류 처리
                }

                // bitmap_contains 함수의 반환 값은 bool 타입입니다. 따라서, 결과를 문자열로 출력합니다.
                bool result = bitmap_contains(bitmap_list[bit_index], start, cnt, value);
                printf("%s\n", result ? "true" : "false");
            }
            else
            {
                printf("Invalid command format.\n");
            }
        }

        else if (strcmp(command, "bitmap_any") == 0 && sscanf(line, "%*s bm%d %zu %zu", &bit_index, &start, &cnt) == 3)
        {
            bool result = bitmap_any(bitmap_list[bit_index], start, cnt);
            printf("%s\n", result ? "true" : "false");
        }
        else if (strcmp(command, "bitmap_none") == 0 && sscanf(line, "%*s bm%d %zu %zu", &bit_index, &start, &cnt) == 3)
        {
            bool result = bitmap_none(bitmap_list[bit_index], start, cnt);
            printf("%s\n", result ? "true" : "false");
        }

        else if (strcmp(command, "bitmap_all") == 0 && sscanf(line, "%*s bm%d %zu %zu", &bit_index, &start, &cnt) == 3)
        {
            bool result = bitmap_all(bitmap_list[bit_index], start, cnt);
            printf("%s\n", result ? "true" : "false");
        }
        else if (strcmp(command, "bitmap_scan") == 0)
        {
            char value_str[6]; // "true" 또는 "false" 문자열을 저장할 배열
            if (sscanf(line, "%*s bm%d %zu %zu %5s", &bit_index, &start, &cnt, value_str) == 4)
            {
                bool bool_value;
                // 문자열을 비교하여 bool 값 설정
                if (strcmp(value_str, "true") == 0)
                {
                    bool_value = true;
                }
                else if (strcmp(value_str, "false") == 0)
                {
                    bool_value = false;
                }
                else
                {
                    printf("Invalid value. Use 'true' or 'false'.\n");
                    return; // 값을 올바르게 변환하지 못한 경우 더 이상 진행하지 않음
                }

                // 변환된 bool_value를 사용하여 bitmap_scan 함수 호출
                size_t result = bitmap_scan(bitmap_list[bit_index], start, cnt, bool_value);

                if (result != BITMAP_ERROR)
                {
                    printf("%zu\n", result);
                }
                else
                {
                    printf("%zu\n", BITMAP_ERROR);
                }
            }
            else
            {
                printf("Invalid command format.\n");
            }
        }

        else if (strcmp(command, "bitmap_scan_and_flip") == 0)
        {
            char value_str[6]; // "true" 또는 "false" 문자열을 저장할 배열
            if (sscanf(line, "%*s bm%d %zu %zu %5s", &bit_index, &start, &cnt, value_str) == 4)
            {
                bool bool_value;
                // 문자열을 비교하여 bool 값 설정
                if (strcmp(value_str, "true") == 0)
                {
                    bool_value = true;
                }
                else if (strcmp(value_str, "false") == 0)
                {
                    bool_value = false;
                }
                else
                {
                    printf("Invalid value. Use 'true' or 'false'.\n");
                    return; // 값을 올바르게 변환하지 못한 경우 더 이상 진행하지 않음
                }

                // 변환된 bool_value를 사용하여 bitmap_scan_and_flip 함수 호출
                size_t result = bitmap_scan_and_flip(bitmap_list[bit_index], start, cnt, bool_value);

                if (result != BITMAP_ERROR)
                {
                    printf("%zu\n", result);
                }
                else
                {
                    printf("%zu\n", BITMAP_ERROR);
                }
            }
        }

        else if (strcmp(command, "bitmap_size") == 0 && sscanf(line, "%*s bm%d", &bit_index) == 1)
        {
            printf("%zu\n", bitmap_size(bitmap_list[bit_index]));
        }

        else if (strcmp(command, "bitmap_expand") == 0)
        {
            if (sscanf(line, "%*s bm%d %d", &bit_index, &size) == 2)
            {
                struct bitmap *expanded_bitmap = bitmap_expand(bitmap_list[bit_index], size + bitmap_size(bitmap_list[bit_index]));
                if (expanded_bitmap != NULL)
                {
                    // 확장 성공: 기존 비트맵을 새 비트맵으로 대체합니다.
                    bitmap_list[bit_index] = expanded_bitmap;
                }
                else
                {
                    printf("Failed to expand bitmap.\n");
                }
            }
            else
            {
                printf("Invalid command format.\n");
            }
        }

        else if (strcmp(command, "hash_insert") == 0 && sscanf(line, "%*s hash%d %d", &hash_index, &data_value) == 2)
        {
            // 새로운 my_struct 인스턴스를 생성하고 초기화
            struct my_struct *new_item = malloc(sizeof(struct my_struct));
            if (new_item == NULL)
            {
                // 메모리 할당 실패 처리
                printf("Failed to allocate memory for new hash item.\n");
                return;
            }
            new_item->data = data_value;

            // hash_insert 함수 호출, &new_item->elem을 전달
            struct hash_elem *prev = hash_insert(hash_tables[hash_index], &new_item->elem);
        }

        else if (strcmp(command, "hash_replace") == 0)
        {
            int data_value;
            if (sscanf(line, "%*s hash%d %d", &hash_index, &data_value) == 2)
            {
                // 새로운 데이터 값을 가지는 요소 생성
                struct my_struct *new_data = malloc(sizeof(struct my_struct));
                if (new_data == NULL)
                {
                    printf("Memory allocation failed.\n");
                    return;
                }
                new_data->data = data_value;                  // 사용자로부터 받은 데이터 값 설정
                struct hash_elem *new_elem = &new_data->elem; // 새로운 hash_elem 포인터 준비

                // hash_replace 함수 호출
                struct hash_elem *old_elem = hash_replace(hash_tables[hash_index], new_elem);
                if (old_elem != NULL)
                {
                    // old_elem이 NULL이 아니라면, 대체된 기존 요소가 존재함
                    struct my_struct *old_data = hash_entry(old_elem, struct my_struct, elem);
                    free(old_data); // 대체된 요소의 메모리 해제
                }
            }
            else
            {
                printf("Invalid command format.\n");
            }
        }

        else if (strcmp(command, "hash_find") == 0)
        {
            int data_value;
            if (sscanf(line, "%*s hash%d %d", &hash_index, &data_value) == 2)
            {
                // 찾고자 하는 값을 가진 임시 요소 생성
                struct my_struct temp;
                temp.data = data_value;
                struct hash_elem *temp_elem = &temp.elem;

                // 찾고자 하는 요소의 hash_elem 포인터를 얻기 위해
                struct hash_elem *found = hash_find(hash_tables[hash_index], temp_elem);

                if (found != NULL)
                {
                    // 요소를 찾은 경우, 해당 요소의 데이터 출력
                    struct my_struct *found_item = hash_entry(found, struct my_struct, elem);
                    printf("%d\n", found_item->data);
                }
            }
            else
            {
                printf("Invalid command format.\n");
            }
        }

        else if (strcmp(command, "hash_delete") == 0)
        {
            int data_value; // 사용자로부터 입력받은 데이터 값
            if (sscanf(line, "%*s hash%d %d", &hash_index, &data_value) == 2)
            {
                if (hash_index >= 0 && hash_index < MAX_SIZE && hash_tables[hash_index] != NULL)
                {
                    // 여기서는 data_value를 기반으로 hash_elem을 찾는 방식이 필요합니다.
                    // 예를 들어, data_value와 일치하는 요소를 찾는 사용자 정의 함수가 필요할 수 있습니다.
                    struct hash_elem *e = find_hash_elem_by_value(hash_tables[hash_index], data_value);
                    if (e != NULL)
                    {
                        hash_delete(hash_tables[hash_index], e);
                        // 삭제된 요소에 대한 후처리 (예: 메모리 해제)가 필요한 경우 여기서 수행
                    }
                }
                else
                {
                    printf("Invalid hash table index or uninitialized hash table.\n");
                }
            }
        }

        else if (strcmp(command, "hash_apply") == 0)
        {
            char action[10]; // 작업을 저장할 변수
            // 명령어에서 해시 테이블 인덱스와 수행할 작업(action)을 파싱
            if (sscanf(line, "%*s hash%d %s", &hash_index, action) == 2)
            {
                if (hash_index >= 0 && hash_index < MAX_SIZE && hash_tables[hash_index] != NULL)
                {
                    // "square" 작업을 수행하는 경우
                    if (strcmp(action, "square") == 0)
                    {
                        hash_apply(hash_tables[hash_index], square);
                    }
                    // "triple" 작업을 수행하는 경우
                    else if (strcmp(action, "triple") == 0)
                    {
                        hash_apply(hash_tables[hash_index], triple);
                    }
                    else
                    {
                        printf("Invalid action.\n");
                    }
                }
                else
                {
                    printf("Invalid hash table index or unitialized hash table.\n");
                }
            }
            else
            {
                printf("Invalid command format.\n");
            }
        }

        else if (strcmp(command, "hash_clear") == 0 && sscanf(line, "%*s hash%d", &hash_index) == 1)
        {
            hash_clear(hash_tables[hash_index], NULL);
        }
        else if (strcmp(command, "hash_size") == 0 && sscanf(line, "%*s hash%d", &hash_index) == 1)
        {
            printf("%d\n", hash_size(hash_tables[hash_index]));
        }
        else if (strcmp(command, "hash_empty") == 0 && sscanf(line, "%*s hash%d", &hash_index) == 1)
        {
            if (hash_index >= 0 && hash_index < MAX_SIZE && hash_tables[hash_index] != NULL)
            {
                // hash_empty 함수의 결과에 따라 "true" 또는 "false"를 출력
                bool isEmpty = hash_empty(hash_tables[hash_index]);
                printf("%s\n", isEmpty ? "true" : "false");
            }
            else
            {
                printf("Invalid hash table index or uninitialized hash table.\n");
            }
        }

        else if (strcmp(command, "list_push_back") == 0 && sscanf(line, "%*s list%d %d", &list_index, &data_value) == 2)
        {
            if (list_index >= 0 && list_index < MAX_SIZE && list_list[list_index] != NULL)
            {
                struct my_data *new_data = malloc(sizeof(struct my_data)); // 새 데이터 요소 생성
                if (new_data != NULL)
                {
                    new_data->data = data_value;                            // 데이터 값 설정
                    list_push_back(list_list[list_index], &new_data->elem); // 리스트에 추가
                }
                else
                {
                    printf("Memory allocation failed.\n");
                }
            }
            else
            {
                printf("Invalid list index or list not initialized.\n");
            }
        }

        // list_push_front 처리
        else if (strcmp(command, "list_push_front") == 0 && sscanf(line, "%*s list%d %d", &list_index, &data_value) == 2)
        {
            if (list_index >= 0 && list_index < MAX_SIZE && list_list[list_index] != NULL)
            {
                struct my_data *new_data = malloc(sizeof(struct my_data)); // 새 데이터 요소 생성
                if (new_data != NULL)
                {
                    new_data->data = data_value;                             // 데이터 값 설정
                    list_push_front(list_list[list_index], &new_data->elem); // 리스트의 앞부분에 추가
                }
                else
                {
                    printf("Memory allocation failed.\n");
                }
            }
            else
            {
                printf("Invalid list index or list not initialized.\n");
            }
        }

        // list_pop_back 처리
        else if (strcmp(command, "list_pop_back") == 0 && sscanf(line, "%*s list%d", &list_index) == 1)
        {
            if (list_index >= 0 && list_index < MAX_SIZE && list_list[list_index] != NULL && !list_empty(list_list[list_index]))
            {
                struct list_elem *e = list_pop_back(list_list[list_index]);
                struct my_data *data = list_entry(e, struct my_data, elem);

                free(data); // 요소가 동적으로 할당된 경우 메모리 해제
            }
        }

        // list_pop_front 처리 및 반환값 출력
        else if (strcmp(command, "list_pop_front") == 0 && sscanf(line, "%*s list%d", &list_index) == 1)
        {
            if (list_index >= 0 && list_index < MAX_SIZE && list_list[list_index] != NULL && !list_empty(list_list[list_index]))
            {
                struct list_elem *e = list_pop_front(list_list[list_index]);
                struct my_data *data = list_entry(e, struct my_data, elem);

                free(data); // 요소가 동적으로 할당된 경우 메모리 해제
            }
        }

        // list_front 처리 및 반환값 출력
        else if (strcmp(command, "list_front") == 0 && sscanf(line, "%*s list%d", &list_index) == 1)
        {
            if (list_index >= 0 && list_index < MAX_SIZE && list_list[list_index] != NULL && !list_empty(list_list[list_index]))
            {
                struct list_elem *e = list_front(list_list[list_index]);
                struct my_data *data = list_entry(e, struct my_data, elem);
                printf("%d\n", data->data);
            }
            else
            {
                printf("Invalid list index, list not initialized, or list is empty.\n");
            }
        }

        // list_back 처리 및 반환값 출력
        else if (strcmp(command, "list_back") == 0 && sscanf(line, "%*s list%d", &list_index) == 1)
        {
            if (list_index >= 0 && list_index < MAX_SIZE && list_list[list_index] != NULL && !list_empty(list_list[list_index]))
            {
                struct list_elem *e = list_back(list_list[list_index]);
                struct my_data *data = list_entry(e, struct my_data, elem);
                printf("%d\n", data->data);
            }
            else
            {
                printf("Invalid list index, list not initialized, or list is empty.\n");
            }
        }

        // list_insert 처리
        // create_list 명령어 처리 예시
        else if (strcmp(command, "create") == 0 && sscanf(line, "%*s %s %s", type, structName) == 2)
        {
            if (strcmp(type, "list") == 0)
            {
                int index = -1;
                if (sscanf(structName, "list%d", &index) == 1 && index >= 0 && index < MAX_SIZE)
                {
                    if (list_list[index] == NULL)
                    {
                        list_list[index] = malloc(sizeof(struct list));
                        list_init(list_list[index]);
                        printf("List %s created and initialized.\n", structName);
                    }
                    else
                    {
                        printf("List %s already exists.\n", structName);
                    }
                }
                else
                {
                    printf("Invalid list name or index out of range.\n");
                }
            }
        }

        // list_insert 명령어 처리 예시
        else if (strcmp(command, "list_insert") == 0)
        {
            int insert_position;
            if (sscanf(line, "list_insert list%d %d %d", &list_index, &insert_position, &data_value) == 3)
            {
                if (list_index >= 0 && list_index < MAX_SIZE && list_list[list_index] != NULL)
                {
                    struct my_data *new_data = malloc(sizeof(struct my_data));
                    if (new_data == NULL)
                    {
                        printf("Memory allocation failed.\n");
                        return;
                    }
                    new_data->data = data_value;

                    if (list_empty(list_list[list_index]) || insert_position == 0)
                    {
                        list_push_front(list_list[list_index], &new_data->elem);
                    }
                    else
                    {
                        // 요소 삽입 로직 (위의 execute_list_insert_command 함수 참조)
                        execute_list_insert_command(list_list[list_index], insert_position, data_value);
                    }
                }
                else
                {
                    printf("Invalid list index or list not initialized.\n");
                }
            }
            else
            {
                printf("Invalid command format.\n");
            }
        }
        else if (strcmp(command, "list_insert_ordered") == 0)
        {
            // 명령어에서 리스트 인덱스와 데이터 값을 파싱합니다.
            if (sscanf(line, "list_insert_ordered list%d %d", &list_index, &data_value) == 2)
            {
                // 유효한 리스트 인덱스인지 확인합니다.
                if (list_index >= 0 && list_index < MAX_SIZE)
                {
                    // 새로운 my_data 구조체 인스턴스를 동적으로 할당합니다.
                    struct my_data *new_data = malloc(sizeof(struct my_data));
                    if (new_data == NULL)
                    {
                        printf("Memory allocation failed\n");
                        return; // 메모리 할당 실패 시 함수 종료
                    }
                    // 할당된 구조체에 데이터 값을 설정합니다.
                    new_data->data = data_value;

                    // 요소를 리스트에 삽입하기 위해 list_elem 포인터를 가져옵니다.
                    struct list_elem *elem = &new_data->elem;

                    // 삽입된 요소를 정렬된 순서로 배치합니다.
                    list_insert_ordered(list_list[list_index], elem, less, aux);
                }
                else
                {
                    printf("Invalid list index.\n");
                }
            }
        }
        // list_max 처리
        else if (strcmp(command, "list_max") == 0 && sscanf(line, "%*s list%d", &list_index) == 1)
        {
            struct list_elem *elem = list_max(list_list[list_index], less, aux);
            if (elem != list_end(list_list[list_index]))
            {
                struct my_data *data = list_entry(elem, struct my_data, elem); // 변환
                printf("%d\n", data->data);                                    // 정확한 출력
            }
            else
            {
                printf("List is empty or no maximum value.\n");
            }
        }

        // list_min 처리
        else if (strcmp(command, "list_min") == 0 && sscanf(line, "%*s list%d", &list_index) == 1)
        {
            struct list_elem *elem = list_min(list_list[list_index], less, aux);
            if (elem != list_end(list_list[list_index]))
            {
                struct my_data *data = list_entry(elem, struct my_data, elem); // 변환
                printf("%d\n", data->data);                                    // 정확한 출력
            }
            else
            {
                printf("List is empty or no maximum value.\n");
            }
        }

        // list_size 처리
        else if (strcmp(command, "list_size") == 0 && sscanf(line, "%*s list%d", &list_index) == 1)
        {
            size_t size = list_size(list_list[list_index]);
            printf("%zu\n", size);
        }

        // list_empty 처리
        else if (strcmp(command, "list_empty") == 0 && sscanf(line, "%*s list%d", &list_index) == 1)
        {
            bool empty = list_empty(list_list[list_index]);
            printf("%s\n", empty ? "true" : "false");
        }
        else if (strcmp(command, "list_remove") == 0 && sscanf(line, "%*s list%d %d", &list_index, &position) == 2)
        {
            struct list *target_list = list_list[list_index];
            if (target_list == NULL)
            {
                printf("List %d does not exist.\n", list_index);
                return;
            }

            struct list_elem *e = list_begin(target_list);
            for (int i = 0; i < position && e != list_end(target_list); i++)
            {
                e = list_next(e);
            }

            if (e != list_end(target_list)) // 요소가 리스트 안에 있으면
            {
                struct my_data *data = list_entry(e, struct my_data, elem);
                list_remove(e);
                free(data); // 요소를 제거한 후 관련 리소스 해제
            }
            else
            {
                printf("Element at index %d does not exist in list %d.\n", position, list_index);
            }
        }

        // list_reverse 처리
        else if (strcmp(command, "list_reverse") == 0 && sscanf(line, "%*s list%d", &list_index) == 1)
        {
            list_reverse(list_list[list_index]);
        }

        // list_shuffle 처리
        else if (strcmp(command, "list_shuffle") == 0 && sscanf(line, "%*s list%d", &list_index) == 1)
        {
            list_shuffle(list_list[list_index]);
        }

        // list_sort 처리
        else if (strcmp(command, "list_sort") == 0 && sscanf(line, "%*s list%d", &list_index) == 1)
        {
            list_sort(list_list[list_index], less, aux); // less, aux는 추가적인 컨텍스트가 필요
        }
        else if (strcmp(command, "list_splice") == 0)
        {
            int target_list_index, before_pos, source_list_index, first_pos, last_pos_plus_one;
            if (sscanf(line, "list_splice list%d %d list%d %d %d", &target_list_index, &before_pos, &source_list_index, &first_pos, &last_pos_plus_one) == 5)
            {
                struct list *target_list = list_list[target_list_index];
                struct list *source_list = list_list[source_list_index];

                if (target_list == NULL || source_list == NULL)
                {
                    printf("One of the lists does not exist.\n");
                    return;
                }

                struct list_elem *before = list_nth_elem(target_list, before_pos);
                struct list_elem *first = list_nth_elem(source_list, first_pos);
                // last_pos_plus_one는 실제로 마지막으로 이동시키고 싶은 요소의 다음 요소를 가리키므로, 따로 조정할 필요 없음
                struct list_elem *last = list_nth_elem(source_list, last_pos_plus_one);

                if (before == NULL || first == NULL || last == NULL)
                {
                    printf("Invalid position.\n");
                    return;
                }

                list_splice(before, first, last);
            }
            else
            {
                printf("Invalid command format.\n");
            }
        }

        else if (strcmp(command, "list_swap") == 0)
        {
            int list_index, index_a, index_b;
            if (sscanf(line, "list_swap list%d %d %d", &list_index, &index_a, &index_b) == 3)
            {
                struct list *selected_list = list_list[list_index];
                if (!selected_list || list_empty(selected_list))
                {
                    printf("List is empty or does not exist.\n");
                    return;
                }

                struct list_elem *elem_a = NULL;
                struct list_elem *elem_b = NULL;
                int current_index = 0;

                // 리스트를 순회하며 해당 인덱스의 요소를 찾음
                for (struct list_elem *e = list_begin(selected_list); e != list_end(selected_list); e = list_next(e))
                {
                    if (current_index == index_a)
                    {
                        elem_a = e;
                    }
                    if (current_index == index_b)
                    {
                        elem_b = e;
                    }
                    current_index++;
                }

                // 두 요소가 모두 찾아졌는지 확인
                if (elem_a && elem_b)
                {
                    // 두 요소의 위치를 교환
                    list_swap(elem_a, elem_b);
                   
                }
                else
                {
                    printf("One or both positions are out of bounds.\n");
                }
            }
        }

        else if (strcmp(command, "list_unique") == 0)
        {
            if (sscanf(line, "%*s list%d list%d", &list_index, &list_index2) == 2)
            {
                if (list_index >= 0 && list_index < MAX_SIZE && list_list[list_index] != NULL &&
                    list_index2 >= 0 && list_index2 < MAX_SIZE && list_list[list_index2] != NULL)
                {
                    // 중복 요소를 제거하는 list_unique 함수를 호출합니다.
                    // 여기서 less는 리스트의 요소를 비교하는 함수이며, NULL은 추가적인 사용자 데이터(aux)입니다.
                    // 필요에 따라 사용자 정의 데이터로 aux를 대체할 수 있습니다.
                    list_unique(list_list[list_index], list_list[list_index2], less, aux);
                }
            }
            else if (sscanf(line, "%*s list%d", &list_index) == 1) // 여기에 있던 닫는 괄호를 제거했습니다.
            {
                list_unique(list_list[list_index], NULL, less, aux);
            }
        }
    }
    return EXIT_SUCCESS;
}
