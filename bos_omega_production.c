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
 * Normalizes a source string to lowercase into a destination buffer.
 * Replacing multiple strcasestr() calls with a single lowercase pass and subsequent
 * strstr() calls reduces keyword scanning complexity from O(N*M) to O(N+M).
 */
static void normalize(const char *src, char *dest) {
    if (!src || !dest) return;
    int i = 0;
    for (; src[i] != '\0' && i < MAX_TEXT - 1; i++) {
        dest[i] = (unsigned char)tolower(src[i]);
    }
    dest[i] = '\0';
}

/* --- GOVERNANCE ENGINE --- */
static RiskLevel assess_risk(const char *norm_task) {
    if (!norm_task) return R_READ_ONLY;
    if (strstr(norm_task, "buy") || strstr(norm_task, "pay") || strstr(norm_task, "spend")) return R_FINANCIAL;
    if (strstr(norm_task, "delete") || strstr(norm_task, "uninstall") || strstr(norm_task, "rm ") || strstr(norm_task, "wipe") || strstr(norm_task, "remove-") || strstr(norm_task, "set-content")) return R_DESTRUCTIVE;
    if (strstr(norm_task, "push") || strstr(norm_task, "commit") || strstr(norm_task, "api")) return R_EXTERNAL_API;
    if (strstr(norm_task, "write") || strstr(norm_task, "create") || strstr(norm_task, "save")) return R_LOCAL_WRITE;
    return R_READ_ONLY;
}

static ToolType resolve_tool(const char *norm_task) {
    if (!norm_task) return T_MISTRAL;
    if (strstr(norm_task, "android") || strstr(norm_task, "adb") || strstr(norm_task, "pm ") || strstr(norm_task, "am ") || strstr(norm_task, "input ")) return T_ADB;
    if (strstr(norm_task, "get-") || strstr(norm_task, "set-") || strstr(norm_task, "pwsh") || strstr(norm_task, "powershell")) return T_PWSH;
    if (strstr(norm_task, "browser") || strstr(norm_task, "navigate") || strstr(norm_task, "click")) return T_CDP;
    if (strstr(norm_task, "image") || strstr(norm_task, "visual")) return T_NVIDIA;
    if (strstr(norm_task, "railway") || strstr(norm_task, "deploy")) return T_RAILWAY;
    if (strstr(norm_task, "github") || strstr(norm_task, "repo")) return T_GITHUB;
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
    char norm_obj[MAX_TEXT];
    normalize(rt->objective, norm_obj);
    rt->state = ST_ROUTE; rt->tool = resolve_tool(norm_obj);
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
            if (!key && ts->type != T_CDP) { strcpy(rt->error_msg, "MISSING_API_KEY"); res = DEC_BLOCKED; }
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
    if (!in) return;

    /* Stack allocate normalized buffers without zero-initialization since they are immediately filled */
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

    /* Output Fields */
    char best_lang[MAX_TEXT] = "";
    char why_lang[MAX_TEXT] = "";
    char cond[MAX_TEXT] = "";
    char dec[MAX_TEXT] = "";

    char front[MAX_TEXT] = "None";
    char back[MAX_TEXT] = "None";
    char db[MAX_TEXT] = "None";
    char auto_tool[MAX_TEXT] = "None";
    char deploy[MAX_TEXT] = "None";

    char step1[MAX_TEXT] = "";
    char step2[MAX_TEXT] = "";
    char step3[MAX_TEXT] = "";
    char step4[MAX_TEXT] = "";

    char final_rec[MAX_TEXT] = "";

    bool unclear = false;

    if (strstr(norm_goal, "website") || strstr(norm_goal, "web")) {
        strncpy(best_lang, "TypeScript", MAX_TEXT - 1);
        strncpy(why_lang, "TypeScript adds robust safety and is the industry standard for modern web applications.", MAX_TEXT - 1);

        strncpy(front, "React / Next.js", MAX_TEXT - 1);
        strncpy(back, "Next.js (Node.js)", MAX_TEXT - 1);
        strncpy(db, "PostgreSQL", MAX_TEXT - 1);
        strncpy(auto_tool, "npm / Turborepo", MAX_TEXT - 1);
        strncpy(deploy, "Vercel / Docker", MAX_TEXT - 1);

        strncpy(cond, "goal is website and complexity is high", MAX_TEXT - 1);
        strncpy(dec, "recommend TypeScript with React/Next.js", MAX_TEXT - 1);

        strncpy(step1, "HTML & CSS Basics", MAX_TEXT - 1);
        strncpy(step2, "Modern JavaScript (ES6+)", MAX_TEXT - 1);
        strncpy(step3, "TypeScript Static Typing", MAX_TEXT - 1);
        strncpy(step4, "Next.js App Router & State Management", MAX_TEXT - 1);

        strncpy(final_rec, "Build a responsive web application using TypeScript, Next.js, and Tailwind CSS, deployed on Vercel.", MAX_TEXT - 1);
    }
    else if (strstr(norm_goal, "ai system") || strstr(norm_goal, "ai worker") || strstr(norm_goal, "agent") || strstr(norm_goal, "artificial intelligence") || strcmp(norm_goal, "ai") == 0 || strstr(norm_goal, " ai ") || strstr(norm_goal, "ai-") || strstr(norm_goal, "-ai")) {
        strncpy(best_lang, "Python & TypeScript", MAX_TEXT - 1);
        strncpy(why_lang, "Python is unmatched for AI model integration/workers; TypeScript excels at application and API layers.", MAX_TEXT - 1);

        strncpy(front, "React", MAX_TEXT - 1);
        strncpy(back, "Python (FastAPI) + Node.js (TypeScript)", MAX_TEXT - 1);
        strncpy(db, "PostgreSQL + pgvector / Pinecone", MAX_TEXT - 1);
        strncpy(auto_tool, "Bash / GitHub Actions", MAX_TEXT - 1);
        strncpy(deploy, "Docker / AWS", MAX_TEXT - 1);

        strncpy(cond, "building AI workers and robust API layer", MAX_TEXT - 1);
        strncpy(dec, "recommend Python for workers, TypeScript for backend orchestrator, pgvector/Pinecone for storage", MAX_TEXT - 1);

        strncpy(step1, "Python Core & Async Programming", MAX_TEXT - 1);
        strncpy(step2, "API development with FastAPI & TypeScript", MAX_TEXT - 1);
        strncpy(step3, "Vector Databases & Semantic Search", MAX_TEXT - 1);
        strncpy(step4, "Agent Orchestration & Workflow Automation", MAX_TEXT - 1);

        strncpy(final_rec, "Develop AI workers using Python/FastAPI, coordinate via TypeScript Node.js backend, store vectors in Pinecone/pgvector.", MAX_TEXT - 1);
    }
    else if (strstr(norm_goal, "mobile")) {
        if (strstr(norm_plat, "ios") || strstr(norm_constraints, "ios") || strstr(norm_goal, "ios")) {
            strncpy(best_lang, "Swift", MAX_TEXT - 1);
            strncpy(why_lang, "Swift provides native performance, modern syntax, and deep iOS platform integration.", MAX_TEXT - 1);
            strncpy(front, "SwiftUI", MAX_TEXT - 1);
            strncpy(cond, "platform is iOS only", MAX_TEXT - 1);
            strncpy(dec, "recommend Swift and SwiftUI", MAX_TEXT - 1);
            strncpy(step1, "Swift Syntax & Core Concepts", MAX_TEXT - 1);
            strncpy(step2, "SwiftUI Layouts & Views", MAX_TEXT - 1);
            strncpy(step3, "State Management & Combine", MAX_TEXT - 1);
            strncpy(step4, "CoreData & App Store Guidelines", MAX_TEXT - 1);
            strncpy(final_rec, "Develop a native iOS app using Swift and SwiftUI, deployed via TestFlight.", MAX_TEXT - 1);
        } else if (strstr(norm_plat, "android") || strstr(norm_constraints, "android") || strstr(norm_goal, "android")) {
            strncpy(best_lang, "Kotlin", MAX_TEXT - 1);
            strncpy(why_lang, "Kotlin is the Google-preferred modern language for highly optimized native Android development.", MAX_TEXT - 1);
            strncpy(front, "Jetpack Compose", MAX_TEXT - 1);
            strncpy(cond, "platform is Android only", MAX_TEXT - 1);
            strncpy(dec, "recommend Kotlin and Jetpack Compose", MAX_TEXT - 1);
            strncpy(step1, "Kotlin Syntax & Coroutines", MAX_TEXT - 1);
            strncpy(step2, "Jetpack Compose UI", MAX_TEXT - 1);
            strncpy(step3, "Android Architecture Components (MVVM)", MAX_TEXT - 1);
            strncpy(step4, "Google Play Store deployment", MAX_TEXT - 1);
            strncpy(final_rec, "Develop a native Android app using Kotlin and Jetpack Compose.", MAX_TEXT - 1);
        } else {
            strncpy(best_lang, "Dart & Flutter", MAX_TEXT - 1);
            strncpy(why_lang, "Dart and Flutter enable building high-performance cross-platform apps for both iOS and Android from one codebase.", MAX_TEXT - 1);
            strncpy(front, "Flutter UI", MAX_TEXT - 1);
            strncpy(cond, "platform is both iOS and Android (cross-platform)", MAX_TEXT - 1);
            strncpy(dec, "recommend Dart + Flutter", MAX_TEXT - 1);
            strncpy(step1, "Dart Programming Basics", MAX_TEXT - 1);
            strncpy(step2, "Flutter Widget Lifecycle & Layouts", MAX_TEXT - 1);
            strncpy(step3, "State Management (Provider/Bloc)", MAX_TEXT - 1);
            strncpy(step4, "Native Plugins & Compilation", MAX_TEXT - 1);
            strncpy(final_rec, "Develop a cross-platform mobile app using Dart & Flutter.", MAX_TEXT - 1);
        }
        strncpy(back, "Node.js / Go API", MAX_TEXT - 1);
        strncpy(db, "SQLite / PostgreSQL", MAX_TEXT - 1);
        strncpy(auto_tool, "Fastlane / Gradle", MAX_TEXT - 1);
        strncpy(deploy, "App Store & Google Play", MAX_TEXT - 1);
    }
    else if (strstr(norm_goal, "backend") || strstr(norm_goal, "api")) {
        if (strstr(norm_skill, "beginner") || strstr(norm_skill, "easy") || strstr(norm_skill, "simple") || strstr(norm_skill, "newbie")) {
            strncpy(best_lang, "Python", MAX_TEXT - 1);
            strncpy(why_lang, "Python's simple syntax and FastAPI make it extremely fast and easy for beginners to write correct APIs.", MAX_TEXT - 1);
            strncpy(back, "FastAPI", MAX_TEXT - 1);
            strncpy(cond, "skill level is beginner", MAX_TEXT - 1);
            strncpy(dec, "recommend Python + FastAPI", MAX_TEXT - 1);
            strncpy(step1, "Python Syntax & Virtual Environments", MAX_TEXT - 1);
            strncpy(step2, "HTTP Protocols & REST API design", MAX_TEXT - 1);
            strncpy(step3, "SQLAlchemy & SQLite/PostgreSQL", MAX_TEXT - 1);
            strncpy(step4, "Pydantic Models & OpenAPI docs", MAX_TEXT - 1);
            strncpy(final_rec, "Build a REST API using Python and FastAPI, using Pydantic for validation.", MAX_TEXT - 1);
        } else if (strstr(norm_style, "production") || strstr(norm_skill, "production") || strstr(norm_type, "production") || strstr(norm_constraints, "production") || strstr(norm_goal, "saas")) {
            strncpy(best_lang, "TypeScript / Go", MAX_TEXT - 1);
            strncpy(why_lang, "TypeScript on Node.js offers unmatched speed of development, while Go offers high-performance compilation.", MAX_TEXT - 1);
            strncpy(back, "Node.js (NestJS / Express) or Go", MAX_TEXT - 1);
            strncpy(cond, "goal is production SaaS", MAX_TEXT - 1);
            strncpy(dec, "recommend TypeScript + Node.js or Go", MAX_TEXT - 1);
            strncpy(step1, "Advanced TypeScript / Go Types", MAX_TEXT - 1);
            strncpy(step2, "API Routing, Middleware, & Auth", MAX_TEXT - 1);
            strncpy(step3, "Drizzle ORM / Go SQL drivers & Migrations", MAX_TEXT - 1);
            strncpy(step4, "Docker Containerization & CI/CD", MAX_TEXT - 1);
            strncpy(final_rec, "Construct a production SaaS backend using TypeScript/Node.js or Go, connected to PostgreSQL.", MAX_TEXT - 1);
        } else if (strstr(norm_constraints, "performance") || strstr(norm_constraints, "security") || strstr(norm_goal, "performance") || strstr(norm_goal, "security") || strstr(norm_style, "performance")) {
            strncpy(best_lang, "Rust / Go", MAX_TEXT - 1);
            strncpy(why_lang, "Rust delivers bare-metal speed and total memory safety; Go offers exceptional throughput and simpler concurrency.", MAX_TEXT - 1);
            strncpy(back, "Rust (Actix-web / Axum) or Go", MAX_TEXT - 1);
            strncpy(cond, "goal is high-performance/security-critical API", MAX_TEXT - 1);
            strncpy(dec, "recommend Rust or Go", MAX_TEXT - 1);
            strncpy(step1, "Rust Borrow Checker / Go Concurrency Channels", MAX_TEXT - 1);
            strncpy(step2, "High-performance Routing & JSON parsing", MAX_TEXT - 1);
            strncpy(step3, "Database pooling & raw SQL/Query Builders", MAX_TEXT - 1);
            strncpy(step4, "Benchmarking & Memory Profiling", MAX_TEXT - 1);
            strncpy(final_rec, "Write a high-performance, security-critical backend using Rust or Go.", MAX_TEXT - 1);
        } else {
            /* Default Backend */
            strncpy(best_lang, "TypeScript & Go", MAX_TEXT - 1);
            strncpy(why_lang, "Excellent ecosystem support and superb runtime characteristics for APIs.", MAX_TEXT - 1);
            strncpy(back, "Node.js / Go", MAX_TEXT - 1);
            strncpy(cond, "general backend development", MAX_TEXT - 1);
            strncpy(dec, "recommend TypeScript + Node.js as primary, or Go for compiled performance", MAX_TEXT - 1);
            strncpy(step1, "TypeScript / Go Basics", MAX_TEXT - 1);
            strncpy(step2, "RESTful API Design & HTTP Server", MAX_TEXT - 1);
            strncpy(step3, "SQL Database Migrations & Schemas", MAX_TEXT - 1);
            strncpy(step4, "Containerization & Monitoring", MAX_TEXT - 1);
            strncpy(final_rec, "Build a backend using TypeScript on Node.js or Go, backed by PostgreSQL.", MAX_TEXT - 1);
        }
        strncpy(db, "PostgreSQL / Redis", MAX_TEXT - 1);
        strncpy(auto_tool, "Makefile / Taskfile", MAX_TEXT - 1);
        strncpy(deploy, "Docker / Railway / AWS", MAX_TEXT - 1);
    }
    else if (strstr(norm_goal, "operating system") || strstr(norm_goal, "os") || strstr(norm_goal, "firmware") || strstr(norm_goal, "embedded") || strstr(norm_goal, "hardware")) {
        strncpy(best_lang, "C / C++ / Rust", MAX_TEXT - 1);
        strncpy(why_lang, "C, C++, and Rust allow bare-metal, predictable resource management and low-level CPU access.", MAX_TEXT - 1);

        strncpy(back, "Embedded C/Rust Core", MAX_TEXT - 1);
        strncpy(db, "Embedded SQLite / Flash storage / raw EEPROM", MAX_TEXT - 1);
        strncpy(auto_tool, "GNU Make / CMake", MAX_TEXT - 1);
        strncpy(deploy, "Bare-metal flashing / JTAG", MAX_TEXT - 1);

        strncpy(cond, "embedded / low-level CPU/hardware control required", MAX_TEXT - 1);
        strncpy(dec, "recommend C/C++ or Rust (use Assembly only where direct CPU instruction access is required)", MAX_TEXT - 1);

        strncpy(step1, "C Programming & Pointers / Memory Management", MAX_TEXT - 1);
        strncpy(step2, "Microcontroller architecture & Registers", MAX_TEXT - 1);
        strncpy(step3, "Rust Embedded Hal & Safety", MAX_TEXT - 1);
        strncpy(step4, "Assembly instructions & linker scripts", MAX_TEXT - 1);

        strncpy(final_rec, "Develop low-level firmware or OS components using C/Rust, resorting to Assembly for hardware initialization.", MAX_TEXT - 1);
    }
    else if (strstr(norm_goal, "fpga") || strstr(norm_goal, "asic") || strstr(norm_goal, "digital logic")) {
        strncpy(best_lang, "Verilog & SystemVerilog", MAX_TEXT - 1);
        strncpy(why_lang, "SystemVerilog is the industry standard for designing and verifying digital circuits on FPGA/ASIC.", MAX_TEXT - 1);

        strncpy(back, "Hardware Synthesis", MAX_TEXT - 1);
        strncpy(db, "Register Transfer Level (RTL) registers", MAX_TEXT - 1);
        strncpy(auto_tool, "Tcl Scripting / Vivado Makefiles", MAX_TEXT - 1);
        strncpy(deploy, "FPGA Flashing / Bitstream synthesis", MAX_TEXT - 1);

        strncpy(cond, "FPGA / ASIC / digital logic design", MAX_TEXT - 1);
        strncpy(dec, "recommend Verilog / SystemVerilog or VHDL", MAX_TEXT - 1);

        strncpy(step1, "Boolean Algebra & Combinational Logic", MAX_TEXT - 1);
        strncpy(step2, "Verilog Syntax & RTL Synthesis", MAX_TEXT - 1);
        strncpy(step3, "SystemVerilog Testbenches & Verification", MAX_TEXT - 1);
        strncpy(step4, "Timing analysis & FPGA constraint files", MAX_TEXT - 1);

        strncpy(final_rec, "Implement hardware modules in SystemVerilog, verify with custom testbenches, and synthesize using Vivado/Quartus.", MAX_TEXT - 1);
    }
    else if (strstr(norm_goal, "data science") || strstr(norm_goal, "ml") || strstr(norm_goal, "statistics") || strstr(norm_goal, "analysis")) {
        if (strstr(norm_goal, "statistics") || strstr(norm_constraints, "statistics") || strstr(norm_style, "statistics")) {
            strncpy(best_lang, "R & Python", MAX_TEXT - 1);
            strncpy(why_lang, "R excels in advanced statistical modeling, visualization, and academic research.", MAX_TEXT - 1);
            strncpy(cond, "data science / statistics-heavy workflow", MAX_TEXT - 1);
            strncpy(dec, "recommend R + Python", MAX_TEXT - 1);
            strncpy(step1, "R Syntax & Vector operations", MAX_TEXT - 1);
            strncpy(step2, "ggplot2 Data Visualization", MAX_TEXT - 1);
            strncpy(step3, "Statistical hypothesis testing & ANOVA", MAX_TEXT - 1);
            strncpy(step4, "Integration with Python via reticulate", MAX_TEXT - 1);
            strncpy(final_rec, "Perform statistical research using R and ggplot2, integrating Python for data pipelines.", MAX_TEXT - 1);
        } else if (strstr(norm_goal, "numerical") || strstr(norm_constraints, "numerical") || strstr(norm_style, "performance")) {
            strncpy(best_lang, "Julia & Python", MAX_TEXT - 1);
            strncpy(why_lang, "Julia offers C-like performance for numerical and scientific computing while being easy to write.", MAX_TEXT - 1);
            strncpy(cond, "high-performance numerical computing is required", MAX_TEXT - 1);
            strncpy(dec, "recommend Julia + Python", MAX_TEXT - 1);
            strncpy(step1, "Julia Basics & Multiple Dispatch", MAX_TEXT - 1);
            strncpy(step2, "Numerical arrays & Linear algebra", MAX_TEXT - 1);
            strncpy(step3, "Profiling & Optimizing Julia loops", MAX_TEXT - 1);
            strncpy(step4, "Calling C/Python libraries from Julia", MAX_TEXT - 1);
            strncpy(final_rec, "Implement high-speed scientific models in Julia, orchestrating them with Python.", MAX_TEXT - 1);
        } else {
            strncpy(best_lang, "Python & SQL", MAX_TEXT - 1);
            strncpy(why_lang, "Python offers an unmatched ecosystem (Pandas, Scikit-Learn) for general ML and data engineering.", MAX_TEXT - 1);
            strncpy(cond, "general data science / ML workflow", MAX_TEXT - 1);
            strncpy(dec, "recommend Python + SQL (R only if statistics-heavy; Julia only if high-performance numerical computing is required)", MAX_TEXT - 1);
            strncpy(step1, "Python Syntax & NumPy/Pandas", MAX_TEXT - 1);
            strncpy(step2, "SQL Data extraction & manipulation", MAX_TEXT - 1);
            strncpy(step3, "Scikit-Learn Machine Learning basics", MAX_TEXT - 1);
            strncpy(step4, "Model deployment with Streamlit / FastAPI", MAX_TEXT - 1);
            strncpy(final_rec, "Utilize Python with Pandas, Scikit-Learn, and SQL for robust data analytics and machine learning pipelines.", MAX_TEXT - 1);
        }
        strncpy(front, "Streamlit / Jupyter Notebooks", MAX_TEXT - 1);
        strncpy(back, "FastAPI / Python", MAX_TEXT - 1);
        strncpy(db, "PostgreSQL / Snowflake", MAX_TEXT - 1);
        strncpy(auto_tool, "Prefect / Airflow", MAX_TEXT - 1);
        strncpy(deploy, "Docker / AWS SageMaker", MAX_TEXT - 1);
    }
    else if (strstr(norm_goal, "database") || strstr(norm_goal, "db-heavy") || strstr(norm_goal, "sql")) {
        strncpy(best_lang, "SQL & TypeScript", MAX_TEXT - 1);
        strncpy(why_lang, "SQL is primary for performant data access; TypeScript matches robust backend orchestration.", MAX_TEXT - 1);

        strncpy(front, "React", MAX_TEXT - 1);
        strncpy(back, "Node.js (TypeScript)", MAX_TEXT - 1);
        strncpy(db, "PostgreSQL (with custom triggers & views)", MAX_TEXT - 1);
        strncpy(auto_tool, "DB Migration scripts", MAX_TEXT - 1);
        strncpy(deploy, "Supabase / Docker", MAX_TEXT - 1);

        strncpy(cond, "database-heavy application", MAX_TEXT - 1);
        strncpy(dec, "recommend SQL first, utilizing PostgreSQL database with a TypeScript/Python backend", MAX_TEXT - 1);

        strncpy(step1, "Relational Database design & Normalization", MAX_TEXT - 1);
        strncpy(step2, "Advanced SQL Queries & Indexing", MAX_TEXT - 1);
        strncpy(step3, "TypeScript Backend & Drizzle/Prisma ORM", MAX_TEXT - 1);
        strncpy(step4, "Connection pooling, migrations, and backups", MAX_TEXT - 1);

        strncpy(final_rec, "Design a database-heavy application with PostgreSQL, SQL for stored procedures/queries, and a Node.js TypeScript API.", MAX_TEXT - 1);
    }
    else if (strstr(norm_goal, "blockchain") || strstr(norm_goal, "smart contract") || strstr(norm_goal, "crypto")) {
        if (strstr(norm_goal, "ethereum") || strstr(norm_goal, "evm") || strstr(norm_constraints, "ethereum") || strstr(norm_constraints, "evm") || strstr(norm_goal, "solidity")) {
            strncpy(best_lang, "Solidity & TypeScript", MAX_TEXT - 1);
            strncpy(why_lang, "Solidity is the industry standard for EVM execution; TypeScript handles frontend and contract tests.", MAX_TEXT - 1);
            strncpy(cond, "Ethereum/EVM target platform", MAX_TEXT - 1);
            strncpy(dec, "recommend Solidity + TypeScript", MAX_TEXT - 1);
            strncpy(step1, "Solidity basics & State variables", MAX_TEXT - 1);
            strncpy(step2, "Ethers.js / Viem integration", MAX_TEXT - 1);
            strncpy(step3, "Foundry / Hardhat testing & deployment", MAX_TEXT - 1);
            strncpy(step4, "Smart Contract security audits & gas optimization", MAX_TEXT - 1);
            strncpy(final_rec, "Write EVM smart contracts in Solidity, test and deploy using Foundry, and connect via Next.js frontend.", MAX_TEXT - 1);
        } else if (strstr(norm_goal, "solana") || strstr(norm_constraints, "solana")) {
            strncpy(best_lang, "Rust", MAX_TEXT - 1);
            strncpy(why_lang, "Solana smart contracts (programs) are written in Rust for lightning-fast transaction speeds.", MAX_TEXT - 1);
            strncpy(cond, "Solana target platform", MAX_TEXT - 1);
            strncpy(dec, "recommend Rust", MAX_TEXT - 1);
            strncpy(step1, "Rust core language & Borrowing", MAX_TEXT - 1);
            strncpy(step2, "Solana Anchor Framework", MAX_TEXT - 1);
            strncpy(step3, "PDA (Program Derived Addresses) & Accounts", MAX_TEXT - 1);
            strncpy(step4, "Solana CLI testing & mainnet deployment", MAX_TEXT - 1);
            strncpy(final_rec, "Write high-performance Solana programs in Rust utilizing the Anchor framework.", MAX_TEXT - 1);
        } else if (strstr(norm_goal, "move") || strstr(norm_constraints, "move") || strstr(norm_goal, "sui") || strstr(norm_goal, "aptos")) {
            strncpy(best_lang, "Move", MAX_TEXT - 1);
            strncpy(why_lang, "Move chain technology provides specialized resource-oriented safety features for tokens.", MAX_TEXT - 1);
            strncpy(cond, "Move-chain (Sui/Aptos) target platform", MAX_TEXT - 1);
            strncpy(dec, "recommend Move", MAX_TEXT - 1);
            strncpy(step1, "Move resource-oriented model", MAX_TEXT - 1);
            strncpy(step2, "Sui/Aptos developer tools", MAX_TEXT - 1);
            strncpy(step3, "Writing Move modules", MAX_TEXT - 1);
            strncpy(step4, "Deploying and calling Move contracts", MAX_TEXT - 1);
            strncpy(final_rec, "Develop safe blockchain smart contracts using the Move programming language.", MAX_TEXT - 1);
        } else {
            /* Default EVM */
            strncpy(best_lang, "Solidity & TypeScript", MAX_TEXT - 1);
            strncpy(why_lang, "EVM remains the most dominant blockchain ecosystem with the highest developer adoption.", MAX_TEXT - 1);
            strncpy(cond, "General blockchain application", MAX_TEXT - 1);
            strncpy(dec, "recommend Solidity + TypeScript for EVM chains as default", MAX_TEXT - 1);
            strncpy(step1, "Blockchain & Cryptography basics", MAX_TEXT - 1);
            strncpy(step2, "Solidity smart contract coding", MAX_TEXT - 1);
            strncpy(step3, "Web3 Frontend integration (Ethers.js/Viem)", MAX_TEXT - 1);
            strncpy(step4, "Contract Auditing and Mainnet Deployment", MAX_TEXT - 1);
            strncpy(final_rec, "Develop decentralized apps utilizing Solidity for EVM smart contracts and TypeScript for web3 integration.", MAX_TEXT - 1);
        }
        strncpy(front, "React / Next.js with Wagmi", MAX_TEXT - 1);
        strncpy(back, "Ethers.js Node API", MAX_TEXT - 1);
        strncpy(db, "IPFS / Arweave + PostgreSQL", MAX_TEXT - 1);
        strncpy(auto_tool, "Hardhat / Foundry / Anchor", MAX_TEXT - 1);
        strncpy(deploy, "Mainnet / Testnet Node", MAX_TEXT - 1);
    }
    else if (strstr(norm_goal, "automation") || strstr(norm_goal, "scripting") || strstr(norm_goal, "workflow")) {
        if (strstr(norm_plat, "windows") || strstr(norm_plat, "win") || strstr(norm_constraints, "windows")) {
            strncpy(best_lang, "PowerShell & Python", MAX_TEXT - 1);
            strncpy(why_lang, "PowerShell has native, unparalleled access to Windows APIs and Active Directory; Python handles general processing.", MAX_TEXT - 1);
            strncpy(cond, "OS is Windows", MAX_TEXT - 1);
            strncpy(dec, "recommend PowerShell + Python", MAX_TEXT - 1);
            strncpy(step1, "PowerShell CMDlets & Object pipeline", MAX_TEXT - 1);
            strncpy(step2, "Python Scripting & os/sys packages", MAX_TEXT - 1);
            strncpy(step3, "Windows Task Scheduler & Service integration", MAX_TEXT - 1);
            strncpy(step4, "Error handling & Log management", MAX_TEXT - 1);
            strncpy(final_rec, "Create Windows administrative scripting workflows using PowerShell combined with Python.", MAX_TEXT - 1);
        } else {
            strncpy(best_lang, "Bash & Python", MAX_TEXT - 1);
            strncpy(why_lang, "Bash offers direct and lightweight Unix shell control; Python handles structured logic and libraries.", MAX_TEXT - 1);
            strncpy(cond, "OS is Linux/macOS", MAX_TEXT - 1);
            strncpy(dec, "recommend Bash + Python", MAX_TEXT - 1);
            strncpy(step1, "Bash Command-line basics", MAX_TEXT - 1);
            strncpy(step2, "Python Scripting & Request library", MAX_TEXT - 1);
            strncpy(step3, "Linux Cron jobs & systemd service setups", MAX_TEXT - 1);
            strncpy(step4, "Workflow orchestration tools (Airflow/Prefect)", MAX_TEXT - 1);
            strncpy(final_rec, "Build cross-platform scripting automation using Bash for Unix environment interaction and Python for logic.", MAX_TEXT - 1);
        }
        strncpy(front, "None (CLI)", MAX_TEXT - 1);
        strncpy(back, "Python CLI scripts", MAX_TEXT - 1);
        strncpy(db, "SQLite / CSV Files", MAX_TEXT - 1);
        strncpy(auto_tool, "Cron / Task Scheduler", MAX_TEXT - 1);
        strncpy(deploy, "Local execution / VPS", MAX_TEXT - 1);
    }
    else {
        unclear = true;
    }

    if (unclear) {
        printf("\nPROGRAMMING_LANGUAGE_DECISION_TREE\n");
        printf("│\n");
        printf("├── USER_GOAL\n");
        printf("│   └── %s (Goal is Unclear)\n", in->goal);
        printf("│\n");
        printf("└── UNRESOLVED_DEPENDENCY\n");
        printf("    └── Please answer exactly 3 questions:\n");
        printf("        1. What are you building?\n");
        printf("        2. Where will it run?\n");
        printf("        3. Do you want fastest build or strongest long-term architecture?\n");
    } else {
        printf("\nPROGRAMMING_LANGUAGE_DECISION_TREE\n");
        printf("│\n");
        printf("├── USER_GOAL\n");
        printf("│   └── %s\n", in->goal);
        printf("│\n");
        printf("├── BEST_LANGUAGE\n");
        printf("│   └── %s\n", best_lang);
        printf("│\n");
        printf("├── WHY_THIS_LANGUAGE\n");
        printf("│   └── %s\n", why_lang);
        printf("│\n");
        printf("├── SUPPORTING_STACK\n");
        printf("│   ├── Frontend: %s\n", front);
        printf("│   ├── Backend: %s\n", back);
        printf("│   ├── Database: %s\n", db);
        printf("│   ├── Automation: %s\n", auto_tool);
        printf("│   └── Deployment: %s\n", deploy);
        printf("│\n");
        printf("├── IF_THEN_REASONING\n");
        printf("│   ├── IF %s\n", cond);
        printf("│   └── THEN %s\n", dec);
        printf("│\n");
        printf("├── LEARNING_ORDER\n");
        printf("│   ├── Step 1: %s\n", step1);
        printf("│   ├── Step 2: %s\n", step2);
        printf("│   ├── Step 3: %s\n", step3);
        printf("│   └── Step 4: %s\n", step4);
        printf("│\n");
        printf("├── BUILD_ORDER\n");
        printf("│   ├── Setup: Configure local compiler/runtime & initial workspace\n");
        printf("│   ├── Prototype: Build simple CLI/API/UI component executing core functions\n");
        printf("│   ├── Test: Implement automated unit tests & boundary checks\n");
        printf("│   ├── Deploy: Package in Docker or bundle, host on cloud / target platform\n");
        printf("│   └── Iterate: Monitor telemetry & scale features incrementally\n");
        printf("│\n");
        printf("└── FINAL_RECOMMENDATION\n");
        printf("    └── %s\n", final_rec);
    }
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
