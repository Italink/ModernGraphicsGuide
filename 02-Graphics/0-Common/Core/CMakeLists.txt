cmake_minimum_required(VERSION 3.12)

file(GLOB_RECURSE PROJECT_SOURCE FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.qrc)

add_library(Core ${PROJECT_SOURCE})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_SOURCE})

target_link_libraries(Core PUBLIC 
    Qt${QT_VERSION_MAJOR}::Widgets 
    Qt${QT_VERSION_MAJOR}::WidgetsPrivate 
    Qt${QT_VERSION_MAJOR}::CorePrivate 
    Qt${QT_VERSION_MAJOR}::ShaderToolsPrivate
)