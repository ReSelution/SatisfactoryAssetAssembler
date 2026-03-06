

add_library(SAA_Reader STATIC
        file_reader.cpp
        file_reader.hpp)

# Header files
target_sources(SAA_Reader
        PUBLIC
        io_global_data.hpp
        IOStoreReader.hpp
        PAKReader.hpp
        Reader.hpp
        UsmapReader.hpp
)

# Source
target_sources(SAA_Reader
        PRIVATE
        IOStoreReader.cpp
        PAKReader.cpp
        Reader.cpp
        UsmapReader.cpp
        io_global_data.cpp
)

target_include_directories(SAA_Reader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(SAA_Reader PUBLIC
        SAA_Lib
        StructuredChaos
)