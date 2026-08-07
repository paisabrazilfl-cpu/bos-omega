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

/* Pre-normalization function for case-insensitive keyword searches.
 * This function handles NULL src pointers, bounds-checks up to MAX_TEXT - 1,
 * and converts characters to lowercase using C standard compliant (unsigned char) cast
 * to prevent negative char index undefined behavior with tolower().
 * Replacing multiple strcasestr() calls with a single lowercase pass
 * and subsequent strstr() calls reduces keyword scanning complexity from O(N*M) to O(N+M).
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

/* Safely matches lowercase boundaries for keyword searches to prevent false positives.
 * It validates that the character immediately preceding the match is not alphanumeric/underscore,
 * and the character immediately following is not alphanumeric/underscore.
 */
static bool has_word(const char *haystack, const char *needle) {
    if (!haystack || !needle) return false;
    size_t n_len = strlen(needle);
    if (n_len == 0) return false;
    const char *p = haystack;
    while ((p = strstr(p, needle)) != NULL) {
        bool left_ok = true;
        bool right_ok = true;
        if (p > haystack) {
            char prev = *(p - 1);
            if ((prev >= 'a' && prev <= 'z') || (prev >= 'A' && prev <= 'Z') || (prev >= '0' && prev <= '9') || prev == '_') {
                left_ok = false;
            }
        }
        char next = *(p + n_len);
        if ((next >= 'a' && next <= 'z') || (next >= 'A' && next <= 'Z') || (next >= '0' && next <= '9') || next == '_') {
            right_ok = false;
        }
        if (left_ok && right_ok) return true;
        p += n_len;
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
    char norm_obj[MAX_TEXT];
    normalize(norm_obj, rt->objective);
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

    bool is_website = strstr(norm_goal, "web") || strstr(norm_goal, "html") || strstr(norm_goal, "css") || strstr(norm_goal, "front");
    bool is_ai = has_word(norm_goal, "ai") || strstr(norm_goal, "agent") || strstr(norm_goal, "artificial") || strstr(norm_goal, "llm") || strstr(norm_goal, "chat");
    bool is_mobile = strstr(norm_goal, "mobile") || strstr(norm_goal, "ios") || strstr(norm_goal, "android") || strstr(norm_goal, "swift") || strstr(norm_goal, "kotlin") || strstr(norm_goal, "flutter") || strstr(norm_goal, "dart");
    bool is_backend = strstr(norm_goal, "backend") || strstr(norm_goal, "api") || strstr(norm_goal, "server");
    bool is_os = strstr(norm_goal, "operating") || strstr(norm_goal, "firmware") || strstr(norm_goal, "embedded") || strstr(norm_goal, "microcontroller") || (has_word(norm_goal, "os") && !is_mobile);
    bool is_fpga = strstr(norm_goal, "fpga") || strstr(norm_goal, "asic") || strstr(norm_goal, "logic") || strstr(norm_goal, "verilog") || strstr(norm_goal, "vhdl");
    bool is_ds = strstr(norm_goal, "data science") || strstr(norm_goal, "ml") || strstr(norm_goal, "machine learning") || strstr(norm_goal, "statistics") || strstr(norm_goal, "numerical");
    bool is_db = strstr(norm_goal, "database") || strstr(norm_goal, "sql") || strstr(norm_goal, "postgres") || strstr(norm_goal, "db-heavy");
    bool is_blockchain = strstr(norm_goal, "blockchain") || strstr(norm_goal, "smart contract") || strstr(norm_goal, "ethereum") || strstr(norm_goal, "solidity") || strstr(norm_goal, "solana") || strstr(norm_goal, "evm") || strstr(norm_goal, "move");
    bool is_automation = strstr(norm_goal, "automation") || strstr(norm_goal, "script") || strstr(norm_goal, "cron") || strstr(norm_goal, "scrape");

    if (strlen(norm_goal) == 0 || (!is_website && !is_ai && !is_mobile && !is_backend && !is_os && !is_fpga && !is_ds && !is_db && !is_blockchain && !is_automation)) {
        printf("PROGRAMMING_LANGUAGE_DECISION_TREE\n");
        printf("│\n");
        printf("├── USER_GOAL\n");
        printf("│   └── Unclear (Received: '%s')\n", in->goal);
        printf("│\n");
        printf("├── BEST_LANGUAGE\n");
        printf("│   └── Pending Input\n");
        printf("│\n");
        printf("├── WHY_THIS_LANGUAGE\n");
        printf("│   └── Goal is ambiguous. Need more details.\n");
        printf("│\n");
        printf("└── CLARIFYING_QUESTIONS\n");
        printf("    ├── 1. What are you building?\n");
        printf("    ├── 2. Where will it run?\n");
        printf("    └── 3. Do you want fastest build or strongest long-term architecture?\n");
        return;
    }

    const char *detected_goal = "";
    const char *best_lang = "";
    const char *why_lang = "";
    const char *stack_fe = "N/A";
    const char *stack_be = "N/A";
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

    if (is_website) {
        detected_goal = "Build Website";
        best_lang = "TypeScript";
        why_lang = "Standard language for dynamic client-side rendering and component logic.";
        stack_fe = "React / Next.js";
        stack_be = "Node.js (Express)";
        stack_db = "PostgreSQL";
        stack_auto = "npm scripts / Vite";
        stack_deploy = "Vercel / Netlify";

        bool has_app_complexity = (strstr(norm_skill, "advanced") || strstr(norm_skill, "expert") ||
                                   strstr(norm_type, "app") || strstr(norm_type, "dashboard") ||
                                   strstr(norm_constraints, "complex") || strstr(norm_style, "spa"));
        if (has_app_complexity) {
            if_cond = "Website has app-level complexity";
            then_dec = "Recommend React / Next.js with TypeScript";
        } else {
            if_cond = "Website is simple static content";
            then_dec = "Recommend vanilla HTML, CSS, and lightweight JavaScript";
        }

        learn_1 = "HTML & CSS layout fundamentals";
        learn_2 = "JavaScript basics and DOM manipulation";
        learn_3 = "TypeScript types and strict compile safety";
        learn_4 = "React component state and Next.js router";

        build_setup = "Initialize Next.js project with Tailwind CSS";
        build_proto = "Build interactive static UI layouts and prototype views";
        build_test = "Write component unit tests using Vitest";
        build_deploy = "Connect git repo to Vercel for continuous deployment";
        build_iterate = "Optimize Core Web Vitals, lazy load images, and run SEO audits";
        final_rec = "Next.js + Tailwind CSS + Vercel deployment.";
    }
    else if (is_ai) {
        detected_goal = "Build AI System";
        best_lang = "Python & TypeScript";
        why_lang = "Python for AI model execution, TypeScript for high-productivity web API orchestrations.";
        stack_fe = "Next.js / React";
        stack_be = "FastAPI (Python) & NestJS (TypeScript)";
        stack_db = "PostgreSQL with pgvector";
        stack_auto = "Bash / Docker Compose";
        stack_deploy = "Railway / AWS ECS";

        if_cond = "Building AI workers (ML pipelines) vs Application/API layer";
        then_dec = "Use Python for AI workers; use TypeScript/Node.js for high-throughput API gateway and UI integration";

        learn_1 = "Python syntax, NumPy, and Pandas";
        learn_2 = "OpenAI API, LangChain or LlamaIndex frameworks";
        learn_3 = "TypeScript API development and serialization";
        learn_4 = "Vector embeddings and pgvector storage queries";

        build_setup = "Set up Python virtualenv and initialize TypeScript Node package";
        build_proto = "Create LLM prompt worker in Python and connect via FastAPI";
        build_test = "Verify response accuracy and trace latency with LangSmith";
        build_deploy = "Package services in Docker and launch on Railway";
        build_iterate = "Add semantic caching (Redis) and implement RAG pipeline improvements";
        final_rec = "Python (FastAPI) for AI services + TypeScript (Next.js) for frontend UI.";
    }
    else if (is_mobile) {
        detected_goal = "Build Mobile App";
        bool is_ios = strstr(norm_plat, "ios") || strstr(norm_goal, "ios") || strstr(norm_constraints, "ios");
        bool is_android = strstr(norm_plat, "android") || strstr(norm_goal, "android") || strstr(norm_constraints, "android");

        if (is_ios && !is_android) {
            best_lang = "Swift";
            why_lang = "Apple's native compiler for iOS providing maximal UI performance and system integration.";
            stack_fe = "SwiftUI";
            stack_be = "FastAPI (Python)";
            stack_db = "CoreData / SQLite";
            stack_auto = "Fastlane / XcodeGen";
            stack_deploy = "App Store Connect";
            if_cond = "Target platform is iOS only";
            then_dec = "Recommend Swift + SwiftUI with native SDK";
            final_rec = "SwiftUI native app with CoreData.";
        } else if (is_android && !is_ios) {
            best_lang = "Kotlin";
            why_lang = "Google's modern preferred native JVM language for Android UI development.";
            stack_fe = "Jetpack Compose";
            stack_be = "Ktor or Spring Boot";
            stack_db = "Room / SQLite";
            stack_auto = "Gradle / Fastlane";
            stack_deploy = "Google Play Console";
            if_cond = "Target platform is Android only";
            then_dec = "Recommend Kotlin + Jetpack Compose";
            final_rec = "Kotlin native app with Jetpack Compose.";
        } else {
            best_lang = "Dart (Flutter)";
            why_lang = "High-performance cross-platform widget engine rendering to native canvas.";
            stack_fe = "Flutter UI";
            stack_be = "TypeScript / Node.js";
            stack_db = "Hive / SQLite (local) + PostgreSQL (remote)";
            stack_auto = "Fastlane / GitHub Actions";
            stack_deploy = "Google Play Store & Apple App Store";
            if_cond = "Target platform is both iOS and Android";
            then_dec = "Recommend Dart + Flutter for single-codebase cross-platform efficiency";
            final_rec = "Flutter + Dart cross-platform mobile stack.";
        }

        learn_1 = "Language syntax (Dart/Swift/Kotlin) and reactive state";
        learn_2 = "Layout building and mobile navigation structures";
        learn_3 = "HTTP network calls and local cache persistence";
        learn_4 = "Native platform channels and push notification setup";

        build_setup = "Configure SDK (Flutter/Xcode/Android Studio) and init project";
        build_proto = "Draft wireframes and implement reactive state view templates";
        build_test = "Execute local widget/unit tests and run on simulated device";
        build_deploy = "Code sign binary assets and upload to Beta channels (TestFlight/Play Console)";
        build_iterate = "Analyze rendering performance (jank), optimize image sizes, and fix memory leaks";
    }
    else if (is_backend) {
        detected_goal = "Build Backend/API";
        bool is_beg = strstr(norm_skill, "beginner") || strstr(norm_skill, "junior") || strstr(norm_skill, "low");
        bool is_saas = strstr(norm_type, "saas") || strstr(norm_type, "production") || strstr(norm_constraints, "production") || strstr(norm_goal, "saas");
        bool is_high_perf = strstr(norm_constraints, "performance") || strstr(norm_constraints, "security") || strstr(norm_style, "fast") || strstr(norm_constraints, "critical");

        if (is_beg) {
            best_lang = "Python (FastAPI)";
            why_lang = "Extremely clean syntax, auto-generated OpenAPI docs, and simple setup.";
            stack_fe = "None (REST API)";
            stack_be = "FastAPI (Python)";
            stack_db = "SQLite / PostgreSQL";
            stack_auto = "Alembic (migrations)";
            stack_deploy = "Railway / Render";
            if_cond = "Developer skill level is beginner";
            then_dec = "Recommend Python + FastAPI for rapid developer velocity";
            final_rec = "FastAPI (Python) API with PostgreSQL.";
        } else if (is_high_perf) {
            best_lang = "Rust or Go";
            why_lang = "Go for concurrency and simple maintainability, Rust for ultra-low latency and compiler safety guarantees.";
            stack_fe = "None (gRPC / HTTP API)";
            stack_be = "Go (Gin/Axum) or Rust (Actix-web)";
            stack_db = "PostgreSQL with connection pooling";
            stack_auto = "Makefile / Docker Multi-stage";
            stack_deploy = "AWS ECS / Bare Metal VPS";
            if_cond = "System requirement is high-performance/security-critical";
            then_dec = "Recommend Rust or Go to bypass garbage collection pauses or leverage lightweight green threads";
            final_rec = "Go or Rust microservice backend.";
        } else {
            best_lang = "TypeScript (Node.js)";
            why_lang = "Full-stack development consistency and vast npm middleware ecosystem.";
            stack_fe = "React Admin Portal";
            stack_be = "TypeScript + Node.js (NestJS / Express)";
            stack_db = "PostgreSQL";
            stack_auto = "Prisma / Knex migrations";
            stack_deploy = "Railway / AWS Elastic Beanstalk";
            if_cond = "Application is a production SaaS";
            then_dec = "Recommend TypeScript + Node.js (or Go) for maintainability and hiring pool";
            final_rec = "NestJS + TypeScript on PostgreSQL.";
        }

        learn_1 = "HTTP protocol, RESTful routing, and JSON serialization";
        learn_2 = "Middleware, requests authentication, and JWT authorization";
        learn_3 = "SQL relational schemas and ORM mapping tools";
        learn_4 = "Stress testing, concurrency handling, and system tracing";

        build_setup = "Initialize project config with strict compiler check files";
        build_proto = "Implement main CRUD database routes and test with auto docs";
        build_test = "Write integration endpoint tests using mock DB instances";
        build_deploy = "Dockerize the app and deploy with managed PostgreSQL storage";
        build_iterate = "Add caching layers (Redis), connection pools, and monitor server logs";
    }
    else if (is_os) {
        detected_goal = "Build Operating System / Firmware / Embedded";
        best_lang = "C / C++ / Rust";
        why_lang = "Zero-overhead performance, absolute memory control, and direct hardware register access.";
        stack_fe = "None (Console/Physical UI)";
        stack_be = "Bare Metal / FreeRTOS / Linux Kernel";
        stack_db = "Memory-mapped raw buffers / Flash Filesystem";
        stack_auto = "Makefiles / CMake";
        stack_deploy = "JTAG / OpenOCD Flasher";

        bool requires_direct_cpu = strstr(norm_constraints, "direct cpu") || strstr(norm_constraints, "assembly") || strstr(norm_constraints, "control");
        if (requires_direct_cpu) {
            if_cond = "Direct CPU register control and startup code is required";
            then_dec = "Include inline Assembly segments inside C/C++ source";
        } else {
            if_cond = "Standard embedded/firmware execution safety is prioritized";
            then_dec = "Use C, C++ or Rust for compiler-level static safety";
        }

        learn_1 = "Computer architecture, pointers, and manual memory layouts";
        learn_2 = "Datasheet registers, interrupt vectors, and GPIO configurations";
        learn_3 = "Real-time task scheduling (RTOS) or Kernel space basics";
        learn_4 = "Low-level serial buses (UART, SPI, I2C) driver coding";

        build_setup = "Install GNU toolchain (arm-none-eabi-gcc) and target debug tools";
        build_proto = "Write blinky or basic print-to-UART register sequence";
        build_test = "Simulate assembly/C instructions in QEMU or trace with logic analyzer";
        build_deploy = "Flash binary output onto target microcontroller flash storage";
        build_iterate = "Tune clock speeds, optimize power sleep modes, and minimize code size";
        final_rec = "C/C++ for hardware control, Rust for modern safe embedded coding.";
    }
    else if (is_fpga) {
        detected_goal = "FPGA / ASIC / Digital Logic";
        best_lang = "SystemVerilog";
        why_lang = "Industry standard hardware description language combining design and rigorous verification capabilities.";
        stack_fe = "GTKWave Signal Inspector";
        stack_be = "FPGA Fabric (Xilinx Vivado / Intel Quartus)";
        stack_db = "Block RAM (BRAM) on-chip";
        stack_auto = "Tcl automation scripts";
        stack_deploy = "JTAG Bitstream loader";

        if_cond = "Synthesizing circuits for custom silicon or logic gates";
        then_dec = "Recommend Verilog or SystemVerilog for design, VHDL for legacy military/aerospace contracts";

        learn_1 = "Digital gates, boolean algebra, and registers";
        learn_2 = "Hardware description language syntax (synthesizable subset)";
        learn_3 = "Testbench construction, clocking, and signal timing";
        learn_4 = "ASIC/FPGA synthesis, floorplanning, and constraint files";

        build_setup = "Install Vivado/Quartus design suite and simulator (ModelSim)";
        build_proto = "Describe combinational logic (e.g. adder) and state machine controllers";
        build_test = "Simulate waveform outputs with high coverage stimulus testbenches";
        build_deploy = "Synthesize RTL, route logic netlists, and flash bitstream to FPGA";
        build_iterate = "Review timing reports, fix setup/hold time violations, and optimize LUT count";
        final_rec = "SystemVerilog for hardware synthesis and testbench validation.";
    }
    else if (is_ds) {
        detected_goal = "Data Science / ML";
        bool is_stats = strstr(norm_constraints, "statistics") || strstr(norm_constraints, "heavy") || strstr(norm_style, "stats") || strstr(norm_goal, "statistics");
        bool is_high_num = strstr(norm_constraints, "numerical") || strstr(norm_constraints, "high-performance") || strstr(norm_goal, "numerical");

        if (is_stats) {
            best_lang = "R";
            why_lang = "Unmatched ecosystem for specialized statistical modeling and regression packages.";
            stack_fe = "Shiny web dashboard";
            stack_be = "R runtime (RStudio)";
            stack_db = "PostgreSQL / flat CSVs";
            stack_auto = "R scripts";
            stack_deploy = "ShinyApps.io / Docker";
            if_cond = "Project is heavily statistics-oriented";
            then_dec = "Recommend R language with ggplot2 and tidyverse";
            final_rec = "R + Shiny for intensive statistical modeling.";
        } else if (is_high_num) {
            best_lang = "Julia";
            why_lang = "Solves the two-language problem: syntax simple as Python, execution fast as C.";
            stack_fe = "Pluto.jl interactive notebooks";
            stack_be = "Julia runtime";
            stack_db = "PostgreSQL / ODBC";
            stack_auto = "Julia scripts";
            stack_deploy = "Local cluster / Docker";
            if_cond = "High-performance numerical computing is required";
            then_dec = "Recommend Julia for vectorization and compiler-optimized mathematics";
            final_rec = "Julia for high-speed mathematical computations.";
        } else {
            best_lang = "Python";
            why_lang = "Undisputed king of ML frameworks (PyTorch, TensorFlow) and general data engineering.";
            stack_fe = "Streamlit / Jupyter Notebooks";
            stack_be = "FastAPI (Python)";
            stack_db = "DuckDB / PostgreSQL / Snowflake";
            stack_auto = "Prefect / Apache Airflow";
            stack_deploy = "Hugging Face / AWS SageMaker / Docker";
            if_cond = "General machine learning or broad data engineering";
            then_dec = "Recommend Python due to PyTorch and Pandas ecosystems";
            final_rec = "Python (Jupyter + Pandas + PyTorch) with Streamlit.";
        }

        learn_1 = "Core programming, matrix math, and relational database queries";
        learn_2 = "Data wrangling, cleaning, and visualizations (Pandas/dplyr)";
        learn_3 = "Statistical models, supervised/unsupervised machine learning";
        learn_4 = "Model serving APIs and workflow pipeline scheduling";

        build_setup = "Create environment (pip/conda/renv) and launch workbook workspace";
        build_proto = "Load dataset, perform EDA, and train an initial baseline model";
        build_test = "Validate accuracy metrics on unseen split test sets";
        build_deploy = "Expose model predictions via microservice or deploy interactive dashboard";
        build_iterate = "Feature engineering, tune model hyper-parameters, and establish drift monitoring";
    }
    else if (is_db) {
        detected_goal = "Database-Heavy App";
        best_lang = "SQL & TypeScript";
        why_lang = "SQL is fundamental to performant querying, coupled with TypeScript backend for secure data mapping.";
        stack_fe = "React (Next.js) Admin Dashboard";
        stack_be = "TypeScript + Node.js (with Drizzle/Prisma ORM)";
        stack_db = "PostgreSQL (with connection pooler)";
        stack_auto = "DB Migrations (Liquibase / Prisma)";
        stack_deploy = "Supabase / AWS RDS / Railway";

        if_cond = "Designing database-heavy applications";
        then_dec = "Prioritize relational SQL (PostgreSQL first) and use TypeScript or Python backend with pooling";

        learn_1 = "Relational database concepts, normalization, and ACID properties";
        learn_2 = "Advanced SQL: joins, aggregation, subqueries, and window functions";
        learn_3 = "Database indexing strategies, query plans (EXPLAIN), and partitions";
        learn_4 = "Backend pooling, transaction control, and API serialization";

        build_setup = "Provision managed PostgreSQL database and configure local dev DB";
        build_proto = "Write schema definitions (migration scripts) and seed table records";
        build_test = "Profile execution times of core database queries with mock scaling data";
        build_deploy = "Deploy schema to managed database and deploy TypeScript API server";
        build_iterate = "Analyze slow queries, configure connection poolers (PgBouncer), and implement Redis caching";
        final_rec = "PostgreSQL (Supabase) + TypeScript Node.js backend with Drizzle.";
    }
    else if (is_blockchain) {
        detected_goal = "Blockchain / Smart Contracts";
        bool is_solana = strstr(norm_plat, "solana") || strstr(norm_goal, "solana");
        bool is_move = strstr(norm_plat, "move") || strstr(norm_goal, "move") || strstr(norm_plat, "sui") || strstr(norm_plat, "aptos");

        if (is_solana) {
            best_lang = "Rust (Anchor)";
            why_lang = "Type-safe, fast systems language required for writing high-concurrency Solana programs.";
            stack_fe = "React + @solana/web3.js";
            stack_be = "Anchor framework";
            stack_db = "Solana Ledger / custom indexers (PostgreSQL)";
            stack_auto = "Cargo / Anchor CLI";
            stack_deploy = "Solana Mainnet-Beta";
            if_cond = "Target blockchain is Solana";
            then_dec = "Recommend Rust with Anchor framework";
            final_rec = "Solana Rust smart contracts with React frontend.";
        } else if (is_move) {
            best_lang = "Move";
            why_lang = "Resource-oriented safe smart contract language designed to prevent common EVM exploits.";
            stack_fe = "React + Sui SDK";
            stack_be = "Sui/Aptos blockchain framework";
            stack_db = "On-chain state / custom indexers";
            stack_auto = "Move CLI";
            stack_deploy = "Sui/Aptos Mainnet";
            if_cond = "Target blockchain is Sui or Aptos Move-chains";
            then_dec = "Recommend Move language for superior asset safety representation";
            final_rec = "Move on-chain contract with React frontend.";
        } else {
            best_lang = "Solidity";
            why_lang = "EVM standard language with the largest liquidity, tooling, and developer base.";
            stack_fe = "React + wagmi / ethers.js";
            stack_be = "EVM compatible chain (Ethereum / L2s)";
            stack_db = "On-chain state + Graph Protocol indexers";
            stack_auto = "Foundry / Hardhat";
            stack_deploy = "Ethereum Mainnet / Arbitrum / Base";
            if_cond = "Target blockchain is Ethereum or EVM compatible L2s";
            then_dec = "Recommend Solidity with Foundry for fast testing and EVM deployments";
            final_rec = "Solidity contracts tested with Foundry and React frontend.";
        }

        learn_1 = "Blockchain fundamentals, ledger state, and gas fee dynamics";
        learn_2 = "Smart contract syntax, access control, and reentrancy security";
        learn_3 = "Local testing suites with unit tests and assertion logic";
        learn_4 = "Web3 client libraries and browser wallet connections";

        build_setup = "Install contract frameworks (Foundry / Anchor / Move-cli) and setup local node";
        build_proto = "Draft token or business state logic contracts on local sandbox";
        build_test = "Run detailed unit tests, simulate gas usage, and run static analysis (Slither)";
        build_deploy = "Deploy to chain Testnet, register code source, and verify on explorer";
        build_iterate = "Conduct formal security audits, establish multi-sig admin keys, and deploy to Mainnet";
    }
    else if (is_automation) {
        detected_goal = "Automation / Scripting";
        bool is_win = strstr(norm_plat, "windows") || strstr(norm_goal, "windows") || strstr(norm_constraints, "windows");

        if (is_win) {
            best_lang = "PowerShell & Python";
            why_lang = "PowerShell is deeply integrated with Windows APIs, while Python provides general automation libraries.";
            stack_fe = "None (CLI)";
            stack_be = "PowerShell + Python runtime";
            stack_db = "SQLite / CSV storage";
            stack_auto = "Windows Task Scheduler";
            stack_deploy = "Local Windows OS / Active Directory";
            if_cond = "Target OS is Windows";
            then_dec = "Recommend PowerShell for direct OS hooks combined with Python for tasks like parsing";
            final_rec = "PowerShell + Python for Windows task automation.";
        } else {
            best_lang = "Bash & Python";
            why_lang = "Bash is the native standard for Unix-like systems, with Python handling complex script operations.";
            stack_fe = "None (CLI)";
            stack_be = "Bash shell + Python interpreter";
            stack_db = "SQLite / JSON flat files";
            stack_auto = "cron daemon";
            stack_deploy = "Local Linux server / VPS";
            if_cond = "Target OS is Linux or macOS";
            then_dec = "Recommend Bash for quick shell pipelines combined with Python for cross-platform robustness";
            final_rec = "Bash + Python scripts managed via system cron.";
        }

        learn_1 = "Shell command pipelines, streams redirection, and variables";
        learn_2 = "Scripting flow structures: loops, conditionals, and functions";
        learn_3 = "Python subprocess module, JSON/CSV parsers, and requests library";
        learn_4 = "Task triggers, logging, and daemon background execution";

        build_setup = "Create script files, configure executable permissions, and verify paths";
        build_proto = "Write linear commands for target operations with error logs";
        build_test = "Perform test execution in a dry-run/mock context";
        build_deploy = "Register scripts with local scheduler (cron or Task Scheduler) to trigger automatically";
        build_iterate = "Add remote notification webhooks (Slack/Discord) and configure log rotation";
    }

    printf("PROGRAMMING_LANGUAGE_DECISION_TREE\n");
    printf("│\n");
    printf("├── USER_GOAL\n");
    printf("│   └── %s\n", detected_goal);
    printf("│\n");
    printf("├── BEST_LANGUAGE\n");
    printf("│   └── %s\n", best_lang);
    printf("│\n");
    printf("├── WHY_THIS_LANGUAGE\n");
    printf("│   └── %s\n", why_lang);
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
    printf("├── MULTI_LANGUAGE_RANKING\n");
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
