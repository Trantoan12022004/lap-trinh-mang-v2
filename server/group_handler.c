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

void handle_request_join_group(int sock, struct json_object *request) {
    struct json_object *data, *token_obj, *group_id_obj;
    
    if (!json_object_object_get_ex(request, "data", &data) ||
        !json_object_object_get_ex(data, "session_token", &token_obj) ||
        !json_object_object_get_ex(data, "group_id", &group_id_obj)) {
        send_error_response(sock, STATUS_BAD_REQUEST, "ERROR_INVALID_REQUEST", "Missing required fields");
        return;
    }
    
    const char *token = json_object_get_string(token_obj);
    int group_id = json_object_get_int(group_id_obj);
    
    // Verify session
    UserInfo *user = db_verify_session(token);
    if (!user) {
        send_error_response(sock, STATUS_UNAUTHORIZED, "ERROR_UNAUTHORIZED", "Invalid or expired session");
        return;
    }
    
    // Check if already member
    if (db_is_group_member(user->user_id, group_id)) {
        send_error_response(sock, STATUS_CONFLICT, "ERROR_CONFLICT", "Already a member of this group");
        free(user);
        return;
    }
    
    // Send join request
    int request_id = db_request_join_group(user->user_id, group_id);
    if (request_id == -2) {
        send_error_response(sock, STATUS_CONFLICT, "ERROR_CONFLICT", "Already a member");
        free(user);
        return;
    } else if (request_id == -3) {
        send_error_response(sock, STATUS_CONFLICT, "ERROR_CONFLICT", "Join request already pending");
        free(user);
        return;
    } else if (request_id < 0) {
        send_error_response(sock, STATUS_INTERNAL_ERROR, "ERROR_INTERNAL_SERVER", "Failed to create join request");
        free(user);
        return;
    }
    
    // Get current time
    time_t now = time(NULL);
    struct tm *tm_info = gmtime(&now);
    char created_at[64];
    strftime(created_at, sizeof(created_at), "%Y-%m-%dT%H:%M:%SZ", tm_info);
    
    struct json_object *response = json_object_new_object();
    json_object_object_add(response, "status", json_object_new_int(STATUS_CREATED));
    json_object_object_add(response, "code", json_object_new_string("SUCCESS_REQUEST_JOIN"));
    json_object_object_add(response, "message", json_object_new_string("Join request sent successfully"));
    
    struct json_object *payload = json_object_new_object();
    json_object_object_add(payload, "request_id", json_object_new_int(request_id));
    json_object_object_add(payload, "group_id", json_object_new_int(group_id));
    json_object_object_add(payload, "user_id", json_object_new_int(user->user_id));
    json_object_object_add(payload, "status", json_object_new_string("pending"));
    json_object_object_add(payload, "created_at", json_object_new_string(created_at));
    json_object_object_add(response, "payload", payload);
    
    const char *json_str = json_object_to_json_string(response);
    send(sock, json_str, strlen(json_str), 0);
    
    free(user);
    json_object_put(response);
}

void handle_list_join_requests(int sock, struct json_object *request) {
    struct json_object *data, *token_obj, *group_id_obj;
    
    if (!json_object_object_get_ex(request, "data", &data) ||
        !json_object_object_get_ex(data, "session_token", &token_obj) ||
        !json_object_object_get_ex(data, "group_id", &group_id_obj)) {
        send_error_response(sock, STATUS_BAD_REQUEST, "ERROR_INVALID_REQUEST", "Missing required fields");
        return;
    }
    
    const char *token = json_object_get_string(token_obj);
    int group_id = json_object_get_int(group_id_obj);
    
    // Verify session
    UserInfo *user = db_verify_session(token);
    if (!user) {
        send_error_response(sock, STATUS_UNAUTHORIZED, "ERROR_UNAUTHORIZED", "Invalid or expired session");
        return;
    }
    
    // Check if user is admin
    if (!db_is_group_admin(user->user_id, group_id)) {
        send_error_response(sock, STATUS_FORBIDDEN, "ERROR_FORBIDDEN", "Only admins can view join requests");
        free(user);
        return;
    }
    
    // Get join requests
    JoinRequestInfo **requests = NULL;
    int count = db_get_join_requests(group_id, &requests);
    
    struct json_object *response = json_object_new_object();
    json_object_object_add(response, "status", json_object_new_int(STATUS_OK));
    json_object_object_add(response, "code", json_object_new_string("SUCCESS_LIST_REQUESTS"));
    json_object_object_add(response, "message", json_object_new_string("Join requests retrieved successfully"));
    
    struct json_object *payload = json_object_new_object();
    json_object_object_add(payload, "group_id", json_object_new_int(group_id));
    
    struct json_object *requests_array = json_object_new_array();
    for (int i = 0; i < count; i++) {
        struct json_object *req_obj = json_object_new_object();
        json_object_object_add(req_obj, "request_id", json_object_new_int(requests[i]->request_id));
        json_object_object_add(req_obj, "user_id", json_object_new_int(requests[i]->user_id));
        json_object_object_add(req_obj, "username", json_object_new_string(requests[i]->username));
        json_object_object_add(req_obj, "full_name", json_object_new_string(requests[i]->full_name));
        json_object_object_add(req_obj, "status", json_object_new_string(requests[i]->status));
        json_object_object_add(req_obj, "created_at", json_object_new_string(requests[i]->created_at));
        json_object_array_add(requests_array, req_obj);
        free(requests[i]);
    }
    if (requests) free(requests);
    
    json_object_object_add(payload, "requests", requests_array);
    json_object_object_add(response, "payload", payload);
    
    const char *json_str = json_object_to_json_string(response);
    send(sock, json_str, strlen(json_str), 0);
    
    free(user);
    json_object_put(response);
}

void handle_approve_join_request(int sock, struct json_object *request) {
    struct json_object *data, *token_obj, *request_id_obj, *action_obj;
    
    if (!json_object_object_get_ex(request, "data", &data) ||
        !json_object_object_get_ex(data, "session_token", &token_obj) ||
        !json_object_object_get_ex(data, "request_id", &request_id_obj) ||
        !json_object_object_get_ex(data, "action", &action_obj)) {
        send_error_response(sock, STATUS_BAD_REQUEST, "ERROR_INVALID_REQUEST", "Missing required fields");
        return;
    }
    
    const char *token = json_object_get_string(token_obj);
    int request_id = json_object_get_int(request_id_obj);
    const char *action = json_object_get_string(action_obj);
    
    // Verify session
    UserInfo *user = db_verify_session(token);
    if (!user) {
        send_error_response(sock, STATUS_UNAUTHORIZED, "ERROR_UNAUTHORIZED", "Invalid or expired session");
        return;
    }
    
    // Get request info to check group
    JoinRequestInfo *req_info = db_get_join_request_by_id(request_id);
    if (!req_info) {
        send_error_response(sock, STATUS_NOT_FOUND, "ERROR_NOT_FOUND", "Join request not found");
        free(user);
        return;
    }
    
    // Check if user is admin
    if (!db_is_group_admin(user->user_id, req_info->group_id)) {
        send_error_response(sock, STATUS_FORBIDDEN, "ERROR_FORBIDDEN", "Only admins can approve/reject requests");
        free(user);
        free(req_info);
        return;
    }
    
    // Approve or reject
    if (db_approve_join_request(request_id, user->user_id, action) < 0) {
        send_error_response(sock, STATUS_INTERNAL_ERROR, "ERROR_INTERNAL_SERVER", "Failed to process request");
        free(user);
        free(req_info);
        return;
    }
    
    // Get current time
    time_t now = time(NULL);
    struct tm *tm_info = gmtime(&now);
    char reviewed_at[64];
    strftime(reviewed_at, sizeof(reviewed_at), "%Y-%m-%dT%H:%M:%SZ", tm_info);
    
    struct json_object *response = json_object_new_object();
    json_object_object_add(response, "status", json_object_new_int(STATUS_OK));
    json_object_object_add(response, "code", json_object_new_string(
        strcmp(action, "approve") == 0 ? "SUCCESS_APPROVE_REQUEST" : "SUCCESS_REJECT_REQUEST"));
    json_object_object_add(response, "message", json_object_new_string(
        strcmp(action, "approve") == 0 ? "Join request approved successfully" : "Join request rejected successfully"));
    
    struct json_object *payload = json_object_new_object();
    json_object_object_add(payload, "request_id", json_object_new_int(request_id));
    json_object_object_add(payload, "user_id", json_object_new_int(req_info->user_id));
    json_object_object_add(payload, "group_id", json_object_new_int(req_info->group_id));
    json_object_object_add(payload, "status", json_object_new_string(
        strcmp(action, "approve") == 0 ? "approved" : "rejected"));
    json_object_object_add(payload, "reviewed_at", json_object_new_string(reviewed_at));
    json_object_object_add(response, "payload", payload);
    
    const char *json_str = json_object_to_json_string(response);
    send(sock, json_str, strlen(json_str), 0);
    
    free(user);
    free(req_info);
    json_object_put(response);
}
