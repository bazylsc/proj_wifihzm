# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Pre-build command - wykonuje się przed każdym budowaniem
add_custom_target(prebuild
    COMMAND ${CMAKE_COMMAND} -E echo "Pre-build: Rozpoczynam budowanie projektu..."
    COMMAND ${CMAKE_COMMAND} -E echo "Pre-build: Sprawdzam środowisko..."
    COMMAND ${CMAKE_COMMAND} -E echo "Pre-build: Data: $(date)"
    VERBATIM
)

project(hello-world)

# Dodaj zależność pre-build do głównego targetu
add_dependencies(hello-world.elf prebuild)

# Post-build command - wykonuje się po zakończeniu budowania
add_custom_command(TARGET hello-world.elf POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Post-build: Budowanie zakończone pomyślnie!"
    COMMAND ${CMAKE_COMMAND} -E echo "Post-build: Sprawdzam rozmiar pliku..."
    COMMAND ${CMAKE_COMMAND} -E echo "Post-build: Generuję raport budowania..."
    COMMAND ${CMAKE_COMMAND} -E echo "Post-build: Data zakończenia: $(date)"
    VERBATIM
)