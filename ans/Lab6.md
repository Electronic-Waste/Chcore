# Lab6

> 思考题 1：请自行查阅资料，并阅读`userland/servers/sd`中的代码，回答以下问题:
>
> - circle中还提供了SDHost的代码。SD卡，EMMC和SDHost三者之间的关系是怎么样的？
> - 请**详细**描述Chcore是如何与SD卡进行交互的？即Chcore发出的指令是如何输送到SD卡上，又是如何得到SD卡的响应的。(提示: IO设备常使用MMIO的方式映射到内存空间当中)
> - 请简要介绍一下SD卡驱动的初始化流程。
> - 在驱动代码的初始化当中，设置时钟频率的意义是什么？为什么需要调用`TimeoutWait`进行等待?

- SD卡、EMMC和SDHost三者之间的关系是怎么样的？

  SD卡是存储介质；EMMC是SD卡内部的管理器，相当于驱动，负责数据的读取、写入、擦除等操作；SD Host是主机设备中的控制器，与EMMC交互，负责主机与SD卡之间的数据传输和交互

- Chcore是如何与SD卡进行交互的？

  1. Chcore首先初始化SD卡驱动，将SD卡设备映射到内存上，注册IPC server进程提供服务
  2. Chcore向SD卡存储服务进程发送IPC指令
  3. SD存储服务进程在收到指令后判断属于read还是write指令，通过mmio对SD卡进行操作
  4. 调用`ipc_return`向Chcore返回结果

- SD卡驱动的初始化流程

  1. 将EMMC设备映射到内存中
  2. 初始化SD卡的配置，打开SD卡的电源
  3. 重置SD卡中的信息

- 在驱动代码的初始化当中，设置时钟频率的意义是什么？为什么需要调用`TimeoutWait`进行等待?

  - 设置时钟频率的意义是什么？

    时钟频率确定了数据传输的速率，通过设置合适的时钟频率可以确保主机设备和外部设备（如存储设备或外设）之间的数据传输同步

  - 为什么需要调用`TimeoutWait`进行等待？

    因为对外部设备寄存器的修改需要较长的时间，调用`TimeoutWait`进行等待可以更好地确定外部设备寄存器按照逾期情况被设置



> 练习 1：完成`userland/servers/sd`中的代码，实现SD卡驱动。驱动程序需实现为用户态系统服务，应用程序与驱动通过 IPC 通信。需要实现 `sdcard_readblock` 与 `sdcard_writeblock` 接口，通过 Logical Block Address(LBA) 作为参数访问 SD 卡的块。

- `emmc.c`中的代码：参照circle中对emmc的实现，删去LED灯以及其它的无关操作

- `sdcard_readblock`与`sdcard_writeblock`：

  根据参数`lba`调用`Seek`函数以设置emmc中的offset参数，然后各自调用`sd_Read`/`sd_Write`



> 练习 2：实现naive_fs。
>
> 你需要在 userland/apps/lab6/naive_fs/file_ops.[ch] 中按下述规范实现接口：
>
> - naive_fs_access，判断参数文件名对应的文件是否存在，不存在返回-1，存在返回0；
> - naive_fs_creat，创建一个空文件，如果文件已经存在则返回-1，创建成功返回0；
> - naive_fs_unlink，删除一个已有文件，如果文件不存在则返回-1，删除成功返回0；
> - naive_fs_pread，根据偏移量和大小读取文件内容，特殊情况的处理请参考 pread 接口的 Linux Manual Page；
> - naive_fs_pwrite，根据偏移量和大小写入文件内容，特殊情况的处理请参考 pwrite 接口的 Linux Manual Page。

> 采用经典的Unix inode设计方式，第0块block存储bitmap，第1块block存储根目录下的dentry信息，第2-63块block存储inode信息，第64块block之后存储数据块信息

- `naive_fs_access`

  1. 调用`sd_bread`读取根目录下的dentry信息
  2. 在dentry中匹配，如果有相应name的dentry则返回0，若无则返回-1

  

- `naive_fs_creat`

  1. 调用`naive_fs_access`检测文件是否已经存在，若是则立即返回-1
  2. 调用`sd_bread`读取根目录下的dentry信息以及bitmap信息
  3. 在bitmap中找到一个空的inode，分配给即将创建的文件
  4. 将新的dentry(“filename-inode_num”对)写入到根目录下
  5. 调用`sd_bwrite`将相关数据结构的更新写回sd卡

  

- `naive_fs_unlink`

  1. 调用`naive_fs_access`检测文件是否存在，若不存在则返回-1
  2. 调用`sd_bread`读取根目录下的dentry信息以及bitmap信息
  3. 删去相应的dentry，清空inode和数据块，将bitmap中相应的位标为0
  4. 调用`sd_bwrite`将相关数据结构的更新写回sd卡

  

- `naive_fs_pread`

  1. 调用`naive_fs_access`检测文件是否存在，若不存在则返回-1
  2. 调用`sd_bread`读取根目录下的dentry信息
  3. 在dentry中根据name进行匹配，获取相应的inode_num
  4. 根据相应的inode_num获取inode信息，进一步获取数据块的信息
  5. 根据offset以及size确定访问的数据块，拷贝相关数据到buffer中，并返回

  

- `naive_fs_pwrite`

  1. 调用`naive_fs_access`检测文件是否存在，若不存在则返回-1
  2. 调用`sd_bread`读取根目录下的dentry信息以及bitmap信息
  3. 在dentry中根据name进行匹配，获取相应的inode_num
  4. 根据相应的inode_num获取inode信息，进一步获取数据块的信息
  5. 根据offset以及size确定访问的数据块，如果没有则分配新的数据块，更新bitmap
  6. 将数据拷贝到数据块中
  7. 调用`sd_bwrite`将相关数据结构的更新写回sd卡，并返回



> 思考题2：查阅资料了解 SD 卡是如何进行分区，又是如何识别分区对应的文件系统的？尝试设计方案为 ChCore 提供多分区的 SD 卡驱动支持，设计其解析与挂载流程。本题的设计部分请在实验报告中详细描述，如果有代码实现，可以编写对应的测试程序放入仓库中提交。

- SD卡如何进行分区：

  1. 创建分区表：在SD卡上创建一个分区表，例如MBR，它记录了每个分区的起始位置、大小和文件系统类型等信息
  2. 分区创建：在分区表中定义一个或多个分区
  3. 格式化：格式化每个分区，创建文件系统的结构和元数据，以准备文件系统，

  

- SD卡如何识别对应的文件系统：

  1. 读取分区表：读取SD卡上的分区表，获取每个分区的信息，包括分区的起始位置和大小
  2. 根据路由识别挂载点：根据相应的路径在分区表中检索挂载点，找到相应的分区
  3. 识别文件系统：读取分区的文件系统元数据，识别分区对应的文件系统类型



- Chcore多分区的SD卡驱动支持
  - 解析流程：
    1. 根据对应的路由进行前缀匹配，找到相应的分区
    2. 读取相应分区的信息，加载相应数据结构到内存中
    3. 裁剪路由，通过相应的驱动调用相应分区文件系统的操作
  - 挂载流程：
    1. 在分区表中创建分区，分配存储空间，记录分区起始位置和大小
    2. 将分区挂载到文件系统树中的一个挂载点，将路径信息写入分区表中
    3. 格式化分区，创建相应文件系统的结构和元数据