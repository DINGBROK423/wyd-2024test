#include "cpu/exec/helper.h"

#if DATA_BYTE == 1

#define SUFFIX b
#define DATA_TYPE uint8_t
#define DATA_TYPE_S int8_t

#elif DATA_BYTE == 2

#define SUFFIX w
#define DATA_TYPE uint16_t
#define DATA_TYPE_S int16_t

#elif DATA_BYTE == 4

#define SUFFIX l
#define DATA_TYPE uint32_t
#define DATA_TYPE_S int32_t

#else

#error unknown DATA_BYTE

#endif

#define REG(index) concat(reg_, SUFFIX) (index)  //访问寄存器
#define REG_NAME(index) concat(regs, SUFFIX) [index]

#define MEM_R(addr) swaddr_read(addr, DATA_BYTE)  //读内存
#define MEM_W(addr, data) swaddr_write(addr, DATA_BYTE, data)//写内存 

#define OPERAND_W(op, src) concat(write_operand_, SUFFIX) (op, src)  //值写入操作数

#define MSB(n) ((DATA_TYPE)(n) >> ((DATA_BYTE << 3) - 1))  //获取最高位的值
