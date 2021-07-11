#include "allocMacro.h"

#include <cstdlib>
#include <cstring>

void printAllocInfo(size_t allocSize)
{
    printf("Alloc size is %lu\n", allocSize);

    auto csize = request2size(allocSize);
    printf("request2size(%lu) = %lu\n", allocSize, csize);

    auto idex = csize2tidx(csize);
    auto csizeT = (csize)-MINSIZE + MALLOC_ALIGNMENT - 1;
    printf("csize2tidx(%lu) = %lu, csizeT is %lu\n", allocSize, idex, csizeT);

    printf("usize2tidx(%lu) = %lu\n", allocSize, usize2tidx(allocSize));

    auto usize = tidx2usize(idex);
    printf("tidx2usize(%lu) = %lu\n", idex, usize);
}

void tcacheAlloc()
{
    const size_t tcacheCommonDiff = MALLOC_ALIGNMENT;
    const size_t tcacheBinsCount = TCACHE_MAX_BINS + 1;   // +1 测试超过 tcache 最大值后内存分配的位置
    const size_t maxBinListLength = 8;
    const char* longString2test = "1234567890123456789012345678901234567890";

    void* ptrArray[tcacheBinsCount][maxBinListLength] = {
        {
            nullptr,
        },
    };

    for (size_t i = 0; i < tcacheBinsCount; i++)
    {
        auto allocSize = tcacheCommonDiff * (i + 1);
        printf("-------[%lu]--------\n", i);
        printAllocInfo(allocSize);
        for (size_t j = 0; j < maxBinListLength; j++)
        {
            ptrArray[i][j] = malloc(allocSize);
            printf("malloc address is %p\n", ptrArray[i][j]);
        }
    }

    // strncpy(static_cast<char*>(ptrArray[0][0]), longString2test, strlen(longString2test));
    for (size_t i = 0; i < tcacheBinsCount; i++)
    {
        for (size_t j = 0; j < maxBinListLength; j++)
        {
            if (ptrArray[i][j] != nullptr)
            {
                free(ptrArray[i][j]);
            }
        }
    }
}
/*
-------[0]--------
Alloc size is 16
request2size(16) = 32
csize2tidx(16) = 0, csizeT is 15
usize2tidx(16) = 0
tidx2usize(0) = 24
malloc address is 0x55555556b2c0
malloc address is 0x55555556b2e0
-------[1]--------
Alloc size is 32
request2size(32) = 48
csize2tidx(32) = 1, csizeT is 31
usize2tidx(32) = 1
tidx2usize(1) = 40
malloc address is 0x55555556b300
malloc address is 0x55555556b330
-------[2]--------
Alloc size is 48
request2size(48) = 64
csize2tidx(48) = 2, csizeT is 47
usize2tidx(48) = 2
tidx2usize(2) = 56
malloc address is 0x55555556b360
malloc address is 0x55555556b3a0
-------[3]--------
Alloc size is 64
request2size(64) = 80
csize2tidx(64) = 3, csizeT is 63
usize2tidx(64) = 3
tidx2usize(3) = 72
malloc address is 0x55555556b3e0
malloc address is 0x55555556b430
-------[4]--------
Alloc size is 80
request2size(80) = 96
csize2tidx(80) = 4, csizeT is 79
usize2tidx(80) = 4
tidx2usize(4) = 88
malloc address is 0x55555556b480
malloc address is 0x55555556b4e0
-------[5]--------
Alloc size is 96
request2size(96) = 112
csize2tidx(96) = 5, csizeT is 95
usize2tidx(96) = 5
tidx2usize(5) = 104
malloc address is 0x55555556b540
malloc address is 0x55555556b5b0
-------[6]--------
Alloc size is 112
request2size(112) = 128
csize2tidx(112) = 6, csizeT is 111
usize2tidx(112) = 6
tidx2usize(6) = 120
malloc address is 0x55555556b620
malloc address is 0x55555556b6a0
-------[7]--------
Alloc size is 128
request2size(128) = 144
csize2tidx(128) = 7, csizeT is 127
usize2tidx(128) = 7
tidx2usize(7) = 136
malloc address is 0x55555556b720
malloc address is 0x55555556b7b0
-------[8]--------
Alloc size is 144
request2size(144) = 160
csize2tidx(144) = 8, csizeT is 143
usize2tidx(144) = 8
tidx2usize(8) = 152
malloc address is 0x55555556b840
malloc address is 0x55555556b8e0
-------[9]--------
Alloc size is 160
request2size(160) = 176
csize2tidx(160) = 9, csizeT is 159
usize2tidx(160) = 9
tidx2usize(9) = 168
malloc address is 0x55555556b980
malloc address is 0x55555556ba30
-------[10]--------
Alloc size is 176
request2size(176) = 192
csize2tidx(176) = 10, csizeT is 175
usize2tidx(176) = 10
tidx2usize(10) = 184
malloc address is 0x55555556bae0
malloc address is 0x55555556bba0
-------[11]--------
Alloc size is 192
request2size(192) = 208
csize2tidx(192) = 11, csizeT is 191
usize2tidx(192) = 11
tidx2usize(11) = 200
malloc address is 0x55555556bc60
malloc address is 0x55555556bd30
-------[12]--------
Alloc size is 208
request2size(208) = 224
csize2tidx(208) = 12, csizeT is 207
usize2tidx(208) = 12
tidx2usize(12) = 216
malloc address is 0x55555556be00
malloc address is 0x55555556bee0
-------[13]--------
Alloc size is 224
request2size(224) = 240
csize2tidx(224) = 13, csizeT is 223
usize2tidx(224) = 13
tidx2usize(13) = 232
malloc address is 0x55555556bfc0
malloc address is 0x55555556c0b0
-------[14]--------
Alloc size is 240
request2size(240) = 256
csize2tidx(240) = 14, csizeT is 239
usize2tidx(240) = 14
tidx2usize(14) = 248
malloc address is 0x55555556c1a0
malloc address is 0x55555556c2a0
-------[15]--------
Alloc size is 256
request2size(256) = 272
csize2tidx(256) = 15, csizeT is 255
usize2tidx(256) = 15
tidx2usize(15) = 264
malloc address is 0x55555556c3a0
malloc address is 0x55555556c4b0
-------[16]--------
Alloc size is 272
request2size(272) = 288
csize2tidx(272) = 16, csizeT is 271
usize2tidx(272) = 16
tidx2usize(16) = 280
malloc address is 0x55555556c5c0
malloc address is 0x55555556c6e0
-------[17]--------
Alloc size is 288
request2size(288) = 304
csize2tidx(288) = 17, csizeT is 287
usize2tidx(288) = 17
tidx2usize(17) = 296
malloc address is 0x55555556c800
malloc address is 0x55555556c930
-------[18]--------
Alloc size is 304
request2size(304) = 320
csize2tidx(304) = 18, csizeT is 303
usize2tidx(304) = 18
tidx2usize(18) = 312
malloc address is 0x55555556ca60
malloc address is 0x55555556cba0
-------[19]--------
Alloc size is 320
request2size(320) = 336
csize2tidx(320) = 19, csizeT is 319
usize2tidx(320) = 19
tidx2usize(19) = 328
malloc address is 0x55555556cce0
malloc address is 0x55555556ce30
-------[20]--------
Alloc size is 336
request2size(336) = 352
csize2tidx(336) = 20, csizeT is 335
usize2tidx(336) = 20
tidx2usize(20) = 344
malloc address is 0x55555556cf80
malloc address is 0x55555556d0e0
-------[21]--------
Alloc size is 352
request2size(352) = 368
csize2tidx(352) = 21, csizeT is 351
usize2tidx(352) = 21
tidx2usize(21) = 360
malloc address is 0x55555556d240
malloc address is 0x55555556d3b0
-------[22]--------
Alloc size is 368
request2size(368) = 384
csize2tidx(368) = 22, csizeT is 367
usize2tidx(368) = 22
tidx2usize(22) = 376
malloc address is 0x55555556d520
malloc address is 0x55555556d6a0
-------[23]--------
Alloc size is 384
request2size(384) = 400
csize2tidx(384) = 23, csizeT is 383
usize2tidx(384) = 23
tidx2usize(23) = 392
malloc address is 0x55555556d820
malloc address is 0x55555556d9b0
-------[24]--------
Alloc size is 400
request2size(400) = 416
csize2tidx(400) = 24, csizeT is 399
usize2tidx(400) = 24
tidx2usize(24) = 408
malloc address is 0x55555556db40
malloc address is 0x55555556dce0
-------[25]--------
Alloc size is 416
request2size(416) = 432
csize2tidx(416) = 25, csizeT is 415
usize2tidx(416) = 25
tidx2usize(25) = 424
malloc address is 0x55555556de80
malloc address is 0x55555556e030
-------[26]--------
Alloc size is 432
request2size(432) = 448
csize2tidx(432) = 26, csizeT is 431
usize2tidx(432) = 26
tidx2usize(26) = 440
malloc address is 0x55555556e1e0
malloc address is 0x55555556e3a0
-------[27]--------
Alloc size is 448
request2size(448) = 464
csize2tidx(448) = 27, csizeT is 447
usize2tidx(448) = 27
tidx2usize(27) = 456
malloc address is 0x55555556e560
malloc address is 0x55555556e730
-------[28]--------
Alloc size is 464
request2size(464) = 480
csize2tidx(464) = 28, csizeT is 463
usize2tidx(464) = 28
tidx2usize(28) = 472
malloc address is 0x55555556e900
malloc address is 0x55555556eae0
-------[29]--------
Alloc size is 480
request2size(480) = 496
csize2tidx(480) = 29, csizeT is 479
usize2tidx(480) = 29
tidx2usize(29) = 488
malloc address is 0x55555556ecc0
malloc address is 0x55555556eeb0
-------[30]--------
Alloc size is 496
request2size(496) = 512
csize2tidx(496) = 30, csizeT is 495
usize2tidx(496) = 30
tidx2usize(30) = 504
malloc address is 0x55555556f0a0
malloc address is 0x55555556f2a0
-------[31]--------
Alloc size is 512
request2size(512) = 528
csize2tidx(512) = 31, csizeT is 511
usize2tidx(512) = 31
tidx2usize(31) = 520
malloc address is 0x55555556f4a0
malloc address is 0x55555556f6b0
-------[32]--------
Alloc size is 528
request2size(528) = 544
csize2tidx(528) = 32, csizeT is 527
usize2tidx(528) = 32
tidx2usize(32) = 536
malloc address is 0x55555556f8c0
malloc address is 0x55555556fae0
-------[33]--------
Alloc size is 544
request2size(544) = 560
csize2tidx(544) = 33, csizeT is 543
usize2tidx(544) = 33
tidx2usize(33) = 552
malloc address is 0x55555556fd00
malloc address is 0x55555556ff30
-------[34]--------
Alloc size is 560
request2size(560) = 576
csize2tidx(560) = 34, csizeT is 559
usize2tidx(560) = 34
tidx2usize(34) = 568
malloc address is 0x555555570160
malloc address is 0x5555555703a0
-------[35]--------
Alloc size is 576
request2size(576) = 592
csize2tidx(576) = 35, csizeT is 575
usize2tidx(576) = 35
tidx2usize(35) = 584
malloc address is 0x5555555705e0
malloc address is 0x555555570830
-------[36]--------
Alloc size is 592
request2size(592) = 608
csize2tidx(592) = 36, csizeT is 591
usize2tidx(592) = 36
tidx2usize(36) = 600
malloc address is 0x555555570a80
malloc address is 0x555555570ce0
-------[37]--------
Alloc size is 608
request2size(608) = 624
csize2tidx(608) = 37, csizeT is 607
usize2tidx(608) = 37
tidx2usize(37) = 616
malloc address is 0x555555570f40
malloc address is 0x5555555711b0
-------[38]--------
Alloc size is 624
request2size(624) = 640
csize2tidx(624) = 38, csizeT is 623
usize2tidx(624) = 38
tidx2usize(38) = 632
malloc address is 0x555555571420
malloc address is 0x5555555716a0
-------[39]--------
Alloc size is 640
request2size(640) = 656
csize2tidx(640) = 39, csizeT is 639
usize2tidx(640) = 39
tidx2usize(39) = 648
malloc address is 0x555555571920
malloc address is 0x555555571bb0
-------[40]--------
Alloc size is 656
request2size(656) = 672
csize2tidx(656) = 40, csizeT is 655
usize2tidx(656) = 40
tidx2usize(40) = 664
malloc address is 0x555555571e40
malloc address is 0x5555555720e0
-------[41]--------
Alloc size is 672
request2size(672) = 688
csize2tidx(672) = 41, csizeT is 671
usize2tidx(672) = 41
tidx2usize(41) = 680
malloc address is 0x555555572380
malloc address is 0x555555572630
-------[42]--------
Alloc size is 688
request2size(688) = 704
csize2tidx(688) = 42, csizeT is 687
usize2tidx(688) = 42
tidx2usize(42) = 696
malloc address is 0x5555555728e0
malloc address is 0x555555572ba0
-------[43]--------
Alloc size is 704
request2size(704) = 720
csize2tidx(704) = 43, csizeT is 703
usize2tidx(704) = 43
tidx2usize(43) = 712
malloc address is 0x555555572e60
malloc address is 0x555555573130
-------[44]--------
Alloc size is 720
request2size(720) = 736
csize2tidx(720) = 44, csizeT is 719
usize2tidx(720) = 44
tidx2usize(44) = 728
malloc address is 0x555555573400
malloc address is 0x5555555736e0
-------[45]--------
Alloc size is 736
request2size(736) = 752
csize2tidx(736) = 45, csizeT is 735
usize2tidx(736) = 45
tidx2usize(45) = 744
malloc address is 0x5555555739c0
malloc address is 0x555555573cb0
-------[46]--------
Alloc size is 752
request2size(752) = 768
csize2tidx(752) = 46, csizeT is 751
usize2tidx(752) = 46
tidx2usize(46) = 760
malloc address is 0x555555573fa0
malloc address is 0x5555555742a0
-------[47]--------
Alloc size is 768
request2size(768) = 784
csize2tidx(768) = 47, csizeT is 767
usize2tidx(768) = 47
tidx2usize(47) = 776
malloc address is 0x5555555745a0
malloc address is 0x5555555748b0
-------[48]--------
Alloc size is 784
request2size(784) = 800
csize2tidx(784) = 48, csizeT is 783
usize2tidx(784) = 48
tidx2usize(48) = 792
malloc address is 0x555555574bc0
malloc address is 0x555555574ee0
-------[49]--------
Alloc size is 800
request2size(800) = 816
csize2tidx(800) = 49, csizeT is 799
usize2tidx(800) = 49
tidx2usize(49) = 808
malloc address is 0x555555575200
malloc address is 0x555555575530
-------[50]--------
Alloc size is 816
request2size(816) = 832
csize2tidx(816) = 50, csizeT is 815
usize2tidx(816) = 50
tidx2usize(50) = 824
malloc address is 0x555555575860
malloc address is 0x555555575ba0
-------[51]--------
Alloc size is 832
request2size(832) = 848
csize2tidx(832) = 51, csizeT is 831
usize2tidx(832) = 51
tidx2usize(51) = 840
malloc address is 0x555555575ee0
malloc address is 0x555555576230
-------[52]--------
Alloc size is 848
request2size(848) = 864
csize2tidx(848) = 52, csizeT is 847
usize2tidx(848) = 52
tidx2usize(52) = 856
malloc address is 0x555555576580
malloc address is 0x5555555768e0
-------[53]--------
Alloc size is 864
request2size(864) = 880
csize2tidx(864) = 53, csizeT is 863
usize2tidx(864) = 53
tidx2usize(53) = 872
malloc address is 0x555555576c40
malloc address is 0x555555576fb0
-------[54]--------
Alloc size is 880
request2size(880) = 896
csize2tidx(880) = 54, csizeT is 879
usize2tidx(880) = 54
tidx2usize(54) = 888
malloc address is 0x555555577320
malloc address is 0x5555555776a0
-------[55]--------
Alloc size is 896
request2size(896) = 912
csize2tidx(896) = 55, csizeT is 895
usize2tidx(896) = 55
tidx2usize(55) = 904
malloc address is 0x555555577a20
malloc address is 0x555555577db0
-------[56]--------
Alloc size is 912
request2size(912) = 928
csize2tidx(912) = 56, csizeT is 911
usize2tidx(912) = 56
tidx2usize(56) = 920
malloc address is 0x555555578140
malloc address is 0x5555555784e0
-------[57]--------
Alloc size is 928
request2size(928) = 944
csize2tidx(928) = 57, csizeT is 927
usize2tidx(928) = 57
tidx2usize(57) = 936
malloc address is 0x555555578880
malloc address is 0x555555578c30
-------[58]--------
Alloc size is 944
request2size(944) = 960
csize2tidx(944) = 58, csizeT is 943
usize2tidx(944) = 58
tidx2usize(58) = 952
malloc address is 0x555555578fe0
malloc address is 0x5555555793a0
-------[59]--------
Alloc size is 960
request2size(960) = 976
csize2tidx(960) = 59, csizeT is 959
usize2tidx(960) = 59
tidx2usize(59) = 968
malloc address is 0x555555579760
malloc address is 0x555555579b30
-------[60]--------
Alloc size is 976
request2size(976) = 992
csize2tidx(976) = 60, csizeT is 975
usize2tidx(976) = 60
tidx2usize(60) = 984
malloc address is 0x555555579f00
malloc address is 0x55555557a2e0
-------[61]--------
Alloc size is 992
request2size(992) = 1008
csize2tidx(992) = 61, csizeT is 991
usize2tidx(992) = 61
tidx2usize(61) = 1000
malloc address is 0x55555557a6c0
malloc address is 0x55555557aab0
-------[62]--------
Alloc size is 1008
request2size(1008) = 1024
csize2tidx(1008) = 62, csizeT is 1007
usize2tidx(1008) = 62
tidx2usize(62) = 1016
malloc address is 0x55555557aea0
malloc address is 0x55555557b2a0
-------[63]--------
Alloc size is 1024
request2size(1024) = 1040
csize2tidx(1024) = 63, csizeT is 1023
usize2tidx(1024) = 63
tidx2usize(63) = 1032
malloc address is 0x55555557b6a0
malloc address is 0x55555557bab0
-------[64]--------
Alloc size is 1040
request2size(1040) = 1056
csize2tidx(1040) = 64, csizeT is 1039
usize2tidx(1040) = 64
tidx2usize(64) = 1048
malloc address is 0x55555557bec0
malloc address is 0x55555557c2e0


------------------------------------

pwndbg> heapinfo
(0x20)     fastbin[0]: 0x0
(0x30)     fastbin[1]: 0x0
(0x40)     fastbin[2]: 0x0
(0x50)     fastbin[3]: 0x0
(0x60)     fastbin[4]: 0x0
(0x70)     fastbin[5]: 0x0
(0x80)     fastbin[6]: 0x0
(0x90)     fastbin[7]: 0x0
(0xa0)     fastbin[8]: 0x0
(0xb0)     fastbin[9]: 0x0
                  top: 0x55555557beb0 (size : 0x1f150)
       last_remainder: 0x0 (size : 0x0)
            unsortbin: 0x0
(0x20)   tcache_entry[0](2): 0x55555556b2e0 --> 0x55555556b2c0
(0x30)   tcache_entry[1](2): 0x55555556b330 --> 0x55555556b300
(0x40)   tcache_entry[2](2): 0x55555556b3a0 --> 0x55555556b360
(0x50)   tcache_entry[3](2): 0x55555556b430 --> 0x55555556b3e0
(0x60)   tcache_entry[4](2): 0x55555556b4e0 --> 0x55555556b480
(0x70)   tcache_entry[5](2): 0x55555556b5b0 --> 0x55555556b540
(0x80)   tcache_entry[6](2): 0x55555556b6a0 --> 0x55555556b620
(0x90)   tcache_entry[7](2): 0x55555556b7b0 --> 0x55555556b720
(0xa0)   tcache_entry[8](2): 0x55555556b8e0 --> 0x55555556b840
(0xb0)   tcache_entry[9](2): 0x55555556ba30 --> 0x55555556b980
(0xc0)   tcache_entry[10](2): 0x55555556bba0 --> 0x55555556bae0
(0xd0)   tcache_entry[11](2): 0x55555556bd30 --> 0x55555556bc60
(0xe0)   tcache_entry[12](2): 0x55555556bee0 --> 0x55555556be00
(0xf0)   tcache_entry[13](2): 0x55555556c0b0 --> 0x55555556bfc0
(0x100)   tcache_entry[14](2): 0x55555556c2a0 --> 0x55555556c1a0
(0x110)   tcache_entry[15](2): 0x55555556c4b0 --> 0x55555556c3a0
(0x120)   tcache_entry[16](2): 0x55555556c6e0 --> 0x55555556c5c0
(0x130)   tcache_entry[17](2): 0x55555556c930 --> 0x55555556c800
(0x140)   tcache_entry[18](2): 0x55555556cba0 --> 0x55555556ca60
(0x150)   tcache_entry[19](2): 0x55555556ce30 --> 0x55555556cce0
(0x160)   tcache_entry[20](2): 0x55555556d0e0 --> 0x55555556cf80
(0x170)   tcache_entry[21](2): 0x55555556d3b0 --> 0x55555556d240
(0x180)   tcache_entry[22](2): 0x55555556d6a0 --> 0x55555556d520
(0x190)   tcache_entry[23](2): 0x55555556d9b0 --> 0x55555556d820
(0x1a0)   tcache_entry[24](2): 0x55555556dce0 --> 0x55555556db40
(0x1b0)   tcache_entry[25](2): 0x55555556e030 --> 0x55555556de80
(0x1c0)   tcache_entry[26](2): 0x55555556e3a0 --> 0x55555556e1e0
(0x1d0)   tcache_entry[27](2): 0x55555556e730 --> 0x55555556e560
(0x1e0)   tcache_entry[28](2): 0x55555556eae0 --> 0x55555556e900
(0x1f0)   tcache_entry[29](2): 0x55555556eeb0 --> 0x55555556ecc0
(0x200)   tcache_entry[30](2): 0x55555556f2a0 --> 0x55555556f0a0
(0x210)   tcache_entry[31](2): 0x55555556f6b0 --> 0x55555556f4a0
(0x220)   tcache_entry[32](2): 0x55555556fae0 --> 0x55555556f8c0
(0x230)   tcache_entry[33](2): 0x55555556ff30 --> 0x55555556fd00
(0x240)   tcache_entry[34](2): 0x5555555703a0 --> 0x555555570160
(0x250)   tcache_entry[35](2): 0x555555570830 --> 0x5555555705e0
(0x260)   tcache_entry[36](2): 0x555555570ce0 --> 0x555555570a80
(0x270)   tcache_entry[37](2): 0x5555555711b0 --> 0x555555570f40
(0x280)   tcache_entry[38](2): 0x5555555716a0 --> 0x555555571420
(0x290)   tcache_entry[39](2): 0x555555571bb0 --> 0x555555571920
(0x2a0)   tcache_entry[40](2): 0x5555555720e0 --> 0x555555571e40
(0x2b0)   tcache_entry[41](2): 0x555555572630 --> 0x555555572380
(0x2c0)   tcache_entry[42](2): 0x555555572ba0 --> 0x5555555728e0
(0x2d0)   tcache_entry[43](2): 0x555555573130 --> 0x555555572e60
(0x2e0)   tcache_entry[44](2): 0x5555555736e0 --> 0x555555573400
(0x2f0)   tcache_entry[45](2): 0x555555573cb0 --> 0x5555555739c0
(0x300)   tcache_entry[46](2): 0x5555555742a0 --> 0x555555573fa0
(0x310)   tcache_entry[47](2): 0x5555555748b0 --> 0x5555555745a0
(0x320)   tcache_entry[48](2): 0x555555574ee0 --> 0x555555574bc0
(0x330)   tcache_entry[49](2): 0x555555575530 --> 0x555555575200
(0x340)   tcache_entry[50](2): 0x555555575ba0 --> 0x555555575860
(0x350)   tcache_entry[51](2): 0x555555576230 --> 0x555555575ee0
(0x360)   tcache_entry[52](2): 0x5555555768e0 --> 0x555555576580
(0x370)   tcache_entry[53](2): 0x555555576fb0 --> 0x555555576c40
(0x380)   tcache_entry[54](2): 0x5555555776a0 --> 0x555555577320
(0x390)   tcache_entry[55](2): 0x555555577db0 --> 0x555555577a20
(0x3a0)   tcache_entry[56](2): 0x5555555784e0 --> 0x555555578140
(0x3b0)   tcache_entry[57](2): 0x555555578c30 --> 0x555555578880
(0x3c0)   tcache_entry[58](2): 0x5555555793a0 --> 0x555555578fe0
(0x3d0)   tcache_entry[59](2): 0x555555579b30 --> 0x555555579760
(0x3e0)   tcache_entry[60](2): 0x55555557a2e0 --> 0x555555579f00
(0x3f0)   tcache_entry[61](2): 0x55555557aab0 --> 0x55555557a6c0
(0x400)   tcache_entry[62](2): 0x55555557b2a0 --> 0x55555557aea0
(0x410)   tcache_entry[63](2): 0x55555557bab0 --> 0x55555557b6a0
*/