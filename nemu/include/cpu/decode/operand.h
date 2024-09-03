#ifndef __OPERAND_H__
#define __OPERAND_H__

enum { OP_TYPE_REG, OP_TYPE_MEM, OP_TYPE_IMM };

#define OP_STR_SIZE 40

typedef struct {
	uint32_t type;  //操作数类型
	size_t size;	//操作数大小
	union {			//操作数的值（联合体）
		uint32_t reg;
		swaddr_t addr;
		uint32_t imm;  //无符号立即数
		int32_t simm;  //有符号立即数
	};
	uint32_t val;	//操作数的值
	char str[OP_STR_SIZE];  //打印调试信息
} Operand;  //操作数

typedef struct {
	uint32_t opcode;
	bool is_operand_size_16;
	Operand src, dest, src2;  //源操作数，目标操作数，第二个源操作数
} Operands;

#endif
