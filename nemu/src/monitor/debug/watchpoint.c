#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include <stdlib.h>

#define NR_WP 32 //定义了观察点的最大数量为 32
// wp_pool：一个 WP 结构体数组，用来存储所有的观察点。
// head：指向当前链表的头部，即当前所有活动的观察点。
// free_：指向空闲观察点的链表头部。
static WP wp_pool[NR_WP];
static WP *head, *free_;

//初始化观察点池
// init_wp_pool：初始化观察点池。将每个 wp_pool 中的观察点按顺序链接起来，形成一个空闲链表。head 初始化为 NULL，free_ 指向第一个空闲的观察点。
void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;
	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
//new_WP：从空闲链表中分配一个新的观察点。如果 free_ 不为空，则返回一个空闲的观察点，并更新 free_ 指针。
static WP* new_WP() {
	assert(free_ != NULL);
	WP *p = free_;
	free_ = free_->next;
	return p;
}
//free_WP：将观察点 p 释放回空闲链表，并且释放观察点中保存的表达式内存。
static void free_WP(WP *p) {
	assert(p >= wp_pool && p < wp_pool + NR_WP);
	free(p->expr);
	p->next = free_;
	free_ = p;
}
//set_watchpoint：设置一个新的观察点。解析表达式 e，如果成功，则创建一个新的观察点，并将其插入到观察点链表的头部。返回观察点的编号。
int set_watchpoint(char *e) {
	uint32_t val;
	bool success;
	val = expr(e, &success);
	if(!success) return -1;

	WP *p = new_WP();
	p->expr = strdup(e);
	p->old_val = val;

	p->next = head;
	head = p;

	return p->NO;
}
//delete_watchpoint：删除指定编号的观察点。如果找到该观察点，则将其从链表中移除，并释放其内存。
bool delete_watchpoint(int NO) {
	WP *p, *prev = NULL;
	for(p = head; p != NULL; prev = p, p = p->next) {
		if(p->NO == NO) { break; }
	}

	if(p == NULL) { return false; }
	if(prev == NULL) { head = p->next; }
	else { prev->next = p->next; }

	free_WP(p);
	return true;
}
//list_watchpoint：列出所有当前设置的观察点。如果没有观察点，则输出“没有观察点”。否则，打印每个观察点的编号、表达式和当前值。
void list_watchpoint() {
	if(head == NULL) {
		printf("No watchpoints\n");
		return;
	}

	printf("%8s\t%8s\t%8s\n", "NO", "Address", "Enable");
	WP *p;
	for(p = head; p != NULL; p = p->next) {
		printf("%8d\t%s\t%#08x\n", p->NO, p->expr, p->old_val);
	}
}
//scan_watchpoint：扫描所有观察点，检查其值是否发生变化。如果发现某个观察点的值发生了变化，则返回该观察点；否则，返回 NULL。
WP* scan_watchpoint() {
	WP *p;
	for(p = head; p != NULL; p = p->next) {
		bool success;
		p->new_val = expr(p->expr, &success);
		if(p->old_val != p->new_val) {
			return p;
		}
	}

	return NULL;
}

