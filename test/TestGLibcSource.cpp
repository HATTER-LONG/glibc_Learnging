#include "catch2/catch.hpp"
#include "gnu/libc-version.h"
#include "spdlog/spdlog.h"


TEST_CASE("Debug Glibc")
{
    int* intPtr = static_cast<int*>(malloc(sizeof(int)));
    *intPtr = 20;
    spdlog::info("Ptr info is {}", fmt::ptr(intPtr));
    spdlog::info("Glibc Version is {}", gnu_get_libc_version());
}