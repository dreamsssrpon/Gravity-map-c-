# CMakeList.txt: проект CMake для RayLibProjects; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#

#find_package(glm)

# Добавьте источник в исполняемый файл этого проекта.
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ENABLE_CPACK   OFF CACHE BOOL "" FORCE) 
add_subdirectory("external_data/raylib-master")

add_executable (RayLibProjects "RayLibProjects.cpp" "RayLibProjects.h" external_data/glm/glm/glm.hpp)
target_link_libraries(RayLibProjects raylib)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET RayLibProjects PROPERTY CXX_STANDARD 20)
endif()

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
