# Lab4

> 思考题 1：阅读汇编代码`kernel/arch/aarch64/boot/raspi3/init/start.S`。说明ChCore是如何选定主CPU，并阻塞其他其他CPU的执行的。

1. 首先ChCore先从`mpidr_el1`中取出各个CPU的`cpu_id`
2. 选定`cpu_id`为0的CPU为主CPU，跳到`primary`中执行
3. 其它的CPU不进行跳转，进入到`wait_until_smp_enabled`中循环，直到对应的`secondary_boot_flag`被设置为0（由主CPU设置）



> 思考题 2：阅读汇编代码`kernel/arch/aarch64/boot/raspi3/init/start.S, init_c.c`以及`kernel/arch/aarch64/main.c`，解释用于阻塞其他CPU核心的`secondary_boot_flag`是物理地址还是虚拟地址？是如何传入函数`enable_smp_cores`中，又该如何赋值的（考虑虚拟地址/物理地址）？

1. `secondary_boot_flag`在其它CPU中是物理地址，因为此时其它CPU中的MMU还处于关闭的状态，虚拟地址与物理地址的转换尚未打开；在主CPU中是虚拟地址，因为主CPU打开了MMU，虚拟地址与物理地址的转换已经开启
2. `secondary_boot_flag`的传递过程如下：
   1. 主CPU运行`init_c`函数时会调用`start_kernel`函数，会把`secondary_boot_flag`传入到`start_kernel`中
   2. 主CPU运行`start_kernel`函数时会调用`main`函数，会把`secondary_boot_flag`传入到`main`函数中
   3. 最后`main`函数将`secondary_boot_flag`传入到`enable_smp_cores`中
3. `secondary_boot_flag`在`enable_smp_cores`中被赋值
   1. 首先`secondary_boot_flag`在刚刚传入`enable_smp_cores`中时是物理地址，调用`phys_to_virt`来将其转化为虚拟地址，因为主CPU打开了MMU，虚拟地址与物理地址的转换已经开启
   2. `secondary_boot_flag`是一个数组，数组下标对应于`cpu_id`，会根据CPU的核数来依次对其进行赋值



> 练习题 3：完善主CPU激活各个其他CPU的函数：`enable_smp_cores`和`kernel/arch/aarch64/main.c`中的`secondary_start`。

- `enable_smp_cores`函数

  1. 需要将其它CPU从`wait_until_smp_enabled`的循环中释放出来，将它们各自的对应的`secondary_boot_flag`值改为非0值

     ```
     secondary_boot_flag[i] = 1;
     ```

  2. 等待各个CPU的初始化进程完成

     ```
     while (cpu_status[i] != cpu_run);
     ```

- `secondary_start`函数

  1. 设置cpu_status

     ```
     cpu_status[cpuid] = cpu_run;
     ```



> 练习题 4：本练习分为以下几个步骤：
>
> 1. 请熟悉排号锁的基本算法，并在`kernel/arch/aarch64/sync/ticket.c`中完成`unlock`和`is_locked`的代码。
> 2. 在`kernel/arch/aarch64/sync/ticket.c`中实现`kernel_lock_init`、`lock_kernel`和`unlock_kernel`。
> 3. 在适当的位置调用`lock_kernel`。
> 4. 判断什么时候需要放锁，添加`unlock_kernel`。（注意：由于这里需要自行判断，没有在需要添加的代码周围插入TODO注释）

1. `unlock`操作需要将`lock->owner`的值加1，`is_locked`的判断依据是`lock->owner`和`lock->next`是否相等
2. 这三个函数分别调用`lock_init`，`lock`，`unlock`即可
3. 在五个函数有`TODO`注释标记的地方添加对`lock_kernel`的调用

4. 在以下位置释放锁：
   1. `sync_el0_64`中调用完`handle_entry_c`之后
   2. `el0_syscall`中从syscall返回之后
   3. `__eret_to_thread`中即将退出时



> 思考题 5：在`el0_syscall`调用`lock_kernel`时，在栈上保存了寄存器的值。这是为了避免调用`lock_kernel`时修改这些寄存器。在`unlock_kernel`时，是否需要将寄存器的值保存到栈中，试分析其原因。

- 在`unlock_kernel`时，不需要将寄存器的值保存到栈中，因为`unlock_kernel`几乎不涉及对寄存器的修改，只是改变了`lock->owner`这一在内存中的值



> 思考题 6：为何`idle_threads`不会加入到等待队列中？请分析其原因？

- 因为如果`idle_threads`加入到了等待队列中，则意味着`idle_threads`也会被调度到。然而`idle_threads`并没有什么实际意义，只是一个循环空转，调度到`idle_threads`是对CPU资源的一种浪费。因此为了节约计算资源提高性能，`idle_threads`不会加入到等待队列中



> 练习题 7：完善`kernel/sched/policy_rr.c`中的调度功能，包括`rr_sched_enqueue`，`rr_sched_dequeue`，`rr_sched_choose_thread`与`rr_sched`，需要填写的代码使用`LAB 4 TODO BEGIN`标出。

- `rr_sched_enqueue`函数
  1. 首先对参数进行一些判断，做错误处理
     1. `thread`或者`thread_ctx`为NULL
     2. 已经入队的thread不能重复入队（state不能是`TS_READY`）
     3. IDLE线程不能入队
  2. 设置当前thread的状态，包括state和cpuid
  3. 将其加入到等待队列中，更新队列长度

- `rr_sched_dequeue`函数
  1. 首先对参数进行一些判断，做错误处理
     1. `thread`或者`thread_ctx`为NULL
     2. 出队的对象不能是IDLE线程
     3. 出队的对象不能是不在队列中的线程（state一定是`TS_READY`）
  2. 设置当前thread的状态，包括state和cpuid
  3. 将其出队，更新队列长度

- `rr_sched_choose_thread`函数
  1. 如果等待队列的长度为0，返回IDLE线程
  2. 否则返回队列中第一个线程，将其出队

- `rr_sched`函数
  1. 进行条件判断
     1. 如果`current_thread`为NULL或者其`thread_ctx`为NULL，不做任何处理
     2. 如果`current_thread`的`thread_exit_state`为`TE_EXITING`，则将`state`设置为`TS_EXIT`，将`thread_exit_state`设置为`TE_EXITED`
     3. 如果`current_thread`的`state`不是`TS_WAITING`，则将`current_thread`入队
  2. 调用`switch_to_thread`换线程



> 思考题 8：如果异常是从内核态捕获的，CPU核心不会在`kernel/arch/aarch64/irq/irq_entry.c`的`handle_irq`中获得大内核锁。但是，有一种特殊情况，即如果空闲线程（以内核态运行）中捕获了错误，则CPU核心还应该获取大内核锁。否则，内核可能会被永远阻塞。请思考一下原因。

- 因为在同一时刻可能有多个CPU在运行空闲线程（以内核态运行），如果不获取大内核锁则有可能出现多个错误处理函数在不同CPU上同时运行内核代码的情况，相应的共享数据结构可能会被错误修改而产生代码运行故障，因此可能会出现内核永远被阻塞的问题。
- 也可能是因为如果空闲线程触发了调度函数，在返回用户态线程的过程中需要放一次锁，如空闲线程在这之前没有拿到锁的话会出现错误



> 练习题 9：在`kernel/sched/sched.c`中实现系统调用`sys_yield()`，使用户态程序可以启动线程调度。此外，ChCore还添加了一个新的系统调用`sys_get_cpu_id`，其将返回当前线程运行的CPU的核心id。请在`kernel/syscall/syscall.c`文件中实现该函数。

- `sys_yield()`函数
  1. 重置当前线程的预算，确保可以立刻调度当前线程
  2. 调用`sched`函数
  3. 调用`eret_to_thread`函数切换线程

- `sys_get_cpu_id()`函数

  为`cpuid`赋值`cpuid = smp_get_cpu_id();`



> 练习题 10：定时器中断初始化的相关代码已包含在本实验的初始代码中（`timer_init`）。请在主CPU以及其他CPU的初始化流程中加入对该函数的调用。此时，`yield_spin.bin`应可以正常工作：主线程应能在一定时间后重新获得对CPU核心的控制并正常终止。

- 在主CPU初始化的`main`函数中

  在`arch_interrupt_init()`调用下方调用`timer_init()`，打开时钟中断

- 在其它CPU初始化的`secondary_start`函数中

  在`lock_kernel()`前调用`timer_init()`，打开时钟中断



> 练习题 11：在`kernel/sched/sched.c`处理时钟中断的函数`sched_handle_timer_irq`中添加相应的代码，以便它可以支持预算机制。更新其他调度函数支持预算机制，不要忘记在`kernel/sched/sched.c`的`sys_yield()`中重置“预算”，确保`sys_yield`在被调用后可以立即调度当前线程。完成本练习后应能够`tst_sched_preemptive`测试并获得5分。

- `sched_handle_timer_irq`函数

  在`current_thread`、`current_thread->thread_ctx`不为NULL，且`budget`不等于0的时候，将budget减去1

- 其它调度函数预算机制的更新

  在`rr_sched`函数中，在当前进程的`budget`不等于0且`affinity`与当前CPU的cpuid匹配时，不改变当前运行的线程

- `sys_yield`

  1. 重置当前线程的预算，确保可以立刻调度当前线程
  2. 调用`sched`函数
  3. 调用`eret_to_thread`函数切换线程



> 练习题 12：在`kernel/object/thread.c`中实现`sys_set_affinity`和`sys_get_affinity`。完善`kernel/sched/policy_rr.c`中的调度功能，增加线程的亲和性支持（如入队时检查亲和度等，请自行考虑）。

- `sys_set_affinity`函数以及`sys_get_affinity`函数

  简单返回或设置`affinity`即可

- `rr_sched_enqueue`函数

  1. 首先检查`aff`是否valid，否则返回`-EINVAL`
  2. 在`aff == NO_AFF`时将thread加入本CPU的等待队列，否则加入到`aff`所对应的CPU中



> 练习题 13：在`userland/servers/procm/launch.c`中填写`launch_process`函数中缺少的代码。

1. 利用`__chcore_sys_create_pmo`函数创建pmo，返回`main_stack_cap`

2. `stack_top`的值为`MAIN_THREAD_STACK_BASE + MAIN_THREAD_STACK_SIZE`

   `offset`的值为`MAIN_THREAD_STACK_SIZE - PAGE_SIZE`(因为最上方的一个page存储了一些数据)

3. 填充参数`pmo_map_requests[0]`，对应于内核栈的物理内存对象

4. `arg.stack`的值为`stack_top - PAGE_SIZE`



> 练习题 14：在`libchcore/src/ipc/ipc.c`与`kernel/ipc/connection.c`中实现了大多数IPC相关的代码，请根据注释完成其余代码。

- `create_connection`

  1. 根据传入参数给定的`stack_base_addr`和`conn_idx`来唯一确定`server_stack_base`
  2. 同上方法确定`server_buf_base`，`client_buf_base`使用传入参数即可
  3. 利用`vmspace_map_range`函数将共享内存分别映射到client和server的虚拟地址空间

- `thread_migrate_to_server`

  1. 服务线程的stack地址为`conn->server_stack_top`
  2. 服务线程的ip为`callback`
  3. 分别将arg和pid作为参数传入服务线程

- `thread_migrate_to_client`

  将client线程的返回值设置为`ret_value`

- `ipc_send_cap`

  调用`cap_copy`将cap复制到server中，同时更新cap_buf

- `sys_ipc_return`

  将client线程的state设置为`TS_INTER`，budget设置为`DEFAULT_BUDGET`

- `sys_ipc_call`

  1. 若`cap_num`大于0，则调用`ipc_send_cap`传递参数
  2. arg是server中`ipc_msg`的地址，需要将client中`ipc_msg`的地址减去offset后传入

- `ipc_register_server`

  设置server中stack的大小和基地址，以及共享内存buffer的大小和基地址

- `ipc_register_client`

  设置client中共享内存buffer的大小和基地址

- `ipc_set_msg_data`

  将数据拷贝到ipc_msg中，需要注意的是data的起始地址是`ipc_msg + ipc_msg->data_offset`



> 练习题 15：ChCore在`kernel/semaphore/semaphore.h`中定义了内核信号量的结构体，并在`kernel/semaphore/semaphore.c`中提供了创建信号量`init_sem`与信号量对应syscall的处理函数。请补齐`wait_sem`操作与`signal_sem`操作。

- `wait_sem`
  1. 当资源数大于0时，直接减1即可
  2. 当资源数等于0且选择了非阻塞时，返回`-EAGAIN`
  3. 当资源数等于0且选择了阻塞时，将线程的状态设置为`TS_WAITING`，将其放入等待队列中，同时设置线程的返回值，同时增加sem的reference数量
- `signal_sem`
  1. 当没有等待线程时，增加sem->sem_count即可
  2. 当有等待线程时，不增加sem->sem_count，激活队列中第一个等待线程，将其放入调度的等待队列中



> 练习题 16：在`userland/apps/lab4/prodcons_impl.c`中实现`producer`和`consumer`。

- `producer`

  等待`empty_slot`的资源，完成producing任务后给`filled_slot`发信号

- `consumer`

  等待`filled_slot`的资源，完成consuming任务后给`empty_slot`发信号



> 练习题 17：请使用内核信号量实现阻塞互斥锁，在`userland/apps/lab4/mutex.c`中填上`lock`与`unlock`的代码。注意，这里不能使用提供的`spinlock`。

- `lock_init`

  使用系统调用`__chcore_sys_create_sem`来初始化锁，同时调用`__chcore_sys_signal_sem`将最初的资源数设置为1

- `lock`

  调用`__chcore_sys_wait_sem`

- `unlock`

  调用`__chcore_sys_signal_sem`