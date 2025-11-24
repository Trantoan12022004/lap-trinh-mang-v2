#ifndef DATABASE_H
#define DATABASE_H

#include <time.h>

typedef struct {
    int user_id;
    char username[51];
    char role[21];
    char email[101];
    char full_name[101];
} UserInfo;

int init_database();
void cleanup_database();
int db_create_user(const char *username, const char *password_hash, const char *email, const char *full_name);
UserInfo* db_verify_user(const char *username, const char *password_hash);
int db_create_session(int user_id, const char *session_token, const char *ip_address, time_t expires_at);
int db_update_last_login(int user_id);
UserInfo* db_verify_session(const char *session_token);
int db_invalidate_session(const char *session_token);
int db_update_profile(int user_id, const char *email, const char *full_name);
UserInfo* db_get_user_by_id(int user_id);
int db_change_password(int user_id, const char *new_password_hash);

#endif