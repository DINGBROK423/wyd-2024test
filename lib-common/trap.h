#ifndef __TRAP_H__
#define __TRAP_H__

#define concat_temp(x, y) x ## y
#define concat(x, y) concat_temp(x, y)

#ifndef __ASSEMBLER__

#define HIT_GOOD_TRAP \
	asm volatile(".byte 0xd6" : : "a" (0))

#define HIT_BAD_TRAP \
	asm volatile(".byte 0xd6" : : "a" (1))

#define nemu_assert(cond) \
	do { \
		if( !(cond) ) HIT_BAD_TRAP; \
	} while(0)

static __attribute__((always_inline)) inline void
set_bp(void) {
	asm volatile ("int3");
} //设置断点
// 你只要在用户程序的源代码中
// 调 用 set_bp() 函 数 , 就 可 以 达 到 设 置 断 点 的 效 果 了 。 你可以在
// testcase/src/mov-c.c 中插入断点, 然后重新编译 mov-c 程序并运行 NEMU 来
// 体会这种断点的设置方法。需要注意的是, 这种简化的做法其实是对 int3 指令的
// 滥用, 因为在真实的操作系统中, 一般的程序不应该使用 int3 指令, 否则它将会
// 在运行时异常终止。
#else

#define HIT_GOOD_TRAP \
	movl $0, %eax; \
	.byte 0xd6

#define HIT_BAD_TRAP \
	movl $1, %eax; \
	.byte 0xd6

#define nemu_assert(reg, val) \
	cmp $val, %reg; \
	je concat(label,__LINE__); HIT_BAD_TRAP; concat(label,__LINE__):

#endif

#endif
