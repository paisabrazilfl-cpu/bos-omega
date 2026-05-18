#!/bin/bash
set -e
echo "Compiling tests..."
gcc test_assess_risk.c -o test_assess_risk -lcurl
echo "Running tests..."
./test_assess_risk
