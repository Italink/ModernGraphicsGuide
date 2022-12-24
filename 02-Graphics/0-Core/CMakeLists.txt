cmake_minimum_required(VERSION 3.12)

set(Au)

file(GLOB_RECURSE PROJECT_SOURCE FILES ${CMAKE_CURRENT_SOURCE_DIR}/Src/*.h  ${CMAKE_CURRENT_SOURCE_DIR}/Src/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/Src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Src/*.qrc)

add_library(Core STATIC ${PROJECT_SOURCE})

set_property(TARGET Core PROPERTY AUTOMOC ON)

target_include_directories(Core PUBLIC ./Src/)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_SOURCE})

add_subdirectory(3rdParty/assimp)

add_subdirectory(3rdParty/QDetailWidget)

target_link_libraries(Core PUBLIC 
    Qt::Gui
    Qt::GuiPrivate
    Qt::Widgets
    Qt::WidgetsPrivate
    Qt::ShaderToolsPrivate
    QDetailWidget
    assimp::assimp
)

function(get_all_targets OUT_LIST)
    set(TARGETS)
    get_all_targets_internal(TARGETS ${CMAKE_CURRENT_SOURCE_DIR})
    set(${OUT_LIST} ${TARGETS} PARENT_SCOPE)
endfunction()

macro(get_all_targets_internal TARGETS DIR)
    get_property(SUB_DIR DIRECTORY ${DIR} PROPERTY SUBDIRECTORIES)
    foreach(subdir ${SUB_DIR})
        get_all_targets_internal(${TARGETS} ${subdir})
    endforeach()
    get_property(CURRENT_TARGETS DIRECTORY ${DIR} PROPERTY BUILDSYSTEM_TARGETS)
    list(APPEND ${TARGETS} ${CURRENT_TARGETS})
endmacro()

get_all_targets(COMMON_LOCAL_TARGETS)
list(APPEND COMMON_TARGETS ${COMMON_LOCAL_TARGETS})
foreach(TARGET_ITEM ${COMMON_LOCAL_TARGETS})
    set_target_properties(${TARGET_ITEM} PROPERTIES FOLDER "0-Core")
endforeach()