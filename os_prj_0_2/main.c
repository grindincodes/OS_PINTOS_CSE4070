#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitmap.h"
#include "debug.h"
#include "hash.h"
#include "hex_dump.h"
#include "limits.h"
#include "list.h"
#include "round.h"

int parse_command(char * cmdline, char ** cmdptr);
void execute_command(char * cmdline, char ** cmdptr, int cmdcnt);
int atoi_num(int select, char* buf); // list, hash -> select : 0, bitmap -> select: 1

struct list list[10];
struct hash hash[10];
struct bitmap *bm[10];

int main() {
	// stdin에서 한 줄의 명령어를 받고, 이를 처리함.
	char cmdline[100];
	char* cmdptr[10];
	int cmdcnt = 0;
	while (1) {
		if (!fgets(cmdline, 100, stdin)) exit(0);
		cmdcnt = parse_command(cmdline,cmdptr); // command parsing
		execute_command(cmdline, cmdptr, cmdcnt);
	}	
	return 0;
}
int parse_command(char* cmdline, char** cmdptr) {
	int cmdcnt = 1;		// default command count	
	cmdptr[0] = cmdline;	// first command
	char* buf;
        // erase newline character	
	if (buf = strchr(cmdline, '\n')){
		*buf = '\0';
	}
	// default command count = 1
	buf = cmdline;
	// find ' '
	while (buf = strchr(buf, ' ')){
		// if there is ' ', replace it and step forward by 1
		*buf = '\0';
		buf++;
		cmdptr[cmdcnt] = buf;
		cmdcnt++;
	}
	return cmdcnt;
}
void execute_command(char * cmdline, char ** cmdptr, int cmdcnt){
	int num;
	int num2;
	list_less_func *less_func_ptr = less_func;
	hash_hash_func *hash_ptr = hash_hash;
	hash_less_func *hash_less_ptr = hash_less;
	if (!strcmp(cmdptr[0], "quit")) {
		exit(0);
	}
	if (!strcmp(cmdptr[0],"create")) {
		if (!strcmp(cmdptr[1], "list")) {
			// create list
			num = atoi_num(0, cmdptr[2]); //e.g. list0
			// initialize list{num}
			list_init(&list[num]);
			return;
		}
		if (!strcmp(cmdptr[1], "hashtable")) {
		    num = atoi_num(0, cmdptr[2]);
		    // create hash table
		    hash_init(&hash[num], hash_ptr, hash_less_ptr, NULL);
		    return;
		}
		if (!strcmp(cmdptr[1], "bitmap")) {
		    num = atoi_num(1, cmdptr[2]);
		    size_t bit_cnt = atoi(cmdptr[3]);
		    bm[num] = bitmap_create(bit_cnt);
		    if (!bm[num]) exit(1);
		    return;
		}
        }
	if (!strcmp(cmdptr[0], "dumpdata")) {
	    if (strstr(cmdptr[1], "list")) {
		num = atoi_num(0, cmdptr[1]); // e.g. list0
	        list_dumpdata(&list[num]);
	    }
	    else if (strstr(cmdptr[1], "hash")) {
		num = atoi_num(0, cmdptr[1]); // e.g. hash0
	        hash_dumpdata(&hash[num]);
	    }
	    else if (strstr(cmdptr[1], "bm")) {
	        num = atoi_num(1, cmdptr[1]); // e.g. bm0
	        bitmap_dumpdata(bm[num]);
	    }
	    return;
	}
	if (!strcmp(cmdptr[0], "delete")) {
	    if (strstr(cmdptr[1], "list")) {
	        num = atoi_num(0, cmdptr[1]); // e.g. list0
		list_delete(&list[num]);
	    }
	    else if (strstr(cmdptr[1], "hash")) {
	        num = atoi_num(0, cmdptr[1]); // e.g. hash0
		hash_clear(&hash[num], hash_action_free);
	    }
	    else if (strstr(cmdptr[1], "bm")) {
	        num = atoi_num(1, cmdptr[1]); // e.g. bm0
		bitmap_destroy(bm[num]);
	    }
            return;
	}
	/* Bitmap command begin */
	if (!strcmp(cmdptr[0], "bitmap_mark")) {
	    num = atoi_num(1, cmdptr[1]);
	    int idx = atoi(cmdptr[2]);
	    bitmap_mark(bm[num], idx);
	    return;
	}
	if (!strcmp(cmdptr[0], "bitmap_test")) {
            num = atoi_num(1, cmdptr[1]);
            int idx = atoi(cmdptr[2]);
            bool b = bitmap_test(bm[num], idx);
	    printf("%s\n", b? "true" : "false");
            return;
	}
	if (!strcmp(cmdptr[0], "bitmap_size")) {
	    num = atoi_num(1, cmdptr[1]);
	    printf("%zu\n", bitmap_size(bm[num]));
	    return;
	}
	if (!strcmp(cmdptr[0], "bitmap_set")) {
            num = atoi_num(1, cmdptr[1]);
            int idx = atoi(cmdptr[2]);
	    if (!strcmp(cmdptr[3],"true")) bitmap_set(bm[num], idx, true);
	    else bitmap_set(bm[num], idx, false);
            return;
        }
	if (!strcmp(cmdptr[0], "bitmap_set_multiple")) {
            num = atoi_num(1, cmdptr[1]);
            int idx_start = atoi(cmdptr[2]);
	    int cnt = atoi(cmdptr[3]);
            if (!strcmp(cmdptr[4],"true")) bitmap_set_multiple(bm[num], idx_start, cnt, true);
            else bitmap_set_multiple(bm[num], idx_start, cnt, false);
            return;
        }
	if (!strcmp(cmdptr[0], "bitmap_set_all")) {
	    num = atoi_num(1, cmdptr[1]);
	    if (!strcmp(cmdptr[2],"true")) bitmap_set_all(bm[num], true);
	    else bitmap_set_all(bm[num], false);
	    return;
	}
	if (!strcmp(cmdptr[0], "bitmap_scan")) {
	    // scan first group of cnt consecutive value starting from start index
	    // return start index of the group
	    num = atoi_num(1, cmdptr[1]);
	    int idx_start = atoi(cmdptr[2]);
            int cnt = atoi(cmdptr[3]);
	    size_t idx_result;
	    if (!strcmp(cmdptr[4],"true")) idx_result = bitmap_scan(bm[num], idx_start, cnt, true);
	    else idx_result = bitmap_scan(bm[num], idx_start, cnt, false);
	    printf("%zu\n", idx_result);
	    return;
	}
	if (!strcmp(cmdptr[0], "bitmap_scan_and_flip")) {
	    num = atoi_num(1, cmdptr[1]);
	    int idx_start = atoi(cmdptr[2]);
            int cnt = atoi(cmdptr[3]);
	    size_t idx_result;
	    if (!strcmp(cmdptr[4],"true")) idx_result = bitmap_scan_and_flip(bm[num], idx_start, cnt, true);
            else idx_result = bitmap_scan_and_flip(bm[num], idx_start, cnt, false);
	    printf("%zu\n", idx_result);
            return;
	}
	if (!strcmp(cmdptr[0], "bitmap_reset")) {
            num = atoi_num(1, cmdptr[1]);
            int idx = atoi(cmdptr[2]);
            bitmap_reset(bm[num], idx);
            return;
        }
	if (!strcmp(cmdptr[0], "bitmap_none")) {
            num = atoi_num(1, cmdptr[1]);
            int idx_start = atoi(cmdptr[2]);
	    int cnt = atoi(cmdptr[3]);
            printf("%s\n", bitmap_none(bm[num], idx_start, cnt) ? "true" : "false");
            return;
        }
	if (!strcmp(cmdptr[0], "bitmap_flip")) {
            num = atoi_num(1, cmdptr[1]);
            int idx = atoi(cmdptr[2]);
            bitmap_flip(bm[num], idx);
            return;
        }
	if (!strcmp(cmdptr[0], "bitmap_expand")) {
            num = atoi_num(1, cmdptr[1]);
            int add_size = atoi(cmdptr[2]);
            bitmap_expand(bm[num], bitmap_size(bm[num]) + add_size);
            return;
        }
	if (!strcmp(cmdptr[0], "bitmap_dump")) {
	    num = atoi_num(1, cmdptr[1]);
	    bitmap_dump(bm[num]);
	}
	if (!strcmp(cmdptr[0], "bitmap_count")) {
            num = atoi_num(1, cmdptr[1]);
            int idx_start = atoi(cmdptr[2]);
            int cnt = atoi(cmdptr[3]);
            size_t result;
            if (!strcmp(cmdptr[4],"true")) result = bitmap_count(bm[num], idx_start, cnt, true);
            else result = bitmap_count(bm[num], idx_start, cnt, false);
            printf("%zu\n", result);
            return;
        }
	if (!strcmp(cmdptr[0], "bitmap_contains")) {
            num = atoi_num(1, cmdptr[1]);
            int idx_start = atoi(cmdptr[2]);
            int cnt = atoi(cmdptr[3]);
            bool result;
            if (!strcmp(cmdptr[4],"true")) result = bitmap_contains(bm[num], idx_start, cnt, true);
            else result = bitmap_contains(bm[num], idx_start, cnt, false);
            printf("%s\n", result ? "true" : "false");
            return;
        }
	if (!strcmp(cmdptr[0], "bitmap_any")) {
            num = atoi_num(1, cmdptr[1]);
            int idx_start = atoi(cmdptr[2]);
            int cnt = atoi(cmdptr[3]);
            bool result;
            result = bitmap_any(bm[num], idx_start, cnt);
            printf("%s\n", result ? "true" : "false");
            return;
        }
	if (!strcmp(cmdptr[0], "bitmap_all")) {
            num = atoi_num(1, cmdptr[1]);
            int idx_start = atoi(cmdptr[2]);
            int cnt = atoi(cmdptr[3]);
            bool result;
            result = bitmap_all(bm[num], idx_start, cnt);
            printf("%s\n", result ? "true" : "false");
            return;
        }
	/* Bitmap end */
	/* Hash command begin */
	if (!strcmp(cmdptr[0], "hash_replace")) {
	    num = atoi_num(0, cmdptr[1]);
	    int value = atoi(cmdptr[2]);
	    struct hash_item *item = (struct hash_item*)malloc(sizeof(struct hash_item));
	    item -> data = value;
	    hash_replace(&hash[num], &(item->elem));
	    return;
	}
	if (!strcmp(cmdptr[0], "hash_insert")) {
	    num = atoi_num(0, cmdptr[1]);
	    int value = atoi(cmdptr[2]);
	    struct hash_item *item = (struct hash_item*)malloc(sizeof(struct hash_item));
            item -> data = value;
	    hash_insert(&hash[num], &(item->elem));
	    return;
	}
	if (!strcmp(cmdptr[0], "hash_find")) {
	    // find element and print elemnet only if found
	    num = atoi_num(0, cmdptr[1]);
	    int value = atoi(cmdptr[2]);
	    struct hash_item i;
	    i.data = value;
	    if(hash_find(&hash[num], &(i.elem)))
	        printf("%d\n", value);
	    return;
	}
	if (!strcmp(cmdptr[0], "hash_delete")) {
	    num = atoi_num(0, cmdptr[1]);
	    int value = atoi(cmdptr[2]);
	    struct hash_item i;
            i.data = value;
	    hash_delete(&hash[num], &(i.elem));
	}
        if (!strcmp(cmdptr[0], "hash_apply")) {
	    num = atoi_num(0, cmdptr[1]);
	    hash_apply_f(&hash[num],cmdptr[2]);
	}
	if (!strcmp(cmdptr[0], "hash_empty")) {
            num = atoi_num(0, cmdptr[1]);
	    printf("%s\n", hash_empty(&hash[num])? "true":"false");
	    return;
	}
	if (!strcmp(cmdptr[0], "hash_size")) {
	    num = atoi_num(0, cmdptr[1]);
	    printf("%zu\n", hash_size(&hash[num]));
	    return;
	}
	if (!strcmp(cmdptr[0], "hash_clear")) {
            num = atoi_num(0, cmdptr[1]);
            hash_clear(&hash[num], NULL);
	    return;
	}
	/* Hash end */
	/* List command begin */
	if (!strcmp(cmdptr[0], "list_push_back")) {
	    num = atoi_num(0, cmdptr[1]); // e.g. list0
	    struct list_item *temp = (struct list_item *)malloc(sizeof(struct list_item));
	    temp -> data = atoi(cmdptr[2]); // e.g. 3
	    list_push_back(&list[num], &(temp -> elem));
	    return;
	}
	if (!strcmp(cmdptr[0], "list_unique")) {
            num = atoi_num(0, cmdptr[1]); // e.g. list0
	    if (cmdcnt > 2) {
	    	// list_unique with another list for duplicates.
		num2 = atoi_num(0, cmdptr[2]); // e.g. list1
		list_unique(&list[num], &list[num2], less_func_ptr, NULL);
	    }
	    else {
	    	list_unique(&list[num], NULL, less_func_ptr, NULL);
	    }
	    return;
        }
	if (!strcmp(cmdptr[0], "list_swap")) {
	    num = atoi_num(0, cmdptr[1]); // e.g. list0
	    
	    // find struct list_elem *A, and *B	by traversal
	    int index_a = atoi(cmdptr[2]);
	    int index_b = atoi(cmdptr[3]);
	    if (index_a > index_b) {
	    	int temp = index_a;
		index_a = index_b;
		index_b = temp;
	    }
	    int count = 0;
	    if (index_b >= list_size(&list[num])) exit(1);
	    struct list_elem *A, *B, *t;
	    for (t = list_begin(&list[num]); t != list_end(&list[num]); t = list_next(t)) {
	        if (count == index_a) A = t;
		if (count == index_b) {B = t; break;}
		count++;
	    }
	    list_swap(A,B);
	    return;
	}
        if (!strcmp(cmdptr[0], "list_splice")) {
	    // e.g. list_splice list0 2 list1 1 4
	    num = atoi_num(0, cmdptr[1]);
	    int index = atoi(cmdptr[2]);
	    struct list_elem *e = list_access(&list[num], index);
	    num2 = atoi_num(0, cmdptr[3]);
	    index = atoi(cmdptr[4]);
	    struct list_elem *A = list_access(&list[num2], index);
	    index = atoi(cmdptr[5]);
            struct list_elem *B = list_access(&list[num2], index);
	    list_splice(e, A, B);
	    return;
	}
	if (!strcmp(cmdptr[0], "list_sort")) {
	    num = atoi_num(0, cmdptr[1]);
	    list_sort(&list[num], less_func_ptr, NULL);
	    return;
	}
	if (!strcmp(cmdptr[0], "list_push_front")) {
	    num = atoi_num(0, cmdptr[1]); // e.g. list0
            struct list_item *temp = (struct list_item *)malloc(sizeof(struct list_item));
            temp -> data = atoi(cmdptr[2]); // e.g. 3
            list_push_front(&list[num], &(temp -> elem));
	}
	if (!strcmp(cmdptr[0], "list_shuffle")) {
	    num = atoi_num(0, cmdptr[1]);
	    list_shuffle(&list[num]);
	}
	if (!strcmp(cmdptr[0], "list_reverse")) {
            num = atoi_num(0, cmdptr[1]);
            list_reverse(&list[num]);
        }
	if (!strcmp(cmdptr[0], "list_remove")) {
	    num = atoi_num(0, cmdptr[1]);
	    int index = atoi(cmdptr[2]);
	    struct list_elem *e = list_access(&list[num], index);
	    list_remove(e);
	}
        if (!strcmp(cmdptr[0], "list_pop_back")) {
            num = atoi_num(0, cmdptr[1]);
            list_pop_back(&list[num]);
        }
	if (!strcmp(cmdptr[0], "list_pop_front")) {
            num = atoi_num(0, cmdptr[1]);
            list_pop_front(&list[num]);
        }
	if (!strcmp(cmdptr[0], "list_empty")) {
	    num = atoi_num(0, cmdptr[1]);
	    printf("%s\n", list_empty(&list[num])? "true": "false");
	}
	if (!strcmp(cmdptr[0], "list_size")) {
	    num = atoi_num(0, cmdptr[1]);
	    printf("%zu\n", list_size(&list[num]));
	}
	if (!strcmp(cmdptr[0], "list_max")) {
	    num = atoi_num(0, cmdptr[1]);
	    struct list_item *i = list_entry(list_max(&list[num], less_func_ptr, NULL), struct list_item, elem);
	    int max = i -> data;
	    printf("%d\n", max);
	}
	if (!strcmp(cmdptr[0], "list_min")) {
            num = atoi_num(0, cmdptr[1]);
            struct list_item *i = list_entry(list_min(&list[num], less_func_ptr, NULL), struct list_item, elem);
            int min = i -> data;
            printf("%d\n", min);
        }
	if (!strcmp(cmdptr[0], "list_insert")) {
	    num = atoi_num(0, cmdptr[1]);
	    int before_index = atoi(cmdptr[2]);
	    int value = atoi(cmdptr[3]);
	    struct list_item *temp = (struct list_item *)malloc(sizeof(struct list_item));
            temp -> data = value; // e.g. 3
            list_insert(list_access(&list[num], before_index), &(temp -> elem));
	}
	if (!strcmp(cmdptr[0], "list_insert_ordered")) {
	    num = atoi_num(0, cmdptr[1]);
	    int value = atoi(cmdptr[2]);
	    struct list_item *temp = (struct list_item *)malloc(sizeof(struct list_item));
            temp -> data = value;
	    list_insert_ordered(&list[num], &(temp->elem), less_func_ptr, NULL);
	}
	if (!strcmp(cmdptr[0], "list_front")) {
            num = atoi_num(0, cmdptr[1]);
            struct list_item *i = list_entry(list_front(&list[num]), struct list_item, elem);
            int front = i -> data;
            printf("%d\n", front);
        }
	if (!strcmp(cmdptr[0], "list_back")) {
            num = atoi_num(0, cmdptr[1]);
            struct list_item *i = list_entry(list_back(&list[num]), struct list_item, elem);
            int back = i -> data;
            printf("%d\n", back);
        }
	/* List command end */
}
// find number in string and use atoi to get an integer value
int atoi_num(int select, char* buf) {
    int num;
    if(select == 0) {
    	buf +=4;
	num = atoi(buf);
    }
    else {
    	buf +=2;
	num = atoi(buf);
    }
    return num;
}
