#!/bin/bash

# Configuration
BINARY="./bos_omega_prod_test"
SOURCE="bos_omega_production.c"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo "--- Compiling $SOURCE ---"
# Check for libcurl dependency
if ! pkg-config --exists libcurl; then
    echo -e "${RED}libcurl development files not found. Please install libcurl4-openssl-dev.${NC}"
    false
fi

if [ $? -eq 0 ]; then
    gcc $SOURCE -o $BINARY $(pkg-config --libs --cflags libcurl)
    if [ $? -ne 0 ]; then
        echo -e "${RED}Compilation failed!${NC}"
        false
    fi
fi

if [ $? -eq 0 ]; then
    total_tests=0
    passed_tests=0

    run_test() {
        local goal="$1"
        local expected_lang="$2"
        local expected_why="$3"
        local expected_stack="$4"

        total_tests=$((total_tests + 1))
        echo "Test $total_tests: Goal = '$goal'"

        # Use timeout to prevent hanging tests
        output=$(timeout 5s $BINARY lang "$goal")
        exit_code=$?

        if [ $exit_code -ne 0 ]; then
            echo -e "  ${RED}[FAILED]${NC} Binary exited with code $exit_code or timed out"
            return
        fi

        # Check each expected string in the output
        local failed=0
        if [[ ! "$output" == *"$goal"* ]]; then
            echo -e "  ${RED}[FAILED]${NC} Goal '$goal' not found in output"
            failed=1
        fi
        if [[ ! "$output" == *"$expected_lang"* ]]; then
            echo -e "  ${RED}[FAILED]${NC} Expected language '$expected_lang' not found"
            failed=1
        fi
        if [[ ! "$output" == *"$expected_why"* ]]; then
            echo -e "  ${RED}[FAILED]${NC} Expected reasoning '$expected_why' not found"
            failed=1
        fi
        if [[ ! "$output" == *"$expected_stack"* ]]; then
            echo -e "  ${RED}[FAILED]${NC} Expected stack '$expected_stack' not found"
            failed=1
        fi

        if [ $failed -eq 0 ]; then
            echo -e "  ${GREEN}[PASSED]${NC}"
            passed_tests=$((passed_tests + 1))
        fi
        echo "----------------------------------------"
    }

    # Test Cases
    # Happy Path: AI
    run_test "Build an AI system" "C / Python / TypeScript" "C for core performance, Python for ML, TS for interface." "BOS-OMEGA Core + FastAPI + Next.js"

    # Happy Path: agent
    run_test "Deploy an agent" "C / Python / TypeScript" "C for core performance, Python for ML, TS for interface." "BOS-OMEGA Core + FastAPI + Next.js"

    # Happy Path: Other
    run_test "Build a web app" "TypeScript / Rust" "Modern safety and scalability." "Node.js + PostgreSQL + Docker"

    # Case sensitivity (str_has uses strcasestr which is case-insensitive)
    run_test "build ai" "C / Python / TypeScript" "C for core performance, Python for ML, TS for interface." "BOS-OMEGA Core + FastAPI + Next.js"

    # Substring matches (e.g. "Mountain" contains "ai")
    run_test "Mountain" "C / Python / TypeScript" "C for core performance, Python for ML, TS for interface." "BOS-OMEGA Core + FastAPI + Next.js"

    # Negative substring match
    run_test "Sky" "TypeScript / Rust" "Modern safety and scalability." "Node.js + PostgreSQL + Docker"

    # Empty goal
    run_test "" "TypeScript / Rust" "Modern safety and scalability." "Node.js + PostgreSQL + Docker"

    # Summary
    echo "Test Summary: $passed_tests / $total_tests passed"

    # Cleanup
    rm -f $BINARY

    [ $passed_tests -eq $total_tests ]
fi
