#include "cpu/exec/helper.h"
//模拟处理器指令执行中的“调用”（CALL）指令


//这个函数处理的是一种形式的CALL指令，其中目标地址是通过某种立即数（Immediate Value）或短立即数（Short Immediate）方式指定的。
make_helper(call_si) {
	int len = decode_si_l(eip + 1);  //从当前指令指针（eip）之后的地址解码出目标地址的长度
	swaddr_t ret_addr = cpu.eip + len + 1;  //计算返回地址，即当前指令执行完毕后应该返回的地址。这是通过将当前指令指针（eip）加上目标地址的长度和1（当前指令的长度，因为CALL指令自身也占用一个地址）来实现的。
	swaddr_write(cpu.esp - 4, 4, ret_addr);  //将返回地址压入栈中（cpu.esp - 4），并更新栈指针（cpu.esp）
	cpu.esp -= 4;  
	cpu.eip += op_src->val;  //cpu.eip更新为目标地址（通过op_src->val获取
	print_asm("call %x", cpu.eip + 1 + len);

	return len + 1;  //返回指令的长度加1，以便模拟执行下一条指令。
}

//这个函数处理的是另一种形式的CALL指令，其中目标地址是通过寄存器或内存地址（Register or Memory）间接指定的
make_helper(call_rm) {
	int len = decode_rm_l(eip + 1);  //同上
	swaddr_t ret_addr = cpu.eip + len + 1;  //同上
	swaddr_write(cpu.esp - 4, 4, ret_addr);  //同上
	cpu.esp -= 4;  //同上
	cpu.eip = op_src->val - (len + 1);  //这里的不同之处在于，目标地址不是直接给出的，而是存储在某个寄存器或内存地址中。因此，cpu.eip被设置为op_src->val - (len + 1)
	print_asm("c all *%s", op_src->str);

	return len + 1;  //同上
}


