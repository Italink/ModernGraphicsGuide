cmake_minimum_required(VERSION 3.12)

set(Au)

file(GLOB_RECURSE PROJECT_SOURCE FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.qrc)

add_library(Core STATIC ${PROJECT_SOURCE})

set_property(TARGET Core PROPERTY AUTOMOC ON)

target_include_directories(Core PUBLIC .)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_SOURCE})

target_link_libraries(Core PUBLIC 
    Qt::Gui
    Qt::GuiPrivate
    Qt::Widgets
    Qt::WidgetsPrivate
    Qt::ShaderToolsPrivate
    QDetailWidget
    assimp::assimp
)