#ifndef __HELPER_H__
#define __HELPER_H__

#include "nemu.h"
#include "cpu/decode/operand.h"
#include "cpu/eflags.h" 

/* All function defined with 'make_helper' return the length of the operation. */
#define make_helper(name) int name(swaddr_t eip)  //swadd_t 地址 eip 程序计数器

static inline uint32_t instr_fetch(swaddr_t addr, size_t len) {
	return swaddr_read(addr, len);//读内存
}
// 在 NEMU 中, exec()函数首先通过 instr_fetch()取出指令的第一
// 个字节, 然后根据取到的这个字节查看opcode_table, 得到指令的helper函数, 
// 从而调用这个 helper 函数来继续模拟这条指令的执行。

/* Instruction Decode and EXecute */
static inline int idex(swaddr_t eip, int (*decode)(swaddr_t), void (*execute) (void)) {  // 后两个函数指针表示译码和执行
	/* eip is pointing to the opcode */
	int len = decode(eip + 1);
	execute();
	return len + 1;	// "1" for opcode   opcode占一个字节
}

/* shared by all helper function */
extern Operands ops_decoded;  //引入（在decode.c文件里）

#define op_src (&ops_decoded.src)
#define op_src2 (&ops_decoded.src2)
#define op_dest (&ops_decoded.dest)


#endif
