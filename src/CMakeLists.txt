# 当前文件夹下所有源文件定义为变量SRC_LIST
aux_source_directory(. SRC_LIST)
# 编译SRC_LIST生成名为mprpc的静态库
add_library(mprpc ${SRC_LIST})
# 设置依赖的库
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)