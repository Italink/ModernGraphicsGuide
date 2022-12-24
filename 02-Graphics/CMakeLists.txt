cmake_minimum_required(VERSION 3.12)

project(ModernGraphicsGuide CXX)

function(add_example EXAMPLE_PATH)
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_PATH} NAME)
    file(GLOB_RECURSE PROJECT_SOURCE FILES ${EXAMPLE_PATH}/*.h  ${EXAMPLE_PATH}/*.cpp ${EXAMPLE_PATH}/*.qrc)
    add_executable(${EXAMPLE_NAME} 
        ${PROJECT_SOURCE}
    )
    target_compile_definitions(${EXAMPLE_NAME} PRIVATE PROJECT_PATH="${EXAMPLE_PATH}")
    target_link_libraries(${EXAMPLE_NAME} PRIVATE Core)
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

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")

add_definitions(-DVK_NO_PROTOTYPES)

add_subdirectory(0-Core)
add_example_dir(${CMAKE_CURRENT_SOURCE_DIR}/1-Basis)
add_example_dir(${CMAKE_CURRENT_SOURCE_DIR}/2-Advance)