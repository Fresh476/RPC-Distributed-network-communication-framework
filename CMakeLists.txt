# cmake最低版本为3.5
cmake_minimum_required(VERSION 3.5) 
# 项目名称为mprpc
project(mprpc)
# 开启gdb选项
set(CMAKE_BUILD_TYPE "Debug")

# 项目可执行文件输出路径:当前目录的bin子目录下
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# 项目库文件输出路径:当前目录的lib子目录下
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)
# 库文件搜索路径
link_directories(${PROJECT_SOURCE_DIR}/lib)

# 向子目录src和example继续构建
add_subdirectory(src)
add_subdirectory(example)

