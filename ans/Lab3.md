# Lab3

> 思考题 1: 内核从完成必要的初始化到用户态程序的过程是怎么样的？尝试描述一下调用关系。

1. 内核在完成必要的初始化之后，先调用`arch_interrupt_init`函数进行异常向量表的初始化
2. 然后调用`create_root_thread`创建第一个用户态程序。在`create_root_thread`中：
   1. 内核先调用`create_root_cap_group`创建内核对象(kernel object)——进程的抽象`cap_group`
   2. 之后再调用`__create_root_thread`在`cap_group`中创建root thread
   3. 最后通过`obj_put`和`switch_to_thread`函数设置新创建的root thread，将`current_thread`设置为root thread
3. 最后回到main函数中，调用`eret_to_thread(switch_context())`进行context switch，真正切换到刚刚创建的root上运行



> 练习题 2: 在 `kernel/object/cap_group.c` 中完善 `cap_group_init`、`sys_create_cap_group`、`create_root_cap_group` 函数。在完成填写之后，你可以通过 Cap create pretest 测试点。

- `cap_group_init`函数是初始化cap_group的函数，需要依次初始化slot_table, thread_list, thread_cnt以及pid（根据传入的参数决定）
- `sys_create_cap_group`是用户创建cap_group的函数，在需要我们填写的部分中：
  1. 调用`obj_alloc`创建`new_cap_group`对象，同时调用`cap_group_init`来初始化cap_group
  2. 调用`obj_alloc`创建`vmspace`对象
- `create_root_cap_group`是用户创建第一个进程所使用的函数，在我们需要填写的部分中：
  1. 调用`obj_alloc`创建`new_cap_group`对象
  2. 调用`cap_group_init`来初始化`cap_group`，同时调用`cap_alloc`来更新`cap_group`的`slot_table`，将`cap_group`自身加入到其中
  3. 调用`obj_alloc`创建`vmspace`对象
  4. 设置`vmspace->pcid`为`ROOT_PCID`，同时调用`vmspace_init`来初始化`vmspace`，再调用`cap_alloc`来更新`cap_group`的`slot_table`，将`vmspace`加入到其中



> 练习题 3: 在 `kernel/object/thread.c` 中完成 `load_binary` 函数，将用户程序 ELF 加载到刚刚创建的进程地址空间中。

由于Chcore系统中尚无文件系统，所有用户程序镜像以ELF二进制的形式直接嵌入到内核镜像中，所以相当于我们可以直接从内存中访问ELF文件，观察`load_binary`函数的参数，我们不难发现`bin`就是ELF文件开头的位置，我们进行如下操作：

1. 从program headers中获取flags（调用`PFLAGS2VMRFLAGS`将其转化为vmr_flags_t类型）

2. 算出`seg_map_sz`，需要以页为粒度去映射，因此需要调用`ROUND_UP`和`ROUND_DOWN`

   ```c++
   seg_map_sz = ROUND_UP(p_vaddr + seg_sz, PAGE_SIZE) - ROUND_DOWN(p_vaddr, PAGE_SIZE);
   ```

3. 为每个ELF section创建一个对应的pmo，调用`create_pmo`

4. 将ELF文件中对应的section拷贝到刚刚创建的pmo的内存中(调用`memcpy`)

5. 调用`vmspace_map_range`添加内存映射，以页为粒度



> 练习题 4: 按照前文所述的表格填写 `kernel/arch/aarch64/irq/irq_entry.S` 中的异常向量表，并且增加对应的函数跳转操作。

1. 异常向量表调用宏定义函数`exception_entry`+`label`来实现，对照前文的表格填入
2. 函数跳转即为`bl <handler>`，`irq_el1h`, `irq_el1t`, `fiq_el1t`, `fiq_el1h`, `error_el1t`, `error_el1h`, `sync_el1t`, `sync_el1h`调用`unexpected_handler`，`sync_el1h`调用`handle_entry_c`



> 练习题 5: 填写 `kernel/arch/aarch64/irq/pgfault.c` 中的 `do_page_fault`，需要将缺页异常转发给 `handle_trans_fault` 函数。

- 直接调用`ret = handle_trans_fault(current_thread->vmspace, fault_addr)`即可



> 练习题 6: 填写 `kernel/mm/pgfault_handler.c` 中的 `handle_trans_fault`，实现 `PMO_SHM` 和 `PMO_ANONYM` 的按需物理页分配。

1. 首先调用`get_page_from_pmo`获取缺页异常的page的物理地址
2. 检查pmo中当前的fault地址对应的物理页是否存在
   1. 若未分配(`pa == 0`)，则通过`get_pages`分配一个物理页，然后用`commit_page_pmo`将页记录在pmo中，用`map_range_in_pgtbl`增加页表映射
   2. 若已分配，则调用`map_range_in_pgtbl`修改页表映射



> 练习题 7: 按照前文所述的表格填写 `kernel/arch/aarch64/irq/irq_entry.S` 中的 `exception_enter` 与 `exception_exit`，实现上下文保存的功能。

- `exception_enter`中用`stp`将`x0`~`x29`寄存器依次保存，然后在获取系统寄存器之后，将这些系统寄存器中的值与`x30`一起保存，最后更新`sp`的值
- `exception_exit`同理，先将系统寄存器中的值从内存中通过`ldp`加载出来存入系统寄存器，然后再依次使用`stp`将`x0`~`x29`从内存中加载出来恢复，最后更新`sp`的值



> 思考题 8： ChCore中的系统调用是通过使用汇编代码直接跳转到`syscall_table`中的 相应条目来处理的。请阅读`kernel/arch/aarch64/irq/irq_entry.S`中的代码，并简要描述ChCore是如何将系统调用从异常向量分派到系统调用表中对应条目的。

1. 在user态进行系统调用时，代码下陷到kernel态触发同步异常，处理器在异常向量表中找到对应的异常处理程序代码`sync_el0_64`
2. 进入到`sync_el0_64`后，首先调用`exception_enter`进行上下文保存，进行一些比较之后调用`el0_syscall`
3. `el0_syscall`先到syscall_table中查询对应的syscall entry，也即对应的异常handler函数，然后通过`blr`调用这个函数
4. 最后调用`exception_exit`恢复上下文，退出syscall



> 练习题 9: 填写 `kernel/syscall/syscall.c` 中的 `sys_putc`、`sys_getc`，`kernel/object/thread.c` 中的 `sys_thread_exit`，`libchcore/include/chcore/internal/raw_syscall.h` 中的 `__chcore_sys_putc`、`__chcore_sys_getc`、`__chcore_sys_thread_exit`，以实现 `putc`、`getc`、`thread_exit` 三个系统调用。

1. `sys_thread_exit`：将`current_thread`的`thread_exit_state`设置为`TE_EXITED`，然后调用`thread_deinit`来销毁当前的thread
2. `sys_putc`：调用`uart_send`
3. `sys_getc`：调用`uart_recv`
4. `__chcore_sys_putc`：调用`__chcore_syscall1`，传入系统调用编号和`ch`
5. `__chcore_sys_getc`：调用`__chcore_sys_call0`，传入系统调用编号
6. `__chcore_sys_thread_exit`：调用`__chcore_syscall0`，传入系统调用编号