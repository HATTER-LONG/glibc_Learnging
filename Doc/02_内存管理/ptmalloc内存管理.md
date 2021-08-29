# Ptmalloc 内存管理

- [Ptmalloc 内存管理](#ptmalloc-内存管理)
  - [前言](#前言)
  - [如何调试 Glibc](#如何调试-glibc)
    - [Arch / manjaro](#arch--manjaro)
    - [Ubuntu](#ubuntu)
    - [编译 debug 版本的 glibc](#编译-debug-版本的-glibc)
  - [进程内存布局](#进程内存布局)

## 前言

本文主要介绍 Glibc 默认的内存分配器 Ptmalloc 的相关实现细节，希望可以帮助你建立印象中的内存模型，在问题调试、内存优化中不再过于束手无策。

> 文中使用的 Glibc 版本为 [2.33（Released 2021-02-01）](https://www.gnu.org/software/libc/)

## 如何调试 Glibc

大多数情况下，当我们在使用系统默认的编译工具 gcc、clang 或者一些交叉编译工具时，所链接的 glibc 库都是不包含调试信息的，当出现问题时往往无法看出具体出现问题的代码甚至连行号都没有，例如 [Arch Linux Debugging/Getting traces wiki](https://wiki.archlinux.org/title/Debugging/Getting_traces) 举例：

```text
[...]
Backtrace was generated from '/usr/bin/epiphany'

(no debugging symbols found)
Using host libthread_db library "/lib/libthread_db.so.1".
(no debugging symbols found)
[Thread debugging using libthread_db enabled]
[New Thread -1241265952 (LWP 12630)]
(no debugging symbols found)
0xb7f25410 in __kernel_vsyscall ()
#0  0xb7f25410 in __kernel_vsyscall ()
#1  0xb741b45b in ?? () from /lib/libpthread.so.0
[...]
```

本节介绍我主要使用的系统 manjaro(类 Arch) 与 Ubuntu 如何为 glibc 添加调试信息，以及如何自己编译一个带有 debug 信息的 glibc 库（推荐）。

### Arch / manjaro

manjaro 同样是使用 Arch linux 的 AUR 进行包管理，因此本节内容主要以 [Arch wiki](https://wiki.archlinux.org/title/Debugging/Getting_traces) 为主。

使用如下[脚本](https://gist.github.com/nbulischeck/bda4397a59b77822703f98f6aeb2cb20)可以进行安装：

```shell
#!/bin/bash

# Install Dependencies
sudo pacman -S git svn gd lib32-gcc-libs patch make bison fakeroot

# Checkout glibc source
svn checkout --depth=empty svn://svn.archlinux.org/packages
cd packages
svn update glibc
cd glibc/repos/core-x86_64

# Add current locale to locale.gen.txt
grep -v "#" /etc/locale.gen >> locale.gen.txt

# Enable debug build in PKGBUILD
sed -i 's#!strip#debug#' PKGBUILD

# Build glibc and glibc-debug packages
makepkg --skipchecksums

# Install glibc-debug
sudo pacman -U *.pkg.tar.xz

sed '/^OPTIONS/ s/!debug/debug/g; /^OPTIONS/ s/strip/!strip/g' /etc/makepkg.conf
```

使用gdb 查看线程堆栈，可以看到函数名称现实正常：

```text
pwndbg> bt
#0  allocThreadMem () at /root/glibc_Learnging/test/main.cpp:12

................

#19 0x00007f0039556c27 in __pthread_once_slow () from /usr/lib/libpthread.so.0

................

#37 0x00007f003954f259 in start_thread () from /usr/lib/libpthread.so.0
#38 0x00007f00391035e3 in clone () from /usr/lib/libc.so.6

```

### Ubuntu

Ubuntu 下可以通过 apt 下载当先系统对应 glibc 相关的[调试编译信息](https://cloud.tencent.com/developer/article/1116156)：

```shell
> sudo apt-get install libc-dbg
```

接下来同样使用 apt 下载相应版本的源码：

```shell
> sudo apt-get install source libc-dev
```

最后可以使用 gdb 命令进行源码路径指定：

```shell
gdb> set substitute-path  /build/glibc-eX1tMB/glibc-2.31 /home/layton/Tools/glibc-2.31

#/build/glibc-eX1tMB/glibc-2.31 路径便是安装 libc-dbg 中指定的源码编译路径
```

### 编译 debug 版本的 glibc

通过系统工具安装带有 debug 信息的 glibc 往往有一个缺点，就是由于编译时一般都是开启了优化选项，一些局部变量甚至代码流程发生了变化，因此最推荐的就是自己编译一份带有 debug 信息的 glibc 库：

1. 确认 glibc 版本，下载对应的源代码。
   - 推荐下载的 glibc 源码版本与当前系统使用的一致，避免链接时存在不兼容问题，可以查看 ldd 工具版本来判断使用的 glibc 版本：

      ```shell
      ❯ ldd --version
      ldd (GNU libc) 2.33
      Copyright (C) 2021 自由软件基金会。
      这是一个自由软件；请见源代码的授权条款。本软件不含任何没有担保；甚至不保证适销性
      或者适合某些特殊目的。
      由 Roland McGrath 和 Ulrich Drepper 编写。
      ```

   - 在 [GNU 官网](https://www.gnu.org/software/libc/)下载对应的 glibc 源码，这里我下载的就是 2.33 版本。

2. 解压并创建编译目录，如下与源码目录同级即可：

    ```shell
    ❯ ls glibc-* -d                 
    glibc-2.33  glibc-debug
    ```

3. 进入编译目录进行编译安装，注意指定 --enable-debug=yes 使能调试：
   > -O0 编译会失败，由于 glibc 中使用了内敛相关语法，gcc 编译器 -O1 才会开启内联优化。
   > 编译时出现缺少 xdr_* 报错失败注意使能 --enable-obsolete-rpc --enable-obsolete-nsl

   ```shell
   ❯ cd glibc-debug

   ❯ ../glibc-2.33/configure --prefix=`pwd`/usr --enable-debug=yes CFLAGS="-O1 -g" CPPFLAGS="-O1 -g"

   ❯ mkdir usr && make all && make install 
   ```

4. 工程中使用 CMakeLists.txt 指定 link_dir：

    ```cmake
    link_directories(/home/layton/Tools/glibc-debug/usr/lib)
    ```

5. 编译完成后使用 ldd 命令查看动态库;

    ```shell
    ~/WorkSpace/GLibcDebug/build/test main*
    ❯ ldd automatedtools_unittests              
            linux-vdso.so.1 (0x00007ffdc1d4d000)
            libpthread.so.0 => /home/layton/Tools/glibc-debug/usr/lib/libpthread.so.0 (0x00007fa69e86f000)
            libspdlog.so.1 => /usr/lib/libspdlog.so.1 (0x00007fa69e7dd000)
            libfmt.so.7 => /usr/lib/libfmt.so.7 (0x00007fa69e7a6000)
            libc.so.6 => /home/layton/Tools/glibc-debug/usr/lib/libc.so.6 (0x00007fa69e5f5000)
            /lib64/ld-linux-x86-64.so.2 => /usr/lib64/ld-linux-x86-64.so.2 (0x00007fa69e896000)
            libstdc++.so.6 => /usr/lib/libstdc++.so.6 (0x00007fa69e3df000)
            libgcc_s.so.1 => /usr/lib/libgcc_s.so.1 (0x00007fa69e3c2000)
            libm.so.6 => /home/layton/Tools/glibc-debug/usr/lib/libm.so.6 (0x00007fa69e28a000)
    ```

6. 最后使用 gdb 启动程序，可以正常使用 pwndbg 插件打印内存信息以及 thread_arena ：

    ![01](./total/01.png)

## 进程内存布局

