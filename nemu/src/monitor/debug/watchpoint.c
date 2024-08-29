#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "cpu/reg.h"
#define NR_WP 32
//extern struct sg_header;
//int result;
static WP wp_pool[NR_WP];
static WP *head, *free_;
//extern  CPU_state cpu; //调用对象
//初始化链表所有节点（全打成NULL）
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
//实现监视点池的管理
//该函数用于从代码框架中的free_链表返回一个闲置的监视点结构。
//有两种情况，一种是head链表为空时，直接head = temp，否则的话要设置一个变量用来查找head最后一个节点，利用尾插法把闲置的节点插上。
WP* new_wp(){
	WP *temp;
	temp = free_;
	free_ = free_->next;
	temp->next = NULL;
	if (head == NULL){
		head = temp;
	} else {
		WP* temp2;
		temp2 = head;
		while (temp2->next != NULL){
			temp2 = temp2->next;
		}
		temp2->next = temp;
	}
	return temp;
}
//编写一个free_wp()，将wp归还至free_链表当中。
//有三种情况，第一种如果当前返回的节点为空，直接assert(0)，第二种的情况head就是wp，否则的话要在head中找到与之相对应的wp，之后用头插法把wp插到free，最后把wp的属性清空。
void free_wp(WP *wp){
	if (wp == NULL){
		assert(0);
	}
	if (wp == head){ 
		head = head->next;
	} else {
		WP* temp = head;
		while (temp != NULL && temp->next != wp){
			temp = temp->next;
		}
		temp->next = temp->next->next;
	}
	wp->next =free_;
	free_ = wp;
	wp->result = 0;
	wp->expr[0] = '\0';
}
//实现类似GDB的监视点功能。
//编写一个checkWP()函数，该函数用于判断监视点是否触发。
//首先进行表达式求值，每当NEMU执行完一条指令，则若触发了用户所设的监视点，程序便会暂停下来，否则打印监视点、旧值和新值。
bool checkWP(){
	bool check = false;
	bool *success = false;
	WP *temp = head;
	int expr_temp;
	while(temp != NULL){
		expr_temp = expr(temp->expr, success);
		if (expr_temp != temp->result){
			check = true;
			printf ("Hint watchpoint %d at address 0x%08x\n", temp->NO, cpu.eip);
			temp = temp->next;
			continue;
		}
		printf ("Watchpoint %d: %s\n",temp->NO,temp->expr);
		printf ("Old value = %d\n",temp->result);
		printf ("New value = %d\n",expr_temp);
		temp->result = expr_temp;
		temp = temp->next;
	}
	return check;
}
//输出
void printf_wp(){
	WP *temp = head;
	if (temp == NULL){
		printf("No watchpoints\n");
	}
	while (temp != NULL){
		printf("Watch point %d: %s\n", temp->NO, temp->expr);
		temp = temp->next;
	}
}
//删除
WP* delete_wp(int p, bool *key){
	WP *temp = head;
	while (temp != NULL && temp->NO != p){
		temp = temp->next;
	}
	if (temp == NULL){
		*key = false;
	}
	return temp;
}





