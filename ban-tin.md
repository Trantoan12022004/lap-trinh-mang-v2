1. Đăng ký và quản lý tài khoản (2 điểm)
1.1 Đăng ký tài khoản
Request:
{
  "command": "REGISTER",
  "data": {
    "username": "user123",
    "password": "securepass123",
    "email": "user@example.com",
    "full_name": "Nguyen Van A"
  }
}
Response:
{
  "status": 201,
  "code": "SUCCESS_REGISTER",
  "message": "User registered successfully",
  "payload": {
    "user_id": 1,
    "username": "user123",
    "created_at": "2025-11-24T10:30:00Z"
  }
}
1.2 Cập nhật thông tin tài khoản
Request:
{
  "command": "UPDATE_PROFILE",
  "data": {
    "session_token": "abc123xyz",
    "email": "newemail@example.com",
    "full_name": "Nguyen Van B"
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_UPDATE",
  "message": "Profile updated successfully",
  "payload": {
    "user_id": 1,
    "username": "user123",
    "email": "newemail@example.com",
    "full_name": "Nguyen Van B"
  }
}
1.3 Đổi mật khẩu
Request:
{
  "command": "CHANGE_PASSWORD",
  "data": {
    "session_token": "abc123xyz",
    "old_password": "securepass123",
    "new_password": "newsecurepass456"
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_CHANGE_PASSWORD",
  "message": "Password changed successfully",
  "payload": {}
}
1.4 Xóa tài khoản
Request:

Response:

2. Đăng nhập và quản lý phiên (2 điểm)
2.1 Đăng nhập
Request:
{
  "command": "LOGIN",
  "data": {
    "username": "user123",
    "password": "securepass123"
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_LOGIN",
  "message": "Login successful",
  "payload": {
    "user_id": 1,
    "username": "user123",
    "session_token": "abc123xyz",
    "role": "user",
    "expires_at": "2025-11-25T10:30:00Z"
  }
}
2.2 Đăng xuất
Request:
{
  "command": "LOGOUT",
  "data": {
    "session_token": "abc123xyz"
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_LOGOUT",
  "message": "Logout successful",
  "payload": {}
}
2.3 Kiểm tra phiên
Request:
{
  "command": "VERIFY_SESSION",
  "data": {
    "session_token": "abc123xyz"
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_VERIFY_SESSION",
  "message": "Session is valid",
  "payload": {
    "user_id": 1,
    "username": "user123",
    "expires_at": "2025-11-25T10:30:00Z"
  }
}
3. Kiểm soát quyền truy cập (2 điểm)
3.1 Lấy quyền của user trong nhóm
Request:
{
  "command": "GET_PERMISSIONS",
  "data": {
    "session_token": "abc123xyz",
    "group_id": 10
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_GET_PERMISSIONS",
  "message": "Permissions retrieved successfully",
  "payload": {
    "user_id": 1,
    "group_id": 10,
    "can_read": true,
    "can_write": true,
    "can_delete": false,
    "can_manage": false
  }
}
3.2 Cập nhật quyền (dành cho admin nhóm)
Request:
{
  "command": "UPDATE_PERMISSIONS",
  "data": {
    "session_token": "abc123xyz",
    "group_id": 10,
    "target_user_id": 5,
    "can_read": true,
    "can_write": true,
    "can_delete": true,
    "can_manage": false
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_UPDATE_PERMISSIONS",
  "message": "Permissions updated successfully",
  "payload": {
    "user_id": 5,
    "group_id": 10,
    "can_read": true,
    "can_write": true,
    "can_delete": true,
    "can_manage": false
  }
}
4. Tạo nhóm chia sẻ (1 điểm)
Request:
{
  "command": "CREATE_GROUP",
  "data": {
    "session_token": "abc123xyz",
    "group_name": "Project Team",
    "description": "Team for sharing project files"
  }
}
Response:
{
  "status": 201,
  "code": "SUCCESS_CREATE_GROUP",
  "message": "Group created successfully",
  "payload": {
    "group_id": 10,
    "group_name": "Project Team",
    "description": "Team for sharing project files",
    "owner_id": 1,
    "created_at": "2025-11-24T10:30:00Z"
  }
}
5. Liệt kê danh sách nhóm (1 điểm)
5.1 Lấy danh sách nhóm của user
Request:
{
  "command": "LIST_MY_GROUPS",
  "data": {
    "session_token": "abc123xyz"
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_LIST_GROUPS",
  "message": "Groups retrieved successfully",
  "payload": {
    "groups": [
      {
        "group_id": 10,
        "group_name": "Project Team",
        "description": "Team for sharing project files",
        "role": "admin",
        "member_count": 5,
        "created_at": "2025-11-24T10:30:00Z"
      },
      {
        "group_id": 15,
        "group_name": "Study Group",
        "description": "Group for study materials",
        "role": "member",
        "member_count": 10,
        "created_at": "2025-11-20T09:00:00Z"
      }
    ]
  }
}
5.2 Tìm kiếm nhóm
Request:
{
  "command": "SEARCH_GROUPS",
  "data": {
    "session_token": "abc123xyz",
    "keyword": "project"
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_SEARCH_GROUPS",
  "message": "Search completed",
  "payload": {
    "groups": [
      {
        "group_id": 10,
        "group_name": "Project Team",
        "description": "Team for sharing project files",
        "owner_name": "Nguyen Van A",
        "member_count": 5
      }
    ]
  }
}
6. Liệt kê danh sách thành viên trong nhóm (1 điểm)
Request:
{
  "command": "LIST_GROUP_MEMBERS",
  "data": {
    "session_token": "abc123xyz",
    "group_id": 10
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_LIST_MEMBERS",
  "message": "Members retrieved successfully",
  "payload": {
    "group_id": 10,
    "members": [
      {
        "user_id": 1,
        "username": "user123",
        "full_name": "Nguyen Van A",
        "role": "admin",
        "status": "approved",
        "joined_at": "2025-11-24T10:30:00Z"
      },
      {
        "user_id": 5,
        "username": "user456",
        "full_name": "Tran Thi B",
        "role": "member",
        "status": "approved",
        "joined_at": "2025-11-24T11:00:00Z"
      }
    ]
  }
}
7. Yêu cầu tham gia một nhóm và phê duyệt (2 điểm)
7.1 Gửi yêu cầu tham gia
Request:
{
  "command": "REQUEST_JOIN_GROUP",
  "data": {
    "session_token": "abc123xyz",
    "group_id": 10
  }
}
Response:
{
  "status": 201,
  "code": "SUCCESS_REQUEST_JOIN",
  "message": "Join request sent successfully",
  "payload": {
    "request_id": 100,
    "group_id": 10,
    "user_id": 1,
    "status": "pending",
    "created_at": "2025-11-24T10:30:00Z"
  }
}
7.2 Liệt kê yêu cầu tham gia (admin)
Request:
{
  "status": 201,
  "code": "SUCCESS_REQUEST_JOIN",
  "message": "Join request sent successfully",
  "payload": {
    "request_id": 100,
    "group_id": 10,
    "user_id": 1,
    "status": "pending",
    "created_at": "2025-11-24T10:30:00Z"
  }
}
Response:
{
  "status": 201,
  "code": "SUCCESS_REQUEST_JOIN",
  "message": "Join request sent successfully",
  "payload": {
    "request_id": 100,
    "group_id": 10,
    "user_id": 1,
    "status": "pending",
    "created_at": "2025-11-24T10:30:00Z"
  }
}
7.3 Phê duyệt/Từ chối yêu cầu
Request:
{
  "command": "APPROVE_JOIN_REQUEST",
  "data": {
    "session_token": "abc123xyz",
    "request_id": 100,
    "action": "approve"
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_APPROVE_REQUEST",
  "message": "Join request approved successfully",
  "payload": {
    "request_id": 100,
    "user_id": 5,
    "group_id": 10,
    "status": "approved",
    "reviewed_at": "2025-11-24T11:00:00Z"
  }
}
8. Mời tham gia vào nhóm và phê duyệt (2 điểm)
8.1 Gửi lời mời
Request:
{
  "command": "INVITE_TO_GROUP",
  "data": {
    "session_token": "abc123xyz",
    "group_id": 10,
    "invitee_username": "user789"
  }
}
Response:
{
  "status": 201,
  "code": "SUCCESS_SEND_INVITATION",
  "message": "Invitation sent successfully",
  "payload": {
    "invitation_id": 200,
    "group_id": 10,
    "inviter_id": 1,
    "invitee_id": 7,
    "status": "pending",
    "created_at": "2025-11-24T10:30:00Z"
  }
}
8.2 Liệt kê lời mời của user
Request:
{
  "command": "LIST_MY_INVITATIONS",
  "data": {
    "session_token": "abc123xyz"
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_LIST_INVITATIONS",
  "message": "Invitations retrieved successfully",
  "payload": {
    "invitations": [
      {
        "invitation_id": 200,
        "group_id": 10,
        "group_name": "Project Team",
        "inviter_username": "user123",
        "inviter_name": "Nguyen Van A",
        "status": "pending",
        "created_at": "2025-11-24T10:30:00Z"
      }
    ]
  }
}
8.3 Chấp nhận/Từ chối lời mời
Request:
{
  "command": "RESPOND_INVITATION",
  "data": {
    "session_token": "abc123xyz",
    "invitation_id": 200,
    "action": "accept"
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_ACCEPT_INVITATION",
  "message": "Invitation accepted successfully",
  "payload": {
    "invitation_id": 200,
    "group_id": 10,
    "status": "accepted",
    "responded_at": "2025-11-24T11:00:00Z"
  }
}
9. Rời nhóm (1 điểm)
Request:
{
  "command": "LEAVE_GROUP",
  "data": {
    "session_token": "abc123xyz",
    "group_id": 10
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_LEAVE_GROUP",
  "message": "Left group successfully",
  "payload": {
    "group_id": 10,
    "user_id": 1,
    "left_at": "2025-11-24T11:00:00Z"
  }
}
10. Xóa thành viên khỏi nhóm (1 điểm)
Request:
{
  "command": "REMOVE_MEMBER",
  "data": {
    "session_token": "abc123xyz",
    "group_id": 10,
    "target_user_id": 5
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_REMOVE_MEMBER",
  "message": "Member removed successfully",
  "payload": {
    "group_id": 10,
    "removed_user_id": 5,
    "removed_at": "2025-11-24T11:00:00Z"
  }
}
11. Liệt kê nội dung thư mục (2 điểm)
Request:
{
  "command": "LIST_DIRECTORY",
  "data": {
    "session_token": "abc123xyz",
    "group_id": 10,
    "directory_path": "/project/docs"
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_LIST_DIRECTORY",
  "message": "Directory contents retrieved successfully",
  "payload": {
    "group_id": 10,
    "current_path": "/project/docs",
    "directories": [
      {
        "directory_id": 50,
        "directory_name": "reports",
        "directory_path": "/project/docs/reports",
        "created_by": "user123",
        "created_at": "2025-11-20T09:00:00Z"
      }
    ],
    "files": [
      {
        "file_id": 500,
        "file_name": "document.pdf",
        "file_path": "/project/docs/document.pdf",
        "file_size": 2048576,
        "file_type": "application/pdf",
        "uploaded_by": "user123",
        "uploaded_at": "2025-11-24T10:00:00Z"
      }
    ]
  }
}
12. Upload/Download file (2 điểm)
12.1 Bắt đầu upload file
Request:
{
  "command": "UPLOAD_FILE_START",
  "data": {
    "session_token": "abc123xyz",
    "group_id": 10,
    "file_name": "document.pdf",
    "file_size": 2048576,
    "file_type": "application/pdf",
    "directory_path": "/project/docs",
    "chunk_size": 65536
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_UPLOAD_START",
  "message": "Ready to receive file",
  "payload": {
    "upload_id": "upload_123abc",
    "file_id": 500,
    "total_chunks": 32,
    "chunk_size": 65536
  }
}
12.2 Upload chunk
Request:
{
  "command": "UPLOAD_FILE_CHUNK",
  "data": {
    "session_token": "abc123xyz",
    "upload_id": "upload_123abc",
    "chunk_index": 0,
    "chunk_data": "base64_encoded_data..."
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_UPLOAD_CHUNK",
  "message": "Chunk received",
  "payload": {
    "upload_id": "upload_123abc",
    "chunk_index": 0,
    "chunks_received": 1,
    "total_chunks": 32
  }
}
12.3 Hoàn thành upload
Request:
{
  "command": "UPLOAD_FILE_COMPLETE",
  "data": {
    "session_token": "abc123xyz",
    "upload_id": "upload_123abc"
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_UPLOAD_COMPLETE",
  "message": "File uploaded successfully",
  "payload": {
    "file_id": 500,
    "file_name": "document.pdf",
    "file_path": "/project/docs/document.pdf",
    "file_size": 2048576,
    "uploaded_at": "2025-11-24T10:30:00Z"
  }
}
12.4 Bắt đầu download file
Request:
{
  "command": "DOWNLOAD_FILE_START",
  "data": {
    "session_token": "abc123xyz",
    "file_id": 500,
    "chunk_size": 65536
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_DOWNLOAD_START",
  "message": "Ready to send file",
  "payload": {
    "download_id": "download_456def",
    "file_id": 500,
    "file_name": "document.pdf",
    "file_size": 2048576,
    "total_chunks": 32,
    "chunk_size": 65536
  }
}
12.5 Download chunk
Request:
{
  "command": "DOWNLOAD_FILE_CHUNK",
  "data": {
    "session_token": "abc123xyz",
    "download_id": "download_456def",
    "chunk_index": 0
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_DOWNLOAD_CHUNK",
  "message": "Chunk sent",
  "payload": {
    "download_id": "download_456def",
    "chunk_index": 0,
    "chunk_data": "base64_encoded_data...",
    "chunks_sent": 1,
    "total_chunks": 32
  }
}
12.6 Hoàn thành download
Request:
{
  "command": "DOWNLOAD_FILE_COMPLETE",
  "data": {
    "session_token": "abc123xyz",
    "download_id": "download_456def"
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_DOWNLOAD_COMPLETE",
  "message": "File downloaded successfully",
  "payload": {
    "file_id": 500,
    "download_id": "download_456def"
  }
}
13. Thao tác với file (2 điểm)
13.1 Đổi tên file
Request:
{
  "command": "RENAME_FILE",
  "data": {
    "session_token": "abc123xyz",
    "file_id": 500,
    "new_name": "new_document.pdf"
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_RENAME_FILE",
  "message": "File renamed successfully",
  "payload": {
    "file_id": 500,
    "old_name": "document.pdf",
    "new_name": "new_document.pdf",
    "updated_at": "2025-11-24T11:00:00Z"
  }
}
13.2 Xóa file
Request:
{
  "command": "DELETE_FILE",
  "data": {
    "session_token": "abc123xyz",
    "file_id": 500
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_DELETE_FILE",
  "message": "File deleted successfully",
  "payload": {
    "file_id": 500,
    "deleted_at": "2025-11-24T11:00:00Z"
  }
}
13.3 Copy file
Request:
{
  "command": "COPY_FILE",
  "data": {
    "session_token": "abc123xyz",
    "file_id": 500,
    "destination_path": "/project/backup"
  }
}
Response:
{
  "command": "COPY_FILE",
  "data": {
    "session_token": "abc123xyz",
    "file_id": 500,
    "destination_path": "/project/backup"
  }
}
13.4 Di chuyển file
Request:
{
  "command": "MOVE_FILE",
  "data": {
    "session_token": "abc123xyz",
    "file_id": 500,
    "destination_path": "/project/archive"
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_MOVE_FILE",
  "message": "File moved successfully",
  "payload": {
    "file_id": 500,
    "old_path": "/project/docs/document.pdf",
    "new_path": "/project/archive/document.pdf",
    "moved_at": "2025-11-24T11:00:00Z"
  }
}
14. Thao tác với thư mục (3 điểm)
14.1 Tạo thư mục
Request:
{
  "command": "CREATE_DIRECTORY",
  "data": {
    "session_token": "abc123xyz",
    "group_id": 10,
    "directory_name": "reports",
    "parent_path": "/project/docs"
  }
}
Response:
{
  "status": 201,
  "code": "SUCCESS_CREATE_DIRECTORY",
  "message": "Directory created successfully",
  "payload": {
    "directory_id": 50,
    "directory_name": "reports",
    "directory_path": "/project/docs/reports",
    "created_at": "2025-11-24T10:30:00Z"
  }
}
14.2 Đổi tên thư mục
Request:
{
  "command": "RENAME_DIRECTORY",
  "data": {
    "session_token": "abc123xyz",
    "directory_id": 50,
    "new_name": "monthly_reports"
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_RENAME_DIRECTORY",
  "message": "Directory renamed successfully",
  "payload": {
    "directory_id": 50,
    "old_name": "reports",
    "new_name": "monthly_reports",
    "old_path": "/project/docs/reports",
    "new_path": "/project/docs/monthly_reports",
    "updated_at": "2025-11-24T11:00:00Z"
  }
}
14.3 Xóa thư mục
Request:
{
  "command": "DELETE_DIRECTORY",
  "data": {
    "session_token": "abc123xyz",
    "directory_id": 50,
    "recursive": true
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_DELETE_DIRECTORY",
  "message": "Directory deleted successfully",
  "payload": {
    "directory_id": 50,
    "deleted_files": 5,
    "deleted_subdirectories": 2,
    "deleted_at": "2025-11-24T11:00:00Z"
  }
}
14.4 Copy thư mục
Request:
{
  "command": "COPY_DIRECTORY",
  "data": {
    "session_token": "abc123xyz",
    "directory_id": 50,
    "destination_path": "/project/backup"
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_COPY_DIRECTORY",
  "message": "Directory copied successfully",
  "payload": {
    "source_directory_id": 50,
    "new_directory_id": 51,
    "new_directory_path": "/project/backup/reports",
    "copied_files": 5,
    "copied_subdirectories": 2,
    "copied_at": "2025-11-24T11:00:00Z"
  }
}
14.5 Di chuyển thư mục
Request:
{
  "command": "MOVE_DIRECTORY",
  "data": {
    "session_token": "abc123xyz",
    "directory_id": 50,
    "destination_path": "/project/archive"
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_MOVE_DIRECTORY",
  "message": "Directory moved successfully",
  "payload": {
    "directory_id": 50,
    "old_path": "/project/docs/reports",
    "new_path": "/project/archive/reports",
    "affected_files": 5,
    "affected_subdirectories": 2,
    "moved_at": "2025-11-24T11:00:00Z"
  }
}
15. Ghi log hoạt động (1 điểm)
15.1 Lấy log hoạt động của user
Request:
{
  "command": "GET_USER_LOGS",
  "data": {
    "session_token": "abc123xyz",
    "start_date": "2025-11-20T00:00:00Z",
    "end_date": "2025-11-24T23:59:59Z",
    "limit": 50,
    "offset": 0
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_GET_LOGS",
  "message": "Logs retrieved successfully",
  "payload": {
    "logs": [
      {
        "log_id": 1000,
        "user_id": 1,
        "action": "LOGIN",
        "target_type": null,
        "target_id": null,
        "details": "Logged in from 192.168.102.24",
        "ip_address": "192.168.102.24",
        "created_at": "2025-11-24T10:30:00Z"
      },
      {
        "log_id": 1001,
        "user_id": 1,
        "action": "UPLOAD",
        "target_type": "FILE",
        "target_id": 500,
        "details": "Uploaded document.pdf to /project/docs",
        "ip_address": "192.168.102.24",
        "created_at": "2025-11-24T10:35:00Z"
      }
    ],
    "total_count": 125
  }
}
15.2 Lấy log hoạt động của nhóm (admin)
Request:
{
  "command": "GET_GROUP_LOGS",
  "data": {
    "session_token": "abc123xyz",
    "group_id": 10,
    "start_date": "2025-11-20T00:00:00Z",
    "end_date": "2025-11-24T23:59:59Z",
    "limit": 50,
    "offset": 0
  }
}
Response:
{
  "status": 200,
  "code": "SUCCESS_GET_GROUP_LOGS",
  "message": "Group logs retrieved successfully",
  "payload": {
    "group_id": 10,
    "logs": [
      {
        "log_id": 2000,
        "user_id": 1,
        "username": "user123",
        "action": "CREATE_GROUP",
        "target_type": "GROUP",
        "target_id": 10,
        "details": "Created group Project Team",
        "ip_address": "192.168.102.24",
        "created_at": "2025-11-20T09:00:00Z"
      },
      {
        "log_id": 2001,
        "user_id": 5,
        "username": "user456",
        "action": "JOIN_GROUP",
        "target_type": "GROUP",
        "target_id": 10,
        "details": "Joined group Project Team",
        "ip_address": "192.168.102.25",
        "created_at": "2025-11-20T10:00:00Z"
      }
    ],
    "total_count": 50
  }
}
Các mã lỗi chung
{
  "status": 400,
  "code": "ERROR_INVALID_REQUEST",
  "message": "Invalid request format",
  "payload": {}
}
{
  "status": 401,
  "code": "ERROR_UNAUTHORIZED",
  "message": "Invalid session token or session expired",
  "payload": {}
}
{
  "status": 403,
  "code": "ERROR_FORBIDDEN",
  "message": "You don't have permission to perform this action",
  "payload": {}
}
{
  "status": 404,
  "code": "ERROR_NOT_FOUND",
  "message": "Resource not found",
  "payload": {}
}
{
  "status": 409,
  "code": "ERROR_CONFLICT",
  "message": "Resource already exists",
  "payload": {}
}
{
  "status": 500,
  "code": "ERROR_INTERNAL_SERVER",
  "message": "Internal server error",
  "payload": {}
}