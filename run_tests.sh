#!/bin/bash
set -e
gcc test_resolve_tool.c -o test_resolve_tool
./test_resolve_tool
