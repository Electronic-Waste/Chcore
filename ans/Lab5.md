# Lab5

> 思考题 1: 文件的数据块使用基数树的形式组织有什么好处? 除此之外还有其他的数据块存储方式吗?

- 用基数树的形式组织的好处
  - 减小了inode的大小，inode存储所占用的空间更少

- 其他的数据块存储方式
  1. Unix V6文件系统：在inode中存储所有direct block和indirect block的block_id
  2. Ext4：使用了区段树的方法，保存起始块地址以及长度



> 练习题 2：实现位于`userland/servers/tmpfs/tmpfs.c`的`tfs_mknod`和`tfs_namex`。

- `tfs_mknod`
  1. 根据`mkdir`的值去判断创建文件/目录的inode
  2. 创建对应的dentry
- `tfs_namex`
  1. 用`/`分割name的各个部分
  2. 调用`tfs_lookup`来寻找dentry
  3. 更新相关变量



> 练习题 3：实现位于`userland/servers/tmpfs/tmpfs.c`的`tfs_file_read`和`tfs_file_write`。提示：由于数据块的大小为PAGE_SIZE，因此读写可能会牵涉到多个页面。读取不能超过文件大小，而写入可能会增加文件大小（也可能需要创建新的数据块）。

- `tfs_file_read`
  1. 首先判断要读取的位置`cur_off+size`是否大于inode的size，若是则更新最大读取size
  2. 在每一个data block中读取数据
- `tfs_file_write`
  1. 读取每一个data block中的数据
  2. 若`cur_off - offset`的值大于inode的size，则更新inode的size



> 练习题 4：实现位于`userland/servers/tmpfs/tmpfs.c`的`tfs_load_image`函数。需要通过之前实现的tmpfs函数进行目录和文件的创建，以及数据的读写。

1. 将`dirat`设置为`tmpfs_root`，调用`tfs_namex`来定位文件
2. 调用`tfs_lookup`来获取对应文件的dentry（若不存在则创建对应文件）
3. 调用`tfs_file_write`将对应的数据写入文件
4. 重复上述过程



> 练习题 5：利用`userland/servers/tmpfs/tmpfs.c`中已经实现的函数，完成在`userland/servers/tmpfs/tmpfs_ops.c`中的`fs_creat`、`tmpfs_unlink`和`tmpfs_mkdir`函数，从而使`tmpfs_*`函数可以被`fs_server_dispatch`调用以提供系统服务。对应关系可以参照`userland/servers/tmpfs/tmpfs_ops.c`中`server_ops`的设置以及`userland/fs_base/fs_wrapper.c`的`fs_server_dispatch`函数。

- `fs_creat`

  调用`tfs_namex`以及`tfs_creat`

- `tmpfs_unlink`

  调用`tfs_namex`以及`tfs_remove`

- `tmpfs_mkdir`

  调用`tfs_namex`以及`tfs_mkdir`



> 练习题 6：补全`libchcore/src/libc/fs.c`与`libchcore/include/libc/FILE.h`文件，以实现`fopen`, `fwrite`, `fread`, `fclose`, `fscanf`, `fprintf`五个函数，函数用法应与libc中一致。

- `fopen`
  1. 填充`fs_request`和`ipc_msg`
  2. 调用`ipc_call`发送相应`FS_REQ_OPEN`类型的`ipc_msg`
  3. 判断返回值
     1. 和`fr`的`new_fd`值相同：不做处理
     2. 若`fr`的`new_fd`值不同：
        1. 模式为"w"：发送`FS_REQ_OPEN`类型的`ipc_msg`来创建文件，然后再进行操作2
        2. 否则报错
  4. 设置FILE，返回
- `fwrite`
  1. 填充`fs_request`和`ipc_msg`
  2. 调用`ipc_call`发送相应`FS_REQ_WRITE`类型的`ipc_msg`
  3. 返回写入的字节数
- `fread`
  1. 填充`fs_request`和`ipc_msg`
  2. 调用`ipc_call`发送相应`FS_REQ_READ`类型的`ipc_msg`
  3. 返回读取的字节数
- `fclose`
  1. 填充`fs_request`和`ipc_msg`
  2. 调用`ipc_call`发送相应`FS_REQ_CLOSE`类型的`ipc_msg`
- `fscanf`
  1. 首先调用`fread`来读取文件中的内容
  2. 分`%s`,`%d`还有其他状况分别进行处理:
     1. `%s`：以字符串形式读出
     2. `%d`：以数字类型读出
     3. 其它：更新cursor位置
- `fprintf`
  1. 分`%s`,`%d`还有其他状况分别进行处理:
     1. `%s`：以字符串形式写入
     2. `%d`：以数字类型写入
     3. 其它：将原本的字符写入并更新cursor位置
  2. 最后调用`fwrite`写入文件



> 练习题 7：实现在`userland/servers/shell/main.c`中定义的`getch`，该函数会每次从标准输入中获取字符，并实现在`userland/servers/shell/shell.c`中的`readline`，该函数会将按下回车键之前的输入内容存入内存缓冲区。代码中可以使用在`libchcore/include/libc/stdio.h`中的定义的I/O函数。

- `getch`

  调用`__chcore_sys_getc`

- `readline`

  判断字符类型：

  1. 若为`\t`，调用`do_complement`进行补全，并打印
  2. 若为`\n`，退出循环停止读入，返回
  3. 其它情况则正常读入



> 练习题 8：根据在`userland/servers/shell/shell.c`中实现好的`bultin_cmd`函数，完成shell中内置命令对应的`do_*`函数，需要支持的命令包括：`ls [dir]`、`echo [string]`、`cat [filename]`和`top`。

- `print_file_content`
  1. 调用`fopen`打开对应的文件
  2. 调用`fread`读取文件内容
  3. 依次打印读出来的内容
- `fs_scan`
  1. 调用`fopen`打开相应的目录
  2. 仿照`demo_gendents`的方式进行scan并打印
- `do_echo`
  1. 首先跳过"echo"
  2. 再跳过空白字符
  3. 打印后续内容



> 练习题 9：实现在`userland/servers/shell/shell.c`中定义的`run_cmd`，以通过输入文件名来运行可执行文件，同时补全`do_complement`函数并修改`readline`函数，以支持按tab键自动补全根目录（`/`）下的文件名。

- `run_cmd`

  调用`chcore_procm_spawn`，传入`fs_server_cap`

- `do_complement`

  1. 打开根目录，调用`getdents`读取根目录下的dentry
  2. 根据`complement_time`来遍历，选中相应的文件名



> 练习题 10：FSM需要两种不同的文件系统才能体现其特点，本实验提供了一个fakefs用于模拟部分文件系统的接口，测试代码会默认将tmpfs挂载到路径`/`，并将fakefs挂载在到路径`/fakefs`。本练习需要实现`userland/server/fsm/main.c`中空缺的部分，使得用户程序将文件系统请求发送给FSM后，FSM根据访问路径向对应文件系统发起请求，并将结果返回给用户程序。实现过程中可以使用`userland/server/fsm`目录下已经实现的函数。

将参数中的`ipc_msg`拷贝至FSM专有的`ipc_msg`中去，再根据`mpinfo`调用`ipc_call`

同时需要进行一定的特殊处理:

1. `FS_REQ_OPEN`需要在获取fd后调用`fsm_set_mount_info_withfd`进行fd相关设置
2. `FS_REQ_READ`需要在完成`ipc_call`之后将读取的文件内容拷贝到参数`ipc_msg`中
3. `FS_REQ_GETDENT64`需要在完成`ipc_call`之后将读取的dentry信息拷贝回参数`ipc_msg`中

