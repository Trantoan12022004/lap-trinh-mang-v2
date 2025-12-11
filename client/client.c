#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <json-c/json.h>
#include "../common/protocol.h"

// Global session storage
char g_session_token[MAX_TOKEN] = "";
int g_user_id = 0;
char g_username[MAX_USERNAME] = "";

void clear_screen() {
    printf("\033[2J\033[H");
}

void print_separator() {
    printf("========================================\n");
}

void print_success(const char *message) {
    printf("✓ %s\n", message);
}

void print_error(const char *message) {
    printf("✗ %s\n", message);
}

void parse_and_display_response(const char *json_str) {
    struct json_object *response = json_tokener_parse(json_str);
    if (!response) {
        printf("\n[RAW RESPONSE]\n%s\n", json_str);
        return;
    }
    
    struct json_object *status_obj, *code_obj, *message_obj, *payload_obj;
    
    json_object_object_get_ex(response, "status", &status_obj);
    json_object_object_get_ex(response, "code", &code_obj);
    json_object_object_get_ex(response, "message", &message_obj);
    json_object_object_get_ex(response, "payload", &payload_obj);
    
    int status = json_object_get_int(status_obj);
    const char *code = json_object_get_string(code_obj);
    const char *message = json_object_get_string(message_obj);
    
    print_separator();
    if (status == 200 || status == 201) {
        print_success(message);
    } else {
        print_error(message);
    }
    printf("Status: %d | Code: %s\n", status, code);
    print_separator();
    
    if (payload_obj) {
        // Parse specific responses
        if (strcmp(code, "SUCCESS_LOGIN") == 0) {
            struct json_object *token_obj, *user_id_obj, *username_obj;
            json_object_object_get_ex(payload_obj, "session_token", &token_obj);
            json_object_object_get_ex(payload_obj, "user_id", &user_id_obj);
            json_object_object_get_ex(payload_obj, "username", &username_obj);
            
            if (token_obj) {
                strncpy(g_session_token, json_object_get_string(token_obj), MAX_TOKEN - 1);
                g_user_id = json_object_get_int(user_id_obj);
                strncpy(g_username, json_object_get_string(username_obj), MAX_USERNAME - 1);
                printf("✓ Session saved! You are now logged in as: %s\n", g_username);
            }
        } else if (strcmp(code, "SUCCESS_LIST_GROUPS") == 0) {
            struct json_object *groups_obj;
            json_object_object_get_ex(payload_obj, "groups", &groups_obj);
            int count = json_object_array_length(groups_obj);
            
            printf("\nYour Groups (%d):\n", count);
            for (int i = 0; i < count; i++) {
                struct json_object *group = json_object_array_get_idx(groups_obj, i);
                struct json_object *id, *name, *role, *members;
                json_object_object_get_ex(group, "group_id", &id);
                json_object_object_get_ex(group, "group_name", &name);
                json_object_object_get_ex(group, "role", &role);
                json_object_object_get_ex(group, "member_count", &members);
                
                printf("  [%d] %s (Role: %s, Members: %d)\n", 
                       json_object_get_int(id),
                       json_object_get_string(name),
                       json_object_get_string(role),
                       json_object_get_int(members));
            }
        } else if (strcmp(code, "SUCCESS_LIST_MEMBERS") == 0) {
            struct json_object *members_obj;
            json_object_object_get_ex(payload_obj, "members", &members_obj);
            int count = json_object_array_length(members_obj);
            
            printf("\nGroup Members (%d):\n", count);
            for (int i = 0; i < count; i++) {
                struct json_object *member = json_object_array_get_idx(members_obj, i);
                struct json_object *id, *username, *name, *role;
                json_object_object_get_ex(member, "user_id", &id);
                json_object_object_get_ex(member, "username", &username);
                json_object_object_get_ex(member, "full_name", &name);
                json_object_object_get_ex(member, "role", &role);
                
                printf("  [ID:%d] %s (%s) - %s\n", 
                       json_object_get_int(id),
                       json_object_get_string(username),
                       json_object_get_string(name),
                       json_object_get_string(role));
            }
        } else if (strcmp(code, "SUCCESS_LIST_REQUESTS") == 0) {
            struct json_object *requests_obj;
            json_object_object_get_ex(payload_obj, "requests", &requests_obj);
            int count = json_object_array_length(requests_obj);
            
            printf("\nJoin Requests (%d):\n", count);
            for (int i = 0; i < count; i++) {
                struct json_object *req = json_object_array_get_idx(requests_obj, i);
                struct json_object *id, *username, *name, *status;
                json_object_object_get_ex(req, "request_id", &id);
                json_object_object_get_ex(req, "username", &username);
                json_object_object_get_ex(req, "full_name", &name);
                json_object_object_get_ex(req, "status", &status);
                
                printf("  [ReqID:%d] %s (%s) - Status: %s\n", 
                       json_object_get_int(id),
                       json_object_get_string(username),
                       json_object_get_string(name),
                       json_object_get_string(status));
            }
        } else if (strcmp(code, "SUCCESS_LIST_INVITATIONS") == 0) {
            struct json_object *invitations_obj;
            json_object_object_get_ex(payload_obj, "invitations", &invitations_obj);
            int count = json_object_array_length(invitations_obj);
            
            printf("\nYour Invitations (%d):\n", count);
            for (int i = 0; i < count; i++) {
                struct json_object *inv = json_object_array_get_idx(invitations_obj, i);
                struct json_object *id, *group_name, *inviter, *status;
                json_object_object_get_ex(inv, "invitation_id", &id);
                json_object_object_get_ex(inv, "group_name", &group_name);
                json_object_object_get_ex(inv, "inviter_username", &inviter);
                json_object_object_get_ex(inv, "status", &status);
                
                printf("  [InvID:%d] Group: %s (from %s) - Status: %s\n", 
                       json_object_get_int(id),
                       json_object_get_string(group_name),
                       json_object_get_string(inviter),
                       json_object_get_string(status));
            }
        } else {
            // Display raw payload for other responses
            printf("\nDetails:\n%s\n", json_object_to_json_string_ext(payload_obj, JSON_C_TO_STRING_PRETTY));
        }
    }
    
    print_separator();
    json_object_put(response);
}

void wait_for_enter() {
    printf("\nPress ENTER to continue...");
    getchar();
    getchar();
}

int connect_to_server() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("172.18.36.255");
    
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return -1;
    }
    
    printf("Connected to server\n");
    return sock;
}

void send_register_request(int sock) {
    char username[MAX_USERNAME], password[MAX_PASSWORD];
    char email[MAX_EMAIL], full_name[MAX_FULLNAME];
    
    printf("\n=== REGISTER ===\n");
    printf("Username: ");
    scanf("%s", username);
    printf("Password: ");
    scanf("%s", password);
    printf("Email: ");
    scanf("%s", email);
    printf("Full name: ");
    getchar(); // consume newline
    fgets(full_name, MAX_FULLNAME, stdin);
    full_name[strcspn(full_name, "\n")] = 0;
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("REGISTER"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "username", json_object_new_string(username));
    json_object_object_add(data, "password", json_object_new_string(password));
    json_object_object_add(data, "email", json_object_new_string(email));
    json_object_object_add(data, "full_name", json_object_new_string(full_name));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    // Receive response
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    printf("\nResponse:\n%s\n", buffer);
}

void send_login_request(int sock) {
    char username[MAX_USERNAME], password[MAX_PASSWORD];
    
    clear_screen();
    printf("\n=== LOGIN ===\n");
    printf("Username: ");
    scanf("%s", username);
    printf("Password: ");
    scanf("%s", password);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("LOGIN"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "username", json_object_new_string(username));
    json_object_object_add(data, "password", json_object_new_string(password));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    // Receive response
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    parse_and_display_response(buffer);
    wait_for_enter();
}

void send_logout_request(int sock) {
    char token[MAX_TOKEN];
    
    printf("\n=== LOGOUT ===\n");
    printf("Session token: ");
    scanf("%s", token);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("LOGOUT"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(token));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    // Receive response
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    printf("\nResponse:\n%s\n", buffer);
}

void send_verify_session_request(int sock) {
    char token[MAX_TOKEN];
    
    printf("\n=== VERIFY SESSION ===\n");
    printf("Session token: ");
    scanf("%s", token);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("VERIFY_SESSION"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(token));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    // Receive response
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    printf("\nResponse:\n%s\n", buffer);
}

void send_update_profile_request(int sock) {
    char token[MAX_TOKEN], email[MAX_EMAIL], full_name[MAX_FULLNAME];
    
    printf("\n=== UPDATE PROFILE ===\n");
    printf("Session token: ");
    scanf("%s", token);
    printf("New email: ");
    scanf("%s", email);
    printf("New full name: ");
    getchar(); // consume newline
    fgets(full_name, MAX_FULLNAME, stdin);
    full_name[strcspn(full_name, "\n")] = 0;
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("UPDATE_PROFILE"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(token));
    json_object_object_add(data, "email", json_object_new_string(email));
    json_object_object_add(data, "full_name", json_object_new_string(full_name));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    // Receive response
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    printf("\nResponse:\n%s\n", buffer);
}

void send_change_password_request(int sock) {
    char token[MAX_TOKEN], old_pass[MAX_PASSWORD], new_pass[MAX_PASSWORD];
    
    printf("\n=== CHANGE PASSWORD ===\n");
    printf("Session token: ");
    scanf("%s", token);
    printf("Old password: ");
    scanf("%s", old_pass);
    printf("New password: ");
    scanf("%s", new_pass);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("CHANGE_PASSWORD"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(token));
    json_object_object_add(data, "old_password", json_object_new_string(old_pass));
    json_object_object_add(data, "new_password", json_object_new_string(new_pass));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    // Receive response
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    printf("\nResponse:\n%s\n", buffer);
}

void send_get_permissions_request(int sock) {
    char token[MAX_TOKEN];
    int group_id;
    
    printf("\n=== GET PERMISSIONS ===\n");
    printf("Session token: ");
    scanf("%s", token);
    printf("Group ID: ");
    scanf("%d", &group_id);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("GET_PERMISSIONS"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(token));
    json_object_object_add(data, "group_id", json_object_new_int(group_id));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    // Receive response
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    printf("\nResponse:\n%s\n", buffer);
}

void send_update_permissions_request(int sock) {
    char token[MAX_TOKEN];
    int group_id, target_user_id;
    int can_read, can_write, can_delete, can_manage;
    
    printf("\n=== UPDATE PERMISSIONS ===\n");
    printf("Session token: ");
    scanf("%s", token);
    printf("Group ID: ");
    scanf("%d", &group_id);
    printf("Target User ID: ");
    scanf("%d", &target_user_id);
    printf("Can read (1/0): ");
    scanf("%d", &can_read);
    printf("Can write (1/0): ");
    scanf("%d", &can_write);
    printf("Can delete (1/0): ");
    scanf("%d", &can_delete);
    printf("Can manage (1/0): ");
    scanf("%d", &can_manage);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("UPDATE_PERMISSIONS"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(token));
    json_object_object_add(data, "group_id", json_object_new_int(group_id));
    json_object_object_add(data, "target_user_id", json_object_new_int(target_user_id));
    json_object_object_add(data, "can_read", json_object_new_boolean(can_read));
    json_object_object_add(data, "can_write", json_object_new_boolean(can_write));
    json_object_object_add(data, "can_delete", json_object_new_boolean(can_delete));
    json_object_object_add(data, "can_manage", json_object_new_boolean(can_manage));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    // Receive response
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    printf("\nResponse:\n%s\n", buffer);
}

void send_create_group_request(int sock) {
    char token[MAX_TOKEN], group_name[101], description[256];
    
    printf("\n=== CREATE GROUP ===\n");
    printf("Session token: ");
    scanf("%s", token);
    printf("Group name: ");
    getchar(); // consume newline
    fgets(group_name, 101, stdin);
    group_name[strcspn(group_name, "\n")] = 0;
    printf("Description: ");
    fgets(description, 256, stdin);
    description[strcspn(description, "\n")] = 0;
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("CREATE_GROUP"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(token));
    json_object_object_add(data, "group_name", json_object_new_string(group_name));
    json_object_object_add(data, "description", json_object_new_string(description));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    // Receive response
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    printf("\nResponse:\n%s\n", buffer);
}

void send_list_my_groups_request(int sock) {
    clear_screen();
    printf("\n=== LIST MY GROUPS ===\n");
    
    if (strlen(g_session_token) == 0) {
        print_error("Please login first!");
        wait_for_enter();
        return;
    }
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("LIST_MY_GROUPS"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(g_session_token));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    // Receive response
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    parse_and_display_response(buffer);
    wait_for_enter();
}

void send_list_group_members_request(int sock) {
    int group_id;
    
    clear_screen();
    printf("\n=== LIST GROUP MEMBERS ===\n");
    printf("Group ID: ");
    scanf("%d", &group_id);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("LIST_GROUP_MEMBERS"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(g_session_token));
    json_object_object_add(data, "group_id", json_object_new_int(group_id));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    // Receive response
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    parse_and_display_response(buffer);
    wait_for_enter();
}

void send_request_join_group_request(int sock) {
    char token[MAX_TOKEN];
    int group_id;
    
    printf("\n=== REQUEST JOIN GROUP ===\n");
    printf("Session token: ");
    scanf("%s", token);
    printf("Group ID: ");
    scanf("%d", &group_id);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("REQUEST_JOIN_GROUP"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(token));
    json_object_object_add(data, "group_id", json_object_new_int(group_id));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    // Receive response
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    printf("\nResponse:\n%s\n", buffer);
}

void send_list_join_requests_request(int sock) {
    int group_id;
    
    clear_screen();
    printf("\n=== LIST JOIN REQUESTS (ADMIN) ===\n");
    printf("Group ID: ");
    scanf("%d", &group_id);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("LIST_JOIN_REQUESTS"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(g_session_token));
    json_object_object_add(data, "group_id", json_object_new_int(group_id));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    // Receive response
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    parse_and_display_response(buffer);
    wait_for_enter();
}

void send_approve_join_request_request(int sock) {
    char token[MAX_TOKEN], action[20];
    int request_id;
    
    printf("\n=== APPROVE/REJECT JOIN REQUEST ===\n");
    printf("Session token: ");
    scanf("%s", token);
    printf("Request ID: ");
    scanf("%d", &request_id);
    printf("Action (approve/reject): ");
    scanf("%s", action);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("APPROVE_JOIN_REQUEST"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(token));
    json_object_object_add(data, "request_id", json_object_new_int(request_id));
    json_object_object_add(data, "action", json_object_new_string(action));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    // Receive response
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    printf("\nResponse:\n%s\n", buffer);
}

void send_invite_to_group_request(int sock) {
    char token[MAX_TOKEN], invitee_username[MAX_USERNAME];
    int group_id;
    
    printf("\n=== INVITE TO GROUP ===\n");
    printf("Session token: ");
    scanf("%s", token);
    printf("Group ID: ");
    scanf("%d", &group_id);
    printf("Invitee username: ");
    scanf("%s", invitee_username);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("INVITE_TO_GROUP"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(token));
    json_object_object_add(data, "group_id", json_object_new_int(group_id));
    json_object_object_add(data, "invitee_username", json_object_new_string(invitee_username));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    // Receive response
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    printf("\nResponse:\n%s\n", buffer);
}

void send_list_my_invitations_request(int sock) {
    clear_screen();
    printf("\n=== LIST MY INVITATIONS ===\n");
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("LIST_MY_INVITATIONS"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(g_session_token));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    // Receive response
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    parse_and_display_response(buffer);
    wait_for_enter();
}

void send_respond_invitation_request(int sock) {
    char token[MAX_TOKEN], action[20];
    int invitation_id;
    
    printf("\n=== RESPOND TO INVITATION ===\n");
    printf("Session token: ");
    scanf("%s", token);
    printf("Invitation ID: ");
    scanf("%d", &invitation_id);
    printf("Action (accept/reject): ");
    scanf("%s", action);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("RESPOND_INVITATION"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(token));
    json_object_object_add(data, "invitation_id", json_object_new_int(invitation_id));
    json_object_object_add(data, "action", json_object_new_string(action));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    // Receive response
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    printf("\nResponse:\n%s\n", buffer);
}

void send_leave_group_request(int sock) {
    char token[MAX_TOKEN];
    int group_id;
    
    printf("\n=== LEAVE GROUP ===\n");
    printf("Session token: ");
    scanf("%s", token);
    printf("Group ID: ");
    scanf("%d", &group_id);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("LEAVE_GROUP"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(token));
    json_object_object_add(data, "group_id", json_object_new_int(group_id));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    // Receive response
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    printf("\nResponse:\n%s\n", buffer);
}

void send_remove_member_request(int sock) {
    char token[MAX_TOKEN];
    int group_id, target_user_id;
    
    printf("\n=== REMOVE MEMBER ===\n");
    printf("Session token: ");
    scanf("%s", token);
    printf("Group ID: ");
    scanf("%d", &group_id);
    printf("Target User ID: ");
    scanf("%d", &target_user_id);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("REMOVE_MEMBER"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(token));
    json_object_object_add(data, "group_id", json_object_new_int(group_id));
    json_object_object_add(data, "target_user_id", json_object_new_int(target_user_id));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    // Receive response
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    printf("\nResponse:\n%s\n", buffer);
}

void send_create_directory_request(int sock) {
    char token[MAX_TOKEN], directory_name[256], parent_path[512];
    int group_id;
    
    printf("\n=== CREATE DIRECTORY ===\n");
    printf("Session token: ");
    scanf("%s", token);
    printf("Group ID: ");
    scanf("%d", &group_id);
    printf("Directory name: ");
    scanf("%s", directory_name);
    printf("Parent path: ");
    scanf("%s", parent_path);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("CREATE_DIRECTORY"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(token));
    json_object_object_add(data, "group_id", json_object_new_int(group_id));
    json_object_object_add(data, "directory_name", json_object_new_string(directory_name));
    json_object_object_add(data, "parent_path", json_object_new_string(parent_path));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    printf("\nResponse:\n%s\n", buffer);
}

void send_rename_directory_request(int sock) {
    char token[MAX_TOKEN], new_name[256];
    int directory_id;
    
    printf("\n=== RENAME DIRECTORY (Admin Only) ===\n");
    printf("Session token: ");
    scanf("%s", token);
    printf("Directory ID: ");
    scanf("%d", &directory_id);
    printf("New name: ");
    scanf("%s", new_name);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("RENAME_DIRECTORY"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(token));
    json_object_object_add(data, "directory_id", json_object_new_int(directory_id));
    json_object_object_add(data, "new_name", json_object_new_string(new_name));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    printf("\nResponse:\n%s\n", buffer);
}

void send_delete_directory_request(int sock) {
    char token[MAX_TOKEN], recursive_input[10];
    int directory_id, recursive;
    
    printf("\n=== DELETE DIRECTORY (Admin Only) ===\n");
    printf("Session token: ");
    scanf("%s", token);
    printf("Directory ID: ");
    scanf("%d", &directory_id);
    printf("Recursive (true/false): ");
    scanf("%s", recursive_input);
    recursive = (strcmp(recursive_input, "true") == 0 || strcmp(recursive_input, "1") == 0);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("DELETE_DIRECTORY"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(token));
    json_object_object_add(data, "directory_id", json_object_new_int(directory_id));
    json_object_object_add(data, "recursive", json_object_new_boolean(recursive));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    printf("\nResponse:\n%s\n", buffer);
}

void send_copy_directory_request(int sock) {
    char token[MAX_TOKEN], destination_path[512];
    int directory_id;
    
    printf("\n=== COPY DIRECTORY (Admin Only) ===\n");
    printf("Session token: ");
    scanf("%s", token);
    printf("Directory ID: ");
    scanf("%d", &directory_id);
    printf("Destination path: ");
    scanf("%s", destination_path);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("COPY_DIRECTORY"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(token));
    json_object_object_add(data, "directory_id", json_object_new_int(directory_id));
    json_object_object_add(data, "destination_path", json_object_new_string(destination_path));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    printf("\nResponse:\n%s\n", buffer);
}

void send_move_directory_request(int sock) {
    char token[MAX_TOKEN], destination_path[512];
    int directory_id;
    
    printf("\n=== MOVE DIRECTORY (Admin Only) ===\n");
    printf("Session token: ");
    scanf("%s", token);
    printf("Directory ID: ");
    scanf("%d", &directory_id);
    printf("Destination path: ");
    scanf("%s", destination_path);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("MOVE_DIRECTORY"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(token));
    json_object_object_add(data, "directory_id", json_object_new_int(directory_id));
    json_object_object_add(data, "destination_path", json_object_new_string(destination_path));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    printf("\nResponse:\n%s\n", buffer);
}

void show_account_menu(int sock);
void show_group_menu(int sock);
void show_directory_menu(int sock);

int main() {
    int sock = connect_to_server();
    if (sock < 0) {
        return 1;
    }
    
    int choice;
    while (1) {
        clear_screen();
        printf("\n╔════════════════════════════════════════╗\n");
        printf("║     FILE SHARING SYSTEM - MAIN MENU    ║\n");
        printf("╚════════════════════════════════════════╝\n");
        
        if (strlen(g_session_token) > 0) {
            printf("  👤 Logged in as: %s (ID: %d)\n", g_username, g_user_id);
            print_separator();
        }
        
        printf("\n📋 MAIN CATEGORIES:\n");
        printf("  1. 👤 Account Management\n");
        printf("  2. 👥 Group Management\n");
        printf("  3. 📁 Directory Management\n");
        printf("  4. 🚪 Exit\n");
        printf("\nChoice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                show_account_menu(sock);
                break;
            case 2:
                show_group_menu(sock);
                break;
            case 3:
                show_directory_menu(sock);
                break;
            case 4:
                printf("\n👋 Goodbye!\n");
                close(sock);
                return 0;
            default:
                printf("\n✗ Invalid choice!\n");
                wait_for_enter();
        }
    }
    
    return 0;
}

void show_account_menu(int sock) {
    int choice;
    while (1) {
        clear_screen();
        printf("\n╔════════════════════════════════════════╗\n");
        printf("║       👤 ACCOUNT MANAGEMENT           ║\n");
        printf("╚════════════════════════════════════════╝\n");
        
        printf("\n1. 📝 Register New Account\n");
        printf("2. 🔐 Login\n");
        printf("3. 🚪 Logout\n");
        printf("4. ✓ Verify Session\n");
        printf("5. ✏️  Update Profile\n");
        printf("6. 🔑 Change Password\n");
        printf("7. 🔙 Back to Main Menu\n");
        printf("\nChoice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                send_register_request(sock);
                break;
            case 2:
                send_login_request(sock);
                break;
            case 3:
                send_logout_request(sock);
                g_session_token[0] = '\0';
                g_user_id = 0;
                g_username[0] = '\0';
                break;
            case 4:
                send_verify_session_request(sock);
                break;
            case 5:
                send_update_profile_request(sock);
                break;
            case 6:
                send_change_password_request(sock);
                break;
            case 7:
                return;
            default:
                printf("\n✗ Invalid choice!\n");
                wait_for_enter();
        }
    }
}

void show_group_menu(int sock) {
    int choice;
    while (1) {
        clear_screen();
        printf("\n╔════════════════════════════════════════╗\n");
        printf("║        👥 GROUP MANAGEMENT            ║\n");
        printf("╚════════════════════════════════════════╝\n");
        
        if (strlen(g_session_token) == 0) {
            printf("\n⚠️  Please login first!\n");
            wait_for_enter();
            return;
        }
        
        printf("\n📋 GROUP OPERATIONS:\n");
        printf("  1. ➕ Create New Group\n");
        printf("  2. 📜 List My Groups\n");
        printf("  3. 👀 View Group Members\n");
        printf("  4. 🚪 Leave Group\n");
        printf("\n📨 JOIN REQUESTS:\n");
        printf("  5. 📤 Send Join Request\n");
        printf("  6. 📥 View Join Requests (Admin)\n");
        printf("  7. ✅ Approve/Reject Join Request (Admin)\n");
        printf("\n💌 INVITATIONS:\n");
        printf("  8. 📧 Invite User to Group (Admin)\n");
        printf("  9. 📬 View My Invitations\n");
        printf(" 10. ✔️  Respond to Invitation\n");
        printf("\n⚙️  ADMIN OPERATIONS:\n");
        printf(" 11. 🔐 Get Permissions\n");
        printf(" 12. ✏️  Update Permissions\n");
        printf(" 13. 🗑️  Remove Member\n");
        printf("\n14. 🔙 Back to Main Menu\n");
        printf("\nChoice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                send_create_group_request(sock);
                break;
            case 2:
                send_list_my_groups_request(sock);
                break;
            case 3:
                send_list_group_members_request(sock);
                break;
            case 4:
                send_leave_group_request(sock);
                break;
            case 5:
                send_request_join_group_request(sock);
                break;
            case 6:
                send_list_join_requests_request(sock);
                break;
            case 7:
                send_approve_join_request_request(sock);
                break;
            case 8:
                send_invite_to_group_request(sock);
                break;
            case 9:
                send_list_my_invitations_request(sock);
                break;
            case 10:
                send_respond_invitation_request(sock);
                break;
            case 11:
                send_get_permissions_request(sock);
                break;
            case 12:
                send_update_permissions_request(sock);
                break;
            case 13:
                send_remove_member_request(sock);
                break;
            case 14:
                return;
            default:
                printf("\n✗ Invalid choice!\n");
                wait_for_enter();
        }
    }
}

void show_directory_menu(int sock) {
    int choice;
    while (1) {
        clear_screen();
        printf("\n╔════════════════════════════════════════╗\n");
        printf("║      📁 DIRECTORY MANAGEMENT          ║\n");
        printf("╚════════════════════════════════════════╝\n");
        
        if (strlen(g_session_token) == 0) {
            printf("\n⚠️  Please login first!\n");
            wait_for_enter();
            return;
        }
        
        printf("\n1. ➕ Create Directory\n");
        printf("2. ✏️  Rename Directory (Admin)\n");
        printf("3. 🗑️  Delete Directory (Admin)\n");
        printf("4. 📋 Copy Directory (Admin)\n");
        printf("5. 📦 Move Directory (Admin)\n");
        printf("6. 🔙 Back to Main Menu\n");
        printf("\nChoice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                send_create_directory_request(sock);
                break;
            case 2:
                send_rename_directory_request(sock);
                break;
            case 3:
                send_delete_directory_request(sock);
                break;
            case 4:
                send_copy_directory_request(sock);
                break;
            case 5:
                send_move_directory_request(sock);
                break;
            case 6:
                return;
            default:
                printf("\n✗ Invalid choice!\n");
                wait_for_enter();
        }
    }
}