add_library(SAP_VFS STATIC)

target_sources(SAP_VFS
        PUBLIC
        vfs.hpp
        vfs_entry.hpp
        PRIVATE
        vfs.cpp
        vfs_entry.cpp
)

target_include_directories(SAP_VFS
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE
        external/cityhash
)
target_link_libraries(SAP_VFS
        PUBLIC
        SAP_Structs
        SAP_Util
        SAP_Reader
        PRIVATE
        CITY_HASH
)