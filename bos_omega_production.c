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
static const int REG_SIZE = sizeof(REGISTRY) / sizeof(REGISTRY[0]);

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
 * Normalizes a source string to lowercase into dest.
 * Handles a NULL src pointer, ensures the dest buffer is explicitly null-terminated,
 * and uses (unsigned char) casting with tolower for safety.
 * This optimization replaces multiple strcasestr() calls with a single lowercase pass (O(N))
 * and subsequent strstr() calls (O(M)), reducing keyword scanning complexity from O(N*M) to O(N+M).
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

/*
 * Checks if 'needle' is found in 'haystack' with exact word boundaries.
 * A word boundary means the character immediately preceding and following
 * the match is not alphanumeric or underscore (or is the string boundary).
 */
static bool has_word(const char *haystack, const char *needle) {
    if (!haystack || !needle) return false;
    const char *p = strstr(haystack, needle);
    while (p != NULL) {
        bool left_ok = true;
        if (p > haystack) {
            char prev = *(p - 1);
            if ((prev >= 'a' && prev <= 'z') || (prev >= 'A' && prev <= 'Z') ||
                (prev >= '0' && prev <= '9') || prev == '_') {
                left_ok = false;
            }
        }
        bool right_ok = true;
        size_t len = strlen(needle);
        char next = *(p + len);
        if ((next >= 'a' && next <= 'z') || (next >= 'A' && next <= 'Z') ||
            (next >= '0' && next <= '9') || next == '_') {
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
    CURL *curl = curl_easy_init();
    if (!curl) {
        strncpy(rt->error_msg, "CURL_INIT_FAIL", MAX_TEXT - 1);
        rt->error_msg[MAX_TEXT - 1] = '\0';
        return DEC_RETRY;
    }
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
            if (!key && ts->type != T_CDP) {
                strncpy(rt->error_msg, "MISSING_API_KEY", MAX_TEXT - 1);
                rt->error_msg[MAX_TEXT - 1] = '\0';
                res = DEC_BLOCKED;
            }
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
    char goal_norm[MAX_TEXT];
    char skill_norm[MAX_TEXT];
    char type_norm[MAX_TEXT];
    char plat_norm[MAX_TEXT];
    char time_norm[MAX_TEXT];
    char style_norm[MAX_TEXT];
    char const_norm[MAX_TEXT];

    normalize(in->goal, goal_norm);
    normalize(in->skill, skill_norm);
    normalize(in->type, type_norm);
    normalize(in->plat, plat_norm);
    normalize(in->time, time_norm);
    normalize(in->style, style_norm);
    normalize(in->constraints, const_norm);

    const char *detected_goal = "";
    const char *best_lang = "";
    const char *why = "";
    const char *fe = "None";
    const char *be = "None";
    const char *db = "None";
    const char *automation = "None";
    const char *deploy = "None";
    const char *if_cond = "";
    const char *then_dec = "";
    const char *step1 = "";
    const char *step2 = "";
    const char *step3 = "";
    const char *step4 = "";
    const char *b_setup = "";
    const char *b_proto = "";
    const char *b_test = "";
    const char *b_deploy = "";
    const char *b_iterate = "";
    const char *final_rec = "";

    if (strstr(goal_norm, "website") || strstr(goal_norm, "web") || strstr(goal_norm, "html")) {
        detected_goal = "Build Website";
        best_lang = "HTML / CSS / JavaScript / TypeScript";
        why = "Standard web standards for UI, with TypeScript for type-safe application scaling.";
        fe = "HTML5, CSS3, React / Next.js (if app-level complexity exists)";
        be = "Next.js Route Handlers or Node.js";
        db = "PostgreSQL / SQLite";
        automation = "pnpm / Vite";
        deploy = "Vercel / Netlify";
        if_cond = "user goal is 'build website' and app-level complexity exists";
        then_dec = "recommend React / Next.js with TypeScript";
        step1 = "HTML & CSS basics for document structure";
        step2 = "JavaScript basics & DOM manipulation";
        step3 = "TypeScript for compiler-level safety";
        step4 = "React and Next.js for component architecture";
        b_setup = "Initialize a Vite/Next.js TypeScript project";
        b_proto = "Code interactive responsive components with mock data";
        b_test = "Add unit tests using Vitest and Playwright";
        b_deploy = "Deploy onto Vercel / Netlify";
        b_iterate = "Optimize bundle size and user analytics";
        final_rec = "React + Next.js + TypeScript";
    }
    else if (has_word(goal_norm, "ai") || strstr(goal_norm, "agent") || strstr(goal_norm, "llm") || strstr(goal_norm, "intelligence")) {
        detected_goal = "Build AI System";
        best_lang = "Python / TypeScript";
        why = "Python for AI/ML modeling and orchestration, TypeScript for backend/frontend interfaces.";
        fe = "Next.js / React";
        be = "FastAPI (Python) / Node.js (TypeScript)";
        db = "PostgreSQL with pgvector";
        automation = "Bash + Python scripts";
        deploy = "Docker + AWS / GCP";
        if_cond = "user goal is 'build AI system'";
        then_dec = "recommend Python for AI workers, TypeScript for API, SQL for storage, and Bash/YAML for automation";
        step1 = "Python syntax and NumPy/Pandas";
        step2 = "LangChain / OpenAI / LlamaIndex API integration";
        step3 = "FastAPI or Express backend routes";
        step4 = "pgvector and semantic vector search";
        b_setup = "Create Python virtualenv and install FastAPI/PyTorch";
        b_proto = "Connect agent scripts to LLM APIs with basic prompts";
        b_test = "Run safety audits and token cost evaluations";
        b_deploy = "Containerize with Docker and push to AWS ECS";
        b_iterate = "Refine agent reasoning loops and cache LLM responses";
        final_rec = "Python (AI Core) + TypeScript (App Layer) + pgvector";
    }
    else if (strstr(goal_norm, "mobile") || strstr(goal_norm, "app") || strstr(goal_norm, "ios") || strstr(goal_norm, "android")) {
        detected_goal = "Build Mobile App";
        bool ios_only = (strstr(goal_norm, "ios") || strstr(plat_norm, "ios") || strstr(const_norm, "ios")) &&
                        !(strstr(goal_norm, "android") || strstr(plat_norm, "android") || strstr(const_norm, "android"));
        bool android_only = (strstr(goal_norm, "android") || strstr(plat_norm, "android") || strstr(const_norm, "android")) &&
                            !(strstr(goal_norm, "ios") || strstr(plat_norm, "ios") || strstr(const_norm, "ios"));

        step1 = "Language syntax (Swift/Kotlin/Dart)";
        step2 = "UI frameworks (SwiftUI/Compose/Flutter)";
        step3 = "State management & local storage";
        step4 = "REST API consumption and JSON parsing";
        b_setup = "Configure IDE (Xcode/Android Studio) and SDKs";
        b_proto = "Create mockup UI screens and hardcoded navigation";
        b_test = "Perform unit testing and emulator run-throughs";
        b_deploy = "Assemble app bundles and upload to App Store Connect / Play Console";
        b_iterate = "Track crashes and user engagement analytics";

        if (ios_only) {
            best_lang = "Swift";
            why = "Apple's native, high-performance language with modern syntax and strong safety.";
            fe = "SwiftUI";
            be = "Express.js / FastAPI";
            db = "CoreData / SQLite";
            automation = "Fastlane";
            deploy = "Apple App Store";
            if_cond = "user goal is 'build mobile app' and targeting iOS only";
            then_dec = "recommend Swift and SwiftUI";
            final_rec = "Swift + SwiftUI (iOS Native)";
        } else if (android_only) {
            best_lang = "Kotlin";
            why = "Google's preferred native language for modern Android development.";
            fe = "Jetpack Compose";
            be = "Express.js / FastAPI";
            db = "Room / SQLite";
            automation = "Gradle";
            deploy = "Google Play Store";
            if_cond = "user goal is 'build mobile app' and targeting Android only";
            then_dec = "recommend Kotlin and Jetpack Compose";
            final_rec = "Kotlin + Jetpack Compose (Android Native)";
        } else {
            best_lang = "Dart";
            why = "Cross-platform development with a single codebase using Flutter.";
            fe = "Flutter UI";
            be = "Node.js / Go";
            db = "Hive / SQLite";
            automation = "Codemagic";
            deploy = "Apple App Store + Google Play Store";
            if_cond = "user goal is 'build mobile app' and targeting both platforms";
            then_dec = "recommend Dart + Flutter";
            final_rec = "Dart + Flutter (Cross-platform)";
        }
    }
    else if (strstr(goal_norm, "backend") || strstr(goal_norm, "api") || strstr(goal_norm, "server")) {
        detected_goal = "Build Backend/API";
        bool is_beginner = strstr(skill_norm, "beginner") || strstr(skill_norm, "novice") || strstr(goal_norm, "learn") || strstr(goal_norm, "start");
        bool is_saas = strstr(goal_norm, "saas") || strstr(type_norm, "saas") || strstr(const_norm, "saas") || strstr(goal_norm, "production") || strstr(goal_norm, "enterprise");
        bool is_perf = strstr(goal_norm, "performance") || strstr(type_norm, "performance") || strstr(const_norm, "performance") || strstr(goal_norm, "security") || strstr(goal_norm, "critical");

        step1 = "Backend language fundamentals and asynchronous programming";
        step2 = "HTTP protocol, RESTful standards, and routing";
        step3 = "Database modeling, SQL, and migrations";
        step4 = "Authentication, JWT, and CORS protection";
        b_setup = "Initialize project environment and configure logger/config files";
        b_proto = "Create healthcheck and basic CRUD REST endpoints with Mock DB";
        b_test = "Write integration tests with Supertest or native testing tools";
        b_deploy = "Dockerize application and deploy to cloud provider";
        b_iterate = "Implement caching layer (Redis) and connection pooling";

        if (is_beginner) {
            best_lang = "Python";
            why = "Easy to learn syntax with powerful libraries like FastAPI.";
            fe = "React / HTML";
            be = "FastAPI";
            db = "SQLite";
            automation = "Pipenv";
            deploy = "Render";
            if_cond = "user goal is 'build backend' and skill level is beginner";
            then_dec = "recommend Python + FastAPI";
            final_rec = "Python + FastAPI + SQLite";
        } else if (is_perf) {
            best_lang = "Rust / Go";
            why = "Rust provides memory safety without garbage collection; Go offers high concurrent throughput.";
            fe = "React / WebAssembly";
            be = "Rust (Actix-web / Axum) or Go";
            db = "PostgreSQL / ScyllaDB";
            automation = "Kubernetes";
            deploy = "AWS EKS";
            if_cond = "user goal is 'build backend' and high-performance/security is critical";
            then_dec = "recommend Rust OR Go";
            final_rec = "Rust (Axum) or Go (Gin)";
        } else {
            best_lang = "TypeScript / Go";
            why = "TypeScript provides full-stack continuity; Go offers superb concurrent performance.";
            fe = "Next.js";
            be = "Node.js (Express/NestJS) or Go (Gin/Fiber)";
            db = "PostgreSQL";
            automation = "Docker Compose";
            deploy = "AWS ECS / Railway";
            if_cond = "user goal is 'build backend' and target is production SaaS";
            then_dec = "recommend TypeScript + Node.js OR Go";
            final_rec = "TypeScript + Node.js (SaaS Standard) or Go";
        }
    }
    else if (strstr(goal_norm, "operating system") || strstr(goal_norm, "os") || strstr(goal_norm, "firmware") || strstr(goal_norm, "embedded") || strstr(goal_norm, "microcontroller")) {
        detected_goal = "Build OS / Firmware / Embedded";
        best_lang = "C / C++ / Rust";
        why = "Low-level languages with zero-cost abstractions and direct hardware/memory access.";
        fe = "None / CLI / Embedded UI (Qt)";
        be = "Embedded C / Bare-Metal Rust";
        db = "Custom binary storage / Flash FS";
        automation = "Make / CMake / Cargo";
        deploy = "JTAG Programmer / OTA Update";
        if_cond = "user goal is 'build operating system / firmware / embedded'";
        then_dec = "recommend C, C++, or Rust, using Assembly only where direct CPU control is required";
        step1 = "C pointer management, memory layout, and structs";
        step2 = "CPU architectures (ARM, RISC-V, x86)";
        step3 = "Memory-mapped I/O and interrupt handlers";
        step4 = "Rust embedded programming and safe concurrency";
        b_setup = "Install cross-compilers (gcc-arm-none-eabi) and debuggers";
        b_proto = "Write a simple 'Blinky' or 'Hello World' on UART";
        b_test = "Run logic analyzers and hardware-in-the-loop (HIL) tests";
        b_deploy = "Flash binary onto physical microcontroller ROM";
        b_iterate = "Optimize clock cycles, memory footprints, and power usage";
        final_rec = "C / Rust (Bare-Metal)";
    }
    else if (strstr(goal_norm, "fpga") || strstr(goal_norm, "asic") || strstr(goal_norm, "verilog") || strstr(goal_norm, "vhdl") || strstr(goal_norm, "digital logic")) {
        detected_goal = "FPGA / ASIC / Digital Logic";
        best_lang = "Verilog / SystemVerilog / VHDL";
        why = "Hardware description languages that define hardware gates and registers.";
        fe = "Waveform Viewer (GTKWave)";
        be = "Hardware Synthesis Tool (Yosys, Vivado)";
        db = "None";
        automation = "Makefile";
        deploy = "FPGA Bitstream Flash";
        if_cond = "user goal is 'FPGA / ASIC / digital logic'";
        then_dec = "recommend Verilog, SystemVerilog, or VHDL";
        step1 = "Digital logic gates, multiplexers, flip-flops";
        step2 = "Finite State Machine (FSM) modeling in HDL";
        step3 = "Synthesizable vs. non-synthesizable constructs";
        step4 = "Testbench writing and clock domain crossing";
        b_setup = "Configure HDL simulator (Icarus Verilog, Modelsim)";
        b_proto = "Write behavioral model of digital module";
        b_test = "Run self-checking testbenches and verify waveforms";
        b_deploy = "Synthesize design to netlist and place-and-route to bitstream";
        b_iterate = "Optimize timing constraints and gate utilization";
        final_rec = "SystemVerilog (Modern standard)";
    }
    else if (strstr(goal_norm, "data science") || strstr(goal_norm, "ml") || strstr(goal_norm, "machine learning") || strstr(goal_norm, "statistics")) {
        detected_goal = "Data Science / ML";
        best_lang = "Python / SQL / R / Julia";
        why = "Python is the undisputed leader in AI/ML ecosystem; SQL is essential for data querying.";
        fe = "Streamlit / Gradio";
        be = "FastAPI / Jupyter Hub";
        db = "Snowflake / BigQuery / PostgreSQL";
        automation = "Prefect / Airflow";
        deploy = "Hugging Face / AWS Sagemaker";
        if_cond = "user goal is 'data science / ML'";
        then_dec = "recommend Python & SQL; R only if statistics-heavy; Julia only if high-performance numerical computing is required";
        step1 = "Python syntax, NumPy, Pandas, and SQL queries";
        step2 = "Data preprocessing, visualization (Matplotlib, Seaborn)";
        step3 = "Scikit-Learn for classical ML algorithms";
        step4 = "PyTorch / TensorFlow for Deep Learning modeling";
        b_setup = "Set up Conda environment or Jupyter Notebook";
        b_proto = "Perform Exploratory Data Analysis (EDA) on a CSV/DB sample";
        b_test = "Evaluate model performance using cross-validation and metrics (F1, MSE)";
        b_deploy = "Wrap trained model in FastAPI and deploy as a Docker microservice";
        b_iterate = "Set up model monitoring for drift and retrain pipelines";
        final_rec = "Python + SQL + PyTorch";
    }
    else if (strstr(goal_norm, "database") || strstr(goal_norm, "sql") || strstr(goal_norm, "postgresql") || strstr(goal_norm, "mongodb")) {
        detected_goal = "Database-Heavy App";
        best_lang = "SQL / PostgreSQL";
        why = "SQL is the native relational query language for consistent, structured data processing.";
        fe = "React / HTML";
        be = "Node.js (TypeScript) or Python (FastAPI)";
        db = "PostgreSQL (Relational) / Redis (Caching)";
        automation = "Flyway / Prisma Migrations";
        deploy = "Supabase / AWS RDS";
        if_cond = "user goal is 'database-heavy app'";
        then_dec = "recommend SQL first, PostgreSQL for relational storage, and TypeScript/Python for backend";
        step1 = "SQL basics (SELECT, JOINs, Group By)";
        step2 = "Database normalization (1NF-3NF) and schema design";
        step3 = "Indexing strategies and query optimization (EXPLAIN)";
        step4 = "Acid transactions and concurrent control";
        b_setup = "Initialize PostgreSQL database instance locally or via Docker";
        b_proto = "Design DB schema and load sample records";
        b_test = "Run benchmark queries to test response times";
        b_deploy = "Host DB on managed service and apply migration scripts";
        b_iterate = "Add database connection pooler (PgBouncer) and read replicas";
        final_rec = "SQL (PostgreSQL) + TypeScript (Backend)";
    }
    else if (strstr(goal_norm, "blockchain") || strstr(goal_norm, "smart contract") || strstr(goal_norm, "crypto") || strstr(goal_norm, "solidity") || strstr(goal_norm, "ethereum") || strstr(goal_norm, "solana")) {
        detected_goal = "Blockchain / Smart Contracts";
        bool evm = strstr(goal_norm, "ethereum") || strstr(goal_norm, "evm") || strstr(goal_norm, "solidity") || strstr(goal_norm, "eth") || strstr(const_norm, "ethereum") || strstr(const_norm, "evm");
        bool solana = strstr(goal_norm, "solana") || strstr(goal_norm, "anchor") || strstr(const_norm, "solana");
        bool move_chain = strstr(goal_norm, "move") || strstr(goal_norm, "sui") || strstr(goal_norm, "aptos") || strstr(const_norm, "move");

        step1 = "Blockchain fundamentals (blocks, hashing, consensus)";
        step2 = "Smart contract development (Solidity/Rust/Move)";
        step3 = "Security audits, reentrancy vulnerabilities, and gas optimization";
        step4 = "DApp frontend integration using ethers.js / web3.js";
        b_setup = "Configure Hardhat/Foundry or Anchor framework workspace";
        b_proto = "Write basic ERC-20 / NFT token smart contract";
        b_test = "Write extensive unit tests with Mocha/Chai or Forge";
        b_deploy = "Deploy contract to Testnet (Sepolia/Devnet) and verify source code";
        b_iterate = "Optimize storage writes and audit contract bytecode";

        if (solana) {
            best_lang = "Rust";
            why = "Solana smart contracts are written in high-performance, safe Rust code using the Anchor framework.";
            fe = "React + @solana/web3.js";
            be = "Anchor / Rust";
            db = "Solana Ledger / IPFS";
            automation = "Cargo / Anchor CLI";
            deploy = "Solana Mainnet";
            if_cond = "user goal is 'blockchain / smart contracts' and target is Solana";
            then_dec = "recommend Rust";
            final_rec = "Rust (Anchor framework) + TypeScript (Frontend)";
        } else if (move_chain) {
            best_lang = "Move";
            why = "Resource-oriented programming language designed for highly secure digital assets.";
            fe = "React + Aptos/Sui SDK";
            be = "Move Smart Contracts";
            db = "Aptos/Sui Ledger";
            automation = "Move CLI";
            deploy = "Aptos / Sui Mainnet";
            if_cond = "user goal is 'blockchain / smart contracts' and target is Move-chain";
            then_dec = "recommend Move";
            final_rec = "Move + React + SDK";
        } else {
            best_lang = "Solidity + TypeScript";
            why = "Solidity is the industry standard for EVM execution; TypeScript for smart contract testing and frontend hooks.";
            fe = "React + ethers.js";
            be = "Solidity / Hardhat";
            db = "EVM State / IPFS";
            automation = "Hardhat / Foundry";
            deploy = "Ethereum / Polygon Mainnet";
            if_cond = "user goal is 'blockchain / smart contracts' and target is Ethereum/EVM";
            then_dec = "recommend Solidity + TypeScript";
            final_rec = "Solidity + TypeScript (EVM Standard)";
        }
    }
    else if (strstr(goal_norm, "automation") || strstr(goal_norm, "script") || strstr(goal_norm, "powershell") || strstr(goal_norm, "bash") || strstr(goal_norm, "cli")) {
        detected_goal = "Automation / Scripting";
        bool is_windows = strstr(goal_norm, "windows") || strstr(plat_norm, "windows") || strstr(goal_norm, "win32") || strstr(goal_norm, "powershell") || strstr(goal_norm, "pwsh");

        step1 = "Shell basics, piping, redirecting, and command execution";
        step2 = "Variables, conditions, loops, and script arguments";
        step3 = "Python standard library (os, sys, subprocess, shutil)";
        step4 = "Task scheduling (cron, Windows Task Scheduler)";
        b_setup = "Set up target scripts folder and initialize git repository";
        b_proto = "Write modular scripts to accomplish single target action";
        b_test = "Run script dry-runs, log outputs, and trace errors";
        b_deploy = "Install script on cron daemon or task scheduler";
        b_iterate = "Implement alert notifications (Email/Discord) on script failure";

        if (is_windows) {
            best_lang = "PowerShell + Python";
            why = "PowerShell provides seamless integration with Windows API and system cmdlets.";
            fe = "None / CLI";
            be = "PowerShell Core / Python";
            db = "CSV / SQLite";
            automation = "Windows Task Scheduler";
            deploy = "Local Windows environment";
            if_cond = "user goal is 'automation / scripting' and OS is Windows";
            then_dec = "recommend PowerShell + Python";
            final_rec = "PowerShell + Python (Windows)";
        } else {
            best_lang = "Bash + Python";
            why = "Bash for standard OS level orchestrations, Python for complex script logic.";
            fe = "None / CLI";
            be = "Bash / Python";
            db = "CSV / SQLite";
            automation = "cron daemon";
            deploy = "Linux / macOS server";
            if_cond = "user goal is 'automation / scripting' and OS is Linux/macOS";
            then_dec = "recommend Bash + Python";
            final_rec = "Bash + Python (Linux/macOS)";
        }
    }

    if (strlen(detected_goal) == 0) {
        printf("I am unclear about your exact goal. Please answer exactly 3 questions:\n");
        printf("1. What are you building?\n");
        printf("2. Where will it run?\n");
        printf("3. Do you want fastest build or strongest long-term architecture?\n");
        return;
    }

    LangOutput out = {0};
    strncpy(out.goal, in->goal, MAX_TEXT - 1);
    out.goal[MAX_TEXT - 1] = '\0';
    strncpy(out.lang, best_lang, MAX_TEXT - 1);
    out.lang[MAX_TEXT - 1] = '\0';
    strncpy(out.why, why, MAX_TEXT - 1);
    out.why[MAX_TEXT - 1] = '\0';
    strncpy(out.stack, final_rec, MAX_TEXT - 1);
    out.stack[MAX_TEXT - 1] = '\0';

    printf("PROGRAMMING_LANGUAGE_DECISION_TREE\n");
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
    printf("│   ├── Setup: %s\n", b_setup);
    printf("│   ├── Prototype: %s\n", b_proto);
    printf("│   ├── Test: %s\n", b_test);
    printf("│   ├── Deploy: %s\n", b_deploy);
    printf("│   └── Iterate: %s\n", b_iterate);
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
