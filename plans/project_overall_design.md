# GD32-LVGL 养猪游戏 — 整体项目设计方案

> **平台**: GD32H7xx (Cortex-M7) + LVGL v8.x + FATFS  
> **屏幕**: 1024×600 LCD (LTDC+TLI)  
> **存储**: SD 卡 (FATFS R0.15)  
> **输入**: 触摸屏  
> **语言**: C

---

## 一、项目概述

本项目是一款运行在嵌入式触摸屏设备上的**模拟养猪农场游戏**。玩家通过拖拽食物喂养 10 只小猪，观察它们从小猪成长为大猪，最终出售换取金币。游戏包含用户登录/注册、存档/读档、天气灾害系统、以及背景音乐播放等功能。

### 核心玩法循环

```
喂食 → 成长/增重 → 长大 → 继续喂食 → 可屠宰 → 出售换金币 → 新小猪出生
```

---

## 二、系统分层架构

```
┌─────────────────────────────────────────────────────────┐
│                    游戏主循环 (main.c)                    │
│  初始化 → LVGL定时器 → 灾害更新 → FSM更新 → UI刷新        │
└─────────────────────────────────────────────────────────┘
          │                    │                    │
    ┌─────▼─────┐      ┌──────▼──────┐      ┌──────▼──────┐
    │  gameui/   │      │ gamelogic/  │      │   gameui/   │
    │  UI 层     │◄────►│  逻辑层      │◄────►│  动画/BGM   │
    │ (屏幕/页面) │      │ (FSM/数据)   │      │ (特效/音效)  │
    └────────────┘      └────────────┘      └─────────────┘
          │                    │                    │
          └────────────────────┼────────────────────┘
                               │
                     ┌─────────▼─────────┐
                     │    common/         │
                     │  共享类型 + API    │
                     └─────────┬─────────┘
                               │
          ┌────────────────────┼────────────────────┐
          │                    │                    │
    ┌─────▼─────┐      ┌──────▼──────┐      ┌──────▼──────┐
    │  Drivers/  │      │   FATFS/    │      │   lvgl/     │
    │  硬件驱动   │      │  文件系统    │      │  GUI 框架   │
    │ LCD/SDRAM  │      │  SD卡读写   │      │  绘图/控件   │
    │ Touch/DMA  │      │             │      │             │
    └────────────┘      └─────────────┘      └─────────────┘
```

### 层次职责

| 层级 | 目录 | 职责 |
|------|------|------|
| **UI 层** | `gameui/` | LVGL 屏幕页面、交互动画、视觉特效 |
| **逻辑层** | `gamelogic/` | FSM 状态机、猪数据、食物、认证、存档、灾害 |
| **共享层** | `common/` | 数据类型定义 (`game_type.h`)、UI/逻辑 API 桥接 (`game_api.h`) |
| **驱动层** | `Drivers/` | LCD 显示、SDRAM 内存、触摸输入、DMA 加速 |
| **文件系统** | `FATFS/` | SD 卡文件读写（用户配置、存档、图片资源） |
| **GUI 框架** | `lvgl/` | LVGL v8.x 图形库，提供控件、动画、事件系统 |
| **主循环** | `main.c` | 系统初始化、主循环调度、全局状态持有 |

---

## 三、UI 层设计 (`gameui/`)

### 3.1 页面流转图

```
┌──────────────┐   点击START    ┌──────────────┐   登录/注册成功   ┌──────────────┐
│  game_start  │──────────────►│  ui_login    │────────────────►│ ui_game      │
│  开始画面     │               │  登录面板     │                  │ _screen      │
│  (全屏背景)   │               │  (悬浮面板)   │                  │  游戏主界面   │
└──────────────┘               └──────────────┘                  └──────┬───────┘
                                                                       │
                                                          ┌────────────┼────────────┐
                                                          │            │            │
                                                    点击猪图标    拖拽食物     点击灾害按钮
                                                          │            │            │
                                                    ┌─────▼─────┐ ┌──▼───┐ ┌────▼────┐
                                                    │  ui_info  │ │ui_   │ │ui_hot/  │
                                                    │  信息弹窗  │ │fruit │ │rain/    │
                                                    │           │ │拖拽喂│ │snow     │
                                                    └───────────┘ │食动画│ │天气特效 │
                                                                   └──────┘ └─────────┘
```

### 3.2 页面详细说明

#### 3.2.1 开始画面 — `game_start.c`

- **功能**: 展示全屏背景图 (`img_start.bin`, 1024×600)，提供透明 START 按钮
- **交互**: 点击 START → 创建登录面板 (`create_login_ui()`) → 淡入切换至游戏画面
- **资源**: SD 卡加载背景图至 SDRAM

#### 3.2.2 登录面板 — `ui_login.c`

- **功能**: 用户注册/登录界面，悬浮于开始画面上方
- **控件**:
  - 标题 "PIG FARMING GAME"
  - Username 输入框 (单行，最长 31 字符)
  - Password 输入框 (密码模式，最长 31 字符)
  - Remember Me 复选框 (红色=已保存，灰色=未保存)
  - LOGIN 按钮 / REGISTER 按钮
  - 全屏键盘 (LVGL keyboard，默认隐藏)
- **交互逻辑**:
  - 点击输入框 → 弹出键盘
  - 点击面板空白 → 隐藏键盘
  - 登录成功 / 注册成功 → 进入游戏主界面
  - 失败 → 红色错误提示弹窗 (msgbox)
- **Remember Me**: 启动时自动从 `0:/remember.cfg` 读取并填充，按钮颜色指示状态

#### 3.2.3 游戏主界面 — `ui_game_screen.c`

- **功能**: 游戏核心画面，所有游戏交互的容器
- **布局** (1024×600):

```
┌──────────────────────────────────────────────────────────┐
│  [金币: 200]             屠宰数: 0       [SAVE] [天气按钮] │  ← 顶部信息栏
│                                                            │
│   ┌─────┐  ┌─────┐  ┌─────┐  ┌─────┐  ┌─────┐           │
│   │ 猪0 │  │ 猪1 │  │ 猪2 │  │ 猪3 │  │ 猪4 │   ← 上排   │
│   │ 🐷  │  │ 🐷  │  │ 🐷  │  │ 🐷  │  │ 🐷  │    y≈200   │
│   └─────┘  └─────┘  └─────┘  └─────┘  └─────┘           │
│                                                            │
│   ┌─────┐  ┌─────┐  ┌─────┐  ┌─────┐  ┌─────┐           │
│   │ 猪5 │  │ 猪6 │  │ 猪7 │  │ 猪8 │  │ 猪9 │   ← 下排   │
│   │ 🐷  │  │ 🐷  │  │ 🐷  │  │ 🐷  │  │ 🐷  │    y≈350   │
│   └─────┘  └─────┘  └─────┘  └─────┘  └─────┘           │
│                                                            │
│        [🌽 Corn]    [🌾 Hay]    [🌿 Grain]    ← 食物按钮   │
│           [?]          [?]          [?]       ← 详情按钮   │
└──────────────────────────────────────────────────────────┘
```

- **核心元素**:
  - 10 只猪图标 (100×100)，5×2 网格排列，初始显示小猪图片
  - 3 个食物按钮 (透明按钮覆盖食物图片区域)，带 "?" 详情按钮
  - 金币标签 (左上橙色) / 屠宰计数 (顶部居中红色)
  - SAVE 按钮 (右上)
  - 6 个天气测试按钮 (HOT/SNOW/RAIN × START/STOP) — 测试用途
  - 每只猪有持续抖动动画 (shack anim)
- **初始化流程**: `pig_fsm_init()` → `create_food()` → 如有存档则 `load_game()`

#### 3.2.4 食物拖拽喂养 — `ui_fruit.c`

- **交互**: 手指按下食物按钮 → 生成浮动食物图片跟随手指 → 松手检测碰撞
- **碰撞检测**: 判断松手坐标是否在任意猪的 100×100 区域内，且金币足够
- **成功喂养**: 扣除金币 → 播放喂食动画 → 触发 FSM `EVENT_START_EAT`
- **详情按钮**: 点击 "?" → 弹出 msgbox 显示食物名称、价格、成长值、增重值

#### 3.2.5 猪信息弹窗 — `ui_info.c`

- **触发**: 点击任意猪图标
- **效果**: 其他猪半透明 (OPA_50)，全屏透明遮罩 (点击关闭)
- **信息框内容** (400×300 图片底板):
  - 猪 ID
  - 体重 (Weight: X.X kg)
  - 成长进度条 (橙色, 0-100)
  - 饥饿进度条 (红色, 0-100)
- **智能定位**: 左侧组猪 → 信息框出现在右侧；右侧组猪 → 信息框镜像翻转出现在左侧

#### 3.2.6 成长动画 — `ui_pig_to_big.c`

- **触发**: `pig_grow_anim()` — 当 growth >= 50 时 FSM 调用
- **动画序列**:
  1. 全屏半透明黑色遮罩 + 猪周围圆形聚光灯
  2. 小猪图片闪烁 10 次 (显示/隐藏交替, 120ms 间隔)
  3. 切换为大猪图片
  4. 大猪图片停留 600ms
  5. 大猪图片闪烁 10 次 (80ms 间隔)
  6. 清除遮罩，动画结束

#### 3.2.7 屠宰/出售动画 — `ui_pig_to_small.c`

- **触发**: `pig_small_anim()` — 点击 "!" 按钮后 FSM 调用
- **动画序列**:
  1. "SELL" 横幅从屏幕顶部掉落至猪位置
  2. 猪图片随横幅一起下坠消失
  3. 笼子图片从顶部降落到猪的原位置
  4. 笼子淡出消失
  5. 新小猪图片从顶部降落到原位（重置为小猪）

#### 3.2.8 喂食动画 — `ui_feed .c`

- **触发**: `pig_feed_anim()` — 拖拽食物成功释放时调用
- **动画序列**:
  1. 全屏黑色半透明遮罩 + 目标猪圆形聚光灯
  2. 食物图片出现在猪旁边
  3. 食物闪烁 + 抖动 10 次 (120ms 间隔, ±6px 偏移)
  4. 食物透明度逐次递减
  5. 清除所有遮罩和食物图

#### 3.2.9 天气特效 — `ui_hot.c` / `ui_rain.c` / `ui_snow.c`

| 灾害 | 视觉效果 | 实现方式 |
|------|---------|---------|
| **HOT** (炎热) | 30% 透明度橙色遮罩 + 猪身上的汗滴动画 | 图片叠加 + timer 动画 |
| **RAIN** (下雨) | 60 个白色短线条雨滴下落 + 60% 灰色雾气 | `LV_EVENT_DRAW_POST` 定时绘制 |
| **SNOW** (下雪) | 35 个白色圆形雪花飘落 + 雪景背景 + 雪景猪图片 | 替换背景/猪图 + 雪花粒子动画 |

---

## 四、游戏逻辑层设计 (`gamelogic/`)

### 4.1 核心数据类型

```c
// === 猪的数据 ===
typedef struct {
    int id;                          // 猪编号 0-9
    float growth;                    // 成长值 0-100
    float weight;                    // 体重
    float hunger;                    // 饥饿值 0-100
    float eat_timer;                 // 进食计时器 (2秒)
    int eat_fruit_idx;               // 当前吃的食物索引
    int x, y;                        // 屏幕坐标
    lv_img_dsc_t image_pig_small;    // 小猪图片 (100×100)
    lv_img_dsc_t image_pig_big;      // 大猪图片 (100×100)
    lv_obj_t *img_pig;               // LVGL 图片对象指针
    lv_obj_t *slaughter_icon;        // "!" 出售按钮
    bool is_slaughtering;            // 是否已计数屠宰
} PigData;

// === 猪的完整实例 (双FSM + 数据) ===
typedef struct {
    Fsm growth_fsm;    // 成长状态机: NORMAL → BIG → SLAUGHTER → NORMAL
    Fsm action_fsm;    // 行为状态机: ACTION_IDLE ⇄ ACTION_EAT
    PigData pig_t;     // 猪的属性数据
} pig_fsm;

// === 食物 ===
typedef struct {
    int cost;              // 价格 (5/10/15)
    int growth_boost;      // 成长加成 (2/4/6)
    int weight_boost;      // 增重加成 (3/5/8)
    char name[20];         // 名称 (Corn/Hay/Grain)
    lv_img_dsc_t img;      // 食物图片
} Food;
```

### 4.2 双 FSM 架构 (有限状态机)

这是本项目的**核心设计模式**。每只猪拥有两个独立的 FSM，并行运行：

```
┌──────────────────────────────────────────────────────┐
│                     pig_fsm                           │
│                                                      │
│  ┌─────────────────────┐  ┌─────────────────────┐    │
│  │    growth_fsm        │  │    action_fsm        │    │
│  │  (成长状态机)         │  │  (行为状态机)         │    │
│  │                      │  │                      │    │
│  │  ┌──────┐ EVENT_GROW │  │  ┌──────────┐       │    │
│  │  │NORMAL│───► BIG    │  │  │ACTION    │       │    │
│  │  │      │◄───┘       │  │  │_IDLE     │       │    │
│  │  └──────┘     ┌──────┐│  │  └───┬──────┘       │    │
│  │   ▲          │SLAUGH││  │      │EVENT_START   │    │
│  │   │          │_TER  ││  │      │_EAT          │    │
│  │   │  EVENT_  └──┬───┘│  │  ┌───▼──────┐       │    │
│  │   │  GROW       │    │  │  │ACTION    │       │    │
│  │   └─────────────┘    │  │  │_EAT      │       │    │
│  │                      │  │  └───┬──────┘       │    │
│  └─────────────────────┘  │      │EVENT_STOP    │    │
│                           │      │_EAT          │    │
│                           │  ┌───▼──────┐       │    │
│                           │  │ACTION    │       │    │
│                           │  │_IDLE     │       │    │
│                           │  └──────────┘       │    │
│                           └─────────────────────┘    │
└──────────────────────────────────────────────────────┘
```

#### Growth FSM (成长状态机) — 3 状态

| 状态 | 进入条件 | 进入回调 | 退出回调 |
|------|---------|---------|---------|
| **NORMAL** | 初始 / 屠宰后重置 | — | — |
| **BIG** | growth ≥ 50 + `EVENT_GROW` | `growtobig()` → 播放成长动画 | — |
| **SLAUGHTER** | growth ≥ 100 + `EVENT_GROW` | `on_enter_slaughter()` → 创建红色 "!" 按钮 | `on_exit_slaughter()` → 删除按钮, 屠宰计数-1 |

**状态转换表**:
```
NORMAL ──[EVENT_GROW]──► BIG      (action: growtobig → pig_grow_anim)
BIG    ──[EVENT_GROW]──► SLAUGHTER (action: growtoslaughter)
SLAUGHTER ──[EVENT_GROW]──► NORMAL (action: growtonormal → pig_small_anim + 重置属性)
```

#### Action FSM (行为状态机) — 2 状态

| 状态 | Update 回调 (每帧) | 说明 |
|------|-------------------|------|
| **ACTION_IDLE** | `on_update_idle()` | 饥饿值上升 0.5/s × 灾害系数；饥饿≥100时扣成长和体重 |
| **ACTION_EAT** | `on_update_eat()` | 饥饿值下降 10/s × 灾害系数；成长+食物加成×灾害系数；2秒后自动停止 |

**状态转换表**:
```
ACTION_IDLE ──[EVENT_START_EAT]──► ACTION_EAT
ACTION_EAT  ──[EVENT_STOP_EAT]──► ACTION_IDLE
```

### 4.3 FSM 引擎 (`state_machine.c`)

```c
// FSM 核心结构
typedef struct {
    int current_state;              // 当前状态
    Fsm_table *table;               // 状态转换表
    int table_size;                 // 表大小
    StateFun enterFun[MAX_STATES];  // 进入回调数组
    StateFun exitFun[MAX_STATES];   // 退出回调数组
    UpdateFun updateFun[MAX_STATES];// 更新回调数组
    void *data;                     // 用户数据 (指向 PigData)
} Fsm;

// FSM 转换表条目
typedef struct {
    Event event;         // 触发事件
    int current_state;   // 当前状态
    ActionFun action;    // 转换动作 (可为NULL)
    int next_state;      // 目标状态
} Fsm_table;
```

**引擎 API**:
- `fsm_init()` — 初始化 FSM
- `fsm_eventhandle()` — 事件驱动：查表匹配 → 执行 action → 调用 exitFun → 切换状态 → 调用 enterFun
- `fsm_update()` — 帧更新：调用当前状态的 updateFun
- `enter_init()` / `exit_init()` / `update_init()` — 注册回调函数

### 4.4 认证系统 (`auth.c`)

- **存储格式**: `0:/user.cfg` — 纯文本，每行 `username:password`
- **Remember Me**: `0:/remember.cfg` — 单条记录 `username:password`
- **API**:

| 函数 | 功能 |
|------|------|
| `auth_register()` | 注册新用户 (检查重复 → 追加写入) |
| `auth_login()` | 登录验证 (逐行匹配用户名和密码) |
| `auth_save_remember()` | 保存记住的凭据 (覆盖写入) |
| `auth_load_remember()` | 读取记住的凭据 |
| `auth_has_remember()` | 检查是否存在记住文件 |
| `auth_clear_remember()` | 删除记住文件 |

### 4.5 存档系统 (`save.c`)

- **存储路径**: `0:/saves/<username>.sav`
- **二进制格式** (小端序):

```
┌────────────┬──────────┬──────────┬──────────┬──────────┬─────────────────────┐
│  Magic     │ Version  │  Money   │ Checksum │ PigCount │ SlaughterNumber     │
│  0x50494753│  1       │  float   │  uint32  │  10      │  float              │
│  4 bytes   │  4 bytes │  4 bytes │  4 bytes │  4 bytes │  4 bytes            │
├────────────┴──────────┴──────────┴──────────┴──────────┴─────────────────────┤
│                         PigSaveData × 10 (36 bytes each)                       │
│  growth(4) + weight(4) + hunger(4) + eat_timer(4) + eat_fruit_idx(4)         │
│  + x(4) + y(4) + growth_state(4) + action_state(4)                           │
└───────────────────────────────────────────────────────────────────────────────┘
```

- **安全机制**: Magic 校验 + Version 检查 + 完整 Checksum
- **自动恢复**: 登录成功后自动检测是否存在存档 → 存在则加载，不存在则新游戏

### 4.6 灾害系统 (`disaster.c`)

```
┌────────────────────────────────────────────────────────────┐
│                    DisasterManager                          │
│  active: bool     type: DisasterType    timer / duration    │
└──────────────────────┬─────────────────────────────────────┘
                       │
         ┌─────────────┼─────────────┐
         ▼             ▼             ▼
    ┌─────────┐  ┌─────────┐  ┌─────────┐
    │  HOT    │  │  SNOW   │  │  RAIN   │
    │ (炎热)   │  │ (暴雪)   │  │ (暴雨)   │
    ├─────────┤  ├─────────┤  ├─────────┤
    │wt:-0.01 │  │wt:-0.02 │  │wt:-0.015│  体重流失率
    │hu:+0.02 │  │hu:+0.03 │  │hu:+0.025│  饥饿加速率
    │gr:×0.90 │  │gr:×0.80 │  │gr:×0.85 │  成长速率
    │eat:×0.80│  │eat:×0.70│  │eat:×0.75│  食物效果
    └─────────┘  └─────────┘  └─────────┘
```

- **触发机制** (main loop 每 30 秒检测):
  1. 平均饥饿 > 60 → 概率 +50%
  2. 平均体重 > 80 → 概率 +1%
  3. 随机掷骰 → 触发随机类型灾害，持续 20 秒
- **修饰器影响**: 通过 `disaster_get_modifier()` 返回系数，应用于 `on_update_eat()` 和 `on_update_idle()` 中的所有数值计算

### 4.7 食物系统 (`type_init.c`)

| 食物 | 价格 | 成长加成 | 增重加成 | 图片文件 |
|------|------|---------|---------|---------|
| **Corn** (玉米) | 5 | 2 | 3 | `fruit1.bin` (60×71) |
| **Hay** (干草) | 10 | 4 | 5 | `fruit2.bin` (66×68) |
| **Grain** (谷物) | 15 | 6 | 8 | `fruit3.bin` (68×73) |

---

## 五、BGM 音乐系统 (`gameui/bgm.c`)

- **硬件**: SPI1 配置为 I2S Master TX 模式
- **音频数据**: `bgm.pcm` (22.4 MB) 加载至 SDRAM
- **播放方式**: DMA 双缓冲循环传输，通过 `SPI1_IRQHandler` 中断自动填充下一段数据
- **初始化**: `i2s_config()` → `nvic_irq_enable(SPI1_IRQn)` → `music_bgm_load()`

> 其余硬件相关内容（LCD 驱动、SDRAM 分配器、SD 卡驱动、触摸驱动、DMA 等）此处不展开。

---

## 六、主循环流程 (`main.c`)

```
main()
  │
  ├─ sys_init()                    // 硬件初始化 (时钟/SDRAM/SD卡)
  ├─ lv_init()                     // LVGL 初始化
  ├─ lv_port_disp_init()           // 显示驱动
  ├─ lv_port_indev_init()          // 触摸输入
  ├─ i2s_config()                  // I2S 音频配置
  ├─ music_bgm_load()              // 加载 BGM
  ├─ ui_game_start()               // 显示开始画面 + 登录面板
  ├─ disaster_init()               // 灾害系统初始化
  │
  └─ while(1)                      // *** 主循环 (~500Hz) ***
       │
       ├─ delay_us(2000)           // 2ms 延时
       ├─ lv_timer_handler()       // LVGL 定时器处理 (UI 刷新)
       │
       ├─ disaster_update(dt)      // 灾害计时更新
       ├─ [每30秒] 灾害触发检测     // 基于平均饥饿/体重概率触发
       │
       ├─ for i in 0..9:           // 所有猪的行为更新
       │    fsm_update(&action_fsm, dt)
       │
       ├─ for i in 0..9:           // 检查进食结束
       │    if eat_timer <= 0 → EVENT_STOP_EAT
       │
       ├─ for i in 0..9:           // 检查成长阶段转换
       │    if growth >= 50 && NORMAL → EVENT_GROW
       │    if growth >= 100 && BIG  → EVENT_GROW
       │
       ├─ 更新金币/屠宰标签显示
       └─ 统计新增可屠宰猪数量
```

---

## 七、数据流图

```
                        ┌─────────────┐
                        │  SD Card    │
                        │  (FATFS)    │
                        └──┬───┬───┬──┘
                           │   │   │
              ┌────────────┼───┼───┼────────────┐
              │            │   │   │            │
              ▼            ▼   ▼   ▼            ▼
        ┌─────────┐  ┌─────────┐  ┌──────────┐  ┌──────────┐
        │图片资源  │  │user.cfg │  │*.sav     │  │bgm.pcm   │
        │*.bin    │  │remember │  │存档文件   │  │音频数据   │
        └────┬────┘  │.cfg     │  └────┬─────┘  └────┬─────┘
             │       └────┬────┘       │             │
             ▼            ▼            ▼             ▼
     ┌───────────┐ ┌───────────┐ ┌───────────┐ ┌──────────┐
     │SDRAM 堆   │ │ auth.c    │ │ save.c    │ │ I2S DMA  │
     │sdram_malloc│ │ 认证逻辑   │ │ 存档逻辑   │ │ 音频播放  │
     └─────┬─────┘ └─────┬─────┘ └─────┬─────┘ └──────────┘
           │             │             │
           ▼             ▼             ▼
     ┌───────────────────────────────────────────────────┐
     │                  main.c 全局状态                    │
     │  pig_fsms[10]  │  foods[3]  │  money  │  current_user │
     └────────┬────────────────────┬─────────┬──────────────┘
              │                    │         │
    ┌─────────▼────────┐  ┌───────▼──────┐  ▼
    │  FSM 引擎         │  │ 灾害系统      │
    │  growth_fsm ×10  │  │ DisasterMgr  │
    │  action_fsm ×10  │  │ 修饰器系数    │
    └────────┬─────────┘  └──────┬───────┘
             │                   │
             └───────┬───────────┘
                     │
                     ▼
     ┌─────────────────────────────────────────┐
     │              UI 层 (LVGL)                │
     │  猪图标 │ 食物按钮 │ 进度条 │ 特效动画     │
     │  金币标签 │ 信息弹窗 │ 天气特效 │ BGM     │
     └─────────────────────────────────────────┘
```

---

## 八、事件驱动交互总览

```
触摸/点击事件                    逻辑响应                      UI 反馈
─────────────                   ────────                      ───────
点击 START 按钮          →     无                           → 淡入切换游戏画面
点击输入框               →     无                           → 弹出键盘
点击 LOGIN               →     auth_login()                → 成功→进游戏/失败→红色弹窗
点击 REGISTER            →     auth_register()             → 成功→进游戏/失败→红色弹窗
点击 Remember Me         →     auth_save/clear_remember()  → 按钮变色
按下食物按钮             →     无                           → 生成浮动食物图片
拖动食物                 →     无                           → 食物跟随手指
松手(命中猪+有钱)        →     扣钱→EVENT_START_EAT         → 喂食动画
松手(未命中/没钱)        →     无                           → 食物图消失
点击猪图标               →     无                           → 信息弹窗(其他猪半透明)
点击 "?" 按钮            →     无                           → 食物详情弹窗
点击 "!" 红色按钮        →     money+=weight → EVENT_GROW   → 屠宰动画→新小猪
点击 SAVE 按钮           →     save_game()                  → 绿色 "Saved!" 提示
点击灾害按钮(HOT等)      →     disaster_start()             → 对应天气特效
FSM: growth>=50          →     EVENT_GROW (自动)            → 成长动画
FSM: growth>=100         →     EVENT_GROW (自动)            → 出现 "!" 按钮
FSM: eat_timer<=0        →     EVENT_STOP_EAT (自动)        → 无
30秒定时器 + 概率        →     disaster_start(随机类型)      → 天气特效
```

---

## 九、文件模块依赖关系

```
main.c
 ├── drivers.h              (硬件抽象)
 ├── lv_port_disp_template.h (LVGL 显示)
 ├── lv_port_indev_template.h(LVGL 输入)
 ├── game_api.h             (UI 函数声明)
 ├── gamelogic/logic.h      (FSM + 数据 + 全局变量)
 │    └── common/game_type.h (基础类型)
 ├── gamelogic/disaster.h   (灾害系统)
 ├── gamelogic/auth.h       (认证系统)
 └── gamelogic/save.h       (存档系统)

gameui/ 各文件
 ├── lvgl.h                 (LVGL 框架)
 ├── drivers.h              (sdram_malloc 等)
 ├── game_api.h             (跨模块调用)
 └── gamelogic/logic.h      (访问 pig_fsms[], foods[], money)

gamelogic/ 各文件
 ├── common/game_type.h     (共享类型)
 └── common/game_api.h      (调用 UI 动画函数)
```

---

## 十、关键设计决策

1. **双 FSM 分离**: 成长 FSM 和行动 FSM 独立运行，互不干扰。成长状态变化不影响进食行为，进食不影响成长状态的判断条件。

2. **数据与视图分离**: `PigData` 持有数值属性（growth/weight/hunger），`lv_obj_t*` 持有 LVGL 对象引用。逻辑层通过 `game_api.h` 调用 UI 动画，UI 层通过 `logic.h` 访问全局数据。

3. **全局状态**: `pig_fsms[]`、`foods[]`、`money`、`current_user` 均为全局变量（定义在 `main.c`，extern 在 `logic.h`），简化了嵌入式环境下的数据传递。

4. **资源从 SD 卡加载**: 所有图片资源（背景、猪、食物、UI 元素）均以 `.bin` 格式存储在 SD 卡，运行时通过 `read_file_to_array()` 加载到 SDRAM，组装为 LVGL 的 `lv_img_dsc_t` 结构。

5. **动画采用链式回调**: LVGL 动画不支持 `await`，所有多阶段动画通过 `lv_anim_set_ready_cb()` 串联，形成动画链。

6. **灾害作为全局修饰器**: 灾害不影响核心 FSM 逻辑，而是通过 `disaster_get_modifier()` 返回系数，插入到 update 函数的数值计算中，实现了非侵入式设计。
