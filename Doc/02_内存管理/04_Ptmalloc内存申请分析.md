# Ptmalloc 内存申请

- [Ptmalloc 内存申请](#ptmalloc-内存申请)
  - [内存申请](#内存申请)
    - [内存分配之单线程](#内存分配之单线程)

## 内存申请

前几篇文章介绍了 Ptmalloc 初始化、Tcache 缓存，本文将继续 malloc 源码的分析：

```cpp
void *
__libc_malloc (size_t bytes)
{
  mstate ar_ptr;
  void *victim;

    ...........

  if (SINGLE_THREAD_P)
    {
      victim = _int_malloc (&main_arena, bytes);
      assert (!victim || chunk_is_mmapped (mem2chunk (victim)) ||
          &main_arena == arena_for_chunk (mem2chunk (victim)));
      return victim;
    }

  arena_get (ar_ptr, bytes);

  victim = _int_malloc (ar_ptr, bytes);
  /* Retry with another arena only if we were able to find a usable arena
     before.  */
  if (!victim && ar_ptr != NULL)
    {
      LIBC_PROBE (memory_malloc_retry, 1, bytes);
      ar_ptr = arena_get_retry (ar_ptr, bytes);
      victim = _int_malloc (ar_ptr, bytes);
    }

  if (ar_ptr != NULL)
    __libc_lock_unlock (ar_ptr->mutex);

  assert (!victim || chunk_is_mmapped (mem2chunk (victim)) ||
          ar_ptr == arena_for_chunk (mem2chunk (victim)));
  return victim;
}
```

### 内存分配之单线程

- [参考文章--堆漏洞挖掘中的 bins 分类](https://blog.csdn.net/qq_41453285/article/details/96865321)

```cpp
  if (SINGLE_THREAD_P) // 判断进程是否为单线程
    {
      victim = _int_malloc (&main_arena, bytes);
      assert (!victim || chunk_is_mmapped (mem2chunk (victim)) ||
          &main_arena == arena_for_chunk (mem2chunk (victim)));
      return victim;
    }
```

`SINGLE_THREAD_P` 判断当前进程是否为单线程，如果为真则调用 `_int_malloc` 方法并传入 main_arena 主分配区进行内存内存，这个接口也是 malloc 的核心代码：

> main_arena 主分配区在 Ptmalloc 初始化分析中介绍过，当进程中第一次调用 malloc 申请内存的线程会进行 ptmalloc 初始化，会构建一个主分配区。

1. _int_malloc 入口部分代码，定义了一些局部变量：

    ```cpp
    static void *
    _int_malloc (mstate av, size_t bytes)
    {
        INTERNAL_SIZE_T nb;               /* normalized request size */
        unsigned int idx;                 /* associated bin index */
        mbinptr bin;                      /* associated bin */

        mchunkptr victim;                 /* inspected/selected chunk */
        INTERNAL_SIZE_T size;             /* its size */
        int victim_index;                 /* its bin index */

        mchunkptr remainder;              /* remainder from a split */
        unsigned long remainder_size;     /* its size */

        unsigned int block;               /* bit map traverser */
        unsigned int bit;                 /* bit map traverser */
        unsigned int map;                 /* current word of binmap */

        mchunkptr fwd;                    /* misc temp for linking */
        mchunkptr bck;                    /* misc temp for linking */

        #if USE_TCACHE
        size_t tcache_unsorted_count;      /* count of unsorted chunks processed */
        #endif

        .......

    }
    ```

2. 将申请的内存字节转化为对应 chunk 且对其后的大小，使用的也是之前分析过的 `checked_request2size`，接下来判断传入的分配区是否存在，如果为空则使用 sysmalloc 创建一个分配区，并使用其申请内存。：

    ```cpp
    /*
        Convert request size to internal form by adding SIZE_SZ bytes
        overhead plus possibly more to obtain necessary alignment and/or
        to obtain a size of at least MINSIZE, the smallest allocatable
        size. Also, checked_request2size returns false for request sizes
        that are so large that they wrap around zero when padded and
        aligned.
    */

    if (!checked_request2size (bytes, &nb))
        {
        __set_errno (ENOMEM);
        return NULL;
        }

    /* There are no usable arenas.  Fall back to sysmalloc to get a chunk from
        mmap.  */
    if (__glibc_unlikely (av == NULL))
        {
        void *p = sysmalloc (nb, av); //TODO: 完善 sysmalloc 分析
        if (p != NULL)
        alloc_perturb (p, bytes);
        return p;
        }

    ```

3. 首先判断申请内存 fastbin 是否能满足，并使用 fastbin 进行分配内存：
   - chunk 的大小在 32 字节~128 字节（0x20~0x80）的chunk称为“fast chunk”，注意是指 struct malloc_chunk 结构的大小。
   - 不会对 free chunk 进行合并：鉴于设计fast bin的初衷就是进行快速的小内存分配和释放，因此系统将属于fast bin的chunk的PREV_INUSE位总是设置为1，这样即使当fast bin中有某个chunk同一个free chunk相邻的时候，系统也不会进行自动合并操作，而是保留两者。虽然这样做可能会造成额外的碎片化问题。

    ```cpp
    /*
        If the size qualifies as a fastbin, first check corresponding bin.
        This code is safe to execute even if av is not yet initialized, so we
        can try it without checking, which saves some time on this fast path.
    */
    // 多线程安全的从 fastbin 里面移除一个 chunk.
    #define REMOVE_FB(fb, victim, pp)            \
    do                            \
        {                            \
        victim = pp;                    \
        if (victim == NULL)                \
        break;                        \
        }                            \
    while ((pp = catomic_compare_and_exchange_val_acq (fb, victim->fd, victim)) \
        != victim);                    \

    if ((unsigned long) (nb) <= (unsigned long) (get_max_fast ()))
        {
        idx = fastbin_index (nb); // 判断某一个fastchunk属于哪一个fastbin链表。
        mfastbinptr *fb = &fastbin (av, idx); // 获得 bin 链表上的 chunk
        mchunkptr pp;
        victim = *fb;

        if (victim != NULL)
        {
        if (SINGLE_THREAD_P)
            *fb = victim->fd;
        else
            REMOVE_FB (fb, pp, victim);
        if (__glibc_likely (victim != NULL))
            {
            size_t victim_idx = fastbin_index (chunksize (victim));
            if (__builtin_expect (victim_idx != idx, 0))
            malloc_printerr ("malloc(): memory corruption (fast)");
            check_remalloced_chunk (av, victim, nb);
    #if USE_TCACHE
            /* While we're here, if we see other chunks of the same size,
            stash them in the tcache.  */
            size_t tc_idx = csize2tidx (nb);
            if (tcache && tc_idx < mp_.tcache_bins)
            {
            mchunkptr tc_victim;

            /* While bin not empty and tcache not full, copy chunks.  */
            while (tcache->counts[tc_idx] < mp_.tcache_count
                && (tc_victim = *fb) != NULL)
                {
                if (SINGLE_THREAD_P)
                *fb = tc_victim->fd;
                else
                {
                REMOVE_FB (fb, pp, tc_victim);
                if (__glibc_unlikely (tc_victim == NULL))
                    break;
                }
                tcache_put (tc_victim, tc_idx);
                }
            }
    #endif
            void *p = chunk2mem (victim);
            alloc_perturb (p, bytes);
            return p;
            }
        }
    ```