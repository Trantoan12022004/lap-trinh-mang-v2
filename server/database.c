#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>
#include "database.h"

static PGconn *conn = NULL;

int init_database() {
    const char *conninfo = "host=localhost dbname=fileshare user=postgres password=your_password";
    
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