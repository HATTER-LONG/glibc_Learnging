# Ptmalloc 内存管理实现

- [Ptmalloc 内存管理实现](#ptmalloc-内存管理实现)
  - [分配内存](#分配内存)
    - [分配内存之计算 Chunk](#分配内存之计算-chunk)
    - [分配内存之 Tcache](#分配内存之-tcache)

## 分配内存

### 分配内存之计算 Chunk

- [C 标准库函数 宏定义浅析](https://www.zybuluo.com/yiltoncent/note/87733)
- [堆溢出学习笔记 (linux)](https://my.oschina.net/u/4396372/blog/3913130)

经过上文讨论已知 ptmalloc 是以 chunk 为单位进行内存管理，因此内存分配的第一步就是计算 chunk：

```cpp

/* MALLOC_ALIGNMENT is the minimum alignment for malloc'ed chunks.  It
   must be a power of two at least 2 * SIZE_SZ, even on machines for
   which smaller alignments would suffice. It may be defined as larger
   than this though. Note however that code and data structures are
   optimized for the case of 8-byte alignment.  */
#define MALLOC_ALIGNMENT (2 * SIZE_SZ < __alignof__ (long double) \
                        ? __alignof__ (long double) : 2 * SIZE_SZ)

/* The corresponding bit mask value.  */
#define MALLOC_ALIGN_MASK (MALLOC_ALIGNMENT - 1)

/* The smallest possible chunk */
#define MIN_CHUNK_SIZE        (offsetof(struct malloc_chunk, fd_nextsize))

/* The smallest size we can malloc is an aligned minimal chunk */
#define MINSIZE  \
  (unsigned long)(((MIN_CHUNK_SIZE+MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK))

#ifndef INTERNAL_SIZE_T
# define INTERNAL_SIZE_T size_t
#endif

/* The corresponding word size.  */
#define SIZE_SZ (sizeof (INTERNAL_SIZE_T))

#define request2size(req)                                         \
  (((req) + SIZE_SZ + MALLOC_ALIGN_MASK < MINSIZE)  ?             \
   MINSIZE :                                                      \
   ((req) + SIZE_SZ + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK)

/* Check if REQ overflows when padded and aligned and if the resulting value
   is less than PTRDIFF_T.  Returns TRUE and the requested size or MINSIZE in
   case the value is less than MINSIZE on SZ or false if any of the previous
   check fail.  */
static inline bool
checked_request2size (size_t req, size_t *sz) __nonnull (1)
{
  if (__glibc_unlikely (req > PTRDIFF_MAX))
    return false;
  *sz = request2size (req);
  return true;
}
```

> 注意 `__nonnull` 表示入参不应为 null，如果显示传入了 null，编译期会报警告。

首先判断请求大小是否溢出，调用这次的主角 `request2size` 计算需要分配的 chunk 大小，计算的方式用户申请的长度（req) + Chunk 头（SIZE_SZ），一般 SIZE_SZ 为 size_t 及 32 位 8 字节、64 位 16 字节。

当一个 chunk 为空闲时，至少要有 prev_size、size、fd 和 bk 四个参数，因此 MINSIZE 就代表了这四个参数需要占用的内存大小；而当一个 chunk 被使用时，prev_size 可能会被前一个 chunk 用来存储，fd 和 bk 也会被当作内存存储数据，因此当 chunk 被使用时，只剩下了 size 参数需要设置，request2size 中的 SIZE_SZ 就是 INTERNAL_SIZE_T 类型的大小，因此至少需要 req + SIZE_SZ 的内存大小。MALLOC_ALIGN_MASK 用来对齐，因此 request2size 就计算出了所需的 chunk 的大小。

以 64 位系统申请 24 字节堆块为例，24 + chunk 头 = 40 （101000），接下来加上 MALLOC_ALIGN_MASK 01111 （102111）接下来 MALLOC_ALIGN_MASK 取反 10000，按位与的结果就是 100000 了，即 32。最终算出需要的 chunk 大小。//TODO: 更好的总结

### 分配内存之 Tcache

- [tcache 机制](https://nocbtm.github.io/2020/02/27/tcache%E6%9C%BA%E5%88%B6/#%E7%9B%B8%E5%85%B3%E6%95%B0%E6%8D%AE%E7%BB%93%E6%9E%84)

在Glibc的2.26中 新增了Tcache机制，这是ptmalloc2的缓存机制，glibc 中默认是开启的：

```cpp
#if USE_TCACHE
  /* int_free also calls request2size, be careful to not pad twice.  */
  size_t tbytes;
  if (!checked_request2size (bytes, &tbytes)) // tbytes 为 bytes请求的 转换后得到的 chunk 的 size
    {
      __set_errno (ENOMEM);
      return NULL;
    }
  size_t tc_idx = csize2tidx (tbytes); // 根据大小 tbytes ， 找到 tcache->entries 索引

  MAYBE_INIT_TCACHE ();

  DIAG_PUSH_NEEDS_COMMENT;
  if (tc_idx < mp_.tcache_bins
      && tcache
      && tcache->counts[tc_idx] > 0) // 如果 tcache->entries[tc_idx] 有 chunk ，就返回
    {
      return tcache_get (tc_idx); // 调用 tcache_get 拿到 chunk 然后返回
    }
  DIAG_POP_NEEDS_COMMENT;
#endif
```
