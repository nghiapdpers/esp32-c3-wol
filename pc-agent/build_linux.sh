#!/bin/bash

# Di chuyển vào thư mục của script
cd "$(dirname "$0")"

echo "📦 Đang khởi tạo quá trình build PC Agent cho Linux..."

# Kiểm tra và cài đặt dependencies
if [ -f "requirements.txt" ]; then
    echo "Installing dependencies..."
    pip install -r requirements.txt
else
    pip install paho-mqtt pyinstaller
fi

# Chạy PyInstaller
# --onefile: Đóng gói tất cả vào 1 file duy nhất
# --name: Đặt tên file đầu ra
# src/agent.py: File nguồn
pyinstaller --onefile --name pc_agent_linux src/agent.py

echo "------------------------------------------"
if [ $? -eq 0 ]; then
    echo "✅ Build thành công!"
    echo "📍 File thực thi nằm tại: pc-agent/dist/pc_agent_linux"
    echo "💡 Mẹo: Bạn có thể chạy bằng lệnh: ./dist/pc_agent_linux"
else
    echo "❌ Build thất bại. Vui lòng kiểm tra lỗi bên trên."
fi
