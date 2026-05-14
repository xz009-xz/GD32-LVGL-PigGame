# 天气灾难系统 — 详细接口设计文档

## 1. 概述

为养猪游戏添加 **全局天气灾难系统**。灾难是一段持续时间内对 **所有猪** 施加的 Debuff 效果，影响体重、饥饿和生长进度。

### 核心原则

- **逻辑与表现分离**：灾难管理器只负责 timer 和 modifier 计算，UI 动画通过查询接口驱动
- **与状态机正交**：不修改现有状态转移逻辑，只在 update 函数中叠加 modifier 计算
- **全局作用域**：灾难发生时所有猪同时受影响

---

## 2. 新增文件

| 文件 | 类型 | 说明 |
|------|------|------|
| `gamelogic/disaster.h` | 新建 | 灾难系统头文件：类型定义、接口声明 |
| `gamelogic/disaster.c` | 新建 | 灾难系统实现 |

---

## 3. 数据结构和接口定义

### 3.1 灾难类型枚举

```c
// gamelogic/disaster.h

#ifndef DISASTER_H
#define DISASTER_H

#include <stdbool.h>

/* ========== 灾难类型 ========== */
typedef enum {
    DISASTER_NONE = 0,
    DISASTER_RAIN,      // 暴雨：轻度 Debuff
    DISASTER_SNOW,      // 大雪：中度 Debuff
    DISASTER_STORM,     // 暴风：重度 Debuff
    DISASTER_TYPE_MAX
} DisasterType;
```

### 3.2 灾难修正系数（Modifier）

这是整个系统的核心数据结构——**灾难如何影响猪的属性**：

```c
/* ========== 灾难修正系数（每帧查询） ========== */
typedef struct {
    float weight_loss_rate;     // 体重减少速率 (kg/s)
                                // 灾难期间所有猪匀速掉重
                                // 无灾难时 = 0.0

    float hunger_gain_factor;   // 饥饿增长倍率
                                // 无灾难时 = 1.0（正常速度）
                                // 灾难期间 > 1.0（饿得更快）
                                // 如 1.5 表示饥饿增长速度是正常的 1.5 倍

    float growth_factor;        // 生长速度倍率
                                // 无灾难时 = 1.0（正常速度）
                                // 灾难期间 < 1.0（长得慢）
                                // 如 0.5 表示生长速度减半

    float eat_effect_factor;    // 进食效果倍率
                                // 无灾难时 = 1.0（正常进食效果）
                                // 灾难期间 < 1.0（吃了也不怎么长）
                                // 如 0.6 表示进食只发挥 60% 效果
} DisasterModifier;
```

**为什么设计 4 个系数而不是直接写死数值？**

| 系数 | 针对什么属性变化 | 生效场景 |
|------|-----------------|---------|
| `weight_loss_rate` | 体重持续下降 | IDLE + EAT 都生效 |
| `hunger_gain_factor` | 饥饿涨得更快 | IDLE 时饥饿自然增长加速 |
| `growth_factor` | 生长速度变慢 | EAT 时生长计算减速 |
| `eat_effect_factor` | 吃了也不长肉 | EAT 时体重增长减速 |

### 3.3 管理器状态结构体

```c
/* ========== 灾难管理器（内部静态变量） ========== */
// 定义在 disaster.c 中，对外部不可见
typedef struct {
    bool active;                // 是否处于灾难中
    DisasterType type;          // 当前灾难类型
    float timer;                // 剩余秒数（递减到 0 自动结束）
    float total_duration;       // 总时长（用于计算 progress）
} DisasterManager;
```

### 3.4 灾难参数配置表

```c
/* ========== 灾难配置表（静态常量，定义在 disaster.c 中） ========== */
typedef struct {
    DisasterType type;
    const char *name;           // 中文名称（调试/显示用）
    float weight_loss_rate;     // 体重减少速率
    float hunger_gain_factor;   // 饥饿增长倍率
    float growth_factor;        // 生长速度倍率
    float eat_effect_factor;    // 进食效果倍率
} DisasterConfig;
```

### 3.5 完整接口声明

```c
/* ========== 管理器接口 ========== */

/**
 * @brief 初始化灾难系统。
 *        在游戏启动时调用一次，将管理器状态清零。
 */
void disaster_init(void);

/**
 * @brief 开始指定类型的灾难。
 * 
 * @param type      灾难类型（DISASTER_RAIN / DISASTER_SNOW / DISASTER_STORM）
 * @param duration  持续秒数（建议 15~30 秒）
 * 
 * @note 如果已有灾难正在持续，新的调用会覆盖旧的（重新开始）
 * @note TODO: 触发 UI 天气动画
 */
void disaster_start(DisasterType type, float duration);

/**
 * @brief 手动结束当前灾难。
 *        灾难计时器归零时也会自动调用此函数。
 * @note TODO: 触发 UI 结束动画
 */
void disaster_stop(void);

/**
 * @brief 每帧调用，更新灾难计时器。
 *        应当在 main loop 的 update 函数中调用。
 * 
 * @param dt    帧间隔时间（秒）
 */
void disaster_update(float dt);

/**
 * @brief 查询当前是否处于灾难中。
 * @return true  = 灾难进行中
 * @return false = 无灾难
 */
bool disaster_is_active(void);

/**
 * @brief 查询当前灾难类型。
 * @return DisasterType 当前灾难类型（无灾难时返回 DISASTER_NONE）
 */
DisasterType disaster_get_type(void);

/**
 * @brief 获取当前灾难的修正系数。
 *        每只猪的 update 函数需要通过此接口获取当前系数，叠加到属性变化计算中。
 * 
 * @return DisasterModifier 
 *         - 无灾难时：weight_loss_rate=0, 其他系数=1.0
 *         - 灾难期间：返回对应类型配置的系数
 */
DisasterModifier disaster_get_modifier(void);

/**
 * @brief 获取灾难进度，范围 0.0 ~ 1.0。
 *        供 UI 层驱动动画使用。
 * 
 * @return float 
 *         0.0 = 灾难刚开始
 *         0.5 = 进行到一半
 *         1.0 = 即将结束（timer 接近 0）
 */
float disaster_get_progress(void);

/**
 * @brief 获取当前灾难的中文名称（调试用）。
 * @return const char* 灾难名称字符串
 */
const char* disaster_get_name(void);

#endif /* DISASTER_H */
```

---

## 4. 灾难参数配置（调参表）

以下数值为初始建议值，后续可根据游戏平衡性调整：

| 灾难类型 | `name` | `weight_loss_rate` | `hunger_gain_factor` | `growth_factor` | `eat_effect_factor` |
|----------|--------|:------------------:|:--------------------:|:---------------:|:-------------------:|
| 无 | "无" | 0.0 | 1.0 | 1.0 | 1.0 |
| 暴雨 ? | "暴雨" | 1.5 | 1.3 | 0.6 | 0.7 |
| 大雪 ?? | "大雪" | 2.5 | 1.6 | 0.4 | 0.5 |
| 暴风 ? | "暴风" | 3.5 | 2.0 | 0.3 | 0.3 |

**参数说明**：
- `weight_loss_rate = 1.5`：每秒掉 1.5 kg。正常进食每帧增重约 `dt * food->weight_boost`，以 `food->weight_boost=3` 计算，每秒约增重 3 kg。暴雨下净增重 = 进食增重 - 1.5，生长效率大幅降低。
- `hunger_gain_factor = 1.3`：IDLE 时饥饿增长速度是正常的 1.3 倍，猪饿得更快。
- `growth_factor = 0.6`：进食带来的生长速度降为 60%，猪长得更慢。
- `eat_effect_factor = 0.7`：进食带来的体重增长只发挥 70% 效果。

---

## 5. 现有文件修改说明

### 5.1 `gamelogic/state_machine.c` — 修改 on_update_idle 和 on_update_eat

#### on_update_idle 修改

```c
// 修改前
void on_update_idle(void *data, float dt) {
    PigData *pig = (PigData *)data;
    pig->hunger += dt * 5;          // 基础饥饿增长
    if (pig->hunger >= 100) pig->hunger = 100;
}

// 修改后
void on_update_idle(void *data, float dt) {
    PigData *pig = (PigData *)data;
    
    DisasterModifier mod = disaster_get_modifier();      // ← 新增
    
    pig->hunger += dt * 5 * mod.hunger_gain_factor;      // ← 叠加饥饿倍率
    pig->weight -= dt * mod.weight_loss_rate;             // ← 新增：灾难掉重
    
    if (pig->hunger >= 100) pig->hunger = 100;
    if (pig->weight < 0) pig->weight = 0;                 // ← 新增：防止负数
}
```

**逻辑变化**：
- 无灾难时：`mod.hunger_gain_factor = 1.0`, `mod.weight_loss_rate = 0.0` → 行为与原来完全一致
- 灾难期间：饥饿涨得更快 + 体重持续下降

#### on_update_eat 修改

```c
// 修改前
void on_update_eat(void *data, float dt, Food *food) {
    PigData *pig = (PigData *)data;
    pig->growth += dt * food[0].growth_boost;
    pig->weight += dt * food[0].weight_boost;
    pig->hunger -= dt * 10;
    if (pig->hunger <= 0) pig->hunger = 0;
}

// 修改后
void on_update_eat(void *data, float dt, Food *food) {
    PigData *pig = (PigData *)data;
    
    DisasterModifier mod = disaster_get_modifier();      // ← 新增
    
    pig->growth += dt * food[0].growth_boost * mod.growth_factor;       // ← 叠加生长倍率
    pig->weight += dt * food[0].weight_boost * mod.eat_effect_factor;   // ← 叠加进食效果倍率
    pig->weight -= dt * mod.weight_loss_rate;                            // ← 新增：灾难掉重
    
    pig->hunger -= dt * 10;                                              // 饥饿降低不受影响
    
    if (pig->hunger <= 0) pig->hunger = 0;
    if (pig->weight < 0) pig->weight = 0;                               // ← 新增：防止负数
}
```

**逻辑变化**：
- 无灾难时：`mod.growth_factor = 1.0`, `mod.eat_effect_factor = 1.0`, `mod.weight_loss_rate = 0.0` → 行为与原来完全一致
- 灾难期间：生长变慢、增重效果打折、同时持续掉重

**为什么不修改 hunger 变化？**
灾难影响的是"饥饿自然增长"（IDLE 时），而不是"进食降低饥饿"（EAT 时）。进食本身就应该是可靠的——猪吃了就该饱。天气不应该影响猪能不能吃饱。

### 5.2 `main.c` — 集成灾难更新

```c
// 修改前
#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "game_api.h"
#include "gamelogic/logic.h"

// 修改后
#include "drivers.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "game_api.h"
#include "gamelogic/logic.h"
#include "gamelogic/disaster.h"          // ← 新增

int main() {
    sys_init();
    // ... 初始化代码 ...
    ui_game_start();
    
    disaster_init();                     // ← 新增：初始化灾难系统
    
    uint32_t last_tick = lv_tick_get();
    while(1) {
        delay_us(2000);
        lv_timer_handler();
        uint32_t current_tick = lv_tick_get();
        float dt = (current_tick - last_tick) / 1000.0f;
        if(dt > 0.1f) dt = 0.1f;
        last_tick = current_tick;
        
        disaster_update(dt);             // ← 新增：更新灾难计时器
        
        // TODO: 在此处添加灾难触发逻辑（详见第6章）
        
        for(int i = 0; i < MAX_PIGS; i++){
            fsm_update(&pig_fsms[i].action_fsm, dt);
        }
        // ... growth_fsm 处理 ...
    }
}
```

### 5.3 各文件修改点汇总

| 文件 | 修改类型 | 修改内容 |
|------|---------|---------|
| `gamelogic/disaster.h` | **新建** | 类型定义 + 接口声明 |
| `gamelogic/disaster.c` | **新建** | 管理器实现 + 配置表 |
| `gamelogic/state_machine.c` | 修改 | 在 `on_update_idle` 和 `on_update_eat` 中调用 `disaster_get_modifier()` |
| `main.c` | 修改 | 加入 `#include "disaster.h"` + `disaster_init()` + `disaster_update(dt)` |

**不需要修改的文件**：
- `common/game_type.h` — 无需变动
- `common/game_api.h` — 无需变动
- `gamelogic/logic.h` — 无需变动（disaster 是独立的子系统）
- `gameui/ui_game_screen.c` — 本次不修改（后续动画对接时再改）

---

## 6. 灾难触发设计（待定，需要讨论）

本章列出几种可选的触发方式，作为扩展讨论。**当前阶段可以先不实现触发逻辑**，仅通过手动 `disaster_start()` 测试，后续再决定触发方式。

### 6.1 方式 A：时间驱动的随机触发

```c
// main.c 的 while(1) 中
static float disaster_cooldown = 0;
disaster_cooldown -= dt;

if (!disaster_is_active() && disaster_cooldown <= 0) {
    // 每 60~120 秒随机触发一次
    if (rand() % 100 < 1) {  // 每帧 1% 概率
        DisasterType type = DISASTER_RAIN + (rand() % 3);
        disaster_start(type, 15.0f + (rand() % 10));
        disaster_cooldown = 60.0f;  // 结束后至少 60 秒冷却
    }
}
```

### 6.2 方式 B：条件触发

```c
// 根据游戏状态触发
static float trigger_timer = 0;
trigger_timer += dt;

if (trigger_timer >= 30.0f) {  // 每 30 秒检查一次
    trigger_timer = 0;
    if (!disaster_is_active()) {
        // 计算所有猪的平均状态
        float avg_hunger = 0, avg_weight = 0;
        for (int i = 0; i < MAX_PIGS; i++) {
            avg_hunger += pig_fsms[i].pig_t.hunger;
            avg_weight += pig_fsms[i].pig_t.weight;
        }
        avg_hunger /= MAX_PIGS;
        avg_weight /= MAX_PIGS;
        
        // 猪群平均饥饿 > 60，触发概率增加
        float chance = 0.005f;
        if (avg_hunger > 60) chance += 0.02f;
        if (avg_weight > 80) chance += 0.01f;
        
        if (rand_float() < chance) {
            disaster_start(DISASTER_RAIN, 20.0f);
        }
    }
}
```

### 6.3 方式 C：后续通过 UI 按钮手动触发

```c
// 在 UI 回调中
void btn_disaster_cb(lv_event_t *e) {
    if (!disaster_is_active()) {
        disaster_start(DISASTER_RAIN, 15.0f);
    }
}
```

---

## 7. UI 动画对接接口

后续 UI 层对接时，灾难管理器提供的接口足够：

```c
// 在 gameui/ui_game_screen.c 中（示意，非本次实现）
void ui_disaster_update(void) {
    if (disaster_is_active()) {
        DisasterType type = disaster_get_type();
        float progress = disaster_get_progress();   // 0.0 ~ 1.0
        
        // 根据 type 加载对应天气图片/动画
        // progress 控制动画进度：
        //   progress = 0   → 动画开始（淡入）
        //   progress = 0.5 → 动画最强（全屏效果）
        //   progress = 1.0 → 动画结束（淡出）
        
        play_weather_animation(type, progress);
    } else {
        stop_weather_animation();
    }
}
```

---

## 8. 测试方案

### 8.1 手动测试

在 `main.c` 中添加测试触发：

```c
// 游戏启动后 5 秒触发一次暴雨，持续 15 秒
static float test_timer = 5.0f;
test_timer -= dt;
if (test_timer <= 0 && !disaster_is_active()) {
    disaster_start(DISASTER_RAIN, 15.0f);  // 手动触发
    // 后续可改为 DISASTER_SNOW 或 DISASTER_STORM
}
```

### 8.2 验证点

| 验证项 | 预期行为 |
|--------|---------|
| 无灾难时属性变化 | 与修改前完全一致（回归测试） |
| 灾难开始时 | `printf` 输出灾难名称和持续时长 |
| 灾难期间 IDLE | 饥饿涨速提升、体重持续下降 |
| 灾难期间 EAT | 生长变慢、增重打折、体重仍下降 |
| 灾难计时归零 | `printf` 输出结束通知，所有系数恢复默认 |
| `disaster_get_progress()` | 从 0.0 线性增长到 1.0 |

---

## 9. 实现顺序

| 步骤 | 内容 | 涉及文件 |
|------|------|---------|
| 1 | 创建 `disaster.h`，定义所有类型和接口声明 | `gamelogic/disaster.h` |
| 2 | 创建 `disaster.c`，实现配置表和管理器 | `gamelogic/disaster.c` |
| 3 | 修改 `state_machine.c`，在 update 函数中注入 modifier | `gamelogic/state_machine.c` |
| 4 | 修改 `main.c`，集成 `disaster_init()` + `disaster_update()` | `main.c` |
| 5 | 添加手动触发测试代码验证 | `main.c` |
| 6 | 编译测试 | - |

---

## 10. 后续扩展方向

- 不同食物类型对灾难的抵抗效果（如某些食物在暴风天效果衰减更少）
- 灾难期间额外扣钱（猪舍损坏维修费）
- 灾难预告系统（先提示"天气预报：暴风即将到来"）
- 多种灾难叠加
