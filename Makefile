ifeq ($(OS),Windows_NT)
    CC = /mingw32/bin/i686-w64-mingw32-gcc
    CXX = /mingw32/bin/i686-w64-mingw32-g++
    OBJC = /mingw32/bin/i686-w64-mingw32-g++
    LN = /mingw32/bin/i686-w64-mingw32-g++
    STRIP = strip
    FIND = find src -type f -name
    PKGCONFIG = pkg-config
    PROJECT = yomu.exe

    LN_FLAGS += -lv8 -lv8_libplatform
    CPP_FLAGS += -DSCRIPT_ENGINE_V8
    CPP_FLAGS += -D__WINDOWS__

    LN_FLAGS += -lopengl32
    LN_FLAGS += -lglew32

    LN_FLAGS += -lole32
    LN_FLAGS += -lws2_32
    LN_FLAGS += -lcrypt32
    ifeq ($(DEBUG),true)
        LN_FLAGS += -mconsole
    else
        LN_FLAGS += -mwindows
    endif

else
    CC = gcc
    CXX = g++
    OBJC = g++
    LN = g++
    STRIP = strip
    FIND = find src -type f -name
    UNAME_S := $(shell uname -s)
    PROJECT = yomu

    ifeq ($(OLDGCC),true)
        LN_FLAGS += -lstdc++fs
    endif

    ifeq ($(UNAME_S),Linux)
	PKGCONFIG = pkg-config
        CCFLAGS += -D LINUX
        BITS := $(shell getconf LONG_BIT)

        ifneq ("$(wildcard /usr/lib/x86_64-linux-gnu/libv8.so)","")
	    CPP_FLAGS += -I/usr/include/nodejs/deps/v8/include
	    LN_FLAGS += -L/usr/lib/x86_64-linux-gnu
	    LN_FLAGS += -lv8 -lv8_libplatform
	    CPP_FLAGS += -DSCRIPT_ENGINE_V8
            ifneq ($(DEBIANELDERLY),true)
	        CPP_FLAGS += -DV8_COMPRESS_POINTERS
            endif
        endif

        ifneq ("$(wildcard /usr/lib/arm-linux-gnueabihf/libv8.so)","")
	    CPP_FLAGS += -I/usr/include/nodejs/deps/v8/include
	    LN_FLAGS += -L/usr/lib/arm-linux-gnueabihf
	    LN_FLAGS += -lv8 -lv8_libplatform
	    CPP_FLAGS += -DSCRIPT_ENGINE_V8
        endif

        ifeq ($(DEBUG),true)
            FLAGS += -rdynamic # debug build
        endif

	LN_FLAGS += -lGL
	LN_FLAGS += -lX11 -lXi
    endif

    ifeq ($(UNAME_S),Darwin)
	PKGCONFIG = /usr/local/bin/pkg-config
	CPP_FILES += $(shell $(FIND) '*.mm')
        CPP_FLAGS += -DV8_COMPRESS_POINTERS # just assume OSX is 64-bit
	CPP_FLAGS += -DGL_SILENCE_DEPRECATION
	LN_FLAGS += -framework OpenGL
	LN_FLAGS += -framework Foundation
    endif

    # ifneq ($(filter %86,$(UNAME_P)),)
    # endif

endif

LIB_DIRS += $(shell find dependencies -type d)
CPP_FLAGS += $(patsubst %,-I%,$(LIB_DIRS))

SRC_DIRS += $(shell find src -type d)
SRC_DIRS += $(LIB_DIRS)
CPP_FLAGS += -Isrc

CPP_FLAGS += -MMD -MP

CPP_FLAGS += -DCPPHTTPLIB_OPENSSL_SUPPORT
CPP_FLAGS += $(shell $(PKGCONFIG) --cflags libssl)
LN_FLAGS += $(shell $(PKGCONFIG) --libs libssl)
CPP_FLAGS += $(shell $(PKGCONFIG) --cflags libcrypto)
LN_FLAGS += $(shell $(PKGCONFIG) --libs libcrypto)

ifeq ($(BACKEND),SDL1)
    CPP_FLAGS += -DUSE_SDL1
    CPP_FLAGS += $(shell $(PKGCONFIG) --cflags sdl)
    LN_FLAGS += $(shell $(PKGCONFIG) --libs sdl)
    LN_FLAGS += -lSDL_image
endif

ifeq ($(BACKEND),SDL2)
    CPP_FLAGS += -DUSE_SDL2
    CPP_FLAGS += $(shell $(PKGCONFIG) --cflags sdl2)
    LN_FLAGS += $(shell $(PKGCONFIG) --libs sdl2)
    LN_FLAGS += -lSDL2_image
endif

ifeq ($(BACKEND),TERM)
    CPP_FLAGS += -DUSE_TERM
endif

ifeq ($(USE_FREETYPE),false)
    CPP_FLAGS += -DNO_FREETYPE
    CPP_FLAGS += -DUSE_STBTTF
else
    CPP_FLAGS += $(shell $(PKGCONFIG) --cflags freetype2)
    LN_FLAGS += $(shell $(PKGCONFIG) --libs freetype2)
endif

CPP_FILES += $(shell find src -type f -name '*.cpp')
CPP_FILES += $(shell find dependencies -type f -name '*.cpp')

C_FLAGS := $(CPP_FLAGS)
C_FILES += $(shell find src -type f -name '*.c')
C_FILES += $(shell find dependencies -type f -name '*.c')

LN_FLAGS += $(SO_FILES)

ODIR = build

CPP_FLAGS += --std=c++17
CPP_FLAGS += -DCMS_NO_REGISTER_KEYWORD

# FLAGS += -m32 # uncomment for 32-bit build
ifeq ($(DEBUG),true)
FLAGS += -Og -g -D_DEBUG # debug build
POSTBUILD =
else
FLAGS += -O3 # release build
POSTBUILD = $(STRIP) $(PROJECT)
endif

FLAGS += -pthread
LN_FLAGS += -lm

OBJ = $(patsubst %,$(ODIR)/%.o,$(CPP_FILES))
OBJ += $(patsubst %,$(ODIR)/%.o,$(C_FILES))
DEP := $(OBJ:.o=.d)

$(ODIR)/%.cpp.o: %.cpp
	@mkdir -p "$$(dirname "$@")"
	$(CXX) -c $< -o $@ $(FLAGS) $(CPP_FLAGS)

$(ODIR)/%.c.o: %.c
	@mkdir -p "$$(dirname "$@")"
	$(CC) -c $< -o $@ $(FLAGS) $(C_FLAGS)

$(ODIR)/%.mm.o: %.mm
	@mkdir -p "$$(dirname "$@")"
	$(OBJC) -c $< -o $@ $(FLAGS) $(CPP_FLAGS)

$(PROJECT): $(OBJ)
	$(LN) $^ -o $@ $(FLAGS) $(LN_FLAGS)
	$(POSTBUILD)

.PHONY: clean

clean:
	rm -rf $(ODIR)

-include $(DEP)
