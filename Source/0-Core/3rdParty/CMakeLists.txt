cmake_minimum_required(VERSION 3.12)

add_subdirectory(assimp)

add_subdirectory(QDetailWidget)


add_library(Imgui
    STATIC
    Imgui/imconfig.h
    Imgui/imgui.cpp
    Imgui/imgui.h
    Imgui/imgui_demo.cpp
    Imgui/imgui_draw.cpp
    Imgui/imgui_internal.h
    Imgui/imgui_widgets.cpp
    Imgui/imgui_tables.cpp
    Imgui/imstb_rectpack.h
    Imgui/imstb_textedit.h
    Imgui/imstb_truetype.h
)
target_include_directories(Imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Imgui)

add_library(ImGuizmo
    STATIC
    ImGuizmo/GraphEditor.h
    ImGuizmo/GraphEditor.cpp
    ImGuizmo/GraphEditor.h
    ImGuizmo/GraphEditor.cpp
    ImGuizmo/ImGradient.cpp
    ImGuizmo/ImGradient.h
    ImGuizmo/ImGuizmo.cpp
    ImGuizmo/ImGuizmo.h
    ImGuizmo/ImSequencer.cpp
    ImGuizmo/ImSequencer.h
    ImGuizmo/ImZoomSlider.h
)
target_link_libraries(ImGuizmo PUBLIC Imgui)
target_include_directories(ImGuizmo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo)

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
    set_target_properties(${TARGET_ITEM} PROPERTIES FOLDER "0-Core/3rdParty")
endforeach()
