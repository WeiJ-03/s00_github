# do not include test target
set(FLATBUFFERS_BUILD_TESTS OFF CACHE BOOL "")

# add flatbuffers dir
add_subdirectory(third_party/flatbuffers EXCLUDE_FROM_ALL)

# get the flatc binary
set(FLATBUFFERS_FLATC_EXECUTABLE $<TARGET_FILE:flatc>)

# use flatc
function(compile_flatbuffers_schema SRC_FBS)
    string(REGEX REPLACE "\\.fbs$" "_generated.h" FLATBUFFERS_GEN ${SRC_FBS})
    get_filename_component(FLATBUFFERS_DIR ${SRC_FBS} DIRECTORY)
    add_custom_command(
            OUTPUT ${FLATBUFFERS_GEN}
            COMMAND "${FLATBUFFERS_FLATC_EXECUTABLE}" -c -o ${FLATBUFFERS_DIR} ${SRC_FBS}
            DEPENDS flatc ${SRC_FBS})
    set(FLATBUFFERS_INC ${FLATBUFFERS_INC} ${FLATBUFFERS_GEN} PARENT_SCOPE)
endfunction()
