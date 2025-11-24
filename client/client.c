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
    server_addr.sin_addr.s_addr = inet_addr("192.168.102.30");
    
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
        printf("7. Exit\n");
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
                close(sock);
                return 0;
            default:
                printf("Invalid choice\n");
        }
    }
    
    return 0;
}