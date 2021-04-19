cmake_minimum_required(VERSION 3.1)


project(TimerAPI)

set(CMAKE_CXX_STANDARD 11)


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

if(WIN32)
        set(CMAKE_USE_WIN32_THREADS_INIT ON)
else()
	set(THREADS_PREFER_PTHREAD_FLAG ON)


endif()



find_package(Threads REQUIRED)


include_directories(
		
			${PROJECT_SOURCE_DIR}/inc
		
			${PROJECT_SOURCE_DIR}/src
)


file(GLOB all_SRCS
		
	"${PROJECT_SOURCE_DIR}/inc/*.hpp"
        
	"${PROJECT_SOURCE_DIR}/src/*.cpp" 
)


add_executable(TimerAPI ${all_SRCS})

target_link_libraries(TimerAPI Threads::Threads)