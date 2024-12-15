../test_cases 是一些测试用例，使用前请移动到本目录下。

你需要撰写实验一词法分析的实验报告，将报告、`auto_lex.l` 以及 `main.c` 一并上传 icoding 进行提交。

在此目录下运行 `make all` 命令会产生一个词法分析器 `lex.yy.c` 以及一个可执行文件 `main`。

在此目录下运行 `make test` 命令，会产生一个 `test_cases_output` 文件夹，可以看到已经将字符流转化为 token 流。

结束后可执行 `make clean` 命令。

```bash
...
Processing ./test_cases/97_matrix_sub.sy -> ./test_cases_output/97_matrix_sub.out
Processing ./test_cases/98_matrix_mul.sy -> ./test_cases_output/98_matrix_mul.out
Processing ./test_cases/99_matrix_tran.sy -> ./test_cases_output/99_matrix_tran.out
All tests completed successfully.
```