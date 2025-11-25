#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <json-c/json.h>
#include "../common/protocol.h"

int connect_to_server() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("172.11.14.114");
    
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
    
    printf("\nResponse:\n%s\n", buffer);
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
    char token[MAX_TOKEN];
    
    printf("\n=== LIST MY GROUPS ===\n");
    printf("Session token: ");
    scanf("%s", token);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("LIST_MY_GROUPS"));
    
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

void send_list_group_members_request(int sock) {
    char token[MAX_TOKEN];
    int group_id;
    
    printf("\n=== LIST GROUP MEMBERS ===\n");
    printf("Session token: ");
    scanf("%s", token);
    printf("Group ID: ");
    scanf("%d", &group_id);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("LIST_GROUP_MEMBERS"));
    
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
    char token[MAX_TOKEN];
    int group_id;
    
    printf("\n=== LIST JOIN REQUESTS ===\n");
    printf("Session token: ");
    scanf("%s", token);
    printf("Group ID: ");
    scanf("%d", &group_id);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("LIST_JOIN_REQUESTS"));
    
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

int main() {
    int sock = connect_to_server();
    if (sock < 0) {
        return 1;
    }
    
    int choice;
    while (1) {
        printf("\n=== MENU ===\n");
        printf("1. Register\n");
        printf("2. Login\n");
        printf("3. Logout\n");
        printf("4. Verify Session\n");
        printf("5. Update Profile\n");
        printf("6. Change Password\n");
        printf("7. Get Permissions\n");
        printf("8. Update Permissions\n");
        printf("9. Create Group\n");
        printf("10. List My Groups\n");
        printf("11. List Group Members\n");
        printf("12. Request Join Group\n");
        printf("13. List Join Requests (Admin)\n");
        printf("14. Approve/Reject Join Request\n");
        printf("15. Exit\n");
        printf("Choice: ");
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
                send_get_permissions_request(sock);
                break;
            case 8:
                send_update_permissions_request(sock);
                break;
            case 9:
                send_create_group_request(sock);
                break;
            case 10:
                send_list_my_groups_request(sock);
                break;
            case 11:
                send_list_group_members_request(sock);
                break;
            case 12:
                send_request_join_group_request(sock);
                break;
            case 13:
                send_list_join_requests_request(sock);
                break;
            case 14:
                send_approve_join_request_request(sock);
                break;
            case 15:
                close(sock);
                return 0;
            default:
                printf("Invalid choice\n");
        }
    }
    
    return 0;
}