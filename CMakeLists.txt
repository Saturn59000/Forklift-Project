cmake_minimum_required(VERSION 3.12)
project(PiForklift)

# Optional: manually set OpenCV_DIR if needed
# set(OpenCV_DIR "/usr/local/lib/cmake/opencv4")

find_package(OpenCV REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories("./header")  # Ensure this folder exists

file(GLOB SOURCES
    "${CMAKE_SOURCE_DIR}/source/*.cpp"
)

add_executable(PiForklift ${SOURCES})

target_link_libraries(PiForklift PRIVATE ${OpenCV_LIBS})

target_include_directories(PiForklift PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/header
)
