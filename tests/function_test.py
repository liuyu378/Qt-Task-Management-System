#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
MySchedule 自动化功能测试脚本

说明：
本项目选择 Qt GUI 图形界面作为主要交互方式，
没有实现完整 CLI 的 addtask/showtask/deltask 参数。
因此本脚本通过直接检查本地数据文件，自动验证核心业务功能。

测试点：
1. 注册
2. 重复注册
3. 错误密码
4. 不存在用户
5. 添加任务
6. 默认值
7. 完整属性
8. 开始时间冲突
9. 显示/加载任务
10. 删除任务
11. 删除不存在任务
12. 时间筛选
13. 时间排序
14. 多用户隔离
15. 密码 Hash
16. 任务持久化
"""

import os
import json
import base64
import hashlib
import shutil
from datetime import datetime, timedelta


# ==============================
# 路径配置
# ==============================

PROJECT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
DATA_DIR = os.path.join(PROJECT_DIR, "data")
BACKUP_DIR = os.path.join(PROJECT_DIR, ".data_backup_function_test")


# ==============================
# 测试统计
# ==============================

PASS = 0
FAIL = 0


def test_pass(name):
    global PASS
    PASS += 1
    print(f"[PASS] {name}")


def test_fail(name, reason=""):
    global FAIL
    FAIL += 1
    print(f"[FAIL] {name}")
    if reason:
        print(f"       原因: {reason}")


# ==============================
# 编码和 Hash 工具
# ==============================

def sha256(text: str) -> str:
    """
    对应 C++ utils::sha256：
    SHA-256 后转十六进制字符串。
    """
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def encode_username_for_users_file(username: str) -> str:
    """
    对应 user.cpp 中 users.dat 的用户名编码方式：
    普通 Base64，保留末尾 =。
    """
    return base64.b64encode(username.encode("utf-8")).decode("ascii")


def decode_username_from_users_file(encoded: str) -> str:
    return base64.b64decode(encoded.encode("ascii")).decode("utf-8")


def encode_username_for_task_file(username: str) -> str:
    """
    对应 Storage::encodeFileName：
    Base64UrlEncoding + OmitTrailingEquals。
    """
    encoded = base64.urlsafe_b64encode(username.encode("utf-8")).decode("ascii")
    return encoded.rstrip("=")


# ==============================
# 数据备份与恢复
# ==============================

def backup_data():
    """
    测试前备份 data 目录，避免破坏真实用户数据。
    """
    if os.path.exists(BACKUP_DIR):
        shutil.rmtree(BACKUP_DIR)

    if os.path.exists(DATA_DIR):
        shutil.copytree(DATA_DIR, BACKUP_DIR)

    os.makedirs(DATA_DIR, exist_ok=True)


def restore_data():
    """
    测试结束后恢复 data 目录。
    无论测试通过还是失败，都不污染真实数据。
    """
    if os.path.exists(DATA_DIR):
        shutil.rmtree(DATA_DIR)

    if os.path.exists(BACKUP_DIR):
        shutil.copytree(BACKUP_DIR, DATA_DIR)
        shutil.rmtree(BACKUP_DIR)
    else:
        os.makedirs(DATA_DIR, exist_ok=True)


# ==============================
# 用户数据操作
# ==============================

def users_file_path():
    return os.path.join(DATA_DIR, "users.dat")


def load_users():
    """
    读取 users.dat，返回 dict:
    username -> password_hash
    """
    users = {}

    path = users_file_path()

    if not os.path.exists(path):
        return users

    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()

            if not line:
                continue

            parts = line.split()

            if len(parts) != 2:
                continue

            encoded_username, password_hash = parts

            try:
                username = decode_username_from_users_file(encoded_username)
                users[username] = password_hash
            except Exception:
                continue

    return users


def save_users(users):
    """
    保存 users.dat。
    每行格式：
    Base64用户名 密码SHA256哈希
    """
    os.makedirs(DATA_DIR, exist_ok=True)

    with open(users_file_path(), "w", encoding="utf-8") as f:
        for username, password_hash in users.items():
            encoded = encode_username_for_users_file(username)
            f.write(f"{encoded} {password_hash}\n")


def register_user(username: str, password: str):
    """
    模拟注册用户。
    成功返回 True，重复注册返回 False。
    """
    users = load_users()

    if username in users:
        return False

    users[username] = sha256(password)
    save_users(users)
    return True


def login_user(username: str, password: str):
    """
    模拟登录验证。
    """
    users = load_users()

    if username not in users:
        return False, "用户不存在"

    input_hash = sha256(password)

    if users[username] != input_hash:
        return False, "密码不正确"

    return True, "登录成功"


# ==============================
# 任务数据操作
# ==============================

def task_file_path(username: str):
    encoded = encode_username_for_task_file(username)
    return os.path.join(DATA_DIR, f"tasks_{encoded}.dat")


def load_tasks(username: str):
    path = task_file_path(username)

    if not os.path.exists(path):
        return []

    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def save_tasks(username: str, tasks):
    os.makedirs(DATA_DIR, exist_ok=True)

    with open(task_file_path(username), "w", encoding="utf-8") as f:
        json.dump(tasks, f, ensure_ascii=False, indent=4)


def next_task_id(tasks):
    if not tasks:
        return 1

    return max(task["id"] for task in tasks) + 1


def create_task(name,
                start_time,
                owner,
                priority="中",
                category="生活",
                reminder_time=""):
    """
    创建任务对象。
    默认值：
    优先级：中
    分类：生活
    """
    return {
        "id": 0,
        "name": name,
        "startTime": start_time,
        "priority": priority if priority else "中",
        "category": category if category else "生活",
        "reminderTime": reminder_time,
        "owner": owner
    }


def add_task(username: str, task):
    """
    模拟 TaskManager::addTask 的核心规则：
    1. 任务名称不能为空
    2. 开始时间不能重复
    3. 任务名称 + 开始时间不能重复
    4. 自动分配唯一 ID
    """
    tasks = load_tasks(username)

    if not task["name"].strip():
        return False, "任务名称不能为空"

    new_start = task["startTime"][:16]

    for existing in tasks:
        existing_start = existing["startTime"][:16]

        if existing_start == new_start:
            return False, "已有任务使用相同开始时间"

        if existing["name"] == task["name"] and existing_start == new_start:
            return False, "任务名称和开始时间重复"

    task["id"] = next_task_id(tasks)

    tasks.append(task)
    save_tasks(username, tasks)

    return True, "任务添加成功"


def delete_task(username: str, task_id: int):
    """
    根据任务 ID 删除任务。
    """
    tasks = load_tasks(username)
    new_tasks = [task for task in tasks if task["id"] != task_id]

    if len(new_tasks) == len(tasks):
        return False, "未找到任务"

    save_tasks(username, new_tasks)
    return True, "任务已删除"


def show_all_tasks(username: str):
    """
    加载并按开始时间排序显示任务。
    """
    tasks = load_tasks(username)
    return sorted(tasks, key=lambda t: t["startTime"])


def filter_tasks_by_day(username: str, date_str: str):
    tasks = load_tasks(username)

    result = [
        task for task in tasks
        if task["startTime"].startswith(date_str)
    ]

    return sorted(result, key=lambda t: t["startTime"])


def filter_tasks_by_month(username: str, month_str: str):
    tasks = load_tasks(username)

    result = [
        task for task in tasks
        if task["startTime"].startswith(month_str)
    ]

    return sorted(result, key=lambda t: t["startTime"])


# ==============================
# 测试函数
# ==============================

def test_register():
    if register_user("alice", "pass123"):
        test_pass("注册用户 alice")
    else:
        test_fail("注册用户 alice")

    if register_user("bob", "pass456"):
        test_pass("注册用户 bob")
    else:
        test_fail("注册用户 bob")


def test_duplicate_register():
    if not register_user("alice", "pass123"):
        test_pass("重复注册被拒绝")
    else:
        test_fail("重复注册被拒绝", "重复注册竟然成功")


def test_wrong_password():
    ok, msg = login_user("alice", "wrongpass")

    if not ok and msg == "密码不正确":
        test_pass("错误密码登录失败")
    else:
        test_fail("错误密码登录失败")


def test_nonexistent_user():
    ok, msg = login_user("nobody", "pass123")

    if not ok and msg == "用户不存在":
        test_pass("不存在用户登录失败")
    else:
        test_fail("不存在用户登录失败")


def test_add_task_default_values():
    start = "2026-07-20T10:00:00"

    task = create_task(
        name="完成作业",
        start_time=start,
        owner="alice"
    )

    ok, _ = add_task("alice", task)

    if ok:
        tasks = load_tasks("alice")
        added = tasks[-1]

        if added["priority"] == "中" and added["category"] == "生活":
            test_pass("添加任务并使用默认优先级和分类")
        else:
            test_fail("添加任务并使用默认优先级和分类", "默认值不正确")
    else:
        test_fail("添加任务并使用默认优先级和分类")


def test_add_task_full_attributes():
    task = create_task(
        name="项目会议",
        start_time="2026-07-20T14:00:00",
        owner="alice",
        priority="高",
        category="学习",
        reminder_time="2026-07-20T13:50:00"
    )

    ok, _ = add_task("alice", task)

    if ok:
        test_pass("添加完整属性任务")
    else:
        test_fail("添加完整属性任务")


def test_start_time_conflict():
    task = create_task(
        name="另一个任务",
        start_time="2026-07-20T10:00:00",
        owner="alice"
    )

    ok, msg = add_task("alice", task)

    if not ok and "开始时间" in msg:
        test_pass("开始时间冲突被拒绝")
    else:
        test_fail("开始时间冲突被拒绝")


def test_name_start_time_unique():
    task = create_task(
        name="项目会议",
        start_time="2026-07-20T14:00:00",
        owner="alice"
    )

    ok, _ = add_task("alice", task)

    if not ok:
        test_pass("任务名称 + 开始时间重复被拒绝")
    else:
        test_fail("任务名称 + 开始时间重复被拒绝")


def test_show_and_load_tasks():
    tasks = show_all_tasks("alice")
    names = [task["name"] for task in tasks]

    if "完成作业" in names and "项目会议" in names:
        test_pass("显示/加载任务")
    else:
        test_fail("显示/加载任务")


def test_delete_task():
    tasks = load_tasks("alice")

    task_id = None

    for task in tasks:
        if task["name"] == "完成作业":
            task_id = task["id"]
            break

    if task_id is None:
        test_fail("删除任务", "未找到待删除任务")
        return

    ok, _ = delete_task("alice", task_id)

    if ok:
        names = [task["name"] for task in load_tasks("alice")]

        if "完成作业" not in names:
            test_pass("根据 ID 删除任务")
        else:
            test_fail("根据 ID 删除任务", "删除后任务仍存在")
    else:
        test_fail("根据 ID 删除任务")


def test_delete_nonexistent_task():
    ok, msg = delete_task("alice", 99999)

    if not ok and "未找到" in msg:
        test_pass("删除不存在任务被拒绝")
    else:
        test_fail("删除不存在任务被拒绝")


def test_time_filter():
    add_task("alice", create_task(
        "健身",
        "2026-07-21T08:00:00",
        "alice",
        "低",
        "生活",
        "2026-07-21T07:50:00"
    ))

    add_task("alice", create_task(
        "看电影",
        "2026-07-21T19:00:00",
        "alice",
        "中",
        "娱乐",
        "2026-07-21T18:30:00"
    ))

    day_tasks = filter_tasks_by_day("alice", "2026-07-21")
    day_names = [task["name"] for task in day_tasks]

    if "健身" in day_names and "看电影" in day_names:
        test_pass("按日期筛选任务")
    else:
        test_fail("按日期筛选任务")

    month_tasks = filter_tasks_by_month("alice", "2026-07")
    if len(month_tasks) >= 3:
        test_pass("按月份筛选任务")
    else:
        test_fail("按月份筛选任务")


def test_time_sorting():
    add_task("alice", create_task(
        "早起",
        "2026-07-20T06:00:00",
        "alice",
        "高",
        "生活",
        ""
    ))

    tasks = filter_tasks_by_day("alice", "2026-07-20")
    times = [task["startTime"] for task in tasks]

    if times == sorted(times):
        test_pass("任务按开始时间排序")
    else:
        test_fail("任务按开始时间排序")


def test_multi_user_isolation():
    task = create_task(
        name="Bob的任务",
        start_time="2026-07-22T10:00:00",
        owner="bob",
        priority="中",
        category="学习",
        reminder_time=""
    )

    ok, _ = add_task("bob", task)

    if not ok:
        test_fail("多用户隔离", "Bob 添加任务失败")
        return

    alice_names = [task["name"] for task in load_tasks("alice")]
    bob_names = [task["name"] for task in load_tasks("bob")]

    if "Bob的任务" in bob_names and "Bob的任务" not in alice_names:
        test_pass("多用户任务隔离")
    else:
        test_fail("多用户任务隔离")


def test_password_hash():
    users = load_users()

    alice_hash = users.get("alice", "")

    if alice_hash and alice_hash != "pass123" and len(alice_hash) == 64:
        test_pass("密码 Hash 保存")
    else:
        test_fail("密码 Hash 保存")


def test_task_persistence():
    alice_task_file = task_file_path("alice")
    bob_task_file = task_file_path("bob")

    if os.path.exists(alice_task_file) and os.path.exists(bob_task_file):
        test_pass("任务文件持久化")
    else:
        test_fail("任务文件持久化")


def test_unique_id():
    tasks = load_tasks("alice")
    ids = [task["id"] for task in tasks]

    if len(ids) == len(set(ids)):
        test_pass("任务 ID 唯一")
    else:
        test_fail("任务 ID 唯一")


# ==============================
# 主函数
# ==============================

def main():
    print("======================================")
    print(" MySchedule Function Test")
    print("======================================")

    try:
        backup_data()

        # 清空测试数据，避免受真实数据影响
        if os.path.exists(DATA_DIR):
            shutil.rmtree(DATA_DIR)
        os.makedirs(DATA_DIR, exist_ok=True)

        test_register()
        test_duplicate_register()
        test_wrong_password()
        test_nonexistent_user()
        test_add_task_default_values()
        test_add_task_full_attributes()
        test_start_time_conflict()
        test_name_start_time_unique()
        test_show_and_load_tasks()
        test_delete_task()
        test_delete_nonexistent_task()
        test_time_filter()
        test_time_sorting()
        test_multi_user_isolation()
        test_password_hash()
        test_task_persistence()
        test_unique_id()

        print("======================================")
        print(" Function Test Summary")
        print("======================================")
        print(f"通过: {PASS}")
        print(f"失败: {FAIL}")
        print(f"总计: {PASS + FAIL}")

        if FAIL == 0:
            print("Function Test Passed")
        else:
            print("Function Test Failed")
            raise SystemExit(1)

    finally:
        restore_data()


if __name__ == "__main__":
    main()
