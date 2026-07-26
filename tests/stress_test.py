#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
MySchedule 压力测试脚本

测试目标：
1. 模拟大量用户注册
2. 模拟大量任务数据
3. 检查本地文件存储是否能承载大量任务
4. 验证任务文件格式是否正确

默认生成：
100 个用户
每个用户 100 个任务
总任务数 10000
"""

import os
import json
import base64
import hashlib
from datetime import datetime, timedelta

PROJECT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
DATA_DIR = os.path.join(PROJECT_DIR, "data")

USER_COUNT = 100
TASKS_PER_USER = 100
PASSWORD = "123456"


def encode_username_for_users_file(username: str) -> str:
    """
    user.cpp 中 users.dat 使用的是普通 Base64 编码用户名。
    """
    return base64.b64encode(username.encode("utf-8")).decode("ascii")


def encode_username_for_task_file(username: str) -> str:
    """
    Storage::encodeFileName 使用的是 Base64UrlEncoding + OmitTrailingEquals。
    Python 中用 urlsafe_b64encode 后去掉末尾 '=' 来保持一致。
    """
    encoded = base64.urlsafe_b64encode(username.encode("utf-8")).decode("ascii")
    return encoded.rstrip("=")


def sha256(text: str) -> str:
    """
    与 utils::sha256 保持一致：SHA-256 后转十六进制字符串。
    """
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def ensure_data_dir():
    os.makedirs(DATA_DIR, exist_ok=True)


def generate_users():
    """
    生成 users.dat。
    每行格式：
    Base64用户名 密码SHA256哈希
    """
    users_path = os.path.join(DATA_DIR, "users.dat")

    with open(users_path, "w", encoding="utf-8") as f:
        for i in range(USER_COUNT):
            username = f"stress_user_{i:03d}"
            encoded_username = encode_username_for_users_file(username)
            password_hash = sha256(PASSWORD)
            f.write(f"{encoded_username} {password_hash}\n")

    print(f"[OK] 生成用户文件: {users_path}")


def generate_tasks_for_user(username: str, user_index: int):
    """
    为单个用户生成任务文件。
    每个用户一个任务文件。
    任务文件格式为 JSON 数组。
    """
    encoded_username = encode_username_for_task_file(username)
    task_file = os.path.join(DATA_DIR, f"tasks_{encoded_username}.dat")

    base_time = datetime.now().replace(second=0, microsecond=0)
    tasks = []

    for j in range(TASKS_PER_USER):
        # 每个任务开始时间错开，确保开始时间唯一
        start_time = base_time + timedelta(days=user_index, minutes=j)

        task = {
            "id": j + 1,
            "name": f"任务_{username}_{j:03d}",
            "startTime": start_time.isoformat(),
            "priority": "中" if j % 3 == 0 else ("高" if j % 3 == 1 else "低"),
            "category": "学习" if j % 3 == 0 else ("娱乐" if j % 3 == 1 else "生活"),
            "reminderTime": (start_time - timedelta(minutes=10)).isoformat(),
            "owner": username
        }

        tasks.append(task)

    with open(task_file, "w", encoding="utf-8") as f:
        json.dump(tasks, f, ensure_ascii=False, indent=4)

    return task_file


def generate_tasks():
    """
    生成所有用户的任务文件。
    """
    total_tasks = 0

    for i in range(USER_COUNT):
        username = f"stress_user_{i:03d}"
        task_file = generate_tasks_for_user(username, i)
        total_tasks += TASKS_PER_USER

        if (i + 1) % 10 == 0:
            print(f"[OK] 已生成 {i + 1} 个用户任务文件")

    print(f"[OK] 总任务数: {total_tasks}")


def verify_generated_data():
    """
    检查生成的数据是否符合预期。
    """
    users_path = os.path.join(DATA_DIR, "users.dat")

    if not os.path.exists(users_path):
        raise RuntimeError("users.dat 不存在")

    with open(users_path, "r", encoding="utf-8") as f:
        users = [line.strip() for line in f if line.strip()]

    if len(users) != USER_COUNT:
        raise RuntimeError(f"用户数量错误，期望 {USER_COUNT}，实际 {len(users)}")

    total_tasks = 0

    for i in range(USER_COUNT):
        username = f"stress_user_{i:03d}"
        encoded_username = encode_username_for_task_file(username)
        task_file = os.path.join(DATA_DIR, f"tasks_{encoded_username}.dat")

        if not os.path.exists(task_file):
            raise RuntimeError(f"任务文件不存在: {task_file}")

        with open(task_file, "r", encoding="utf-8") as f:
            tasks = json.load(f)

        if len(tasks) != TASKS_PER_USER:
            raise RuntimeError(
                f"{username} 任务数错误，期望 {TASKS_PER_USER}，实际 {len(tasks)}"
            )

        # 检查同一用户开始时间唯一
        start_times = set()
        for task in tasks:
            start_time = task["startTime"]
            if start_time in start_times:
                raise RuntimeError(f"{username} 存在重复开始时间: {start_time}")
            start_times.add(start_time)

        total_tasks += len(tasks)

    print(f"[OK] 数据校验通过，用户数: {USER_COUNT}, 任务数: {total_tasks}")


def main():
    print("======================================")
    print(" MySchedule Stress Test")
    print("======================================")

    ensure_data_dir()
    generate_users()
    generate_tasks()
    verify_generated_data()

    print("======================================")
    print(" Stress Test Passed")
    print("======================================")
    print("")
    print("说明：")
    print(f"已生成 {USER_COUNT} 个用户，每个用户 {TASKS_PER_USER} 个任务。")
    print("可使用以下账号登录测试：")
    print("用户名: stress_user_000")
    print("密码: 123456")


if __name__ == "__main__":
    main()
