#ifndef DISASTER_H
#define DISASTER_H
#include<stdbool.h>

typedef enum {
    DISASTER_NONE = 0,
    DISASTER_HOT,
    DISASTER_SNOW,
    DISASTER_RAIN,
    DISASTER_MAX
} DisasterType;

typedef struct {

    float weight_loss_rate;
    float hunger_increase_rate;
    float growth_rate_modifier;
    float eat_effect_modifier;

}DisasterModifier;

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
 * @note 如果已有灾难正在持续，新的调用会覆盖旧的（重新开始）,注意处理好动画切换。
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

#endif
