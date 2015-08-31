# Retrieve all source files

include_directories(SYSTEM
  ${LIBSNDFILE_INCLUDE_DIR}
)

include_directories(
  ${ZENGARDEN_INCLUDE_DIR}
)

# Can't be bothered manually adding files!
file(GLOB
     ZENGARDEN_LIB_SRC
     *.cpp
)
list(REMOVE_ITEM
     ZENGARDEN_LIB_SRC
     ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# Expose variables to parent CMake files
set(ZENGARDEN_LIB_SRC
    ${ZENGARDEN_LIB_SRC}
    PARENT_SCOPE
)

file(GLOB
     ZENGARDEN_LIB_HDR
     *.h
)

set(ZENGARDEN_LIBSNDFILE_HDR
  ${LIBSNDFILE_INCLUDE_DIR}/sndfile.h
)

# Expose variables to parent CMake files
set(ZENGARDEN_LIB_HDR
    ${ZENGARDEN_LIB_HDR}
    PARENT_SCOPE
)

# Target
add_library(zengarden_lib
  ${ZENGARDEN_LIB_SRC}
  ${ZENGARDEN_LIB_HDR}
  ${ZENGARDEN_LIBSNDFILE_HDR}
)

add_executable(zengarden
  main.cpp
)

target_link_libraries(zengarden
  zengarden_lib
  ${LIBSNDFILE_LIBRARIES}
)
