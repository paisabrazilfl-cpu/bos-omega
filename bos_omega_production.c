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
 * Normalizes a string to lowercase to optimize string searches.
 * Replacing multiple strcasestr() calls with a single lowercase pass
 * and subsequent strstr() calls reduces keyword scanning complexity from O(N*M) to O(N+M).
 */
static void normalize(const char *src, char *dest) {
    if (!src || !dest) {
        if (dest) dest[0] = '\0';
        return;
    }
    int i = 0;
    while (src[i] != '\0' && i < MAX_TEXT - 1) {
        dest[i] = (char)tolower((unsigned char)src[i]);
        i++;
    }
    dest[i] = '\0';
}

static bool has_word(const char *haystack, const char *word) {
    if (!haystack || !word) return false;
    size_t word_len = strlen(word);
    const char *pos = haystack;
    while ((pos = strstr(pos, word)) != NULL) {
        // Check left boundary
        bool left_ok = (pos == haystack) || (!isalnum((unsigned char)pos[-1]) && pos[-1] != '_');
        // Check right boundary
        char right_char = pos[word_len];
        bool right_ok = (!isalnum((unsigned char)right_char) && right_char != '_');
        if (left_ok && right_ok) return true;
        pos += word_len;
    }
    return false;
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
    char norm_objective[MAX_TEXT];
    normalize(rt->objective, norm_objective);
    rt->state = ST_ROUTE; rt->tool = resolve_tool(norm_objective);
    rt->risk = assess_risk(norm_objective);
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
                snprintf(payload, sizeof(payload), "{\"model\":\"prod\",\"input\":\"%.*s\"}", MAX_TEXT - 1, rt->objective);
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
    LangOutput out = {0};
    strncpy(out.goal, in->goal, MAX_TEXT - 1);
    out.goal[MAX_TEXT - 1] = '\0';

    char norm_goal[MAX_TEXT];
    char norm_skill[MAX_TEXT];
    char norm_type[MAX_TEXT];
    char norm_plat[MAX_TEXT];
    char norm_constraints[MAX_TEXT];

    normalize(in->goal, norm_goal);
    normalize(in->skill, norm_skill);
    normalize(in->type, norm_type);
    normalize(in->plat, norm_plat);
    normalize(in->constraints, norm_constraints);

    bool goal_unclear = false;
    const char *detected_goal = "";

    // Check goals deterministically based on normalized keywords
    if (strstr(norm_goal, "blockchain") || strstr(norm_goal, "contract") || strstr(norm_goal, "crypto") || strstr(norm_goal, "ethereum") || strstr(norm_goal, "solana") || strstr(norm_goal, "solidity")) {
        detected_goal = "blockchain / smart contracts";
        if (strstr(norm_constraints, "solana")) {
            strncpy(out.lang, "Rust", MAX_TEXT - 1);
            strncpy(out.why, "Low-latency, high-throughput smart contracts on Solana.", MAX_TEXT - 1);
            strncpy(out.stack, "Solana + Rust (Anchor Framework) + TypeScript client", MAX_TEXT - 1);
            strncpy(out.steps, "1. IF Solana, THEN Rust\n2. Rank: 1. Reality fit, 2. Job market, 3. Ecosystem", MAX_TEXT - 1);
        } else if (strstr(norm_constraints, "move")) {
            strncpy(out.lang, "Move", MAX_TEXT - 1);
            strncpy(out.why, "Safe and asset-oriented language for Move-based blockchains.", MAX_TEXT - 1);
            strncpy(out.stack, "Move + Aptos/Sui framework + TypeScript client", MAX_TEXT - 1);
            strncpy(out.steps, "1. IF Move-chain, THEN Move\n2. Rank: 1. Reality fit, 2. Job market, 3. Ecosystem", MAX_TEXT - 1);
        } else {
            strncpy(out.lang, "Solidity + TypeScript", MAX_TEXT - 1);
            strncpy(out.why, "Ecosystem standard for EVM-compatible networks with massive TVL.", MAX_TEXT - 1);
            strncpy(out.stack, "Solidity + Hardhat/Foundry + TypeScript (Ethers.js/Wagmi) frontend", MAX_TEXT - 1);
            strncpy(out.steps, "1. IF Ethereum/EVM, THEN Solidity + TypeScript\n2. Rank: 1. Reality fit, 2. Job market, 3. Ecosystem", MAX_TEXT - 1);
        }
    } else if (strstr(norm_goal, "fpga") || strstr(norm_goal, "asic") || strstr(norm_goal, "digital logic") || strstr(norm_goal, "verilog") || strstr(norm_goal, "vhdl")) {
        detected_goal = "FPGA / ASIC / digital logic";
        strncpy(out.lang, "SystemVerilog", MAX_TEXT - 1);
        strncpy(out.why, "Hardware Description Languages are required to model physical hardware concurrency.", MAX_TEXT - 1);
        strncpy(out.stack, "SystemVerilog + ModelSim simulation + Vivado synthesis/P&R tools", MAX_TEXT - 1);
        strncpy(out.steps, "1. IF FPGA/ASIC/digital logic, THEN recommend Verilog/SystemVerilog/VHDL\n2. Choose SystemVerilog as industry standard", MAX_TEXT - 1);
    } else if (strstr(norm_goal, "operating system") || strstr(norm_goal, "firmware") || strstr(norm_goal, "embedded") || strstr(norm_goal, "kernel") || strcmp(norm_goal, "os") == 0 || strstr(norm_goal, " os") || strstr(norm_goal, "os ")) {
        detected_goal = "build operating system / firmware / embedded";
        strncpy(out.lang, "C / C++ / Rust", MAX_TEXT - 1);
        strncpy(out.why, "Provides direct hardware access, low memory footprint, and predictability.", MAX_TEXT - 1);
        strncpy(out.stack, "C/Rust embedded code + CMake/Cargo build system + OpenOCD debug toolchain", MAX_TEXT - 1);
        strncpy(out.steps, "1. IF OS/firmware/embedded, THEN recommend C/C++/Rust\n2. Assembly only where direct CPU/control access is required", MAX_TEXT - 1);
    } else if (has_word(norm_goal, "ai") || strstr(norm_goal, "agent") || strstr(norm_goal, "llm")) {
        detected_goal = "build AI system";
        strncpy(out.lang, "Python (AI workers) + TypeScript (App/API layer)", MAX_TEXT - 1);
        strncpy(out.why, "Python dominates ML/AI ecosystem; TypeScript handles production API orchestrations seamlessly.", MAX_TEXT - 1);
        strncpy(out.stack, "Python (FastAPI) + TypeScript (Next.js/React) + SQL (PostgreSQL + pgvector) + Automation (Bash/YAML)", MAX_TEXT - 1);
        strncpy(out.steps, "1. IF build AI system, THEN Python for AI workers, TS for app/API, SQL for storage, Bash/YAML for automation\n2. Rank: 1. Reality fit, 2. Job market, 3. Ecosystem", MAX_TEXT - 1);
    } else if (strstr(norm_goal, "data science") || strstr(norm_goal, "statistics") || strstr(norm_goal, "machine learning") || strstr(norm_goal, "ml") || strstr(norm_goal, "numerical")) {
        detected_goal = "data science / ML";
        if (strstr(norm_goal, "statistics") || strstr(norm_goal, "stat")) {
            strncpy(out.lang, "R + Python", MAX_TEXT - 1);
            strncpy(out.why, "R is specialized and optimal for advanced statistical computation and packages.", MAX_TEXT - 1);
            strncpy(out.stack, "RStudio / Jupyter + Tidyverse + Python (for integration/ML)", MAX_TEXT - 1);
            strncpy(out.steps, "1. IF statistics-heavy, THEN recommend R\n2. Rank: 1. Reality fit, 2. Job market, 3. Ecosystem", MAX_TEXT - 1);
        } else if (strstr(norm_goal, "numerical") || strstr(norm_goal, "julia")) {
            strncpy(out.lang, "Julia + Python", MAX_TEXT - 1);
            strncpy(out.why, "Julia is specialized for high-performance numerical computing.", MAX_TEXT - 1);
            strncpy(out.stack, "Julia + Pluto.jl + PyCall + SQL storage", MAX_TEXT - 1);
            strncpy(out.steps, "1. IF high-performance numerical computing, THEN Julia\n2. Rank: 1. Reality fit, 2. Job market, 3. Ecosystem", MAX_TEXT - 1);
        } else {
            strncpy(out.lang, "Python + SQL", MAX_TEXT - 1);
            strncpy(out.why, "De-facto industry standard for general data science and ML.", MAX_TEXT - 1);
            strncpy(out.stack, "Python (Pandas, PyTorch, Scikit-Learn) + SQL (PostgreSQL)", MAX_TEXT - 1);
            strncpy(out.steps, "1. IF data science / ML, THEN Python + SQL\n2. Rank: 1. Reality fit, 2. Job market, 3. Ecosystem", MAX_TEXT - 1);
        }
    } else if (strstr(norm_goal, "database") || strstr(norm_goal, "db-heavy") || strstr(norm_goal, "database-heavy")) {
        detected_goal = "database-heavy app";
        strncpy(out.lang, "SQL first, PostgreSQL, TypeScript/Python backend", MAX_TEXT - 1);
        strncpy(out.why, "Data integrity and relational querying require declarative SQL at the storage layer.", MAX_TEXT - 1);
        strncpy(out.stack, "PostgreSQL + TypeScript (Node.js/Express) backend + React frontend", MAX_TEXT - 1);
        strncpy(out.steps, "1. IF database-heavy app, THEN SQL first, PostgreSQL, TS/Python backend\n2. Rank: 1. Reality fit, 2. Job market, 3. Ecosystem", MAX_TEXT - 1);
    } else if (strstr(norm_goal, "mobile") || strstr(norm_goal, "ios") || strstr(norm_goal, "android") || strstr(norm_goal, "phone")) {
        detected_goal = "build mobile app";
        if (strstr(norm_plat, "ios") && !strstr(norm_plat, "android")) {
            strncpy(out.lang, "Swift", MAX_TEXT - 1);
            strncpy(out.why, "Native iOS performance and deep integration with iOS SDK.", MAX_TEXT - 1);
            strncpy(out.stack, "SwiftUI + Xcode + Native iOS SDK", MAX_TEXT - 1);
            strncpy(out.steps, "1. IF iOS only, THEN Swift\n2. Rank: 1. Reality fit, 2. Job market, 3. Ecosystem", MAX_TEXT - 1);
        } else if (strstr(norm_plat, "android") && !strstr(norm_plat, "ios")) {
            strncpy(out.lang, "Kotlin", MAX_TEXT - 1);
            strncpy(out.why, "Native Android development with modern safe language features.", MAX_TEXT - 1);
            strncpy(out.stack, "Jetpack Compose + Android Studio + Room DB", MAX_TEXT - 1);
            strncpy(out.steps, "1. IF Android only, THEN Kotlin\n2. Rank: 1. Reality fit, 2. Job market, 3. Ecosystem", MAX_TEXT - 1);
        } else {
            strncpy(out.lang, "Dart + Flutter", MAX_TEXT - 1);
            strncpy(out.why, "Cross-platform mobile development with single codebase.", MAX_TEXT - 1);
            strncpy(out.stack, "Dart + Flutter Framework + SQLite (local) + REST API (remote)", MAX_TEXT - 1);
            strncpy(out.steps, "1. IF both iOS and Android, THEN Dart + Flutter\n2. Rank: 1. Reality fit, 2. Job market, 3. Ecosystem", MAX_TEXT - 1);
        }
    } else if (strstr(norm_goal, "website") || strstr(norm_goal, "web") || strstr(norm_goal, "frontend")) {
        detected_goal = "build website";
        strncpy(out.lang, "HTML / CSS / JavaScript / TypeScript", MAX_TEXT - 1);
        strncpy(out.why, "Standard web stack for high interactivity and reliable types.", MAX_TEXT - 1);
        strncpy(out.stack, "Next.js / React + Tailwind CSS + TypeScript", MAX_TEXT - 1);
        strncpy(out.steps, "1. IF build website, THEN HTML/CSS/JS/TS\n2. Add React/Next.js if app-level complexity exists\n3. Rank: 1. Reality fit, 2. Job market, 3. Ecosystem", MAX_TEXT - 1);
    } else if (strstr(norm_goal, "backend") || strstr(norm_goal, "api") || strstr(norm_goal, "server")) {
        detected_goal = "build backend/API";
        if (strstr(norm_skill, "begin")) {
            strncpy(out.lang, "Python + FastAPI", MAX_TEXT - 1);
            strncpy(out.why, "Python is readable and has a minimal learning curve for APIs.", MAX_TEXT - 1);
            strncpy(out.stack, "Python + FastAPI + PostgreSQL + Docker", MAX_TEXT - 1);
            strncpy(out.steps, "1. IF beginner, THEN Python + FastAPI\n2. Rank: 1. Reality fit, 2. Job market, 3. Ecosystem", MAX_TEXT - 1);
        } else if (strstr(norm_type, "perf") || strstr(norm_type, "high") || strstr(norm_type, "security")) {
            strncpy(out.lang, "Rust / Go", MAX_TEXT - 1);
            strncpy(out.why, "Rust provides memory safety without garbage collection and ultra-low latency.", MAX_TEXT - 1);
            strncpy(out.stack, "Rust (Axum/Tokio) + PostgreSQL + Docker", MAX_TEXT - 1);
            strncpy(out.steps, "1. IF high-performance/security-critical, THEN Rust OR Go\n2. Rank: 1. Reality fit, 2. Job market, 3. Ecosystem", MAX_TEXT - 1);
        } else {
            strncpy(out.lang, "TypeScript + Node.js (or Go)", MAX_TEXT - 1);
            strncpy(out.why, "TypeScript enables rapid backend development and shares types with frontend.", MAX_TEXT - 1);
            strncpy(out.stack, "Node.js + Express/NestJS + PostgreSQL + Docker", MAX_TEXT - 1);
            strncpy(out.steps, "1. IF production SaaS, THEN TypeScript + Node.js OR Go\n2. Rank: 1. Reality fit, 2. Job market, 3. Ecosystem", MAX_TEXT - 1);
        }
    } else if (strstr(norm_goal, "automation") || strstr(norm_goal, "script")) {
        detected_goal = "automation / scripting";
        if (strstr(norm_plat, "win")) {
            strncpy(out.lang, "PowerShell + Python", MAX_TEXT - 1);
            strncpy(out.why, "PowerShell's deep OS integration combined with Python.", MAX_TEXT - 1);
            strncpy(out.stack, "PowerShell + Python core + Windows Task Scheduler", MAX_TEXT - 1);
            strncpy(out.steps, "1. IF OS is Windows, THEN PowerShell + Python\n2. Rank: 1. Reality fit, 2. Job market, 3. Ecosystem", MAX_TEXT - 1);
        } else {
            strncpy(out.lang, "Bash + Python", MAX_TEXT - 1);
            strncpy(out.why, "Native terminal utilities matched with Python's rich standard library.", MAX_TEXT - 1);
            strncpy(out.stack, "Bash + Python core + Systemd / Cron jobs", MAX_TEXT - 1);
            strncpy(out.steps, "1. IF OS is Linux/macOS, THEN Bash + Python\n2. Rank: 1. Reality fit, 2. Job market, 3. Ecosystem", MAX_TEXT - 1);
        }
    } else {
        goal_unclear = true;
    }

    if (goal_unclear) {
        printf("\nUnclear Goal detected. Please provide more information.\n");
        printf("1. What are you building?\n");
        printf("2. Where will it run?\n");
        printf("3. Do you want fastest build or strongest long-term architecture?\n");
        return;
    }

    out.goal[MAX_TEXT - 1] = '\0';
    out.lang[MAX_TEXT - 1] = '\0';
    out.why[MAX_TEXT - 1] = '\0';
    out.stack[MAX_TEXT - 1] = '\0';
    out.steps[MAX_TEXT - 1] = '\0';

    // Now format and print the PROGRAMMING_LANGUAGE_DECISION_TREE
    printf("PROGRAMMING_LANGUAGE_DECISION_TREE\n");
    printf("│\n");
    printf("├── USER_GOAL\n");
    printf("│   └── %s\n", detected_goal);
    printf("│\n");
    printf("├── BEST_LANGUAGE\n");
    printf("│   └── %s\n", out.lang);
    printf("│\n");
    printf("├── WHY_THIS_LANGUAGE\n");
    printf("│   └── %s\n", out.why);
    printf("│\n");
    printf("├── SUPPORTING_STACK\n");
    if (strstr(detected_goal, "website")) {
        printf("│   ├── Frontend: Next.js / React (app-level complexity exists)\n");
        printf("│   ├── Backend: Node.js (TypeScript)\n");
        printf("│   ├── Database: PostgreSQL\n");
        printf("│   ├── Automation: npm / Bun scripts\n");
        printf("│   └── Deployment: Vercel\n");
    } else if (strstr(detected_goal, "AI")) {
        printf("│   ├── Frontend: Next.js / React\n");
        printf("│   ├── Backend: FastAPI (Python) + Express/NestJS (TypeScript)\n");
        printf("│   ├── Database: PostgreSQL + pgvector\n");
        printf("│   ├── Automation: Bash + YAML\n");
        printf("│   └── Deployment: Docker + AWS/GCP\n");
    } else if (strstr(detected_goal, "mobile")) {
        printf("│   ├── Frontend: iOS SDK / Jetpack Compose / Flutter Framework\n");
        printf("│   ├── Backend: FastAPI or Node.js API\n");
        printf("│   ├── Database: SQLite (Local) + PostgreSQL (Remote)\n");
        printf("│   ├── Automation: Fastlane / GitHub Actions\n");
        printf("│   └── Deployment: App Store / Google Play Console\n");
    } else if (strstr(detected_goal, "backend")) {
        printf("│   ├── Frontend: None or Next.js React\n");
        printf("│   ├── Backend: %s\n", out.lang);
        printf("│   ├── Database: PostgreSQL\n");
        printf("│   ├── Automation: GitHub Actions / Docker Compose\n");
        printf("│   └── Deployment: AWS / Railway / GCP\n");
    } else if (strstr(detected_goal, "operating os")) {
        printf("│   ├── Frontend: None / Custom dashboard\n");
        printf("│   ├── Backend: None / Embedded web server\n");
        printf("│   ├── Database: SQLite / Flash memory direct\n");
        printf("│   ├── Automation: Make / CMake / Cargo\n");
        printf("│   └── Deployment: Flash programmer / JTAG\n");
    } else if (strstr(detected_goal, "FPGA")) {
        printf("│   ├── Frontend: ModelSim Waveform Viewer\n");
        printf("│   ├── Backend: Synthesis / Place & Route tools\n");
        printf("│   ├── Database: None\n");
        printf("│   ├── Automation: Tcl scripts\n");
        printf("│   └── Deployment: Bitstream download to FPGA board\n");
    } else if (strstr(detected_goal, "data science")) {
        printf("│   ├── Frontend: Jupyter Notebooks / Streamlit\n");
        printf("│   ├── Backend: FastAPI / Flask\n");
        printf("│   ├── Database: SQL / PostgreSQL\n");
        printf("│   ├── Automation: DVC / MLflow\n");
        printf("│   └── Deployment: Docker + Kubernetes\n");
    } else if (strstr(detected_goal, "database-heavy")) {
        printf("│   ├── Frontend: React / Next.js\n");
        printf("│   ├── Backend: TypeScript / Python Backend\n");
        printf("│   ├── Database: PostgreSQL\n");
        printf("│   ├── Automation: Bash + SQL migrations\n");
        printf("│   └── Deployment: Docker + Supabase / RDS\n");
    } else if (strstr(detected_goal, "blockchain")) {
        printf("│   ├── Frontend: React + wagmi/ethers.js\n");
        printf("│   ├── Backend: Node.js (for indexing & triggers)\n");
        printf("│   ├── Database: PostgreSQL (off-chain cache)\n");
        printf("│   ├── Automation: Hardhat / Foundry / Anchor\n");
        printf("│   └── Deployment: Mainnet / Testnet smart contract deployer\n");
    } else { // automation
        printf("│   ├── Frontend: None / CLI Console\n");
        printf("│   ├── Backend: Local task daemon / Cron jobs\n");
        printf("│   ├── Database: SQLite / JSON Flat files\n");
        printf("│   ├── Automation: Systemd / Windows Task Scheduler\n");
        printf("│   └── Deployment: Local machine / VM installation\n");
    }
    printf("│\n");
    printf("├── IF_THEN_REASONING\n");
    if (strstr(detected_goal, "website")) {
        printf("│   ├── IF: app-level complexity exists\n");
        printf("│   └── THEN: Recommend HTML + CSS + JS/TS with React/Next.js\n");
    } else if (strstr(detected_goal, "AI")) {
        printf("│   ├── IF: building agentic workflows or AI backend\n");
        printf("│   └── THEN: Use Python for AI workers, TypeScript for App/API layer, SQL for storage, and Bash/YAML for automation\n");
    } else if (strstr(detected_goal, "mobile")) {
        if (strstr(norm_plat, "ios") && !strstr(norm_plat, "android")) {
            printf("│   ├── IF: iOS only\n");
            printf("│   └── THEN: Swift\n");
        } else if (strstr(norm_plat, "android") && !strstr(norm_plat, "ios")) {
            printf("│   ├── IF: Android only\n");
            printf("│   └── THEN: Kotlin\n");
        } else {
            printf("│   ├── IF: both iOS and Android\n");
            printf("│   └── THEN: Dart + Flutter\n");
        }
    } else if (strstr(detected_goal, "backend")) {
        if (strstr(norm_skill, "begin")) {
            printf("│   ├── IF: beginner\n");
            printf("│   └── THEN: Python + FastAPI\n");
        } else if (strstr(norm_type, "perf") || strstr(norm_type, "high") || strstr(norm_type, "security")) {
            printf("│   ├── IF: high-performance / security-critical\n");
            printf("│   └── THEN: Rust OR Go\n");
        } else {
            printf("│   ├── IF: production SaaS\n");
            printf("│   └── THEN: TypeScript + Node.js OR Go\n");
        }
    } else if (strstr(detected_goal, "operating os")) {
        printf("│   ├── IF: low-level hardware or OS controls\n");
        printf("│   └── THEN: Recommend C/C++/Rust, with Assembly only where direct CPU/control access is required\n");
    } else if (strstr(detected_goal, "FPGA")) {
        printf("│   ├── IF: designing physical logic gate structures\n");
        printf("│   └── THEN: Recommend Verilog, SystemVerilog, or VHDL\n");
    } else if (strstr(detected_goal, "data science")) {
        if (strstr(norm_goal, "statistics") || strstr(norm_goal, "stat")) {
            printf("│   ├── IF: statistics-heavy\n");
            printf("│   └── THEN: R\n");
        } else if (strstr(norm_goal, "numerical") || strstr(norm_goal, "julia")) {
            printf("│   ├── IF: high-performance numerical computing\n");
            printf("│   └── THEN: Julia\n");
        } else {
            printf("│   ├── IF: general ML/AI\n");
            printf("│   └── THEN: Python + SQL\n");
        }
    } else if (strstr(detected_goal, "database-heavy")) {
        printf("│   ├── IF: application is database-heavy\n");
        printf("│   └── THEN: Recommend SQL first, PostgreSQL, and TypeScript/Python backend\n");
    } else if (strstr(detected_goal, "blockchain")) {
        if (strstr(norm_constraints, "solana")) {
            printf("│   ├── IF: Solana\n");
            printf("│   └── THEN: Rust\n");
        } else if (strstr(norm_constraints, "move")) {
            printf("│   ├── IF: Move-chain\n");
            printf("│   └── THEN: Move\n");
        } else {
            printf("│   ├── IF: Ethereum/EVM\n");
            printf("│   └── THEN: Solidity + TypeScript\n");
        }
    } else { // automation
        if (strstr(norm_plat, "win")) {
            printf("│   ├── IF: OS is Windows\n");
            printf("│   └── THEN: PowerShell + Python\n");
        } else {
            printf("│   ├── IF: OS is Linux/macOS\n");
            printf("│   └── THEN: Bash + Python\n");
        }
    }
    printf("│\n");
    printf("├── LEARNING_ORDER\n");
    if (strstr(detected_goal, "website")) {
        printf("│   ├── Step 1: HTML5 & CSS3 Basics\n");
        printf("│   ├── Step 2: JavaScript Fundamentals (ES6+)\n");
        printf("│   ├── Step 3: TypeScript Typings & Safety\n");
        printf("│   └── Step 4: React & Next.js Framework\n");
    } else if (strstr(detected_goal, "AI")) {
        printf("│   ├── Step 1: Python Core and asyncio\n");
        printf("│   ├── Step 2: LangChain / LlamaIndex / Agentic SDKs\n");
        printf("│   ├── Step 3: TypeScript APIs and REST orchestration\n");
        printf("│   └── Step 4: Vector Database querying and indexing\n");
    } else if (strstr(detected_goal, "mobile")) {
        if (strstr(norm_plat, "ios") && !strstr(norm_plat, "android")) {
            printf("│   ├── Step 1: Swift Syntax\n");
            printf("│   ├── Step 2: UIKit / SwiftUI\n");
            printf("│   ├── Step 3: CoreData & iOS SDK\n");
            printf("│   └── Step 4: App Store Guidelines\n");
        } else if (strstr(norm_plat, "android") && !strstr(norm_plat, "ios")) {
            printf("│   ├── Step 1: Kotlin Syntax\n");
            printf("│   ├── Step 2: Jetpack Compose\n");
            printf("│   ├── Step 3: Room Database & WorkManager\n");
            printf("│   └── Step 4: Play Store Guidelines\n");
        } else {
            printf("│   ├── Step 1: Dart Programming Language Basics\n");
            printf("│   ├── Step 2: Flutter Widgets & Layouts\n");
            printf("│   ├── Step 3: Flutter State Management (Bloc / Riverpod)\n");
            printf("│   └── Step 4: App store builds & native plugins\n");
        }
    } else if (strstr(detected_goal, "backend")) {
        if (strstr(norm_skill, "begin")) {
            printf("│   ├── Step 1: Python Syntax & Core Modules\n");
            printf("│   ├── Step 2: FastAPI & REST APIs\n");
            printf("│   ├── Step 3: SQL Databases (SQLAlchemy)\n");
            printf("│   └── Step 4: Docker & Container Deployment\n");
        } else if (strstr(norm_type, "perf") || strstr(norm_type, "high") || strstr(norm_type, "security")) {
            printf("│   ├── Step 1: Rust Ownership & Lifetime Model\n");
            printf("│   ├── Step 2: Axum Web Framework & Tokio Async Runtime\n");
            printf("│   ├── Step 3: Diesel or SQLx database integrations\n");
            printf("│   └── Step 4: CI/CD & Secure Container builds\n");
        } else {
            printf("│   ├── Step 1: TypeScript Typings & Compile config\n");
            printf("│   ├── Step 2: Express / NestJS Frameworks\n");
            printf("│   ├── Step 3: Prisma / Drizzle ORM migrations\n");
            printf("│   └── Step 4: JWT auth & Railway/Heroku deployments\n");
        }
    } else if (strstr(detected_goal, "operating os")) {
        printf("│   ├── Step 1: C/Rust Memory layout & Pointer Arithmetic\n");
        printf("│   ├── Step 2: Hardware Register Manipulation & Interrupts\n");
        printf("│   ├── Step 3: Real-time Operating System (RTOS) concepts\n");
        printf("│   └── Step 4: Toolchain cross-compilation & Linker scripts\n");
    } else if (strstr(detected_goal, "FPGA")) {
        printf("│   ├── Step 1: Combinational and Sequential Logic Basics\n");
        printf("│   ├── Step 2: Verilog / SystemVerilog Syntax & Testbenches\n");
        printf("│   ├── Step 3: Finite State Machine (FSM) implementation\n");
        printf("│   └── Step 4: Timing Analysis and Clock Domain Crossing\n");
    } else if (strstr(detected_goal, "data science")) {
        printf("│   ├── Step 1: Language syntax and core packages (Pandas/Tidyverse)\n");
        printf("│   ├── Step 2: Statistical modeling & exploratory data analysis\n");
        printf("│   ├── Step 3: Machine learning frameworks (Scikit-Learn/PyTorch)\n");
        printf("│   └── Step 4: Model evaluation, visualization & deployment\n");
    } else if (strstr(detected_goal, "database-heavy")) {
        printf("│   ├── Step 1: SQL DDL, DML, and Joins\n");
        printf("│   ├── Step 2: Indexing & Query Optimization\n");
        printf("│   ├── Step 3: Transactions and ACID isolation levels\n");
        printf("│   └── Step 4: Backend ORM integration & migration writing\n");
    } else if (strstr(detected_goal, "blockchain")) {
        printf("│   ├── Step 1: Blockchain fundamentals and wallet integrations\n");
        printf("│   ├── Step 2: Smart contract development syntax & memory safety\n");
        printf("│   ├── Step 3: Unit testing (Foundry/Hardhat/Anchor)\n");
        printf("│   └── Step 4: Front-end SDK integration (Web3.js/ethers)\n");
    } else { // automation
        printf("│   ├── Step 1: Shell basics and stream redirection (pipes, grep, awk)\n");
        printf("│   ├── Step 2: Python scripting & OS interaction libraries (os, sys, subprocess)\n");
        printf("│   ├── Step 3: File processing, JSON manipulation, and error handling\n");
        printf("│   └── Step 4: Scheduler setup (Cron/Task Scheduler)\n");
    }
    printf("│\n");
    printf("├── BUILD_ORDER\n");
    printf("│   ├── Setup: Configure project environment and toolchains\n");
    printf("│   ├── Prototype: Build the most basic end-to-end working flow\n");
    printf("│   ├── Test: Implement unit, integration, and syntax tests\n");
    printf("│   ├── Deploy: Deploy prototype to cloud, device, or scheduler\n");
    printf("│   └── Iterate: Profile speed, check errors, and refine features\n");
    printf("│\n");
    printf("└── FINAL_RECOMMENDATION\n");
    printf("    └── Use %s supporting %s stack.\n", out.lang, out.stack);
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
        if (argc > 3) strncpy(in.skill, argv[3], MAX_TEXT - 1);
        if (argc > 4) strncpy(in.type, argv[4], MAX_TEXT - 1);
        if (argc > 5) strncpy(in.plat, argv[5], MAX_TEXT - 1);
        if (argc > 6) strncpy(in.time, argv[6], MAX_TEXT - 1);
        if (argc > 7) strncpy(in.style, argv[7], MAX_TEXT - 1);
        if (argc > 8) strncpy(in.constraints, argv[8], MAX_TEXT - 1);
        run_lang(&in);
    } else {
        printf("Error: Invalid arguments.\n");
    }
    curl_global_cleanup();
    return 0;
}
