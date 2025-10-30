主要是利用 flex 工具生成一个词法分析器 lexer.

词法分析

请根据词法分析实验中给定的文法，利用 flex 设计一词法分析器，该分析器从标准输入读入源代码后，输出单词的类别编号及附加信息。 附加信息规定如下： 当类别为 Y_ID、num_INT 或 num_FLOAT 时，附加信息为该类别对应的属性，如 main, 100, 29.3等； 当类别为 关键字 时，附件信息为 KEYWORD； 当类别为 运算符 时，附件信息为 OPERATOR； 当类别为 其它符号 时，附件信息为 SYMBOL； 例如对于源代码

```c
int main(){
    return 3;
}
```
词法分析器的输出为：
```
<261, KEYWORD>
<260, main>
<285, SYMBOL>
<286, SYMBOL>
<287, SYMBOL>
<269, KEYWORD>
<258, 3>
<292, SYMBOL>
<288, SYMBOL>
```

请在本页面上传词法分析实验所有源代码及实验报告（源代码至少包括一个flex源文件，即 .l 文件，如有其它 .c 或 .h 文件，也请一并上传）

注：所有源代码及实验报告需同时上传，需同时上传，需同时上传。

本实验中的单词类别定义如下，该定义包含在 token.h 中，此头文件可直接使用，无需上传

```c
// 单词类别
enum yytokentype {
	num_INT = 258,
	num_FLOAT = 259,

	Y_ID = 260,

	Y_INT = 261,
	Y_VOID = 262,
	Y_CONST = 263,
	Y_IF = 264,
	Y_ELSE = 265,
	Y_WHILE = 266,
	Y_BREAK = 267,
	Y_CONTINUE = 268,
	Y_RETURN = 269,

	Y_ADD = 270,
	Y_SUB = 271,
	Y_MUL = 272,
	Y_DIV = 273,
	Y_MODULO = 274,
	Y_LESS = 275,
	Y_LESSEQ = 276,
	Y_GREAT = 277,
	Y_GREATEQ = 278,
	Y_NOTEQ = 279,
	Y_EQ = 280,
	Y_NOT = 281,
	Y_AND = 282,
	Y_OR = 283,
	Y_ASSIGN = 284,

	Y_LPAR = 285,
	Y_RPAR = 286,
	Y_LBRACKET = 287,
	Y_RBRACKET = 288,
	Y_LSQUARE = 289,
	Y_RSQUARE = 290,
	Y_COMMA = 291,
	Y_SEMICOLON = 292,

	Y_FLOAT = 293
};
```