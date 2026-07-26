#!/bin/bash

# MySchedule 自动化测试总入口
# 功能：
# 1. 构建测试
# 2. 压力测试
# 3. 故障测试
#
# 注意：
# 测试过程中 stress_test.py 和 fault_test.py 可能会修改 data 目录。
# 所以本脚本会在测试前备份 data，测试结束后自动恢复。
# 无论测试成功还是失败，都会执行恢复操作。

set -e

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
DATA_DIR="$PROJECT_DIR/data"
BACKUP_DIR="$PROJECT_DIR/.data_backup_for_test"
HAD_DATA_DIR=0

backup_data() {
    echo "======================================"
    echo " 备份测试前的数据目录"
    echo "======================================"

    rm -rf "$BACKUP_DIR"

    if [ -d "$DATA_DIR" ]; then
        cp -a "$DATA_DIR" "$BACKUP_DIR"
        HAD_DATA_DIR=1
        echo "[OK] 已备份 data 目录到 $BACKUP_DIR"
    else
        HAD_DATA_DIR=0
        echo "[INFO] 测试前不存在 data 目录"
    fi
}

restore_data() {
    echo "======================================"
    echo " 恢复测试前的数据目录"
    echo "======================================"

    # 删除测试过程中生成或修改的 data
    rm -rf "$DATA_DIR"

    if [ "$HAD_DATA_DIR" -eq 1 ] && [ -d "$BACKUP_DIR" ]; then
        cp -a "$BACKUP_DIR" "$DATA_DIR"
        echo "[OK] 已恢复原 data 目录"
    else
        echo "[INFO] 测试前没有 data 目录，已清理测试数据"
    fi

    # 删除备份目录
    rm -rf "$BACKUP_DIR"

    # 清理故障测试可能产生的临时目录
    rm -rf "$PROJECT_DIR/data_fault_test"
    rm -rf "$PROJECT_DIR/data_backup_for_fault_test"

    echo "[OK] 测试数据已恢复完成"
}

# 不管脚本正常结束、出错、Ctrl+C，都执行 restore_data
trap restore_data EXIT

echo "======================================"
echo " MySchedule Automated Tests"
echo "======================================"

backup_data

echo "[1/4] 构建测试..."
cd "$PROJECT_DIR"

rm -rf build
mkdir -p build
cd build
cmake ..
cmake --build .

if [ ! -f "$PROJECT_DIR/build/final_work" ]; then
    echo "错误：未生成 final_work 可执行文件"
    exit 1
fi

echo "[OK] 构建测试通过"

echo "[2/4] 压力测试..."
cd "$PROJECT_DIR"
python3 tests/stress_test.py

echo "[OK] 压力测试通过"

echo "[3/4] 功能测试..."
python3 tests/function_test.py

echo "[4/4] 故障测试..."
python3 tests/fault_test.py

echo "[OK] 故障测试通过"

echo "======================================"
echo " All Tests Passed"
echo "======================================"
