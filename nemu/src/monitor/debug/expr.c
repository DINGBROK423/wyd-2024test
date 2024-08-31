#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

enum {
	/* TODO: Add more token types */
	NOTYPE = 256, EQ, NUM, NEQ, OR, AND, REG, REF, NEG
};   
//枚举类型，hash值从256开始，防止与ascll码冲突。
//同时隐式表示了各个运算符运算顺序
//REG:寄存器 REF解引用 NEG 否定 

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */
	//定义规则：正则表达式语法
	{" +",	NOTYPE},				// spaces
	{"\\+", '+'},					// plus
	{"==", EQ},						// equal
	{"0x[0-9a-fA-F]{1,8}", NUM},			// hex（十六进制）
	{"[0-9]{1,10}", NUM},					// dec（十进制）
	{"\\$[a-z]{1,31}", REG},				// register names 
	{"-", '-'},
	{"\\*", '*'},
	{"/", '/'},
	{"%", '%'},
	{"!=", NEQ},
	{"&&", AND},
	{"\\|\\|", OR},
	{"!", '!'},
	{"\\(", '('},
	{"\\)", ')'} 
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;
	//regcomp： 编译数组规则中的每个 regex 错误处理： 如果 regex 编译失败，程序会生成错误信息，并断言
	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token; //结构体储存token

Token tokens[32];
int nr_token;
//make_token:该函数遍历 regex 规则列表，并使用 .regexec 逐一应用这些规则 
//Token Creation： 如果找到匹配，则将相应的标记存储到数组
//tokens substr_len 和 substr_start： 确定匹配子串的长度和起始位置： 将匹配的子字符串复制到令牌的字符串字段，用于令牌类型和 
//NUMREG 错误处理： 如果未找到匹配字符串，函数将返回 false
static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;
				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */
				switch(rules[i].token_type) {
                                        case NOTYPE: break;
                                        case NUM:
					//default: panic("please implement me");
                                        case REG: sprintf(tokens[nr_token].str, "%.*s", substr_len, substr_start);
					default: tokens[nr_token].type = rules[i].token_type;
							 nr_token ++;
				}
				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}
	return true; 
}

/*TODO: Expression evaluation*/
//op_prec 功能： 返回运算符的优先级。 数字越小优先级越高
static int op_prec(int t) {
	switch(t) {
		case '!': case NEG: case REF: return 0;
		case '*': case '/': case '%': return 1;
		case '+': case '-': return 2;
		case EQ: case NEQ: return 4;
		case AND: return 8;
		case OR: return 9;
		default: assert(0);
	}
}

static inline int op_prec_cmp(int t1, int t2) {
	return op_prec(t1) - op_prec(t2);
}
//find_dominated_op 函数： 查找给定范围内优先级最低的运算符（占优运算符）。 
//括号处理： 该函数通过跟踪 .bracket_level 主导运算符，正确处理括号内的表达式： 该函数返回表达式中优先级最低的运算符的索引。
static int find_dominated_op(int s, int e, bool *success) {
	int i;
	int bracket_level = 0;
	int dominated_op = -1;
	for(i = s; i <= e; i ++) {
		switch(tokens[i].type) {
			case REG: case NUM: break;

			case '(': 
				bracket_level ++; 
				break;

			case ')': 
				bracket_level --; 
				if(bracket_level < 0) {
					*success = false; //括号数量不符
					return 0;
				}
				break;

			default:
				if(bracket_level == 0) {
					if(dominated_op == -1 || 
							op_prec_cmp(tokens[dominated_op].type, tokens[i].type) < 0 ||
							(op_prec_cmp(tokens[dominated_op].type, tokens[i].type) == 0 && 
							 tokens[i].type != '!' && tokens[i].type != '~' &&
							 tokens[i].type != NEG && tokens[i].type != REF) ) {
						dominated_op = i;
					}
				}
				break;
		}
	}

	*success = (dominated_op != -1);
	return dominated_op;
}

uint32_t get_reg_val(const char*, bool *);
//eval 函数： 对表达式进行递归求值 
//基本情况：如果表达式是单个标记(REG,NUM),函数将返回其值.
// 如果表达式位于括号内，函数将对内部表达式进行求值。 
//递归求值： 该函数查找主导运算符，并递归计算其两侧的子表达式，根据运算符合并结果。
static uint32_t eval(int s, int e, bool *success) {
	if(s > e) {
		// bad expression
		*success = false;
		return 0;
	}
	else if(s == e) {
		// single token
		uint32_t val;
		switch(tokens[s].type) {
			case REG: val = get_reg_val(tokens[s].str + 1, success);	// +1 to skip '$'
					  if(!*success) { return 0; }
					  break;

			case NUM: val = strtol(tokens[s].str, NULL, 0); break;

			default: assert(0);
		}

		*success = true;
		return val;
	}
	else if(tokens[s].type == '(' && tokens[e].type == ')') {//左右括号匹配
		return eval(s + 1, e - 1, success);
	}  
	else {
		int dominated_op = find_dominated_op(s, e, success);
		if(!*success) { return 0; }

		int op_type = tokens[dominated_op].type;
		if(op_type == '!' || op_type == NEG || op_type == REF) {
			uint32_t val = eval(dominated_op + 1, e, success);
			if(!*success) { return 0; }

			switch(op_type) {
				case '!': return !val;
				case NEG: return -val;
				case REF: return swaddr_read(val, 4);
				default: assert(0);
			}
		}

		uint32_t val1 = eval(s, dominated_op - 1, success);
		if(!*success) { return 0; }
		uint32_t val2 = eval(dominated_op + 1, e, success);
		if(!*success) { return 0; }

		switch(op_type) {
			case '+': return val1 + val2;
			case '-': return val1 - val2;
			case '*': return val1 * val2;
			case '/': return val1 / val2;
			case '%': return val1 % val2;
			case EQ: return val1 == val2;
			case NEQ: return val1 != val2;
			case AND: return val1 && val2;
			case OR: return val1 || val2;
			default: assert(0);
		}
	}
}

/* TODO: Expression evaluation end */

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}//预处理

	/* TODO: Insert codes to evaluate the expression. */
       	//panic("please implement me");
	//return 0;
        /* Detect REF and NEG tokens */
	int i;
	int prev_type;
	//NEG：如果前面没有有效操作数（数字、寄存器或结束括号），则将减号标识为否定运算符
	// REF： 如果前面没有有效操作数，则将星号标识为解引用运算符。
	for(i = 0; i < nr_token; i ++) {
		if(tokens[i].type == '-') {
			if(i == 0) {
				tokens[i].type = NEG;
				continue;
			}

			prev_type = tokens[i - 1].type;
			if( !(prev_type == ')' || prev_type == NUM || prev_type == REG) ) {
				tokens[i].type = NEG;
			}
		}

		else if(tokens[i].type == '*') {
			if(i == 0) {
				tokens[i].type = REF;
				continue;
			}

			prev_type = tokens[i - 1].type;
			if( !(prev_type == ')' || prev_type == NUM || prev_type == REG) ) {
				tokens[i].type = REF;
			}
		}
	}

	return eval(0, nr_token - 1, success);//递归，nr_token减1，然后进入下一层
}

