#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <json-c/json.h>
#include "group_handler.h"
#include "database.h"
#include "auth_handler.h"
#include "../common/protocol.h"

void handle_create_group(int sock, struct json_object *request) {
    struct json_object *data_obj, *field;
    
    if (!json_object_object_get_ex(request, "data", &data_obj)) {
        send_error_response(sock, STATUS_BAD_REQUEST, "ERROR_INVALID_REQUEST", "Missing data field");
        return;
    }
    
    const char *session_token = NULL, *group_name = NULL, *description = NULL;
    
    if (json_object_object_get_ex(data_obj, "session_token", &field))
        session_token = json_object_get_string(field);
    if (json_object_object_get_ex(data_obj, "group_name", &field))
        group_name = json_object_get_string(field);
    if (json_object_object_get_ex(data_obj, "description", &field))
        description = json_object_get_string(field);
    
    if (!session_token || !group_name) {
        send_error_response(sock, STATUS_BAD_REQUEST, "ERROR_INVALID_REQUEST", "Missing required fields");
        return;
    }
    
    // Verify session
    UserInfo *user = db_verify_session(session_token);
    if (!user) {
        send_error_response(sock, STATUS_UNAUTHORIZED, "ERROR_UNAUTHORIZED", "Invalid session token or session expired");
        return;
    }
    
    // Create group
    int group_id = db_create_group(user->user_id, group_name, description);
    if (group_id <= 0) {
        free(user);
        send_error_response(sock, STATUS_INTERNAL_ERROR, "ERROR_INTERNAL_SERVER", "Failed to create group");
        return;
    }
    
    // Get current timestamp
    time_t now = time(NULL);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
    
    // Send success response
    struct json_object *response = json_object_new_object();
    json_object_object_add(response, "status", json_object_new_int(STATUS_CREATED));
    json_object_object_add(response, "code", json_object_new_string("SUCCESS_CREATE_GROUP"));
    json_object_object_add(response, "message", json_object_new_string("Group created successfully"));
    
    struct json_object *payload = json_object_new_object();
    json_object_object_add(payload, "group_id", json_object_new_int(group_id));
    json_object_object_add(payload, "group_name", json_object_new_string(group_name));
    json_object_object_add(payload, "description", json_object_new_string(description ? description : ""));
    json_object_object_add(payload, "owner_id", json_object_new_int(user->user_id));
    json_object_object_add(payload, "created_at", json_object_new_string(timestamp));
    json_object_object_add(response, "payload", payload);
    
    const char *json_str = json_object_to_json_string(response);
    send(sock, json_str, strlen(json_str), 0);
    
    free(user);
    json_object_put(response);
}

void handle_list_my_groups(int sock, struct json_object *request) {
    struct json_object *data_obj, *field;
    
    if (!json_object_object_get_ex(request, "data", &data_obj)) {
        send_error_response(sock, STATUS_BAD_REQUEST, "ERROR_INVALID_REQUEST", "Missing data field");
        return;
    }
    
    const char *session_token = NULL;
    
    if (json_object_object_get_ex(data_obj, "session_token", &field))
        session_token = json_object_get_string(field);
    
    if (!session_token) {
        send_error_response(sock, STATUS_BAD_REQUEST, "ERROR_INVALID_REQUEST", "Missing session_token");
        return;
    }
    
    // Verify session
    UserInfo *user = db_verify_session(session_token);
    if (!user) {
        send_error_response(sock, STATUS_UNAUTHORIZED, "ERROR_UNAUTHORIZED", "Invalid session token or session expired");
        return;
    }
    
    // Get user's groups
    GroupInfo **groups = NULL;
    int count = db_get_user_groups(user->user_id, &groups);
    
    // Send success response
    struct json_object *response = json_object_new_object();
    json_object_object_add(response, "status", json_object_new_int(STATUS_OK));
    json_object_object_add(response, "code", json_object_new_string("SUCCESS_LIST_GROUPS"));
    json_object_object_add(response, "message", json_object_new_string("Groups retrieved successfully"));
    
    struct json_object *payload = json_object_new_object();
    struct json_object *groups_array = json_object_new_array();
    
    for (int i = 0; i < count; i++) {
        struct json_object *group_obj = json_object_new_object();
        json_object_object_add(group_obj, "group_id", json_object_new_int(groups[i]->group_id));
        json_object_object_add(group_obj, "group_name", json_object_new_string(groups[i]->group_name));
        json_object_object_add(group_obj, "description", json_object_new_string(groups[i]->description));
        json_object_object_add(group_obj, "role", json_object_new_string(groups[i]->role));
        json_object_object_add(group_obj, "member_count", json_object_new_int(groups[i]->member_count));
        json_object_object_add(group_obj, "created_at", json_object_new_string(groups[i]->created_at));
        json_object_array_add(groups_array, group_obj);
        free(groups[i]);
    }
    if (groups) free(groups);
    
    json_object_object_add(payload, "groups", groups_array);
    json_object_object_add(response, "payload", payload);
    
    const char *json_str = json_object_to_json_string(response);
    send(sock, json_str, strlen(json_str), 0);
    
    free(user);
    json_object_put(response);
}

void handle_list_group_members(int sock, struct json_object *request) {
    struct json_object *data_obj, *field;
    
    if (!json_object_object_get_ex(request, "data", &data_obj)) {
        send_error_response(sock, STATUS_BAD_REQUEST, "ERROR_INVALID_REQUEST", "Missing data field");
        return;
    }
    
    const char *session_token = NULL;
    int group_id = 0;
    
    if (json_object_object_get_ex(data_obj, "session_token", &field))
        session_token = json_object_get_string(field);
    if (json_object_object_get_ex(data_obj, "group_id", &field))
        group_id = json_object_get_int(field);
    
    if (!session_token || group_id <= 0) {
        send_error_response(sock, STATUS_BAD_REQUEST, "ERROR_INVALID_REQUEST", "Missing required fields");
        return;
    }
    
    // Verify session
    UserInfo *user = db_verify_session(session_token);
    if (!user) {
        send_error_response(sock, STATUS_UNAUTHORIZED, "ERROR_UNAUTHORIZED", "Invalid session token or session expired");
        return;
    }
    
    // Check if user is member of the group
    if (!db_is_group_member(user->user_id, group_id)) {
        free(user);
        send_error_response(sock, STATUS_FORBIDDEN, "ERROR_FORBIDDEN", "You are not a member of this group");
        return;
    }
    
    // Get group members
    MemberInfo **members = NULL;
    int count = db_get_group_members(group_id, &members);
    
    // Send success response
    struct json_object *response = json_object_new_object();
    json_object_object_add(response, "status", json_object_new_int(STATUS_OK));
    json_object_object_add(response, "code", json_object_new_string("SUCCESS_LIST_MEMBERS"));
    json_object_object_add(response, "message", json_object_new_string("Members retrieved successfully"));
    
    struct json_object *payload = json_object_new_object();
    json_object_object_add(payload, "group_id", json_object_new_int(group_id));
    
    struct json_object *members_array = json_object_new_array();
    
    for (int i = 0; i < count; i++) {
        struct json_object *member_obj = json_object_new_object();
        json_object_object_add(member_obj, "user_id", json_object_new_int(members[i]->user_id));
        json_object_object_add(member_obj, "username", json_object_new_string(members[i]->username));
        json_object_object_add(member_obj, "full_name", json_object_new_string(members[i]->full_name));
        json_object_object_add(member_obj, "role", json_object_new_string(members[i]->role));
        json_object_object_add(member_obj, "status", json_object_new_string(members[i]->status));
        json_object_object_add(member_obj, "joined_at", json_object_new_string(members[i]->joined_at));
        json_object_array_add(members_array, member_obj);
        free(members[i]);
    }
    if (members) free(members);
    
    json_object_object_add(payload, "members", members_array);
    json_object_object_add(response, "payload", payload);
    
    const char *json_str = json_object_to_json_string(response);
    send(sock, json_str, strlen(json_str), 0);
    
    free(user);
    json_object_put(response);
}
