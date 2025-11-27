cmake_minimum_required(VERSION 3.16)
project(Alexandria LANGUAGES CXX)

# Basic language / standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Defaults matching the Makefile
set(EVALFILE "nn.net" CACHE STRING "NN eval file (mirrors Makefile EVALFILE)")
set(NETWORK_NAME "Alexandria" CACHE STRING "Network name (mirrors Makefile NETWORK_NAME)")

option(USE_NATIVE "Enable -march=native" ON)
option(USE_AVX2 "Enable AVX2 feature flags" OFF)
option(USE_BMI2 "Enable BMI2 feature flags" OFF)
option(USE_AVX512 "Enable AVX512 feature flags" OFF)
option(USE_VNNI512 "Enable VNNI512 feature flags" OFF)
option(DEBUG_SANITIZERS "Build with sanitizers (debug target)" OFF)

# Gather sources
file(GLOB_RECURSE SOURCES src/*.cpp)
if(NOT SOURCES)
  message(FATAL_ERROR "No source files found in src/. Make sure you're in the project root.")
endif()

# Compiler-specific warning flags (from Makefile)
set(WARNING_FLAGS
  -Wall -Wcast-qual -Wextra -Wshadow -Wdouble-promotion -Wformat=2
  -Wnull-dereference -Wlogical-op -Wold-style-cast -Wundef -pedantic)

# Base flags: keep behavior similar to the Makefile
if(DEBUG_SANITIZERS)
  set(BASE_FLAGS -O3 -g3 -fno-omit-frame-pointer -std=gnu++2a -fsanitize=address -fsanitize=leak -fsanitize=undefined)
else()
  set(BASE_FLAGS -funroll-loops -O3 -fno-exceptions -std=gnu++2a -DNDEBUG)
endif()

# Feature mappings
set(FEATURE_FLAGS)
if(USE_NATIVE)
  list(APPEND FEATURE_FLAGS -march=native)
endif()
if(USE_AVX2)
  list(APPEND FEATURE_FLAGS -DUSE_AVX2 -DUSE_SIMD -mavx2 -mbmi -mfma)
endif()
if(USE_BMI2)
  list(APPEND FEATURE_FLAGS -DUSE_AVX2 -DUSE_SIMD -mavx2 -mbmi -mbmi2 -mfma)
endif()
if(USE_AVX512)
  list(APPEND FEATURE_FLAGS -DUSE_AVX512 -DUSE_SIMD -mavx512f -mavx512bw -mfma)
endif()
if(USE_VNNI512)
  list(APPEND FEATURE_FLAGS -DUSE_VNNI512 -DUSE_AVX512 -DUSE_SIMD -mavx512f -mavx512bw -mavx512vnni -mfma)
endif()

# Create executable target - compile all sources in a single command without object files
add_executable(${PROJECT_NAME} ${SOURCES})

# Disable object file generation by setting a custom compiler command
# This mimics the Makefile behavior of compiling all .cpp files directly to executable
set_target_properties(${PROJECT_NAME} PROPERTIES
  COMPILE_FLAGS ""
  LINKER_LANGUAGE CXX
)

# Ensure headers in `src/` resolve correctly
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_compile_options(${PROJECT_NAME} PRIVATE ${BASE_FLAGS} ${WARNING_FLAGS} ${FEATURE_FLAGS})
target_compile_definitions(${PROJECT_NAME} PRIVATE NETWORK_NAME=\"${NETWORK_NAME}\" EVALFILE=\"${EVALFILE}\")

# Linker flags: pthread on non-Windows platforms similar to Makefile
if(NOT WIN32)
  target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
endif()

# Provide a small PGO helper target and instructions
find_program(LLVM_PROFDATA llvm-profdata)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(PGO_GEN_FLAG -fprofile-instr-generate)
  set(PGO_USE_FLAG -fprofile-instr-use=alexandria.profdata)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  set(PGO_GEN_FLAG -fprofile-generate)
  set(PGO_USE_FLAG -fprofile-use)
endif()

add_custom_target(pgo
  COMMENT "PGO: build with profile generation, run bench, merge and build with profile use (manual step may be required)"
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -E echo "Run these steps manually in ${CMAKE_BINARY_DIR}:"
  COMMAND ${CMAKE_COMMAND} -E echo "1) Build with profile generation: cmake --build . --target ${PROJECT_NAME} -- -DCMAKE_CXX_FLAGS=\"${PGO_GEN_FLAG}\""
  COMMAND ${CMAKE_COMMAND} -E echo "2) Run: ./$(PROJECT_NAME) bench"
  COMMAND ${CMAKE_COMMAND} -E echo "3) Merge / move generated profiles (clang: llvm-profdata merge -output=alexandria.profdata *.profraw)"
  COMMAND ${CMAKE_COMMAND} -E echo "4) Rebuild with profile use: cmake --build . --target ${PROJECT_NAME} -- -DCMAKE_CXX_FLAGS=\"${PGO_USE_FLAG}\""
)

# Helpful cache defaults to replicate Makefile behavior
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (Release/Debug)")

message(STATUS "Alexandria CMake helper: build type=${CMAKE_BUILD_TYPE} native=${USE_NATIVE} avx2=${USE_AVX2} debug_sanitizers=${DEBUG_SANITIZERS}")
