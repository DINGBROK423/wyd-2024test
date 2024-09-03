void init_monitor(int, char *[]);
void reg_test();
void restart();
void ui_mainloop();

int main(int argc, char *argv[]) {

	/* Initialize the monitor. */
	init_monitor(argc, argv);  //参数初始化,打开log文件，加载字符串和符号表，编译正则表达式，初始化监视点结构池

	/* Test the implementation of the `CPU_state' structure. */
	reg_test();//测试寄存器结构

	/* Initialize the virtual computer system. */
	restart();//虚拟机启动  0x100000位置加载程序 load_entry

	/* Receive commands from user. */
	ui_mainloop();//模拟器主循环 输入c进入指令执行主循环 cpu_exec()

	return 0;
}
// 指令前缀默认只有Operand-Size 存在 如果存在一定是十六进制66   表示指令操作数的长 存在证明操作舒适16位，不存在可能是8或32位           