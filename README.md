# 目录结构
* `boot`: 引导操作系统
* `build`: 默认为空，存放编译结果
    * `build/fs.img`: 文件系统镜像（由`build/fs`文件夹经`tools/mkfs`打包而成）
    * `build/xv86.img`: 引导及内核
* `distrib`: 用户态程序及相关组件
    * `distrib/ulib`: 常用的库函数、及系统调用接口（静态链接至程序）
    * `distrib/uprogs`: 用户态程序，包括 shell 和常用的命令
* `include`: 所有头文件
    * `include/xv6`: 与操作系统紧密相关的 API
* `kernel`: 内核
* `tools`: 开发工具，目前仅有文件系统镜像打包

# 配置
运行以下命令可以编译并运行（`*-nox`避免图形界面）。

    make qemu-nox

可以在 `Makefile.common` 中配置编译的 Release/Debug 选项，调试的端口以及模拟器的性能。
默认为 Debug 编译，调试端口为 26000。可运行 gdb，输入如下命令进行远程调试。

    target remote localhost:26000
    file kernel

# 安利
CLion 支持远程 gdb 调试，效果很好。

# 计划
1. 优化进程调度、内存管理等代码
2. 试图兼容 ANSI C 库

