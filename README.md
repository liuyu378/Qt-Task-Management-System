# MySchedule 日程管理软件

一个基于 C++/Qt 的桌面日程管理应用，支持用户注册登录、任务增删改查、筛选排序、后台提醒、语音录入等功能。

## 功能特性

- **用户系统**：注册、登录、注销，密码 SHA-256 哈希存储，不保存明文
- **任务管理**：添加、编辑、删除任务，自定义优先级（低/中/高）和分类（生活/学习/娱乐/其他）
- **筛选排序**：支持按全部/今日/本月筛选，可选按开始时间升序排列
- **定时提醒**：后台线程每 30 秒检查，到达提醒时间弹窗 + 音效通知，同任务不重复提醒
- **语音录入**：调用 Vosk 离线语音识别，自动解析日期、时间、分类和任务名称
- **数据持久化**：用户和任务数据以文件形式存储在 `data/` 目录，关闭时自动保存
- **线程安全**：QMutex 互斥锁保护共享数据，锁外处理 UI 操作，避免死锁

## 项目结构

```
├── CMakeLists.txt            # CMake 构建配置
├── README.md
├── test.sh                   # 测试入口脚本
├── include/                  # 头文件
│   ├── logindialog.h
│   ├── mainwindow.h
│   ├── storage.h
│   ├── task.h
│   ├── taskdialog.h
│   ├── TaskManager.h
│   ├── user.h
│   └── utils.h
├── src/                      # 源文件
│   ├── main.cpp              # 程序入口，登录循环
│   ├── logindialog.cpp       # 登录/注册对话框
│   ├── mainwindow.cpp        # 主界面窗口
│   ├── storage.cpp           # 文件存储管理
│   ├── taskdialog.cpp        # 任务编辑对话框
│   ├── TaskManager.cpp       # 任务增删查 + JSON 读写
│   ├── user.cpp              # 用户注册/登录
│   └── utils.cpp             # SHA-256、时间格式化
├── ui/                       # Qt Designer UI 文件
├── scripts/                  # Python 辅助脚本
│   └── voice_recognize.py    # Vosk 语音识别
├── models/                   # Vosk 语音模型
├── resources/                # 音效等资源文件
├── tests/                    # 测试脚本
└── data/                     # 运行时数据（自动生成）
```

## 环境依赖

| 依赖 | 说明 |
|------|------|
| C++17 | 编译标准 |
| CMake 3.16+ | 构建系统 |
| Qt5 或 Qt6 | GUI 框架（Widgets + Multimedia） |
| Python 3 | 语音识别脚本运行环境 |
| Vosk | 离线语音识别库（可选，仅语音录入需要） |
| sounddevice | Python 音频录制库（可选） |

## 构建运行

```bash
# 1. 进入构建目录
mkdir build && cd build

# 2. 生成 Makefile
cmake ..

# 3. 编译
make -j$(nproc)

# 4. 运行
./final_work
```

## 使用说明

1. 启动后进入登录界面，首次使用请注册新账户
2. 登录后进入主界面，可添加、编辑、删除任务
3. 通过顶部下拉框筛选任务，勾选"按开始时间排序"调整显示顺序
4. 设置提醒时间后，后台每 30 秒自动检查并弹窗 + 音效提醒
5. 点击"语音录入"可通过麦克风语音创建任务
6. 关闭窗口或退出登录时自动保存数据

## 数据存储

- 用户数据：`data/users.dat`（每行一个用户：Base64用户名 + 空格 + SHA-256密码哈希）
- 任务数据：`data/tasks_<Base64用户名>.dat`（JSON 数组格式）
- 用户名经 Base64Url 编码后作为文件名，避免特殊字符导致的路径问题

## 关键技术点

- **登录循环**：`QApplication::setQuitOnLastWindowClosed(false)` + 局部 `QEventLoop` 实现"登录→主界面→注销→登录"流程
- **无锁线程安全**：后台线程通过 `QMetaObject::invokeMethod` + `Qt::QueuedConnection` 将提醒投递到主线程，配合 `QMutex` 保证数据一致性
- **先删后加编辑**：编辑任务采用先删除旧任务再添加新任务的策略，失败时自动回滚恢复
- **时间精度统一**：所有任务时间精确到分钟，秒和毫秒清零，保证比较一致性

## 测试

```bash
./test.sh
```

测试脚本位于 `tests/` 目录，包含功能测试、故障测试和压力测试。
