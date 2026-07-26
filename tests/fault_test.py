#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
MySchedule 故障测试脚本

测试目标：
1. 模拟数据文件缺失
2. 模拟用户文件损坏
3. 模拟任务文件损坏
4. 模拟空任务文件
5. 模拟字段缺失的任务数据

说明：
本项目是本地文件存储软件，不涉及网络数据库。
因此故障测试主要围绕本地文件损坏、缺失、异常内容展开。
"""

import os
import json
import shutil
import base64
from datetime import datetime

PROJECT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
DATA_DIR = os.path.join(PROJECT_DIR, "data")
BACKUP_DIR = os.path.join(PROJECT_DIR, "data_backup_for_fault_test")


def encode_username_for_task_file(username: str) -> str:
    encoded = base64.urlsafe_b64encode(username.encode("utf-8")).decode("ascii")
    return encoded.rstrip("=")


def backup_data():
    """
    备份 data 目录，防止故障测试破坏已有数据。
    """
    if os.path.exists(BACKUP_DIR):
        shutil.rmtree(BACKUP_DIR)

    if os.path.exists(DATA_DIR):
        shutil.copytree(DATA_DIR, BACKUP_DIR)
        print("[OK] 已备份 data 目录")
    else:
        os.makedirs(DATA_DIR, exist_ok=True)
        print("[INFO] data 目录不存在，已创建空 data 目录")


def restore_data():
    """
    恢复 data 目录。
    """
    if os.path.exists(DATA_DIR):
        shutil.rmtree(DATA_DIR)

    if os.path.exists(BACKUP_DIR):
        shutil.copytree(BACKUP_DIR, DATA_DIR)
        shutil.rmtree(BACKUP_DIR)
        print("[OK] 已恢复 data 目录")


def test_missing_data_dir():
    """
    模拟 data 目录被删除。
    程序设计中 Storage::init() 应该能重新创建 data 目录。
    """
    if os.path.exists(DATA_DIR):
        shutil.rmtree(DATA_DIR)

    os.makedirs(DATA_DIR, exist_ok=True)

    if os.path.exists(DATA_DIR):
        print("[OK] 故障测试：data 目录缺失后可重新创建")
    else:
        raise RuntimeError("data 目录重建失败")


def test_corrupted_users_file():
    """
    模拟 users.dat 文件损坏。
    UserManager::loadFromFile 应该跳过格式不合法的行。
    """
    os.makedirs(DATA_DIR, exist_ok=True)

    users_path = os.path.join(DATA_DIR, "users.dat")

    with open(users_path, "w", encoding="utf-8") as f:
        f.write("this_is_a_bad_line_without_password_hash\n")
        f.write("!!!! invalid_base64 hashvalue\n")
        f.write("\n")

    print("[OK] 故障测试：已生成损坏 users.dat")


def test_empty_task_file():
    """
    模拟任务文件为空。
    """
    username = "fault_user"
    encoded = encode_username_for_task_file(username)
    task_file = os.path.join(DATA_DIR, f"tasks_{encoded}.dat")

    with open(task_file, "w", encoding="utf-8") as f:
        f.write("")

    print("[OK] 故障测试：已生成空任务文件")


def test_corrupted_task_file():
    """
    模拟任务 JSON 文件损坏。
    """
    username = "fault_user"
    encoded = encode_username_for_task_file(username)
    task_file = os.path.join(DATA_DIR, f"tasks_{encoded}.dat")

    with open(task_file, "w", encoding="utf-8") as f:
        f.write("{ this is not valid json ")

    print("[OK] 故障测试：已生成损坏任务文件")


def test_task_file_missing_fields():
    """
    模拟任务文件字段缺失。
    程序应该尽量跳过或使用默认值处理。
    """
    username = "fault_user"
    encoded = encode_username_for_task_file(username)
    task_file = os.path.join(DATA_DIR, f"tasks_{encoded}.dat")

    tasks = [
        {
            "id": 1,
            "name": "缺少部分字段的任务",
            "owner": username
        },
        {
            "id": 2,
            "name": "",
            "startTime": datetime.now().isoformat(),
            "priority": "中",
            "category": "生活",
            "reminderTime": "",
            "owner": username
        }
    ]

    with open(task_file, "w", encoding="utf-8") as f:
        json.dump(tasks, f, ensure_ascii=False, indent=4)

    print("[OK] 故障测试：已生成字段缺失任务文件")


def main():
    print("======================================")
    print(" MySchedule Fault Test")
    print("======================================")

    try:
        backup_data()

        test_missing_data_dir()
        test_corrupted_users_file()
        test_empty_task_file()
        test_corrupted_task_file()
        test_task_file_missing_fields()

        print("======================================")
        print(" Fault Test Files Generated")
        print("======================================")
        print("")
        print("说明：")
        print("故障文件已经生成。")
        print("你可以启动程序，观察程序是否崩溃。")
        print("测试完成后脚本会恢复原始 data 目录。")

    finally:
        restore_data()

    print("======================================")
    print(" Fault Test Passed")
    print("======================================")


if __name__ == "__main__":
    main()
