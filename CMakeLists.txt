cmake_minimum_required(VERSION 3.15)
project(fltk_app VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find FLTK package
find_package(FLTK CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)

# Add executable
add_executable(${PROJECT_NAME} WIN32
    src/main.cpp
    src/image_list_table.cpp
)

# Add library path for HCNetSDK
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/incEn
)

# Link HCNetSDK libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/HCNetSDK.lib
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/HCCore.lib
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/PlayCtrl.lib
    fltk fltk_gl fltk_forms fltk_images CURL::libcurl
    pugixml::shared pugixml::pugixml
    unofficial::sqlite3::sqlite3
)


# Set Windows subsystem to Windows for GUI application
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Set output directories
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/incEn
    ${CMAKE_CURRENT_SOURCE_DIR}/include
) 