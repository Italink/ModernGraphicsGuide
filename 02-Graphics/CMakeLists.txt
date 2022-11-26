cmake_minimum_required(VERSION 3.12)

project(GraphicsNinja CXX)

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

function(initlize_common_libaray)
    add_subdirectory(0-Common/Core)
    add_subdirectory(0-Common/3rdParty/QDetailWidget)
    get_all_targets(COMMON_LOCAL_TARGETS)
    list(APPEND COMMON_TARGETS ${COMMON_LOCAL_TARGETS})
    foreach(TARGET_ITEM ${COMMON_LOCAL_TARGETS})
        set_target_properties(${TARGET_ITEM} PROPERTIES FOLDER "0-Common")
    endforeach()
endfunction()

function(add_example EXAMPLE_PATH)
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_PATH} NAME)
    file(GLOB_RECURSE PROJECT_SOURCE FILES ${EXAMPLE_PATH}/*.h  ${EXAMPLE_PATH}/*.cpp ${EXAMPLE_PATH}/*.qrc)
    add_executable(${EXAMPLE_NAME} 
        ${PROJECT_SOURCE}
    )
    target_compile_definitions(${EXAMPLE_NAME} PRIVATE PROJECT_PATH="${EXAMPLE_PATH}")
    target_link_libraries(${EXAMPLE_NAME} PRIVATE Core )
endfunction()

function(add_example_dir DIR_PATH)
    get_filename_component(DIR_NAME ${DIR_PATH} NAME)
    file(GLOB EXAMPLE_LIST RELATIVE ${DIR_PATH} ${DIR_PATH}/*)
    foreach(EXAMPLE_NAME ${EXAMPLE_LIST})
        if(IS_DIRECTORY ${DIR_PATH}/${EXAMPLE_NAME})
            add_example(${DIR_PATH}/${EXAMPLE_NAME})
            set_target_properties(${EXAMPLE_NAME} PROPERTIES FOLDER ${DIR_NAME})
        endif()
    endforeach()
endfunction()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets ShaderTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets ShaderTools REQUIRED)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

initlize_common_libaray()

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")

add_definitions(-DVK_NO_PROTOTYPES)

add_example_dir(${CMAKE_CURRENT_SOURCE_DIR}/1-Basis)
add_example_dir(${CMAKE_CURRENT_SOURCE_DIR}/2-Advance)