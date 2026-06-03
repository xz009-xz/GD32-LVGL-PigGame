#ifndef AUTH_H
#define AUTH_H

#include <stdbool.h>
#define MAX_USERNAME_LEN 31
#define MAX_PASSWORD_LEN 31
#define USER_CFG_PATH "0:/user.cfg"
#define REMEMBER_CFG_PATH "0:/remember.cfg"

typedef enum {
    AUTH_OK = 0,
    AUTH_ERR_FILE,
    AUTH_ERR_FORMAT,
    AUTH_ERR_USER_EXIST,
    AUTH_ERR_USER_NOT_FOUND,
    AUTH_ERR_WRONG_PASSWORD,
} AuthResult;

/* 用户注册：向 user.cfg 追加新用户 */
AuthResult auth_register(const char *username, const char *password);

/* 用户登录：验证 user.cfg 中的用户名密码 */
AuthResult auth_login(const char *username, const char *password);

/* 记住我 */
AuthResult auth_save_remember(const char *username, const char *password);
AuthResult auth_load_remember(char *username, char *password, int max_len);
bool auth_has_remember(void);
void auth_clear_remember(void);

#endif
