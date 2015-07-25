#头文件目录
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/DomiEngine
					${CMAKE_SOURCE_DIR}/DomiServer
					${CMAKE_SOURCE_DIR}/3rd/include/libevent
					${CMAKE_SOURCE_DIR}/3rd/include/libevent/include
					${CMAKE_SOURCE_DIR}/3rd/include/libevent/compat
					${CMAKE_SOURCE_DIR}/3rd/include/libevent/WIN32-Code
					${CMAKE_SOURCE_DIR}/3rd/include
)

#message("------------------------- ${CMAKE_SOURCE_DIR} -------------------------")

IF(WIN32)
    IF(MSVC)
#        SET(CMAKE_CXX_FLAGS "-DWIN32_LEAN_AND_MEAN -D_WIN32_WINNT=0x0501 -EHsc")
#        SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
#        SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    ENDIF(MSVC)
ENDIF(WIN32)


IF(WIN32)
  IF(MSVC)
    SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/../Output/bin)
  ENDIF()
ELSEIF(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")

    SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../Output/bin/${CMAKE_BUILD_TYPE}/)
ENDIF()

#server 所有源文件
file (GLOB_RECURSE all_src "*.h" "*.cpp")

#logic 部分
#logic_net
file (GLOB_RECURSE logicnet_src "logic/logicnet/*.h" "logic/logicnet/*.cpp")
source_group("logic\\logicnet"  FILES ${logicnet_src})

#main 部分
file (GLOB_RECURSE main_src "main/*.h" "main/*.cpp")
source_group("main"  FILES ${main_src})

add_executable(domi_srv ${all_src})

#lib
file (GLOB_RECURSE dependence_lib "${CMAKE_SOURCE_DIR}/3rd/lib/*.lib")
set(LIBS engine)

# Winsock.
if(WIN32)
    set(WIN_LIB  ws2_32.lib wsock32.lib)
endif()

list(APPEND LIBS ${dependence_lib} ${WIN_LIB})

TARGET_LINK_LIBRARIES(domi_srv ${LIBS})

#LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/DomiEngine/enginelib)
