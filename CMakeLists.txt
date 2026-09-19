cmake_minimum_required(VERSION 3.22)
project(sc1tosc2 C CXX)

include(FetchContent)

# Same dependency setup as sc2fla-cpp (SupercellFlash pulls everything else).
set(SC2FLA_DEPS_TAG "main" CACHE STRING
    "Git tag used for SupercellFlash + Workshop-Core + SupercellCompression + SupercellTexture")

FetchContent_Declare(WorkshopCore
    GIT_REPOSITORY https://github.com/sc-workshop/Workshop-Core.git
    GIT_TAG ${SC2FLA_DEPS_TAG})
FetchContent_Declare(supercell-compression
    GIT_REPOSITORY https://github.com/sc-workshop/SupercellCompression.git
    GIT_TAG ${SC2FLA_DEPS_TAG})
FetchContent_Declare(supercell-texture
    GIT_REPOSITORY https://github.com/sc-workshop/SupercellTexture.git
    GIT_TAG ${SC2FLA_DEPS_TAG})
FetchContent_Declare(SupercellFlash
    GIT_REPOSITORY https://github.com/sc-workshop/SupercellFlash.git
    GIT_TAG ${SC2FLA_DEPS_TAG}
    GIT_SUBMODULES_RECURSE TRUE)

FetchContent_MakeAvailable(SupercellFlash)

add_executable(sc1tosc2 src/main.cpp)
target_compile_features(sc1tosc2 PRIVATE cxx_std_20)
target_link_libraries(sc1tosc2 PRIVATE supercell::flash)

if(MSVC)
    target_compile_options(sc1tosc2 PRIVATE /Zc:char8_t- /D_USE_MATH_DEFINES /permissive)
    add_compile_options(/Zc:char8_t- /D_USE_MATH_DEFINES /permissive)
endif()
