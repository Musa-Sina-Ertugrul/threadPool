add_library(thPool ${CMAKE_CURRENT_SOURCE_DIR}/src/ReturnTypes.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/ThreadPool.cpp)
set(srcReturnTypes "${CMAKE_CURRENT_SOURCE_DIR}/include/ReturnTypes.hpp")
set(srcThreadPool "${CMAKE_CURRENT_SOURCE_DIR}/include/ThreadPool.hpp")
add_definitions(-DsrcReturnTypes="${srcReturnTypes}")
add_definitions(-DsrcThreadPool="${srcThreadPool}")
set(srcReturnTypes "${CMAKE_CURRENT_SOURCE_DIR}/include/ReturnTypes.hpp" PARENT_SCOPE)
set(srcThreadPool "${CMAKE_CURRENT_SOURCE_DIR}/include/ThreadPool.hpp" PARENT_SCOPE)

