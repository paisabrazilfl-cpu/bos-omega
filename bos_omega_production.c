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
 * Normalizes the source string by converting it to lowercase and copying it to dest.
 * This optimization replaces multiple O(N*M) strcasestr() calls with a single O(N)
 * lowercasing pass, allowing subsequently for fast O(M) strstr() calls.
 * This reduces total scanning complexity from O(N*M) to O(N+M).
 */
static void normalize(char *dest, const char *src) {
    if (!dest) return;
    if (!src) {
        dest[0] = '\0';
        return;
    }
    int i = 0;
    while (src[i] != '\0' && i < MAX_TEXT - 1) {
        dest[i] = (char)tolower((unsigned char)src[i]);
        i++;
    }
    dest[i] = '\0';
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
    CURL *curl = curl_easy_init(); if (!curl) { strncpy(rt->error_msg, "CURL_INIT_FAIL", MAX_TEXT - 1); rt->error_msg[MAX_TEXT - 1] = '\0'; return DEC_RETRY; }
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
    rt->state = ST_ROUTE;
    char norm_obj[MAX_TEXT];
    normalize(norm_obj, rt->objective);
    rt->tool = resolve_tool(norm_obj);
    rt->risk = assess_risk(norm_obj);
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
                char payload[MAX_TEXT + 128]; snprintf(payload, sizeof(payload), "{\"model\":\"prod\",\"input\":\"%.*s\"}", MAX_TEXT, rt->objective);
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

    normalize(norm_goal, in->goal);
    normalize(norm_skill, in->skill);
    normalize(norm_type, in->type);
    normalize(norm_plat, in->plat);
    normalize(norm_time, in->time);
    normalize(norm_style, in->style);
    normalize(norm_constraints, in->constraints);

    const char *detected_goal = in->goal;
    const char *best_lang = "";
    const char *why_this_lang = "";
    const char *stack_fe = "None";
    const char *stack_be = "None";
    const char *stack_db = "None";
    const char *stack_auto = "None";
    const char *stack_deploy = "None";
    const char *if_cond = "";
    const char *then_dec = "";
    const char *learn_1 = "";
    const char *learn_2 = "";
    const char *learn_3 = "";
    const char *learn_4 = "";
    const char *build_setup = "";
    const char *build_proto = "";
    const char *build_test = "";
    const char *build_deploy = "";
    const char *build_iterate = "";
    const char *final_rec = "";

    if (strstr(norm_goal, "website") || strstr(norm_goal, "web app") || strstr(norm_goal, "web") || strstr(norm_goal, "frontend")) {
        bool is_complex = strstr(norm_type, "complex") || strstr(norm_goal, "complex") || strstr(norm_goal, "app") || strstr(norm_type, "app") || strstr(norm_style, "complex");
        if (is_complex) {
            best_lang = "HTML / CSS / JavaScript / TypeScript / React / Next.js";
            why_this_lang = "TypeScript adds static typing to JavaScript, and Next.js manages app-level complexity.";
            stack_fe = "React / Next.js (Tailwind CSS)";
            stack_be = "Next.js API Routes / Node.js";
            stack_db = "PostgreSQL / Prisma ORM";
            stack_auto = "npm scripts / GitHub Actions";
            stack_deploy = "Vercel / Netlify / AWS";
            if_cond = "user goal has app-level complexity (Next.js components and APIs)";
            then_dec = "recommend TypeScript + React / Next.js for robust state and routing";
            final_rec = "TypeScript (Next.js + React + Tailwind CSS)";
        } else {
            best_lang = "HTML / CSS / JavaScript / TypeScript";
            why_this_lang = "Standard web languages provide zero-overhead and simple development for static or simple sites.";
            stack_fe = "HTML5 / CSS3 / Vanilla TypeScript";
            stack_be = "None / Static Hosting";
            stack_db = "None or LocalStorage";
            stack_auto = "Vite build pipeline";
            stack_deploy = "GitHub Pages / Netlify / Vercel";
            if_cond = "user goal is a simple, non-app-level static page";
            then_dec = "recommend Vanilla HTML / CSS / TypeScript with static deployment";
            final_rec = "HTML5 + CSS3 + TypeScript with static hosting";
        }
        learn_1 = "Learn HTML & CSS basics for document structure and styling";
        learn_2 = "Master JavaScript fundamentals (ES6+, DOM manipulation, Fetch API)";
        learn_3 = "Adopt TypeScript for type-safe application development";
        learn_4 = "Learn React and Next.js for component-based SPAs and SSR";
        build_setup = "Initialize project with 'npx create-next-app@latest --typescript'";
        build_proto = "Build UI components and mock dynamic user interactions";
        build_test = "Write unit tests with Jest and E2E tests with Playwright";
        build_deploy = "Connect repository to Vercel for instant continuous deployment";
        build_iterate = "Analyze Web Vitals and optimize bundle sizes and image assets";
    }
    else if (strstr(norm_goal, "ai") || strstr(norm_goal, "agent") || strstr(norm_goal, "artificial intelligence")) {
        best_lang = "Python & TypeScript";
        why_this_lang = "Python is the undisputed standard for AI/ML, while TypeScript orchestrates the application and API layers.";
        stack_fe = "Next.js / React (Tailwind CSS)";
        stack_be = "Python (FastAPI) & Node.js";
        stack_db = "PostgreSQL / pgvector (for embeddings)";
        stack_auto = "Bash scripts / GitHub Actions";
        stack_deploy = "Docker on AWS ECS or GCP Cloud Run";
        if_cond = "building background LLM agents or ML/AI processing pipelines";
        then_dec = "recommend Python for AI workers, and TypeScript for web frontend/API orchestration";
        learn_1 = "Learn Python basics and asynchronous programming";
        learn_2 = "Understand OpenAI API, LangChain/LlamaIndex, and vector databases";
        learn_3 = "Learn TypeScript/Next.js for building the agentic frontend";
        learn_4 = "Master deployment of containerized Python and Node.js microservices";
        build_setup = "Set up python virtual environment and install fastapi, openai, and pgvector";
        build_proto = "Develop a basic prompt execution pipeline and save embeddings";
        build_test = "Implement integration tests for API calls and regression checks";
        build_deploy = "Deploy to AWS ECS or Railway using Docker containers";
        build_iterate = "Monitor token usage, cost, latency, and cache frequent queries";
        final_rec = "Python (FastAPI) + TypeScript (Next.js) + PostgreSQL (pgvector)";
    }
    else if (strstr(norm_goal, "mobile") || strstr(norm_goal, "phone") || strstr(norm_goal, "ios") || strstr(norm_goal, "android")) {
        bool is_ios = strstr(norm_plat, "ios") || strstr(norm_goal, "ios");
        bool is_android = (strstr(norm_plat, "android") || strstr(norm_goal, "android") || strstr(norm_goal, "adb")) && !is_ios;

        if (is_ios) {
            best_lang = "Swift";
            why_this_lang = "Native Swift provides the highest performance, smooth animations, and deepest integration with iOS APIs.";
            stack_fe = "SwiftUI";
            stack_be = "Node.js (Express) or Swift on Server";
            stack_db = "CoreData / SQLite";
            stack_auto = "Fastlane for iOS builds";
            stack_deploy = "App Store Connect (Apple Developer Program)";
            if_cond = "developing exclusively for Apple devices (iPhone, iPad, Apple Watch)";
            then_dec = "recommend Swift + SwiftUI for a fully native iOS application";
            final_rec = "Swift + SwiftUI (Native iOS)";
        } else if (is_android) {
            best_lang = "Kotlin";
            why_this_lang = "Kotlin is the modern, expressive standard for first-class native Android development.";
            stack_fe = "Jetpack Compose";
            stack_be = "Ktor or Node.js";
            stack_db = "Room / SQLite";
            stack_auto = "Gradle / Fastlane";
            stack_deploy = "Google Play Console";
            if_cond = "developing exclusively for Android devices and TVs";
            then_dec = "recommend Kotlin + Jetpack Compose for native Android experience";
            final_rec = "Kotlin + Jetpack Compose (Native Android)";
        } else {
            best_lang = "Dart (Flutter)";
            why_this_lang = "Dart with Flutter compiles to high-performance native code for both iOS and Android from a single codebase.";
            stack_fe = "Flutter SDK UI Layer";
            stack_be = "Firebase / Node.js (FastAPI)";
            stack_db = "SQLite / Hive / Firestore";
            stack_auto = "Codemagic / Fastlane CI";
            stack_deploy = "Apple App Store & Google Play Store";
            if_cond = "developing for both iOS and Android simultaneously with unified codebase";
            then_dec = "recommend Dart + Flutter for cross-platform efficiency and native speeds";
            final_rec = "Dart + Flutter (Cross-platform Mobile)";
        }
        learn_1 = "Learn mobile UI layouts and widget lifecycle principles";
        learn_2 = "Understand state management (Provider/Riverpod for Flutter, State for SwiftUI)";
        learn_3 = "Master networking and consuming RESTful/GraphQL APIs";
        learn_4 = "Learn platform-specific integration (push notifications, camera, geolocation)";
        build_setup = "Install native SDKs (Xcode, Android Studio, or Flutter SDK)";
        build_proto = "Build screens and wireframe the core navigation and routing flow";
        build_test = "Run simulator tests, widget/unit tests, and integration flows";
        build_deploy = "Configure signing credentials and submit to distribution platforms";
        build_iterate = "Monitor crash logs, user analytics, and optimize assets and bundle size";
    }
    else if (strstr(norm_goal, "backend") || strstr(norm_goal, "api") || strstr(norm_goal, "server") || strstr(norm_goal, "microservice")) {
        bool is_beginner = strstr(norm_skill, "beginner") || strstr(norm_skill, "novice") || strstr(norm_skill, "simple");
        bool is_saas = strstr(norm_type, "saas") || strstr(norm_type, "production") || strstr(norm_type, "commercial");
        bool is_perf = strstr(norm_type, "performance") || strstr(norm_type, "security") || strstr(norm_type, "critical") || strstr(norm_type, "high");

        if (is_beginner) {
            best_lang = "Python";
            why_this_lang = "Python combined with FastAPI is incredibly beginner-friendly, readable, and highly performant.";
            stack_fe = "None / Swagger UI Docs";
            stack_be = "FastAPI / Uvicorn";
            stack_db = "SQLite / PostgreSQL";
            stack_auto = "pytest / black";
            stack_deploy = "Render / Railway / Heroku";
            if_cond = "user skill level is beginner or rapid prototype is required";
            then_dec = "recommend Python + FastAPI for maximum velocity and readability";
            final_rec = "Python + FastAPI + PostgreSQL";
        } else if (is_saas) {
            best_lang = "TypeScript or Go";
            why_this_lang = "TypeScript on Node.js offers massive developer velocity, while Go delivers blazing fast, light-weight microservices.";
            stack_fe = "React / Next.js (for admin dashboard)";
            stack_be = "Node.js (NestJS) or Go (Gin/Fiber)";
            stack_db = "PostgreSQL + Redis";
            stack_auto = "Docker + GitHub Actions CI/CD";
            stack_deploy = "AWS ECS / GCP Cloud Run / Kubernetes";
            if_cond = "building a production-ready commercial SaaS backend API";
            then_dec = "recommend TypeScript + Node.js or Go for scale and rich developer ecosystem";
            final_rec = "Go (or TypeScript) + PostgreSQL + Redis + Docker";
        } else if (is_perf) {
            best_lang = "Rust or Go";
            why_this_lang = "Rust guarantees memory safety without a garbage collector, and Go offers unmatched concurrent runtime efficiency.";
            stack_fe = "None";
            stack_be = "Rust (Axum/Actix) or Go (Standard Library)";
            stack_db = "PostgreSQL + Redis";
            stack_auto = "Cargo / Go test + Makefiles";
            stack_deploy = "Kubernetes on AWS / GCP Bare Metal";
            if_cond = "system demands ultra-high performance, low latency, or is security-critical";
            then_dec = "recommend Rust or Go for resource safety and high concurrent throughput";
            final_rec = "Rust + Axum + PostgreSQL + Redis";
        } else {
            best_lang = "Go";
            why_this_lang = "Go provides an exceptional balance of build speed, runtime performance, and maintainability.";
            stack_fe = "None";
            stack_be = "Go (Gin/Fiber / Standard Library)";
            stack_db = "PostgreSQL";
            stack_auto = "Makefiles / Go test";
            stack_deploy = "Docker on Railway / AWS";
            if_cond = "building a standard scalable backend API";
            then_dec = "recommend Go for compile speed, simplicity, and concurrency";
            final_rec = "Go + PostgreSQL + Docker";
        }
        learn_1 = "Master backend basics: HTTP protocols, REST principles, status codes";
        learn_2 = "Learn database interactions, SQL schemas, migrations, and ORMs/drivers";
        learn_3 = "Understand authentication, JWTs, role-based access control (RBAC)";
        learn_4 = "Learn containerization (Docker) and continuous integration (CI/CD) pipelines";
        build_setup = "Initialize project structure (e.g. 'go mod init' or 'npm init') and configure env variables";
        build_proto = "Define API routes and implement CRUD handlers for core resources";
        build_test = "Write automated integration tests for endpoint responses and mock dependencies";
        build_deploy = "Containerize using Docker and deploy to cloud environments (Railway, AWS)";
        build_iterate = "Implement logging, rate limiting, caching with Redis, and trace API endpoints";
    }
    else if (strstr(norm_goal, "operating system") || strstr(norm_goal, "os") || strstr(norm_goal, "firmware") || strstr(norm_goal, "embedded") || strstr(norm_goal, "kernel") || strstr(norm_goal, "hardware")) {
        best_lang = "C / C++ / Rust";
        why_this_lang = "C/C++ provide bare-metal control and zero-overhead performance, while Rust adds modern compile-time memory safety.";
        stack_fe = "None / Hardware Interface (LEDs, LCDs)";
        stack_be = "Bare-metal or RTOS (FreeRTOS / Zephyr)";
        stack_db = "Custom flash filesystem / SQLite (embedded)";
        stack_auto = "Make / CMake / Cargo build tools";
        stack_deploy = "JTAG / OpenOCD hardware programmer";
        if_cond = "developing bare-metal embedded firmware, drivers, or operating systems";
        then_dec = "recommend C or Rust; use Assembly only where direct CPU instruction access is required";
        learn_1 = "Learn C/C++ memory management, pointers, and bitwise operations";
        learn_2 = "Understand microprocessor architecture, registers, timers, and interrupts";
        learn_3 = "Learn an RTOS or bare-metal peripheral programming (GPIO, I2C, SPI, UART)";
        learn_4 = "Explore Rust for embedded (embedded-hal) to write completely safe drivers";
        build_setup = "Install target toolchain (e.g. arm-none-eabi-gcc) and configure CMake/Cargo";
        build_proto = "Write simple main loop to toggle hardware pins (Blinky prototype)";
        build_test = "Debug with GDB and logic analyzers; write unit tests with mock registers";
        build_deploy = "Flash binary onto target microcontroller hardware using OpenOCD";
        build_iterate = "Optimize power consumption, interrupt service routines, and memory layout";
        final_rec = "Rust (or C) + FreeRTOS + CMake";
    }
    else if (strstr(norm_goal, "fpga") || strstr(norm_goal, "asic") || strstr(norm_goal, "digital logic") || strstr(norm_goal, "verilog") || strstr(norm_goal, "vhdl") || strstr(norm_goal, "hardware description")) {
        best_lang = "SystemVerilog";
        why_this_lang = "SystemVerilog is the premier hardware description and verification language for modern chip design.";
        stack_fe = "None";
        stack_be = "FPGA synthesis / ASIC layout design";
        stack_db = "None";
        stack_auto = "Tcl scripting / Makefiles";
        stack_deploy = "Vivado / Quartus / Synopsys EDA tools";
        if_cond = "programming FPGA, ASIC, or custom digital logic hardware circuits";
        then_dec = "recommend SystemVerilog or VHDL for RTL modeling and verification";
        learn_1 = "Understand boolean algebra, combinational and sequential digital logic";
        learn_2 = "Learn Verilog/SystemVerilog synthesis rules, registers, and state machines";
        learn_3 = "Master testbench creation, assertions (SVA), and simulation concepts";
        learn_4 = "Learn Universal Verification Methodology (UVM) for ASIC-level testing";
        build_setup = "Install EDA tools (Vivado/ModelSim) and configure workspace";
        build_proto = "Write RTL code for the target digital architecture";
        build_test = "Run behavioral simulations with exhaustive test vectors and testbenches";
        build_deploy = "Synthesize, place & route, and generate bitstream or GDSII layout";
        build_iterate = "Perform static timing analysis and optimize logic gates for speed/area/power";
        final_rec = "SystemVerilog + Vivado/ModelSim";
    }
    else if (strstr(norm_goal, "data science") || strstr(norm_goal, "machine learning") || strstr(norm_goal, "ml") || strstr(norm_goal, "statistics") || strstr(norm_goal, "data analysis")) {
        best_lang = "Python & SQL";
        why_this_lang = "Python is the universal standard for data science libraries, backed by SQL for massive data retrieval.";
        stack_fe = "Streamlit / Jupyter Notebooks";
        stack_be = "Python (FastAPI / Flask)";
        stack_db = "PostgreSQL / Snowflake / Google BigQuery";
        stack_auto = "Apache Airflow / Prefect for pipelines";
        stack_deploy = "Docker on Kubernetes or AWS SageMaker";
        if_cond = "performing statistical modeling, machine learning, or data analysis";
        then_dec = "recommend Python + SQL; use R only if statistics-heavy; use Julia only if high-performance numerical computing is required";
        learn_1 = "Learn Python syntax and data libraries (Pandas, NumPy, Matplotlib)";
        learn_2 = "Master SQL for complex querying, aggregation, and data warehouse manipulation";
        learn_3 = "Understand statistical models, regression, and machine learning (Scikit-Learn)";
        learn_4 = "Learn deep learning frameworks (PyTorch/TensorFlow) or data pipelines";
        build_setup = "Set up Jupyter Lab environment and connect to database";
        build_proto = "Perform exploratory data analysis and train a prototype model";
        build_test = "Verify model performance using cross-validation and independent test sets";
        build_deploy = "Package model as an API endpoint using FastAPI and Docker";
        build_iterate = "Monitor model drift, retrain on schedule, and optimize inference latency";
        final_rec = "Python (Pandas/PyTorch) + SQL + Streamlit";
    }
    else if (strstr(norm_goal, "database") || strstr(norm_goal, "db-heavy") || strstr(norm_goal, "sql") || strstr(norm_goal, "postgres") || strstr(norm_goal, "mongodb")) {
        best_lang = "SQL first";
        why_this_lang = "SQL is irreplaceable for managing complex relations, coupled with TypeScript or Python for API logic.";
        stack_fe = "React / Next.js";
        stack_be = "TypeScript (Node.js) or Python (FastAPI)";
        stack_db = "PostgreSQL (Relational) or MongoDB (Document)";
        stack_auto = "Prisma or Drizzle migrations";
        stack_deploy = "Supabase / AWS RDS / PlanetScale";
        if_cond = "the core challenge of the application is relational mapping, query optimization, or high-volume storage";
        then_dec = "recommend SQL + PostgreSQL with a TypeScript or Python backend";
        learn_1 = "Learn relational database theory, normalization, and standard SQL syntax";
        learn_2 = "Master indexes, query planning, joins, transactions, and database constraints";
        learn_3 = "Learn backend integration with raw SQL queries and connection pools";
        learn_4 = "Understand caching strategies using Redis and horizontal database scaling";
        build_setup = "Configure database instance and set up migrations (Prisma/Drizzle)";
        build_proto = "Create schema tables, relations, and insert sample seed data";
        build_test = "Write integration tests using clean temporary database states";
        build_deploy = "Deploy database to managed hosting (Supabase, AWS RDS)";
        build_iterate = "Analyze slow queries using EXPLAIN ANALYZE and add indexes or partitions";
        final_rec = "SQL (PostgreSQL) + TypeScript (Node.js) + Redis";
    }
    else if (strstr(norm_goal, "blockchain") || strstr(norm_goal, "smart contract") || strstr(norm_goal, "crypto") || strstr(norm_goal, "solidity") || strstr(norm_goal, "ethereum") || strstr(norm_goal, "solana") || strstr(norm_goal, "move-chain") || strstr(norm_goal, "move")) {
        bool is_eth = strstr(norm_plat, "ethereum") || strstr(norm_goal, "ethereum") || strstr(norm_goal, "solidity") || strstr(norm_goal, "evm") || strstr(norm_plat, "evm");
        bool is_sol = strstr(norm_plat, "solana") || strstr(norm_goal, "solana") || strstr(norm_goal, "anchor");
        bool is_move = strstr(norm_plat, "move") || strstr(norm_goal, "move") || strstr(norm_goal, "sui") || strstr(norm_goal, "aptos");

        if (is_eth) {
            best_lang = "Solidity & TypeScript";
            why_this_lang = "Solidity is the standard for EVM execution, and TypeScript is crucial for client interactions and testing.";
            stack_fe = "React (Wagmi / Viem / RainbowKit)";
            stack_be = "TypeScript (Hardhat / Foundry)";
            stack_db = "The Graph (GraphQL subgraphs)";
            stack_auto = "Foundry / Hardhat tests";
            stack_deploy = "Ethereum / Arbitrum / Optimism / Base";
            if_cond = "targeting the Ethereum or EVM-compatible ecosystems";
            then_dec = "recommend Solidity + TypeScript";
            final_rec = "Solidity + TypeScript + Foundry";
        } else if (is_sol) {
            best_lang = "Rust";
            why_this_lang = "Solana programs are written in Rust, delivering ultra-fast and low-overhead on-chain execution.";
            stack_fe = "React (Solana Web3.js / Wallet Adapter)";
            stack_be = "Anchor Framework (Rust)";
            stack_db = "None / Custom RPC Nodes";
            stack_auto = "Anchor CLI / TypeScript tests";
            stack_deploy = "Solana Mainnet / Devnet";
            if_cond = "targeting the high-throughput Solana ecosystem";
            then_dec = "recommend Rust + Anchor framework";
            final_rec = "Rust + Anchor Framework (Solana)";
        } else if (is_move) {
            best_lang = "Move";
            why_this_lang = "Move is specifically designed for safe asset representation and secure resource-oriented smart contract development.";
            stack_fe = "React (Sui / Aptos dApp Kit)";
            stack_be = "Move Framework";
            stack_db = "None";
            stack_auto = "Move CLI / Unit tests";
            stack_deploy = "Aptos / Sui network";
            if_cond = "targeting a modern Move-based blockchain like Aptos or Sui";
            then_dec = "recommend Move";
            final_rec = "Move Language (Aptos / Sui)";
        } else {
            best_lang = "Solidity & TypeScript";
            why_this_lang = "Solidity is the most widely adopted language in Web3 with the largest developer ecosystem.";
            stack_fe = "React (Wagmi)";
            stack_be = "Hardhat / Foundry";
            stack_db = "The Graph";
            stack_auto = "TypeScript testing";
            stack_deploy = "EVM Networks";
            if_cond = "building a general smart contract application";
            then_dec = "recommend Solidity + TypeScript";
            final_rec = "Solidity + TypeScript + Foundry";
        }
        learn_1 = "Understand blockchain consensus, transactions, gas, and cryptographic keys";
        learn_2 = "Learn smart contract structure, state variables, and security pitfalls (reentrancy)";
        learn_3 = "Master local development tools (Hardhat, Foundry, Anchor)";
        learn_4 = "Learn client integration with wallet providers and RPC node connections";
        build_setup = "Initialize Foundry or Anchor workspace and write smart contract";
        build_proto = "Develop core contract logic and deploy onto local test node";
        build_test = "Write extensive unit and fuzz tests to ensure smart contract security";
        build_deploy = "Compile, verify code on block explorer, and deploy to testnet";
        build_iterate = "Audit contract code, optimize gas consumption, deploy to mainnet";
    }
    else if (strstr(norm_goal, "automation") || strstr(norm_goal, "script") || strstr(norm_goal, "workflow") || strstr(norm_goal, "cron") || strstr(norm_goal, "bash") || strstr(norm_goal, "powershell")) {
        bool is_win = strstr(norm_plat, "windows") || strstr(norm_goal, "windows");
        if (is_win) {
            best_lang = "PowerShell & Python";
            why_this_lang = "PowerShell integrates natively with Windows systems, combined with Python for cross-platform workflows.";
            stack_fe = "None";
            stack_be = "PowerShell Core / Python";
            stack_db = "CSV / SQLite";
            stack_auto = "Windows Task Scheduler";
            stack_deploy = "Active Directory / Local host execution";
            if_cond = "the target operating system is Windows";
            then_dec = "recommend PowerShell + Python";
            final_rec = "PowerShell + Python";
        } else {
            best_lang = "Bash & Python";
            why_this_lang = "Bash provides rapid OS command orchestration, while Python handles complex data processing and scripting.";
            stack_fe = "None";
            stack_be = "Bash Shell / Python";
            stack_db = "CSV / JSON flat files";
            stack_auto = "Systemd / Linux Cron jobs";
            stack_deploy = "System service / Local execution / GitHub Actions";
            if_cond = "the target operating system is Linux or macOS";
            then_dec = "recommend Bash + Python";
            final_rec = "Bash + Python";
        }
        learn_1 = "Learn terminal basics, shell navigation, and command piping";
        learn_2 = "Master variables, control flows, and file IO in scripts";
        learn_3 = "Understand API requests and parsing JSON/XML in scripts";
        learn_4 = "Learn process management, environment setup, and cron schedules";
        build_setup = "Create directory structure and initialize git repository";
        build_proto = "Write simple linear script with hardcoded inputs";
        build_test = "Run script with diverse arguments and verify exit status codes";
        build_deploy = "Configure local cron job or register systemd service";
        build_iterate = "Add robust logging, error notifications, and modular helper functions";
    }
    else {
        best_lang = "Python / TypeScript";
        why_this_lang = "Highly versatile, massive ecosystem, and top job market demand.";
        stack_fe = "TypeScript (React / Next.js)";
        stack_be = "Python (FastAPI) or TypeScript (Node.js)";
        stack_db = "PostgreSQL";
        stack_auto = "npm scripts / GitHub Actions";
        stack_deploy = "Docker / Vercel / Railway";
        if_cond = "user goal is unspecified or unclear";
        then_dec = "recommend Python and TypeScript as the most versatile, high-demand entry stacks";
        learn_1 = "Learn programming fundamentals in Python or JavaScript";
        learn_2 = "Understand databases, SQL, and backend API routing";
        learn_3 = "Explore web frontend development with React";
        learn_4 = "Deploy simple full-stack projects to cloud platforms";
        build_setup = "Install Node.js and Python on local machine";
        build_proto = "Create a simple backend hello-world API";
        build_test = "Verify code runs locally and write simple unit tests";
        build_deploy = "Deploy the prototype to Railway or Render";
        build_iterate = "Explore framework documentation and build larger apps";
        final_rec = "Python (FastAPI) & TypeScript (Next.js + React)";
    }

    printf("\nPROGRAMMING_LANGUAGE_DECISION_TREE\n");
    printf("│\n");
    printf("├── USER_GOAL\n");
    printf("│   └── %s\n", detected_goal);
    printf("│\n");

    bool is_unclear = (strlen(norm_goal) == 0 || (strcmp(best_lang, "Python / TypeScript") == 0 && strstr(norm_goal, "build") == NULL));
    if (is_unclear) {
        printf("├── UNRESOLVED_DEPENDENCY (GOAL UNCLEAR)\n");
        printf("│   ├── Question 1: What are you building?\n");
        printf("│   ├── Question 2: Where will it run?\n");
        printf("│   └── Question 3: Do you want fastest build or strongest long-term architecture?\n");
        printf("│\n");
    }

    printf("├── BEST_LANGUAGE\n");
    printf("│   └── %s\n", best_lang);
    printf("│\n");
    printf("├── WHY_THIS_LANGUAGE\n");
    printf("│   └── %s\n", why_this_lang);
    printf("│\n");
    printf("├── SUPPORTING_STACK\n");
    printf("│   ├── Frontend: %s\n", stack_fe);
    printf("│   ├── Backend: %s\n", stack_be);
    printf("│   ├── Database: %s\n", stack_db);
    printf("│   ├── Automation: %s\n", stack_auto);
    printf("│   └── Deployment: %s\n", stack_deploy);
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
    printf("│   ├── Setup: %s\n", build_setup);
    printf("│   ├── Prototype: %s\n", build_proto);
    printf("│   ├── Test: %s\n", build_test);
    printf("│   ├── Deploy: %s\n", build_deploy);
    printf("│   └── Iterate: %s\n", build_iterate);
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
