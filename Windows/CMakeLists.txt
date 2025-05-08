cmake_minimum_required(VERSION 3.12)
project(WinForklift)

# Manually set path to OpenCVConfig.cmake if needed
#set(OpenCV_DIR "C:/Users/alecb/Desktop/opencv/build")  # Uncomment and adjust if needed

find_package(OpenCV REQUIRED)
set(OpenCV_RUNTIME vc17)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories("./header")

# Gather source files from multiple folders
file(GLOB SOURCES
    "${CMAKE_SOURCE_DIR}/source/*.cpp"
    "${CMAKE_SOURCE_DIR}/headers/*.cpp"
    "${CMAKE_SOURCE_DIR}/include/*.cpp"
)

# Create the executable
add_executable(WinForklift ${SOURCES})

# Link OpenCV libraries
target_link_libraries(WinForklift PRIVATE ${OpenCV_LIBS})

# Set include directories scoped to the target
target_include_directories(WinForklift PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/include
)

