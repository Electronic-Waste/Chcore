# Lab1

- 思考题1：阅读 `_start` 函数的开头，尝试说明 ChCore 是如何让其中一个核首先进入初始化流程，并让其他核暂停执行的。

  答：各个CPU首先从系统中取出`mpidr_el1`，其中`mpidr_el1`为各个CPU的序列号，经过出处理后，进入一个conditional branch。如果CPU的序列号等于0则跳转到primary函数执行CPU初始化操作，若CPU的序列号不等于0则调用`b .`停留在原地，直到引入SMP



- 练习题 2：在 `arm64_elX_to_el1` 函数的 `LAB 1 TODO 1` 处填写一行汇编代码，获取 CPU 当前异常级别。

  ```
  mrs x9, CurrentEL
  ```

  通过`mrs`指令将`CurrentEL`寄存器中的值读入到x9寄存器中，在gdb中打印后输出为12，是EL3的exception level。



- 练习题 3：在 `arm64_elX_to_el1` 函数的 `LAB 1 TODO 2` 处填写大约 4 行汇编代码，设置从 EL3 跳转到 EL1 所需的 `elr_el3` 和 `spsr_el3` 寄存器值。具体地，我们需要在跳转到 EL1 时暂时屏蔽所有中断、并使用内核栈（`sp_el1` 寄存器指定的栈指针）。

  ```
  adr x9, .Ltarget
  msr elr_el3, x9
  mov x9, SPSR_ELX_DAIF | SPSR_ELX_EL1H
  msr spsr_el3, x9
  ```

  > 在这里我们是在为eret做准备，eret调用后会使用`elr_el3`中的地址作为下一条指令的地址，同时将`spsr_el3`中保存的process state设置新的exception level中的process state

  1. 首先设置`elr_el3`的值。我们选择将其设置为`.Ltarget`的地址，和EL2共享同一个出口

  2. 然后我们在`spsr_el3`中设置DAIF bits以关闭中断，设置execution mode以进入el1(内核态)



- 思考题 4：结合此前 ICS 课的知识，并参考 `kernel.img` 的反汇编（通过 `aarch64-linux-gnu-objdump -S` 可获得），说明为什么要在进入 C 函数之前设置启动栈。如果不设置，会发生什么？

  答：因为C语言会有压栈操作（参数传递、Callee-saved、Caller-saved, ret addr等）。如果不设置栈的话，则C语言压栈相关的操作便无法正常执行，可能会发生程序崩溃。



- 思考题 5：在实验 1 中，其实不调用 `clear_bss` 也不影响内核的执行，请思考不清理 `.bss` 段在之后的何种情况下会导致内核无法工作。

  答：在`.bss`段存储的未被初始化全局变量和静态变量若未执行`clear_bss`，则在后面使用时的初始值便不是0，这对一些假定全局变量和静态变量的初始值为0的内核段代码会是致命的错误。



- 练习题 6：在 `kernel/arch/aarch64/boot/raspi3/peripherals/uart.c` 中 `LAB 1 TODO 3` 处实现通过 UART 输出字符串的逻辑。

  ```c++
  void uart_send_string(char *str)
  {
          /* LAB 1 TODO 3 BEGIN */
          early_uart_init();      // Initialize uart first
          int offset = 0;
          while (str[offset] != '\0') 
                  early_uart_send(str[offset++]);
          /* LAB 1 TODO 3 END */
  }
  ```

  首先调用`early_uart_init()`来初始化uart

  接着我们对字符串`str`中每一个字符调用`early_uart_send`函数，一个一个将字符输出



- 练习题 7：在 `kernel/arch/aarch64/boot/raspi3/init/tools.S` 中 `LAB 1 TODO 4` 处填写一行汇编代码，以启用 MMU。

  ```
  orr		x8, x8, #SCTLR_EL1_M
  ```

  仿照下方设置的格式，清除用`bic`指令，设置用`orr`指令。设置了`SCTLR_EL1_M`的位之后通过`msr`指令将设置好的位更新到`sctlr_el1`寄存器中

