#include "save.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ff.h" 
#include "gamelogic/logic.h"

void save_build_path(const char *username, char *path, int max_len){
    snprintf(path, max_len, "%s/%s.sav", SAVE_DIR, username);
}

bool save_exists(const char *username) {
    char path[64];
    save_build_path(username, path, sizeof(path));
    FIL file;
    FRESULT res = f_open(&file, path, FA_READ);
    if (res == FR_OK) {
        f_close(&file);
        return true;
    }
    return false;
}

bool save_game(const char *username) {
    char path[64];
    save_build_path(username, path, sizeof(path));

    // 创建 saves 目录 (如不存在)
    f_mkdir(SAVE_DIR);

    FIL file;
    FRESULT res = f_open(&file, path, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) return false;

    UINT bw;

    // 写入头部
    uint32_t magic = SAVE_MAGIC;
    uint32_t version = SAVE_VERSION;
    uint32_t count = MAX_PIGS;
    uint32_t checksum = 0;

    f_write(&file, &magic, 4, &bw);
    f_write(&file, &version, 4, &bw);
    f_write(&file, &money, 4, &bw);
    checksum += *(uint32_t*)&money;
    f_write(&file, &checksum, 4, &bw);  // 占位，最后回填
    f_write(&file, &count, 4, &bw);

    // 写入每只猪的数据
    for (int i = 0; i < MAX_PIGS; i++) {
        PigSaveData psd;
        psd.growth        = pig_fsms[i].pig_t.growth;
        psd.weight        = pig_fsms[i].pig_t.weight;
        psd.hunger        = pig_fsms[i].pig_t.hunger;
        psd.eat_timer     = pig_fsms[i].pig_t.eat_timer;
        psd.eat_fruit_idx = pig_fsms[i].pig_t.eat_fruit_idx;
        psd.x             = pig_fsms[i].pig_t.x;
        psd.y             = pig_fsms[i].pig_t.y;
        psd.growth_state  = pig_fsms[i].growth_fsm.current_state;
        psd.action_state  = pig_fsms[i].action_fsm.current_state;

        f_write(&file, &psd, sizeof(PigSaveData), &bw);

        // 累加校验和
        uint8_t *p = (uint8_t*)&psd;
        for (int j = 0; j < sizeof(PigSaveData); j++) {
            checksum += p[j];
        }
    }

    // 回填校验和 (位于偏移 12)
    f_lseek(&file, 12);
    f_write(&file, &checksum, 4, &bw);

    f_close(&file);
    return true;
}

bool load_game(const char *username) {
    char path[64];
    save_build_path(username, path, sizeof(path));

    FIL file;
    FRESULT res = f_open(&file, path, FA_READ);
    if (res != FR_OK) return false;

    UINT br;
    uint32_t magic, version, checksum, saved_checksum, count;

    f_read(&file, &magic, 4, &br);
    if (magic != SAVE_MAGIC) { f_close(&file); return false; }

    f_read(&file, &version, 4, &br);
    if (version != SAVE_VERSION) { f_close(&file); return false; }

    f_read(&file, &money, 4, &br);

    f_read(&file, &saved_checksum, 4, &br);

    f_read(&file, &count, 4, &br);
    if (count != MAX_PIGS) { f_close(&file); return false; }

    uint32_t calc_checksum = *(uint32_t*)&money;

    for (int i = 0; i < MAX_PIGS; i++) {
        PigSaveData psd;
        f_read(&file, &psd, sizeof(PigSaveData), &br);

        // 覆写猪数据
        pig_fsms[i].pig_t.growth        = psd.growth;
        pig_fsms[i].pig_t.weight        = psd.weight;
        pig_fsms[i].pig_t.hunger        = psd.hunger;
        pig_fsms[i].pig_t.eat_timer     = psd.eat_timer;
        pig_fsms[i].pig_t.eat_fruit_idx = psd.eat_fruit_idx;
        pig_fsms[i].pig_t.x             = psd.x;
        pig_fsms[i].pig_t.y             = psd.y;

        // 恢复 FSM 状态
        pig_fsms[i].growth_fsm.current_state = psd.growth_state;
        pig_fsms[i].action_fsm.current_state = psd.action_state;

        // 恢复猪图像
        if (pig_fsms[i].pig_t.img_pig != NULL) {
            if (psd.growth_state == BIG) {
                lv_img_set_src(pig_fsms[i].pig_t.img_pig,
                              &pig_fsms[i].pig_t.image_pig_big);
            } else {
                lv_img_set_src(pig_fsms[i].pig_t.img_pig,
                              &pig_fsms[i].pig_t.image_pig_small);
            }
            lv_obj_set_pos(pig_fsms[i].pig_t.img_pig, psd.x, psd.y);
        }

        // 累加校验和
        uint8_t *p = (uint8_t*)&psd;
        for (int j = 0; j < sizeof(PigSaveData); j++) {
            calc_checksum += p[j];
        }
    }

    f_close(&file);

    // 校验
    if (calc_checksum != saved_checksum) {
        // 校验失败，恢复默认值
        money = 200.0f;
        return false;
    }

    // 更新金币显示
    if (coin_label != NULL) {
        lv_label_set_text_fmt(coin_label, "%.0f", money);
    }

    return true;
}
