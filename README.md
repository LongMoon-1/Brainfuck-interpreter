# BF Interpreter

一个功能完备的 **Brainfuck** 语言解释器，使用 C 语言实现。

不仅支持标准的 Brainfuck 代码执行，还提供了交互式命令行环境，内置文件系统操作命令，可直接执行 `.bf` 文件。

---

## 特性

- ✅ 完整支持 Brainfuck 指令：`> < + - . , [ ]`
- ✅ 交互式 REPL 模式，逐行运行 BF 代码
- ✅ 内置命令：`help`、`clear`、`exit`、`cd`、`pwd`、`dir` / `ls`、`bf 文件.bf`
- ✅ 支持从文件加载并执行 `.bf` 程序
- ✅ UTF-8 控制台支持（Windows 自动设置）
- ✅ 清晰的帮助信息与错误提示
