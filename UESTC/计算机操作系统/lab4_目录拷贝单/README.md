# 实验内容

利用 POSIX API 在 Linux 系统上编写应用程序，仿写 cp 命令的部分功能，将源文件/目录复制到另外一个文件或复制到另外一个目录。

源文件路径和目标文件路径通过命令行参数来指定。

```shell
[test@linux test]$ ./mycp /home/test1.txt /usr/test2.txt
[test@linux test]$ ./mycp /home/test1.txt /tmp
[test@linux test]$ ./mycp /home/test(目录) /tmp
```

