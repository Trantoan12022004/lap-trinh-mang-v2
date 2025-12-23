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

void display_notification(struct json_object *notif_obj) {
    struct json_object *id_obj, *type_obj, *title_obj, *message_obj, 
                       *is_read_obj, *created_at_obj, *related_type_obj, *related_id_obj;
    
    json_object_object_get_ex(notif_obj, "notification_id", &id_obj);
    json_object_object_get_ex(notif_obj, "type", &type_obj);
    json_object_object_get_ex(notif_obj, "title", &title_obj);
    json_object_object_get_ex(notif_obj, "message", &message_obj);
    json_object_object_get_ex(notif_obj, "is_read", &is_read_obj);
    json_object_object_get_ex(notif_obj, "created_at", &created_at_obj);
    json_object_object_get_ex(notif_obj, "related_type", &related_type_obj);
    json_object_object_get_ex(notif_obj, "related_id", &related_id_obj);
    
    int id = json_object_get_int(id_obj);
    const char *type = json_object_get_string(type_obj);
    const char *title = json_object_get_string(title_obj);
    const char *message = json_object_get_string(message_obj);
    int is_read = json_object_get_boolean(is_read_obj);
    const char *created_at = json_object_get_string(created_at_obj);
    
    // Icon based on type
    const char *icon = "📬";
    if (strcmp(type, "JOIN_REQUEST") == 0) icon = "🙋";
    else if (strcmp(type, "JOIN_REQUEST_RESPONSE") == 0) icon = "✅";
    else if (strcmp(type, "GROUP_INVITATION") == 0) icon = "💌";
    else if (strcmp(type, "INVITATION_ACCEPTED") == 0) icon = "🎉";
    else if (strcmp(type, "MEMBER_LEFT") == 0) icon = "👋";
    else if (strcmp(type, "REMOVED_FROM_GROUP") == 0) icon = "🚫";
    
    // Status indicator
    const char *status_mark = is_read ? "  " : "🔴";
    
    printf("┌─────────────────────────────────────────────────────────┐\n");
    printf("│ %s %s [ID:%d] %s\n", status_mark, icon, id, is_read ? "" : "NEW");
    printf("├─────────────────────────────────────────────────────────┤\n");
    printf("│ 📌 %s\n", title);
    printf("│ 💬 %s\n", message);
    printf("│ 🕒 %s\n", created_at);
    printf("└─────────────────────────────────────────────────────────┘\n");
}

void parse_and_display_response(const char *json_str) {
    // Display raw response first
    printf("\n[RAW RESPONSE FROM SERVER]\n");
    print_separator();
    printf("%s\n", json_str);
    print_separator();
    
    struct json_object *response = json_tokener_parse(json_str);
    if (!response) {
        print_error("Failed to parse JSON response");
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
    
    // Display processed response
    printf("\n[PROCESSED RESPONSE]\n");
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
        if (strcmp(code, "SUCCESS_REGISTER") == 0) {
            struct json_object *user_id_obj, *username_obj, *created_at_obj;
            json_object_object_get_ex(payload_obj, "user_id", &user_id_obj);
            json_object_object_get_ex(payload_obj, "username", &username_obj);
            json_object_object_get_ex(payload_obj, "created_at", &created_at_obj);
            
            printf("\n✓ Registration Successful!\n");
            printf("  👤 Username: %s\n", json_object_get_string(username_obj));
            printf("  🆔 User ID: %d\n", json_object_get_int(user_id_obj));
            printf("  📅 Created at: %s\n", json_object_get_string(created_at_obj));
        } else if (strcmp(code, "SUCCESS_LOGIN") == 0) {
            struct json_object *token_obj, *user_id_obj, *username_obj, *email_obj, *full_name_obj;
            json_object_object_get_ex(payload_obj, "session_token", &token_obj);
            json_object_object_get_ex(payload_obj, "user_id", &user_id_obj);
            json_object_object_get_ex(payload_obj, "username", &username_obj);
            json_object_object_get_ex(payload_obj, "email", &email_obj);
            json_object_object_get_ex(payload_obj, "full_name", &full_name_obj);
            
            if (token_obj) {
                strncpy(g_session_token, json_object_get_string(token_obj), MAX_TOKEN - 1);
                g_user_id = json_object_get_int(user_id_obj);
                strncpy(g_username, json_object_get_string(username_obj), MAX_USERNAME - 1);
                
                printf("\n✓ Login Successful!\n");
                printf("  👤 Username: %s\n", g_username);
                printf("  🆔 User ID: %d\n", g_user_id);
                printf("  📧 Email: %s\n", json_object_get_string(email_obj));
                printf("  📝 Full Name: %s\n", json_object_get_string(full_name_obj));
                printf("  🔑 Session saved!\n");
            }
        } else if (strcmp(code, "SUCCESS_LOGOUT") == 0) {
            printf("\n✓ Successfully logged out!\n");
        } else if (strcmp(code, "SUCCESS_VERIFY_SESSION") == 0) {
            struct json_object *user_id_obj, *username_obj, *email_obj;
            json_object_object_get_ex(payload_obj, "user_id", &user_id_obj);
            json_object_object_get_ex(payload_obj, "username", &username_obj);
            json_object_object_get_ex(payload_obj, "email", &email_obj);
            
            printf("\n✓ Session is valid!\n");
            printf("  👤 Username: %s\n", json_object_get_string(username_obj));
            printf("  🆔 User ID: %d\n", json_object_get_int(user_id_obj));
            printf("  📧 Email: %s\n", json_object_get_string(email_obj));
        } else if (strcmp(code, "SUCCESS_UPDATE_PROFILE") == 0) {
            struct json_object *email_obj, *full_name_obj;
            json_object_object_get_ex(payload_obj, "email", &email_obj);
            json_object_object_get_ex(payload_obj, "full_name", &full_name_obj);
            
            printf("\n✓ Profile updated successfully!\n");
            printf("  📧 New Email: %s\n", json_object_get_string(email_obj));
            printf("  📝 New Full Name: %s\n", json_object_get_string(full_name_obj));
        } else if (strcmp(code, "SUCCESS_CHANGE_PASSWORD") == 0) {
            printf("\n✓ Password changed successfully!\n");
        } else if (strcmp(code, "SUCCESS_CREATE_GROUP") == 0) {
            struct json_object *group_id_obj, *group_name_obj, *description_obj, *created_at_obj;
            json_object_object_get_ex(payload_obj, "group_id", &group_id_obj);
            json_object_object_get_ex(payload_obj, "group_name", &group_name_obj);
            json_object_object_get_ex(payload_obj, "description", &description_obj);
            json_object_object_get_ex(payload_obj, "created_at", &created_at_obj);
            
            printf("\n✓ Group created successfully!\n");
            printf("  🆔 Group ID: %d\n", json_object_get_int(group_id_obj));
            printf("  👥 Name: %s\n", json_object_get_string(group_name_obj));
            printf("  📝 Description: %s\n", json_object_get_string(description_obj));
            printf("  📅 Created at: %s\n", json_object_get_string(created_at_obj));
        } else if (strcmp(code, "SUCCESS_LIST_GROUPS") == 0) {
            struct json_object *groups_obj;
            json_object_object_get_ex(payload_obj, "groups", &groups_obj);
            int count = json_object_array_length(groups_obj);
            
            printf("\n👥 Your Groups (%d):\n", count);
            if (count == 0) {
                printf("  📭 No groups yet. Create or join one!\n");
            } else {
                for (int i = 0; i < count; i++) {
                    struct json_object *group = json_object_array_get_idx(groups_obj, i);
                    struct json_object *id, *name, *role, *members;
                    json_object_object_get_ex(group, "group_id", &id);
                    json_object_object_get_ex(group, "group_name", &name);
                    json_object_object_get_ex(group, "role", &role);
                    json_object_object_get_ex(group, "member_count", &members);
                    
                    printf("\n  [%d] %s\n", json_object_get_int(id), json_object_get_string(name));
                    printf("      Role: %s | Members: %d\n", 
                           json_object_get_string(role),
                           json_object_get_int(members));
                }
            }
        } else if (strcmp(code, "SUCCESS_LIST_MEMBERS") == 0) {
            struct json_object *members_obj, *group_id_obj;
            json_object_object_get_ex(payload_obj, "members", &members_obj);
            json_object_object_get_ex(payload_obj, "group_id", &group_id_obj);
            int count = json_object_array_length(members_obj);
            
            printf("\n👥 Group Members (Group ID: %d) - Total: %d\n", 
                   json_object_get_int(group_id_obj), count);
            if (count == 0) {
                printf("  📭 No members in this group.\n");
            } else {
                for (int i = 0; i < count; i++) {
                    struct json_object *member = json_object_array_get_idx(members_obj, i);
                    struct json_object *id, *username, *name, *role;
                    json_object_object_get_ex(member, "user_id", &id);
                    json_object_object_get_ex(member, "username", &username);
                    json_object_object_get_ex(member, "full_name", &name);
                    json_object_object_get_ex(member, "role", &role);
                    
                    printf("\n  [ID:%d] %s\n", json_object_get_int(id), json_object_get_string(username));
                    printf("      Name: %s | Role: %s\n", 
                           json_object_get_string(name),
                           json_object_get_string(role));
                }
            }
        } else if (strcmp(code, "SUCCESS_REQUEST_JOIN") == 0) {
            struct json_object *request_id_obj, *group_id_obj, *created_at_obj;
            json_object_object_get_ex(payload_obj, "request_id", &request_id_obj);
            json_object_object_get_ex(payload_obj, "group_id", &group_id_obj);
            json_object_object_get_ex(payload_obj, "created_at", &created_at_obj);
            
            printf("\n✓ Join request sent successfully!\n");
            printf("  🆔 Request ID: %d\n", json_object_get_int(request_id_obj));
            printf("  👥 Group ID: %d\n", json_object_get_int(group_id_obj));
            printf("  📅 Created at: %s\n", json_object_get_string(created_at_obj));
            printf("  ⏳ Status: Pending approval\n");
        } else if (strcmp(code, "SUCCESS_LIST_REQUESTS") == 0) {
            struct json_object *requests_obj, *group_id_obj;
            json_object_object_get_ex(payload_obj, "requests", &requests_obj);
            json_object_object_get_ex(payload_obj, "group_id", &group_id_obj);
            int count = json_object_array_length(requests_obj);
            
            printf("\n🙋 Join Requests for Group ID %d - Total: %d\n", 
                   json_object_get_int(group_id_obj), count);
            if (count == 0) {
                printf("  📭 No pending join requests.\n");
            } else {
                for (int i = 0; i < count; i++) {
                    struct json_object *req = json_object_array_get_idx(requests_obj, i);
                    struct json_object *id, *username, *name, *status;
                    json_object_object_get_ex(req, "request_id", &id);
                    json_object_object_get_ex(req, "username", &username);
                    json_object_object_get_ex(req, "full_name", &name);
                    json_object_object_get_ex(req, "status", &status);
                    
                    printf("\n  [ReqID:%d] %s (%s)\n", 
                           json_object_get_int(id),
                           json_object_get_string(username),
                           json_object_get_string(name));
                    printf("      Status: %s\n", json_object_get_string(status));
                }
            }
        } else if (strcmp(code, "SUCCESS_APPROVE_REQUEST") == 0 || strcmp(code, "SUCCESS_REJECT_REQUEST") == 0) {
            struct json_object *request_id_obj, *user_id_obj, *status_obj;
            json_object_object_get_ex(payload_obj, "request_id", &request_id_obj);
            json_object_object_get_ex(payload_obj, "user_id", &user_id_obj);
            json_object_object_get_ex(payload_obj, "status", &status_obj);
            
            const char *action = strcmp(code, "SUCCESS_APPROVE_REQUEST") == 0 ? "Approved" : "Rejected";
            printf("\n✓ Request %s!\n", action);
            printf("  🆔 Request ID: %d\n", json_object_get_int(request_id_obj));
            printf("  👤 User ID: %d\n", json_object_get_int(user_id_obj));
            printf("  📊 Status: %s\n", json_object_get_string(status_obj));
        } else if (strcmp(code, "SUCCESS_SEND_INVITATION") == 0) {
            struct json_object *invitation_id_obj, *group_id_obj, *invitee_id_obj, *created_at_obj;
            json_object_object_get_ex(payload_obj, "invitation_id", &invitation_id_obj);
            json_object_object_get_ex(payload_obj, "group_id", &group_id_obj);
            json_object_object_get_ex(payload_obj, "invitee_id", &invitee_id_obj);
            json_object_object_get_ex(payload_obj, "created_at", &created_at_obj);
            
            printf("\n✓ Invitation sent successfully!\n");
            printf("  🆔 Invitation ID: %d\n", json_object_get_int(invitation_id_obj));
            printf("  👥 Group ID: %d\n", json_object_get_int(group_id_obj));
            printf("  👤 Invitee ID: %d\n", json_object_get_int(invitee_id_obj));
            printf("  📅 Created at: %s\n", json_object_get_string(created_at_obj));
        } else if (strcmp(code, "SUCCESS_LIST_INVITATIONS") == 0) {
            struct json_object *invitations_obj;
            json_object_object_get_ex(payload_obj, "invitations", &invitations_obj);
            int count = json_object_array_length(invitations_obj);
            
            printf("\n💌 Your Invitations - Total: %d\n", count);
            if (count == 0) {
                printf("  📭 No pending invitations.\n");
            } else {
                for (int i = 0; i < count; i++) {
                    struct json_object *inv = json_object_array_get_idx(invitations_obj, i);
                    struct json_object *id, *group_name, *inviter, *status;
                    json_object_object_get_ex(inv, "invitation_id", &id);
                    json_object_object_get_ex(inv, "group_name", &group_name);
                    json_object_object_get_ex(inv, "inviter_username", &inviter);
                    json_object_object_get_ex(inv, "status", &status);
                    
                    printf("\n  [InvID:%d] Group: %s\n", 
                           json_object_get_int(id),
                           json_object_get_string(group_name));
                    printf("      From: %s | Status: %s\n", 
                           json_object_get_string(inviter),
                           json_object_get_string(status));
                }
            }
        } else if (strcmp(code, "SUCCESS_ACCEPT_INVITATION") == 0 || strcmp(code, "SUCCESS_REJECT_INVITATION") == 0) {
            struct json_object *invitation_id_obj, *group_id_obj, *status_obj;
            json_object_object_get_ex(payload_obj, "invitation_id", &invitation_id_obj);
            json_object_object_get_ex(payload_obj, "group_id", &group_id_obj);
            json_object_object_get_ex(payload_obj, "status", &status_obj);
            
            const char *action = strcmp(code, "SUCCESS_ACCEPT_INVITATION") == 0 ? "Accepted" : "Rejected";
            printf("\n✓ Invitation %s!\n", action);
            printf("  🆔 Invitation ID: %d\n", json_object_get_int(invitation_id_obj));
            printf("  👥 Group ID: %d\n", json_object_get_int(group_id_obj));
            printf("  📊 Status: %s\n", json_object_get_string(status_obj));
        } else if (strcmp(code, "SUCCESS_LEAVE_GROUP") == 0) {
            struct json_object *group_id_obj;
            json_object_object_get_ex(payload_obj, "group_id", &group_id_obj);
            
            printf("\n✓ Successfully left the group!\n");
            printf("  👥 Group ID: %d\n", json_object_get_int(group_id_obj));
        } else if (strcmp(code, "SUCCESS_REMOVE_MEMBER") == 0) {
            struct json_object *group_id_obj, *user_id_obj;
            json_object_object_get_ex(payload_obj, "group_id", &group_id_obj);
            json_object_object_get_ex(payload_obj, "user_id", &user_id_obj);
            
            printf("\n✓ Member removed successfully!\n");
            printf("  👥 Group ID: %d\n", json_object_get_int(group_id_obj));
            printf("  👤 User ID: %d\n", json_object_get_int(user_id_obj));
        } else if (strcmp(code, "SUCCESS_GET_NOTIFICATIONS") == 0) {
            struct json_object *notif_array, *total_count_obj;
            json_object_object_get_ex(payload_obj, "notifications", &notif_array);
            json_object_object_get_ex(payload_obj, "total_count", &total_count_obj);
            
            int count = json_object_get_int(total_count_obj);
            
            printf("\n╔══════════════════════════════════════════════════════════╗\n");
            printf("║              📬 YOUR NOTIFICATIONS (%d)                ║\n", count);
            printf("╚══════════════════════════════════════════════════════════╝\n\n");
            
            if (count == 0) {
                printf("  📭 No notifications yet. You're all caught up!\n\n");
            } else {
                // Phân loại thông báo
                int unread_count = 0;
                for (int i = 0; i < json_object_array_length(notif_array); i++) {
                    struct json_object *notif = json_object_array_get_idx(notif_array, i);
                    struct json_object *is_read_obj;
                    json_object_object_get_ex(notif, "is_read", &is_read_obj);
                    if (!json_object_get_boolean(is_read_obj)) {
                        unread_count++;
                    }
                }
                
                if (unread_count > 0) {
                    printf("🔴 Unread: %d | 📖 Read: %d\n\n", unread_count, count - unread_count);
                }
                
                for (int i = 0; i < json_object_array_length(notif_array); i++) {
                    struct json_object *notif = json_object_array_get_idx(notif_array, i);
                    display_notification(notif);
                    printf("\n");
                }
            }
        } else if (strcmp(code, "SUCCESS_MARK_NOTIFICATION_READ") == 0) {
            struct json_object *notif_id_obj;
            json_object_object_get_ex(payload_obj, "notification_id", &notif_id_obj);
            
            printf("\n✓ Notification marked as read!\n");
            printf("  🆔 Notification ID: %d\n", json_object_get_int(notif_id_obj));
        } else if (strcmp(code, "SUCCESS_MARK_ALL_READ") == 0) {
            struct json_object *marked_count_obj;
            json_object_object_get_ex(payload_obj, "marked_count", &marked_count_obj);
            
            printf("\n✓ All notifications marked as read!\n");
            printf("  📊 Total marked: %d\n", json_object_get_int(marked_count_obj));
        } else if (strcmp(code, "SUCCESS_GET_UNREAD_COUNT") == 0) {
            struct json_object *count_obj;
            json_object_object_get_ex(payload_obj, "unread_count", &count_obj);
            int count = json_object_get_int(count_obj);
            
            printf("\n");
            if (count > 0) {
                printf("🔴 You have %d unread notification%s\n", count, count > 1 ? "s" : "");
            } else {
                printf("✅ You're all caught up! No unread notifications.\n");
            }
        } else if (strcmp(code, "SUCCESS_GET_PERMISSIONS") == 0 ||
                   strcmp(code, "SUCCESS_UPDATE_PERMISSIONS") == 0) {
            printf("\n✓ Permissions operation completed!\n");
            printf("  Details: %s\n", json_object_to_json_string_ext(payload_obj, JSON_C_TO_STRING_PRETTY));
        } else if (strcmp(code, "SUCCESS_LIST_AVAILABLE_GROUPS") == 0) {
            struct json_object *groups_obj, *total_count_obj;
            json_object_object_get_ex(payload_obj, "groups", &groups_obj);
            json_object_object_get_ex(payload_obj, "total_count", &total_count_obj);
            int count = json_object_get_int(total_count_obj);
            
            printf("\n🔍 Available Groups (%d):\n", count);
            if (count == 0) {
                printf("  📭 No groups available to join.\n");
            }
        } else {
            // Display raw payload for unknown responses
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
    server_addr.sin_addr.s_addr = inet_addr("172.18.38.233");
    
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
    
    clear_screen();
    printf("\n=== REGISTER NEW ACCOUNT ===\n");
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
    
    parse_and_display_response(buffer);
    wait_for_enter();
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
    clear_screen();
    printf("\n=== LOGOUT ===\n");
    
    if (strlen(g_session_token) == 0) {
        print_error("You are not logged in!");
        wait_for_enter();
        return;
    }
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("LOGOUT"));
    
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
    
    // Clear session
    g_session_token[0] = '\0';
    g_user_id = 0;
    g_username[0] = '\0';
    
    wait_for_enter();
}

void send_verify_session_request(int sock) {
    clear_screen();
    printf("\n=== VERIFY SESSION ===\n");
    
    if (strlen(g_session_token) == 0) {
        print_error("Please login first!");
        wait_for_enter();
        return;
    }
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("VERIFY_SESSION"));
    
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

void send_update_profile_request(int sock) {
    char email[MAX_EMAIL], full_name[MAX_FULLNAME];
    
    clear_screen();
    printf("\n=== UPDATE PROFILE ===\n");
    
    if (strlen(g_session_token) == 0) {
        print_error("Please login first!");
        wait_for_enter();
        return;
    }
    
    printf("New email: ");
    scanf("%s", email);
    printf("New full name: ");
    getchar(); // consume newline
    fgets(full_name, MAX_FULLNAME, stdin);
    full_name[strcspn(full_name, "\n")] = 0;
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("UPDATE_PROFILE"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(g_session_token));
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
    
    parse_and_display_response(buffer);
    wait_for_enter();
}

void send_change_password_request(int sock) {
    char old_pass[MAX_PASSWORD], new_pass[MAX_PASSWORD];
    
    clear_screen();
    printf("\n=== CHANGE PASSWORD ===\n");
    
    if (strlen(g_session_token) == 0) {
        print_error("Please login first!");
        wait_for_enter();
        return;
    }
    
    printf("Old password: ");
    scanf("%s", old_pass);
    printf("New password: ");
    scanf("%s", new_pass);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("CHANGE_PASSWORD"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(g_session_token));
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
    
    parse_and_display_response(buffer);
    wait_for_enter();
}

void send_get_permissions_request(int sock) {
    int group_id;
    
    clear_screen();
    printf("\n=== GET PERMISSIONS ===\n");
    
    if (strlen(g_session_token) == 0) {
        print_error("Please login first!");
        wait_for_enter();
        return;
    }
    
    printf("Group ID: ");
    scanf("%d", &group_id);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("GET_PERMISSIONS"));
    
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

void send_update_permissions_request(int sock) {
    int group_id, target_user_id;
    int can_read, can_write, can_delete, can_manage;
    
    clear_screen();
    printf("\n=== UPDATE PERMISSIONS ===\n");
    
    if (strlen(g_session_token) == 0) {
        print_error("Please login first!");
        wait_for_enter();
        return;
    }
    
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
    json_object_object_add(data, "session_token", json_object_new_string(g_session_token));
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
    
    parse_and_display_response(buffer);
    wait_for_enter();
}

void send_create_group_request(int sock) {
    char group_name[101], description[256];
    
    clear_screen();
    printf("\n=== CREATE GROUP ===\n");
    
    if (strlen(g_session_token) == 0) {
        print_error("Please login first!");
        wait_for_enter();
        return;
    }
    
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
    json_object_object_add(data, "session_token", json_object_new_string(g_session_token));
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
    
    parse_and_display_response(buffer);
    wait_for_enter();
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
    clear_screen();
    printf("\n=== LIST GROUP MEMBERS ===\n");
    
    if (strlen(g_session_token) == 0) {
        print_error("Please login first!");
        wait_for_enter();
        return;
    }
    
    // Bước 1: Lấy danh sách tất cả groups
    struct json_object *list_req = json_object_new_object();
    json_object_object_add(list_req, "command", json_object_new_string("LIST_MY_GROUPS"));
    
    struct json_object *list_data = json_object_new_object();
    json_object_object_add(list_data, "session_token", json_object_new_string(g_session_token));
    json_object_object_add(list_req, "data", list_data);
    
    const char *list_json = json_object_to_json_string(list_req);
    send(sock, list_json, strlen(list_json), 0);
    json_object_put(list_req);
    
    // Nhận danh sách groups
    char list_buffer[BUFFER_SIZE];
    int list_bytes = recv(sock, list_buffer, BUFFER_SIZE - 1, 0);
    list_buffer[list_bytes] = '\0';
    
    struct json_object *list_response = json_tokener_parse(list_buffer);
    if (!list_response) {
        print_error("Failed to get groups list");
        wait_for_enter();
        return;
    }
    
    struct json_object *status_obj, *payload_obj, *groups_obj;
    json_object_object_get_ex(list_response, "status", &status_obj);
    json_object_object_get_ex(list_response, "payload", &payload_obj);
    
    if (json_object_get_int(status_obj) != 200 || !payload_obj) {
        print_error("Failed to retrieve groups");
        json_object_put(list_response);
        wait_for_enter();
        return;
    }
    
    json_object_object_get_ex(payload_obj, "groups", &groups_obj);
    int group_count = json_object_array_length(groups_obj);
    
    if (group_count == 0) {
        printf("\n📭 No groups available.\n");
        json_object_put(list_response);
        wait_for_enter();
        return;
    }
    
    // Hiển thị danh sách groups
    printf("\n👥 Available Groups:\n");
    print_separator();
    for (int i = 0; i < group_count; i++) {
        struct json_object *group = json_object_array_get_idx(groups_obj, i);
        struct json_object *id_obj, *name_obj, *member_count_obj;
        json_object_object_get_ex(group, "group_id", &id_obj);
        json_object_object_get_ex(group, "group_name", &name_obj);
        json_object_object_get_ex(group, "member_count", &member_count_obj);
        
        printf("[%d] %s (Members: %d)\n", 
               json_object_get_int(id_obj),
               json_object_get_string(name_obj),
               json_object_get_int(member_count_obj));
    }
    print_separator();
    
    // Cho người dùng chọn
    int selected_group_id;
    printf("\nEnter Group ID to view members (0 to cancel): ");
    scanf("%d", &selected_group_id);
    
    json_object_put(list_response);
    
    if (selected_group_id == 0) {
        return;
    }
    
    // Bước 2: Lấy danh sách members của group đã chọn
    struct json_object *member_req = json_object_new_object();
    json_object_object_add(member_req, "command", json_object_new_string("LIST_GROUP_MEMBERS"));
    
    struct json_object *member_data = json_object_new_object();
    json_object_object_add(member_data, "session_token", json_object_new_string(g_session_token));
    json_object_object_add(member_data, "group_id", json_object_new_int(selected_group_id));
    json_object_object_add(member_req, "data", member_data);
    
    const char *member_json = json_object_to_json_string(member_req);
    send(sock, member_json, strlen(member_json), 0);
    json_object_put(member_req);
    
    // Nhận danh sách members
    char member_buffer[BUFFER_SIZE];
    int member_bytes = recv(sock, member_buffer, BUFFER_SIZE - 1, 0);
    member_buffer[member_bytes] = '\0';
    
    parse_and_display_response(member_buffer);
    wait_for_enter();
}

void send_request_join_group_request(int sock) {
    clear_screen();
    printf("\n=== REQUEST JOIN GROUP ===\n");
    
    if (strlen(g_session_token) == 0) {
        print_error("Please login first!");
        wait_for_enter();
        return;
    }
    
    // Bước 1: Lấy danh sách các nhóm có thể tham gia
    struct json_object *list_req = json_object_new_object();
    json_object_object_add(list_req, "command", json_object_new_string("LIST_AVAILABLE_GROUPS"));
    
    struct json_object *list_data = json_object_new_object();
    json_object_object_add(list_data, "session_token", json_object_new_string(g_session_token));
    json_object_object_add(list_req, "data", list_data);
    
    const char *list_json = json_object_to_json_string(list_req);
    send(sock, list_json, strlen(list_json), 0);
    json_object_put(list_req);
    
    // Nhận danh sách groups
    char list_buffer[BUFFER_SIZE];
    int list_bytes = recv(sock, list_buffer, BUFFER_SIZE - 1, 0);
    list_buffer[list_bytes] = '\0';
    
    struct json_object *list_response = json_tokener_parse(list_buffer);
    if (!list_response) {
        print_error("Failed to get available groups");
        wait_for_enter();
        return;
    }
    
    struct json_object *status_obj, *payload_obj, *groups_obj;
    json_object_object_get_ex(list_response, "status", &status_obj);
    json_object_object_get_ex(list_response, "payload", &payload_obj);
    
    if (json_object_get_int(status_obj) != 200 || !payload_obj) {
        print_error("Failed to retrieve available groups");
        json_object_put(list_response);
        wait_for_enter();
        return;
    }
    
    json_object_object_get_ex(payload_obj, "groups", &groups_obj);
    int group_count = json_object_array_length(groups_obj);
    
    if (group_count == 0) {
        printf("\n📭 No available groups to join.\n");
        printf("   You are already a member of all existing groups,\n");
        printf("   or you have pending requests/invitations for all other groups.\n");
        json_object_put(list_response);
        wait_for_enter();
        return;
    }
    
    // Hiển thị danh sách groups
        printf("\n🔍 Available Groups to Join (%d):\n", group_count);
    
    for (int i = 0; i < group_count; i++) {
        struct json_object *group = json_object_array_get_idx(groups_obj, i);
        struct json_object *id_obj, *name_obj, *desc_obj, *member_count_obj, *created_at_obj;
        json_object_object_get_ex(group, "group_id", &id_obj);
        json_object_object_get_ex(group, "group_name", &name_obj);
        json_object_object_get_ex(group, "description", &desc_obj);
        json_object_object_get_ex(group, "member_count", &member_count_obj);
        json_object_object_get_ex(group, "created_at", &created_at_obj);
        
        printf("\n┌─────────────────────────────────────────────────────┐\n");
        printf("│ [%d] %s\n", 
               json_object_get_int(id_obj),
               json_object_get_string(name_obj));
        printf("├─────────────────────────────────────────────────────┤\n");
        printf("│ 📝 Description: %s\n", json_object_get_string(desc_obj));
        printf("│ 👥 Members: %d\n", json_object_get_int(member_count_obj));
        printf("│ 📅 Created: %s\n", json_object_get_string(created_at_obj));
        printf("└─────────────────────────────────────────────────────┘\n");
    }
    
    // Cho người dùng chọn
    int selected_group_id;
    printf("\n➤ Enter Group ID to request join (0 to cancel): ");
    scanf("%d", &selected_group_id);
    getchar(); // consume newline
    
    json_object_put(list_response);
    
    if (selected_group_id == 0) {
        printf("\n❌ Request cancelled.\n");
        wait_for_enter();
        return;
    }
    
    // Bước 2: Gửi yêu cầu tham gia
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("REQUEST_JOIN_GROUP"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(g_session_token));
    json_object_object_add(data, "group_id", json_object_new_int(selected_group_id));
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
    char action[20];
    int request_id;
    
    clear_screen();
    printf("\n=== APPROVE/REJECT JOIN REQUEST ===\n");
    
    if (strlen(g_session_token) == 0) {
        print_error("Please login first!");
        wait_for_enter();
        return;
    }
    
    printf("Request ID: ");
    scanf("%d", &request_id);
    printf("Action (approve/reject): ");
    scanf("%s", action);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("APPROVE_JOIN_REQUEST"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(g_session_token));
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
    
    parse_and_display_response(buffer);
    wait_for_enter();
}

void send_invite_to_group_request(int sock) {
    char invitee_username[MAX_USERNAME];
    int group_id;
    
    clear_screen();
    printf("\n=== INVITE TO GROUP ===\n");
    
    if (strlen(g_session_token) == 0) {
        print_error("Please login first!");
        wait_for_enter();
        return;
    }
    
    printf("Group ID: ");
    scanf("%d", &group_id);
    printf("Invitee username: ");
    scanf("%s", invitee_username);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("INVITE_TO_GROUP"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(g_session_token));
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
    
    parse_and_display_response(buffer);
    wait_for_enter();
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
    char action[20];
    int invitation_id;
    
    clear_screen();
    printf("\n=== RESPOND TO INVITATION ===\n");
    
    if (strlen(g_session_token) == 0) {
        print_error("Please login first!");
        wait_for_enter();
        return;
    }
    
    printf("Invitation ID: ");
    scanf("%d", &invitation_id);
    printf("Action (accept/reject): ");
    scanf("%s", action);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("RESPOND_INVITATION"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(g_session_token));
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
    
    parse_and_display_response(buffer);
    wait_for_enter();
}

void send_leave_group_request(int sock) {
    int group_id;
    
    clear_screen();
    printf("\n=== LEAVE GROUP ===\n");
    
    if (strlen(g_session_token) == 0) {
        print_error("Please login first!");
        wait_for_enter();
        return;
    }
    
    printf("Group ID: ");
    scanf("%d", &group_id);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("LEAVE_GROUP"));
    
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

void send_remove_member_request(int sock) {
    int group_id, target_user_id;
    
    clear_screen();
    printf("\n=== REMOVE MEMBER ===\n");
    
    if (strlen(g_session_token) == 0) {
        print_error("Please login first!");
        wait_for_enter();
        return;
    }
    
    printf("Group ID: ");
    scanf("%d", &group_id);
    printf("Target User ID: ");
    scanf("%d", &target_user_id);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("REMOVE_MEMBER"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(g_session_token));
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
    
    parse_and_display_response(buffer);
    wait_for_enter();
}

void send_create_directory_request(int sock) {
    char directory_name[256], parent_path[512];
    int group_id;
    
    clear_screen();
    printf("\n=== CREATE DIRECTORY ===\n");
    
    if (strlen(g_session_token) == 0) {
        print_error("Please login first!");
        wait_for_enter();
        return;
    }
    
    printf("Group ID: ");
    scanf("%d", &group_id);
    getchar(); // consume newline after number input
    
    printf("Directory name: ");
    fgets(directory_name, sizeof(directory_name), stdin);
    directory_name[strcspn(directory_name, "\n")] = 0; // remove newline
    
    printf("Parent path (e.g., / or /folder): ");
    fgets(parent_path, sizeof(parent_path), stdin);
    parent_path[strcspn(parent_path, "\n")] = 0; // remove newline
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("CREATE_DIRECTORY"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(g_session_token));
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
    
    parse_and_display_response(buffer);
    wait_for_enter();
}

void send_rename_directory_request(int sock) {
    char new_name[256];
    int directory_id;
    
    clear_screen();
    printf("\n=== RENAME DIRECTORY (Admin Only) ===\n");
    
    if (strlen(g_session_token) == 0) {
        print_error("Please login first!");
        wait_for_enter();
        return;
    }
    
    printf("Directory ID: ");
    scanf("%d", &directory_id);
    printf("New name: ");
    scanf("%s", new_name);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("RENAME_DIRECTORY"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(g_session_token));
    json_object_object_add(data, "directory_id", json_object_new_int(directory_id));
    json_object_object_add(data, "new_name", json_object_new_string(new_name));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    parse_and_display_response(buffer);
    wait_for_enter();
}

void send_delete_directory_request(int sock) {
    char recursive_input[10];
    int directory_id, recursive;
    
    clear_screen();
    printf("\n=== DELETE DIRECTORY (Admin Only) ===\n");
    
    if (strlen(g_session_token) == 0) {
        print_error("Please login first!");
        wait_for_enter();
        return;
    }
    
    printf("Directory ID: ");
    scanf("%d", &directory_id);
    printf("Recursive (true/false): ");
    scanf("%s", recursive_input);
    recursive = (strcmp(recursive_input, "true") == 0 || strcmp(recursive_input, "1") == 0);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("DELETE_DIRECTORY"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(g_session_token));
    json_object_object_add(data, "directory_id", json_object_new_int(directory_id));
    json_object_object_add(data, "recursive", json_object_new_boolean(recursive));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    parse_and_display_response(buffer);
    wait_for_enter();
}

void send_copy_directory_request(int sock) {
    char destination_path[512];
    int directory_id;
    
    clear_screen();
    printf("\n=== COPY DIRECTORY (Admin Only) ===\n");
    
    if (strlen(g_session_token) == 0) {
        print_error("Please login first!");
        wait_for_enter();
        return;
    }
    
    printf("Directory ID: ");
    scanf("%d", &directory_id);
    printf("Destination path: ");
    scanf("%s", destination_path);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("COPY_DIRECTORY"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(g_session_token));
    json_object_object_add(data, "directory_id", json_object_new_int(directory_id));
    json_object_object_add(data, "destination_path", json_object_new_string(destination_path));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    parse_and_display_response(buffer);
    wait_for_enter();
}

void send_move_directory_request(int sock) {
    char destination_path[512];
    int directory_id;
    
    clear_screen();
    printf("\n=== MOVE DIRECTORY (Admin Only) ===\n");
    
    if (strlen(g_session_token) == 0) {
        print_error("Please login first!");
        wait_for_enter();
        return;
    }
    
    printf("Directory ID: ");
    scanf("%d", &directory_id);
    printf("Destination path: ");
    scanf("%s", destination_path);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("MOVE_DIRECTORY"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(g_session_token));
    json_object_object_add(data, "directory_id", json_object_new_int(directory_id));
    json_object_object_add(data, "destination_path", json_object_new_string(destination_path));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    
    json_object_put(request);
    
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    buffer[bytes] = '\0';
    
    parse_and_display_response(buffer);
    wait_for_enter();
}

void show_account_menu(int sock);
void show_group_menu(int sock);

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
        printf("  3. � Exit\n");
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
        // printf("4. ✓ Verify Session\n");
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


void send_get_notifications_request(int sock) {
    if (strlen(g_session_token) == 0) {
        print_error("Please login first!");
        wait_for_enter();
        return;
    }
    
    print_separator();
    printf("=== GET NOTIFICATIONS ===\n");
    printf("Session token: %s\n\n", g_session_token);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("GET_NOTIFICATIONS"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(g_session_token));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    json_object_put(request);
    
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        printf("\nResponse:\n");
        parse_and_display_response(buffer);
    }
    
    wait_for_enter();
}

void send_mark_notification_read_request(int sock) {
    if (strlen(g_session_token) == 0) {
        print_error("Please login first!");
        wait_for_enter();
        return;
    }
    
    print_separator();
    printf("=== MARK NOTIFICATION READ ===\n");
    printf("Session token: %s\n", g_session_token);
    
    int notification_id;
    printf("Notification ID: ");
    scanf("%d", &notification_id);
    getchar();
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("MARK_NOTIFICATION_READ"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(g_session_token));
    json_object_object_add(data, "notification_id", json_object_new_int(notification_id));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    json_object_put(request);
    
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        printf("\nResponse:\n");
        parse_and_display_response(buffer);
    }
    
    wait_for_enter();
}

void send_get_unread_count_request(int sock) {
    if (strlen(g_session_token) == 0) {
        print_error("Please login first!");
        wait_for_enter();
        return;
    }
    
    print_separator();
    printf("=== GET UNREAD NOTIFICATION COUNT ===\n");
    printf("Session token: %s\n\n", g_session_token);
    
    struct json_object *request = json_object_new_object();
    json_object_object_add(request, "command", json_object_new_string("GET_UNREAD_COUNT"));
    
    struct json_object *data = json_object_new_object();
    json_object_object_add(data, "session_token", json_object_new_string(g_session_token));
    json_object_object_add(request, "data", data);
    
    const char *json_str = json_object_to_json_string(request);
    send(sock, json_str, strlen(json_str), 0);
    json_object_put(request);
    
    char buffer[BUFFER_SIZE];
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        printf("\nResponse:\n");
        parse_and_display_response(buffer);
    }
    
    wait_for_enter();
}

void show_group_menu(int sock) {
    while (1) {
        clear_screen();
        
        // Lấy số thông báo chưa đọc
        struct json_object *check_request = json_object_new_object();
        json_object_object_add(check_request, "command", json_object_new_string("GET_UNREAD_COUNT"));
        struct json_object *check_data = json_object_new_object();
        json_object_object_add(check_data, "session_token", json_object_new_string(g_session_token));
        json_object_object_add(check_request, "data", check_data);
        
        const char *check_json = json_object_to_json_string(check_request);
        send(sock, check_json, strlen(check_json), 0);
        
        char buffer[BUFFER_SIZE];
        int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        buffer[bytes] = '\0';
        
        int unread_count = 0;
        struct json_object *check_response = json_tokener_parse(buffer);
        if (check_response) {
            struct json_object *payload, *count_obj;
            if (json_object_object_get_ex(check_response, "payload", &payload) &&
                json_object_object_get_ex(payload, "unread_count", &count_obj)) {
                unread_count = json_object_get_int(count_obj);
            }
            json_object_put(check_response);
        }
        json_object_put(check_request);
        
        print_separator();
        printf("👥 GROUP MANAGEMENT MENU\n");
        print_separator();
        printf("User: %s (ID: %d)\n", g_username, g_user_id);
        if (unread_count > 0) {
            printf("🔴 You have %d unread notification%s!\n", unread_count, unread_count > 1 ? "s" : "");
        }
        printf("\n");
        
        printf("1.  Create Group\n");
        printf("2.  List My Groups\n");
        printf("3.  List Group Members\n");
        printf("4.  Request Join Group\n");
        printf("5.  List Join Requests (Admin)\n");
        printf("6.  Approve/Reject Join Request (Admin)\n");
        printf("7.  Invite to Group (Admin)\n");
        printf("8.  List My Invitations\n");
        printf("9.  Respond to Invitation\n");
        printf("10. Leave Group\n");
        printf("11. Remove Member (Admin)\n");
        printf("12. 🔔 View All Notifications%s\n", unread_count > 0 ? " 🔴" : "");
        printf("13. Mark Notification as Read\n");
        printf("14. Mark All as Read\n");
        printf("0.  Back to Main Menu\n");
        print_separator();
        
        int choice;
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();
        
        switch (choice) {
            case 1: send_create_group_request(sock); break;
            case 2: send_list_my_groups_request(sock); break;
            case 3: send_list_group_members_request(sock); break;
            case 4: send_request_join_group_request(sock); break;
            case 5: send_list_join_requests_request(sock); break;
            case 6: send_approve_join_request_request(sock); break;
            case 7: send_invite_to_group_request(sock); break;
            case 8: send_list_my_invitations_request(sock); break;
            case 9: send_respond_invitation_request(sock); break;
            case 10: send_leave_group_request(sock); break;
            case 11: send_remove_member_request(sock); break;
            case 12: send_get_notifications_request(sock); break;
            case 13: send_mark_notification_read_request(sock); break;
            case 14: 
                // Mark all as read
                {
                    struct json_object *mark_all = json_object_new_object();
                    json_object_object_add(mark_all, "command", json_object_new_string("MARK_ALL_NOTIFICATIONS_READ"));
                    struct json_object *mark_data = json_object_new_object();
                    json_object_object_add(mark_data, "session_token", json_object_new_string(g_session_token));
                    json_object_object_add(mark_all, "data", mark_data);
                    
                    const char *mark_json = json_object_to_json_string(mark_all);
                    send(sock, mark_json, strlen(mark_json), 0);
                    json_object_put(mark_all);
                    
                    char resp[BUFFER_SIZE];
                    bytes = recv(sock, resp, BUFFER_SIZE - 1, 0);
                    resp[bytes] = '\0';
                    parse_and_display_response(resp);
                    wait_for_enter();
                }
                break;
            case 0: return;
            default:
                print_error("Invalid choice!");
                wait_for_enter();
        }
    }
}

