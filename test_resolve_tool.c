#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

typedef enum {
    T_MISTRAL, T_NVIDIA, T_RERANKER, T_RAILWAY, T_GITHUB, T_APIFY,
    T_CDP, T_PWSH, T_ADB, T_NONE
} ToolType;

static bool str_has(const char *haystack, const char *needle) {
    return haystack && needle && strcasestr(haystack, needle) != NULL;
}

static ToolType resolve_tool(const char *task) {
    if (str_has(task, "android") || str_has(task, "adb") || str_has(task, "pm ") || str_has(task, "am ") || str_has(task, "input ")) return T_ADB;
    if (str_has(task, "Get-") || str_has(task, "Set-") || str_has(task, "pwsh") || str_has(task, "powershell")) return T_PWSH;
    if (str_has(task, "browser") || str_has(task, "navigate") || str_has(task, "click")) return T_CDP;
    if (str_has(task, "image") || str_has(task, "visual")) return T_NVIDIA;
    if (str_has(task, "railway") || str_has(task, "deploy")) return T_RAILWAY;
    if (str_has(task, "github") || str_has(task, "repo")) return T_GITHUB;
    return T_MISTRAL;
}

void test_resolve_tool() {
    // T_ADB
    assert(resolve_tool("install android app") == T_ADB);
    assert(resolve_tool("adb shell ls") == T_ADB);
    assert(resolve_tool("pm list packages") == T_ADB);
    assert(resolve_tool("am start") == T_ADB);
    assert(resolve_tool("input tap 100 100") == T_ADB);

    // T_PWSH
    assert(resolve_tool("Get-Process") == T_PWSH);
    assert(resolve_tool("Set-Location") == T_PWSH);
    assert(resolve_tool("run pwsh script") == T_PWSH);
    assert(resolve_tool("open powershell") == T_PWSH);

    // T_CDP
    assert(resolve_tool("open browser") == T_CDP);
    assert(resolve_tool("navigate to google") == T_CDP);
    assert(resolve_tool("click button") == T_CDP);

    // T_NVIDIA
    assert(resolve_tool("generate image") == T_NVIDIA);
    assert(resolve_tool("visual processing") == T_NVIDIA);

    // T_RAILWAY
    assert(resolve_tool("railway up") == T_RAILWAY);
    assert(resolve_tool("deploy to cloud") == T_RAILWAY);

    // T_GITHUB
    assert(resolve_tool("github clone") == T_GITHUB);
    assert(resolve_tool("create repo") == T_GITHUB);

    // T_MISTRAL (default)
    assert(resolve_tool("hello world") == T_MISTRAL);
    assert(resolve_tool("") == T_MISTRAL);
    assert(resolve_tool(NULL) == T_MISTRAL);

    // Case Insensitivity
    assert(resolve_tool("ANDROID") == T_ADB);
    assert(resolve_tool("GET-DATE") == T_PWSH);
    assert(resolve_tool("BROWSER") == T_CDP);

    printf("All tests passed for resolve_tool!\n");
}

int main() {
    test_resolve_tool();
    return 0;
}
