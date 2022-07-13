cmake_minimum_required(VERSION 3.21)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(bing_wallpaper_daemon)

add_executable(bing_wallpaper_daemon
  src/main.cpp
)

include(FetchContent)

# CPR - C++ Curl bindings
FetchContent_Declare(cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG 6ea2dec23c3df14ac3b27b7d2d6bbff0cb7ba1b0
)
FetchContent_MakeAvailable(cpr)
target_link_libraries(bing_wallpaper_daemon PRIVATE cpr::cpr)

# Nlohmann json
FetchContent_Declare(json
  URL https://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz
)
FetchContent_MakeAvailable(json)
target_link_libraries(bing_wallpaper_daemon PRIVATE nlohmann_json::nlohmann_json)
