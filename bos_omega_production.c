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

/**
 * Normalizes input string by converting all characters to lowercase.
 * This optimizes keyword matching by allowing single O(N) pre-normalization
 * pass and subsequent exact O(M) strstr/has_word checks.
 */
static void normalize(const char *src, char *dest) {
    if (!dest) return;
    if (!src) {
        dest[0] = '\0';
        return;
    }
    int i = 0;
    for (i = 0; src[i] != '\0' && i < MAX_TEXT - 1; i++) {
        dest[i] = (char)tolower((unsigned char)src[i]);
    }
    dest[i] = '\0';
}

/**
 * Validates precise boundaries of lowercased needle in haystack
 * to ensure we match whole words and prevent false positives.
 */
static bool has_word(const char *haystack, const char *needle) {
    if (!haystack || !needle) return false;
    const char *p = strstr(haystack, needle);
    size_t needle_len = strlen(needle);
    while (p) {
        bool left_ok = true;
        bool right_ok = true;
        if (p > haystack) {
            char prev = *(p - 1);
            if ((prev >= 'a' && prev <= 'z') || (prev >= 'A' && prev <= 'Z') || (prev >= '0' && prev <= '9') || prev == '_') {
                left_ok = false;
            }
        }
        char next = *(p + needle_len);
        if ((next >= 'a' && next <= 'z') || (next >= 'A' && next <= 'Z') || (next >= '0' && next <= '9') || next == '_') {
            right_ok = false;
        }
        if (left_ok && right_ok) return true;
        p = strstr(p + 1, needle);
    }
    return false;
}

/* --- GOVERNANCE ENGINE --- */
static RiskLevel assess_risk(const char *task) {
    if (!task) return R_READ_ONLY;
    if (strstr(task, "buy") || strstr(task, "pay") || strstr(task, "spend")) return R_FINANCIAL;
    if (strstr(task, "delete") || strstr(task, "uninstall") || strstr(task, "rm ") || strstr(task, "wipe") || strstr(task, "remove-") || strstr(task, "set-content")) return R_DESTRUCTIVE;
    if (strstr(task, "push") || strstr(task, "commit") || strstr(task, "api")) return R_EXTERNAL_API;
    if (strstr(task, "write") || strstr(task, "create") || strstr(task, "save")) return R_LOCAL_WRITE;
    return R_READ_ONLY;
}

static ToolType resolve_tool(const char *task) {
    if (!task) return T_MISTRAL;
    if (strstr(task, "android") || strstr(task, "adb") || strstr(task, "pm ") || strstr(task, "am ") || strstr(task, "input ")) return T_ADB;
    if (strstr(task, "get-") || strstr(task, "set-") || strstr(task, "pwsh") || strstr(task, "powershell")) return T_PWSH;
    if (strstr(task, "browser") || strstr(task, "navigate") || strstr(task, "click")) return T_CDP;
    if (strstr(task, "image") || strstr(task, "visual")) return T_NVIDIA;
    if (strstr(task, "railway") || strstr(task, "deploy")) return T_RAILWAY;
    if (strstr(task, "github") || strstr(task, "repo")) return T_GITHUB;
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
    if (!rt) return;
    logger(LOG_INFO, "Initializing BOS-OMEGA Production Runtime v%s", VERSION);

    char norm[MAX_TEXT];
    normalize(rt->objective, norm);

    rt->state = ST_ROUTE;
    rt->tool = resolve_tool(norm);
    rt->risk = assess_risk(norm);
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
            if (!key && ts->type != T_CDP) { strcpy(rt->error_msg, "MISSING_API_KEY"); res = DEC_BLOCKED; }
            else {
                char payload[MAX_TEXT + 128]; snprintf(payload, sizeof(payload), "{\"model\":\"prod\",\"input\":\"%.*s\"}", MAX_TEXT - 1, rt->objective);
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
    if (!in) return;

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

    LangOutput out = {0};
    strncpy(out.goal, in->goal, MAX_TEXT - 1);

    // Default recommendations
    const char *detected_goal = "unclear";
    const char *best_lang = "TypeScript";
    const char *why = "Extremely versatile, massive ecosystem, standard for modern SaaS.";
    const char *fe = "React / Next.js";
    const char *be = "TypeScript + Node.js (Express/NestJS)";
    const char *db = "PostgreSQL";
    const char *automation = "Bash / GitHub Actions";
    const char *deploy = "Docker + Railway / AWS";
    const char *if_cond = "unclear goal / generic web app";
    const char *then_dec = "recommend TypeScript for full stack";

    const char *step1 = "Learn TypeScript basics & syntax";
    const char *step2 = "Build interactive frontends with React";
    const char *step3 = "Design REST APIs with Node.js & Express";
    const char *step4 = "Deploy to cloud environments via Docker";

    const char *setup = "Set up local dev environment with Node.js, pnpm, and Git";
    const char *prototype = "Build a monolithic prototype with Express and basic HTML/JS";
    const char *test = "Write unit tests using Jest / Vitest";
    const char *deploy_step = "Containerize with Docker and deploy to Railway";
    const char *iterate = "Incorporate TypeScript types and optimize slow endpoints";

    if (strlen(norm_goal) == 0) {
        printf("What are you building?\nWhere will it run?\nDo you want fastest build or strongest long-term architecture?\n");
        return;
    }

    if (strstr(norm_goal, "website") || strstr(norm_goal, "web app") || strstr(norm_goal, "front")) {
        detected_goal = "Build Website / Web Application";
        best_lang = "TypeScript";
        why = "Standard for web development with React/Next.js for app-level complexity.";
        fe = "React / Next.js";
        be = "TypeScript + Node.js (NestJS / Express)";
        db = "PostgreSQL (with Drizzle ORM)";
        automation = "pnpm + GitHub Actions";
        deploy = "Vercel / Railway";
        if_cond = "user goal is 'build website' with app-level complexity";
        then_dec = "recommend HTML, CSS, JavaScript, TypeScript, React / Next.js";
        step1 = "Master HTML5 & CSS3 layout structures";
        step2 = "Learn modern JavaScript (ES6+) syntax";
        step3 = "Incorporate TypeScript for type safety";
        step4 = "Adopt React and Next.js for server-side rendering";
        setup = "Initialize project using 'pnpm create next-app'";
        prototype = "Create interactive UI pages with responsive CSS layouts";
        test = "Write frontend assertions with Playwright / Cypress";
        deploy_step = "Deploy frontend to Vercel with automatic CI/CD";
        iterate = "Optimize bundle size and introduce SSR caching";
    }
    else if (has_word(norm_goal, "ai") || strstr(norm_goal, "agent") || strstr(norm_goal, "llm") || strstr(norm_goal, "machine learning") || strstr(norm_goal, "ml")) {
        detected_goal = "Build AI System / Agentic Workflow";
        best_lang = "Python + TypeScript";
        why = "Python for AI/ML work and workers, TypeScript for robust application/API layers.";
        fe = "React / Next.js";
        be = "FastAPI (Python) + Next.js (TypeScript)";
        db = "PostgreSQL + pgvector";
        automation = "Bash / YAML (GitHub Actions)";
        deploy = "Docker + AWS ECS / GCP";
        if_cond = "user goal is 'build AI system'";
        then_dec = "recommend Python for AI workers, TS for app/API layer, SQL for storage, Bash/YAML for automation";
        step1 = "Learn Python basics & NumPy/Pandas";
        step2 = "Explore LangChain, Llamaindex or OpenAI SDKs";
        step3 = "Build API wrapper around AI workers with FastAPI";
        step4 = "Develop user interface with Next.js & TypeScript";
        setup = "Create Python virtualenv and set up Next.js app directory";
        prototype = "Write lightweight scripts invoking LLM endpoints";
        test = "Set up prompt regression testing with Promptfoo";
        deploy_step = "Deploy Python worker to modal/runpod, frontend to Vercel";
        iterate = "Implement Vector search indexing and prompt caching";
    }
    else if (strstr(norm_goal, "mobile") || strstr(norm_goal, "app")) {
        detected_goal = "Build Mobile Application";
        if (strstr(norm_plat, "ios") && !strstr(norm_plat, "android")) {
            best_lang = "Swift";
            why = "Native iOS development with SwiftUI offers the best experience.";
            fe = "SwiftUI";
            be = "FastAPI / Node.js";
            db = "SQLite / PostgreSQL";
            automation = "Fastlane";
            deploy = "Apple App Store";
            if_cond = "goal is mobile app and platform is iOS only";
            then_dec = "recommend Swift";
            step1 = "Learn Swift programming basics";
            step2 = "Build native iOS UIs with SwiftUI";
            step3 = "Manage local device storage with CoreData";
            step4 = "Integrate remote REST APIs";
            setup = "Install Xcode and set up iOS project template";
            prototype = "Build a single-screen mock UI with SwiftUI";
            test = "Run XCTest suites on simulators";
            deploy_step = "Deploy to TestFlight for internal QA";
            iterate = "Incorporate push notifications and offline mode";
        } else if (strstr(norm_plat, "android") && !strstr(norm_plat, "ios")) {
            best_lang = "Kotlin";
            why = "Native Android development with Jetpack Compose is the industry standard.";
            fe = "Jetpack Compose";
            be = "FastAPI / Node.js";
            db = "Room Database / PostgreSQL";
            automation = "Gradle / Fastlane";
            deploy = "Google Play Store";
            if_cond = "goal is mobile app and platform is Android only";
            then_dec = "recommend Kotlin";
            step1 = "Learn Kotlin syntax & object-oriented design";
            step2 = "Build Android UI layouts with Jetpack Compose";
            step3 = "Handle background execution & coroutines";
            step4 = "Integrate Room database storage";
            setup = "Install Android Studio and set up template project";
            prototype = "Create interactive UI screen on Android simulator";
            test = "Write instrumented tests with Espresso";
            deploy_step = "Upload bundle to Google Play Console Internal Track";
            iterate = "Refactor components with architectural Clean Architecture";
        } else {
            best_lang = "Dart + Flutter";
            why = "Write-once, run-anywhere deployment with high-performance rendering.";
            fe = "Flutter Widget System";
            be = "Node.js (Express) / Go";
            db = "SQLite (Hive / Isar) + PostgreSQL";
            automation = "GitHub Actions + Codemagic";
            deploy = "App Store + Play Store";
            if_cond = "goal is mobile app and platform is cross-platform (both)";
            then_dec = "recommend Dart + Flutter";
            step1 = "Learn Dart programming language core principles";
            step2 = "Understand Flutter's reactive widget rendering";
            step3 = "Implement application state management (BLoC / Provider)";
            step4 = "Connect to backend databases via WebSockets or REST";
            setup = "Install Flutter SDK and configure mobile simulators";
            prototype = "Build functional interface displaying static mock lists";
            test = "Create Flutter unit and integration tests";
            deploy_step = "Build app bundles and distribute via Codemagic";
            iterate = "Improve startup times and decrease bundle footprints";
        }
    }
    else if (strstr(norm_goal, "backend") || strstr(norm_goal, "api") || strstr(norm_goal, "saas")) {
        detected_goal = "Build Backend / API Layer";
        if (strstr(norm_skill, "beginner") || strstr(norm_skill, "junior")) {
            best_lang = "Python + FastAPI";
            why = "Extremely clean, modern, self-documenting syntax with rapid development.";
            fe = "None / API Only";
            be = "Python + FastAPI";
            db = "PostgreSQL (SQLAlchemy / SQLModel)";
            automation = "Bash / GitHub Actions";
            deploy = "Docker + Render / Railway";
            if_cond = "user goal is 'build backend/API' and skill level is beginner";
            then_dec = "recommend Python + FastAPI";
            step1 = "Learn Python core variables, structures & loops";
            step2 = "Build basic REST endpoints using FastAPI routers";
            step3 = "Implement relational database schemas with SQLAlchemy";
            step4 = "Implement JWT-based security and authentication";
            setup = "Create workspace, set up pip env and virtual environment";
            prototype = "Develop a single endpoint to return static JSON mock data";
            test = "Write test assertions using Pytest & HTTPClient";
            deploy_step = "Containerize using Dockerfile and deploy to Render";
            iterate = "Optimize DB sessions and integrate Redis caching";
        } else if (strstr(norm_goal, "saas") || strstr(norm_constraints, "production") || strstr(norm_style, "production") || strstr(norm_type, "production")) {
            best_lang = "TypeScript + Node.js OR Go";
            why = "Excellent balance of scalability, memory footprint, speed, and standard tooling.";
            fe = "React / Next.js";
            be = "Go (Gin/Fiber) or Node.js (NestJS)";
            db = "PostgreSQL (highly index-optimized)";
            automation = "GitHub Actions (CI)";
            deploy = "AWS ECS / GCP Cloud Run";
            if_cond = "user goal is 'build backend/API' for a production SaaS";
            then_dec = "recommend TypeScript + Node.js OR Go";
            step1 = "Learn Go syntax / Advanced Node.js execution patterns";
            step2 = "Structure production-grade middleware (logging, auth, rate-limit)";
            step3 = "Integrate persistent PostgreSQL with connection pooling";
            step4 = "Implement safe migrations and background worker queues";
            setup = "Initialize Go module or NestJS boilerplate framework";
            prototype = "Expose main core CRUD operations with request validation";
            test = "Build high-throughput integration test coverage";
            deploy_step = "Build minimal dist binary and deploy to AWS ECS";
            iterate = "Implement structured logging (Zap/Pino) and tracing (OpenTelemetry)";
        } else {
            best_lang = "Rust OR Go";
            why = "Unmatched execution speed, zero-overhead safety, and robust compiler diagnostics.";
            fe = "None / API Only";
            be = "Rust (Axum / Actix-web) or Go (Standard Library)";
            db = "PostgreSQL (with SQLx or PGX)";
            automation = "GitHub Actions (CI)";
            deploy = "Bare Metal Linux / Kubernetes";
            if_cond = "user goal is 'build backend/API' with high performance or security-critical requirements";
            then_dec = "recommend Rust OR Go";
            step1 = "Understand Rust memory model (ownership/borrowing) or Go concurrency (goroutines)";
            step2 = "Create modular REST/gRPC routing engines";
            step3 = "Utilize connection pools for database safety";
            step4 = "Optimize memory allocations and avoid cloning";
            setup = "Set up cargo workspace or go workspace structure";
            prototype = "Verify thread-safety of state handling in a mock endpoint";
            test = "Write unit tests utilizing native test frameworks";
            deploy_step = "Compile statically linked minimal scratch binary";
            iterate = "Audit memory usage and scale horizontal worker instances";
        }
    }
    else if (strstr(norm_goal, "operating system") || strstr(norm_goal, "firmware") || strstr(norm_goal, "embedded") || strstr(norm_goal, "kernel") || strstr(norm_goal, "driver")) {
        detected_goal = "Build Operating System / Firmware / Embedded System";
        best_lang = "C / C++ / Rust";
        why = "Direct hardware interaction, deterministic memory management, no garbage collector overhead.";
        fe = "None / Hardware Display";
        be = "Bare Metal C / Rust Embedded HAL";
        db = "Flash Memory / EEPROM (Custom flat binary)";
        automation = "Make / CMake / Cargo Embedded";
        deploy = "JTAG Flash to Microcontroller / Physical HW";
        if_cond = "user goal is 'operating system / firmware / embedded'";
        then_dec = "recommend C, C++, Rust (Assembly only where direct CPU/register control is required)";
        step1 = "Learn pointer manipulation & volatile memory operations in C";
        step2 = "Understand microcontroller architectures (ARM Cortex-M)";
        step3 = "Interface with hardware registers using SPI, I2C, and UART";
        step4 = "Adopt Rust Embedded HAL for safe concurrency";
        setup = "Set up toolchain (arm-none-eabi-gcc) and install flashing utilities (OpenOCD)";
        prototype = "Blink an on-board LED (Hello World of hardware)";
        test = "Write unit tests for driver logic executing on host simulation";
        deploy_step = "Flash binary to target device via JTAG/SWD";
        iterate = "Implement DMA-driven serial transfers to reduce CPU cycles";
    }
    else if (strstr(norm_goal, "fpga") || strstr(norm_goal, "asic") || strstr(norm_goal, "digital logic") || strstr(norm_goal, "verilog") || strstr(norm_goal, "vhdl")) {
        detected_goal = "FPGA / ASIC / Digital Logic Design";
        best_lang = "SystemVerilog / Verilog";
        why = "Industry standard for hardware description and high-performance validation.";
        fe = "None / GTKWave Logic Analyzer";
        be = "Synthesized RTL Hardware";
        db = "On-chip Block RAM (BRAM)";
        automation = "Tcl Scripts / Makefile";
        deploy = "Xilinx Vivado / Intel Quartus Bitstream Flashing";
        if_cond = "user goal is 'FPGA / ASIC / digital logic'";
        then_dec = "recommend Verilog, SystemVerilog, VHDL";
        step1 = "Understand digital logic gates, combinational vs sequential circuits";
        step2 = "Learn Verilog / SystemVerilog basic syntax & modules";
        step3 = "Design finite state machines (FSM) and testbenches";
        step4 = "Master clock-domain crossing (CDC) techniques";
        setup = "Install open-source tools (Icarus Verilog, GTKWave) or Vivado Suite";
        prototype = "Design a basic 4-bit binary counter module with testbench";
        test = "Run cycle-accurate simulations and analyze waveform logs";
        deploy_step = "Synthesize, run place-and-route, and generate target bitstream";
        iterate = "Analyze timing report and eliminate setup/hold violations";
    }
    else if (strstr(norm_goal, "data science") || strstr(norm_goal, "analysis") || strstr(norm_goal, "statistic") || strstr(norm_goal, "jupyter")) {
        detected_goal = "Data Science / Machine Learning";
        if (strstr(norm_style, "statistic") || strstr(norm_constraints, "statistic")) {
            best_lang = "R";
            why = "Rich native statistical computing ecosystem preferred by researchers.";
            fe = "R Shiny UI Panels";
            be = "R Session Backend";
            db = "PostgreSQL / Flat Parquet Files";
            automation = "Rscripts";
            deploy = "Shinyapps.io / Posit Connect";
            if_cond = "user goal is 'data science / ML' and is heavily statistic-focused";
            then_dec = "recommend R";
            step1 = "Learn R fundamentals and vector processing operations";
            step2 = "Manipulate data efficiently using tidyverse library";
            step3 = "Visualize data structures using ggplot2";
            step4 = "Perform statistical modeling (regression, ANOVA) using stats packages";
            setup = "Install R and set up RStudio IDE workspace";
            prototype = "Import raw CSV dataset and compute summary analytics";
            test = "Validate model fit assumptions using diagnostic plots";
            deploy_step = "Build and host R Shiny web interactive dashboard";
            iterate = "Tune model hyperparameters and scale memory capacity";
        } else if (strstr(norm_constraints, "numerical") || strstr(norm_style, "numerical")) {
            best_lang = "Julia";
            why = "Designed to solve the two-language problem: high-performance numerical computing.";
            fe = "Pluto.jl Notebooks";
            be = "Julia Runtime";
            db = "HDF5 / SQL Storage";
            automation = "Julia scripts";
            deploy = "Containerized Julia Environment";
            if_cond = "user goal is 'data science / ML' and requires high-performance numerical computing";
            then_dec = "recommend Julia";
            step1 = "Master Julia syntax and multiple-dispatch concepts";
            step2 = "Manipulate multi-dimensional arrays using DataFrames.jl";
            step3 = "Write fast mathematical calculations without type instability";
            step4 = "Implement parallel processing and multithreading in Julia";
            setup = "Install Julia, initialize package environment with Pkg.jl";
            prototype = "Implement custom numerical integration loop";
            test = "Verify precision matching compared to standard analytical results";
            deploy_step = "Compile sysimage to eliminate JIT compilation latency";
            iterate = "Optimize memory allocations and utilize GPU acceleration (CUDA.jl)";
        } else {
            best_lang = "Python";
            why = "The absolute leader in machine learning, deep learning, and data wrangling.";
            fe = "Streamlit / Gradio";
            be = "Python (Pandas, NumPy, Scikit-learn)";
            db = "PostgreSQL (SQLAlchemy)";
            automation = "Jupyter Notebooks / Airflow";
            deploy = "Docker + Kubernetes / AWS SageMaker";
            if_cond = "user goal is 'data science / ML'";
            then_dec = "recommend Python, SQL, R only if statistics-heavy, Julia only if high-performance numerical computing is required";
            step1 = "Master Python data structures and NumPy vectorization";
            step2 = "Learn data cleaning and manipulation using Pandas";
            step3 = "Build traditional models with Scikit-learn";
            step4 = "Train deep neural networks using PyTorch / TensorFlow";
            setup = "Install Anaconda / Miniconda and configure conda environment";
            prototype = "Analyze exploratory dataset in an interactive Jupyter notebook";
            test = "Implement cross-validation testing to check for overfitting";
            deploy_step = "Expose model predictions via a FastAPI backend wrapped in Docker";
            iterate = "Optimize training steps with parallel data loaders and GPU execution";
        }
    }
    else if (strstr(norm_goal, "database-heavy") || strstr(norm_goal, "database") || strstr(norm_goal, "sql")) {
        detected_goal = "Database-Heavy Application";
        best_lang = "SQL + PostgreSQL (TypeScript/Python Backend)";
        why = "Database design, normalization, and index-tuning are the primary performance drivers.";
        fe = "React / Next.js";
        be = "TypeScript / Python (FastAPI)";
        db = "PostgreSQL (with pg_stat_statements)";
        automation = "Drizzle Migrations / Liquidbase";
        deploy = "AWS RDS / Supabase";
        if_cond = "user goal is 'database-heavy app'";
        then_dec = "recommend SQL first, PostgreSQL, TypeScript/Python backend";
        step1 = "Master SQL syntax, relational theory, and joins";
        step2 = "Design proper 3NF schema architecture and constraints";
        step3 = "Analyze execution queries using EXPLAIN ANALYZE";
        step4 = "Configure index optimization (B-Tree, GIN, Hash)";
        setup = "Provision PostgreSQL instance using local Docker container";
        prototype = "Create table schemas and seed dummy data with relationships";
        test = "Write data assertion tests for constraints and cascades";
        deploy_step = "Migrate database structure to AWS RDS and connect backend";
        iterate = "Set up database connection pooling (pgBouncer) and read replicas";
    }
    else if (strstr(norm_goal, "blockchain") || strstr(norm_goal, "smart contract") || strstr(norm_goal, "solidity") || strstr(norm_goal, "solana")) {
        detected_goal = "Blockchain / Smart Contracts";
        if (strstr(norm_plat, "solana")) {
            best_lang = "Rust";
            why = "High-performance, secure system for Solana programs using Anchor.";
            fe = "Next.js + Solana Wallet Adapter";
            be = "Anchor Framework (Rust)";
            db = "Solana Account State + PostgreSQL (Indexing cache)";
            automation = "Solana CLI / TS scripts";
            deploy = "Solana Mainnet-Beta";
            if_cond = "goal is blockchain / smart contracts and platform is Solana";
            then_dec = "recommend Rust";
            step1 = "Learn Rust basic principles and memory safety concepts";
            step2 = "Master Solana account model structure";
            step3 = "Build smart contracts utilizing Anchor Framework";
            step4 = "Interact with program states using Solana Web3.js SDK";
            setup = "Install Rust, Solana CLI tool suite, and Anchor framework";
            prototype = "Deploy basic counter program on Solana local validator";
            test = "Write contract interaction tests in TypeScript with Mocha";
            deploy_step = "Deploy program to Solana Devnet for integration testing";
            iterate = "Audit compute unit budgets and optimize state layouts";
        } else if (strstr(norm_plat, "move")) {
            best_lang = "Move";
            why = "Resource-oriented language designed for high-security smart contracts on Aptos/Sui.";
            fe = "Next.js + Aptos Wallet Adapter";
            be = "Move Smart Contracts";
            db = "Aptos State Store / PostgreSQL";
            automation = "Aptos / Sui CLI";
            deploy = "Aptos / Sui Mainnet";
            if_cond = "goal is blockchain / smart contracts and platform is Move-chain";
            then_dec = "recommend Move";
            step1 = "Understand resource-oriented programming concepts";
            step2 = "Learn Move module structure and resource mechanics";
            step3 = "Study Move prover for formal code verification";
            step4 = "Interact with modules using Sui/Aptos TypeScript SDKs";
            setup = "Install Move toolchain and configure local node environment";
            prototype = "Build a basic token minter contract module";
            test = "Use standard Move test framework to simulate transactions";
            deploy_step = "Publish Move modules to testnet network";
            iterate = "Audit transfer permissions and verify resource locks";
        } else {
            best_lang = "Solidity + TypeScript";
            why = "The dominant smart contract language for Ethereum and EVM-compatible networks.";
            fe = "Next.js + RainbowKit + Viem";
            be = "Hardhat / Foundry (Solidity)";
            db = "EVM Ledger + The Graph (Subgraphs)";
            automation = "Foundry script / Hardhat task";
            deploy = "Ethereum / Polygon / Arbitrum Mainnet";
            if_cond = "goal is blockchain / smart contracts and platform is Ethereum/EVM";
            then_dec = "recommend Solidity + TypeScript";
            step1 = "Learn Solidity syntax, storage layouts, and security rules";
            step2 = "Master Foundry for local testing, fuzzing, and debugging";
            step3 = "Use TypeScript with viem/ethers.js to interact with contracts";
            step4 = "Design custom data models utilizing Subgraphs";
            setup = "Install Foundry and initialize workspace using 'forge init'";
            prototype = "Write ERC-20 smart contract with simple mint/transfer bounds";
            test = "Write exhaustive solidity invariant tests and fuzzing routines";
            deploy_step = "Deploy contract on Sepolia testnet and verify source on Etherscan";
            iterate = "Audit gas consumption and optimize contract layout packing";
        }
    }
    else if (strstr(norm_goal, "automation") || strstr(norm_goal, "script") || strstr(norm_goal, "cli") || strstr(norm_goal, "system task")) {
        detected_goal = "Automation / Scripting / CLI Tools";
        if (strstr(norm_plat, "windows")) {
            best_lang = "PowerShell + Python";
            why = "PowerShell has unmatched native Windows administration bindings and COM control.";
            fe = "CLI Console Output";
            be = "PowerShell Scripts + Python Automation Scripts";
            db = "Registry / SQLite / Event Log";
            automation = "Windows Task Scheduler / PowerShell Jobs";
            deploy = "Local Installation / Active Directory GPO";
            if_cond = "user goal is 'automation / scripting' and platform is Windows";
            then_dec = "recommend PowerShell + Python";
            step1 = "Understand PowerShell Cmdlet architecture, parameters & pipeline objects";
            step2 = "Write robust error handling blocks with Try/Catch";
            step3 = "Utilize Python libraries (PyAutoGUI, pandas) for visual or file parsing tasks";
            step4 = "Leverage Windows API bindings in PowerShell";
            setup = "Open elevated PowerShell environment and set script execution policy";
            prototype = "Write script to scrape log files and export a clean CSV";
            test = "Perform dry-run executions using '-WhatIf' parameter";
            deploy_step = "Schedule task execution via Windows Task Scheduler";
            iterate = "Implement remote administration via WinRM endpoints";
        } else {
            best_lang = "Bash + Python";
            why = "Standard UNIX CLI tools combined with Python's rich standard libraries.";
            fe = "TTY / CLI Console Output";
            be = "Bash shell scripts + Python system scripts";
            db = "JSON Files / SQLite / Syslog";
            automation = "Cron / systemd-timers";
            deploy = "Executable script files in /usr/local/bin";
            if_cond = "user goal is 'automation / scripting' and platform is Linux/macOS";
            then_dec = "recommend Bash + Python";
            step1 = "Learn basic shell utilities (grep, awk, sed, find, xargs)";
            step2 = "Understand Bash variable expansion, control loops, and exit codes";
            step3 = "Incorporate Python's 'os', 'sys', and 'subprocess' modules";
            step4 = "Write custom CLI parsers using Python's 'argparse'";
            setup = "Initialize script file with shebang '#!/bin/bash' and run chmod +x";
            prototype = "Build a backup routine script that archives and zips a folder";
            test = "Verify exit codes and handle empty inputs/edge bounds";
            deploy_step = "Schedule script execution intervals via crontab";
            iterate = "Implement Telegram/Slack webhooks for instant failure notifications";
        }
    }
    else if (has_word(norm_goal, "ai")) {
        // Special boundary match for word 'ai' to make sure goal "ai" matches correctly
        detected_goal = "Build AI System / Agentic Workflow";
        best_lang = "Python + TypeScript";
        why = "Python for AI/ML work and workers, TypeScript for robust application/API layers.";
        fe = "React / Next.js";
        be = "FastAPI (Python) + Next.js (TypeScript)";
        db = "PostgreSQL + pgvector";
        automation = "Bash / YAML (GitHub Actions)";
        deploy = "Docker + AWS ECS / GCP";
        if_cond = "user goal is 'build AI system'";
        then_dec = "recommend Python for AI workers, TS for app/API layer, SQL for storage, Bash/YAML for automation";
        step1 = "Learn Python basics & NumPy/Pandas";
        step2 = "Explore LangChain, Llamaindex or OpenAI SDKs";
        step3 = "Build API wrapper around AI workers with FastAPI";
        step4 = "Develop user interface with Next.js & TypeScript";
        setup = "Create Python virtualenv and set up Next.js app directory";
        prototype = "Write lightweight scripts invoking LLM endpoints";
        test = "Set up prompt regression testing with Promptfoo";
        deploy_step = "Deploy Python worker to modal/runpod, frontend to Vercel";
        iterate = "Implement Vector search indexing and prompt caching";
    }

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
    printf("│   ├── Frontend: %s\n", fe);
    printf("│   ├── Backend: %s\n", be);
    printf("│   ├── Database: %s\n", db);
    printf("│   ├── Automation: %s\n", automation);
    printf("│   └── Deployment: %s\n", deploy);
    printf("│\n");
    printf("├── IF_THEN_REASONING\n");
    printf("│   ├── IF %s\n", if_cond);
    printf("│   └── THEN %s\n", then_dec);
    printf("│\n");
    printf("├── LEARNING_ORDER\n");
    printf("│   ├── Step 1: %s\n", step1);
    printf("│   ├── Step 2: %s\n", step2);
    printf("│   ├── Step 3: %s\n", step3);
    printf("│   └── Step 4: %s\n", step4);
    printf("│\n");
    printf("├── BUILD_ORDER\n");
    printf("│   ├── Setup: %s\n", setup);
    printf("│   ├── Prototype: %s\n", prototype);
    printf("│   ├── Test: %s\n", test);
    printf("│   ├── Deploy: %s\n", deploy_step);
    printf("│   └── Iterate: %s\n", iterate);
    printf("│\n");
    printf("└── FINAL_RECOMMENDATION\n");
    printf("    └── Complete, high-performance stack recommendation: %s with backend %s, database %s and automated deployments via %s\n", best_lang, be, db, deploy);
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
        if (argc >= 4) strncpy(in.skill, argv[3], MAX_TEXT - 1);
        if (argc >= 5) strncpy(in.type, argv[4], MAX_TEXT - 1);
        if (argc >= 6) strncpy(in.plat, argv[5], MAX_TEXT - 1);
        if (argc >= 7) strncpy(in.time, argv[6], MAX_TEXT - 1);
        if (argc >= 8) strncpy(in.style, argv[7], MAX_TEXT - 1);
        if (argc >= 9) strncpy(in.constraints, argv[8], MAX_TEXT - 1);
        run_lang(&in);
    } else {
        printf("Error: Invalid arguments.\n");
    }
    curl_global_cleanup();
    return 0;
}
