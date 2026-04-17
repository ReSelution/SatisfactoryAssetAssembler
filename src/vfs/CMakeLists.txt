add_library(SAA_VFS STATIC
        game_file.cpp
        game_file.hpp)

target_sources(SAA_VFS
        PUBLIC
        vfs.hpp
        vfs_entry.hpp
        PRIVATE
        vfs.cpp
        vfs_entry.cpp
)

target_include_directories(SAA_VFS
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE
        external/cityhash
)
target_link_libraries(SAA_VFS
        PUBLIC
        SAA_Structs
        SAA_Util
        SAA_Reader
        PRIVATE
)