#头文件目录
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/DomiEngine
					${CMAKE_SOURCE_DIR}/Dependence/include/libevent
					${CMAKE_SOURCE_DIR}/Dependence/include/libevent/include
					${CMAKE_SOURCE_DIR}/Dependence/include/libevent/compat
					${CMAKE_SOURCE_DIR}/Dependence/include/libevent/WIN32-Code
					${CMAKE_SOURCE_DIR}/Dependence/include
)

file(GLOB_RECURSE main_SRCS *.h *.cpp)

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

file (GLOB_RECURSE logic_src "logic/*.h" "logic/*.cpp")
source_group("logic"  FILES ${logic_src})

file (GLOB_RECURSE main_src "main/*.h" "main/*.cpp")
source_group("main"  FILES ${main_src})

add_executable(domi_srv ${main_SRCS})

TARGET_LINK_LIBRARIES(domi_srv engine
	${CMAKE_SOURCE_DIR}/Dependence/lib/libevent.lib
	${CMAKE_SOURCE_DIR}/Dependence/lib/libprotobuf.lib
)
#LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/DomiEngine/enginelib)
