import socket

# Cấu hình server
SERVER_IP = "192.168.0.100"
SERVER_PORT = 1234

# Tạo socket TCP
client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    # Kết nối đến server
    client.connect((SERVER_IP, SERVER_PORT))
    print(f"Đã kết nối tới {SERVER_IP}:{SERVER_PORT}")

    # Gửi string (cần mã hóa thành bytes)
    message = ">0,-0\n"
    client.sendall(message.encode('utf-8'))
    print(f"Đã gửi: {message}")

    # Nhận phản hồi (nếu server gửi lại)
    response = client.recv(1024)
    print(f"Nhận phản hồi: {response.decode('utf-8')}")

except ConnectionRefusedError:
    print(f"Lỗi: Không thể kết nối tới {SERVER_IP}:{SERVER_PORT}. Server có đang chạy không?")
except Exception as e:
    print(f"Lỗi: {e}")
finally:
    client.close()
    print("Đã đóng kết nối")