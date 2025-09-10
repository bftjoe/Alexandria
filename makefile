_THIS       := $(realpath $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
_ROOT       := $(_THIS)
CXX         := g++
TARGET      := fchess
WARNINGS     = -Wall -Wcast-qual -Wextra -Wshadow -Wdouble-promotion -Wformat=2 -Wnull-dereference -Wlogical-op -Wold-style-cast -Wundef -pedantic
CXXFLAGS    := -funroll-loops -O3 -flto -flto-partition=one -fno-exceptions -std=c++2c -DNDEBUG $(WARNINGS)
NATIVE       = -march=native
AVX2FLAGS    = -DUSE_AVX2 -DUSE_SIMD -mavx2 -mbmi -mfma
BMI2FLAGS    = -DUSE_AVX2 -DUSE_SIMD -mavx2 -mbmi -mbmi2 -mfma
AVX512FLAGS  = -DUSE_AVX512 -DUSE_SIMD -mavx512f -mavx512bw -mfma
VNNI512FLAGS = -DUSE_VNNI512 -DUSE_AVX512 -DUSE_SIMD -mavx512f -mavx512bw -mavx512vnni -mfma

# engine name
NAME        := fchess

TMPDIR = .tmp

# Detect Clang
ifeq ($(CXX), clang++)
CXXFLAGS = -funroll-loops -O3 -flto -fuse-ld=lld -fno-exceptions -std=c++2c -DNDEBUG
endif

# Detect Windows
ifeq ($(OS), Windows_NT)
	MKDIR   := mkdir
else
ifeq ($(COMP), MINGW)
	MKDIR   := mkdir
else
	MKDIR   := mkdir -p
endif
endif

# Detect Windows
ifeq ($(OS), Windows_NT)
	uname_S  := Windows
	SUFFIX   := .exe
	CXXFLAGS += -static
else
	FLAGS    = -pthread
	SUFFIX  :=
	uname_S := $(shell uname -s)
endif

# Different native flag for macOS
ifeq ($(uname_S), Darwin)
	NATIVE = -mcpu=apple-a14	
	FLAGS = 
endif

FLAGS_DETECTED = 
PROPERTIES = $(shell echo | $(CXX) -march=native -E -dM -)

ifneq ($(findstring __AVX2__, $(PROPERTIES)),)
	FLAGS_DETECTED = $(AVX2FLAGS)
endif

ifneq ($(findstring __BMI2__, $(PROPERTIES)),)
	FLAGS_DETECTED = $(BMI2FLAGS)
endif

ifneq ($(findstring __AVX512F__, $(PROPERTIES)),)
	ifneq ($(findstring __AVX512BW__, $(PROPERTIES)),)
		FLAGS_DETECTED = $(AVX512FLAGS)
	endif
endif

ifneq ($(findstring __AVX512VNNI__, $(PROPERTIES)),)
	FLAGS_DETECTED = $(VNNI512FLAGS)
endif

# Remove native for builds
ifdef build
	NATIVE =
else
	CXXFLAGS += $(FLAGS_DETECTED)
endif

# SPECIFIC BUILDS
ifeq ($(build), native)
	NATIVE     = -march=native
	ARCH       = -x86-64-native
	CXXFLAGS += $(FLAGS_DETECTED)
endif

ifeq ($(build), x86-64)
	NATIVE       = -mtune=znver1
	INSTRUCTIONS = -msse -msse2 -mpopcnt
	ARCH         = -x86-64
endif

ifeq ($(build), x86-64-modern)
	NATIVE       = -mtune=znver2
	INSTRUCTIONS = -m64 -msse -msse3 -mpopcnt
	ARCH         = -x86-64-modern
endif

ifeq ($(build), x86-64-avx2)
	NATIVE    = -march=bdver4 -mno-tbm -mno-sse4a -mno-bmi2
	ARCH      = -x86-64-avx2
	CXXFLAGS += $(AVX2FLAGS)
endif

ifeq ($(build), x86-64-bmi2)
	NATIVE    = -march=haswell
	ARCH      = -x86-64-bmi2
	CXXFLAGS += $(BMI2FLAGS)
endif

ifeq ($(build), x86-64-avx512)
	NATIVE    = -march=x86-64-v4 -mtune=znver4
	ARCH      = -x86-64-avx512
	CXXFLAGS += $(AVX512FLAGS)
endif

ifeq ($(build), x86-64-vnni512)
	NATIVE    = -march=x86-64-v4 -mtune=znver4
	ARCH      = -x86-64-vnni512
	CXXFLAGS += $(VNNI512FLAGS)
endif

ifeq ($(build), debug)
	CXXFLAGS = -O3 -g3 -fno-omit-frame-pointer -std=c++2c -fsanitize=address -fsanitize=leak -fsanitize=undefined
	NATIVE   = -msse -msse3 -mpopcnt
	FLAGS    = -lpthread -lstdc++
	CXXFLAGS += $(FLAGS_DETECTED)
endif

# valgrind doesn't like avx512 code
ifeq ($(build), debug-avx2)
	CXXFLAGS = -O3 -g3 -fno-omit-frame-pointer -std=c++2c -fsanitize=address -fsanitize=leak -fsanitize=undefined
	NATIVE   = -msse -msse3 -mpopcnt
	FLAGS    = -lpthread -lstdc++
	CXXFLAGS += $(AVX2FLAGS)
endif

EXE := $(NAME)$(SUFFIX)

all:
	$(CXX) $(CXXFLAGS) $(NATIVE) -o $(EXE) src/attack.cpp src/history.cpp src/init.cpp src/io.cpp src/main.cpp src/makemove.cpp src/movegen.cpp src/movepicker.cpp src/nnue.cpp src/position.cpp src/search.cpp src/ttable.cpp src/uci.cpp $(FLAGS)

clean:
	@rm -rf $(NAME)$(SUFFIX)
