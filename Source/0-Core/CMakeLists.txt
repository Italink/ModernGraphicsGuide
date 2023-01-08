cmake_minimum_required(VERSION 3.12)

file(GLOB_RECURSE PROJECT_SOURCE FILES ${CMAKE_CURRENT_SOURCE_DIR}/Src/*.h  ${CMAKE_CURRENT_SOURCE_DIR}/Src/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/Src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Src/*.qrc)

add_library(Core STATIC ${PROJECT_SOURCE})

set_property(TARGET Core PROPERTY AUTOMOC ON)

target_include_directories(Core PUBLIC ./Src/)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_SOURCE})

add_subdirectory(3rdParty)

target_link_libraries(Core PUBLIC 
    Qt::Gui
    Qt::GuiPrivate
    Qt::Widgets
    Qt::WidgetsPrivate
    Qt::ShaderToolsPrivate
    QDetailWidget
    assimp::assimp
    Imgui
    ImGuizmo
)

set_target_properties(Core PROPERTIES FOLDER "0-Core")