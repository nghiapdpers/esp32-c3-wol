cmake_minimum_required(VERSION 3.10)
project(PCAgent)

set(CMAKE_CXX_STANDARD 17)

# Cấu hình Mosquitto (Yêu cầu đã cài đặt libmosquitto)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MOSQUITTO REQUIRED mosquitto)

include_directories(${MOSQUITTO_INCLUDE_DIRS})
link_directories(${MOSQUITTO_LIBRARY_DIRS})

add_executable(pc_agent main.cpp)
target_link_libraries(pc_agent ${MOSQUITTO_LIBRARIES})

# Nếu biên dịch trên Windows, cần thêm thư viện Iphlpapi
if(WIN32)
    target_link_libraries(pc_agent iphlpapi)
endif()
