# 要求的 CMake 最低版本（根据你的系统安装的 CMake 版本调整，3.10 是比较通用的版本）
cmake_minimum_required(VERSION 3.10)

# 项目名称（可自定义，这里用 inOneWeekend，与之前的可执行文件名保持一致）
project(inOneWeekend)

# 指定 C++ 标准（推荐 C++17 及以上，支持更多现代特性）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # 强制使用指定的 C++ 标准，不 fallback 到低版本
set(CMAKE_CXX_EXTENSIONS OFF)        # 禁用编译器特定扩展（如 g++ 的 -std=gnu++17）

# 添加可执行文件：将 main.cpp 编译为名为 inOneWeekend 的可执行程序
add_executable(inOneWeekend main.cpp)