set(cxxopts_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/cxxopts/include)
set(spdlog_include_dir
        ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/include/spdlog/*/*
        ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/include/spdlog/*
        ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/include
        )
set(json_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/json/include)
set(external_include_dir ${cxxopts_include_dir} ${spdlog_include_dir} ${json_include_dir} PARENT_SCOPE)

#file(GLOB cxxopts_source
#        ${CMAKE_CURRENT_SOURCE_DIR}/module/command/*.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/module/kernels/*.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/module/memory/*.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/module/register/*.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/module/nputop/*.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/module/weight/*.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/utils/fixpoint/*.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/utils/src/*.cpp
#        )
