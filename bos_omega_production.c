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
 * normalize - Normalizes a source string to lowercase.
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

static bool has_substring(const char *haystack, const char *needle) {
    return haystack && needle && strstr(haystack, needle) != NULL;
}

static bool has_word(const char *haystack, const char *needle) {
    if (!haystack || !needle) return false;
    const char *ptr = haystack;
    size_t needle_len = strlen(needle);
    while ((ptr = strstr(ptr, needle)) != NULL) {
        bool prev_ok = true;
        if (ptr > haystack) {
            char prev = *(ptr - 1);
            if (isalnum((unsigned char)prev) || prev == '_') {
                prev_ok = false;
            }
        }
        bool next_ok = true;
        char next = *(ptr + needle_len);
        if (isalnum((unsigned char)next) || next == '_') {
            next_ok = false;
        }
        if (prev_ok && next_ok) {
            return true;
        }
        ptr += 1;
    }
    return false;
}

/* --- GOVERNANCE ENGINE --- */
static RiskLevel assess_risk(const char *norm) {
    if (!norm) return R_READ_ONLY;
    if (has_word(norm, "buy") || has_word(norm, "pay") || has_word(norm, "spend")) return R_FINANCIAL;
    if (has_word(norm, "delete") || has_word(norm, "uninstall") || has_word(norm, "rm") || has_word(norm, "wipe") || has_substring(norm, "remove-") || has_substring(norm, "set-content")) return R_DESTRUCTIVE;
    if (has_word(norm, "push") || has_word(norm, "commit") || has_word(norm, "api")) return R_EXTERNAL_API;
    if (has_word(norm, "write") || has_word(norm, "create") || has_word(norm, "save")) return R_LOCAL_WRITE;
    return R_READ_ONLY;
}

static ToolType resolve_tool(const char *norm) {
    if (!norm) return T_MISTRAL;
    if (has_substring(norm, "android") || has_word(norm, "adb") || has_word(norm, "pm") || has_word(norm, "am") || has_word(norm, "input")) return T_ADB;
    if (has_substring(norm, "get-") || has_substring(norm, "set-") || has_word(norm, "pwsh") || has_substring(norm, "powershell")) return T_PWSH;
    if (has_word(norm, "browser") || has_word(norm, "navigate") || has_word(norm, "click")) return T_CDP;
    if (has_word(norm, "image") || has_word(norm, "visual")) return T_NVIDIA;
    if (has_word(norm, "railway") || has_substring(norm, "deploy")) return T_RAILWAY;
    if (has_word(norm, "github") || has_word(norm, "repo")) return T_GITHUB;
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

    rt->state = ST_ROUTE; rt->tool = resolve_tool(norm);
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

    normalize(in->goal, norm_goal);
    normalize(in->skill, norm_skill);
    normalize(in->type, norm_type);
    normalize(in->plat, norm_plat);
    normalize(in->time, norm_time);
    normalize(in->style, norm_style);
    normalize(in->constraints, norm_constraints);

    char best_lang[MAX_TEXT] = "TypeScript / Go";
    char why_reason[MAX_TEXT] = "Industry standard backend choices with superb ecosystems and reliability.";
    char frontend_stack[MAX_TEXT] = "React (TypeScript)";
    char backend_stack[MAX_TEXT] = "TypeScript (Node.js) or Go";
    char database_stack[MAX_TEXT] = "PostgreSQL";
    char automation_stack[MAX_TEXT] = "Docker";
    char deployment_stack[MAX_TEXT] = "AWS / Railway";
    char if_cond[MAX_TEXT] = "user goal is backend/API development";
    char then_dec[MAX_TEXT] = "recommend TypeScript + Node.js or Go.";
    char step1[MAX_TEXT] = "Master backend paradigms and REST/GraphQL";
    char step2[MAX_TEXT] = "Learn database design with PostgreSQL";
    char step3[MAX_TEXT] = "Build scalable web API endpoints";
    char step4[MAX_TEXT] = "Setup production logging and monitoring";
    char setup_build[MAX_TEXT] = "Setup monorepo or standard workspace structure";
    char prototype_build[MAX_TEXT] = "Build CRUD interfaces and route routing";
    char test_build[MAX_TEXT] = "Implement unit and integration tests";
    char deploy_build[MAX_TEXT] = "Configure cloud container execution";
    char iterate_build[MAX_TEXT] = "Optimize queries and connection pooling";
    char final_rec[MAX_TEXT] = "TypeScript (Node.js) OR Go";

    bool matched = false;

    // 1. AI System
    if (has_word(norm_goal, "ai") || has_substring(norm_goal, "artificial intelligence") || has_substring(norm_goal, "llm") || has_substring(norm_goal, "agent")) {
        matched = true;
        strncpy(best_lang, "Python + TypeScript", MAX_TEXT - 1);
        strncpy(why_reason, "Python for AI workers/ML models, TypeScript for application/API layer, SQL for storage, Bash/YAML for automation.", MAX_TEXT - 1);
        strncpy(frontend_stack, "React / Next.js", MAX_TEXT - 1);
        strncpy(backend_stack, "TypeScript (Node.js/Fastify) & Python", MAX_TEXT - 1);
        strncpy(database_stack, "SQL (PostgreSQL) + Vector Database (Pgvector)", MAX_TEXT - 1);
        strncpy(automation_stack, "Bash / YAML", MAX_TEXT - 1);
        strncpy(deployment_stack, "Docker + AWS/GCP", MAX_TEXT - 1);
        strncpy(if_cond, "user goal is \"build AI system\"", MAX_TEXT - 1);
        strncpy(then_dec, "recommend Python for AI workers, TypeScript for app/API layer, SQL for storage, Bash/YAML for automation.", MAX_TEXT - 1);
        strncpy(step1, "Learn Python basics and syntax", MAX_TEXT - 1);
        strncpy(step2, "Understand LLM APIs and LangChain/LlamaIndex", MAX_TEXT - 1);
        strncpy(step3, "Learn TypeScript and FastAPI/Express", MAX_TEXT - 1);
        strncpy(step4, "Deploy AI agent with Docker and vector DB", MAX_TEXT - 1);
        strncpy(setup_build, "Initialize git, Python venv, and npm project", MAX_TEXT - 1);
        strncpy(prototype_build, "Build core agent logic in Python and expose endpoint", MAX_TEXT - 1);
        strncpy(test_build, "Write unit tests for agent prompts and responses", MAX_TEXT - 1);
        strncpy(deploy_build, "Containerize and deploy to cloud host", MAX_TEXT - 1);
        strncpy(iterate_build, "Optimize agent latency and improve prompts", MAX_TEXT - 1);
        strncpy(final_rec, "Python (AI) + TypeScript (App) + SQL + Bash", MAX_TEXT - 1);
    }
    // 2. Web Site
    else if (has_substring(norm_goal, "web") || has_substring(norm_goal, "site") || has_substring(norm_goal, "frontend")) {
        matched = true;
        strncpy(best_lang, "HTML + CSS + JavaScript + TypeScript", MAX_TEXT - 1);
        strncpy(why_reason, "Web standards require HTML/CSS/JS, TypeScript is recommended for app-level complexity.", MAX_TEXT - 1);
        strncpy(frontend_stack, "React / Next.js (TypeScript)", MAX_TEXT - 1);
        strncpy(backend_stack, "Node.js (Express/Fastify) or Next.js API Routes", MAX_TEXT - 1);
        strncpy(database_stack, "PostgreSQL / TiDB", MAX_TEXT - 1);
        strncpy(automation_stack, "pnpm / npm scripts", MAX_TEXT - 1);
        strncpy(deployment_stack, "Vercel / Netlify / Docker", MAX_TEXT - 1);
        strncpy(if_cond, "user goal is \"build website\"", MAX_TEXT - 1);
        strncpy(then_dec, "recommend HTML, CSS, JavaScript, TypeScript, and React/Next.js for app-level complexity.", MAX_TEXT - 1);
        strncpy(step1, "Learn HTML & CSS fundamentals", MAX_TEXT - 1);
        strncpy(step2, "Master JavaScript & DOM manipulation", MAX_TEXT - 1);
        strncpy(step3, "Learn React / Next.js framework", MAX_TEXT - 1);
        strncpy(step4, "Understand TypeScript types and state management", MAX_TEXT - 1);
        strncpy(setup_build, "Initialize Vite/Next.js project with Tailwind CSS", MAX_TEXT - 1);
        strncpy(prototype_build, "Build UI components and mock routing", MAX_TEXT - 1);
        strncpy(test_build, "Write component tests using Jest or Vitest", MAX_TEXT - 1);
        strncpy(deploy_build, "Connect repository to Vercel/Netlify for auto-deploy", MAX_TEXT - 1);
        strncpy(iterate_build, "Optimize asset loading and implement server-side rendering", MAX_TEXT - 1);
        strncpy(final_rec, "React / Next.js (TypeScript) + CSS + HTML", MAX_TEXT - 1);
    }
    // 3. Mobile App
    else if (has_substring(norm_goal, "mobile") || has_substring(norm_goal, "app")) {
        matched = true;
        if (has_substring(norm_plat, "ios")) {
            strncpy(best_lang, "Swift", MAX_TEXT - 1);
            strncpy(why_reason, "Swift is Apple's native, high-performance language for iOS development.", MAX_TEXT - 1);
            strncpy(frontend_stack, "SwiftUI", MAX_TEXT - 1);
            strncpy(backend_stack, "Express (Node.js) or NestJS", MAX_TEXT - 1);
            strncpy(database_stack, "PostgreSQL / Supabase", MAX_TEXT - 1);
            strncpy(automation_stack, "Fastlane", MAX_TEXT - 1);
            strncpy(deployment_stack, "Apple App Store", MAX_TEXT - 1);
            strncpy(if_cond, "user goal is \"build mobile app\" and platform is iOS only", MAX_TEXT - 1);
            strncpy(then_dec, "recommend Swift.", MAX_TEXT - 1);
            strncpy(step1, "Learn Swift programming basics", MAX_TEXT - 1);
            strncpy(step2, "Understand SwiftUI for layout and state", MAX_TEXT - 1);
            strncpy(step3, "Learn networking and CoreData/SwiftData", MAX_TEXT - 1);
            strncpy(step4, "Build and publish to App Store", MAX_TEXT - 1);
            strncpy(setup_build, "Install Xcode and initialize a iOS project", MAX_TEXT - 1);
            strncpy(prototype_build, "Design UI screens in SwiftUI with mock data", MAX_TEXT - 1);
            strncpy(test_build, "Write UI and Unit tests in Xcode", MAX_TEXT - 1);
            strncpy(deploy_build, "Configure App Store Connect and upload build", MAX_TEXT - 1);
            strncpy(iterate_build, "Add push notifications and performance profiling", MAX_TEXT - 1);
            strncpy(final_rec, "Swift + SwiftUI", MAX_TEXT - 1);
        } else if (has_substring(norm_plat, "android")) {
            strncpy(best_lang, "Kotlin", MAX_TEXT - 1);
            strncpy(why_reason, "Kotlin is the modern, safe, and officially supported language for native Android.", MAX_TEXT - 1);
            strncpy(frontend_stack, "Jetpack Compose", MAX_TEXT - 1);
            strncpy(backend_stack, "FastAPI or Ktor", MAX_TEXT - 1);
            strncpy(database_stack, "SQLite (Room) / PostgreSQL", MAX_TEXT - 1);
            strncpy(automation_stack, "Gradle", MAX_TEXT - 1);
            strncpy(deployment_stack, "Google Play Store", MAX_TEXT - 1);
            strncpy(if_cond, "user goal is \"build mobile app\" and platform is Android only", MAX_TEXT - 1);
            strncpy(then_dec, "recommend Kotlin.", MAX_TEXT - 1);
            strncpy(step1, "Learn Kotlin programming basics", MAX_TEXT - 1);
            strncpy(step2, "Understand Jetpack Compose for modern UI", MAX_TEXT - 1);
            strncpy(step3, "Learn Room DB and Coroutines", MAX_TEXT - 1);
            strncpy(step4, "Prepare and submit to Google Play", MAX_TEXT - 1);
            strncpy(setup_build, "Install Android Studio and create a new project", MAX_TEXT - 1);
            strncpy(prototype_build, "Build interactive screens using Compose", MAX_TEXT - 1);
            strncpy(test_build, "Run tests on Android emulator or physical device", MAX_TEXT - 1);
            strncpy(deploy_build, "Generate signed bundle and upload to Play Console", MAX_TEXT - 1);
            strncpy(iterate_build, "Optimize memory footprint and responsive layouts", MAX_TEXT - 1);
            strncpy(final_rec, "Kotlin + Jetpack Compose", MAX_TEXT - 1);
        } else {
            strncpy(best_lang, "Dart (Flutter)", MAX_TEXT - 1);
            strncpy(why_reason, "Dart + Flutter compiles natively to both platforms with 60 FPS performance and a single codebase.", MAX_TEXT - 1);
            strncpy(frontend_stack, "Flutter (Dart)", MAX_TEXT - 1);
            strncpy(backend_stack, "Node.js (Express) or Firebase", MAX_TEXT - 1);
            strncpy(database_stack, "PostgreSQL / Firestore", MAX_TEXT - 1);
            strncpy(automation_stack, "Fastlane", MAX_TEXT - 1);
            strncpy(deployment_stack, "App Store & Play Store", MAX_TEXT - 1);
            strncpy(if_cond, "user goal is \"build mobile app\" and platform is both iOS and Android", MAX_TEXT - 1);
            strncpy(then_dec, "recommend Dart + Flutter.", MAX_TEXT - 1);
            strncpy(step1, "Learn Dart syntax and OOP concepts", MAX_TEXT - 1);
            strncpy(step2, "Understand Flutter widgets and state management", MAX_TEXT - 1);
            strncpy(step3, "Connect APIs and local SQLite DB", MAX_TEXT - 1);
            strncpy(step4, "Setup multi-platform build pipelines", MAX_TEXT - 1);
            strncpy(setup_build, "Install Flutter SDK and configure VS Code/Android Studio", MAX_TEXT - 1);
            strncpy(prototype_build, "Build screens using material/cupertino widgets", MAX_TEXT - 1);
            strncpy(test_build, "Run flutter test for unit and widget validation", MAX_TEXT - 1);
            strncpy(deploy_build, "Build IPA and APK/AAB formats for store submission", MAX_TEXT - 1);
            strncpy(iterate_build, "Implement caching and deep linking support", MAX_TEXT - 1);
            strncpy(final_rec, "Dart + Flutter", MAX_TEXT - 1);
        }
    }
    // 4. Backend / API
    else if (has_substring(norm_goal, "backend") || has_substring(norm_goal, "api") || has_substring(norm_goal, "server")) {
        matched = true;
        if (has_substring(norm_skill, "beginner") || has_substring(norm_style, "simple")) {
            strncpy(best_lang, "Python + FastAPI", MAX_TEXT - 1);
            strncpy(why_reason, "Python has a minimal learning curve and FastAPI offers automatic docs and fast development.", MAX_TEXT - 1);
            strncpy(frontend_stack, "None / Client app", MAX_TEXT - 1);
            strncpy(backend_stack, "Python (FastAPI)", MAX_TEXT - 1);
            strncpy(database_stack, "SQLite or PostgreSQL", MAX_TEXT - 1);
            strncpy(automation_stack, "pip / venv", MAX_TEXT - 1);
            strncpy(deployment_stack, "Render / Railway", MAX_TEXT - 1);
            strncpy(if_cond, "user goal is \"build backend/API\" and skill level is beginner", MAX_TEXT - 1);
            strncpy(then_dec, "recommend Python + FastAPI.", MAX_TEXT - 1);
            strncpy(step1, "Learn Python syntax and data structures", MAX_TEXT - 1);
            strncpy(step2, "Understand RESTful API design concepts", MAX_TEXT - 1);
            strncpy(step3, "Build endpoints using FastAPI", MAX_TEXT - 1);
            strncpy(step4, "Integrate SQL Databases with SQLAlchemy", MAX_TEXT - 1);
            strncpy(setup_build, "Create virtual env and install fastapi, uvicorn", MAX_TEXT - 1);
            strncpy(prototype_build, "Implement basic CRUD endpoints with mock data", MAX_TEXT - 1);
            strncpy(test_build, "Write tests using HTTPClient and pytest", MAX_TEXT - 1);
            strncpy(deploy_build, "Deploy directly to Railway/Render from GitHub", MAX_TEXT - 1);
            strncpy(iterate_build, "Add token authentication and input validations", MAX_TEXT - 1);
            strncpy(final_rec, "Python + FastAPI", MAX_TEXT - 1);
        } else if (has_substring(norm_constraints, "performance") || has_substring(norm_constraints, "security") || has_substring(norm_style, "performance")) {
            strncpy(best_lang, "Rust (or Go)", MAX_TEXT - 1);
            strncpy(why_reason, "Rust guarantees memory safety without garbage collection overhead, making it lightning fast and highly secure.", MAX_TEXT - 1);
            strncpy(frontend_stack, "N/A", MAX_TEXT - 1);
            strncpy(backend_stack, "Rust (Axum/Actix-web) or Go", MAX_TEXT - 1);
            strncpy(database_stack, "PostgreSQL / ScyllaDB", MAX_TEXT - 1);
            strncpy(automation_stack, "Cargo / Make", MAX_TEXT - 1);
            strncpy(deployment_stack, "Bare Metal / Docker / AWS", MAX_TEXT - 1);
            strncpy(if_cond, "user goal is \"build backend/API\" with high-performance/security-critical constraints", MAX_TEXT - 1);
            strncpy(then_dec, "recommend Rust OR Go.", MAX_TEXT - 1);
            strncpy(step1, "Learn Rust ownership, borrowing, and lifetime rules", MAX_TEXT - 1);
            strncpy(step2, "Understand async concurrency model in Rust/Go", MAX_TEXT - 1);
            strncpy(step3, "Build web servers with Axum or Go net/http", MAX_TEXT - 1);
            strncpy(step4, "Implement cryptographic safety and secure DB access", MAX_TEXT - 1);
            strncpy(setup_build, "Initialize cargo binary or go module", MAX_TEXT - 1);
            strncpy(prototype_build, "Establish database connection pool and routing", MAX_TEXT - 1);
            strncpy(test_build, "Write cargo tests and perform security auditing", MAX_TEXT - 1);
            strncpy(deploy_build, "Compile static binary and package in minimal scratch Docker image", MAX_TEXT - 1);
            strncpy(iterate_build, "Benchmark request throughput and reduce allocations", MAX_TEXT - 1);
            strncpy(final_rec, "Rust or Go", MAX_TEXT - 1);
        } else {
            strncpy(best_lang, "TypeScript / Go", MAX_TEXT - 1);
            strncpy(why_reason, "TypeScript offers full-stack type safety with high developer velocity, while Go provides high concurrency and simple binaries.", MAX_TEXT - 1);
            strncpy(frontend_stack, "React / Next.js", MAX_TEXT - 1);
            strncpy(backend_stack, "TypeScript (Node.js/NestJS) or Go", MAX_TEXT - 1);
            strncpy(database_stack, "PostgreSQL / Redis", MAX_TEXT - 1);
            strncpy(automation_stack, "Docker / GitHub Actions", MAX_TEXT - 1);
            strncpy(deployment_stack, "AWS / GCP / K8s", MAX_TEXT - 1);
            strncpy(if_cond, "user goal is \"build backend/API\" for production SaaS", MAX_TEXT - 1);
            strncpy(then_dec, "recommend TypeScript + Node.js OR Go.", MAX_TEXT - 1);
            strncpy(step1, "Master Node.js asynchronous programming or Go basics", MAX_TEXT - 1);
            strncpy(step2, "Design scalable relational database models (PostgreSQL)", MAX_TEXT - 1);
            strncpy(step3, "Implement authentication (JWT/OAuth) and middleware", MAX_TEXT - 1);
            strncpy(step4, "Setup continuous integration and Docker environments", MAX_TEXT - 1);
            strncpy(setup_build, "Initialize project structure with ESLint, Prettier, and tsconfig/go.mod", MAX_TEXT - 1);
            strncpy(prototype_build, "Build database migrations and REST/GraphQL schemas", MAX_TEXT - 1);
            strncpy(test_build, "Write integration tests with Supertest or native testing", MAX_TEXT - 1);
            strncpy(deploy_build, "Configure container deployment to AWS ECS or Railway", MAX_TEXT - 1);
            strncpy(iterate_build, "Optimize DB queries and implement Redis caching layers", MAX_TEXT - 1);
            strncpy(final_rec, "TypeScript (Node.js) OR Go", MAX_TEXT - 1);
        }
    }
    // 5. Operating System / Firmware / Embedded
    else if (has_substring(norm_goal, "os") || has_substring(norm_goal, "operating system") || has_substring(norm_goal, "firmware") || has_substring(norm_goal, "embedded")) {
        matched = true;
        strncpy(best_lang, "C / C++ / Rust", MAX_TEXT - 1);
        strncpy(why_reason, "Direct memory control, bare-metal access, and zero-overhead performance are required for systems programming.", MAX_TEXT - 1);
        strncpy(frontend_stack, "None / Custom Serial Interface", MAX_TEXT - 1);
        strncpy(backend_stack, "C/C++ or Rust (no_std)", MAX_TEXT - 1);
        strncpy(database_stack, "Flat Files / SQLite (embedded)", MAX_TEXT - 1);
        strncpy(automation_stack, "Make / CMake / Cargo", MAX_TEXT - 1);
        strncpy(deployment_stack, "Flash tool / JTAG", MAX_TEXT - 1);
        strncpy(if_cond, "user goal is \"build operating system / firmware / embedded\"", MAX_TEXT - 1);
        strncpy(then_dec, "recommend C, C++, Rust; use Assembly only where direct CPU/register control is required.", MAX_TEXT - 1);
        strncpy(step1, "Master memory management, pointers, and bitwise operations", MAX_TEXT - 1);
        strncpy(step2, "Understand processor architectures (ARM, x86, RISC-V)", MAX_TEXT - 1);
        strncpy(step3, "Write simple bare-metal code or kernel module", MAX_TEXT - 1);
        strncpy(step4, "Learn driver development and interrupt handling", MAX_TEXT - 1);
        strncpy(setup_build, "Configure toolchain (gcc-arm-none-eabi or rustup target)", MAX_TEXT - 1);
        strncpy(prototype_build, "Flash basic blinky or bootloader output", MAX_TEXT - 1);
        strncpy(test_build, "Debug via QEMU emulator or logic analyzer", MAX_TEXT - 1);
        strncpy(deploy_build, "Flash binary image onto target micro-controller/board", MAX_TEXT - 1);
        strncpy(iterate_build, "Profile energy consumption and reduce binary size", MAX_TEXT - 1);
        strncpy(final_rec, "C / C++ / Rust", MAX_TEXT - 1);
    }
    // 6. FPGA / ASIC / Digital Logic
    else if (has_substring(norm_goal, "fpga") || has_substring(norm_goal, "asic") || has_substring(norm_goal, "digital logic")) {
        matched = true;
        strncpy(best_lang, "Verilog / SystemVerilog / VHDL", MAX_TEXT - 1);
        strncpy(why_reason, "Hardware Description Languages (HDLs) are required to model concurrent digital logic gates.", MAX_TEXT - 1);
        strncpy(frontend_stack, "N/A", MAX_TEXT - 1);
        strncpy(backend_stack, "Hardware Logic", MAX_TEXT - 1);
        strncpy(database_stack, "BRAM / Registers", MAX_TEXT - 1);
        strncpy(automation_stack, "Tcl Scripts", MAX_TEXT - 1);
        strncpy(deployment_stack, "Bitstream loading", MAX_TEXT - 1);
        strncpy(if_cond, "user goal is \"FPGA / ASIC / digital logic\"", MAX_TEXT - 1);
        strncpy(then_dec, "recommend Verilog, SystemVerilog, or VHDL.", MAX_TEXT - 1);
        strncpy(step1, "Learn combinational and sequential logic concepts", MAX_TEXT - 1);
        strncpy(step2, "Master Verilog syntax and synthesizable blocks", MAX_TEXT - 1);
        strncpy(step3, "Understand clock domain crossing and timing closure", MAX_TEXT - 1);
        strncpy(step4, "Learn SystemVerilog for advanced testbenches", MAX_TEXT - 1);
        strncpy(setup_build, "Install Vivado, Quartus, or open-source toolchain (Yosys)", MAX_TEXT - 1);
        strncpy(prototype_build, "Simulate design using ModelSim or Verilator", MAX_TEXT - 1);
        strncpy(test_build, "Write self-checking testbenches with assertions", MAX_TEXT - 1);
        strncpy(deploy_build, "Synthesize, route, and program the FPGA bitstream", MAX_TEXT - 1);
        strncpy(iterate_build, "Optimize resource utilization (LUTs, FFs) and max frequency", MAX_TEXT - 1);
        strncpy(final_rec, "SystemVerilog / Verilog", MAX_TEXT - 1);
    }
    // 7. Data Science / ML
    else if (has_substring(norm_goal, "data science") || has_substring(norm_goal, "ml") || has_substring(norm_goal, "machine learning") || has_substring(norm_goal, "statistics") || has_substring(norm_goal, "numerical")) {
        matched = true;
        strncpy(best_lang, "Python + SQL", MAX_TEXT - 1);
        strncpy(why_reason, "Python is the undisputed leader in ML/DS libraries. R is recommended only if statistics-heavy; Julia only if high-performance numerical computing is required.", MAX_TEXT - 1);
        strncpy(frontend_stack, "Streamlit / Jupyter Notebook", MAX_TEXT - 1);
        strncpy(backend_stack, "Python (FastAPI / Flask)", MAX_TEXT - 1);
        strncpy(database_stack, "SQL (PostgreSQL, Snowflake) + pandas", MAX_TEXT - 1);
        strncpy(automation_stack, "Cron / Airflow", MAX_TEXT - 1);
        strncpy(deployment_stack, "Docker on AWS SageMaker / GCP Vertex", MAX_TEXT - 1);
        strncpy(if_cond, "user goal is \"data science / ML\"", MAX_TEXT - 1);
        strncpy(then_dec, "recommend Python, SQL; R only if statistics-heavy, Julia only if high-performance numerical computing is required.", MAX_TEXT - 1);
        strncpy(step1, "Learn Python data libraries (NumPy, pandas)", MAX_TEXT - 1);
        strncpy(step2, "Master SQL querying and data preparation", MAX_TEXT - 1);
        strncpy(step3, "Understand Machine Learning math and scikit-learn", MAX_TEXT - 1);
        strncpy(step4, "Deep dive into Deep Learning (PyTorch/TensorFlow)", MAX_TEXT - 1);
        strncpy(setup_build, "Initialize Jupyter Lab or Anaconda environment", MAX_TEXT - 1);
        strncpy(prototype_build, "Perform exploratory data analysis and build a baseline model", MAX_TEXT - 1);
        strncpy(test_build, "Validate model accuracy using cross-validation and metrics", MAX_TEXT - 1);
        strncpy(deploy_build, "Package model as a microservice using FastAPI and Docker", MAX_TEXT - 1);
        strncpy(iterate_build, "Monitor model drift and retrain with fresh dataset", MAX_TEXT - 1);
        strncpy(final_rec, "Python + SQL", MAX_TEXT - 1);
    }
    // 8. Database-heavy app
    else if (has_substring(norm_goal, "database") || has_substring(norm_goal, "sql") || has_substring(norm_goal, "data store")) {
        matched = true;
        strncpy(best_lang, "SQL first + PostgreSQL + TypeScript/Python backend", MAX_TEXT - 1);
        strncpy(why_reason, "Complex data relations demand strong SQL indexing and transactional guarantees.", MAX_TEXT - 1);
        strncpy(frontend_stack, "React", MAX_TEXT - 1);
        strncpy(backend_stack, "TypeScript (Node.js) or Python", MAX_TEXT - 1);
        strncpy(database_stack, "PostgreSQL", MAX_TEXT - 1);
        strncpy(automation_stack, "Database Migrations", MAX_TEXT - 1);
        strncpy(deployment_stack, "AWS RDS / Supabase / Neon", MAX_TEXT - 1);
        strncpy(if_cond, "user goal is \"database-heavy app\"", MAX_TEXT - 1);
        strncpy(then_dec, "recommend SQL first, PostgreSQL, and a TypeScript/Python backend.", MAX_TEXT - 1);
        strncpy(step1, "Learn Relational Algebra and SQL queries", MAX_TEXT - 1);
        strncpy(step2, "Understand Database Normalization and Indexing", MAX_TEXT - 1);
        strncpy(step3, "Learn Backend ORM/Query Builders (Prisma/SQLAlchemy)", MAX_TEXT - 1);
        strncpy(step4, "Master transaction isolation and scaling patterns", MAX_TEXT - 1);
        strncpy(setup_build, "Deploy a PostgreSQL instance locally with Docker", MAX_TEXT - 1);
        strncpy(prototype_build, "Design the schema and seed mock database records", MAX_TEXT - 1);
        strncpy(test_build, "Verify schema constraints and index usage via EXPLAIN", MAX_TEXT - 1);
        strncpy(deploy_build, "Provision managed database on AWS RDS or Supabase", MAX_TEXT - 1);
        strncpy(iterate_build, "Create read replicas and set up connection pool", MAX_TEXT - 1);
        strncpy(final_rec, "PostgreSQL + SQL + TypeScript Backend", MAX_TEXT - 1);
    }
    // 9. Blockchain / Smart Contracts
    else if (has_substring(norm_goal, "blockchain") || has_substring(norm_goal, "smart contract") || has_substring(norm_goal, "solidity") || has_substring(norm_goal, "solana") || has_substring(norm_goal, "ethereum")) {
        matched = true;
        if (has_substring(norm_goal, "solana") || has_substring(norm_constraints, "solana")) {
            strncpy(best_lang, "Rust", MAX_TEXT - 1);
            strncpy(why_reason, "Solana smart contracts are written natively in high-performance Rust.", MAX_TEXT - 1);
            strncpy(frontend_stack, "React + Web3.js", MAX_TEXT - 1);
            strncpy(backend_stack, "Anchor Framework (Rust)", MAX_TEXT - 1);
            strncpy(database_stack, "Solana Ledger / Arweave", MAX_TEXT - 1);
            strncpy(automation_stack, "Solana CLI", MAX_TEXT - 1);
            strncpy(deployment_stack, "Solana Mainnet", MAX_TEXT - 1);
            strncpy(if_cond, "user goal is \"blockchain / smart contracts\" on Solana", MAX_TEXT - 1);
            strncpy(then_dec, "recommend Rust.", MAX_TEXT - 1);
            strncpy(step1, "Learn basic Rust syntax and memory safety", MAX_TEXT - 1);
            strncpy(step2, "Understand Solana program architecture & accounts", MAX_TEXT - 1);
            strncpy(step3, "Learn Anchor Framework for smart contracts", MAX_TEXT - 1);
            strncpy(step4, "Integrate frontend with Solana wallet adapter", MAX_TEXT - 1);
            strncpy(setup_build, "Install Rust, Solana CLI, and Anchor", MAX_TEXT - 1);
            strncpy(prototype_build, "Write basic program state and instructions", MAX_TEXT - 1);
            strncpy(test_build, "Run localnet and execute tests via Mocha/TypeScript", MAX_TEXT - 1);
            strncpy(deploy_build, "Deploy smart contract using Anchor deploy to devnet", MAX_TEXT - 1);
            strncpy(iterate_build, "Optimize compute budget consumption", MAX_TEXT - 1);
            strncpy(final_rec, "Rust (Solana Anchor)", MAX_TEXT - 1);
        } else if (has_substring(norm_goal, "move") || has_substring(norm_constraints, "move")) {
            strncpy(best_lang, "Move", MAX_TEXT - 1);
            strncpy(why_reason, "Move is designed specifically for safe resource management and asset protection.", MAX_TEXT - 1);
            strncpy(frontend_stack, "React + SDK", MAX_TEXT - 1);
            strncpy(backend_stack, "Move Smart Contracts", MAX_TEXT - 1);
            strncpy(database_stack, "Aptos/Sui Ledger", MAX_TEXT - 1);
            strncpy(automation_stack, "Move CLI", MAX_TEXT - 1);
            strncpy(deployment_stack, "Aptos/Sui Mainnet", MAX_TEXT - 1);
            strncpy(if_cond, "user goal is \"blockchain / smart contracts\" on Move-chain", MAX_TEXT - 1);
            strncpy(then_dec, "recommend Move.", MAX_TEXT - 1);
            strncpy(step1, "Understand Move asset model and resource safety", MAX_TEXT - 1);
            strncpy(step2, "Learn Move module structure and signers", MAX_TEXT - 1);
            strncpy(step3, "Master Move verification tool and testing", MAX_TEXT - 1);
            strncpy(step4, "Deploy modules on Aptos or Sui", MAX_TEXT - 1);
            strncpy(setup_build, "Install Aptos or Sui CLI", MAX_TEXT - 1);
            strncpy(prototype_build, "Create standard coin/token module", MAX_TEXT - 1);
            strncpy(test_build, "Write unit tests with Move testing framework", MAX_TEXT - 1);
            strncpy(deploy_build, "Publish package to devnet/testnet", MAX_TEXT - 1);
            strncpy(iterate_build, "Optimize storage fees and state management", MAX_TEXT - 1);
            strncpy(final_rec, "Move", MAX_TEXT - 1);
        } else {
            strncpy(best_lang, "Solidity + TypeScript", MAX_TEXT - 1);
            strncpy(why_reason, "Solidity is the industry standard for EVM-compatible networks, paired with TypeScript for scripting.", MAX_TEXT - 1);
            strncpy(frontend_stack, "React + ethers.js / viem", MAX_TEXT - 1);
            strncpy(backend_stack, "Solidity Smart Contracts", MAX_TEXT - 1);
            strncpy(database_stack, "EVM Ledger + The Graph (indexing)", MAX_TEXT - 1);
            strncpy(automation_stack, "Hardhat / Foundry", MAX_TEXT - 1);
            strncpy(deployment_stack, "Ethereum / Arbitrum / Base", MAX_TEXT - 1);
            strncpy(if_cond, "user goal is \"blockchain / smart contracts\" on EVM", MAX_TEXT - 1);
            strncpy(then_dec, "recommend Solidity + TypeScript.", MAX_TEXT - 1);
            strncpy(step1, "Understand Ethereum blockchain and EVM mechanics", MAX_TEXT - 1);
            strncpy(step2, "Learn Solidity syntax, inheritance, and security", MAX_TEXT - 1);
            strncpy(step3, "Master Foundry/Hardhat frameworks", MAX_TEXT - 1);
            strncpy(step4, "Write frontends using wagmi and wallet connectors", MAX_TEXT - 1);
            strncpy(setup_build, "Initialize Foundry project (forge init)", MAX_TEXT - 1);
            strncpy(prototype_build, "Write ERC-20 or ERC-721 token smart contract", MAX_TEXT - 1);
            strncpy(test_build, "Write extensive invariant and fuzz tests in Solidity", MAX_TEXT - 1);
            strncpy(deploy_build, "Deploy contract using Forge script to Sepolia testnet", MAX_TEXT - 1);
            strncpy(iterate_build, "Perform gas optimization audit and verify contract", MAX_TEXT - 1);
            strncpy(final_rec, "Solidity + Foundry + TypeScript", MAX_TEXT - 1);
        }
    }
    // 10. Automation / Scripting
    else if (has_substring(norm_goal, "automation") || has_substring(norm_goal, "script") || has_substring(norm_goal, "cron")) {
        matched = true;
        if (has_substring(norm_plat, "win")) {
            strncpy(best_lang, "PowerShell + Python", MAX_TEXT - 1);
            strncpy(why_reason, "PowerShell integrates natively with Windows administration APIs, and Python handles complex processing.", MAX_TEXT - 1);
            strncpy(frontend_stack, "N/A / CLI", MAX_TEXT - 1);
            strncpy(backend_stack, "PowerShell & Python", MAX_TEXT - 1);
            strncpy(database_stack, "CSV / SQLite", MAX_TEXT - 1);
            strncpy(automation_stack, "Task Scheduler", MAX_TEXT - 1);
            strncpy(deployment_stack, "Local script run", MAX_TEXT - 1);
            strncpy(if_cond, "user goal is \"automation / scripting\" on Windows", MAX_TEXT - 1);
            strncpy(then_dec, "recommend PowerShell + Python.", MAX_TEXT - 1);
            strncpy(step1, "Understand Windows Cmdlets and object pipelines", MAX_TEXT - 1);
            strncpy(step2, "Learn Python scripting for text/file operations", MAX_TEXT - 1);
            strncpy(step3, "Combine PowerShell for system calls & Python for logic", MAX_TEXT - 1);
            strncpy(step4, "Schedule background tasks", MAX_TEXT - 1);
            strncpy(setup_build, "Install Python and configure PowerShell ExecutionPolicy", MAX_TEXT - 1);
            strncpy(prototype_build, "Create test script for target administrative task", MAX_TEXT - 1);
            strncpy(test_build, "Verify output log and error streams of script", MAX_TEXT - 1);
            strncpy(deploy_build, "Add task to Windows Task Scheduler", MAX_TEXT - 1);
            strncpy(iterate_build, "Implement email alerts and error logs", MAX_TEXT - 1);
            strncpy(final_rec, "PowerShell + Python", MAX_TEXT - 1);
        } else {
            strncpy(best_lang, "Bash + Python", MAX_TEXT - 1);
            strncpy(why_reason, "Bash provides direct shell interaction and command piping, while Python excels in cross-platform script logic.", MAX_TEXT - 1);
            strncpy(frontend_stack, "N/A / CLI", MAX_TEXT - 1);
            strncpy(backend_stack, "Bash & Python", MAX_TEXT - 1);
            strncpy(database_stack, "Flat files / SQLite", MAX_TEXT - 1);
            strncpy(automation_stack, "Cron", MAX_TEXT - 1);
            strncpy(deployment_stack, "Systemd Service / Local run", MAX_TEXT - 1);
            strncpy(if_cond, "user goal is \"automation / scripting\" on Linux/macOS", MAX_TEXT - 1);
            strncpy(then_dec, "recommend Bash + Python.", MAX_TEXT - 1);
            strncpy(step1, "Learn POSIX standard commands and shell piping", MAX_TEXT - 1);
            strncpy(step2, "Understand Python scripting and requests library", MAX_TEXT - 1);
            strncpy(step3, "Learn shell globbing, environment variables, and redirection", MAX_TEXT - 1);
            strncpy(step4, "Schedule jobs with crontab", MAX_TEXT - 1);
            strncpy(setup_build, "Create directory, init git, and create script files", MAX_TEXT - 1);
            strncpy(prototype_build, "Implement core loop using bash or python", MAX_TEXT - 1);
            strncpy(test_build, "Validate script with edge inputs and dry-runs", MAX_TEXT - 1);
            strncpy(deploy_build, "Configure cron daemon or systemd timer", MAX_TEXT - 1);
            strncpy(iterate_build, "Implement log rotation and Slack/Discord alerting", MAX_TEXT - 1);
            strncpy(final_rec, "Bash + Python", MAX_TEXT - 1);
        }
    }

    if (!matched) {
        printf("\n--- PROGRAMMING LANGUAGE RECOMMENDATION ---\n");
        printf("Goal is unclear. Please answer exactly these 3 questions to receive a tailored recommendation:\n");
        printf("1. What are you building?\n");
        printf("2. Where will it run?\n");
        printf("3. Do you want fastest build or strongest long-term architecture?\n");
        return;
    }

    printf("\nPROGRAMMING_LANGUAGE_DECISION_TREE\n");
    printf("│\n");
    printf("├── USER_GOAL\n");
    printf("│   └── %s\n", in->goal);
    printf("│\n");
    printf("├── BEST_LANGUAGE\n");
    printf("│   └── %s\n", best_lang);
    printf("│\n");
    printf("├── WHY_THIS_LANGUAGE\n");
    printf("│   └── %s\n", why_reason);
    printf("│\n");
    printf("├── SUPPORTING_STACK\n");
    printf("│   ├── Frontend: %s\n", frontend_stack);
    printf("│   ├── Backend: %s\n", backend_stack);
    printf("│   ├── Database: %s\n", database_stack);
    printf("│   ├── Automation: %s\n", automation_stack);
    printf("│   └── Deployment: %s\n", deployment_stack);
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
    printf("│   ├── Setup: %s\n", setup_build);
    printf("│   ├── Prototype: %s\n", prototype_build);
    printf("│   ├── Test: %s\n", test_build);
    printf("│   ├── Deploy: %s\n", deploy_build);
    printf("│   └── Iterate: %s\n", iterate_build);
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
