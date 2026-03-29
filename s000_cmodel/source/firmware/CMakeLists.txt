#compile_flatbuffers_schema(${CMAKE_CURRENT_SOURCE_DIR}/schema/setup.fbs)
#compile_flatbuffers_schema(${CMAKE_CURRENT_SOURCE_DIR}/schema/cpu.fbs)

#set(BTF_SRC
#        src/setup_parser.cpp
#        src/setup_720_v1.cpp
#        ${FLATBUFFERS_INC})
#
#add_library(btf_internal ${BTF_SRC})
#target_link_libraries(btf_internal flatbuffers)
#target_include_directories(btf_internal PUBLIC src schema)
