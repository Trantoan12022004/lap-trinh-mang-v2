## Mục tiêu
Xây dựng ứng dụng chia sẻ file
Client và Server sử dung ngon ngu C, C++
· Phải tự xử lý phần giao tiếp socket
. Client và server phải chạy trên 2 máy khác nhau, chạy bằng wsl(unbuntu)
ip Client:192.168.102.24
ip Server:192.168.102.30
· Client gửi yêu cầu lên server bằng Json 
Ví dụ Cấu trúc bản tin Json:
request:
{
  "command": "LOGIN",
  "data": {
    "username": "toan",
    "password": "123456"
  }
}
response:
{
  "status": 200,
  "code": "SUCCESS_LOGIN",
  "message": "Login successful",
  "payload": {
    "userId": 123,
    "role": "admin"
  }
}

Dữ liệu: Postgres  lưu data user/group/log hoạt động/thông tin file; File system  lưu trữ tài liệu tệp, thư mục chia sẻ.

Các chức năng chính:
- Đăng ký và quản lý tài khoản: 2 điểm
- Đăng nhập và quản lý phiên: 2 điểm
- Kiểm soát quyền truy cập: 2 điểm
- Tạo nhóm chia sẻ: 1 điểm
- Liệt kê danh sách nhóm: 1 điểm
- Liệt kê danh sách thành viên trong nhóm: 1 điểm
- Yêu cầu tham gia một nhóm và phê duyệt: 2 điểm
- Mời tham gia vào nhóm và phê duyệt: 2 điểm
- Rời nhóm: 1 điểm
- Xóa thành viên khỏi nhóm: 1 điểm
- Liệt kê nội dung thư mục: 2 điểm
- Upload/Download file: 2 điểm
- Xử lý file có kích thước lớn bất kỳ: 2 điểm
- Thao tác với file (sửa tên, xóa, copy, di chuyển): 2 điểm
- Thao tác với thư mục(tạo thư mục, sửa tên, xóa ,copy, di chuyển): 3 điểm
- Ghi log hoạt động: 1 điểm
