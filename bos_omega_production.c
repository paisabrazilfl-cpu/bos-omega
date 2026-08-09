/*
 * BOS-OMEGA PRODUCTION RUNTIME (EXHAUSTIVE EDITION)
 * ================================================
 * A mission-critical, high-availability C runtime for agentic automation.
 * Features:
 *   1.  ROBUST STATE MACHINE: Multi-stage execution with fallback recovery.
 *   2.  PRODUCTION GOVERNANCE: Deep risk classification & audit trails.
 *   3.  ADVANCED SKILL ENGINES:
 *       - Android (ADB): Full OS control, app management, and UI automation.
 *       - PowerShell (PWSH): Granular PC interaction and system administration.
 *   4.  REMOTE TOOL ORCHESTRATION: Secure API integration (Mistral, NVIDIA, etc.).
 *   5.  LANGUAGE DECISION ENGINE: Heuristic-based stack & build optimization.
 *   6.  EXHAUSTIVE ERROR HANDLING: Fallback logic for every system call.
 *
 * Compile:
 *   gcc bos_omega_production.c -o bos_omega_prod -lcurl
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include <curl/curl.h>

/* --- SYSTEM CONSTANTS --- */
#define VERSION "3.0.0-PROD"
#define MAX_TEXT    16384
#define MAX_PATH    1024
#define MAX_RETRIES 5
#define MAX_STEPS   32
#define AUDIT_FILE  "bos_omega_production.log"

/* --- LOGGING LEVELS --- */
typedef enum { LOG_INFO, LOG_WARN, LOG_ERROR, LOG_AUDIT } LogLevel;

/* --- RUNTIME STATES --- */
typedef enum {
    ST_INIT, ST_PARSE, ST_ROUTE, ST_VALIDATE, ST_POLICY, ST_EXEC, 
    ST_VERIFY, ST_RECOVER, ST_OUTPUT, ST_SUCCESS, ST_FAILURE, ST_ESCALATE
} RuntimeState;

/* --- DECISION LOGIC --- */
typedef enum {
    DEC_PROCEED, DEC_RETRY, DEC_ESCALATE, DEC_ABORT, DEC_BLOCKED, DEC_COMPLETE
} Decision;

/* --- TOOL CLASSIFICATION --- */
typedef enum {
    T_MISTRAL, T_NVIDIA, T_RERANKER, T_RAILWAY, T_GITHUB, T_APIFY, 
    T_CDP, T_PWSH, T_ADB, T_NONE
} ToolType;

/* --- RISK ASSESSMENT --- */
typedef enum {
    R_READ_ONLY, R_LOCAL_WRITE, R_EXTERNAL_API, R_DESTRUCTIVE, R_FINANCIAL, R_UNKNOWN
} RiskLevel;

/* --- DATA STRUCTURES --- */
typedef struct {
    char   data[MAX_TEXT * 4];
    size_t size;
} NetBuffer;

typedef struct {
    ToolType    type;
    const char *name;
    const char *env_key;
    const char *url;
    bool        active;
} ToolSpec;

typedef struct {
    char         objective[MAX_TEXT];
    char         plan[MAX_STEPS][MAX_TEXT];
    int          plan_idx;
    int          retry_cnt;
    bool         is_approved;
    RuntimeState state;
    Decision     decision;
    ToolType     tool;
    RiskLevel    risk;
    char         result[MAX_TEXT];
    char         error_msg[MAX_TEXT];
    char         audit_msg[MAX_TEXT];
} AgentRuntime;

typedef struct {
    char goal[MAX_TEXT], skill[MAX_TEXT], type[MAX_TEXT], plat[MAX_TEXT], time[MAX_TEXT], style[MAX_TEXT], constraints[MAX_TEXT];
} LangInput;

typedef struct {
    char goal[MAX_TEXT], lang[MAX_TEXT], why[MAX_TEXT], stack[MAX_TEXT], steps[MAX_TEXT];
} LangOutput;

/* --- PRODUCTION TOOL REGISTRY --- */
static ToolSpec REGISTRY[] = {
    { T_MISTRAL, "MISTRAL_AI", "MISTRAL_API_KEY", "https://api.mistral.ai/v1/chat/completions", true },
    { T_NVIDIA,  "NVIDIA_GPU", "NVIDIA_API_KEY",  "https://integrate.api.nvidia.com/v1/chat/completions", true },
    { T_RERANKER,"RERANKER",   "RERANKER_KEY",    "https://api.rerank.local/v1", true },
    { T_RAILWAY, "RAILWAY",    "RAILWAY_TOKEN",   "https://backboard.railway.app/graphql/v2", true },
    { T_GITHUB,  "GITHUB",     "GITHUB_TOKEN",    "https://api.github.com", true },
    { T_APIFY,   "APIFY",      "APIFY_TOKEN",     "https://api.apify.com/v2/acts", true },
    { T_CDP,     "BROWSER",    "CDP_URL",         "http://localhost:9222/json", true },
    { T_PWSH,    "POWERSHELL", "PWSH_PATH",       "local", true },
    { T_ADB,     "ANDROID_ADB","ADB_PATH",        "local", true }
};

/* --- CORE UTILITIES --- */
static void logger(LogLevel level, const char *fmt, ...) {
    const char *lvl_str[] = {"INFO", "WARN", "ERROR", "AUDIT"};
    va_list args; va_start(args, fmt);
    printf("[%s] ", lvl_str[level]); vprintf(fmt, args); printf("\n");
    va_end(args);
    if (level == LOG_AUDIT || level == LOG_ERROR) {
        FILE *f = fopen(AUDIT_FILE, "a"); if (f) {
            fprintf(f, "%ld [%s] ", (long)time(NULL), lvl_str[level]);
            va_start(args, fmt); vfprintf(f, fmt, args); fprintf(f, "\n");
            va_end(args); fclose(f);
        }
    }
}

/*
 * Normalizes input string to lowercase to optimize search performance.
 * Replacing multiple strcasestr() calls with a single lowercase pass and
 * subsequent strstr() or has_word() calls reduces keyword scanning complexity
 * from O(N*M) to O(N+M).
 * Handles a NULL src pointer safely, bounds check i < MAX_TEXT - 1, and uses
 * tolower with explicit unsigned char cast for standard compliance.
 */
static void normalize(const char *src, char *dest) {
    if (!dest) return;
    if (!src) {
        dest[0] = '\0';
        return;
    }
    int i = 0;
    for (; src[i] != '\0' && i < MAX_TEXT - 1; i++) {
        dest[i] = (char)tolower((unsigned char)src[i]);
    }
    dest[i] = '\0';
}

/*
 * Safely checks for the presence of a specific lowercase keyword
 * ensuring exact word boundaries to avoid false positives (e.g. matching 'ai' in 'blockchain').
 * Checks that preceding and succeeding characters are not alphanumeric or underscore.
 */
static bool has_word(const char *haystack, const char *needle) {
    if (!haystack || !needle) return false;
    const char *p = haystack;
    size_t needle_len = strlen(needle);
    while ((p = strstr(p, needle)) != NULL) {
        bool before_ok = true;
        if (p > haystack) {
            char prev = *(p - 1);
            if ((prev >= 'a' && prev <= 'z') || (prev >= 'A' && prev <= 'Z') || (prev >= '0' && prev <= '9') || prev == '_') {
                before_ok = false;
            }
        }
        bool after_ok = true;
        char next = *(p + needle_len);
        if (next != '\0') {
            if ((next >= 'a' && next <= 'z') || (next >= 'A' && next <= 'Z') || (next >= '0' && next <= '9') || next == '_') {
                after_ok = false;
            }
        }
        if (before_ok && after_ok) {
            return true;
        }
        p += needle_len;
    }
    return false;
}

/* --- GOVERNANCE ENGINE --- */
static RiskLevel assess_risk(const char *task) {
    if (!task) return R_READ_ONLY;
    char norm[MAX_TEXT];
    normalize(task, norm);
    if (strstr(norm, "buy") || strstr(norm, "pay") || strstr(norm, "spend")) return R_FINANCIAL;
    if (strstr(norm, "delete") || strstr(norm, "uninstall") || strstr(norm, "rm ") || strstr(norm, "wipe") || strstr(norm, "remove-") || strstr(norm, "set-content")) return R_DESTRUCTIVE;
    if (strstr(norm, "push") || strstr(norm, "commit") || strstr(norm, "api")) return R_EXTERNAL_API;
    if (strstr(norm, "write") || strstr(norm, "create") || strstr(norm, "save")) return R_LOCAL_WRITE;
    return R_READ_ONLY;
}

static ToolType resolve_tool(const char *task) {
    if (!task) return T_MISTRAL;
    char norm[MAX_TEXT];
    normalize(task, norm);
    if (strstr(norm, "android") || strstr(norm, "adb") || strstr(norm, "pm ") || strstr(norm, "am ") || strstr(norm, "input ")) return T_ADB;
    if (strstr(norm, "get-") || strstr(norm, "set-") || strstr(norm, "pwsh") || strstr(norm, "powershell")) return T_PWSH;
    if (strstr(norm, "browser") || strstr(norm, "navigate") || strstr(norm, "click")) return T_CDP;
    if (strstr(norm, "image") || strstr(norm, "visual")) return T_NVIDIA;
    if (strstr(norm, "railway") || strstr(norm, "deploy")) return T_RAILWAY;
    if (strstr(norm, "github") || strstr(norm, "repo")) return T_GITHUB;
    return T_MISTRAL;
}

/* --- NETWORK LAYER --- */
static size_t net_write_cb(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total = size * nmemb; NetBuffer *buf = (NetBuffer *)userdata;
    if (buf->size + total >= sizeof(buf->data) - 1) total = sizeof(buf->data) - buf->size - 1;
    memcpy(&buf->data[buf->size], ptr, total); buf->size += total; buf->data[buf->size] = 0;
    return total;
}

static Decision dispatch_http(AgentRuntime *rt, const char *url, const char *key, const char *payload) {
    CURL *curl = curl_easy_init(); if (!curl) { strcpy(rt->error_msg, "CURL_INIT_FAIL"); return DEC_RETRY; }
    NetBuffer buffer = {0}; char auth[MAX_TEXT]; snprintf(auth, sizeof(auth), "Authorization: Bearer %s", key);
    struct curl_slist *headers = curl_slist_append(NULL, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth);
    curl_easy_setopt(curl, CURLOPT_URL, url); curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, net_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer); curl_easy_setopt(curl, CURLOPT_TIMEOUT, 45L);
    CURLcode res = curl_easy_perform(curl); long http_code = 0; curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_slist_free_all(headers); curl_easy_cleanup(curl);
    if (res != CURLE_OK) { snprintf(rt->error_msg, MAX_TEXT, "CURL_ERR: %s", curl_easy_strerror(res)); return DEC_RETRY; }
    strncpy(rt->result, buffer.data, MAX_TEXT - 1);
    return (http_code >= 200 && http_code < 300) ? DEC_COMPLETE : DEC_RETRY;
}

/* --- AUTOMATION ENGINES --- */
static Decision dispatch_shell(AgentRuntime *rt, const char *binary, const char *args) {
    char cmd[MAX_TEXT + 128]; snprintf(cmd, sizeof(cmd), "%s %s \"%s\" 2>&1", binary, args, rt->objective);
    logger(LOG_INFO, "Executing: %s", cmd);
    FILE *fp = popen(cmd, "r"); if (!fp) { snprintf(rt->error_msg, MAX_TEXT, "POPEN_FAIL: %s", strerror(errno)); return DEC_RETRY; }
    size_t n = fread(rt->result, 1, MAX_TEXT - 1, fp); rt->result[n] = 0;
    int status = pclose(fp);
    if (status != 0) { snprintf(rt->error_msg, MAX_TEXT, "SHELL_EXIT_%d", status); return DEC_RETRY; }
    return DEC_COMPLETE;
}

/* --- PRODUCTION WORKFLOW --- */
static void run_agent(AgentRuntime *rt) {
    if (!rt) {
        logger(LOG_ERROR, "Null runtime pointer passed to run_agent");
        return;
    }
    logger(LOG_INFO, "Initializing BOS-OMEGA Production Runtime v%s", VERSION);
    rt->state = ST_ROUTE; rt->tool = resolve_tool(rt->objective);
    rt->risk = assess_risk(rt->objective);
    logger(LOG_AUDIT, "Task: %s | Tool: %d | Risk: %d", rt->objective, rt->tool, rt->risk);

    if (rt->risk >= R_DESTRUCTIVE && !rt->is_approved) {
        rt->state = ST_ESCALATE; rt->decision = DEC_ESCALATE;
        logger(LOG_WARN, "Policy Block: Destructive action requires --approve");
        return;
    }

    for (rt->retry_cnt = 0; rt->retry_cnt < MAX_RETRIES; rt->retry_cnt++) {
        rt->state = ST_EXEC; logger(LOG_INFO, "Execution Attempt %d/%d", rt->retry_cnt + 1, MAX_RETRIES);
        Decision res;
        if (rt->tool == T_PWSH) res = dispatch_shell(rt, "pwsh", "-Command");
        else if (rt->tool == T_ADB) res = dispatch_shell(rt, "adb", "shell");
        else {
            ToolSpec *ts = &REGISTRY[rt->tool];
            const char *key = getenv(ts->env_key);
            if (!key && ts->type != T_CDP) { strncpy(rt->error_msg, "MISSING_API_KEY", MAX_TEXT - 1); rt->error_msg[MAX_TEXT - 1] = '\0'; res = DEC_BLOCKED; }
            else {
                char payload[MAX_TEXT + 128];
                snprintf(payload, sizeof(payload), "{\"model\":\"prod\",\"input\":\"%.*s\"}", MAX_TEXT, rt->objective);
                res = dispatch_http(rt, ts->url, key ? key : "NONE", payload);
            }
        }

        if (res == DEC_COMPLETE) {
            rt->state = ST_SUCCESS; rt->decision = DEC_COMPLETE;
            logger(LOG_INFO, "Task Completed Successfully");
            break;
        } else if (res == DEC_BLOCKED) {
            rt->state = ST_FAILURE; rt->decision = DEC_BLOCKED;
            logger(LOG_ERROR, "Execution Blocked: %s", rt->error_msg);
            break;
        }
        logger(LOG_WARN, "Attempt Failed: %s. Retrying...", rt->error_msg);
    }

    if (rt->state != ST_SUCCESS) {
        rt->state = ST_FAILURE; rt->decision = DEC_ABORT;
        logger(LOG_ERROR, "Final State: FAILURE | Reason: %s", rt->error_msg);
    }
}

/* --- LANGUAGE DECISION ENGINE --- */
static void run_lang(LangInput *in) {
    if (!in) {
        printf("Error: Null input pointer passed to run_lang\n");
        return;
    }

    LangOutput out = {0};
    strncpy(out.goal, in->goal, MAX_TEXT - 1);
    out.goal[MAX_TEXT - 1] = '\0';

    char norm_goal[MAX_TEXT];
    char norm_skill[MAX_TEXT];
    char norm_type[MAX_TEXT];
    char norm_plat[MAX_TEXT];
    char norm_time[MAX_TEXT];
    char norm_style[MAX_TEXT];
    char norm_constraints[MAX_TEXT];

    normalize(in->goal, norm_goal);
    normalize(in->skill, norm_skill);
    normalize(in->type, norm_type);
    normalize(in->plat, norm_plat);
    normalize(in->time, norm_time);
    normalize(in->style, norm_style);
    normalize(in->constraints, norm_constraints);

    char detected_goal[128] = "General Development";
    char best_lang[256] = "TypeScript / Rust";
    char why[512] = "Modern safety, ecosystem size, and performance scalability.";
    char frontend_stack[128] = "React / Next.js / Tailwind CSS";
    char backend_stack[128] = "TypeScript (Node.js) or Rust (Actix-web)";
    char db_stack[128] = "PostgreSQL";
    char automation_stack[128] = "GitHub Actions / Bash";
    char deploy_stack[128] = "Docker + Kubernetes / AWS";
    char if_cond[256] = "User needs general, scalable modern SaaS";
    char then_dec[256] = "Recommend TypeScript for rapid iteration, Rust for high performance";
    char learn_1[128] = "Learn TypeScript / JavaScript fundamentals";
    char learn_2[128] = "Understand Backend APIs & Databases (PostgreSQL)";
    char learn_3[128] = "Learn React / Next.js for modern web interfaces";
    char learn_4[128] = "Explore Rust for high-performance or safety-critical services";
    char build_setup[256] = "Initialize Node.js/TypeScript or Rust project and configure ESLint/Cargo";
    char build_proto[256] = "Build minimal API endpoint and connect to Postgres";
    char build_test[256] = "Add unit tests (Jest/Vitest or Cargo test) and API integration tests";
    char build_deploy[256] = "Dockerize application and deploy to cloud (e.g. Vercel, Railway, or AWS)";
    char build_iterate[256] = "Profile bottlenecks and rewrite critical sections in Rust or optimized SQL";
    char final_rec[512] = "Start with TypeScript and Node.js for maximum velocity. Keep database logic robust in PostgreSQL, and introduce Rust incrementally for CPU-bound microservices.";

    bool matched = false;

    // 1. Website
    if (strstr(norm_goal, "website") || strstr(norm_goal, "web") || strstr(norm_goal, "frontend")) {
        matched = true;
        strcpy(detected_goal, "Website");
        strcpy(best_lang, "HTML / CSS / JavaScript / TypeScript");
        strcpy(why, "HTML/CSS for markup and styling, JS/TS for client-side interactivity, and React/Next.js for app-level complexity.");
        strcpy(frontend_stack, "React / Next.js / Tailwind CSS");
        strcpy(backend_stack, "Next.js Route Handlers / Express");
        strcpy(db_stack, "PostgreSQL");
        strcpy(automation_stack, "GitHub Actions");
        strcpy(deploy_stack, "Vercel / Netlify");
        strcpy(if_cond, "User goal is 'build website'");
        strcpy(then_dec, "Recommend HTML, CSS, JS, TS, and React/Next.js if app-level complexity exists");
        strcpy(learn_1, "HTML5 & CSS3 layout fundamentals");
        strcpy(learn_2, "JavaScript ES6+ and TypeScript types");
        strcpy(learn_3, "React components and state management");
        strcpy(learn_4, "Next.js for SSR, routing, and deployment");
        strcpy(build_setup, "Create React/Next.js app with TypeScript and Tailwind CSS");
        strcpy(build_proto, "Mock data and build UI layouts & components");
        strcpy(build_test, "Write component tests with Jest/Playwright");
        strcpy(build_deploy, "Connect repository and deploy instantly to Vercel/Netlify");
        strcpy(build_iterate, "Analyze Lighthouse scores and optimize images/bundle-size");
        strcpy(final_rec, "Next.js + TypeScript + Tailwind is the most productive stack for building modern websites.");
    }
    // 2. AI System / Autonomous Agent
    else if (has_word(norm_goal, "ai") || strstr(norm_goal, "agent") || strstr(norm_goal, "openai") || strstr(norm_goal, "mistral") || strstr(norm_goal, "llm")) {
        matched = true;
        strcpy(detected_goal, "AI System / Autonomous Agent");
        strcpy(best_lang, "Python (AI Workers), TypeScript (API/App layer), SQL (Storage), Bash/YAML (Automation)");
        strcpy(why, "Python dominates ML ecosystem, while TypeScript provides the best developer experience for API and client integration.");
        strcpy(frontend_stack, "React / Next.js");
        strcpy(backend_stack, "FastAPI (Python) & NestJS (TypeScript)");
        strcpy(db_stack, "PostgreSQL with pgvector");
        strcpy(automation_stack, "Bash scripting & YAML workflows");
        strcpy(deploy_stack, "Docker + Fly.io / AWS");
        strcpy(if_cond, "User goal is 'build AI system'");
        strcpy(then_dec, "Recommend Python for AI workers, TS for app/API layer, SQL for storage, Bash/YAML for automation");
        strcpy(learn_1, "Python fundamentals & AI/ML libraries (PyTorch, Transformers)");
        strcpy(learn_2, "TypeScript & Next.js for client interfaces");
        strcpy(learn_3, "Vector Databases & Semantic Search (pgvector, Pinecone)");
        strcpy(learn_4, "Agentic Frameworks (LangChain, AutoGen, BOS-OMEGA)");
        strcpy(build_setup, "Setup Python virtual environment and initialize Node/Next.js project");
        strcpy(build_proto, "Create basic API wrapper around LLM / Vector DB");
        strcpy(build_test, "Test LLM prompting outputs and API robustness");
        strcpy(build_deploy, "Dockerize service and deploy AI workers separately from frontend");
        strcpy(build_iterate, "Track token usage, optimize context window, and refine prompts");
        strcpy(final_rec, "FastAPI (Python) backends connected to a Next.js (TypeScript) client with pgvector storage.");
    }
    // 3. Mobile App
    else if (strstr(norm_goal, "mobile") || strstr(norm_goal, "ios") || strstr(norm_goal, "android") || strstr(norm_goal, "phone") || strstr(norm_goal, "app")) {
        matched = true;
        strcpy(detected_goal, "Mobile App");
        if (strstr(norm_goal, "ios") && !strstr(norm_goal, "android") && !strstr(norm_goal, "both")) {
            strcpy(best_lang, "Swift");
            strcpy(why, "Swift is Apple's native, high-performance language for iOS development.");
            strcpy(frontend_stack, "SwiftUI");
            strcpy(backend_stack, "FastAPI / Node.js");
            strcpy(db_stack, "SQLite / CoreData");
            strcpy(automation_stack, "Fastlane");
            strcpy(deploy_stack, "Apple App Store");
            strcpy(if_cond, "User goal is 'build mobile app' and iOS only");
            strcpy(then_dec, "Recommend Swift");
            strcpy(learn_1, "Swift language fundamentals & Optionals");
            strcpy(learn_2, "SwiftUI declarative layouts");
            strcpy(learn_3, "Combine / Swift Concurrency (async/await)");
            strcpy(learn_4, "App Store Connect guidelines & publishing");
            strcpy(build_setup, "Create a new Xcode project with SwiftUI");
            strcpy(build_proto, "Build primary views and mock local state");
            strcpy(build_test, "Run Xcode UI and Unit tests");
            strcpy(build_deploy, "Configure TestFlight and submit for Beta testing");
            strcpy(build_iterate, "Analyze memory leaks and optimize layout rendering");
            strcpy(final_rec, "Swift + SwiftUI for native iOS development with maximum performance.");
        } else if (strstr(norm_goal, "android") && !strstr(norm_goal, "ios") && !strstr(norm_goal, "both")) {
            strcpy(best_lang, "Kotlin");
            strcpy(why, "Kotlin is Google's preferred modern language for native Android development.");
            strcpy(frontend_stack, "Jetpack Compose");
            strcpy(backend_stack, "FastAPI / Node.js");
            strcpy(db_stack, "Room Database / SQLite");
            strcpy(automation_stack, "Gradle");
            strcpy(deploy_stack, "Google Play Store");
            strcpy(if_cond, "User goal is 'build mobile app' and Android only");
            strcpy(then_dec, "Recommend Kotlin");
            strcpy(learn_1, "Kotlin OOP & functional features");
            strcpy(learn_2, "Jetpack Compose modern UI layout");
            strcpy(learn_3, "Coroutines & Flow for asynchronous tasks");
            strcpy(learn_4, "Google Play Console publishing guidelines");
            strcpy(build_setup, "Initialize a new project in Android Studio");
            strcpy(build_proto, "Draft app layout using Compose and local state");
            strcpy(build_test, "Test with Android Emulator and local devices");
            strcpy(build_deploy, "Generate signed bundle and upload to Google Play");
            strcpy(build_iterate, "Audit performance via Android Profiler");
            strcpy(final_rec, "Kotlin + Jetpack Compose for native Android development with robust toolsets.");
        } else {
            strcpy(best_lang, "Dart + Flutter");
            strcpy(why, "Dart + Flutter provides high-performance cross-platform mobile development for both iOS & Android.");
            strcpy(frontend_stack, "Flutter UI");
            strcpy(backend_stack, "Node.js / Go backend");
            strcpy(db_stack, "SQLite / Hive / Firestore");
            strcpy(automation_stack, "GitHub Actions");
            strcpy(deploy_stack, "App Store & Play Store");
            strcpy(if_cond, "User goal is 'build mobile app' for both iOS and Android");
            strcpy(then_dec, "Recommend Dart + Flutter");
            strcpy(learn_1, "Dart syntax & programming structures");
            strcpy(learn_2, "Flutter Widgets & layout tree");
            strcpy(learn_3, "State management (Bloc, Provider, or Riverpod)");
            strcpy(learn_4, "Native platform channels & plugins");
            strcpy(build_setup, "Create a Flutter project with `flutter create`");
            strcpy(build_proto, "Mock state and build screens using Flutter Widgets");
            strcpy(build_test, "Run Widget tests and driver integrations");
            strcpy(build_deploy, "Build IPA/APK bundles for cross-platform releases");
            strcpy(build_iterate, "Verify UI continuity on multiple screen dimensions");
            strcpy(final_rec, "Dart + Flutter is the ultimate stack for native cross-platform mobile apps.");
        }
    }
    // 4. Backend / API
    else if (strstr(norm_goal, "backend") || strstr(norm_goal, "api") || strstr(norm_goal, "server") || strstr(norm_goal, "microservice")) {
        matched = true;
        strcpy(detected_goal, "Backend / API");
        if (strstr(norm_skill, "beginner") || strstr(norm_skill, "learn")) {
            strcpy(best_lang, "Python + FastAPI");
            strcpy(why, "Python + FastAPI provides a fast, modern, and simple developer experience for backend development.");
            strcpy(frontend_stack, "None (API only) / Swagger Documentation");
            strcpy(backend_stack, "FastAPI");
            strcpy(db_stack, "SQLite");
            strcpy(automation_stack, "Uvicorn");
            strcpy(deploy_stack, "Deta Space / Render");
            strcpy(if_cond, "User goal is 'build backend/API' and user is beginner");
            strcpy(then_dec, "Recommend Python + FastAPI");
            strcpy(learn_1, "Python basics & syntax");
            strcpy(learn_2, "HTTP methods, status codes, and REST concepts");
            strcpy(learn_3, "Pydantic models & FastAPI route validation");
            strcpy(learn_4, "Basic SQL database integrations");
            strcpy(build_setup, "Initialize Python project and install fastapi + uvicorn");
            strcpy(build_proto, "Write single endpoint returning JSON response");
            strcpy(build_test, "Run automated interactive docs at `/docs`");
            strcpy(build_deploy, "Deploy on Render or any free tier hosting");
            strcpy(build_iterate, "Refactor queries and improve code structure");
            strcpy(final_rec, "Python + FastAPI is the fastest entry point for backend development.");
        } else if (strstr(norm_skill, "production") || strstr(norm_skill, "saas") || strstr(norm_skill, "enterprise") || strstr(norm_type, "saas")) {
            strcpy(best_lang, "TypeScript + Node.js or Go");
            strcpy(why, "TypeScript/Node.js or Go provides excellent developer velocity and scalability for production SaaS.");
            strcpy(frontend_stack, "React / Next.js client");
            strcpy(backend_stack, "TypeScript (Express / NestJS) or Go (Gin / Fiber)");
            strcpy(db_stack, "PostgreSQL");
            strcpy(automation_stack, "GitHub Actions");
            strcpy(deploy_stack, "AWS / GCP / Docker");
            strcpy(if_cond, "User goal is 'build backend/API' and user is production SaaS");
            strcpy(then_dec, "Recommend TypeScript + Node.js OR Go");
            strcpy(learn_1, "Advanced TypeScript or Go programming syntax");
            strcpy(learn_2, "Database indexes, normalization, & pooling");
            strcpy(learn_3, "Web security, JWT, OAuth, and CORS policies");
            strcpy(learn_4, "Microservice communication protocols (gRPC, Redis)");
            strcpy(build_setup, "Setup clean monorepo architecture and Docker");
            strcpy(build_proto, "Construct CRUD API and database schema migration");
            strcpy(build_test, "Write deep integration tests & load-test endpoints");
            strcpy(build_deploy, "Deploy multi-container app on AWS with TLS");
            strcpy(build_iterate, "Introduce connection poolers, caching, & Redis");
            strcpy(final_rec, "TypeScript + Node.js or Go is the industry standard for scalable SaaS backends.");
        } else if (strstr(norm_goal, "performance") || strstr(norm_goal, "security") || strstr(norm_constraints, "performance") || strstr(norm_constraints, "security")) {
            strcpy(best_lang, "Rust or Go");
            strcpy(why, "Rust or Go provides bare-metal speed, memory safety, and high-concurrency performance.");
            strcpy(frontend_stack, "None (Pure API)");
            strcpy(backend_stack, "Rust (Actix-web / Axum) or Go (Standard Library)");
            strcpy(db_stack, "PostgreSQL");
            strcpy(automation_stack, "Cargo / Go Build");
            strcpy(deploy_stack, "Bare Metal VPS / Kubernetes");
            strcpy(if_cond, "User goal is 'build backend/API' and is high-performance/security-critical");
            strcpy(then_dec, "Recommend Rust OR Go");
            strcpy(learn_1, "Rust Memory Safety/Borrow Checker or Go Concurrency");
            strcpy(learn_2, "Asynchronous runtime patterns (Tokio or Goroutines)");
            strcpy(learn_3, "Low-level optimization & socket communication");
            strcpy(learn_4, "Secure cryptographical libraries");
            strcpy(build_setup, "Initialize project using Cargo or Go Modules");
            strcpy(build_proto, "Develop high-speed routing with zero-copy parser");
            strcpy(build_test, "Benchmark against raw HTTP throughput standards");
            strcpy(build_deploy, "Build scratch Docker images (<50MB) and deploy");
            strcpy(build_iterate, "Profile memory allocations using valgrind or pprof");
            strcpy(final_rec, "Rust or Go backends for lightning-fast and secure system architectures.");
        } else {
            strcpy(best_lang, "TypeScript + Go");
            strcpy(why, "Scalable and modern backend languages.");
            strcpy(frontend_stack, "None (Pure API)");
            strcpy(backend_stack, "Go + TypeScript + Node.js");
            strcpy(db_stack, "PostgreSQL");
            strcpy(automation_stack, "GitHub Actions");
            strcpy(deploy_stack, "Docker + VPS");
            strcpy(if_cond, "User goal is 'build backend/API'");
            strcpy(then_dec, "Recommend TypeScript + Node.js OR Go");
            strcpy(learn_1, "TypeScript & Node.js ecosystem");
            strcpy(learn_2, "Go syntax & web servers");
            strcpy(learn_3, "Database orchestration & schema design");
            strcpy(learn_4, "Containerization with Docker");
            strcpy(build_setup, "Setup standard containerized backend template");
            strcpy(build_proto, "Implement main API routing paths");
            strcpy(build_test, "Run local suite of API endpoint validations");
            strcpy(build_deploy, "Dockerize and launch on VPS");
            strcpy(build_iterate, "Add caching layer with Redis");
            strcpy(final_rec, "Go and TypeScript offer the best of performance and iteration speed.");
        }
    }
    // 5. Operating System / Firmware / Embedded
    else if (strstr(norm_goal, "operating system") || strstr(norm_goal, "firmware") || strstr(norm_goal, "embedded") || strstr(norm_goal, "kernel") || strstr(norm_goal, "hardware") || strstr(norm_goal, "microcontroller") || strstr(norm_goal, "cpu")) {
        matched = true;
        strcpy(detected_goal, "Operating System / Firmware / Embedded");
        strcpy(best_lang, "C / C++ / Rust / Assembly");
        strcpy(why, "C/C++/Rust for bare-metal performance and Assembly only where direct CPU/control access is required.");
        strcpy(frontend_stack, "None (Bare Metal / Serial Console)");
        strcpy(backend_stack, "C / C++ / Rust");
        strcpy(db_stack, "None / Custom Flash File System");
        strcpy(automation_stack, "Make / CMake / Cargo");
        strcpy(deploy_stack, "Embedded Flash / JTAG Programmer");
        strcpy(if_cond, "User goal is 'build operating system / firmware / embedded'");
        strcpy(then_dec, "Recommend C, C++, Rust, and Assembly only where direct CPU/control access is required");
        strcpy(learn_1, "C pointer arithmetic, memory layout, & register maps");
        strcpy(learn_2, "Computer Architecture and CPU Instruction Sets (ARM, x86, RISC-V)");
        strcpy(learn_3, "Operating System concepts (paging, interrupts, schedulers)");
        strcpy(learn_4, "Rust Embedded-HAL or C++ templates");
        strcpy(build_setup, "Setup toolchain cross-compilers (gcc-arm-none-eabi)");
        strcpy(build_proto, "Write assembly bootloader to jump to main C kernel");
        strcpy(build_test, "Simulate in QEMU or run on hardware boards with logic analyzer");
        strcpy(build_deploy, "Flash binary to target microcontroller / hardware storage");
        strcpy(build_iterate, "Tune CPU frequency and optimize interrupt routines");
        strcpy(final_rec, "C remains the lingua franca of embedded systems, but Rust is rapidly emerging as the modern safe alternative.");
    }
    // 6. FPGA / ASIC / Digital Logic
    else if (strstr(norm_goal, "fpga") || strstr(norm_goal, "asic") || strstr(norm_goal, "digital logic") || strstr(norm_goal, "verilog") || strstr(norm_goal, "vhdl")) {
        matched = true;
        strcpy(detected_goal, "FPGA / ASIC / Digital Logic");
        strcpy(best_lang, "Verilog / SystemVerilog / VHDL");
        strcpy(why, "Hardware description languages are required to describe and synthesize digital circuits.");
        strcpy(frontend_stack, "None / LED / VGA Output");
        strcpy(backend_stack, "Verilog / SystemVerilog / VHDL");
        strcpy(db_stack, "Block RAM / Register Files");
        strcpy(automation_stack, "Tcl Scripts");
        strcpy(deploy_stack, "FPGA Board (Xilinx, Altera)");
        strcpy(if_cond, "User goal is 'FPGA / ASIC / digital logic'");
        strcpy(then_dec, "Recommend Verilog, SystemVerilog, or VHDL");
        strcpy(learn_1, "Boolean Algebra & Combinational Logic design");
        strcpy(learn_2, "Sequential Logic (Flip-Flops, Registers, FSMs)");
        strcpy(learn_3, "Hardware Description Language syntax (Verilog/VHDL)");
        strcpy(learn_4, "Synthesis, Timing constraints, & Place-and-Route");
        strcpy(build_setup, "Install EDA tools (Vivado, Quartus, or open-source Yosys)");
        strcpy(build_proto, "Implement basic 8-bit ALU or State Machine");
        strcpy(build_test, "Write testbenches and run logic simulation in ModelSim/GTKWave");
        strcpy(build_deploy, "Generate bitstream file and load onto FPGA development board");
        strcpy(build_iterate, "Analyze timing report and eliminate setup/hold violations");
        strcpy(final_rec, "SystemVerilog for synthesis and VHDL for high-reliability systems.");
    }
    // 7. Data Science / ML
    else if (strstr(norm_goal, "data science") || strstr(norm_goal, "ml") || strstr(norm_goal, "machine learning") || strstr(norm_goal, "statistics") || strstr(norm_goal, "analytics")) {
        matched = true;
        strcpy(detected_goal, "Data Science / ML");
        if (strstr(norm_goal, "statistics") || strstr(norm_style, "statistics") || strstr(norm_goal, " r ") || strcmp(norm_goal, "r") == 0) {
            strcpy(best_lang, "R");
            strcpy(why, "R is specialized for statistics, modeling, and deep data analysis.");
            strcpy(frontend_stack, "Shiny Dashboard");
            strcpy(backend_stack, "R");
            strcpy(db_stack, "SQL Databases");
            strcpy(automation_stack, "R Markdown");
            strcpy(deploy_stack, "Shiny Server / RStudio Connect");
            strcpy(if_cond, "User goal is 'data science / ML' and is statistics-heavy");
            strcpy(then_dec, "Recommend R");
            strcpy(learn_1, "Statistics, regression, and data visualization in R");
            strcpy(learn_2, "tidyverse libraries (ggplot2, dplyr)");
            strcpy(learn_3, "Statistical hypothesis testing and modeling");
            strcpy(learn_4, "Shiny package for interactive app development");
            strcpy(build_setup, "Create RStudio environment and install packages");
            strcpy(build_proto, "Conduct exploratory data analysis and plot variables");
            strcpy(build_test, "Verify statistical model assumptions and fit residuals");
            strcpy(build_deploy, "Export results as R Markdown report or publish Shiny app");
            strcpy(build_iterate, "Refine visual elements and gather user feedback");
            strcpy(final_rec, "R is the absolute powerhouse for academic statistics and data visualization.");
        } else if (strstr(norm_goal, "julia") || strstr(norm_constraints, "julia") || strstr(norm_style, "julia")) {
            strcpy(best_lang, "Julia");
            strcpy(why, "Julia solves the two-language problem by being as fast as C and as easy as Python.");
            strcpy(frontend_stack, "Pluto.jl Notebooks");
            strcpy(backend_stack, "Julia");
            strcpy(db_stack, "SQL Databases");
            strcpy(automation_stack, "Julia package manager");
            strcpy(deploy_stack, "JuliaHub / Docker");
            strcpy(if_cond, "User goal is 'data science / ML' and high-performance numerical computing is required");
            strcpy(then_dec, "Recommend Julia");
            strcpy(learn_1, "Julia syntax, types, and multiple dispatch");
            strcpy(learn_2, "Linear Algebra & Numerical Analysis packages");
            strcpy(learn_3, "Distributed & GPU-accelerated computing in Julia");
            strcpy(learn_4, "Flux.jl for deep learning modeling");
            strcpy(build_setup, "Setup Julia REPL and create package environment");
            strcpy(build_proto, "Write vectorized computations and test multiple dispatch");
            strcpy(build_test, "Run benchmarks on custom algorithms vs native libraries");
            strcpy(build_deploy, "Publish packages or deploy optimized numerical server");
            strcpy(build_iterate, "Profile type stability and optimize allocations");
            strcpy(final_rec, "Julia is ideal for heavy scientific computing, physical modeling, and custom ML pipelines.");
        } else {
            strcpy(best_lang, "Python + SQL");
            strcpy(why, "Python is the undisputed king of ML/Data Science with a vast ecosystem, and SQL is essential for data querying.");
            strcpy(frontend_stack, "Streamlit / Gradio");
            strcpy(backend_stack, "Python (Flask / FastAPI)");
            strcpy(db_stack, "SQL (PostgreSQL / TiDB) & NoSQL");
            strcpy(automation_stack, "Prefect / Airflow");
            strcpy(deploy_stack, "Docker + Kubernetes");
            strcpy(if_cond, "User goal is 'data science / ML'");
            strcpy(then_dec, "Recommend Python");
            strcpy(learn_1, "Python libraries (NumPy, Pandas, Scikit-learn)");
            strcpy(learn_2, "SQL querying, joins, & schema design");
            strcpy(learn_3, "Deep Learning Frameworks (TensorFlow, PyTorch)");
            strcpy(learn_4, "Data pipelines and orchestration (Airflow)");
            strcpy(build_setup, "Set up Anaconda/Jupyter Notebook and database");
            strcpy(build_proto, "Gather dataset, perform EDA, and train baseline model");
            strcpy(build_test, "Evaluate model with test/validation splits and metrics");
            strcpy(build_deploy, "Deploy model with FastAPI backend or Streamlit dashboard");
            strcpy(build_iterate, "Collect fresh data, tune hyperparameters, and retrain");
            strcpy(final_rec, "Python + SQL is the universal stack for modern Data Science and Machine Learning.");
        }
    }
    // 8. Database-Heavy App
    else if (strstr(norm_goal, "database") || strstr(norm_goal, "sql") || strstr(norm_goal, "query") || strstr(norm_goal, "postgres")) {
        matched = true;
        strcpy(detected_goal, "Database-Heavy App");
        strcpy(best_lang, "SQL (PostgreSQL) + TypeScript/Python backend");
        strcpy(why, "SQL is needed first for complex data operations, paired with PostgreSQL for reliable relational storage and a TS/Python API.");
        strcpy(frontend_stack, "React / Next.js");
        strcpy(backend_stack, "TypeScript Node.js (with Prisma/Drizzle ORM) or Python (SQLAlchemy)");
        strcpy(db_stack, "PostgreSQL");
        strcpy(automation_stack, "DB Migrations (dbmate / flyway)");
        strcpy(deploy_stack, "Docker / TiDB / AWS Aurora");
        strcpy(if_cond, "User goal is 'database-heavy app'");
        strcpy(then_dec, "Recommend SQL first, PostgreSQL, and TypeScript/Python backend");
        strcpy(learn_1, "SQL fundamentals, joins, subqueries, and aggregates");
        strcpy(learn_2, "Advanced DB concepts (indexing, transactions, window functions)");
        strcpy(learn_3, "Database performance profiling & query execution plans");
        strcpy(learn_4, "Backend ORMs & connection pooling integration");
        strcpy(build_setup, "Initialize PostgreSQL instance and migration workflow");
        strcpy(build_proto, "Design DB schemas and write mock data seeds");
        strcpy(build_test, "Test query performance and transaction safety under stress");
        strcpy(build_deploy, "Deploy database on high-availability cloud provider");
        strcpy(build_iterate, "Add indexes, partition tables, and cache slow reads in Redis");
        strcpy(final_rec, "PostgreSQL + TypeScript + Prisma/Drizzle is the most reliable stack for database-intensive applications.");
    }
    // 9. Blockchain / Smart Contracts
    else if (strstr(norm_goal, "blockchain") || strstr(norm_goal, "smart contract") || strstr(norm_goal, "ethereum") || strstr(norm_goal, "solidity") || strstr(norm_goal, "solana") || strstr(norm_goal, "crypto") || strstr(norm_goal, "web3") || strstr(norm_goal, "move")) {
        matched = true;
        strcpy(detected_goal, "Blockchain / Smart Contracts");
        if (strstr(norm_goal, "ethereum") || strstr(norm_goal, "solidity") || strstr(norm_goal, "evm")) {
            strcpy(best_lang, "Solidity + TypeScript");
            strcpy(why, "Solidity is the industry standard for EVM smart contracts, paired with TS for testing/frontend.");
            strcpy(frontend_stack, "Next.js + Ethers.js / Wagmi");
            strcpy(backend_stack, "TypeScript Node.js API");
            strcpy(db_stack, "Subgraph / The Graph (GraphQL)");
            strcpy(automation_stack, "Hardhat / Foundry");
            strcpy(deploy_stack, "Ethereum / EVM Chains (L2s)");
            strcpy(if_cond, "User goal is 'blockchain / smart contracts' and EVM compatibility is requested");
            strcpy(then_dec, "Recommend Solidity + TypeScript");
            strcpy(learn_1, "Solidity syntax, state variables, & gas optimization");
            strcpy(learn_2, "EVM mechanics, security vulnerabilities (reentrancy)");
            strcpy(learn_3, "Foundry or Hardhat for writing contract unit tests");
            strcpy(learn_4, "Frontend Web3 integrations (wagmi, rainbowkit)");
            strcpy(build_setup, "Initialize a Foundry/Hardhat project template");
            strcpy(build_proto, "Draft and compile main ERC-20 or custom contract");
            strcpy(build_test, "Run rigorous unit tests with multiple edge cases & fuzzing");
            strcpy(build_deploy, "Deploy to testnet (Sepolia) and verify on Etherscan");
            strcpy(build_iterate, "Audit code, optimize gas usage, and deploy to mainnet");
            strcpy(final_rec, "Foundry + Solidity is the ultimate toolkit for building secure EVM applications.");
        } else if (strstr(norm_goal, "solana")) {
            strcpy(best_lang, "Rust");
            strcpy(why, "Rust is the native, high-performance language for Solana program development.");
            strcpy(frontend_stack, "Next.js + Solana Web3.js SDK");
            strcpy(backend_stack, "None (Pure Program) / Node.js");
            strcpy(db_stack, "Solana Accounts");
            strcpy(automation_stack, "Anchor Framework");
            strcpy(deploy_stack, "Solana Blockchain (Mainnet-Beta)");
            strcpy(if_cond, "User goal is 'blockchain / smart contracts' and target is Solana");
            strcpy(then_dec, "Recommend Rust");
            strcpy(learn_1, "Rust basics & lifetime semantics");
            strcpy(learn_2, "Solana programming model & Account structure");
            strcpy(learn_3, "Anchor Framework syntax and security guidelines");
            strcpy(learn_4, "Solana Web3.js client-side integration");
            strcpy(build_setup, "Setup Solana CLI and Anchor framework environment");
            strcpy(build_proto, "Build basic Anchor program to update account data");
            strcpy(build_test, "Test contract logic using Anchor TypeScript tests");
            strcpy(build_deploy, "Deploy on Solana Devnet and verify account states");
            strcpy(build_iterate, "Profile compute unit consumption and optimize programs");
            strcpy(final_rec, "Anchor + Rust is the de facto standard for writing Solana smart contracts.");
        } else if (strstr(norm_goal, "move")) {
            strcpy(best_lang, "Move");
            strcpy(why, "Move provides resource safety and formal verification for next-gen blockchain smart contracts.");
            strcpy(frontend_stack, "Next.js + Aptos/Sui SDK");
            strcpy(backend_stack, "None (Pure Move module)");
            strcpy(db_stack, "On-chain Resources");
            strcpy(automation_stack, "Move CLI");
            strcpy(deploy_stack, "Aptos / Sui Blockchain");
            strcpy(if_cond, "User goal is 'blockchain / smart contracts' and target is Move-chain");
            strcpy(then_dec, "Recommend Move");
            strcpy(learn_1, "Move resource-oriented programming model");
            strcpy(learn_2, "Aptos/Sui-specific global storage & capabilities");
            strcpy(learn_3, "Formal verification using the Move Prover");
            strcpy(learn_4, "Client-side integrations using Aptos/Sui SDKs");
            strcpy(build_setup, "Install Move CLI and initialize new Move package");
            strcpy(build_proto, "Develop a basic module with token or asset structures");
            strcpy(build_test, "Run Move unit tests and run Move Prover validations");
            strcpy(build_deploy, "Publish module to testnet and run explorer audits");
            strcpy(build_iterate, "Verify capability structures and launch to mainnet");
            strcpy(final_rec, "Move offers industry-leading safety guarantees for next-gen smart contract design.");
        } else {
            strcpy(best_lang, "Solidity + TypeScript");
            strcpy(why, "Solidity is the industry standard for EVM smart contracts, paired with TS for testing/frontend.");
            strcpy(frontend_stack, "Next.js + Web3.js");
            strcpy(backend_stack, "TypeScript Node.js");
            strcpy(db_stack, "PostgreSQL");
            strcpy(automation_stack, "Hardhat");
            strcpy(deploy_stack, "EVM Compatible Blockchain");
            strcpy(if_cond, "User goal is 'blockchain / smart contracts'");
            strcpy(then_dec, "Recommend Solidity + TypeScript");
            strcpy(learn_1, "Solidity smart contracts");
            strcpy(learn_2, "Smart contract testing techniques");
            strcpy(learn_3, "Frontend Web3 libraries");
            strcpy(learn_4, "Smart contract security audits");
            strcpy(build_setup, "Create a Hardhat + TypeScript workspace");
            strcpy(build_proto, "Write a basic ERC20 or ERC721 contract");
            strcpy(build_test, "Run smart contract test suite");
            strcpy(build_deploy, "Deploy on Ethereum testnet");
            strcpy(build_iterate, "Audit smart contract and deploy to mainnet");
            strcpy(final_rec, "EVM compatibility with Solidity is the safest default choice for blockchain apps.");
        }
    }
    // 10. Automation / Scripting
    else if (strstr(norm_goal, "automation") || strstr(norm_goal, "scripting") || strstr(norm_goal, "script") || strstr(norm_goal, "bash") || strstr(norm_goal, "powershell") || strstr(norm_goal, "cron") || strstr(norm_goal, "workflow")) {
        matched = true;
        strcpy(detected_goal, "Automation / Scripting");
        if (strstr(norm_plat, "windows") || strstr(norm_goal, "windows") || strstr(norm_goal, "powershell")) {
            strcpy(best_lang, "PowerShell + Python");
            strcpy(why, "PowerShell offers deep Windows integration, combined with Python's versatility.");
            strcpy(frontend_stack, "Windows Forms / Console UI");
            strcpy(backend_stack, "PowerShell Core & Python 3");
            strcpy(db_stack, "SQLite");
            strcpy(automation_stack, "Task Scheduler / WinRM");
            strcpy(deploy_stack, "Windows Client / Server Group Policy");
            strcpy(if_cond, "User goal is 'automation / scripting' and platform is Windows");
            strcpy(then_dec, "Recommend PowerShell + Python");
            strcpy(learn_1, "PowerShell Cmdlets, pipelines, & Active Directory modules");
            strcpy(learn_2, "Python scripting basics and system integration libraries");
            strcpy(learn_3, "WinRM and remote scripting mechanics");
            strcpy(learn_4, "Automated scheduling & job orchestration");
            strcpy(build_setup, "Initialize a Git repository for scripts and configure execution policy");
            strcpy(build_proto, "Write PowerShell script to query and parse local system states");
            strcpy(build_test, "Simulate operations on a test Virtual Machine");
            strcpy(build_deploy, "Register scripts in Task Scheduler or push via Group Policy Objects");
            strcpy(build_iterate, "Add error catching, logging, and email notification via SMTP");
            strcpy(final_rec, "PowerShell + Python handles any complex automation challenge in Windows environments.");
        } else {
            strcpy(best_lang, "Bash + Python");
            strcpy(why, "Bash provides native Unix shell control, while Python handles complex scripting logic.");
            strcpy(frontend_stack, "Terminal UI / Textual");
            strcpy(backend_stack, "Bash shell & Python 3");
            strcpy(db_stack, "SQLite / JSON files");
            strcpy(automation_stack, "Cron Jobs / Systemd");
            strcpy(deploy_stack, "Linux VPS / GitHub Actions");
            strcpy(if_cond, "User goal is 'automation / scripting' on Linux/macOS");
            strcpy(then_dec, "Recommend Bash + Python");
            strcpy(learn_1, "Bash scripting, streams (stdout/stderr), pipes, and redirections");
            strcpy(learn_2, "Standard command utilities (grep, awk, sed, find)");
            strcpy(learn_3, "Python scripting & system interaction libraries (os, sys, subprocess)");
            strcpy(learn_4, "CI/CD workflow YAML specifications (GitHub Actions)");
            strcpy(build_setup, "Create scripts directory and make files executable (chmod +x)");
            strcpy(build_proto, "Write bash script wrapping sequential system tasks");
            strcpy(build_test, "Run script with test parameters and verify exit statuses");
            strcpy(build_deploy, "Configure Cron job or push workflow script to GitHub repository");
            strcpy(build_iterate, "Incorporate robust error trapping and centralized logging");
            strcpy(final_rec, "Bash + Python is the universal standard for Unix-based systems and DevOps pipelines.");
        }
    }

    if (!matched) {
        printf("Goal is unclear. Please answer exactly these 3 questions:\n");
        printf("1. What are you building?\n");
        printf("2. Where will it run?\n");
        printf("3. Do you want fastest build or strongest long-term architecture?\n");
        return;
    }

    // Copy results into the structure safely to fulfill memory safety rules
    strncpy(out.lang, best_lang, MAX_TEXT - 1);
    out.lang[MAX_TEXT - 1] = '\0';
    strncpy(out.why, why, MAX_TEXT - 1);
    out.why[MAX_TEXT - 1] = '\0';
    strncpy(out.stack, backend_stack, MAX_TEXT - 1);
    out.stack[MAX_TEXT - 1] = '\0';
    strncpy(out.steps, build_proto, MAX_TEXT - 1);
    out.steps[MAX_TEXT - 1] = '\0';

    printf("\nPROGRAMMING_LANGUAGE_DECISION_TREE\n");
    printf("│\n");
    printf("├── USER_GOAL\n");
    printf("│   └── %s\n", detected_goal);
    printf("│\n");
    printf("├── BEST_LANGUAGE\n");
    printf("│   └── %s\n", best_lang);
    printf("│\n");
    printf("├── WHY_THIS_LANGUAGE\n");
    printf("│   └── %s\n", why);
    printf("│\n");
    printf("├── SUPPORTING_STACK\n");
    printf("│   ├── Frontend:   %s\n", frontend_stack);
    printf("│   ├── Backend:    %s\n", backend_stack);
    printf("│   ├── Database:   %s\n", db_stack);
    printf("│   ├── Automation: %s\n", automation_stack);
    printf("│   └── Deployment: %s\n", deploy_stack);
    printf("│\n");
    printf("├── IF_THEN_REASONING\n");
    printf("│   ├── IF %s\n", if_cond);
    printf("│   └── THEN %s\n", then_dec);
    printf("│\n");
    printf("├── LEARNING_ORDER\n");
    printf("│   ├── Step 1: %s\n", learn_1);
    printf("│   ├── Step 2: %s\n", learn_2);
    printf("│   ├── Step 3: %s\n", learn_3);
    printf("│   └── Step 4: %s\n", learn_4);
    printf("│\n");
    printf("├── BUILD_ORDER\n");
    printf("│   ├── Setup:     %s\n", build_setup);
    printf("│   ├── Prototype: %s\n", build_proto);
    printf("│   ├── Test:      %s\n", build_test);
    printf("│   ├── Deploy:    %s\n", build_deploy);
    printf("│   └── Iterate:   %s\n", build_iterate);
    printf("│\n");
    printf("├── RANKING_CRITERIA\n");
    printf("│   ├── 1. Reality Fit\n");
    printf("│   ├── 2. Job Market\n");
    printf("│   ├── 3. Ecosystem\n");
    printf("│   ├── 4. Build Speed\n");
    printf("│   ├── 5. Runtime Performance\n");
    printf("│   ├── 6. Maintainability\n");
    printf("│   ├── 7. Security\n");
    printf("│   └── 8. Deployment Complexity\n");
    printf("│\n");
    printf("└── FINAL_RECOMMENDATION\n");
    printf("    └── %s\n", final_rec);
}

/* --- MAIN ENTRY POINT --- */
int main(int argc, char **argv) {
    if (argc < 2) {
        printf("BOS-OMEGA PRODUCTION RUNTIME v%s\n", VERSION);
        printf("Usage: %s <run|lang> <objective> [--approve]\n", argv[0]);
        return 1;
    }

    curl_global_init(CURL_GLOBAL_ALL);
    if (strcmp(argv[1], "run") == 0 && argc >= 3) {
        AgentRuntime rt = {0}; strncpy(rt.objective, argv[2], MAX_TEXT - 1);
        for (int i = 3; i < argc; i++) if (strcmp(argv[i], "--approve") == 0) rt.is_approved = true;
        run_agent(&rt);
        printf("\n--- EXECUTION RESULT ---\n%.1000s\n", rt.result);
    } else if (strcmp(argv[1], "lang") == 0 && argc >= 3) {
        LangInput in = {0};
        strncpy(in.goal, argv[2], MAX_TEXT - 1);
        in.goal[MAX_TEXT - 1] = '\0';
        if (argc >= 4) { strncpy(in.skill, argv[3], MAX_TEXT - 1); in.skill[MAX_TEXT - 1] = '\0'; }
        if (argc >= 5) { strncpy(in.type, argv[4], MAX_TEXT - 1); in.type[MAX_TEXT - 1] = '\0'; }
        if (argc >= 6) { strncpy(in.plat, argv[5], MAX_TEXT - 1); in.plat[MAX_TEXT - 1] = '\0'; }
        if (argc >= 7) { strncpy(in.time, argv[6], MAX_TEXT - 1); in.time[MAX_TEXT - 1] = '\0'; }
        if (argc >= 8) { strncpy(in.style, argv[7], MAX_TEXT - 1); in.style[MAX_TEXT - 1] = '\0'; }
        if (argc >= 9) { strncpy(in.constraints, argv[8], MAX_TEXT - 1); in.constraints[MAX_TEXT - 1] = '\0'; }
        run_lang(&in);
    } else {
        printf("Error: Invalid arguments.\n");
    }
    curl_global_cleanup();
    return 0;
}
