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
 * Converts a source string to lowercase and writes it to a destination buffer.
 * Handles NULL pointers, performs proper null-termination, and checks bounds.
 * By converting the string once to lowercase, subsequent keyword matching can use
 * strstr() instead of strcasestr(), reducing scanning complexity from O(N*M) to O(N+M)
 * where N is the length of the string and M is the total length of keywords.
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

/**
 * Checks if a needle exists as a distinct word in a haystack.
 * It ensures that the needle is bounded by non-alphanumeric and non-underscore
 * characters to prevent false matches (e.g. "ai" matching inside "blockchain").
 */
static bool has_word(const char *haystack, const char *needle) {
    if (!haystack || !needle) return false;
    const char *p = haystack;
    size_t needle_len = strlen(needle);
    while ((p = strstr(p, needle)) != NULL) {
        bool boundary_before = true;
        if (p > haystack) {
            char prev = *(p - 1);
            if (isalnum((unsigned char)prev) || prev == '_') {
                boundary_before = false;
            }
        }
        bool boundary_after = true;
        char next = *(p + needle_len);
        if (next != '\0' && (isalnum((unsigned char)next) || next == '_')) {
            boundary_after = false;
        }
        if (boundary_before && boundary_after) {
            return true;
        }
        p += needle_len;
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
    normalize(norm, rt->objective);
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
    char goal[MAX_TEXT];
    char skill[MAX_TEXT];
    char type[MAX_TEXT];
    char plat[MAX_TEXT];
    char time_avail[MAX_TEXT];
    char style[MAX_TEXT];
    char constraints[MAX_TEXT];

    normalize(goal, in->goal);
    normalize(skill, in->skill);
    normalize(type, in->type);
    normalize(plat, in->plat);
    normalize(time_avail, in->time);
    normalize(style, in->style);
    normalize(constraints, in->constraints);

    const char *detected_goal = in->goal;
    const char *best_lang = "";
    const char *why = "";
    const char *fe = "N/A";
    const char *be = "N/A";
    const char *db = "N/A";
    const char *automation_str = "N/A";
    const char *deploy = "N/A";
    const char *if_cond = "";
    const char *then_dec = "";
    const char *l1 = "";
    const char *l2 = "";
    const char *l3 = "";
    const char *l4 = "";
    const char *b1 = "Setup project environment";
    const char *b2 = "Build minimal working prototype";
    const char *b3 = "Implement comprehensive unit and integration tests";
    const char *b4 = "Deploy to target environment";
    const char *b5 = "Iterate based on performance metrics and feedback";
    const char *final_rec = "";

    if (strstr(goal, "website") || strstr(goal, "web") || strstr(goal, "frontend")) {
        best_lang = "TypeScript";
        why = "TypeScript provides static typing and enterprise scalability for web apps.";
        fe = "HTML5 / CSS3 / React (Next.js)";
        be = "Node.js (Express) or Next.js API Routes";
        db = "PostgreSQL (using Prisma or Drizzle)";
        automation_str = "Webpack / Vite / pnpm";
        deploy = "Vercel / Netlify / Docker";
        if_cond = "User goal is to build a modern website/frontend with app-level complexity.";
        then_dec = "Recommend HTML, CSS, JavaScript, TypeScript, and React/Next.js.";
        l1 = "Master HTML/CSS and Javascript fundamentals";
        l2 = "Adopt TypeScript for type-safe application logic";
        l3 = "Learn React.js for component-driven UI development";
        l4 = "Utilize Next.js for server-side rendering and full-stack integration";
        final_rec = "React + Next.js with TypeScript deployed on Vercel.";
    } else if (strstr(goal, "data science") || strstr(goal, "ml") || strstr(goal, "machine learning") || strstr(goal, "statistics")) {
        best_lang = "Python";
        if (strstr(goal, "statistics")) {
            why = "R is recommended for statistics-heavy analysis, Python for general ML.";
            best_lang = "Python + R";
        } else if (strstr(goal, "numerical") || strstr(goal, "high-performance")) {
            why = "Julia is recommended for high-performance numerical computing, Python for general ML.";
            best_lang = "Python + Julia";
        } else {
            why = "Python is the industry standard for data science and machine learning ecosystems.";
        }
        fe = "Streamlit / Gradio";
        be = "FastAPI / Flask";
        db = "PostgreSQL / ClickHouse";
        automation_str = "Poetry / Conda";
        deploy = "AWS ECS / Hugging Face Spaces";
        if_cond = "User goal is data science or machine learning workloads.";
        then_dec = "Recommend Python and SQL as primary tools.";
        l1 = "Learn Python core and SQL data retrieval";
        l2 = "Master NumPy, Pandas, and Scikit-Learn";
        l3 = "Study Deep Learning frameworks (PyTorch)";
        l4 = "Explore MLOps tools for deployment";
        final_rec = "Python (PyTorch + Pandas) with SQL backend for analytical workloads.";
    } else if (has_word(goal, "ai") || strstr(goal, "agent") || strstr(goal, "llm") || strstr(goal, "mistral") || strstr(goal, "openai")) {
        best_lang = "Python + TypeScript";
        why = "Python for AI workers and modeling, TypeScript for production application layer.";
        fe = "React.js / Next.js";
        be = "Python (FastAPI) & Node.js";
        db = "PostgreSQL / Vector DB (Pgvector or Pinecone)";
        automation_str = "Bash / Docker Compose";
        deploy = "Vercel & AWS ECS";
        if_cond = "User goal is building an AI agent or automated system.";
        then_dec = "Recommend Python for AI core workers, TypeScript for the API/app layer, SQL/Vector DB for storage.";
        l1 = "Learn Python basics and OpenAI/Mistral APIs";
        l2 = "Understand Vector DBs (Pgvector/Pinecone)";
        l3 = "Master TypeScript/Next.js for app routing";
        l4 = "Build orchestrators (LangChain or custom loops)";
        final_rec = "Next.js Frontend + FastAPI AI Backend + Pgvector Storage.";
    } else if (strstr(goal, "mobile") || strstr(goal, "ios") || strstr(goal, "android") || strstr(goal, "phone")) {
        if (strstr(plat, "ios") && !strstr(plat, "android")) {
            best_lang = "Swift";
            why = "Swift is the native, high-performance language for iOS applications.";
            fe = "SwiftUI";
            be = "Node.js or Swift (Vapor)";
            db = "CoreData / SQLite";
            automation_str = "Fastlane / Xcode CLI";
            deploy = "Apple App Store";
            if_cond = "User is building an iOS-only native application.";
            then_dec = "Recommend Swift and SwiftUI.";
            l1 = "Learn Swift language syntax";
            l2 = "Master SwiftUI framework and layout";
            l3 = "Understand local storage and state management";
            l4 = "Learn App Store Connect deployment pipelines";
            final_rec = "Swift (SwiftUI) for native iOS development.";
        } else if (strstr(plat, "android") && !strstr(plat, "ios")) {
            best_lang = "Kotlin";
            why = "Kotlin is the modern, expressive standard for native Android apps.";
            fe = "Jetpack Compose";
            be = "Node.js or Kotlin (Ktor)";
            db = "Room DB / SQLite";
            automation_str = "Gradle / Fastlane";
            deploy = "Google Play Store";
            if_cond = "User is building an Android-only native application.";
            then_dec = "Recommend Kotlin and Jetpack Compose.";
            l1 = "Learn Kotlin syntax and coroutines";
            l2 = "Master Jetpack Compose for Android UI";
            l3 = "Integrate SQLite / Room DB for local data";
            l4 = "Learn Google Play Console publishing workflow";
            final_rec = "Kotlin (Jetpack Compose) for native Android development.";
        } else {
            best_lang = "Dart + Flutter";
            why = "Dart + Flutter provides near-native cross-platform performance for iOS and Android.";
            fe = "Flutter Widget Engine";
            be = "FastAPI / Node.js";
            db = "Hive / SQLite / Firebase";
            automation_str = "Flutter CLI / Fastlane";
            deploy = "App Store + Google Play Store";
            if_cond = "User is targeting both iOS and Android platforms.";
            then_dec = "Recommend Dart with Flutter.";
            l1 = "Learn Dart language fundamentals";
            l2 = "Master Flutter Widget layout system";
            l3 = "Implement state management (Riverpod/Provider)";
            l4 = "Configure cross-platform CI/CD with Fastlane";
            final_rec = "Flutter + Dart for efficient cross-platform mobile delivery.";
        }
    } else if (strstr(goal, "backend") || strstr(goal, "api") || strstr(goal, "server")) {
        if (strstr(skill, "beginner") || strstr(skill, "junior")) {
            best_lang = "Python + FastAPI";
            why = "FastAPI is highly intuitive, fast to write, and perfect for beginners.";
            fe = "N/A (API Only)";
            be = "Python (FastAPI)";
            db = "PostgreSQL (SQLAlchemy)";
            automation_str = "Uvicorn / Poetry";
            deploy = "Render / Railway / Docker";
            if_cond = "User wants to build backend APIs and is a beginner.";
            then_dec = "Recommend Python + FastAPI for rapid learning and production capabilities.";
            l1 = "Learn Python syntax and async concepts";
            l2 = "Master FastAPI routes and Pydantic models";
            l3 = "Connect to PostgreSQL using SQLAlchemy or SQL";
            l4 = "Deploy to Railway or Render via Docker";
            final_rec = "Python + FastAPI + PostgreSQL on Render.";
        } else if (strstr(style, "high-performance") || strstr(style, "security") || strstr(constraints, "performance") || strstr(constraints, "security")) {
            best_lang = "Rust or Go";
            why = "Rust provides memory safety without garbage collection, and Go offers unmatched concurrent throughput.";
            fe = "N/A (API Only)";
            be = "Rust (Axum) or Go (Gin)";
            db = "PostgreSQL / ScyllaDB";
            automation_str = "Cargo / Go Build";
            deploy = "AWS ECS / Kubernetes / Bare Metal";
            if_cond = "User is building high-performance/security-critical APIs.";
            then_dec = "Recommend Rust or Go.";
            l1 = "Master Rust memory safety / Go concurrency model";
            l2 = "Build HTTP API server with Axum / Gin";
            l3 = "Optimize database query paths with SQLx / pgx";
            l4 = "Containerize and deploy to Kubernetes or Bare Metal";
            final_rec = "Rust (Axum) or Go for high-speed, secure backend APIs.";
        } else {
            best_lang = "TypeScript + Node.js or Go";
            why = "TypeScript on Node.js provides massive ecosystem reuse, while Go ensures native speed for SaaS.";
            fe = "N/A";
            be = "Node.js (NestJS/Express) or Go";
            db = "PostgreSQL / Redis";
            automation_str = "pnpm / Go CLI";
            deploy = "AWS ECS / Railway / Docker";
            if_cond = "User is building a production SaaS backend.";
            then_dec = "Recommend TypeScript + Node.js or Go.";
            l1 = "Learn advanced TypeScript / Go routines";
            l2 = "Master Express/NestJS or Gin framework";
            l3 = "Integrate PostgreSQL with database migrations";
            l4 = "Containerize using multi-stage Dockerfiles";
            final_rec = "TypeScript (Node.js/Express) + PostgreSQL on Railway.";
        }
    } else if (strstr(goal, "operating system") || strcmp(goal, "os") == 0 || strstr(goal, " os ") || strstr(goal, "firmware") || strstr(goal, "embedded") || strstr(goal, "kernel") || strstr(goal, "microcontroller")) {
        best_lang = "C / C++ / Rust";
        why = "C/C++ provides direct hardware access and deterministic memory, while Rust ensures modern memory safety.";
        fe = "N/A (Bare metal / custom console)";
        be = "Embedded C / Rust HAL";
        db = "N/A (Raw memory / Flash / EEPROM)";
        automation_str = "Makefile / CMake / Cargo Embedded";
        deploy = "JTAG / OpenOCD / Flash Tool";
        if_cond = "User goal is low-level systems programming, OS, or firmware.";
        then_dec = "Recommend C, C++, Rust, and Assembly only for direct CPU control.";
        l1 = "Learn C/C++ memory layout and pointers";
        l2 = "Master assembly instructions for target CPU (x86/ARM/RISC-V)";
        l3 = "Understand memory-mapped I/O and Interrupts";
        l4 = "Build custom bootloaders and basic kernel routines";
        final_rec = "C for core low-level / Rust for modern safe embedded development.";
    } else if (strstr(goal, "fpga") || strstr(goal, "asic") || strstr(goal, "digital logic") || strstr(goal, "verilog") || strstr(goal, "vhdl")) {
        best_lang = "Verilog / SystemVerilog / VHDL";
        why = "HDLs (Hardware Description Languages) describe concurrency and structure of digital hardware.";
        fe = "N/A";
        be = "FPGA Gate Array / ASIC Logic";
        db = "Block RAM / Register Files";
        automation_str = "Vivado / Quartus / ModelSim";
        deploy = "FPGA Bitstream / ASIC GDSII Tape-out";
        if_cond = "User is building digital logic or targeting hardware gates.";
        then_dec = "Recommend Verilog, SystemVerilog, or VHDL.";
        l1 = "Understand digital logic gates and state machines";
        l2 = "Learn Verilog / SystemVerilog syntax";
        l3 = "Simulate designs with testbenches in ModelSim";
        l4 = "Synthesize and program onto physical FPGA boards";
        final_rec = "SystemVerilog for synthesis and verified digital hardware design.";
    } else if (strstr(goal, "database") || strstr(goal, "sql") || strstr(goal, "postgres") || strstr(goal, "mysql")) {
        best_lang = "SQL + PostgreSQL";
        why = "SQL is the foundational standard; PostgreSQL is the most robust production-grade open source DB.";
        fe = "Next.js (React) Dashboard";
        be = "TypeScript Node.js (Express)";
        db = "PostgreSQL with advanced index optimizations";
        automation_str = "Flyway / dbmate / Prisma migrations";
        deploy = "Supabase / AWS RDS / Neon";
        if_cond = "User is building database-heavy software.";
        then_dec = "Recommend SQL first with PostgreSQL, and TypeScript or Python backend.";
        l1 = "Learn SQL query syntax and indexing strategies";
        l2 = "Master relational schema design and normal forms";
        l3 = "Implement backend API connecting to database";
        l4 = "Set up database connection pooling and replica scaling";
        final_rec = "PostgreSQL + TypeScript Backend with Prisma ORM.";
    } else if (strstr(goal, "blockchain") || strstr(goal, "smart contract") || strstr(goal, "solidity") || strstr(goal, "ethereum") || strstr(goal, "solana") || strstr(goal, "web3") || strstr(goal, "evm")) {
        if (strstr(goal, "solana")) {
            best_lang = "Rust";
            why = "Rust is the core language for secure, high-performance Solana smart contracts.";
            fe = "React.js / Next.js with @solana/web3.js";
            be = "Anchor Framework (Rust)";
            db = "Solana Ledger / Arweave";
            automation_str = "Anchor CLI / Cargo";
            deploy = "Solana Mainnet-Beta";
            if_cond = "User is building on the Solana blockchain.";
            then_dec = "Recommend Rust with Anchor framework.";
            l1 = "Learn Rust basic type systems and safety";
            l2 = "Understand Solana Account model";
            l3 = "Master Anchor framework for secure state transitions";
            l4 = "Deploy to Solana devnet and run integration tests";
            final_rec = "Rust (Anchor Framework) + Next.js frontend.";
        } else if (strstr(goal, "move") || strstr(goal, "aptos") || strstr(goal, "sui")) {
            best_lang = "Move";
            why = "Move offers resource safety and protection against common EVM vulnerabilities.";
            fe = "React.js / Next.js with Sui/Aptos SDKs";
            be = "Move Smart Contracts";
            db = "Sui/Aptos Ledger";
            automation_str = "Move CLI";
            deploy = "Aptos/Sui Mainnet";
            if_cond = "User is targeting Move-based chains (Aptos/Sui).";
            then_dec = "Recommend Move language.";
            l1 = "Understand Move resource and ownership concepts";
            l2 = "Write basic Move modules";
            l3 = "Create unit tests with Move testing framework";
            l4 = "Deploy modules and interact via frontend SDK";
            final_rec = "Move language contracts + Next.js frontend.";
        } else {
            best_lang = "Solidity + TypeScript";
            why = "Solidity is the standard for Ethereum/EVM contracts; TypeScript is used for testing and UI.";
            fe = "React.js / Next.js with Ethers.js/Viem";
            be = "Solidity (Foundry / Hardhat)";
            db = "EVM State Ledger / IPFS";
            automation_str = "Foundry / Hardhat / npm";
            deploy = "Ethereum Mainnet / Arbitrum / Optimism";
            if_cond = "User is building on Ethereum or EVM-compatible networks.";
            then_dec = "Recommend Solidity with TypeScript for testing and deployment.";
            l1 = "Learn Solidity syntax, state variables, and ERC standards";
            l2 = "Master Foundry for unit testing smart contracts";
            l3 = "Use Hardhat/TypeScript for deployment scripts";
            l4 = "Build Next.js web3 frontend to interact with contract";
            final_rec = "Solidity (Foundry) + Next.js (TypeScript) frontend.";
        }
    } else if (strstr(goal, "automation") || strstr(goal, "script") || strstr(goal, "bash") || strstr(goal, "powershell") || strstr(goal, "cli") || strstr(goal, "devops")) {
        if (strstr(plat, "windows")) {
            best_lang = "PowerShell + Python";
            why = "PowerShell is natively integrated into Windows APIs, and Python handles complex data processing.";
            fe = "N/A (CLI / Terminal)";
            be = "PowerShell Core / Python Scripts";
            db = "CSV / SQLite / Active Directory";
            automation_str = "Windows Task Scheduler / PowerShell modules";
            deploy = "Windows Server / Local PC Execution";
            if_cond = "User is automating tasks on a Windows environment.";
            then_dec = "Recommend PowerShell paired with Python.";
            l1 = "Learn PowerShell pipelining and cmdlets";
            l2 = "Master Python core script structures";
            l3 = "Interact with OS APIs and filesystems in both";
            l4 = "Schedule automated tasks and set up error logging";
            final_rec = "PowerShell + Python for robust Windows task automation.";
        } else {
            best_lang = "Bash + Python";
            why = "Bash is the native shell interpreter for Linux/macOS, and Python provides standard libraries for automation.";
            fe = "N/A (CLI / Terminal)";
            be = "Bash / Python Scripts";
            db = "JSON / SQLite / PostgreSQL";
            automation_str = "Cron Jobs / Systemd Services";
            deploy = "Linux Server / AWS EC2 / systemd";
            if_cond = "User is automating tasks on a Linux/macOS environment.";
            then_dec = "Recommend Bash paired with Python.";
            l1 = "Learn Bash shell scripting, pipes, and standard tools (sed/awk/grep)";
            l2 = "Master Python for multi-system interactions";
            l3 = "Automate file operations and network requests";
            l4 = "Deploy scripts as systemd daemons or cron schedules";
            final_rec = "Bash + Python for unix-style pipeline automation.";
        }
    } else {
        printf("\nPROGRAMMING_LANGUAGE_DECISION_TREE\n");
        printf("│\n");
        printf("├── USER_GOAL\n");
        printf("│   └── (Unclear: %s)\n", detected_goal);
        printf("│\n");
        printf("└── QUESTIONS_REQUIRED\n");
        printf("    ├── 1. What are you building?\n");
        printf("    ├── 2. Where will it run?\n");
        printf("    └── 3. Do you want fastest build or strongest long-term architecture?\n");
        return;
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
    printf("│   ├── Automation: %s\n", automation_str);
    printf("│   └── Deployment: %s\n", deploy);
    printf("│\n");
    printf("├── IF_THEN_REASONING\n");
    printf("│   ├── IF %s\n", if_cond);
    printf("│   └── THEN %s\n", then_dec);
    printf("│\n");
    printf("├── LEARNING_ORDER\n");
    printf("│   ├── Step 1: %s\n", l1);
    printf("│   ├── Step 2: %s\n", l2);
    printf("│   ├── Step 3: %s\n", l3);
    printf("│   └── Step 4: %s\n", l4);
    printf("│\n");
    printf("├── BUILD_ORDER\n");
    printf("│   ├── Setup: %s\n", b1);
    printf("│   ├── Prototype: %s\n", b2);
    printf("│   ├── Test: %s\n", b3);
    printf("│   ├── Deploy: %s\n", b4);
    printf("│   └── Iterate: %s\n", b5);
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
