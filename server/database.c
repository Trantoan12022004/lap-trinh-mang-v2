#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>
#include "database.h"

static PGconn *conn = NULL;

int init_database() {
    const char *conninfo = "host=localhost dbname=file_share_db user=postgres password=120204";
    
    conn = PQconnectdb(conninfo);
    
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
        PQfinish(conn);
        conn = NULL;
        return 0;
    }
    
    printf("Connected to database successfully\n");
    return 1;
}

void cleanup_database() {
    if (conn) {
        PQfinish(conn);
        conn = NULL;
    }
}

int db_create_user(const char *username, const char *password_hash, const char *email, const char *full_name) {
    if (!conn) return -1;
    
    const char *paramValues[4] = {username, password_hash, email, full_name};
    
    PGresult *res = PQexecParams(conn,
        "INSERT INTO users (username, password_hash, email, full_name) VALUES ($1, $2, $3, $4) RETURNING user_id",
        4, NULL, paramValues, NULL, NULL, 0);
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "INSERT failed: %s", PQerrorMessage(conn));
        
        // Check for duplicate username
        if (strstr(PQerrorMessage(conn), "duplicate key") != NULL) {
            PQclear(res);
            return -2;
        }
        
        PQclear(res);
        return -1;
    }
    
    int user_id = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    
    return user_id;
}

UserInfo* db_verify_user(const char *username, const char *password_hash) {
    if (!conn) return NULL;
    
    const char *paramValues[2] = {username, password_hash};
    
    PGresult *res = PQexecParams(conn,
        "SELECT user_id, username, role FROM users WHERE username = $1 AND password_hash = $2",
        2, NULL, paramValues, NULL, NULL, 0);
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        PQclear(res);
        return NULL;
    }
    
    UserInfo *user = (UserInfo*)malloc(sizeof(UserInfo));
    user->user_id = atoi(PQgetvalue(res, 0, 0));
    strncpy(user->username, PQgetvalue(res, 0, 1), 50);
    user->username[50] = '\0';
    strncpy(user->role, PQgetvalue(res, 0, 2), 20);
    user->role[20] = '\0';
    
    PQclear(res);
    
    return user;
}

int db_create_session(int user_id, const char *session_token, const char *ip_address, time_t expires_at) {
    if (!conn) return 0;
    
    char user_id_str[32], expires_str[32];
    sprintf(user_id_str, "%d", user_id);
    sprintf(expires_str, "%ld", expires_at);
    
    const char *paramValues[4] = {user_id_str, session_token, ip_address, expires_str};
    
    PGresult *res = PQexecParams(conn,
        "INSERT INTO sessions (user_id, session_token, ip_address, expires_at) VALUES ($1, $2, $3, to_timestamp($4))",
        4, NULL, paramValues, NULL, NULL, 0);
    
    int success = (PQresultStatus(res) == PGRES_COMMAND_OK);
    PQclear(res);
    
    return success;
}

int db_update_last_login(int user_id) {
    if (!conn) return 0;
    
    char user_id_str[32];
    sprintf(user_id_str, "%d", user_id);
    
    const char *paramValues[1] = {user_id_str};
    
    PGresult *res = PQexecParams(conn,
        "UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE user_id = $1",
        1, NULL, paramValues, NULL, NULL, 0);
    
    int success = (PQresultStatus(res) == PGRES_COMMAND_OK);
    PQclear(res);
    
    return success;
}

UserInfo* db_verify_session(const char *session_token) {
    if (!conn) return NULL;
    
    const char *paramValues[1] = {session_token};
    
    PGresult *res = PQexecParams(conn,
        "SELECT s.user_id, u.username, u.role, s.expires_at FROM sessions s "
        "JOIN users u ON s.user_id = u.user_id "
        "WHERE s.session_token = $1 AND s.is_active = TRUE AND s.expires_at > CURRENT_TIMESTAMP",
        1, NULL, paramValues, NULL, NULL, 0);
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        PQclear(res);
        return NULL;
    }
    
    UserInfo *user = (UserInfo*)malloc(sizeof(UserInfo));
    user->user_id = atoi(PQgetvalue(res, 0, 0));
    strncpy(user->username, PQgetvalue(res, 0, 1), 50);
    user->username[50] = '\0';
    strncpy(user->role, PQgetvalue(res, 0, 2), 20);
    user->role[20] = '\0';
    
    PQclear(res);
    return user;
}

int db_invalidate_session(const char *session_token) {
    if (!conn) return 0;
    
    const char *paramValues[1] = {session_token};
    
    PGresult *res = PQexecParams(conn,
        "UPDATE sessions SET is_active = FALSE WHERE session_token = $1",
        1, NULL, paramValues, NULL, NULL, 0);
    
    int success = (PQresultStatus(res) == PGRES_COMMAND_OK);
    PQclear(res);
    
    return success;
}

int db_update_profile(int user_id, const char *email, const char *full_name) {
    if (!conn) return 0;
    
    char user_id_str[32];
    sprintf(user_id_str, "%d", user_id);
    
    const char *paramValues[3] = {user_id_str, email, full_name};
    
    PGresult *res = PQexecParams(conn,
        "UPDATE users SET email = $2, full_name = $3 WHERE user_id = $1",
        3, NULL, paramValues, NULL, NULL, 0);
    
    int success = (PQresultStatus(res) == PGRES_COMMAND_OK);
    PQclear(res);
    
    return success;
}

UserInfo* db_get_user_by_id(int user_id) {
    if (!conn) return NULL;
    
    char user_id_str[32];
    sprintf(user_id_str, "%d", user_id);
    
    const char *paramValues[1] = {user_id_str};
    
    PGresult *res = PQexecParams(conn,
        "SELECT user_id, username, role, email, full_name FROM users WHERE user_id = $1",
        1, NULL, paramValues, NULL, NULL, 0);
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        PQclear(res);
        return NULL;
    }
    
    UserInfo *user = (UserInfo*)malloc(sizeof(UserInfo));
    user->user_id = atoi(PQgetvalue(res, 0, 0));
    strncpy(user->username, PQgetvalue(res, 0, 1), 50);
    user->username[50] = '\0';
    strncpy(user->role, PQgetvalue(res, 0, 2), 20);
    user->role[20] = '\0';
    strncpy(user->email, PQgetvalue(res, 0, 3), 100);
    user->email[100] = '\0';
    strncpy(user->full_name, PQgetvalue(res, 0, 4), 100);
    user->full_name[100] = '\0';
    
    PQclear(res);
    return user;
}

int db_change_password(int user_id, const char *new_password_hash) {
    if (!conn) return 0;
    
    char user_id_str[32];
    sprintf(user_id_str, "%d", user_id);
    
    const char *paramValues[2] = {user_id_str, new_password_hash};
    
    PGresult *res = PQexecParams(conn,
        "UPDATE users SET password_hash = $2 WHERE user_id = $1",
        2, NULL, paramValues, NULL, NULL, 0);
    
    int success = (PQresultStatus(res) == PGRES_COMMAND_OK);
    PQclear(res);
    
    return success;
}

PermissionInfo* db_get_permissions(int user_id, int group_id) {
    if (!conn) return NULL;
    
    char user_id_str[32], group_id_str[32];
    sprintf(user_id_str, "%d", user_id);
    sprintf(group_id_str, "%d", group_id);
    
    const char *paramValues[2] = {user_id_str, group_id_str};
    
    PGresult *res = PQexecParams(conn,
        "SELECT permission_id, user_id, group_id, can_read, can_write, can_delete, can_manage "
        "FROM permissions WHERE user_id = $1 AND group_id = $2",
        2, NULL, paramValues, NULL, NULL, 0);
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        PQclear(res);
        return NULL;
    }
    
    PermissionInfo *perm = (PermissionInfo*)malloc(sizeof(PermissionInfo));
    perm->permission_id = atoi(PQgetvalue(res, 0, 0));
    perm->user_id = atoi(PQgetvalue(res, 0, 1));
    perm->group_id = atoi(PQgetvalue(res, 0, 2));
    perm->can_read = strcmp(PQgetvalue(res, 0, 3), "t") == 0 ? 1 : 0;
    perm->can_write = strcmp(PQgetvalue(res, 0, 4), "t") == 0 ? 1 : 0;
    perm->can_delete = strcmp(PQgetvalue(res, 0, 5), "t") == 0 ? 1 : 0;
    perm->can_manage = strcmp(PQgetvalue(res, 0, 6), "t") == 0 ? 1 : 0;
    
    PQclear(res);
    return perm;
}

int db_update_permissions(int user_id, int group_id, int can_read, int can_write, int can_delete, int can_manage) {
    if (!conn) return 0;
    
    char user_id_str[32], group_id_str[32];
    char can_read_str[10], can_write_str[10], can_delete_str[10], can_manage_str[10];
    
    sprintf(user_id_str, "%d", user_id);
    sprintf(group_id_str, "%d", group_id);
    sprintf(can_read_str, "%s", can_read ? "true" : "false");
    sprintf(can_write_str, "%s", can_write ? "true" : "false");
    sprintf(can_delete_str, "%s", can_delete ? "true" : "false");
    sprintf(can_manage_str, "%s", can_manage ? "true" : "false");
    
    const char *paramValues[6] = {user_id_str, group_id_str, can_read_str, can_write_str, can_delete_str, can_manage_str};
    
    // Check if permission exists
    PGresult *check_res = PQexecParams(conn,
        "SELECT permission_id FROM permissions WHERE user_id = $1 AND group_id = $2",
        2, NULL, paramValues, NULL, NULL, 0);
    
    int exists = (PQresultStatus(check_res) == PGRES_TUPLES_OK && PQntuples(check_res) > 0);
    PQclear(check_res);
    
    PGresult *res;
    if (exists) {
        // Update existing permission
        res = PQexecParams(conn,
            "UPDATE permissions SET can_read = $3, can_write = $4, can_delete = $5, can_manage = $6 "
            "WHERE user_id = $1 AND group_id = $2",
            6, NULL, paramValues, NULL, NULL, 0);
    } else {
        // Insert new permission
        res = PQexecParams(conn,
            "INSERT INTO permissions (user_id, group_id, can_read, can_write, can_delete, can_manage) "
            "VALUES ($1, $2, $3, $4, $5, $6)",
            6, NULL, paramValues, NULL, NULL, 0);
    }
    
    int success = (PQresultStatus(res) == PGRES_COMMAND_OK);
    PQclear(res);
    
    return success;
}

int db_is_group_admin(int user_id, int group_id) {
    if (!conn) return 0;
    
    char user_id_str[32], group_id_str[32];
    sprintf(user_id_str, "%d", user_id);
    sprintf(group_id_str, "%d", group_id);
    
    const char *paramValues[2] = {user_id_str, group_id_str};
    
    // Check if user is owner of the group
    PGresult *res = PQexecParams(conn,
        "SELECT group_id FROM groups WHERE group_id = $2 AND owner_id = $1",
        2, NULL, paramValues, NULL, NULL, 0);
    
    int is_owner = (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0);
    PQclear(res);
    
    if (is_owner) return 1;
    
    // Check if user is admin member of the group
    res = PQexecParams(conn,
        "SELECT member_id FROM group_members WHERE user_id = $1 AND group_id = $2 AND role = 'admin' AND status = 'approved'",
        2, NULL, paramValues, NULL, NULL, 0);
    
    int is_admin = (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0);
    PQclear(res);
    
    return is_admin;
}

int db_create_group(int owner_id, const char *group_name, const char *description) {
    if (!conn) return -1;
    
    char owner_id_str[32];
    sprintf(owner_id_str, "%d", owner_id);
    
    const char *paramValues[3] = {group_name, description ? description : "", owner_id_str};
    
    PGresult *res = PQexecParams(conn,
        "INSERT INTO groups (group_name, description, owner_id) VALUES ($1, $2, $3) RETURNING group_id",
        3, NULL, paramValues, NULL, NULL, 0);
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "INSERT group failed: %s", PQerrorMessage(conn));
        PQclear(res);
        return -1;
    }
    
    int group_id = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    
    // Add owner as admin member
    char group_id_str[32];
    sprintf(group_id_str, "%d", group_id);
    const char *memberParams[2] = {group_id_str, owner_id_str};
    
    res = PQexecParams(conn,
        "INSERT INTO group_members (group_id, user_id, role, status) VALUES ($1, $2, 'admin', 'approved')",
        2, NULL, memberParams, NULL, NULL, 0);
    PQclear(res);
    
    // Create full permissions for owner (admin)
    const char *permParams[2] = {group_id_str, owner_id_str};
    res = PQexecParams(conn,
        "INSERT INTO permissions (group_id, user_id, can_read, can_write, can_delete, can_manage) "
        "VALUES ($1, $2, TRUE, TRUE, TRUE, TRUE)",
        2, NULL, permParams, NULL, NULL, 0);
    PQclear(res);
    
    return group_id;
}

int db_get_user_groups(int user_id, GroupInfo ***groups) {
    if (!conn) return 0;
    
    char user_id_str[32];
    sprintf(user_id_str, "%d", user_id);
    
    const char *paramValues[1] = {user_id_str};
    
    PGresult *res = PQexecParams(conn,
        "SELECT g.group_id, g.group_name, g.description, "
        "CASE WHEN g.owner_id = $1 THEN 'admin' ELSE COALESCE(gm.role, 'member') END as role, "
        "(SELECT COUNT(*) FROM group_members WHERE group_id = g.group_id AND status = 'approved') as member_count, "
        "g.created_at "
        "FROM groups g "
        "LEFT JOIN group_members gm ON g.group_id = gm.group_id AND gm.user_id = $1 "
        "WHERE g.owner_id = $1 OR (gm.user_id = $1 AND gm.status = 'approved') "
        "ORDER BY g.created_at DESC",
        1, NULL, paramValues, NULL, NULL, 0);
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return 0;
    }
    
    int count = PQntuples(res);
    if (count == 0) {
        PQclear(res);
        return 0;
    }
    
    *groups = (GroupInfo**)malloc(count * sizeof(GroupInfo*));
    
    for (int i = 0; i < count; i++) {
        (*groups)[i] = (GroupInfo*)malloc(sizeof(GroupInfo));
        (*groups)[i]->group_id = atoi(PQgetvalue(res, i, 0));
        strncpy((*groups)[i]->group_name, PQgetvalue(res, i, 1), 100);
        (*groups)[i]->group_name[100] = '\0';
        strncpy((*groups)[i]->description, PQgetvalue(res, i, 2), 255);
        (*groups)[i]->description[255] = '\0';
        strncpy((*groups)[i]->role, PQgetvalue(res, i, 3), 20);
        (*groups)[i]->role[20] = '\0';
        (*groups)[i]->member_count = atoi(PQgetvalue(res, i, 4));
        strncpy((*groups)[i]->created_at, PQgetvalue(res, i, 5), 63);
        (*groups)[i]->created_at[63] = '\0';
    }
    
    PQclear(res);
    return count;
}

int db_is_group_member(int user_id, int group_id) {
    if (!conn) return 0;
    
    char user_id_str[32], group_id_str[32];
    sprintf(user_id_str, "%d", user_id);
    sprintf(group_id_str, "%d", group_id);
    
    const char *paramValues[2] = {user_id_str, group_id_str};
    
    // Check if user is owner
    PGresult *res = PQexecParams(conn,
        "SELECT group_id FROM groups WHERE group_id = $2 AND owner_id = $1",
        2, NULL, paramValues, NULL, NULL, 0);
    
    int is_owner = (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0);
    PQclear(res);
    
    if (is_owner) return 1;
    
    // Check if user is approved member
    res = PQexecParams(conn,
        "SELECT member_id FROM group_members WHERE user_id = $1 AND group_id = $2 AND status = 'approved'",
        2, NULL, paramValues, NULL, NULL, 0);
    
    int is_member = (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0);
    PQclear(res);
    
    return is_member;
}

int db_get_group_members(int group_id, MemberInfo ***members) {
    if (!conn) return 0;
    
    char group_id_str[32];
    sprintf(group_id_str, "%d", group_id);
    
    const char *paramValues[1] = {group_id_str};
    
    PGresult *res = PQexecParams(conn,
        "SELECT u.user_id, u.username, u.full_name, gm.role, gm.status, gm.joined_at "
        "FROM group_members gm "
        "JOIN users u ON gm.user_id = u.user_id "
        "WHERE gm.group_id = $1 "
        "ORDER BY gm.joined_at ASC",
        1, NULL, paramValues, NULL, NULL, 0);
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return 0;
    }
    
    int count = PQntuples(res);
    if (count == 0) {
        PQclear(res);
        return 0;
    }
    
    *members = (MemberInfo**)malloc(count * sizeof(MemberInfo*));
    
    for (int i = 0; i < count; i++) {
        (*members)[i] = (MemberInfo*)malloc(sizeof(MemberInfo));
        (*members)[i]->user_id = atoi(PQgetvalue(res, i, 0));
        strncpy((*members)[i]->username, PQgetvalue(res, i, 1), 50);
        (*members)[i]->username[50] = '\0';
        strncpy((*members)[i]->full_name, PQgetvalue(res, i, 2), 100);
        (*members)[i]->full_name[100] = '\0';
        strncpy((*members)[i]->role, PQgetvalue(res, i, 3), 20);
        (*members)[i]->role[20] = '\0';
        strncpy((*members)[i]->status, PQgetvalue(res, i, 4), 20);
        (*members)[i]->status[20] = '\0';
        strncpy((*members)[i]->joined_at, PQgetvalue(res, i, 5), 63);
        (*members)[i]->joined_at[63] = '\0';
    }
    
    PQclear(res);
    return count;
}

int db_request_join_group(int user_id, int group_id) {
    if (!conn) return -1;
    
    char user_id_str[32], group_id_str[32];
    sprintf(user_id_str, "%d", user_id);
    sprintf(group_id_str, "%d", group_id);
    
    const char *paramValues[2] = {group_id_str, user_id_str};
    
    // Check if already a member
    PGresult *res = PQexecParams(conn,
        "SELECT member_id FROM group_members WHERE group_id = $1 AND user_id = $2",
        2, NULL, paramValues, NULL, NULL, 0);
    
    if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
        PQclear(res);
        return -2; // Already a member or request exists
    }
    PQclear(res);
    
    // Check if request already exists
    res = PQexecParams(conn,
        "SELECT request_id FROM join_requests WHERE group_id = $1 AND user_id = $2 AND status = 'pending'",
        2, NULL, paramValues, NULL, NULL, 0);
    
    if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
        PQclear(res);
        return -3; // Request already pending
    }
    PQclear(res);
    
    // Create join request
    res = PQexecParams(conn,
        "INSERT INTO join_requests (group_id, user_id, status) VALUES ($1, $2, 'pending') RETURNING request_id",
        2, NULL, paramValues, NULL, NULL, 0);
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "INSERT join_request failed: %s", PQerrorMessage(conn));
        PQclear(res);
        return -1;
    }
    
    int request_id = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    
    return request_id;
}

int db_get_join_requests(int group_id, JoinRequestInfo ***requests) {
    if (!conn) return 0;
    
    char group_id_str[32];
    sprintf(group_id_str, "%d", group_id);
    
    const char *paramValues[1] = {group_id_str};
    
    PGresult *res = PQexecParams(conn,
        "SELECT jr.request_id, jr.group_id, jr.user_id, u.username, u.full_name, jr.status, jr.created_at "
        "FROM join_requests jr "
        "JOIN users u ON jr.user_id = u.user_id "
        "WHERE jr.group_id = $1 AND jr.status = 'pending' "
        "ORDER BY jr.created_at DESC",
        1, NULL, paramValues, NULL, NULL, 0);
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return 0;
    }
    
    int count = PQntuples(res);
    if (count == 0) {
        PQclear(res);
        return 0;
    }
    
    *requests = (JoinRequestInfo**)malloc(count * sizeof(JoinRequestInfo*));
    
    for (int i = 0; i < count; i++) {
        (*requests)[i] = (JoinRequestInfo*)malloc(sizeof(JoinRequestInfo));
        (*requests)[i]->request_id = atoi(PQgetvalue(res, i, 0));
        (*requests)[i]->group_id = atoi(PQgetvalue(res, i, 1));
        (*requests)[i]->user_id = atoi(PQgetvalue(res, i, 2));
        strncpy((*requests)[i]->username, PQgetvalue(res, i, 3), 50);
        (*requests)[i]->username[50] = '\0';
        strncpy((*requests)[i]->full_name, PQgetvalue(res, i, 4), 100);
        (*requests)[i]->full_name[100] = '\0';
        strncpy((*requests)[i]->status, PQgetvalue(res, i, 5), 20);
        (*requests)[i]->status[20] = '\0';
        strncpy((*requests)[i]->created_at, PQgetvalue(res, i, 6), 63);
        (*requests)[i]->created_at[63] = '\0';
    }
    
    PQclear(res);
    return count;
}

JoinRequestInfo* db_get_join_request_by_id(int request_id) {
    if (!conn) return NULL;
    
    char request_id_str[32];
    sprintf(request_id_str, "%d", request_id);
    
    const char *paramValues[1] = {request_id_str};
    
    PGresult *res = PQexecParams(conn,
        "SELECT jr.request_id, jr.group_id, jr.user_id, u.username, u.full_name, jr.status, jr.created_at "
        "FROM join_requests jr "
        "JOIN users u ON jr.user_id = u.user_id "
        "WHERE jr.request_id = $1",
        1, NULL, paramValues, NULL, NULL, 0);
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        PQclear(res);
        return NULL;
    }
    
    JoinRequestInfo *info = (JoinRequestInfo*)malloc(sizeof(JoinRequestInfo));
    info->request_id = atoi(PQgetvalue(res, 0, 0));
    info->group_id = atoi(PQgetvalue(res, 0, 1));
    info->user_id = atoi(PQgetvalue(res, 0, 2));
    strncpy(info->username, PQgetvalue(res, 0, 3), 50);
    info->username[50] = '\0';
    strncpy(info->full_name, PQgetvalue(res, 0, 4), 100);
    info->full_name[100] = '\0';
    strncpy(info->status, PQgetvalue(res, 0, 5), 20);
    info->status[20] = '\0';
    strncpy(info->created_at, PQgetvalue(res, 0, 6), 63);
    info->created_at[63] = '\0';
    
    PQclear(res);
    return info;
}

int db_approve_join_request(int request_id, int reviewer_id, const char *action) {
    if (!conn) return -1;
    
    char request_id_str[32], reviewer_id_str[32];
    sprintf(request_id_str, "%d", request_id);
    sprintf(reviewer_id_str, "%d", reviewer_id);
    
    // Get request info
    JoinRequestInfo *info = db_get_join_request_by_id(request_id);
    if (!info) return -1;
    
    int group_id = info->group_id;
    int user_id = info->user_id;
    
    // Update request status
    const char *status = (strcmp(action, "approve") == 0) ? "approved" : "rejected";
    const char *paramValues[3] = {status, reviewer_id_str, request_id_str};
    
    PGresult *res = PQexecParams(conn,
        "UPDATE join_requests SET status = $1, reviewed_at = CURRENT_TIMESTAMP, reviewed_by = $2 WHERE request_id = $3",
        3, NULL, paramValues, NULL, NULL, 0);
    
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "UPDATE join_request failed: %s", PQerrorMessage(conn));
        PQclear(res);
        free(info);
        return -1;
    }
    PQclear(res);
    
    // If approved, add to group_members
    if (strcmp(action, "approve") == 0) {
        char group_id_str[32], user_id_str[32];
        sprintf(group_id_str, "%d", group_id);
        sprintf(user_id_str, "%d", user_id);
        
        const char *memberParams[2] = {group_id_str, user_id_str};
        
        res = PQexecParams(conn,
            "INSERT INTO group_members (group_id, user_id, role, status) VALUES ($1, $2, 'member', 'approved')",
            2, NULL, memberParams, NULL, NULL, 0);
        PQclear(res);
        
        // Create default permissions
        res = PQexecParams(conn,
            "INSERT INTO permissions (group_id, user_id, can_read, can_write, can_delete, can_manage) "
            "VALUES ($1, $2, TRUE, FALSE, FALSE, FALSE)",
            2, NULL, memberParams, NULL, NULL, 0);
        PQclear(res);
    }
    
    free(info);
    return 0;
}