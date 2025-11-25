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

typedef struct {
    int permission_id;
    int user_id;
    int group_id;
    int can_read;
    int can_write;
    int can_delete;
    int can_manage;
} PermissionInfo;

typedef struct {
    int group_id;
    char group_name[101];
    char description[256];
    char role[21];
    int member_count;
    char created_at[64];
} GroupInfo;

typedef struct {
    int user_id;
    char username[51];
    char full_name[101];
    char role[21];
    char status[21];
    char joined_at[64];
} MemberInfo;

typedef struct {
    int request_id;
    int group_id;
    int user_id;
    char username[51];
    char full_name[101];
    char status[21];
    char created_at[64];
} JoinRequestInfo;

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
PermissionInfo* db_get_permissions(int user_id, int group_id);
int db_update_permissions(int user_id, int group_id, int can_read, int can_write, int can_delete, int can_manage);
int db_is_group_admin(int user_id, int group_id);
int db_create_group(int owner_id, const char *group_name, const char *description);
int db_get_user_groups(int user_id, GroupInfo ***groups);
int db_is_group_member(int user_id, int group_id);
int db_get_group_members(int group_id, MemberInfo ***members);
int db_request_join_group(int user_id, int group_id);
int db_get_join_requests(int group_id, JoinRequestInfo ***requests);
int db_approve_join_request(int request_id, int reviewer_id, const char *action);
JoinRequestInfo* db_get_join_request_by_id(int request_id);

#endif