#ifndef GROUP_HANDLER_H
#define GROUP_HANDLER_H

#include <json-c/json.h>

void handle_create_group(int sock, struct json_object *request);
void handle_list_my_groups(int sock, struct json_object *request);
void handle_list_group_members(int sock, struct json_object *request);
void handle_request_join_group(int sock, struct json_object *request);
void handle_list_join_requests(int sock, struct json_object *request);
void handle_approve_join_request(int sock, struct json_object *request);

#endif
