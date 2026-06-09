cmake_minimum_required(VERSION 3.20)
project(SwapCups LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# wxWidgets (Homebrew provides wx-config; the legacy module locates it).
find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

add_executable(SwapCups MACOSX_BUNDLE
    src/SwapCupsApp.cpp
    src/SwapCupsFrame.cpp
    src/GamePanel.cpp
)

target_include_directories(SwapCups PRIVATE src)
target_link_libraries(SwapCups PRIVATE ${wxWidgets_LIBRARIES})
