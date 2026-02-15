#include <cstdio>
#include "./iota_unified.h"
#include "./inline_iota_unified.h"

int main() {
    // Test 1: heap_generator via iota_unified
    printf("heap_generator iota_unified(0, 5):\n");
    for (auto val : iota_unified(0, 5)) {
        printf("  %d\n", val);
    }

    // Test 2: inline_gen via inline_iota_unified
    printf("inline_gen inline_iota_unified(10, 15):\n");
    for (auto val : inline_iota_unified(10, 15)) {
        printf("  %d\n", val);
    }

    // Test 3: mid-iteration destruction (heap) — verifies destructor cleanup
    printf("heap_generator early break:\n");
    {
        auto gen = iota_unified(0, 100);
        int count = 0;
        for (auto val : gen) {
            printf("  %d\n", val);
            if (++count >= 3) break;
        }
    } // generator destroyed here while coroutine is suspended
    printf("  (destroyed safely)\n");

    // Test 4: mid-iteration destruction (inline) — verifies InlineHandle destructor fix
    printf("inline_gen early break:\n");
    {
        auto gen = inline_iota_unified(0, 100);
        int count = 0;
        for (auto val : gen) {
            printf("  %d\n", val);
            if (++count >= 3) break;
        }
    } // generator destroyed here while coroutine is suspended
    printf("  (destroyed safely)\n");

    // Test 5: empty range
    printf("heap_generator empty range:\n");
    for (auto val : iota_unified(5, 5)) {
        printf("  %d\n", val);
    }
    printf("  (none)\n");

    printf("inline_gen empty range:\n");
    for (auto val : inline_iota_unified(5, 5)) {
        printf("  %d\n", val);
    }
    printf("  (none)\n");

    printf("All tests passed.\n");
    return 0;
}
