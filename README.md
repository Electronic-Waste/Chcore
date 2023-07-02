# ChCore

>  This is the repository of ChCore labs in SE3357, 2023 Spring.

## 课程/Lab相关情况说明

- 评分规则：课程平时分占15%，六个Lab占45%，期末考试占40%
- 课上无点名，但是夏老师、海波老师、糜泽羽老师讲得都非常好，不来绝对是你的损失(doge)
- Chcore Lab非常硬核，主要内容是编写采用微内核架构的Chcore OS相关代码，会让你深入操作系统内核，编码实现从启动内核，到虚拟/物理地址管理、进程线程创建、异常处理、多核多进程调度、IPC机制、文件系统、Shell终端、设备驱动等多个模块。**相信如果Lab从头到尾都是你自己做的话，你对操作系统内核的设计思路以及工作机制的认识将会有极大的提升！**
- Chcore Lab是一个很好的Lab，希望学弟学妹珍惜这次学习的机会，**尽量自己写，拒绝做Copycat**，以后就没有这样好的机会了哇！



## 文档目录说明

- [/ans](./ans)：各个Lab实现的说明文档，包含思考题的回答以及实践题的实现思路
- [/doc](./doc)：各个Lab的作业要求文档
- [/hw](./hw)：老师布置的两次作业（虚拟化部分没有Lab，本学期用作业的形式代替）
- [/notes](./notes)：本学期我记的笔记，前半部分比较详尽，后半部分比较偷懒（雾），希望能帮到学弟学妹



## 运行

> 以下为课程助教给出的README.md文档原内容

### Build

- `make` or `make build`: Build ChCore
- `make clean`: Clean ChCore

### Emulate

- `make qemu`: Start a QEMU instance to run ChCore

### Debug with GBD

- `make qemu-gdb`: Start a QEMU instance with GDB server
- `make gdb`: Start a GDB (gdb-multiarch) client

### Grade

- `make grade`: Show your grade of labs in the current branch

### Other

- Press `Ctrl+a x` to quit QEMU
- Press `Ctrl+d` to quit GDB
