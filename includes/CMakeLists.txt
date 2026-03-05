Set(LIBRARY_SOURCES
    LRUCache.cc
)

add_library(Library STATIC ${LIBRARY_SOURCES})
target_include_directories(Library PUBLIC "./")
