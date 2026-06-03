#ifndef SAVE_H
#define SAVE_H

#include <stdbool.h>

#define SAVE_MAGIC      0x50494753  // "PIGS"
#define SAVE_VERSION    1
#define SAVE_DIR        "0:/saves"

/* 每只猪的持久化数据结构 (36 bytes) */
typedef struct {
    float growth;
    float weight;
    float hunger;
    float eat_timer;
    int   eat_fruit_idx;
    int   x;
    int   y;
    int   growth_state;      // 0=NORMAL, 1=BIG, 2=SLAUGHTER
    int   action_state;      // 0=ACTION_IDLE, 1=ACTION_EAT
} PigSaveData;

/* 保存游戏进度到 0:/saves/<username>.sav */
bool save_game(const char *username);

/* 从 0:/saves/<username>.sav 加载游戏进度 */
bool load_game(const char *username);

/* 检查用户是否有存档文件 */
bool save_exists(const char *username);

/* 构建存档文件路径 (内部工具函数) */
void save_build_path(const char *username, char *path, int max_len);

#endif
