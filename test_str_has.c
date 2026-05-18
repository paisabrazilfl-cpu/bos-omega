#define TESTING
#include "bos_omega_production.c"
#include <assert.h>
#include <stdio.h>

void test_str_has() {
    printf("Running str_has tests...\n");

    // Null checks
    assert(str_has(NULL, "test") == false);
    assert(str_has("test", NULL) == false);
    assert(str_has(NULL, NULL) == false);

    // Empty strings
    assert(str_has("", "test") == false);
    assert(str_has("test", "") == true); // strcasestr returns haystack for empty needle

    // Case sensitivity
    assert(str_has("Hello World", "hello") == true);
    assert(str_has("Hello World", "WORLD") == true);
    assert(str_has("hello world", "HELLO") == true);

    // Matches
    assert(str_has("The quick brown fox", "quick") == true);
    assert(str_has("The quick brown fox", "FOX") == true);
    assert(str_has("The quick brown fox", "The quick brown fox") == true);

    // Non-matches
    assert(str_has("The quick brown fox", "lazy") == false);
    assert(str_has("The quick brown fox", "dog") == false);

    printf("All str_has tests passed!\n");
}

int main() {
    test_str_has();
    return 0;
}
