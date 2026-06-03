#include "auth.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ff.h"

AuthResult auth_register(const char *username, const char *password) {

    FIL fil;
    FRESULT res;


    res = f_open(&fil, USER_CFG_PATH, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
    if (res != FR_OK) {
        return AUTH_ERR_FILE;
    }

    char line[64];
    while (f_gets(line, sizeof(line), &fil)) {
        char *sep = strchr(line, ':');
        if (!sep) continue;
        *sep = '\0';
        if (strcmp(line, username) == 0) {
            f_close(&fil);
            return AUTH_ERR_USER_EXIST;
        }
    }

    f_lseek(&fil, f_size(&fil));
    f_printf(&fil, "%s:%s\n", username, password);
    f_close(&fil);
    return AUTH_OK;
}

AuthResult auth_login(const char *username, const char *password) {

    FIL fil;
    FRESULT res;

    /* "r" → FA_READ */
    res = f_open(&fil, USER_CFG_PATH, FA_READ);
    if (res != FR_OK) {
        return AUTH_ERR_FILE;
    }
    char line[64];
    while (f_gets(line, sizeof(line), &fil)) {
        char *sep = strchr(line, ':');
        if (!sep) continue;
        *sep = '\0';
        if (strcmp(line, username) == 0) {
            char *stored_pass = sep + 1;
            stored_pass[strcspn(stored_pass, "\r\n")] = '\0';
            f_close(&fil);
            if (strcmp(stored_pass, password) == 0) {
                return AUTH_OK;
            } else {
                return AUTH_ERR_WRONG_PASSWORD;
            }
        }
    }
    f_close(&fil);
    return AUTH_ERR_USER_NOT_FOUND;
}

AuthResult auth_save_remember(const char *username, const char *password){

    if (!username || !password || username[0] == '\0' || password[0] == '\0') {
        return AUTH_ERR_FORMAT;
    }

    FIL fil;
    FRESULT res;

    /* "w" → FA_WRITE | FA_CREATE_ALWAYS: create new or overwrite existing */
    res = f_open(&fil, REMEMBER_CFG_PATH, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) {
        return AUTH_ERR_FILE;
    }
    f_printf(&fil, "%s:%s\n", username, password);
    f_close(&fil);
    return AUTH_OK;
}

AuthResult auth_load_remember(char *username, char *password, int max_len){

    FIL fil;
    FRESULT res;

    /* "r" → FA_READ */
    res = f_open(&fil, REMEMBER_CFG_PATH, FA_READ);
    if (res != FR_OK) {
        return AUTH_ERR_FILE;
    }
    char line[64];
    if (!f_gets(line, sizeof(line), &fil)) {
        f_close(&fil);
        return AUTH_ERR_FORMAT;
    }
    f_close(&fil);

    char *sep = strchr(line, ':');
    if (!sep) {
        return AUTH_ERR_FORMAT;
    }
    *sep = '\0';
    char *stored_user = line;
    char *stored_pass = sep + 1;
    stored_pass[strcspn(stored_pass, "\r\n")] = '\0';

    if (username && username[0] != '\0') {
        if (strcmp(username, stored_user) != 0) {
            return AUTH_ERR_USER_NOT_FOUND;
        }
        strncpy(password, stored_pass, max_len);
        password[max_len - 1] = '\0';
        return AUTH_OK;
    }

    strncpy(username, stored_user, max_len);
    username[max_len - 1] = '\0';
    strncpy(password, stored_pass, max_len);
    password[max_len - 1] = '\0';
    return AUTH_OK;
}

bool auth_has_remember(void){
    FIL fil;
    FRESULT res;

    /* "r" → FA_READ: just check if file exists */
    res = f_open(&fil, REMEMBER_CFG_PATH, FA_READ);
    if (res != FR_OK) {
        return false;
    }
    f_close(&fil);
    return true;
}

void auth_clear_remember(void){
    /* remove() → f_unlink() */
    f_unlink(REMEMBER_CFG_PATH);
}
