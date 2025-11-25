#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <json-c/json.h>
#include "file_handler.h"
#include "database.h"
#include "auth_handler.h"
#include "../common/protocol.h"

// 14.1 Create directory - All members can create
void handle_create_directory(int sock, struct json_object *request) {
    struct json_object *data_obj, *field;
    
    if (!json_object_object_get_ex(request, "data", &data_obj)) {
        send_error_response(sock, STATUS_BAD_REQUEST, "ERROR_INVALID_REQUEST", "Missing data field");
        return;
    }
    
    const char *session_token = NULL, *directory_name = NULL, *parent_path = NULL;
    int group_id = 0;
    
    if (json_object_object_get_ex(data_obj, "session_token", &field))
        session_token = json_object_get_string(field);
    if (json_object_object_get_ex(data_obj, "group_id", &field))
        group_id = json_object_get_int(field);
    if (json_object_object_get_ex(data_obj, "directory_name", &field))
        directory_name = json_object_get_string(field);
    if (json_object_object_get_ex(data_obj, "parent_path", &field))
        parent_path = json_object_get_string(field);
    
    if (!session_token || !directory_name || !parent_path || group_id == 0) {
        send_error_response(sock, STATUS_BAD_REQUEST, "ERROR_INVALID_REQUEST", "Missing required fields");
        return;
    }
    
    // Verify session
    UserInfo *user = db_verify_session(session_token);
    if (!user) {
        send_error_response(sock, STATUS_UNAUTHORIZED, "ERROR_UNAUTHORIZED", "Invalid or expired session");
        return;
    }
    
    int user_id = user->user_id;
    free(user);
    
    // Check if user is member of the group
    if (!db_is_group_member(user_id, group_id)) {
        send_error_response(sock, STATUS_FORBIDDEN, "ERROR_FORBIDDEN", "You are not a member of this group");
        return;
    }
    
    // Create directory
    int directory_id = db_create_directory(group_id, directory_name, parent_path, user_id);
    if (directory_id < 0) {
        send_error_response(sock, STATUS_INTERNAL_ERROR, "ERROR_INTERNAL_SERVER", "Failed to create directory");
        return;
    }
    
    // Get directory info to return
    DirectoryInfo *dir = db_get_directory_by_id(directory_id);
    if (!dir) {
        send_error_response(sock, STATUS_INTERNAL_ERROR, "ERROR_INTERNAL_SERVER", "Failed to retrieve directory info");
        return;
    }
    
    struct json_object *response = json_object_new_object();
    json_object_object_add(response, "status", json_object_new_int(STATUS_CREATED));
    json_object_object_add(response, "code", json_object_new_string("SUCCESS_CREATE_DIRECTORY"));
    json_object_object_add(response, "message", json_object_new_string("Directory created successfully"));
    
    struct json_object *payload = json_object_new_object();
    json_object_object_add(payload, "directory_id", json_object_new_int(dir->directory_id));
    json_object_object_add(payload, "directory_name", json_object_new_string(dir->directory_name));
    json_object_object_add(payload, "directory_path", json_object_new_string(dir->directory_path));
    json_object_object_add(payload, "created_at", json_object_new_string(dir->created_at));
    
    json_object_object_add(response, "payload", payload);
    
    const char *response_str = json_object_to_json_string(response);
    send(sock, response_str, strlen(response_str), 0);
    
    json_object_put(response);
    free(dir);
}

// 14.2 Rename directory - Admin only
void handle_rename_directory(int sock, struct json_object *request) {
    struct json_object *data_obj, *field;
    
    if (!json_object_object_get_ex(request, "data", &data_obj)) {
        send_error_response(sock, STATUS_BAD_REQUEST, "ERROR_INVALID_REQUEST", "Missing data field");
        return;
    }
    
    const char *session_token = NULL, *new_name = NULL;
    int directory_id = 0;
    
    if (json_object_object_get_ex(data_obj, "session_token", &field))
        session_token = json_object_get_string(field);
    if (json_object_object_get_ex(data_obj, "directory_id", &field))
        directory_id = json_object_get_int(field);
    if (json_object_object_get_ex(data_obj, "new_name", &field))
        new_name = json_object_get_string(field);
    
    if (!session_token || directory_id == 0 || !new_name) {
        send_error_response(sock, STATUS_BAD_REQUEST, "ERROR_INVALID_REQUEST", "Missing required fields");
        return;
    }
    
    // Verify session
    UserInfo *user = db_verify_session(session_token);
    if (!user) {
        send_error_response(sock, STATUS_UNAUTHORIZED, "ERROR_UNAUTHORIZED", "Invalid or expired session");
        return;
    }
    
    int user_id = user->user_id;
    free(user);
    
    // Get directory info
    DirectoryInfo *old_dir = db_get_directory_by_id(directory_id);
    if (!old_dir) {
        send_error_response(sock, STATUS_NOT_FOUND, "ERROR_NOT_FOUND", "Directory not found");
        return;
    }
    
    // Check if user is admin
    if (!db_is_group_admin(user_id, old_dir->group_id)) {
        free(old_dir);
        send_error_response(sock, STATUS_FORBIDDEN, "ERROR_FORBIDDEN", "Only group admin can rename directories");
        return;
    }
    
    char old_name[256], old_path[512];
    strcpy(old_name, old_dir->directory_name);
    strcpy(old_path, old_dir->directory_path);
    free(old_dir);
    
    // Rename directory
    if (db_rename_directory(directory_id, new_name) != 0) {
        send_error_response(sock, STATUS_INTERNAL_ERROR, "ERROR_INTERNAL_SERVER", "Failed to rename directory");
        return;
    }
    
    // Get updated directory info
    DirectoryInfo *new_dir = db_get_directory_by_id(directory_id);
    if (!new_dir) {
        send_error_response(sock, STATUS_INTERNAL_ERROR, "ERROR_INTERNAL_SERVER", "Failed to retrieve directory info");
        return;
    }
    
    time_t now = time(NULL);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
    
    struct json_object *response = json_object_new_object();
    json_object_object_add(response, "status", json_object_new_int(STATUS_OK));
    json_object_object_add(response, "code", json_object_new_string("SUCCESS_RENAME_DIRECTORY"));
    json_object_object_add(response, "message", json_object_new_string("Directory renamed successfully"));
    
    struct json_object *payload = json_object_new_object();
    json_object_object_add(payload, "directory_id", json_object_new_int(directory_id));
    json_object_object_add(payload, "old_name", json_object_new_string(old_name));
    json_object_object_add(payload, "new_name", json_object_new_string(new_dir->directory_name));
    json_object_object_add(payload, "old_path", json_object_new_string(old_path));
    json_object_object_add(payload, "new_path", json_object_new_string(new_dir->directory_path));
    json_object_object_add(payload, "updated_at", json_object_new_string(timestamp));
    
    json_object_object_add(response, "payload", payload);
    
    const char *response_str = json_object_to_json_string(response);
    send(sock, response_str, strlen(response_str), 0);
    
    json_object_put(response);
    free(new_dir);
}

// 14.3 Delete directory - Admin only
void handle_delete_directory(int sock, struct json_object *request) {
    struct json_object *data_obj, *field;
    
    if (!json_object_object_get_ex(request, "data", &data_obj)) {
        send_error_response(sock, STATUS_BAD_REQUEST, "ERROR_INVALID_REQUEST", "Missing data field");
        return;
    }
    
    const char *session_token = NULL;
    int directory_id = 0, recursive = 0;
    
    if (json_object_object_get_ex(data_obj, "session_token", &field))
        session_token = json_object_get_string(field);
    if (json_object_object_get_ex(data_obj, "directory_id", &field))
        directory_id = json_object_get_int(field);
    if (json_object_object_get_ex(data_obj, "recursive", &field))
        recursive = json_object_get_boolean(field);
    
    if (!session_token || directory_id == 0) {
        send_error_response(sock, STATUS_BAD_REQUEST, "ERROR_INVALID_REQUEST", "Missing required fields");
        return;
    }
    
    // Verify session
    UserInfo *user = db_verify_session(session_token);
    if (!user) {
        send_error_response(sock, STATUS_UNAUTHORIZED, "ERROR_UNAUTHORIZED", "Invalid or expired session");
        return;
    }
    
    int user_id = user->user_id;
    free(user);
    
    // Get directory info
    DirectoryInfo *dir = db_get_directory_by_id(directory_id);
    if (!dir) {
        send_error_response(sock, STATUS_NOT_FOUND, "ERROR_NOT_FOUND", "Directory not found");
        return;
    }
    
    // Check if user is admin
    if (!db_is_group_admin(user_id, dir->group_id)) {
        free(dir);
        send_error_response(sock, STATUS_FORBIDDEN, "ERROR_FORBIDDEN", "Only group admin can delete directories");
        return;
    }
    
    free(dir);
    
    // Delete directory
    int deleted_files = 0, deleted_subdirs = 0;
    if (db_delete_directory(directory_id, &deleted_files, &deleted_subdirs) != 0) {
        send_error_response(sock, STATUS_INTERNAL_ERROR, "ERROR_INTERNAL_SERVER", "Failed to delete directory");
        return;
    }
    
    time_t now = time(NULL);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
    
    struct json_object *response = json_object_new_object();
    json_object_object_add(response, "status", json_object_new_int(STATUS_OK));
    json_object_object_add(response, "code", json_object_new_string("SUCCESS_DELETE_DIRECTORY"));
    json_object_object_add(response, "message", json_object_new_string("Directory deleted successfully"));
    
    struct json_object *payload = json_object_new_object();
    json_object_object_add(payload, "directory_id", json_object_new_int(directory_id));
    json_object_object_add(payload, "deleted_files", json_object_new_int(deleted_files));
    json_object_object_add(payload, "deleted_subdirectories", json_object_new_int(deleted_subdirs));
    json_object_object_add(payload, "deleted_at", json_object_new_string(timestamp));
    
    json_object_object_add(response, "payload", payload);
    
    const char *response_str = json_object_to_json_string(response);
    send(sock, response_str, strlen(response_str), 0);
    
    json_object_put(response);
}

// 14.4 Copy directory - Admin only
void handle_copy_directory(int sock, struct json_object *request) {
    struct json_object *data_obj, *field;
    
    if (!json_object_object_get_ex(request, "data", &data_obj)) {
        send_error_response(sock, STATUS_BAD_REQUEST, "ERROR_INVALID_REQUEST", "Missing data field");
        return;
    }
    
    const char *session_token = NULL, *destination_path = NULL;
    int directory_id = 0;
    
    if (json_object_object_get_ex(data_obj, "session_token", &field))
        session_token = json_object_get_string(field);
    if (json_object_object_get_ex(data_obj, "directory_id", &field))
        directory_id = json_object_get_int(field);
    if (json_object_object_get_ex(data_obj, "destination_path", &field))
        destination_path = json_object_get_string(field);
    
    if (!session_token || directory_id == 0 || !destination_path) {
        send_error_response(sock, STATUS_BAD_REQUEST, "ERROR_INVALID_REQUEST", "Missing required fields");
        return;
    }
    
    // Verify session
    UserInfo *user = db_verify_session(session_token);
    if (!user) {
        send_error_response(sock, STATUS_UNAUTHORIZED, "ERROR_UNAUTHORIZED", "Invalid or expired session");
        return;
    }
    
    int user_id = user->user_id;
    free(user);
    
    // Get directory info
    DirectoryInfo *dir = db_get_directory_by_id(directory_id);
    if (!dir) {
        send_error_response(sock, STATUS_NOT_FOUND, "ERROR_NOT_FOUND", "Directory not found");
        return;
    }
    
    // Check if user is admin
    if (!db_is_group_admin(user_id, dir->group_id)) {
        free(dir);
        send_error_response(sock, STATUS_FORBIDDEN, "ERROR_FORBIDDEN", "Only group admin can copy directories");
        return;
    }
    
    free(dir);
    
    // Copy directory
    int new_dir_id = db_copy_directory(directory_id, destination_path, user_id);
    if (new_dir_id < 0) {
        send_error_response(sock, STATUS_INTERNAL_ERROR, "ERROR_INTERNAL_SERVER", "Failed to copy directory");
        return;
    }
    
    // Get new directory info
    DirectoryInfo *new_dir = db_get_directory_by_id(new_dir_id);
    if (!new_dir) {
        send_error_response(sock, STATUS_INTERNAL_ERROR, "ERROR_INTERNAL_SERVER", "Failed to retrieve directory info");
        return;
    }
    
    time_t now = time(NULL);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
    
    struct json_object *response = json_object_new_object();
    json_object_object_add(response, "status", json_object_new_int(STATUS_OK));
    json_object_object_add(response, "code", json_object_new_string("SUCCESS_COPY_DIRECTORY"));
    json_object_object_add(response, "message", json_object_new_string("Directory copied successfully"));
    
    struct json_object *payload = json_object_new_object();
    json_object_object_add(payload, "source_directory_id", json_object_new_int(directory_id));
    json_object_object_add(payload, "new_directory_id", json_object_new_int(new_dir_id));
    json_object_object_add(payload, "new_directory_path", json_object_new_string(new_dir->directory_path));
    json_object_object_add(payload, "copied_files", json_object_new_int(0));
    json_object_object_add(payload, "copied_subdirectories", json_object_new_int(0));
    json_object_object_add(payload, "copied_at", json_object_new_string(timestamp));
    
    json_object_object_add(response, "payload", payload);
    
    const char *response_str = json_object_to_json_string(response);
    send(sock, response_str, strlen(response_str), 0);
    
    json_object_put(response);
    free(new_dir);
}

// 14.5 Move directory - Admin only
void handle_move_directory(int sock, struct json_object *request) {
    struct json_object *data_obj, *field;
    
    if (!json_object_object_get_ex(request, "data", &data_obj)) {
        send_error_response(sock, STATUS_BAD_REQUEST, "ERROR_INVALID_REQUEST", "Missing data field");
        return;
    }
    
    const char *session_token = NULL, *destination_path = NULL;
    int directory_id = 0;
    
    if (json_object_object_get_ex(data_obj, "session_token", &field))
        session_token = json_object_get_string(field);
    if (json_object_object_get_ex(data_obj, "directory_id", &field))
        directory_id = json_object_get_int(field);
    if (json_object_object_get_ex(data_obj, "destination_path", &field))
        destination_path = json_object_get_string(field);
    
    if (!session_token || directory_id == 0 || !destination_path) {
        send_error_response(sock, STATUS_BAD_REQUEST, "ERROR_INVALID_REQUEST", "Missing required fields");
        return;
    }
    
    // Verify session
    UserInfo *user = db_verify_session(session_token);
    if (!user) {
        send_error_response(sock, STATUS_UNAUTHORIZED, "ERROR_UNAUTHORIZED", "Invalid or expired session");
        return;
    }
    
    int user_id = user->user_id;
    free(user);
    
    // Get directory info
    DirectoryInfo *old_dir = db_get_directory_by_id(directory_id);
    if (!old_dir) {
        send_error_response(sock, STATUS_NOT_FOUND, "ERROR_NOT_FOUND", "Directory not found");
        return;
    }
    
    // Check if user is admin
    if (!db_is_group_admin(user_id, old_dir->group_id)) {
        free(old_dir);
        send_error_response(sock, STATUS_FORBIDDEN, "ERROR_FORBIDDEN", "Only group admin can move directories");
        return;
    }
    
    char old_path[512];
    strcpy(old_path, old_dir->directory_path);
    free(old_dir);
    
    // Move directory
    int affected_files = 0, affected_subdirs = 0;
    if (db_move_directory(directory_id, destination_path, &affected_files, &affected_subdirs) != 0) {
        send_error_response(sock, STATUS_INTERNAL_ERROR, "ERROR_INTERNAL_SERVER", "Failed to move directory");
        return;
    }
    
    // Get updated directory info
    DirectoryInfo *new_dir = db_get_directory_by_id(directory_id);
    if (!new_dir) {
        send_error_response(sock, STATUS_INTERNAL_ERROR, "ERROR_INTERNAL_SERVER", "Failed to retrieve directory info");
        return;
    }
    
    time_t now = time(NULL);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
    
    struct json_object *response = json_object_new_object();
    json_object_object_add(response, "status", json_object_new_int(STATUS_OK));
    json_object_object_add(response, "code", json_object_new_string("SUCCESS_MOVE_DIRECTORY"));
    json_object_object_add(response, "message", json_object_new_string("Directory moved successfully"));
    
    struct json_object *payload = json_object_new_object();
    json_object_object_add(payload, "directory_id", json_object_new_int(directory_id));
    json_object_object_add(payload, "old_path", json_object_new_string(old_path));
    json_object_object_add(payload, "new_path", json_object_new_string(new_dir->directory_path));
    json_object_object_add(payload, "affected_files", json_object_new_int(affected_files));
    json_object_object_add(payload, "affected_subdirectories", json_object_new_int(affected_subdirs));
    json_object_object_add(payload, "moved_at", json_object_new_string(timestamp));
    
    json_object_object_add(response, "payload", payload);
    
    const char *response_str = json_object_to_json_string(response);
    send(sock, response_str, strlen(response_str), 0);
    
    json_object_put(response);
    free(new_dir);
}
