#ifndef DATABASE_H
#define DATABASE_H

#include <time.h>

typedef struct {
    int user_id;
    char username[51];
    char role[21];
} UserInfo;

int init_database();
void cleanup_database();
int db_create_user(const char *username, const char *password_hash, const char *email, const char *full_name);
UserInfo* db_verify_user(const char *username, const char *password_hash);
int db_create_session(int user_id, const char *session_token, const char *ip_address, time_t expires_at);
int db_update_last_login(int user_id);

#endif