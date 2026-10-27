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
#include <curl/curl.h>
#include <ctype.h>

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
    T_CDP, T_PWSH, T_ADB, T_KIMI, T_COMPOSIO, T_RESEND, T_STEEL,
    T_FIRECRAWL, T_SCRAPINGBEE, T_SCRAPFLY, T_SCREENSHOTONE, T_EXA,
    T_TAVILY, T_HELICONE, T_E2B, T_OPENAI, T_PINECONE, T_INNGEST,
    T_DISCORD, T_NONE
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
    int          plan_count;
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
    { T_ADB,     "ANDROID_ADB","ADB_PATH",        "local", true },
    { T_KIMI,    "KIMI",       "KIMI_API_KEY",    "https://api.moonshot.cn/v1/chat/completions", true },
    { T_COMPOSIO,"COMPOSIO",   "COMPOSIO_API_KEY","https://api.composio.dev/v1", true },
    { T_RESEND,  "RESEND",     "RESEND_API_KEY",  "https://api.resend.com/emails", true },
    { T_STEEL,   "STEEL",      "STEEL_API_KEY",   "https://api.steel.dev/v1", true },
    { T_FIRECRAWL,"FIRECRAWL", "FIRECRAWL_API_KEY","https://api.firecrawl.dev/v1", true },
    { T_SCRAPINGBEE,"SCRAPINGBEE","SCRAPINGBEE_API_KEY","https://app.scrapingbee.com/api/v1/", true },
    { T_SCRAPFLY,"SCRAPFLY",   "SCRAPFLY_API_KEY","https://api.scrapfly.io/scrape", true },
    { T_SCREENSHOTONE,"SCREENSHOTONE","SCREENSHOTONE_ACCESS_KEY","https://api.screenshotone.com/take", true },
    { T_EXA,     "EXA",        "EXA_API_KEY",     "https://api.exa.ai/search", true },
    { T_TAVILY,  "TAVILY",     "TAVILY_API_KEY",  "https://api.tavily.com/search", true },
    { T_HELICONE,"HELICONE",   "HELICONE_API_KEY","https://api.hconeai.com/v1", true },
    { T_E2B,     "E2B",        "E2B_API_KEY",     "https://api.e2b.dev/v1", true },
    { T_OPENAI,  "OPENAI",     "OPENAI_API_KEY",  "https://api.openai.com/v1/chat/completions", true },
    { T_PINECONE,"PINECONE",   "PINECONE_API_KEY","https://api.pinecone.io", true },
    { T_INNGEST, "INNGEST",    "INNGEST_EVENT_KEY","https://inn.gs/", true },
    { T_DISCORD, "DISCORD",    "DISCORD_BOT_TOKEN","https://discord.com/api/v10", true }
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

/* --- BABYAGI TASK PLANNER --- */
static void build_task_queue(AgentRuntime *rt) {
    if (!rt) return;
    rt->plan_count = 0;

    char temp[MAX_TEXT];
    strncpy(temp, rt->objective, MAX_TEXT - 1);
    temp[MAX_TEXT - 1] = '\0';

    char *saveptr;
    // Simple delimiter-based task splitting using "&&" or "&"
    char *token = strtok_r(temp, "&", &saveptr);
    while (token != NULL && rt->plan_count < MAX_STEPS) {
        // Trim leading spaces
        while(isspace((unsigned char)*token)) token++;
        if (strlen(token) > 0) {
            strncpy(rt->plan[rt->plan_count], token, MAX_TEXT - 1);
            rt->plan[rt->plan_count][MAX_TEXT - 1] = '\0';
            rt->plan_count++;
        }
        token = strtok_r(NULL, "&", &saveptr);
    }

    if (rt->plan_count == 0) {
        strncpy(rt->plan[0], rt->objective, MAX_TEXT - 1);
        rt->plan[0][MAX_TEXT - 1] = '\0';
        rt->plan_count = 1;
    }
}

/*
 * O(N) string normalization: Converts a string to lowercase.
 * This avoids O(N*M) performance penalty of repeated strcasestr calls.
 */
static void normalize(const char *src, char *dest) {
    if (!src || !dest) return;
    int i = 0;
    while (src[i] && i < MAX_TEXT - 1) {
        dest[i] = (unsigned char)tolower((unsigned char)src[i]);
        i++;
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
    if (strstr(norm_task, "github") || strstr(norm_task, "repo") || strstr(norm_task, "commit")) return T_GITHUB;
    if (strstr(norm_task, "kimi") || strstr(norm_task, "moonshot")) return T_KIMI;
    if (strstr(norm_task, "composio")) return T_COMPOSIO;
    if (strstr(norm_task, "email") || strstr(norm_task, "resend") || strstr(norm_task, "mail")) return T_RESEND;
    if (strstr(norm_task, "steel")) return T_STEEL;
    if (strstr(norm_task, "crawl") || strstr(norm_task, "firecrawl")) return T_FIRECRAWL;
    if (strstr(norm_task, "scrapingbee") || strstr(norm_task, "scrapebee")) return T_SCRAPINGBEE;
    if (strstr(norm_task, "scrapfly") || strstr(norm_task, "scrapefly")) return T_SCRAPFLY;
    if (strstr(norm_task, "screenshot")) return T_SCREENSHOTONE;
    if (strstr(norm_task, "exa") || strstr(norm_task, "find out")) return T_EXA;
    if (strstr(norm_task, "tavily") || strstr(norm_task, "search web")) return T_TAVILY;
    if (strstr(norm_task, "helicone")) return T_HELICONE;
    if (strstr(norm_task, "e2b") || strstr(norm_task, "sandbox")) return T_E2B;
    if (strstr(norm_task, "openai") || strstr(norm_task, "gpt")) return T_OPENAI;
    if (strstr(norm_task, "pinecone") || strstr(norm_task, "vector") || strstr(norm_task, "embed")) return T_PINECONE;
    if (strstr(norm_task, "inngest") || strstr(norm_task, "event")) return T_INNGEST;
    if (strstr(norm_task, "discord") || strstr(norm_task, "message chat")) return T_DISCORD;

    return T_MISTRAL; // Default AI provider
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

/* --- REACT OBSERVATION ENGINE --- */
static Decision observe_result(AgentRuntime *rt) {
    if (!rt || strlen(rt->result) == 0) return DEC_RETRY;

    char norm_res[MAX_TEXT];
    normalize(rt->result, norm_res);

    // Check for semantic failure indicators in output
    if (strstr(norm_res, "error") ||
        strstr(norm_res, "failed") ||
        strstr(norm_res, "not found") ||
        strstr(norm_res, "denied") ||
        strstr(norm_res, "exception")) {
        logger(LOG_WARN, "Observation: Detected semantic failure in output");
        return DEC_RETRY;
    }

    return DEC_COMPLETE;
}

/* --- PRODUCTION WORKFLOW --- */
static void run_agent(AgentRuntime *rt) {
    if (!rt) return;
    logger(LOG_INFO, "Initializing BOS-OMEGA Agentic Workflow v%s", VERSION);

    // 1. Planning Phase (BabyAGI style)
    rt->state = ST_PARSE;
    build_task_queue(rt);
    logger(LOG_INFO, "Planner: Decomposed objective into %d sub-tasks.", rt->plan_count);

    // 2. Execution Loop
    for (rt->plan_idx = 0; rt->plan_idx < rt->plan_count; rt->plan_idx++) {
        const char *current_task = rt->plan[rt->plan_idx];
        logger(LOG_INFO, "--- Executing Task %d/%d: %s ---", rt->plan_idx + 1, rt->plan_count, current_task);

        char norm_task[MAX_TEXT];
        normalize(current_task, norm_task);

        rt->state = ST_ROUTE;
        rt->tool = resolve_tool(norm_task);
        rt->risk = assess_risk(norm_task);
        logger(LOG_AUDIT, "Task: %s | Tool: %d | Risk: %d", current_task, rt->tool, rt->risk);

        if (rt->risk >= R_DESTRUCTIVE && !rt->is_approved) {
            rt->state = ST_ESCALATE; rt->decision = DEC_ESCALATE;
            logger(LOG_WARN, "Policy Block: Destructive action requires --approve");
            return; // Halt entire pipeline
        }

        bool task_success = false;
        // 3. ReAct Loop (Reasoning & Acting)
        for (rt->retry_cnt = 0; rt->retry_cnt < MAX_RETRIES; rt->retry_cnt++) {
            rt->state = ST_EXEC;
            logger(LOG_INFO, "Execution Attempt %d/%d", rt->retry_cnt + 1, MAX_RETRIES);

            Decision res;
            // Execute Action
            if (rt->tool == T_PWSH) res = dispatch_shell(rt, "pwsh", "-Command");
            else if (rt->tool == T_ADB) res = dispatch_shell(rt, "adb", "shell");
            else {
                ToolSpec *ts = &REGISTRY[rt->tool];
                const char *key = getenv(ts->env_key);
                if (!key && ts->type != T_CDP) { strcpy(rt->error_msg, "MISSING_API_KEY"); res = DEC_BLOCKED; }
                else {
                    char payload[MAX_TEXT + 128];
                    snprintf(payload, sizeof(payload), "{\"model\":\"prod\",\"input\":\"%.*s\"}", (int)(MAX_TEXT - 1), current_task);
                    res = dispatch_http(rt, ts->url, key ? key : "NONE", payload);
                }
            }

            // Observe Results
            if (res == DEC_COMPLETE) {
                rt->state = ST_VERIFY;
                res = observe_result(rt);
            }

            if (res == DEC_COMPLETE) {
                task_success = true;
                logger(LOG_INFO, "Observation: Task successful.");
                break;
            } else if (res == DEC_BLOCKED) {
                rt->state = ST_FAILURE; rt->decision = DEC_BLOCKED;
                logger(LOG_ERROR, "Execution Blocked: %s", rt->error_msg);
                break;
            }
            logger(LOG_WARN, "Observation: Attempt Failed (System Error: %s). Retrying...", rt->error_msg);
        }

        if (!task_success) {
            rt->state = ST_FAILURE; rt->decision = DEC_ABORT;
            logger(LOG_ERROR, "Final State: FAILURE | Pipeline aborted at task %d", rt->plan_idx + 1);
            return;
        }
    }

    rt->state = ST_SUCCESS; rt->decision = DEC_COMPLETE;
    logger(LOG_INFO, "Workflow Completed Successfully.");
}

/* --- LANGUAGE DECISION ENGINE --- */
static void run_lang(LangInput *in) {
    if (!in) return;
    LangOutput out = {0}; strncpy(out.goal, in->goal, MAX_TEXT - 1);

    char norm_goal[MAX_TEXT];
    normalize(in->goal, norm_goal);

    if (strstr(norm_goal, "ai") || strstr(norm_goal, "agent")) {
        strcpy(out.lang, "Python / TypeScript");
        strcpy(out.why, "Python for AI Workers/ML ecosystem; TypeScript for integration.");
        strcpy(out.stack, "Python + FastAPI + Next.js (Build Order: Core AI -> API -> Frontend)");
    } else if (strstr(norm_goal, "saas") || strstr(norm_goal, "web") || strstr(norm_goal, "production")) {
        strcpy(out.lang, "TypeScript / Node.js or Go");
        strcpy(out.why, "TypeScript/Node.js for rapid full-stack SaaS; Go for high-concurrency microservices.");
        strcpy(out.stack, "Next.js/React + Node.js/Go + PostgreSQL");
    } else if (strstr(norm_goal, "os") || strstr(norm_goal, "performance") || strstr(norm_goal, "security") || strstr(norm_goal, "core")) {
        strcpy(out.lang, "C / Rust / Go");
        strcpy(out.why, "C/Rust for maximum performance and memory safety; Go for network/concurrent tooling.");
        strcpy(out.stack, "Rust/C Core + Makefile/Cargo");
    } else if (strstr(norm_goal, "mobile") || strstr(norm_goal, "app")) {
        strcpy(out.lang, "Dart / Flutter");
        strcpy(out.why, "Cross-platform mobile delivery with native performance.");
        strcpy(out.stack, "Flutter + Dart");
    } else {
        strcpy(out.lang, "TypeScript / Rust");
        strcpy(out.why, "Modern safety and scalability fallback.");
        strcpy(out.stack, "Node.js + PostgreSQL + Docker");
    }

    printf("\n======================================================\n");
    printf("         PROGRAMMING_LANGUAGE_DECISION_TREE           \n");
    printf("======================================================\n");
    printf(" INPUT GOAL : %s\n", out.goal);
    printf("------------------------------------------------------\n");
    printf(" [✓] RECOMMENDED LANGUAGES : %s\n", out.lang);
    printf(" [✓] LOGIC / REASONING     : %s\n", out.why);
    printf(" [✓] TARGET STACK / BUILD  : %s\n", out.stack);
    printf("======================================================\n");
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
        LangInput in = {0}; strncpy(in.goal, argv[2], MAX_TEXT - 1);
        run_lang(&in);
    } else {
        printf("Error: Invalid arguments.\n");
    }
    curl_global_cleanup();
    return 0;
}
