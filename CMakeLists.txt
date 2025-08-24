cmake_minimum_required(VERSION 3.10.0)
project(RTDServer VERSION 0.1.0 LANGUAGES C CXX)

# 设置编译器选项
set(CMAKE_CXX_STANDARD 20)

# 根据编译器架构设置不同的输出名称
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64位
    set(LIBRARY_OUTPUT_NAME "${PROJECT_NAME}64")
else()
    # 32位
    set(LIBRARY_OUTPUT_NAME "${PROJECT_NAME}32")
endif()

# 添加源文件
add_library(${PROJECT_NAME} SHARED main.cpp RTDServer.cpp RtdDll.cpp RTDTopic.cpp RtdDll.def)

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    PREFIX ""
    OUTPUT_NAME ${LIBRARY_OUTPUT_NAME}
)

# 添加依赖库
target_link_libraries(${PROJECT_NAME} PRIVATE wininet)

# 添加测试程序
add_executable(test Test.cpp)

# 设置MSVC编译选项
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /source-charset:utf-8 /execution-charset:utf-8")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /source-charset:utf-8 /execution-charset:utf-8")
endif()

# 设置编译选项
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    
        target_compile_options(${PROJECT_NAME} PRIVATE
            -ffunction-sections
            -fdata-sections
        )

        set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "-flto -Wl,--gc-sections"
        )

        set(CMAKE_EXE_LINKER_FLAGS "-s -static")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s -static")

    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${PROJECT_NAME} PRIVATE /MT)
    endif()
endif()
