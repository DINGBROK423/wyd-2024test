#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"
// #include "watchpoint.c"
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
extern void free_wp(WP *wp);
extern bool checkWP();
extern void printf_wp();
extern WP* delete_wp(int p, bool *key);
void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);
static int cmd_si(char *args){
	char *sencondWord = strtok(NULL," ");
	int step = 0;
	int i;
	if (sencondWord == NULL){
		cpu_exec(1);
		return 0;	
	}
	sscanf(sencondWord, "%d", &step);
	if (step <= 0){
		printf("MISINIPUT\n");
		return 0;
	}
	for (i = 0; i < step; i++){
		cpu_exec(1);
	}
	return 0;
}
static int cmd_info(char *args){
	char *sencondWord = strtok(NULL," ");
	int i;
	if (strcmp(sencondWord, "r") == 0){
		for (i = 0; i < 8; i++){
			printf("%s\t\t", regsl[i]);
			printf("0x%08x\t\t%d\n", cpu.gpr[i]._32, cpu.gpr[i]._32);
		}
		printf("eip\t\t0x%08x\t\t%d\n", cpu.eip, cpu.eip);
	return 0;
	}
	//如果分隔后的第一个字符是w就打印监视点的功能。这里貌似不能定义另一个函数来打印监视点，和之前的会有冲突，所以直接在cmd_info添加判断。
	if (strcmp(sencondWord, "w") == 0){
		printf_wp();
		return 0;
	}
	printf("MISINPUT\n");
	return 0;
}
static int cmd_x(char *args){
	char *sencondWord = strtok(NULL," ");
	char *thirdWord = strtok(NULL, " ");
	
	int step = 0;
	swaddr_t address;
	
	sscanf(sencondWord, "%d", &step);
	sscanf(thirdWord, "%x", &address);

	int i, j = 0;
	for (i = 0; i < step; i++){
		if (j % 4 == 0){
			printf("0x%x:", address);
		}
		printf("0x%08x ", swaddr_read(address, 4));
		address += 4;
		j++;
		if (j % 4 == 0){
			printf("\n");
		}
			}
	printf("\n");
	return 0;
}
static int cmd_p(char *args){
	bool *success = false;
	int i;
	i = expr(args, success);
	if (!success){
		printf("%d\n", i);
	}
	return 0;
}
static int cmd_w(char *args){
	char *sencondWord = strtok(NULL," ");
	//如果分隔后的第一个字符是w就打印监视点的功能。这里貌似不能定义另一个函数来打印监视点，和之前的会有冲突，所以直接在cmd_info添加判断。
	if (strcmp(sencondWord, "w") == 0){
		printf_wp();
		return 0;
	}
	printf("MISINPUT\n");
	return 0;
}
//添加删除指令。
static int cmd_d(char *args){
	int p;
	bool key = true;
	sscanf(args, "%d", &p);
	WP* q = delete_wp(p, &key);
	if (key){
		printf("Delete watchpoint %d: %s\n", q->NO, q->expr);
		free_wp(q);
		return 0;
	} else {
		printf("No found watchpoint %d\n", p);
		return 0;
	}
	return 0;
}

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si", "One step", cmd_si },
	{ "info", "Display all informations of regisiters", cmd_info },
	{ "x", "Scan Memory", cmd_x },
	{ "p", "Evaluation of expression", cmd_p},
	{ "w", "Set breakpoint", cmd_w},
	{ "d", "Delete breakpoint", cmd_d}
	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
