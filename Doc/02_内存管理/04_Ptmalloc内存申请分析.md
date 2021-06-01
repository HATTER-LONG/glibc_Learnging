# Ptmalloc 内存申请

- [Ptmalloc 内存申请](#ptmalloc-内存申请)
  - [内存申请](#内存申请)
    - [内存分配之单线程](#内存分配之单线程)
    - [内存分配之 `_int_malloc`](#内存分配之-_int_malloc)

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
- [参考文章--Linux 堆内存管理深入分析](https://www.cnblogs.com/alisecurity/p/5520847.html)

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

### 内存分配之 `_int_malloc`

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
   - chunk 的大小在 32 字节~128 字节（0x20~0x80）的 chunk 称为 “fast chunk”，注意是指 struct malloc_chunk 结构的大小。
   - 不会对 free chunk 进行合并：鉴于设计 fast bin 的初衷就是进行快速的小内存分配和释放，因此系统将属于 fast bin 的 chunk 的 PREV_INUSE 位总是设置为 1，这样即使当 fast bin 中有某个 chunk 同一个 free chunk 相邻的时候，系统也不会进行自动合并操作，而是保留两者。虽然这样做可能会造成额外的碎片化问题。

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
        idx = fastbin_index (nb); // 判断某一个 fastchunk 属于哪一个 fastbin 链表。
        mfastbinptr *fb = &fastbin (av, idx); // 获得 bin 链表上的 chunk
        mchunkptr pp;
        victim = *fb;

        if (victim != NULL)
        {
        if (SINGLE_THREAD_P)
            *fb = victim->fd; // 如果为单线程则直接删除对应 chunk 节点即可
        else
            REMOVE_FB (fb, pp, victim); // 多线程需要使用 原子操作 进行删除
        if (__glibc_likely (victim != NULL))
            {
            size_t victim_idx = fastbin_index (chunksize (victim)); // 检查分配的 chunk 内存
            if (__builtin_expect (victim_idx != idx, 0))
            malloc_printerr ("malloc(): memory corruption (fast)");
            check_remalloced_chunk (av, victim, nb);
    #if USE_TCACHE
            /* While we're here, if we see other chunks of the same size,
            stash them in the tcache.  
            发现其他相同大小的块，将它们存放在 tcache 中，直到 tcache 对应链长度表达到 tcache_count = 7
            */
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
            void *p = chunk2mem (victim); // 定位到 chunk 中用户内存地址
            alloc_perturb (p, bytes); //对用户使用的内存进行初始化
            return p;
            }
        }
    ```

    - `REMOVE_FB` 中不好理解的宏便是 `catomic_compare_and_exchange_val_acq`，其中传入的参数 `mem` 与 `oldval` 一般都是相等的，因此其作用便是将 mem 指向 newval 并返回 oldval，达成在 mem 链表中取出 oldval 的目的：

        ```cpp
        /* Atomically store NEWVAL in *MEM if *MEM is equal to OLDVAL.
           Return the old *MEM value.  
           如果 *MEM 等于 OLDVAL，则将 NEWVAL 原子地存储在 *MEM 中。
           返回旧的 *MEM 值。
        */
        #ifndef catomic_compare_and_exchange_val_acq
        # ifdef __arch_c_compare_and_exchange_val_32_acq
        #  define catomic_compare_and_exchange_val_acq(mem, newval, oldval) \
        __atomic_val_bysize (__arch_c_compare_and_exchange_val,acq,         \
                    mem, newval, oldval)
        # else
        #  define catomic_compare_and_exchange_val_acq(mem, newval, oldval) \
        atomic_compare_and_exchange_val_acq (mem, newval, oldval)
        # endif
        #endif
        ```

    - `fastbin_index` 与 `fastbin` 可以看出其组合使用取出目标大小的 chunk 链表节点，`fastbin_index` 判断 64 未系统则除以 8 再减去 2，64 位系统举例： sz 为 32 时，32 >> 4 = 2 -2 = 0; 得出 fastbin 数字的对应 chunk 链表下标，因此可以反推 fastbin 是 32 字节开始，最大的请求为 160 字节（注意这并不是指 fastbin 中 chunk 的大小，其是通过 set_max_fast 设置，后文会分析）：

        ```cpp
        typedef struct malloc_chunk *mfastbinptr;
        #define fastbin(ar_ptr, idx) ((ar_ptr)->fastbinsY[idx])

        /* offset 2 to use otherwise unindexable first 2 bins */
        #define fastbin_index(sz) \
        ((((unsigned int) (sz)) >> (SIZE_SZ == 8 ? 4 : 3)) - 2)

        /* The maximum fastbin request size we support */
        #define MAX_FAST_SIZE     (80 * SIZE_SZ / 4)

        #define NFASTBINS  (fastbin_index (request2size (MAX_FAST_SIZE)) + 1)
        struct malloc_state
        {
            ......
            /* Fastbins */
            mfastbinptr fastbinsY[NFASTBINS];
            .......
        };
        ```

    - 当获取到正确大小的 chunk 后，会有进行一个检查操作：

        ```cpp
        size_t victim_idx = fastbin_index (chunksize (victim)); // 检查分配的 chunk 内存
        if (__builtin_expect (victim_idx != idx, 0))            // 应该与申请时的下表保持一致，否则可能 chunk 内存被破坏
        malloc_printerr ("malloc(): memory corruption (fast)");
        check_remalloced_chunk (av, victim, nb);  // 这一部分用于 debug 模式

        .....

        /* size field is or'ed with PREV_INUSE when previous adjacent chunk in use */
        #define PREV_INUSE 0x1
        /* size field is or'ed with IS_MMAPPED if the chunk was obtained with mmap() */
        #define IS_MMAPPED 0x2
        /* size field is or'ed with NON_MAIN_ARENA if the chunk was obtained
        from a non-main arena.  This is only set immediately before handing
        the chunk to the user, if necessary.  */
        #define NON_MAIN_ARENA 0x4

        /*
        Bits to mask off when extracting size

        Note: IS_MMAPPED is intentionally not masked off from size field in
        macros for which mmapped chunks should never be seen. This should
        cause helpful core dumps to occur if it is tried by accident by
        people extending or adapting this malloc.
        */
        #define SIZE_BITS (PREV_INUSE | IS_MMAPPED | NON_MAIN_ARENA)

        /* Get size, ignoring use bits */
        #define chunksize(p) (chunksize_nomask (p) & ~(SIZE_BITS))
        /* Like chunksize, but do not mask SIZE_BITS.  */
        #define chunksize_nomask(p)         ((p)->mchunk_size)
        ```

    - 终于 fastbin 内存分配就分析完了，但是有一点需要注意，我们默认 `get_max_fast` 是有效的，但是从源码可以看出 `global_max_fast` 是需要设置的，那么是谁设置的？这就是个疑问了，不过暂时先记住问题，然我们继续向下分析。

        ```cpp
        /*
        Set value of max_fast.
        Use impossibly small value if 0.
        Precondition: there are no existing fastbin chunks in the main arena.
        Since do_check_malloc_state () checks this, we call malloc_consolidate ()
        before changing max_fast.  Note other arenas will leak their fast bin
        entries if max_fast is reduced.
            */

        #define set_max_fast(s) \
        global_max_fast = (((s) == 0)    \
                            ? MIN_CHUNK_SIZE / 2 : ((s + SIZE_SZ) & ~MALLOC_ALIGN_MASK))

        static inline INTERNAL_SIZE_T
        get_max_fast (void)
        {
        /* Tell the GCC optimizers that global_max_fast is never larger
            than MAX_FAST_SIZE.  This avoids out-of-bounds array accesses in
            _int_malloc after constant propagation of the size parameter.
            (The code never executes because malloc preserves the
            global_max_fast invariant, but the optimizers may not recognize
            this.)  */
        if (global_max_fast > MAX_FAST_SIZE)
            __builtin_unreachable ();
        return global_max_fast;
        }
        ```
