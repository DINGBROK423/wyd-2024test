#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;

	/* TODO: Add more members if necessary */
	//增加结构体的成员
	char expr[100];
	int result;

} WP;

#endif
