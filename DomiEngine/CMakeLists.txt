
#预处理器定义
add_definitions(-D _SCL_SECURE_NO_WARNINGS)

#头文件目录
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/DomiEngine
					${CMAKE_SOURCE_DIR}/3rd/include/libevent
					${CMAKE_SOURCE_DIR}/3rd/include/libevent/include
					${CMAKE_SOURCE_DIR}/3rd/include/libevent/compat
					${CMAKE_SOURCE_DIR}/3rd/include/libevent/WIN32-Code
					${CMAKE_SOURCE_DIR}/3rd/include
)

#lib
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/3rd/lib)

IF(WIN32)
    IF(MSVC)
#        SET(CMAKE_CXX_FLAGS "-DWIN32_LEAN_AND_MEAN -D_WIN32_WINNT=0x0501 -EHsc")
#        SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
#        SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    ENDIF(MSVC)
ENDIF(WIN32)


IF(WIN32)
  IF(MSVC)
    SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/Output/lib)
  ENDIF()
ELSEIF(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")

    SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/Output/lib/${CMAKE_BUILD_TYPE}/)
ENDIF()

#common
file (GLOB_RECURSE common_src "common/*.h" "common/*.cpp")

file (GLOB_RECURSE basic_src "common/basic/*.h" "common/basic/*.cpp")
source_group("common\\basic"  FILES ${basic_src})

file (GLOB_RECURSE log_src "common/log/*.h" "common/log/*.cpp")
source_group("common\\log"  FILES ${log_src})

file (GLOB_RECURSE pool_src "common/pool/*.h" "common/pool/*.cpp")
source_group("common\\pool"  FILES ${pool_src})

file (GLOB_RECURSE serverroot_src "common/serverroot/*.h" "common/serverroot/*.cpp")
source_group("common\\serverroot"  FILES ${serverroot_src})

file (GLOB_RECURSE singleton_src "common/singleton/*.h" "common/singleton/*.cpp")
source_group("common\\singleton"  FILES ${singleton_src})

file (GLOB_RECURSE stream_src "common/stream/*.h" "common/stream/*.cpp")
source_group("common\\stream"  FILES ${stream_src})

file (GLOB_RECURSE thread_src "common/thread/*.h" "common/thread/*.cpp")
source_group("common\\thread"  FILES ${thread_src})

file (GLOB_RECURSE timer_src "common/timer/*.h" "common/timer/*.cpp")
source_group("common\\timer"  FILES ${timer_src})

# database
# 暂无

# network
file (GLOB_RECURSE network_src "network/*.h" "network/*.cpp" "network/*.cc")
source_group("network"  FILES ${network_src})

add_library(engine ${common_src} ${network_src})