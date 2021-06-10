# Ptmalloc

- [Ptmalloc](#ptmalloc)
  - [`_int_malloc` 之 `large bin`](#_int_malloc-之-large-bin)

## `_int_malloc` 之 `large bin`

- [参考文章--浅析 largebin attack](https://xz.aliyun.com/t/5177)
- [参考文章--largebin 学习从源码到做题](https://xz.aliyun.com/t/6596)

前文已经介绍了 `small bin` 以及当其无法满足需求时，需要使用 `large bin`，并且会进行 `fast bin` 的内存合并相关操作，接下来开始分析 `_int_malloc` 中 `large bin` 的分配流程。

1. 首先判断 tcache 是否存在当前申请的内存块，初始化一些局部变量。

    ```cpp

    #if USE_TCACHE
    INTERNAL_SIZE_T tcache_nb = 0;
    size_t tc_idx = csize2tidx (nb);
    if (tcache && tc_idx < mp_.tcache_bins)
        tcache_nb = nb;
    int return_cached = 0;

    tcache_unsorted_count = 0;
    #endif
    ```

2. 两个循环，最外层用于无法满足需求用于合并重试，里层遍历所有 unsorted 节点：

    ```cpp
    //这里需要外循环，因为可能直到 malloc 结束时才意识到应该合并，所以必须这样做并重试。这最多发生一次，并且仅在需要扩展内存以服务“小”请求时才发生。
    for (;; ) 
    
        {
        int iters = 0;
        while ((victim = unsorted_chunks (av)->bk) != unsorted_chunks (av)) // 判断 unsorted 不为空
            {
            bck = victim->bk; //取出下一个空闲的 chunk
            size = chunksize (victim);
            mchunkptr next = chunk_at_offset (victim, size);  // 偏移到再下一个 chunk 头 用于检查链表是否正常

            if (__glibc_unlikely (size <= 2 * SIZE_SZ)
                || __glibc_unlikely (size > av->system_mem))
                malloc_printerr ("malloc(): invalid size (unsorted)");
            if (__glibc_unlikely (chunksize_nomask (next) < 2 * SIZE_SZ)
                || __glibc_unlikely (chunksize_nomask (next) > av->system_mem))
                malloc_printerr ("malloc(): invalid next size (unsorted)");
            if (__glibc_unlikely ((prev_size (next) & ~(SIZE_BITS)) != size))
                malloc_printerr ("malloc(): mismatching next->prev_size (unsorted)");
            if (__glibc_unlikely (bck->fd != victim)
                || __glibc_unlikely (victim->fd != unsorted_chunks (av)))
                malloc_printerr ("malloc(): unsorted double linked list corrupted");
            if (__glibc_unlikely (prev_inuse (next)))
                malloc_printerr ("malloc(): invalid next->prev_inuse (unsorted)");
            
                .......
                .......
            }
        }
    ```

3. 经过检查后进入到内存分配流程，当小内存请求，且 unsorted 中仅剩余一个 chunk 时，使用 last_remiainder 进行切割分配：

    ```cpp
          /*
             If a small request, try to use last remainder if it is the
             only chunk in unsorted bin.  This helps promote locality for
             runs of consecutive small requests. This is the only
             exception to best-fit, and applies only when there is
             no exact fit for a small chunk.
             如果是小请求，则尝试使用最后一个余数只有未排序 bin 中的块。 
             这有助于促进本地化运行连续的小请求。 
             这是唯一最佳拟合的例外，仅适用于不完全合适一小块内存。
           */

          if (in_smallbin_range (nb) &&
              bck == unsorted_chunks (av) && //当剩余 1 个未使用的 chunk
              victim == av->last_remainder && // 指向上一个 chunk 分配出一个 small chunk 之后剩余的部分
              (unsigned long) (size) > (unsigned long) (nb + MINSIZE))
            {
              /* split and reattach remainder */
              remainder_size = size - nb; //减去所需的内存
              remainder = chunk_at_offset (victim, nb);
              unsorted_chunks (av)->bk = unsorted_chunks (av)->fd = remainder;
              av->last_remainder = remainder; //更新
              remainder->bk = remainder->fd = unsorted_chunks (av); //将删减过剩余的内存重新添加回链表
              if (!in_smallbin_range (remainder_size))
                {
                  remainder->fd_nextsize = NULL;
                  remainder->bk_nextsize = NULL;
                }

              set_head (victim, nb | PREV_INUSE |
                        (av != &main_arena ? NON_MAIN_ARENA : 0));
              set_head (remainder, remainder_size | PREV_INUSE);
              set_foot (remainder, remainder_size);

              check_malloced_chunk (av, victim, nb);
              void *p = chunk2mem (victim);
              alloc_perturb (p, bytes);
              return p;
            }
    ```

4. 将 victim 从 unsorted list 中移除，

    ```cpp
          /* remove from unsorted list */
          if (__glibc_unlikely (bck->fd != victim))
            malloc_printerr ("malloc(): corrupted unsorted chunks 3");
          unsorted_chunks (av)->bk = bck;
          bck->fd = unsorted_chunks (av);

          /* Take now instead of binning if exact fit */

          if (size == nb) //如果当前 chunk 的大小符合申请
            {
                set_inuse_bit_at_offset (victim, size); //设置当前块为使用状态
                if (av != &main_arena) //非主分区设置标志位
                set_non_main_arena (victim);
        #if USE_TCACHE
                /* Fill cache first, return to user only if cache fills.
                We may return one of these chunks later.  
                先填充缓存，缓存满才返回给用户。我们稍后可能会返回这些块之一。*/
                if (tcache_nb
                && tcache->counts[tc_idx] < mp_.tcache_count)
                {
                    tcache_put (victim, tc_idx);
                    return_cached = 1;
                    continue;
                }
                else
                {
        #endif
                    check_malloced_chunk (av, victim, nb);
                    void *p = chunk2mem (victim); // 返回用户内存地址
                    alloc_perturb (p, bytes);
                    return p;
        #if USE_TCACHE
                }
        #endif
            }
    ```

