#!/bin/bash

# MySchedule 测试入口脚本
# 调用 tests/run_all_tests.sh 执行自动化测试

set -e

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"

bash "$PROJECT_DIR/tests/run_all_tests.sh"
