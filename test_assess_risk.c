#define UNIT_TEST
#define _GNU_SOURCE
#include "bos_omega_production.c"
#include <assert.h>
#include <stdio.h>

void test_financial() {
    printf("Testing Financial Risk...\n");
    assert(assess_risk("buy some stocks") == R_FINANCIAL);
    assert(assess_risk("pay the bill") == R_FINANCIAL);
    assert(assess_risk("spend money") == R_FINANCIAL);
    assert(assess_risk("BUY NOW") == R_FINANCIAL);
}

void test_destructive() {
    printf("Testing Destructive Risk...\n");
    assert(assess_risk("delete all files") == R_DESTRUCTIVE);
    assert(assess_risk("uninstall app") == R_DESTRUCTIVE);
    assert(assess_risk("rm -rf /") == R_DESTRUCTIVE);
    assert(assess_risk("wipe disk") == R_DESTRUCTIVE);
    assert(assess_risk("Remove-Item path") == R_DESTRUCTIVE);
    assert(assess_risk("Set-Content file") == R_DESTRUCTIVE);
    assert(assess_risk("DELETE EVERYTHING") == R_DESTRUCTIVE);
}

void test_external_api() {
    printf("Testing External API Risk...\n");
    assert(assess_risk("push to origin") == R_EXTERNAL_API);
    assert(assess_risk("commit changes") == R_EXTERNAL_API);
    assert(assess_risk("call the api") == R_EXTERNAL_API);
    assert(assess_risk("PUSH it") == R_EXTERNAL_API);
}

void test_local_write() {
    printf("Testing Local Write Risk...\n");
    assert(assess_risk("write to log") == R_LOCAL_WRITE);
    assert(assess_risk("create a file") == R_LOCAL_WRITE);
    assert(assess_risk("save progress") == R_LOCAL_WRITE);
    assert(assess_risk("WRITE text") == R_LOCAL_WRITE);
}

void test_read_only() {
    printf("Testing Read Only Risk...\n");
    assert(assess_risk("list files") == R_READ_ONLY);
    assert(assess_risk("read document") == R_READ_ONLY);
    assert(assess_risk("check status") == R_READ_ONLY);
}

void test_edge_cases() {
    printf("Testing Edge Cases...\n");
    assert(assess_risk("") == R_READ_ONLY);
    assert(assess_risk(NULL) == R_READ_ONLY);
}

int main() {
    test_financial();
    test_destructive();
    test_external_api();
    test_local_write();
    test_read_only();
    test_edge_cases();
    printf("ALL TESTS PASSED!\n");
    return 0;
}
