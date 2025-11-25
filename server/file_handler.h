#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <json-c/json.h>

void handle_create_directory(int client_sock, struct json_object *request);
void handle_rename_directory(int client_sock, struct json_object *request);
void handle_delete_directory(int client_sock, struct json_object *request);
void handle_copy_directory(int client_sock, struct json_object *request);
void handle_move_directory(int client_sock, struct json_object *request);

#endif
