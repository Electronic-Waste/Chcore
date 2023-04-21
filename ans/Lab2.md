# Lab2

> 思考题 1：请思考多级页表相比单级页表带来的优势和劣势（如果有的话），并计算在 AArch64 页表中分别以 4KB 粒度和 2MB 粒度映射 0～4GB 地址范围所需的物理内存大小（或页表页数量）。

- 多级页表的优势：更加节省内存，不需要为一些未被分配的内存分配内存页
- 多级页表的劣势：相比于单级页表，查询页的地址时多了几次内存查询，性能开销较高
- 4KB粒度映射：4GB / (4KB * 512) = 2048个页表页
- 2MB粒度映射：4GB / (2MB * 512) = 4个页表页



> 练习题 2：请在 `init_boot_pt` 函数的 `LAB 2 TODO 1` 处配置内核高地址页表（`boot_ttbr1_l0`、`boot_ttbr1_l1` 和 `boot_ttbr1_l2`），以 2MB 粒度映射。

```c++
/* TTBR1_EL1 0-1G */
        /* LAB 2 TODO 1 BEGIN */
        /* Step 1: set L0 and L1 page table entry */
        vaddr = KERNEL_VADDR;
        boot_ttbr1_l0[GET_L0_INDEX(vaddr)] = ((u64) boot_ttbr1_l1) | IS_TABLE
                                             | IS_VALID | NG;
        boot_ttbr1_l1[GET_L1_INDEX(vaddr)] = ((u64) boot_ttbr1_l2) | IS_TABLE
                                             | IS_VALID | NG;

        /* Step 2: map PHYSMEM_START ~ PERIPHERAL_BASE with 2MB granularity */
        for (; vaddr < KERNEL_VADDR + PERIPHERAL_BASE; vaddr += SIZE_2M) {
                boot_ttbr1_l2[GET_L2_INDEX(vaddr)] =
                        (vaddr - KERNEL_VADDR) /* High mem, va = pa - KERNEL_VADDR */
                        | UXN /* Unprivileged execute never */
                        | ACCESSED /* Set access flag */
                        | NG /* Mark as not global */
                        | DEVICE_MEMORY /* Device memory */
                        | IS_VALID;
        }

        /* Step 2: map PERIPHERAL_BASE ~ PHYSMEM_END with 2MB granularity */
        vaddr = KERNEL_VADDR + PERIPHERAL_BASE;
        for (; vaddr < KERNEL_VADDR + PHYSMEM_END; vaddr += SIZE_2M) {
                boot_ttbr1_l2[GET_L2_INDEX(vaddr)] =
                        (vaddr - KERNEL_VADDR) /* High mem, va = pa - KERNEL_VADDR */
                        | UXN /* Unprivileged execute never */
                        | ACCESSED /* Set access flag */
                        | NG /* Mark as not global */
                        | DEVICE_MEMORY /* Device memory */
                        | IS_VALID;
        }
        
        /* LAB 2 TODO 1 END */
```

1. 首先，高地址从`KERNEL_VADDR`开始，我们需要先设置`TTBR1_EL1`中L0到L1级页表的映射以及L1级页表到L2级页表的映射

   > 其中L1级页表对应的内存页的大小为1G，刚好是Physical Memory的大小，所以我们在映射高地址的时候不需要设置L1，只需要设置L2

2. 然后我们将高地址中[`KERNEL_VADDR`, `KERNEL_VADDR` + `PERIPHERAL_BASE`]处的地址映射到物理内存中的[`PHYSMEM_START`, `PERIPHERAL_BASE`]，其中页的粒度为2M，同时需要设置PTE，如上面的代码所示

3. 接着我们将高地址中的[`KERNEL_VADDR + PERIPHERAL_BASE`, `KERNEL_VADDR + PHYSMEM_END`]处的地址映射到物理内存中的[`PERIPHERAL_BASE`, `PHYSMEM_END`]，其中页的粒度为2M，同时需要设置PTE，如上面的代码所示



> 思考题 3：请思考在 `init_boot_pt` 函数中为什么还要为低地址配置页表，并尝试验证自己的解释。

- 因为在开启MMU后，PC中的值（PC = 开启MMU的那行代码的物理地址 + 4）不再被解释为物理内存中的地址，立刻变成了虚拟地址。如果我们没有配置低地址页表，此时就会出现错误，跳转到异常向量表；而我们配置了低地址页表之后，虚拟地址中的低地址映射到物理地址中数值相等的内存区域，便不会出现错误，能顺利执行接下来的代码。

  > 我们在Lab1的实验中，GDB运行到`el1_mmu_activate`代码的第266行`msr     sctlr_el1, x8`便会卡住，最终跳转到异常向量表中，这是由于没有配置页表导致的
  >
  > 如果我们在Lab2中能接着执行代码的话，则说明我们的解释是正确的

- 结果发现在GDB能在`el1_mmu_activate`中第266行之后继续执行代码，则说明我们的解释是正确的



> 思考题 4：请解释 `ttbr0_el1` 与 `ttbr1_el1` 是具体如何被配置的，给出代码位置，并思考页表基地址配置后为何需要`ISB`指令。

* 我们在`mmu.c`中初始化了一些和页表相关的全局变量（比如`boot_ttbr0_l0`和`boot_ttbr1_l0`），我们在`tool.S`中将其读出来读到x8中，再利用`msr`指令将分别写入`ttbr0_el1`中以及`ttbr1_el1`中

- 代码的位置在`tool.S`文件的第246~250行

  ```c
  /* Write ttbr with phys addr of the translation table */
  	adrp    x8, boot_ttbr0_l0
  	msr     ttbr0_el1, x8
  	adrp    x8, boot_ttbr1_l0
  	msr     ttbr1_el1, x8
  	isb
  ```

- `isb`指令的作用是**指令同步隔离指令**，在该指令执行完成之前，后面的指令不会得到执行。在页表基地址配置后需要`ISB`是因为这些汇编指令并不是完全顺序执行的，若在配置好页表基地址之前执行了开启MMU的代码，则程序会出现错误，所以我们需要`isb`指令来确保指令执行的顺序



> 练习题 5：完成 `kernel/mm/buddy.c` 中的 `split_page`、`buddy_get_pages`、`merge_page` 和 `buddy_free_pages` 函数中的 `LAB 2 TODO 2` 部分，其中 `buddy_get_pages` 用于分配指定阶大小的连续物理页，`buddy_free_pages` 用于释放已分配的连续物理页。

1. `split_page`的主体是一个while循环，当`page->order == order`时循环结束返回，在其它情况时将这个page一分为二并且更新两个page的order，维护free_lists等数据结构
2. `buddy_get_pages`的主体也是一个while循环，需要在free_lists里面从order开始逐级向上寻找空闲的page，找到之后调用`split_page`进行分割，更新相关数据结构后返回分割后的页
3. `merge_page`的主体是一个while循环，首先获取page的buddy，如果这个buddy为NULL，或已经被分配，或与page的order不匹配，或者page的order>=BUDDY_MAX_ORDER-1，则无法合并，直接返回当前的page；否则，则进行合并操作，更新page的order，并维护相关的数据结构，进入下一次循环
4. `buddy_free_pages`首先将目标page的allocated位设置为0，再调用`merge_page`去尝试将这个page与它的buddy进行合并，最后再维护相关的数据结构



> 练习题 6：完成 `kernel/arch/aarch64/mm/page_table.c` 中的 `get_next_ptp`、 `query_in_pgtbl`、`map_range_in_pgtbl`、`unmap_range_in_pgtbl` 函数中的 `LAB 2 TODO 3` 部分，后三个函数分别实现页表查询、映射、取消映射操作，其中映射和取消映射以 4KB 页为粒度。

1. `query_in_pgtbl`的主体是一个while循环，调用`get_next_ptp`寻找下一个page table page，根据返回值的情况进行处理：
   1. 若返回值大小为`-ENOMAPPING`，则返回`-ENOMAPPING`（va is not mapped)
   2. 若level值为3或者返回值大小为`BLOCK_PTP`，则说明对应的pte指向的是一个物理页，根据level值确定offset和对应的物理页基地址，返回0
   3. 若返回值大小为`NORMAL_PTP`，则将`cur_ptp`更新为`next_ptp`，增加`level`值
2. `get_next_ptp`中待完成部分的逻辑是，通过`get_pages`得出下一个ptp，并且将其对应地址PAGE_SIZE大小内的空间置0，设置`is_valid`和`is_table`位为1，`next_table_addr`为物理地址右移12位的地址
3. `map_range_in_pgtbl`主体是一个for循环，以4KB为粒度增加cursor，调用`get_next_ptp`到L3页表，获取对应的entry，并设置`l3_page`的`is_page`为1，`is_valid`为1，`pfn`为物理地址右移12位的地址
4. `unmap_range_in_pgtbl`主体是一个for循环，以4KB为粒度增加cursor，调用`get_next_ptp`到L3页表，获取对应的entry，将`l3_page`的`is_valid`位置为0



> 练习题 7：完成 `kernel/arch/aarch64/mm/page_table.c` 中的 `map_range_in_pgtbl_huge` 和 `unmap_range_in_pgtbl_huge` 函数中的 `LAB 2 TODO 4` 部分，实现大页（2MB、1GB 页）支持

1. `map_range_in_pgtbl_huge`主体为三个while语句，第一个语句处理1GB页的映射，第二个语句处理2MB页的映射，第三个语句处理4KB页的映射（调用`map_range_in_pgtbl`），逻辑大致都与之前实现的`map_range_in_pgtbl`类似
2. `unmap_range_in_pgtbl_huge`主体为三个while语句，第一个语句处理1GB页的unmap，第二个语句处理2MB页的unmap，第三个语句处理4KB页的unmap（调用`unmap_range_in_pgtbl`），逻辑大致都与之前实现的`unmap_range_in_pgtbl`类似



> 思考题 8：阅读 Arm Architecture Reference Manual，思考要在操作系统中支持写时拷贝（Copy-on-Write，CoW）[^cow]需要配置页表描述符的哪个/哪些字段，并在发生缺页异常（实际上是 permission fault）时如何处理。

- 需要配置页表描述符的`AP`属性，将PTE中的AP设置成仅允许读取(read-only)
- 在缺页异常时，我们在buddy system中再分配一块内存，同时将对应的PTE中的地址更新成该新分配的内存块的地址，同时将`AP`字段设置成允许读写(read/write)



> 思考题 9：为了简单起见，在 ChCore 实验中没有为内核页表使用细粒度的映射，而是直接沿用了启动时的粗粒度页表，请思考这样做有什么问题。

- ChCore中大部分的文件大小没有2MB，这意味着会在分配的内存页中，会有大量的internel fragment，内存的利用率较低



> 挑战题 10：使用前面实现的 `page_table.c` 中的函数，在内核启动后重新配置内核页表，进行细粒度的映射。