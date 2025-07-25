#
# iowolfsp Makefile
#
# GNU Make required
#
COMPILE_PLATFORM=$(shell uname | sed -e 's/_.*//' | tr '[:upper:]' '[:lower:]' | sed -e 's/\//_/g')
COMPILE_ARCH=$(shell uname -m | sed -e 's/i.86/x86/')

ifeq ($(COMPILE_PLATFORM),sunos)
  # Solaris uname and GNU uname differ
  COMPILE_ARCH=$(shell uname -p | sed -e 's/i.86/x86/')
endif

ifndef BUILD_STANDALONE
  BUILD_STANDALONE =
endif
ifndef BUILD_CLIENT
  BUILD_CLIENT     =
endif
ifndef BUILD_SERVER
  BUILD_SERVER     = 0
endif
ifndef BUILD_GAME_SO
  BUILD_GAME_SO    =
endif
ifndef BUILD_GAME_QVM
  BUILD_GAME_QVM   = 0
endif
ifndef BUILD_BASEGAME
  BUILD_BASEGAME =
endif
ifndef BUILD_ARCHIVE
  BUILD_ARCHIVE = 0
endif

#############################################################################
#
# If you require a different configuration from the defaults below, create a
# new file named "Makefile.local" in the same directory as this file and define
# your parameters there. This allows you to change configuration without
# causing problems with keeping up to date with the repository.
#
#############################################################################
-include Makefile.local

ifeq ($(COMPILE_PLATFORM),cygwin)
  PLATFORM=mingw32
  CFLAGS += -DWIN32
  CLIENT_CFLAGS += -DWIN32
endif

ifndef PLATFORM
PLATFORM=$(COMPILE_PLATFORM)
endif
export PLATFORM

ifeq ($(PLATFORM),mingw32)
  MINGW=1
endif
ifeq ($(PLATFORM),mingw64)
  MINGW=1
endif

ifeq ($(COMPILE_ARCH),i386)
  COMPILE_ARCH=x86
endif
ifeq ($(COMPILE_ARCH),i86pc)
  COMPILE_ARCH=x86
endif

ifeq ($(COMPILE_ARCH),amd64)
  COMPILE_ARCH=x86_64
endif
ifeq ($(COMPILE_ARCH),x64)
  COMPILE_ARCH=x86_64
endif

ifeq ($(COMPILE_ARCH),powerpc)
  COMPILE_ARCH=ppc
endif
ifeq ($(COMPILE_ARCH),powerpc64)
  COMPILE_ARCH=ppc64
endif

ifeq ($(COMPILE_ARCH),axp)
  COMPILE_ARCH=alpha
endif

ifndef ARCH
ARCH=$(COMPILE_ARCH)
endif
export ARCH

# For historical compatibility reasons on non-windows
# platform output files use i386 instead of x86
ifeq ($(ARCH),x86)
  ifndef MINGW
    FILE_ARCH=i386
  endif
endif

ifndef FILE_ARCH
FILE_ARCH=$(ARCH)
endif
export FILE_ARCH

ifneq ($(PLATFORM),$(COMPILE_PLATFORM))
  CROSS_COMPILING=1
else
  CROSS_COMPILING=0

  ifneq ($(ARCH),$(COMPILE_ARCH))
    CROSS_COMPILING=1
  endif
endif
export CROSS_COMPILING

ifndef VERSION
VERSION=5.2
endif

ifndef CLIENTBIN
 ifdef MINGW
    CLIENTBIN=RealRTCW
  else
    CLIENTBIN=RealRTCW
  endif
endif

ifndef SERVERBIN
  ifdef MINGW
    SERVERBIN=ioWolfSPDED
  else
    SERVERBIN=iowolfspded
  endif
endif

ifndef BASEGAME
BASEGAME=main
endif

ifndef BASEGAME_CFLAGS
BASEGAME_CFLAGS=
endif

ifndef COPYDIR
COPYDIR="/usr/local/games/wolf"
endif

ifndef COPYBINDIR
COPYBINDIR=$(COPYDIR)
endif

ifndef MOUNT_DIR
MOUNT_DIR=code
endif

ifndef BUILD_DIR
BUILD_DIR=build
endif

ifndef TEMPDIR
TEMPDIR=/tmp
endif

ifndef GENERATE_DEPENDENCIES
GENERATE_DEPENDENCIES=1
endif

ifndef USE_OPENAL
USE_OPENAL=1
endif

ifndef USE_OPENAL_DLOPEN
USE_OPENAL_DLOPEN=1
endif

ifndef USE_CURL
USE_CURL=0
endif

ifndef USE_CURL_DLOPEN
  ifdef MINGW
    USE_CURL_DLOPEN=0
  else
    USE_CURL_DLOPEN=1
  endif
endif

ifndef USE_CODEC_VORBIS
USE_CODEC_VORBIS=1
endif

ifndef USE_CODEC_OPUS
USE_CODEC_OPUS=1
endif

ifndef USE_MUMBLE
USE_MUMBLE=1
endif

ifndef USE_VOIP
USE_VOIP=1
endif

ifndef USE_FREETYPE
USE_FREETYPE=1
endif

ifndef USE_INTERNAL_LIBS
USE_INTERNAL_LIBS=1
endif

ifndef USE_INTERNAL_OGG
USE_INTERNAL_OGG=$(USE_INTERNAL_LIBS)
endif

ifndef USE_INTERNAL_VORBIS
USE_INTERNAL_VORBIS=$(USE_INTERNAL_LIBS)
endif

ifndef USE_INTERNAL_OPUS
USE_INTERNAL_OPUS=$(USE_INTERNAL_LIBS)
endif

ifndef USE_INTERNAL_ZLIB
USE_INTERNAL_ZLIB=$(USE_INTERNAL_LIBS)
endif

ifndef USE_INTERNAL_JPEG
USE_INTERNAL_JPEG=$(USE_INTERNAL_LIBS)
endif

ifndef USE_INTERNAL_FREETYPE
USE_INTERNAL_FREETYPE=$(USE_INTERNAL_LIBS)
endif

ifndef USE_LOCAL_HEADERS
USE_LOCAL_HEADERS=$(USE_INTERNAL_LIBS)
endif

ifndef USE_RENDERER_DLOPEN
USE_RENDERER_DLOPEN=1
endif

ifndef USE_XDG
USE_XDG=0
endif

ifndef USE_YACC
USE_YACC=0
endif

ifndef DEBUG_CFLAGS
DEBUG_CFLAGS=-ggdb -O0
endif

ifndef USE_BLOOM
USE_BLOOM=1
endif

ifndef USE_OPENGLES
USE_OPENGLES=0
endif

ifndef USE_AUTHORIZE_SERVER
USE_AUTHORIZE_SERVER=0
endif


#############################################################################

BD=$(BUILD_DIR)/debug-$(PLATFORM)-$(ARCH)
BR=$(BUILD_DIR)/release-$(PLATFORM)-$(ARCH)
STEAMDIR=$(MOUNT_DIR)/steam
STEAMSHIMDIR=$(MOUNT_DIR)/steamshim
CDIR=$(MOUNT_DIR)/client
SDIR=$(MOUNT_DIR)/server
RDIR=$(MOUNT_DIR)/renderer
CMDIR=$(MOUNT_DIR)/qcommon
SDLDIR=$(MOUNT_DIR)/sdl
ASMDIR=$(MOUNT_DIR)/asm
SYSDIR=$(MOUNT_DIR)/sys
GDIR=$(MOUNT_DIR)/game
CGDIR=$(MOUNT_DIR)/cgame
BLIBDIR=$(MOUNT_DIR)/botlib
NDIR=$(MOUNT_DIR)/null
UIDIR=$(MOUNT_DIR)/ui
JPDIR=$(MOUNT_DIR)/jpeg-8c
OGGDIR=$(MOUNT_DIR)/libogg-1.3.3
VORBISDIR=$(MOUNT_DIR)/libvorbis-1.3.6
OPUSDIR=$(MOUNT_DIR)/opus-1.2.1
OPUSFILEDIR=$(MOUNT_DIR)/opusfile-0.9
ZDIR=$(MOUNT_DIR)/zlib-1.2.11
FTDIR=$(MOUNT_DIR)/freetype-2.9
SPLDIR=$(MOUNT_DIR)/splines
TOOLSDIR=$(MOUNT_DIR)/tools
Q3ASMDIR=$(MOUNT_DIR)/tools/asm
LBURGDIR=$(MOUNT_DIR)/tools/lcc/lburg
Q3CPPDIR=$(MOUNT_DIR)/tools/lcc/cpp
Q3LCCETCDIR=$(MOUNT_DIR)/tools/lcc/etc
Q3LCCSRCDIR=$(MOUNT_DIR)/tools/lcc/src
SDLHDIR=$(MOUNT_DIR)/SDL2
LIBSDIR=$(MOUNT_DIR)/libs

bin_path=$(shell which $(1) 2> /dev/null)

# We won't need this if we only build the server
ifneq ($(BUILD_CLIENT),0)
  # set PKG_CONFIG_PATH or PKG_CONFIG to influence this, e.g.
  # PKG_CONFIG_PATH=/opt/cross/i386-mingw32msvc/lib/pkgconfig or
  # PKG_CONFIG=arm-linux-gnueabihf-$(PKG_CONFIG)
  ifeq ($(CROSS_COMPILING),0)
    PKG_CONFIG ?= pkg-config
  else
  ifneq ($(PKG_CONFIG_PATH),)
    PKG_CONFIG ?= pkg-config
  else
    # Don't use host pkg-config when cross-compiling.
    # (unknown-pkg-config is meant to be a non-existant command.)
    PKG_CONFIG ?= unknown-pkg-config
  endif
  endif

  ifneq ($(call bin_path, $(PKG_CONFIG)),)
    CURL_CFLAGS ?= $(shell $(PKG_CONFIG) --silence-errors --cflags libcurl)
    CURL_LIBS ?= $(shell $(PKG_CONFIG) --silence-errors --libs libcurl)
    OPENAL_CFLAGS ?= $(shell $(PKG_CONFIG) --silence-errors --cflags openal)
    OPENAL_LIBS ?= $(shell $(PKG_CONFIG) --silence-errors --libs openal)
    SDL_CFLAGS ?= $(shell $(PKG_CONFIG) --silence-errors --cflags sdl2|sed 's/-Dmain=SDL_main//')
    SDL_LIBS ?= $(shell $(PKG_CONFIG) --silence-errors --libs sdl2)
  else
    # assume they're in the system default paths (no -I or -L needed)
    CURL_LIBS ?= -lcurl
    OPENAL_LIBS ?= -lopenal
  endif
  # Use sdl2-config if all else fails
  ifeq ($(SDL_CFLAGS),)
    ifneq ($(call bin_path, sdl2-config),)
      SDL_CFLAGS = $(shell sdl2-config --cflags)
      SDL_LIBS = $(shell sdl2-config --libs)
    endif
  endif
endif

# Add git version info
USE_GIT=
ifeq ($(wildcard ../.git),../.git)
  GIT_REV=$(shell git show -s --pretty=format:%h-%ad --date=short)
  ifneq ($(GIT_REV),)
    VERSION:=$(VERSION)_GIT_$(GIT_REV)
    USE_GIT=1
  endif
endif


#############################################################################
# SETUP AND BUILD -- LINUX
#############################################################################

INSTALL=install
MKDIR=mkdir -p
EXTRA_FILES=
CLIENT_EXTRA_FILES=

ifneq (,$(findstring "$(COMPILE_PLATFORM)", "linux" "gnu_kfreebsd" "kfreebsd-gnu" "gnu"))
  TOOLS_CFLAGS += -DARCH_STRING=\"$(COMPILE_ARCH)\"
endif

ifneq (,$(findstring "$(PLATFORM)", "linux" "gnu_kfreebsd" "kfreebsd-gnu" "gnu"))
  BASE_CFLAGS = -Wall -fno-strict-aliasing \
    -pipe -DUSE_ICON -DARCH_STRING=\\\"$(FILE_ARCH)\\\"
  CLIENT_CFLAGS += $(SDL_CFLAGS)

  ifeq ($(ARCH),x86_64)
    OPTIMIZEVM = -O3
    OPTIMIZE = $(OPTIMIZEVM) -ffast-math
  else
  ifeq ($(ARCH),x86)
    OPTIMIZEVM = -O3 -march=i586
    OPTIMIZE = $(OPTIMIZEVM) -ffast-math
  else
  ifeq ($(ARCH),ppc)
    OPTIMIZEVM = -O3
    OPTIMIZE = $(OPTIMIZEVM) -ffast-math
    ALTIVEC_CFLAGS = -maltivec
  endif
  ifeq ($(ARCH),ppc64)
    OPTIMIZEVM = -O3
    OPTIMIZE = $(OPTIMIZEVM) -ffast-math
    ALTIVEC_CFLAGS = -maltivec
  endif
  ifeq ($(ARCH),sparc)
    OPTIMIZEVM += -mcpu=v9 -mtune=v9 -mv8plus
    OPTIMIZE = $(OPTIMIZEVM) -ffast-math
  endif
  ifeq ($(ARCH),sparc64)
    OPTIMIZEVM += -mcpu=v9 -mtune=v9 -mv8plus
    OPTIMIZE = $(OPTIMIZEVM) -ffast-math
  endif
  ifeq ($(ARCH),alpha)
    # According to http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=410555
    # -ffast-math will cause the client to die with SIGFPE on Alpha
    OPTIMIZE = $(OPTIMIZEVM)
  endif
  endif
  endif

  SHLIBEXT=so
  SHLIBCFLAGS=-fPIC -fvisibility=hidden
  SHLIBLDFLAGS=-shared $(LDFLAGS)

  THREAD_LIBS=-lpthread
  LIBS=-ldl -lm

  ifeq ($(USE_LOCAL_HEADERS),1)
    CLIENT_CFLAGS += -I$(SDLHDIR)/include
  endif

  ifeq ($(USE_INTERNAL_LIBS),1)
    ifeq ($(CROSS_COMPILING),1)
      ifeq ($(ARCH),x86)
      SDL_LIBS = $(LIBSDIR)/linux32/libSDL2main.a \
                 $(LIBSDIR)/linux32/libSDL2-2.0.so.0.18.0
      endif
      ifeq ($(ARCH),x86_64)
      SDL_LIBS = $(LIBSDIR)/linux64/libSDL2main.a \
                 $(LIBSDIR)/linux64/libSDL2-2.0.so.0.18.0
      endif
    endif
  endif

  ifeq ($(USE_OPENGLES),1)
    BASE_CFLAGS += -DUSE_OPENGLES
    CLIENT_LIBS = $(SDL_LIBS)
    RENDERER_LIBS = $(SDL_LIBS)
    SERVER_LIBS =
  else
    CLIENT_CFLAGS += $(SDL_CFLAGS)
    CLIENT_LIBS += $(SDL_LIBS)
    RENDERER_LIBS += $(SDL_LIBS)
    SERVER_LIBS =
  endif

  ifeq ($(USE_OPENAL),1)
    ifneq ($(USE_OPENAL_DLOPEN),1)
      CLIENT_LIBS += $(THREAD_LIBS) $(OPENAL_LIBS)
    endif
  endif

  ifeq ($(USE_CURL),1)
    CLIENT_CFLAGS += $(CURL_CFLAGS)
    ifneq ($(USE_CURL_DLOPEN),1)
      CLIENT_LIBS += $(CURL_LIBS)
    endif
  endif

  ifeq ($(USE_MUMBLE),1)
    CLIENT_LIBS += -lrt
  endif

else # ifeq Linux

#############################################################################
# SETUP AND BUILD -- MAC OS X
#############################################################################

ifeq ($(PLATFORM),darwin)
  LIBS = -framework Cocoa
  CLIENT_LIBS=
  RENDERER_LIBS=
  OPTIMIZEVM = -O3

  # Default minimum Mac OS X version
  ifeq ($(MACOSX_VERSION_MIN),)
    MACOSX_VERSION_MIN=10.5
    ifeq ($(ARCH),arm64)
      MACOSX_VERSION_MIN=11.0
    endif
  endif

  MACOSX_MAJOR=$(shell echo $(MACOSX_VERSION_MIN) | cut -d. -f1)
  MACOSX_MINOR=$(shell echo $(MACOSX_VERSION_MIN) | cut -d. -f2)
  ifeq ($(shell test $(MACOSX_MINOR) -gt 9; echo $$?),0)
    # Multiply and then remove decimal. 10.10 -> 101000.0 -> 101000
    MAC_OS_X_VERSION_MIN_REQUIRED=$(shell echo "$(MACOSX_MAJOR) * 10000 + $(MACOSX_MINOR) * 100" | bc | cut -d. -f1)
  else
    # Multiply by 100 and then remove decimal. 10.7 -> 1070.0 -> 1070
    MAC_OS_X_VERSION_MIN_REQUIRED=$(shell echo "$(MACOSX_VERSION_MIN) * 100" | bc | cut -d. -f1)
  endif

  LDFLAGS += -mmacosx-version-min=$(MACOSX_VERSION_MIN)
  BASE_CFLAGS += -mmacosx-version-min=$(MACOSX_VERSION_MIN) \
                 -DMAC_OS_X_VERSION_MIN_REQUIRED=$(MAC_OS_X_VERSION_MIN_REQUIRED)

  MACOSX_ARCH=$(ARCH)
  ifeq ($(ARCH),x86)
    MACOSX_ARCH=i386
  endif

  ifeq ($(ARCH),ppc)
    BASE_CFLAGS += -arch ppc
    ALTIVEC_CFLAGS = -faltivec
  endif
  ifeq ($(ARCH),ppc64)
    BASE_CFLAGS += -arch ppc64
    ALTIVEC_CFLAGS = -faltivec
  endif
  ifeq ($(ARCH),x86)
    OPTIMIZEVM += -march=prescott -mfpmath=sse
    # x86 vm will crash without -mstackrealign since MMX instructions will be
    # used no matter what and they corrupt the frame pointer in VM calls
    BASE_CFLAGS += -arch i386 -m32 -mstackrealign
  endif
  ifeq ($(ARCH),x86_64)
    OPTIMIZEVM += -mfpmath=sse
    BASE_CFLAGS += -arch x86_64
  endif
  ifeq ($(ARCH),arm64)
    BASE_CFLAGS += -arch arm64
  endif

  # When compiling on OSX for OSX, we're not cross compiling as far as the
  # Makefile is concerned, as target architecture is specified as a compiler
  # argument
  ifeq ($(COMPILE_PLATFORM),darwin)
    CROSS_COMPILING=0
  endif

  ifeq ($(CROSS_COMPILING),1)
    # If CC is already set to something generic, we probably want to use
    # something more specific
    ifneq ($(findstring $(strip $(CC)),cc gcc),)
      CC=
    endif

    ifndef CC
      ifndef DARWIN
        # macOS 10.5 SDK
          DARWIN=9
        ifeq ($(ARCH),arm64)
          # macOS 11.0 SDK
          DARWIN=20.1
        endif
      endif

      CC=$(MACOSX_ARCH)-apple-darwin$(DARWIN)-cc
      RANLIB=$(MACOSX_ARCH)-apple-darwin$(DARWIN)-ranlib
      LIPO=$(MACOSX_ARCH)-apple-darwin$(DARWIN)-lipo

      ifeq ($(call bin_path, $(CC)),)
        $(error Unable to find osxcross $(CC))
      endif
    endif
  endif

  ifndef LIPO
    LIPO=lipo
  endif

  BASE_CFLAGS += -fno-strict-aliasing -fno-common -pipe

  ifeq ($(USE_OPENAL),1)
    ifneq ($(USE_LOCAL_HEADERS),1)
      CLIENT_CFLAGS += -I/System/Library/Frameworks/OpenAL.framework/Headers
    endif
    ifneq ($(USE_OPENAL_DLOPEN),1)
      ifneq ($(USE_INTERNAL_LIBS),1)
        CLIENT_CFLAGS += $(OPENAL_CFLAGS)
        CLIENT_LIBS += $(THREAD_LIBS) $(OPENAL_LIBS)
        CLIENT_EXTRA_FILES += $(LIBSDIR)/macosx/libopenal.dylib
      else
        CLIENT_LIBS += -framework OpenAL
      endif
    endif
  endif

  ifeq ($(USE_CURL),1)
    CLIENT_CFLAGS += $(CURL_CFLAGS)
    ifneq ($(USE_CURL_DLOPEN),1)
      CLIENT_LIBS += $(CURL_LIBS)
    endif
  endif

  BASE_CFLAGS += -D_THREAD_SAFE=1

  CLIENT_LIBS += -framework IOKit
  RENDERER_LIBS += -framework OpenGL

  ifeq ($(USE_LOCAL_HEADERS),1)
    # libSDL2-2.0.0.dylib for PPC is SDL 2.0.1 + changes to compile
    ifneq ($(findstring $(ARCH),ppc ppc64),)
      BASE_CFLAGS += -I$(SDLHDIR)/include-2.0.1
    else
      BASE_CFLAGS += -I$(SDLHDIR)/include
    endif

    # We copy sdlmain before ranlib'ing it so that subversion doesn't think
    #  the file has been modified by each build.
    LIBSDLMAIN=$(B)/libSDL2main.a
    LIBSDLMAINSRC=$(LIBSDIR)/macosx/libSDL2main.a
    CLIENT_LIBS += $(LIBSDIR)/macosx/libSDL2-2.0.0.dylib
    RENDERER_LIBS += $(LIBSDIR)/macosx/libSDL2-2.0.0.dylib
    CLIENT_EXTRA_FILES += $(LIBSDIR)/macosx/libSDL2-2.0.0.dylib
  else
    BASE_CFLAGS += -I/Library/Frameworks/SDL2.framework/Headers
    CLIENT_LIBS += -framework SDL2
    RENDERER_LIBS += -framework SDL2
  endif

  OPTIMIZE = $(OPTIMIZEVM) -ffast-math

  SHLIBEXT=dylib
  SHLIBCFLAGS=-fPIC -fno-common
  SHLIBLDFLAGS=-dynamiclib $(LDFLAGS) -Wl,-U,_com_altivec

  NOTSHLIBCFLAGS=-mdynamic-no-pic

else # ifeq darwin


#############################################################################
# SETUP AND BUILD -- MINGW32
#############################################################################

ifdef MINGW

  ifeq ($(CROSS_COMPILING),1)
    # If CC is already set to something generic, we probably want to use
    # something more specific
    ifneq ($(findstring $(strip $(CC)),cc gcc),)
      CC=
    endif
    ifneq ($(findstring $(strip $(CXX)),g++),)
      CXX=
    endif

    # We need to figure out the correct gcc and windres
    ifeq ($(ARCH),x86_64)
      MINGW_PREFIXES=x86_64-w64-mingw32 amd64-mingw32msvc
    endif
    ifeq ($(ARCH),x86)
      MINGW_PREFIXES=i686-w64-mingw32 i586-mingw32msvc i686-pc-mingw32
    endif

    ifndef CC
      CC=$(firstword $(strip $(foreach MINGW_PREFIX, $(MINGW_PREFIXES), \
         $(call bin_path, $(MINGW_PREFIX)-gcc))))
    endif

    ifndef CXX
      CXX=$(firstword $(strip $(foreach MINGW_PREFIX, $(MINGW_PREFIXES), \
         $(call bin_path, $(MINGW_PREFIX)-g++))))
    endif

    ifndef WINDRES
      WINDRES=$(firstword $(strip $(foreach MINGW_PREFIX, $(MINGW_PREFIXES), \
         $(call bin_path, $(MINGW_PREFIX)-windres))))
    endif
  else
    # Some MinGW installations define CC to cc, but don't actually provide cc,
    # so check that CC points to a real binary and use gcc if it doesn't
    ifeq ($(call bin_path, $(CC)),)
      CC=gcc
    endif

    ifndef CXX
      CXX=g++
    endif

  endif

  # using generic windres if specific one is not present
  ifndef WINDRES
    WINDRES=windres
  endif

  ifeq ($(CC),)
    $(error Cannot find a suitable cross compiler for $(PLATFORM))
  endif

  BASE_CFLAGS = -Wall -fno-strict-aliasing \
    -DUSE_ICON



  # In the absence of wspiapi.h, require Windows XP or later
  ifeq ($(shell test -e $(CMDIR)/wspiapi.h; echo $$?),1)
    BASE_CFLAGS += -DWINVER=0x501
  endif

  ifeq ($(USE_OPENAL),1)
    CLIENT_CFLAGS += $(OPENAL_CFLAGS)
    ifneq ($(USE_OPENAL_DLOPEN),1)
      CLIENT_LDFLAGS += $(OPENAL_LDFLAGS)
    endif
  endif

  ifeq ($(ARCH),x86_64)
    OPTIMIZEVM = -O3
    OPTIMIZE = $(OPTIMIZEVM) -ffast-math -fno-omit-frame-pointer
    FILE_ARCH=x64
  endif
  ifeq ($(ARCH),x86)
    OPTIMIZEVM = -O3 -march=i586
    OPTIMIZE = $(OPTIMIZEVM) -ffast-math -fno-omit-frame-pointer
  endif

  SHLIBEXT=dll
  SHLIBCFLAGS=
  SHLIBLDFLAGS=-shared $(LDFLAGS)

  # clang 3.5 doesn't support this
  ifneq ("$(CC)", $(findstring "$(CC)", "clang" "clang++"))
    LDFLAGS += -mwindows -static -static-libgcc -static-libstdc++
  endif

  BINEXT=.exe

  ifeq ($(CROSS_COMPILING),0)
    TOOLS_BINEXT=.exe
  endif

  ifeq ($(COMPILE_PLATFORM),cygwin)
    TOOLS_BINEXT=.exe
    
    # Under cygwin the default of using gcc for TOOLS_CC won't work, so
    # we need to figure out the appropriate compiler to use, based on the
    # host architecture that we're running under (as tools run on the host)
    ifeq ($(COMPILE_ARCH),x86_64)
      TOOLS_MINGW_PREFIXES=x86_64-w64-mingw32 amd64-mingw32msvc
    endif
    ifeq ($(COMPILE_ARCH),x86)
      TOOLS_MINGW_PREFIXES=i686-w64-mingw32 i586-mingw32msvc i686-pc-mingw32
    endif

    TOOLS_CC=$(firstword $(strip $(foreach TOOLS_MINGW_PREFIX, $(TOOLS_MINGW_PREFIXES), \
      $(call bin_path, $(TOOLS_MINGW_PREFIX)-gcc))))
  endif

  LIBS= -lws2_32 -lwinmm -lpsapi
  CLIENT_LIBS = -lgdi32 -lole32
  RENDERER_LIBS = -lgdi32 -lole32

  ifeq ($(USE_FREETYPE),1)
    ifneq ($(USE_INTERNAL_FREETYPE),1)
      FREETYPE_CFLAGS = -Ifreetype2
    endif
  endif

  ifeq ($(USE_CURL),1)
    CLIENT_CFLAGS += $(CURL_CFLAGS)
    ifneq ($(USE_CURL_DLOPEN),1)
      ifeq ($(USE_LOCAL_HEADERS),1)
        CLIENT_CFLAGS += -DCURL_STATICLIB
        ifeq ($(ARCH),x86_64)
          CLIENT_LIBS += $(LIBSDIR)/win64/libcurl.a -lcrypt32
        else
          CLIENT_LIBS += $(LIBSDIR)/win32/libcurl.a -lcrypt32
        endif
      else
        CLIENT_LIBS += $(CURL_LIBS)
      endif
    endif
  endif


  # libmingw32 must be linked before libSDLmain
  CLIENT_LIBS += -lmingw32
  RENDERER_LIBS += -lmingw32

  ifeq ($(USE_LOCAL_HEADERS),1)
    CLIENT_CFLAGS += -I$(SDLHDIR)/include
    ifeq ($(ARCH),x86)
    CLIENT_LIBS += $(LIBSDIR)/win32/libSDL2main.a \
                      $(LIBSDIR)/win32/libSDL2.dll.a
    RENDERER_LIBS += $(LIBSDIR)/win32/libSDL2main.a \
                      $(LIBSDIR)/win32/libSDL2.dll.a
    SDLDLL=SDL2.dll
    CLIENT_EXTRA_FILES += $(LIBSDIR)/win32/SDL2.dll $(LIBSDIR)/win32/OpenAL32.dll
    else
    CLIENT_LIBS += $(LIBSDIR)/win64/libSDL264main.a \
                      $(LIBSDIR)/win64/libSDL264.dll.a
    RENDERER_LIBS += $(LIBSDIR)/win64/libSDL264main.a \
                      $(LIBSDIR)/win64/libSDL264.dll.a
    SDLDLL=SDL264.dll
    CLIENT_EXTRA_FILES += $(LIBSDIR)/win64/SDL264.dll $(LIBSDIR)/win64/OpenAL64.dll
    endif
  else
    CLIENT_CFLAGS += $(SDL_CFLAGS)
    CLIENT_LIBS += $(SDL_LIBS)
    RENDERER_LIBS += $(SDL_LIBS)
    SDLDLL=SDL2.dll
  endif

else # ifdef MINGW

#############################################################################
# SETUP AND BUILD -- FREEBSD
#############################################################################

ifeq ($(PLATFORM),freebsd)

  # flags
  BASE_CFLAGS = \
    -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes \
    -DUSE_ICON -DMAP_ANONYMOUS=MAP_ANON
  CLIENT_CFLAGS += $(SDL_CFLAGS)

  OPTIMIZEVM =
  OPTIMIZE = $(OPTIMIZEVM) -ffast-math

  SHLIBEXT=so
  SHLIBCFLAGS=-fPIC
  SHLIBLDFLAGS=-shared $(LDFLAGS)

  THREAD_LIBS=-lpthread
  # don't need -ldl (FreeBSD)
  LIBS=-lm

  CLIENT_LIBS =

  CLIENT_LIBS += $(SDL_LIBS)
  RENDERER_LIBS = $(SDL_LIBS)

  # optional features/libraries
  ifeq ($(USE_OPENAL),1)
    ifeq ($(USE_OPENAL_DLOPEN),1)
      CLIENT_LIBS += $(THREAD_LIBS) $(OPENAL_LIBS)
    endif
  endif

  ifeq ($(USE_CURL),1)
    CLIENT_CFLAGS += $(CURL_CFLAGS)
    ifeq ($(USE_CURL_DLOPEN),1)
      CLIENT_LIBS += $(CURL_LIBS)
    endif
  endif

  # cross-compiling tweaks
  ifeq ($(ARCH),x86)
    ifeq ($(CROSS_COMPILING),1)
      BASE_CFLAGS += -m32
    endif
  endif
  ifeq ($(ARCH),x86_64)
    ifeq ($(CROSS_COMPILING),1)
      BASE_CFLAGS += -m64
    endif
  endif
else # ifeq freebsd

#############################################################################
# SETUP AND BUILD -- OPENBSD
#############################################################################

ifeq ($(PLATFORM),openbsd)

  BASE_CFLAGS = -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes \
    -pipe -DUSE_ICON -DMAP_ANONYMOUS=MAP_ANON
  CLIENT_CFLAGS += $(SDL_CFLAGS)

  ifeq ($(ARCH),x86_64)
    OPTIMIZEVM = -O3
    OPTIMIZE = $(OPTIMIZEVM) -ffast-math
  else
  ifeq ($(ARCH),x86)
    OPTIMIZEVM = -O3 -march=i586
    OPTIMIZE = $(OPTIMIZEVM) -ffast-math
  else
  ifeq ($(ARCH),ppc)
    OPTIMIZEVM = -O3
    OPTIMIZE = $(OPTIMIZEVM) -ffast-math
    ALTIVEC_CFLAGS = -maltivec
  endif
  ifeq ($(ARCH),ppc64)
    OPTIMIZEVM = -O3
    OPTIMIZE = $(OPTIMIZEVM) -ffast-math
    ALTIVEC_CFLAGS = -maltivec
  endif
  ifeq ($(ARCH),sparc)
    OPTIMIZEVM += -mcpu=v9 -mtune=v9 -mv8plus
    OPTIMIZE = $(OPTIMIZEVM) -ffast-math
  endif
  ifeq ($(ARCH),sparc64)
    OPTIMIZEVM += -mcpu=v9 -mtune=v9 -mv8plus
    OPTIMIZE = $(OPTIMIZEVM) -ffast-math
  endif
  ifeq ($(ARCH),alpha)
    # According to http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=410555
    # -ffast-math will cause the client to die with SIGFPE on Alpha
    OPTIMIZE = $(OPTIMIZEVM)
  endif
  endif
  endif

  ifeq ($(USE_CURL),1)
    CLIENT_CFLAGS += $(CURL_CFLAGS)
    USE_CURL_DLOPEN=0
  endif

  # no shm_open on OpenBSD
  USE_MUMBLE=0

  SHLIBEXT=so
  SHLIBCFLAGS=-fPIC
  SHLIBLDFLAGS=-shared $(LDFLAGS)

  THREAD_LIBS=-lpthread
  LIBS=-lm

  CLIENT_LIBS =

  CLIENT_LIBS += $(SDL_LIBS)
  RENDERER_LIBS = $(SDL_LIBS)

  ifeq ($(USE_OPENAL),1)
    ifneq ($(USE_OPENAL_DLOPEN),1)
      CLIENT_LIBS += $(THREAD_LIBS) $(OPENAL_LIBS)
    endif
  endif

  ifeq ($(USE_CURL),1)
    ifneq ($(USE_CURL_DLOPEN),1)
      CLIENT_LIBS += $(CURL_LIBS)
    endif
  endif
else # ifeq openbsd

#############################################################################
# SETUP AND BUILD -- NETBSD
#############################################################################

ifeq ($(PLATFORM),netbsd)

  LIBS=-lm
  SHLIBEXT=so
  SHLIBCFLAGS=-fPIC
  SHLIBLDFLAGS=-shared $(LDFLAGS)
  THREAD_LIBS=-lpthread

  BASE_CFLAGS = -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes

  ifeq ($(ARCH),x86)
    HAVE_VM_COMPILED=true
  endif

  BUILD_CLIENT = 0
else # ifeq netbsd

#############################################################################
# SETUP AND BUILD -- IRIX
#############################################################################

ifeq ($(PLATFORM),irix64)
  LIB=lib

  ARCH=mips

  CC = c99

  BASE_CFLAGS=-Dstricmp=strcasecmp -Xcpluscomm -woff 1185 \
    -I. -I$(ROOT)/usr/include
  CLIENT_CFLAGS += $(SDL_CFLAGS)
  OPTIMIZE = -O3

  SHLIBEXT=so
  SHLIBCFLAGS=
  SHLIBLDFLAGS=-shared

  LIBS=-ldl -lm -lgen
  # FIXME: The X libraries probably aren't necessary?
  CLIENT_LIBS=-L/usr/X11/$(LIB) $(SDL_LIBS) \
    -lX11 -lXext -lm
  RENDERER_LIBS = $(SDL_LIBS)

else # ifeq IRIX

#############################################################################
# SETUP AND BUILD -- SunOS
#############################################################################

ifeq ($(PLATFORM),sunos)

  CC=gcc
  INSTALL=ginstall
  MKDIR=gmkdir -p
  COPYDIR="/usr/local/share/games/wolf"

  ifneq ($(ARCH),x86)
    ifneq ($(ARCH),sparc)
      ifneq ($(ARCH),sparc64)
        $(error arch $(ARCH) is currently not supported)
      endif
    endif
  endif

  BASE_CFLAGS = -Wall -fno-strict-aliasing \
    -pipe -DUSE_ICON
  CLIENT_CFLAGS += $(SDL_CFLAGS)

  ifeq ($(ARCH),sparc)
    OPTIMIZEVM += -mcpu=v9 -mtune=v9 -mv8plus
    OPTIMIZE = $(OPTIMIZEVM) -ffast-math
  endif
  ifeq ($(ARCH),sparc64)
    OPTIMIZEVM += -mcpu=v9 -mtune=v9 -mv8plus
    OPTIMIZE = $(OPTIMIZEVM) -ffast-math
  endif
  ifeq ($(ARCH),x86)
    OPTIMIZEVM += -march=i586 -fomit-frame-pointer \
      -falign-functions=2 -fstrength-reduce
    OPTIMIZE = $(OPTIMIZEVM) -ffast-math
    BASE_CFLAGS += -m32
    CLIENT_CFLAGS += -I/usr/X11/include/NVIDIA
    CLIENT_LDFLAGS += -L/usr/X11/lib/NVIDIA -R/usr/X11/lib/NVIDIA
  endif

  SHLIBEXT=so
  SHLIBCFLAGS=-fPIC
  SHLIBLDFLAGS=-shared $(LDFLAGS)

  THREAD_LIBS=-lpthread
  LIBS=-lsocket -lnsl -ldl -lm

  BOTCFLAGS=-O0

  CLIENT_LIBS +=$(SDL_LIBS) -lX11 -lXext -liconv -lm
  RENDERER_LIBS = $(SDL_LIBS)

else # ifeq sunos

#############################################################################
# SETUP AND BUILD -- GENERIC
#############################################################################
  BASE_CFLAGS=
  OPTIMIZE = -O3

  SHLIBEXT=so
  SHLIBCFLAGS=-fPIC
  SHLIBLDFLAGS=-shared

endif #Linux
endif #darwin
endif #MINGW
endif #FreeBSD
endif #OpenBSD
endif #NetBSD
endif #IRIX
endif #SunOS

ifndef CC
  CC=gcc
endif

ifndef CXX
  CXX=g++
endif

ifndef RANLIB
  RANLIB=ranlib
endif

ifeq ($(ARCH),x86)
  BASE_CFLAGS += -m32
endif
ifeq ($(ARCH),x86_64)
  BASE_CFLAGS += -m64
endif
ifeq ($(ARCH),ppc)
  BASE_CFLAGS += -m32
endif
ifeq ($(ARCH),ppc64)
  BASE_CFLAGS += -m64
endif
ifeq ($(ARCH),sparc)
  BASE_CFLAGS += -m32
endif
ifeq ($(ARCH),sparc64)
  BASE_CFLAGS += -m64
endif

ifndef HAVE_VM_COMPILED
  HAVE_VM_COMPILED=false
endif

ifneq ($(filter $(ARCH),armv7l x86 x86_64 ppc sparc),)
  HAVE_VM_COMPILED=true
endif

ifneq ($(HAVE_VM_COMPILED),true)
  BASE_CFLAGS += -DNO_VM_COMPILED
  BUILD_GAME_QVM=0
endif

TARGETS =

ifndef FULLBINEXT
  ifndef MINGW
    FULLBINEXT=.$(ARCH)$(BINEXT)
  else
    FULLBINEXT=.$(FILE_ARCH)$(BINEXT)
  endif
endif

ifndef SHLIBNAME
  SHLIBNAME=$(FILE_ARCH).$(SHLIBEXT)
endif

ifneq ($(BUILD_SERVER),0)
  TARGETS += $(B)/$(SERVERBIN)$(FULLBINEXT)
endif

ifneq ($(BUILD_CLIENT),0)
  ifneq ($(USE_RENDERER_DLOPEN),0)
    TARGETS += $(B)/$(CLIENTBIN)$(FULLBINEXT) $(B)/renderer_sp_opengl1_$(SHLIBNAME)
  else
    TARGETS += $(B)/$(CLIENTBIN)$(FULLBINEXT)
  endif
endif

ifneq ($(BUILD_GAME_SO),0)
  ifneq ($(BUILD_BASEGAME),0)
   ifdef MINGW
    TARGETS += \
	$(B)/$(BASEGAME)/cgame_sp_$(SHLIBNAME) \
	$(B)/$(BASEGAME)/qagame_sp_$(SHLIBNAME) \
	$(B)/$(BASEGAME)/ui_sp_$(SHLIBNAME)
   else
   TARGETS += \
        $(B)/$(BASEGAME)/cgame.sp.$(SHLIBNAME) \
        $(B)/$(BASEGAME)/qagame.sp.$(SHLIBNAME) \
        $(B)/$(BASEGAME)/ui.sp.$(SHLIBNAME)
   endif
  endif
endif

ifneq ($(BUILD_GAME_QVM),0)
  ifneq ($(BUILD_BASEGAME),0)
    TARGETS += \
    $(B)/$(BASEGAME)/vm/cgame.sp.qvm \
    $(B)/$(BASEGAME)/vm/qagame.sp.qvm \
    $(B)/$(BASEGAME)/vm/ui.sp.qvm
  endif
endif

ifeq ($(USE_OPENAL),1)
  CLIENT_CFLAGS += -DUSE_OPENAL
  ifeq ($(USE_OPENAL_DLOPEN),1)
    CLIENT_CFLAGS += -DUSE_OPENAL_DLOPEN
  endif
endif

ifeq ($(USE_CURL),1)
  CLIENT_CFLAGS += -DUSE_CURL
  ifeq ($(USE_CURL_DLOPEN),1)
    CLIENT_CFLAGS += -DUSE_CURL_DLOPEN
  endif
endif

ifeq ($(USE_VOIP),1)
  CLIENT_CFLAGS += -DUSE_VOIP
  SERVER_CFLAGS += -DUSE_VOIP
  NEED_OPUS=1
endif

ifeq ($(USE_CODEC_OPUS),1)
  CLIENT_CFLAGS += -DUSE_CODEC_OPUS
  NEED_OPUS=1
endif

ifeq ($(NEED_OPUS),1)
  ifeq ($(USE_INTERNAL_OPUS),1)
    OPUS_CFLAGS = -DOPUS_BUILD -DHAVE_LRINTF -DFLOATING_POINT -DFLOAT_APPROX -DUSE_ALLOCA \
      -I$(OPUSDIR)/include -I$(OPUSDIR)/celt -I$(OPUSDIR)/silk \
      -I$(OPUSDIR)/silk/float -I$(OPUSFILEDIR)/include
  else
    OPUS_CFLAGS ?= $(shell $(PKG_CONFIG) --silence-errors --cflags opusfile opus || true)
    OPUS_LIBS ?= $(shell $(PKG_CONFIG) --silence-errors --libs opusfile opus || echo -lopusfile -lopus)
  endif
  CLIENT_CFLAGS += $(OPUS_CFLAGS)
  CLIENT_LIBS += $(OPUS_LIBS)
  NEED_OGG=1
endif

ifeq ($(USE_CODEC_VORBIS),1)
  CLIENT_CFLAGS += -DUSE_CODEC_VORBIS
  ifeq ($(USE_INTERNAL_VORBIS),1)
    CLIENT_CFLAGS += -I$(VORBISDIR)/include -I$(VORBISDIR)/lib
  else
    VORBIS_CFLAGS ?= $(shell $(PKG_CONFIG) --silence-errors --cflags vorbisfile vorbis || true)
    VORBIS_LIBS ?= $(shell $(PKG_CONFIG) --silence-errors --libs vorbisfile vorbis || echo -lvorbisfile -lvorbis)
  endif
  CLIENT_CFLAGS += $(VORBIS_CFLAGS)
  CLIENT_LIBS += $(VORBIS_LIBS)
  NEED_OGG=1
endif

ifeq ($(NEED_OGG),1)
  ifeq ($(USE_INTERNAL_OGG),1)
    OGG_CFLAGS = -I$(OGGDIR)/include
  else
    OGG_CFLAGS ?= $(shell $(PKG_CONFIG) --silence-errors --cflags ogg || true)
    OGG_LIBS ?= $(shell $(PKG_CONFIG) --silence-errors --libs ogg || echo -logg)
  endif
  CLIENT_CFLAGS += $(OGG_CFLAGS)
  CLIENT_LIBS += $(OGG_LIBS)
endif

ifeq ($(USE_RENDERER_DLOPEN),1)
  CLIENT_CFLAGS += -DUSE_RENDERER_DLOPEN
endif

ifeq ($(USE_XDG),1)
  CLIENT_CFLAGS += -DUSE_XDG
  SERVER_CFLAGS += -DUSE_XDG
endif

ifeq ($(USE_MUMBLE),1)
  CLIENT_CFLAGS += -DUSE_MUMBLE
endif

ifdef STEAM
  CFLAGS += -DSTEAM
  CLIENT_CFLAGS += -DSTEAM
endif

ifeq ($(USE_INTERNAL_ZLIB),1)
  ZLIB_CFLAGS = -DNO_GZIP -I$(ZDIR)
else
  ZLIB_CFLAGS ?= $(shell $(PKG_CONFIG) --silence-errors --cflags zlib || true)
  ZLIB_LIBS ?= $(shell $(PKG_CONFIG) --silence-errors --libs zlib || echo -lz)
endif
BASE_CFLAGS += $(ZLIB_CFLAGS)
LIBS += $(ZLIB_LIBS)

ifeq ($(USE_INTERNAL_JPEG),1)
  BASE_CFLAGS += -DUSE_INTERNAL_JPEG
  BASE_CFLAGS += -I$(JPDIR)
else
  # IJG libjpeg doesn't have pkg-config, but libjpeg-turbo uses libjpeg.pc;
  # we fall back to hard-coded answers if libjpeg.pc is unavailable
  JPEG_CFLAGS ?= $(shell $(PKG_CONFIG) --silence-errors --cflags libjpeg || true)
  JPEG_LIBS ?= $(shell $(PKG_CONFIG) --silence-errors --libs libjpeg || echo -ljpeg)
  BASE_CFLAGS += $(JPEG_CFLAGS)
  RENDERER_LIBS += $(JPEG_LIBS)
endif

ifeq ($(USE_FREETYPE),1)
  ifeq ($(USE_INTERNAL_FREETYPE),1)
    FREETYPE_CFLAGS += -I$(FTDIR)/include -DFT2_BUILD_LIBRARY
  else
    FREETYPE_CFLAGS ?= $(shell $(PKG_CONFIG) --silence-errors --cflags freetype2 || true)
    FREETYPE_LIBS ?= $(shell $(PKG_CONFIG) --silence-errors --libs freetype2 || echo -lfreetype)
  endif
  BASE_CFLAGS += -DBUILD_FREETYPE $(FREETYPE_CFLAGS)
  RENDERER_LIBS += $(FREETYPE_LIBS)
endif

ifeq ("$(CC)", $(findstring "$(CC)", "clang" "clang++"))
  BASE_CFLAGS += -Qunused-arguments
endif

ifeq ("$(CXX)", $(findstring "$(CXX)", "clang" "clang++"))
  BASE_CFLAGS += -Qunused-arguments
endif

ifdef DEFAULT_BASEDIR
  BASE_CFLAGS += -DDEFAULT_BASEDIR=\\\"$(DEFAULT_BASEDIR)\\\"
endif

ifeq ($(USE_LOCAL_HEADERS),1)
  BASE_CFLAGS += -DUSE_LOCAL_HEADERS
endif

ifeq ($(BUILD_STANDALONE),1)
  BASE_CFLAGS += -DSTANDALONE
endif

ifeq ($(USE_AUTHORIZE_SERVER),1)
  BASE_CFLAGS += -DUSE_AUTHORIZE_SERVER
endif

ifeq ($(GENERATE_DEPENDENCIES),1)
  DEPEND_CFLAGS = -MMD
else
  DEPEND_CFLAGS =
endif

ifeq ($(NO_STRIP),1)
  STRIP_FLAG =
else
  STRIP_FLAG = -s
endif

ifeq ($(USE_BLOOM),1)
  CLIENT_CFLAGS += -DUSE_BLOOM
endif

# https://reproducible-builds.org/specs/source-date-epoch/
ifdef SOURCE_DATE_EPOCH
  BASE_CFLAGS += -DPRODUCT_DATE=\\\"$(shell date --date="@$$SOURCE_DATE_EPOCH" "+%b %_d %Y" | sed -e 's/ /\\\ /'g)\\\"
endif

BASE_CFLAGS += -DPRODUCT_VERSION=\\\"$(VERSION)\\\"
BASE_CFLAGS += -Wformat=2 -Wformat-security -Wno-format-nonliteral
BASE_CFLAGS += -Wstrict-aliasing=2 -Wmissing-format-attribute
BASE_CFLAGS += -Wdisabled-optimization

ifeq ($(V),1)
echo_cmd=@:
Q=
else
echo_cmd=@echo
Q=@
endif

define DO_CC
$(echo_cmd) "CC $<"
$(Q)$(CC) $(NOTSHLIBCFLAGS) $(CFLAGS) $(CLIENT_CFLAGS) $(OPTIMIZE) -o $@ -c $<
endef

define DO_CC_ALTIVEC
$(echo_cmd) "CC $<"
$(Q)$(CC) $(NOTSHLIBCFLAGS) $(CFLAGS) $(CLIENT_CFLAGS) $(OPTIMIZE) $(ALTIVEC_CFLAGS) -o $@ -c $<
endef

define DO_REF_CC
$(echo_cmd) "REF_CC $<"
$(Q)$(CC) $(SHLIBCFLAGS) $(CFLAGS) $(CLIENT_CFLAGS) $(OPTIMIZE) -o $@ -c $<
endef

define DO_REF_CC_ALTIVEC
$(echo_cmd) "REF_CC $<"
$(Q)$(CC) $(SHLIBCFLAGS) $(CFLAGS) $(CLIENT_CFLAGS) $(OPTIMIZE) $(ALTIVEC_CFLAGS) -o $@ -c $<
endef

define DO_REF_STR
$(echo_cmd) "REF_STR $<"
$(Q)rm -f $@
$(Q)$(STRINGIFY) $< $@
endef

define DO_BOT_CC
$(echo_cmd) "BOT_CC $<"
$(Q)$(CC) $(NOTSHLIBCFLAGS) $(CFLAGS) $(BOTCFLAGS) $(OPTIMIZE) -DBOTLIB -o $@ -c $<
endef

ifeq ($(GENERATE_DEPENDENCIES),1)
  DO_QVM_DEP=cat $(@:%.o=%.d) | sed -e 's/\.o/\.asm/g' >> $(@:%.o=%.d)
endif

define DO_SHLIB_CC
$(echo_cmd) "SHLIB_CC $<"
$(Q)$(CC) $(BASEGAME_CFLAGS) $(SHLIBCFLAGS) $(CFLAGS) $(OPTIMIZEVM) -o $@ -c $<
$(Q)$(DO_QVM_DEP)
endef

define DO_GAME_CC
$(echo_cmd) "GAME_CC $<"
$(Q)$(CC) $(BASEGAME_CFLAGS) -DGAMEDLL -DQAGAME $(SHLIBCFLAGS) $(CFLAGS) $(OPTIMIZEVM) -o $@ -c $<
$(Q)$(DO_QVM_DEP)
endef

define DO_CGAME_CC
$(echo_cmd) "CGAME_CC $<"
$(Q)$(CC) $(BASEGAME_CFLAGS) -DCGAMEDLL -DCGAME $(SHLIBCFLAGS) $(CFLAGS) $(OPTIMIZEVM) -o $@ -c $<
$(Q)$(DO_QVM_DEP)
endef

define DO_UI_CC
$(echo_cmd) "UI_CC $<"
$(Q)$(CC) $(BASEGAME_CFLAGS) -DUI $(SHLIBCFLAGS) $(CFLAGS) $(OPTIMIZEVM) -o $@ -c $<
$(Q)$(DO_QVM_DEP)
endef

define DO_AS
$(echo_cmd) "AS $<"
$(Q)$(CC) $(CFLAGS) $(OPTIMIZE) -x assembler-with-cpp -o $@ -c $<
endef

define DO_DED_CC
$(echo_cmd) "DED_CC $<"
$(Q)$(CC) $(NOTSHLIBCFLAGS) -DDEDICATED $(CFLAGS) $(SERVER_CFLAGS) $(OPTIMIZE) -o $@ -c $<
endef

define DO_WINDRES
$(echo_cmd) "WINDRES $<"
$(Q)$(WINDRES) -i $< -o $@
endef

define DO_SPLINE_CXX
$(echo_cmd) "SPLINE_CXX $<"
$(Q)$(CXX) $(NOTSHLIBCFLAGS) $(CFLAGS) $(CLIENT_CFLAGS) $(OPTIMIZE) -o $@ -c $<
endef









#############################################################################
# STEAMWORKS INTEGRATION
#############################################################################

ifeq ($(ARCH),x86)
  CFLAGS += -DARCH_32
endif
ifeq ($(ARCH),x86_64)
  CFLAGS += -DARCH_64
endif
  







#############################################################################
# MAIN TARGETS
#############################################################################

default: release
all: debug release

debug:
	@$(MAKE) targets B=$(BD) CFLAGS="$(CFLAGS) $(BASE_CFLAGS) $(DEPEND_CFLAGS)" \
	  OPTIMIZE="$(DEBUG_CFLAGS)" OPTIMIZEVM="$(DEBUG_CFLAGS)" \
	  CLIENT_CFLAGS="$(CLIENT_CFLAGS)" SERVER_CFLAGS="$(SERVER_CFLAGS)" V=$(V)

release:
	@$(MAKE) targets B=$(BR) CFLAGS="$(CFLAGS) $(BASE_CFLAGS) $(DEPEND_CFLAGS)" \
	  OPTIMIZE="-DNDEBUG $(OPTIMIZE)" OPTIMIZEVM="-DNDEBUG $(OPTIMIZEVM)" \
	  CLIENT_CFLAGS="$(CLIENT_CFLAGS)" SERVER_CFLAGS="$(SERVER_CFLAGS)" V=$(V)

ifneq ($(call bin_path, tput),)
  TERM_COLUMNS=$(shell if c=`tput cols`; then echo $$(($$c-4)); else echo 76; fi)
else
  TERM_COLUMNS=76
endif

define ADD_COPY_TARGET
TARGETS += $2
$2: $1
	$(echo_cmd) "CP $$<"
	@cp $1 $2
endef

# These functions allow us to generate rules for copying a list of files
# into the base directory of the build; this is useful for bundling libs,
# README files or whatever else
define GENERATE_COPY_TARGETS
$(foreach FILE,$1, \
  $(eval $(call ADD_COPY_TARGET, \
    $(FILE), \
    $(addprefix $(B)/,$(notdir $(FILE))))))
endef

$(call GENERATE_COPY_TARGETS,$(EXTRA_FILES))

ifneq ($(BUILD_CLIENT),0)
  $(call GENERATE_COPY_TARGETS,$(CLIENT_EXTRA_FILES))
endif

NAKED_TARGETS=$(shell echo $(TARGETS) | sed -e "s!$(B)/!!g")

print_list=-@for i in $(1); \
     do \
             echo "    $$i"; \
     done

ifneq ($(call bin_path, fmt),)
  print_wrapped=@echo $(1) | fmt -w $(TERM_COLUMNS) | sed -e "s/^\(.*\)$$/    \1/"
else
  print_wrapped=$(print_list)
endif

# Create the build directories, check libraries and print out
# an informational message, then start building
targets: makedirs
	@echo ""
	@echo "Building in $(B):"
	@echo "  PLATFORM: $(PLATFORM)"
	@echo "  ARCH: $(ARCH)"
	@echo "  FILE_ARCH: $(FILE_ARCH)"
	@echo "  VERSION: $(VERSION)"
	@echo "  COMPILE_PLATFORM: $(COMPILE_PLATFORM)"
	@echo "  COMPILE_ARCH: $(COMPILE_ARCH)"
	@echo "  HAVE_VM_COMPILED: $(HAVE_VM_COMPILED)"
	@echo "  PKG_CONFIG: $(PKG_CONFIG)"
	@echo "  CC: $(CC)"
	@echo "  CXX: $(CXX)"
ifeq ($(PLATFORM),mingw32)
	@echo "  WINDRES: $(WINDRES)"
endif
	@echo ""
	@echo "  CFLAGS:"
	$(call print_wrapped, $(CFLAGS) $(OPTIMIZE))
	@echo ""
	@echo "  CLIENT_CFLAGS:"
	$(call print_wrapped, $(CLIENT_CFLAGS))
	@echo ""
	@echo "  SERVER_CFLAGS:"
	$(call print_wrapped, $(SERVER_CFLAGS))
	@echo ""
	@echo "  TOOLS_CFLAGS:"
	$(call print_wrapped, $(TOOLS_CFLAGS))
	@echo ""
	@echo "  LDFLAGS:"
	$(call print_wrapped, $(LDFLAGS))
	@echo ""
	@echo "  CLIENT_LDFLAGS:"
	$(call print_wrapped, $(CLIENT_LDFLAGS))
	@echo ""
	@echo "  SERVER_LDFLAGS:"
	$(call print_wrapped, $(SERVER_LDFLAGS))
	@echo ""
	@echo "  TOOLS_LDFLAGS:"
	$(call print_wrapped, $(TOOLS_LDFLAGS))
	@echo ""
	@echo "  LIBS:"
	$(call print_wrapped, $(LIBS))
	@echo ""
	@echo "  CLIENT_LIBS:"
	$(call print_wrapped, $(CLIENT_LIBS))
	@echo ""
	@echo "  RENDERER_LIBS:"
	$(call print_wrapped, $(RENDERER_LIBS))
	@echo ""
	@echo "  TOOLS_LIBS:"
	$(call print_wrapped, $(TOOLS_LIBS))
	@echo ""
	@echo "  SERVER_LIBS:"
	$(call print_wrapped, $(SERVER_LIBS))
	@echo ""
	@echo "  Output:"
	$(call print_list, $(NAKED_TARGETS))
	@echo ""
ifneq ($(BUILD_ARCHIVE),0)
  ifneq ($(TARGETS),)
    ifndef DEBUG_MAKEFILE
	@$(MAKE) $(TARGETS) $(B).zip V=$(V)
    endif
  endif
else
  ifneq ($(TARGETS),)
    ifndef DEBUG_MAKEFILE
	@$(MAKE) $(TARGETS) V=$(V)
    endif
  endif
endif

$(B).zip: $(TARGETS)
ifeq ($(PLATFORM),darwin)
  ifdef ARCHIVE
	@("./make-macosx-app.sh" release $(ARCH); if [ "$$?" -eq 0 ] && [ -d "$(B)/iowolfsp.app" ]; then rm -f $@; cd $(B) && zip --symlinks -r9 ../../$@ `find "iowolfsp.app" -print | sed -e "s!$(B)/!!g"`; else rm -f $@; cd $(B) && zip -r9 ../../$@ $(NAKED_TARGETS); fi)
  endif
endif
ifneq ($(PLATFORM),darwin)
  ifdef ARCHIVE
	@rm -f $@
	@(cd $(B) && zip -r9 ../../$@ $(NAKED_TARGETS))
  endif
endif

makedirs:
	@$(MKDIR) $(B)/splines
	@$(MKDIR) $(B)/client/opus
	@$(MKDIR) $(B)/client/vorbis
	@$(MKDIR) $(B)/renderer
	@$(MKDIR) $(B)/ded
	@$(MKDIR) $(B)/$(BASEGAME)/cgame
	@$(MKDIR) $(B)/$(BASEGAME)/game
	@$(MKDIR) $(B)/$(BASEGAME)/ui
	@$(MKDIR) $(B)/$(BASEGAME)/qcommon
	@$(MKDIR) $(B)/$(BASEGAME)/vm
	@$(MKDIR) $(B)/tools/asm
	@$(MKDIR) $(B)/tools/etc
	@$(MKDIR) $(B)/tools/rcc
	@$(MKDIR) $(B)/tools/cpp
	@$(MKDIR) $(B)/tools/lburg

#############################################################################
# QVM BUILD TOOLS
#############################################################################

ifndef TOOLS_CC
  # A compiler which probably produces native binaries
  TOOLS_CC = gcc
endif

ifndef YACC
  YACC = yacc
endif

TOOLS_OPTIMIZE = -g -Wall -fno-strict-aliasing
TOOLS_CFLAGS += $(TOOLS_OPTIMIZE) \
                -DTEMPDIR=\"$(TEMPDIR)\" -DSYSTEM=\"\" \
                -I$(Q3LCCSRCDIR) \
                -I$(LBURGDIR)
TOOLS_LIBS =
TOOLS_LDFLAGS =

ifeq ($(GENERATE_DEPENDENCIES),1)
  TOOLS_CFLAGS += -MMD
endif

define DO_YACC
$(echo_cmd) "YACC $<"
$(Q)$(YACC) $<
$(Q)mv -f y.tab.c $@
endef

define DO_TOOLS_CC
$(echo_cmd) "TOOLS_CC $<"
$(Q)$(TOOLS_CC) $(TOOLS_CFLAGS) -o $@ -c $<
endef

define DO_TOOLS_CC_DAGCHECK
$(echo_cmd) "TOOLS_CC_DAGCHECK $<"
$(Q)$(TOOLS_CC) $(TOOLS_CFLAGS) -Wno-unused -o $@ -c $<
endef

LBURG       = $(B)/tools/lburg/lburg$(TOOLS_BINEXT)
DAGCHECK_C  = $(B)/tools/rcc/dagcheck.c
Q3RCC       = $(B)/tools/q3rcc$(TOOLS_BINEXT)
Q3CPP       = $(B)/tools/q3cpp$(TOOLS_BINEXT)
Q3LCC       = $(B)/tools/q3lcc$(TOOLS_BINEXT)
Q3ASM       = $(B)/tools/q3asm$(TOOLS_BINEXT)
STRINGIFY   = $(B)/tools/stringify$(TOOLS_BINEXT)

LBURGOBJ= \
  $(B)/tools/lburg/lburg.o \
  $(B)/tools/lburg/gram.o

# override GNU Make built-in rule for converting gram.y to gram.c
%.c: %.y
ifeq ($(USE_YACC),1)
	$(DO_YACC)
endif

$(B)/tools/lburg/%.o: $(LBURGDIR)/%.c
	$(DO_TOOLS_CC)

$(LBURG): $(LBURGOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(TOOLS_CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

Q3RCCOBJ = \
  $(B)/tools/rcc/alloc.o \
  $(B)/tools/rcc/bind.o \
  $(B)/tools/rcc/bytecode.o \
  $(B)/tools/rcc/dag.o \
  $(B)/tools/rcc/dagcheck.o \
  $(B)/tools/rcc/decl.o \
  $(B)/tools/rcc/enode.o \
  $(B)/tools/rcc/error.o \
  $(B)/tools/rcc/event.o \
  $(B)/tools/rcc/expr.o \
  $(B)/tools/rcc/gen.o \
  $(B)/tools/rcc/init.o \
  $(B)/tools/rcc/inits.o \
  $(B)/tools/rcc/input.o \
  $(B)/tools/rcc/lex.o \
  $(B)/tools/rcc/list.o \
  $(B)/tools/rcc/main.o \
  $(B)/tools/rcc/null.o \
  $(B)/tools/rcc/output.o \
  $(B)/tools/rcc/prof.o \
  $(B)/tools/rcc/profio.o \
  $(B)/tools/rcc/simp.o \
  $(B)/tools/rcc/stmt.o \
  $(B)/tools/rcc/string.o \
  $(B)/tools/rcc/sym.o \
  $(B)/tools/rcc/symbolic.o \
  $(B)/tools/rcc/trace.o \
  $(B)/tools/rcc/tree.o \
  $(B)/tools/rcc/types.o

$(DAGCHECK_C): $(LBURG) $(Q3LCCSRCDIR)/dagcheck.md
	$(echo_cmd) "LBURG $(Q3LCCSRCDIR)/dagcheck.md"
	$(Q)$(LBURG) $(Q3LCCSRCDIR)/dagcheck.md $@

$(B)/tools/rcc/dagcheck.o: $(DAGCHECK_C)
	$(DO_TOOLS_CC_DAGCHECK)

$(B)/tools/rcc/%.o: $(Q3LCCSRCDIR)/%.c
	$(DO_TOOLS_CC)

$(Q3RCC): $(Q3RCCOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(TOOLS_CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

Q3CPPOBJ = \
  $(B)/tools/cpp/cpp.o \
  $(B)/tools/cpp/lex.o \
  $(B)/tools/cpp/nlist.o \
  $(B)/tools/cpp/tokens.o \
  $(B)/tools/cpp/macro.o \
  $(B)/tools/cpp/eval.o \
  $(B)/tools/cpp/include.o \
  $(B)/tools/cpp/hideset.o \
  $(B)/tools/cpp/getopt.o \
  $(B)/tools/cpp/unix.o

$(B)/tools/cpp/%.o: $(Q3CPPDIR)/%.c
	$(DO_TOOLS_CC)

$(Q3CPP): $(Q3CPPOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(TOOLS_CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

Q3LCCOBJ = \
	$(B)/tools/etc/lcc.o \
	$(B)/tools/etc/bytecode.o

$(B)/tools/etc/%.o: $(Q3LCCETCDIR)/%.c
	$(DO_TOOLS_CC)

$(Q3LCC): $(Q3LCCOBJ) $(Q3RCC) $(Q3CPP)
	$(echo_cmd) "LD $@"
	$(Q)$(TOOLS_CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $(Q3LCCOBJ) $(TOOLS_LIBS)

$(STRINGIFY): $(TOOLSDIR)/stringify.c
	$(echo_cmd) "TOOLS_CC $@"
	$(Q)$(TOOLS_CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $(TOOLSDIR)/stringify.c $(TOOLS_LIBS)

define DO_Q3LCC
$(echo_cmd) "Q3LCC $<"
$(Q)$(Q3LCC) $(BASEGAME_CFLAGS) -o $@ $<
endef

define DO_CGAME_Q3LCC
$(echo_cmd) "CGAME_Q3LCC $<"
$(Q)$(Q3LCC) $(BASEGAME_CFLAGS) -DCGAMEDLL -DCGAME -o $@ $<
endef

define DO_GAME_Q3LCC
$(echo_cmd) "GAME_Q3LCC $<"
$(Q)$(Q3LCC) $(BASEGAME_CFLAGS) -DGAMEDLL -DQAGAME -o $@ $<
endef

define DO_UI_Q3LCC
$(echo_cmd) "UI_Q3LCC $<"
$(Q)$(Q3LCC) $(BASEGAME_CFLAGS) -DUI -o $@ $<
endef

Q3ASMOBJ = \
  $(B)/tools/asm/q3asm.o \
  $(B)/tools/asm/cmdlib.o

$(B)/tools/asm/%.o: $(Q3ASMDIR)/%.c
	$(DO_TOOLS_CC)

$(Q3ASM): $(Q3ASMOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(TOOLS_CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)


#############################################################################
# CLIENT/SERVER
#############################################################################
Q3OBJ = \
  $(B)/client/cl_cgame.o \
  $(B)/client/cl_cin.o \
  $(B)/client/cl_console.o \
  $(B)/client/cl_input.o \
  $(B)/client/cl_keys.o \
  $(B)/client/cl_main.o \
  $(B)/client/cl_net_chan.o \
  $(B)/client/cl_parse.o \
  $(B)/client/cl_scrn.o \
  $(B)/client/cl_ui.o \
  $(B)/client/cl_avi.o \
  \
  $(B)/client/cm_load.o \
  $(B)/client/cm_patch.o \
  $(B)/client/cm_polylib.o \
  $(B)/client/cm_test.o \
  $(B)/client/cm_trace.o \
  \
  $(B)/client/cmd.o \
  $(B)/client/common.o \
  $(B)/client/cvar.o \
  $(B)/client/files.o \
  $(B)/client/md4.o \
  $(B)/client/md5.o \
  $(B)/client/msg.o \
  $(B)/client/net_chan.o \
  $(B)/client/net_ip.o \
  $(B)/client/huffman.o \
  \
  $(B)/client/snd_altivec.o \
  $(B)/client/snd_adpcm.o \
  $(B)/client/snd_dma.o \
  $(B)/client/snd_mem.o \
  $(B)/client/snd_mix.o \
  $(B)/client/snd_wavelet.o \
  \
  $(B)/client/snd_main.o \
  $(B)/client/snd_codec.o \
  $(B)/client/snd_codec_wav.o \
  $(B)/client/snd_codec_ogg.o \
  $(B)/client/snd_codec_opus.o \
  \
  $(B)/client/qal.o \
  $(B)/client/snd_openal.o \
  \
  $(B)/client/cl_curl.o \
  \
  $(B)/client/sv_bot.o \
  $(B)/client/sv_ccmds.o \
  $(B)/client/sv_client.o \
  $(B)/client/sv_game.o \
  $(B)/client/sv_init.o \
  $(B)/client/sv_main.o \
  $(B)/client/sv_net_chan.o \
  $(B)/client/sv_snapshot.o \
  $(B)/client/sv_world.o \
  \
  $(B)/client/q_math.o \
  $(B)/client/q_shared.o \
  \
  $(B)/client/unzip.o \
  $(B)/client/ioapi.o \
  $(B)/client/puff.o \
  $(B)/client/vm.o \
  $(B)/client/vm_interpreted.o \
  \
  $(B)/client/be_aas_bspq3.o \
  $(B)/client/be_aas_cluster.o \
  $(B)/client/be_aas_debug.o \
  $(B)/client/be_aas_entity.o \
  $(B)/client/be_aas_file.o \
  $(B)/client/be_aas_main.o \
  $(B)/client/be_aas_move.o \
  $(B)/client/be_aas_optimize.o \
  $(B)/client/be_aas_reach.o \
  $(B)/client/be_aas_route.o \
  $(B)/client/be_aas_routealt.o \
  $(B)/client/be_aas_routetable.o \
  $(B)/client/be_aas_sample.o \
  $(B)/client/be_ai_char.o \
  $(B)/client/be_ai_chat.o \
  $(B)/client/be_ai_gen.o \
  $(B)/client/be_ai_goal.o \
  $(B)/client/be_ai_move.o \
  $(B)/client/be_ai_weap.o \
  $(B)/client/be_ai_weight.o \
  $(B)/client/be_ea.o \
  $(B)/client/be_interface.o \
  $(B)/client/l_crc.o \
  $(B)/client/l_libvar.o \
  $(B)/client/l_log.o \
  $(B)/client/l_memory.o \
  $(B)/client/l_precomp.o \
  $(B)/client/l_script.o \
  $(B)/client/l_struct.o \
  \
  $(B)/splines/math_angles.o \
  $(B)/splines/math_matrix.o \
  $(B)/splines/math_quaternion.o \
  $(B)/splines/math_vector.o \
  $(B)/splines/q_parse.o \
  $(B)/splines/splines.o \
  $(B)/splines/util_str.o \
  \
  $(B)/client/sdl_input.o \
  $(B)/client/sdl_snd.o \
  \
  $(B)/client/con_log.o \
  $(B)/client/sys_main.o

ifdef MINGW
  Q3OBJ += \
    $(B)/client/con_passive.o
else
  Q3OBJ += \
    $(B)/client/con_tty.o
endif

Q3ROBJ = \
  $(B)/renderer/tr_altivec.o \
  $(B)/renderer/tr_animation.o \
  $(B)/renderer/tr_backend.o \
  $(B)/renderer/tr_bsp.o \
  $(B)/renderer/tr_cmds.o \
  $(B)/renderer/tr_cmesh.o \
  $(B)/renderer/tr_curve.o \
  $(B)/renderer/tr_flares.o \
  $(B)/renderer/tr_font.o \
  $(B)/renderer/tr_image.o \
  $(B)/renderer/tr_image_bmp.o \
  $(B)/renderer/tr_image_jpg.o \
  $(B)/renderer/tr_image_pcx.o \
  $(B)/renderer/tr_image_png.o \
  $(B)/renderer/tr_image_tga.o \
  $(B)/renderer/tr_init.o \
  $(B)/renderer/tr_light.o \
  $(B)/renderer/tr_main.o \
  $(B)/renderer/tr_marks.o \
  $(B)/renderer/tr_mesh.o \
  $(B)/renderer/tr_model.o \
  $(B)/renderer/tr_model_iqm.o \
  $(B)/renderer/tr_noise.o \
  $(B)/renderer/tr_scene.o \
  $(B)/renderer/tr_shade.o \
  $(B)/renderer/tr_shade_calc.o \
  $(B)/renderer/tr_shader.o \
  $(B)/renderer/tr_shadows.o \
  $(B)/renderer/tr_sky.o \
  $(B)/renderer/tr_surface.o \
  $(B)/renderer/tr_world.o \

ifeq ($(USE_BLOOM),1)
  Q3ROBJ += $(B)/renderer/tr_bloom.o 
endif

  Q3ROBJ += $(B)/renderer/sdl_gamma.o
  Q3ROBJ += $(B)/renderer/sdl_glimp.o

ifneq ($(USE_RENDERER_DLOPEN), 0)
  Q3ROBJ += \
    $(B)/renderer/q_shared.o \
    $(B)/renderer/puff.o \
    $(B)/renderer/q_math.o \
    $(B)/renderer/tr_subs.o

  Q3R2OBJ += \
    $(B)/renderer/q_shared.o \
    $(B)/renderer/puff.o \
    $(B)/renderer/q_math.o \
    $(B)/renderer/tr_subs.o
endif

ifneq ($(USE_INTERNAL_JPEG),0)
  JPGOBJ = \
    $(B)/renderer/jaricom.o \
    $(B)/renderer/jcapimin.o \
    $(B)/renderer/jcapistd.o \
    $(B)/renderer/jcarith.o \
    $(B)/renderer/jccoefct.o  \
    $(B)/renderer/jccolor.o \
    $(B)/renderer/jcdctmgr.o \
    $(B)/renderer/jchuff.o   \
    $(B)/renderer/jcinit.o \
    $(B)/renderer/jcmainct.o \
    $(B)/renderer/jcmarker.o \
    $(B)/renderer/jcmaster.o \
    $(B)/renderer/jcomapi.o \
    $(B)/renderer/jcparam.o \
    $(B)/renderer/jcprepct.o \
    $(B)/renderer/jcsample.o \
    $(B)/renderer/jctrans.o \
    $(B)/renderer/jdapimin.o \
    $(B)/renderer/jdapistd.o \
    $(B)/renderer/jdarith.o \
    $(B)/renderer/jdatadst.o \
    $(B)/renderer/jdatasrc.o \
    $(B)/renderer/jdcoefct.o \
    $(B)/renderer/jdcolor.o \
    $(B)/renderer/jddctmgr.o \
    $(B)/renderer/jdhuff.o \
    $(B)/renderer/jdinput.o \
    $(B)/renderer/jdmainct.o \
    $(B)/renderer/jdmarker.o \
    $(B)/renderer/jdmaster.o \
    $(B)/renderer/jdmerge.o \
    $(B)/renderer/jdpostct.o \
    $(B)/renderer/jdsample.o \
    $(B)/renderer/jdtrans.o \
    $(B)/renderer/jerror.o \
    $(B)/renderer/jfdctflt.o \
    $(B)/renderer/jfdctfst.o \
    $(B)/renderer/jfdctint.o \
    $(B)/renderer/jidctflt.o \
    $(B)/renderer/jidctfst.o \
    $(B)/renderer/jidctint.o \
    $(B)/renderer/jmemmgr.o \
    $(B)/renderer/jmemnobs.o \
    $(B)/renderer/jquant1.o \
    $(B)/renderer/jquant2.o \
    $(B)/renderer/jutils.o
endif

ifeq ($(USE_FREETYPE),1)
ifneq ($(USE_INTERNAL_FREETYPE),0)
  FTOBJ += \
    $(B)/renderer/ftsystem.o \
    $(B)/renderer/ftdebug.o \
    $(B)/renderer/ftinit.o \
    $(B)/renderer/ftbase.o \
    $(B)/renderer/ftbbox.o \
    $(B)/renderer/ftbdf.o \
    $(B)/renderer/ftbitmap.o \
    $(B)/renderer/ftcid.o \
    $(B)/renderer/ftfntfmt.o \
    $(B)/renderer/ftfstype.o \
    $(B)/renderer/ftgasp.o \
    $(B)/renderer/ftglyph.o \
    $(B)/renderer/ftgxval.o \
    $(B)/renderer/ftlcdfil.o \
    $(B)/renderer/ftmm.o \
    $(B)/renderer/ftotval.o \
    $(B)/renderer/ftpatent.o \
    $(B)/renderer/ftpfr.o \
    $(B)/renderer/ftstroke.o \
    $(B)/renderer/ftsynth.o \
    $(B)/renderer/fttype1.o \
    $(B)/renderer/ftwinfnt.o \
    $(B)/renderer/truetype.o \
    $(B)/renderer/type1.o \
    $(B)/renderer/cff.o \
    $(B)/renderer/type1cid.o \
    $(B)/renderer/pfr.o \
    $(B)/renderer/type42.o \
    $(B)/renderer/winfnt.o \
    $(B)/renderer/pcf.o \
    $(B)/renderer/bdf.o \
    $(B)/renderer/sfnt.o \
    $(B)/renderer/autofit.o \
    $(B)/renderer/pshinter.o \
    $(B)/renderer/raster.o \
    $(B)/renderer/smooth.o \
    $(B)/renderer/ftcache.o \
    $(B)/renderer/ftgzip.o \
    $(B)/renderer/ftlzw.o \
    $(B)/renderer/ftbzip2.o \
    $(B)/renderer/psaux.o \
    $(B)/renderer/psnames.o
endif
endif

ifeq ($(ARCH),x86)
  Q3OBJ += \
    $(B)/client/snd_mixa.o \
    $(B)/client/matha.o \
    $(B)/client/snapvector.o \
    $(B)/client/ftola.o
endif
ifeq ($(ARCH),x86_64)
  Q3OBJ += \
    $(B)/client/snapvector.o \
    $(B)/client/ftola.o
endif

ifeq ($(NEED_OPUS),1)
ifeq ($(USE_INTERNAL_OPUS),1)
Q3OBJ += \
  $(B)/client/opus/opus.o \
  $(B)/client/opus/opus_decoder.o \
  $(B)/client/opus/opus_encoder.o \
  $(B)/client/opus/opus_multistream.o \
  $(B)/client/opus/opus_multistream_encoder.o \
  $(B)/client/opus/opus_multistream_decoder.o \
  $(B)/client/opus/repacketizer.o \
  $(B)/client/opus/analysis.o \
  $(B)/client/opus/mlp.o \
  $(B)/client/opus/mlp_data.o \
  \
  $(B)/client/opus/bands.o \
  $(B)/client/opus/celt.o \
  $(B)/client/opus/celt_encoder.o \
  $(B)/client/opus/celt_decoder.o \
  $(B)/client/opus/cwrs.o \
  $(B)/client/opus/entcode.o \
  $(B)/client/opus/entdec.o \
  $(B)/client/opus/entenc.o \
  $(B)/client/opus/kiss_fft.o \
  $(B)/client/opus/laplace.o \
  $(B)/client/opus/mathops.o \
  $(B)/client/opus/mdct.o \
  $(B)/client/opus/modes.o \
  $(B)/client/opus/pitch.o \
  $(B)/client/opus/celt_lpc.o \
  $(B)/client/opus/quant_bands.o \
  $(B)/client/opus/rate.o \
  $(B)/client/opus/vq.o \
  \
  $(B)/client/opus/CNG.o \
  $(B)/client/opus/code_signs.o \
  $(B)/client/opus/init_decoder.o \
  $(B)/client/opus/decode_core.o \
  $(B)/client/opus/decode_frame.o \
  $(B)/client/opus/decode_parameters.o \
  $(B)/client/opus/decode_indices.o \
  $(B)/client/opus/decode_pulses.o \
  $(B)/client/opus/decoder_set_fs.o \
  $(B)/client/opus/dec_API.o \
  $(B)/client/opus/enc_API.o \
  $(B)/client/opus/encode_indices.o \
  $(B)/client/opus/encode_pulses.o \
  $(B)/client/opus/gain_quant.o \
  $(B)/client/opus/interpolate.o \
  $(B)/client/opus/LP_variable_cutoff.o \
  $(B)/client/opus/NLSF_decode.o \
  $(B)/client/opus/NSQ.o \
  $(B)/client/opus/NSQ_del_dec.o \
  $(B)/client/opus/PLC.o \
  $(B)/client/opus/shell_coder.o \
  $(B)/client/opus/tables_gain.o \
  $(B)/client/opus/tables_LTP.o \
  $(B)/client/opus/tables_NLSF_CB_NB_MB.o \
  $(B)/client/opus/tables_NLSF_CB_WB.o \
  $(B)/client/opus/tables_other.o \
  $(B)/client/opus/tables_pitch_lag.o \
  $(B)/client/opus/tables_pulses_per_block.o \
  $(B)/client/opus/VAD.o \
  $(B)/client/opus/control_audio_bandwidth.o \
  $(B)/client/opus/quant_LTP_gains.o \
  $(B)/client/opus/VQ_WMat_EC.o \
  $(B)/client/opus/HP_variable_cutoff.o \
  $(B)/client/opus/NLSF_encode.o \
  $(B)/client/opus/NLSF_VQ.o \
  $(B)/client/opus/NLSF_unpack.o \
  $(B)/client/opus/NLSF_del_dec_quant.o \
  $(B)/client/opus/process_NLSFs.o \
  $(B)/client/opus/stereo_LR_to_MS.o \
  $(B)/client/opus/stereo_MS_to_LR.o \
  $(B)/client/opus/check_control_input.o \
  $(B)/client/opus/control_SNR.o \
  $(B)/client/opus/init_encoder.o \
  $(B)/client/opus/control_codec.o \
  $(B)/client/opus/A2NLSF.o \
  $(B)/client/opus/ana_filt_bank_1.o \
  $(B)/client/opus/biquad_alt.o \
  $(B)/client/opus/bwexpander_32.o \
  $(B)/client/opus/bwexpander.o \
  $(B)/client/opus/debug.o \
  $(B)/client/opus/decode_pitch.o \
  $(B)/client/opus/inner_prod_aligned.o \
  $(B)/client/opus/lin2log.o \
  $(B)/client/opus/log2lin.o \
  $(B)/client/opus/LPC_analysis_filter.o \
  $(B)/client/opus/LPC_inv_pred_gain.o \
  $(B)/client/opus/table_LSF_cos.o \
  $(B)/client/opus/NLSF2A.o \
  $(B)/client/opus/NLSF_stabilize.o \
  $(B)/client/opus/NLSF_VQ_weights_laroia.o \
  $(B)/client/opus/pitch_est_tables.o \
  $(B)/client/opus/resampler.o \
  $(B)/client/opus/resampler_down2_3.o \
  $(B)/client/opus/resampler_down2.o \
  $(B)/client/opus/resampler_private_AR2.o \
  $(B)/client/opus/resampler_private_down_FIR.o \
  $(B)/client/opus/resampler_private_IIR_FIR.o \
  $(B)/client/opus/resampler_private_up2_HQ.o \
  $(B)/client/opus/resampler_rom.o \
  $(B)/client/opus/sigm_Q15.o \
  $(B)/client/opus/sort.o \
  $(B)/client/opus/sum_sqr_shift.o \
  $(B)/client/opus/stereo_decode_pred.o \
  $(B)/client/opus/stereo_encode_pred.o \
  $(B)/client/opus/stereo_find_predictor.o \
  $(B)/client/opus/stereo_quant_pred.o \
  $(B)/client/opus/LPC_fit.o \
  \
  $(B)/client/opus/apply_sine_window_FLP.o \
  $(B)/client/opus/corrMatrix_FLP.o \
  $(B)/client/opus/encode_frame_FLP.o \
  $(B)/client/opus/find_LPC_FLP.o \
  $(B)/client/opus/find_LTP_FLP.o \
  $(B)/client/opus/find_pitch_lags_FLP.o \
  $(B)/client/opus/find_pred_coefs_FLP.o \
  $(B)/client/opus/LPC_analysis_filter_FLP.o \
  $(B)/client/opus/LTP_analysis_filter_FLP.o \
  $(B)/client/opus/LTP_scale_ctrl_FLP.o \
  $(B)/client/opus/noise_shape_analysis_FLP.o \
  $(B)/client/opus/process_gains_FLP.o \
  $(B)/client/opus/regularize_correlations_FLP.o \
  $(B)/client/opus/residual_energy_FLP.o \
  $(B)/client/opus/warped_autocorrelation_FLP.o \
  $(B)/client/opus/wrappers_FLP.o \
  $(B)/client/opus/autocorrelation_FLP.o \
  $(B)/client/opus/burg_modified_FLP.o \
  $(B)/client/opus/bwexpander_FLP.o \
  $(B)/client/opus/energy_FLP.o \
  $(B)/client/opus/inner_product_FLP.o \
  $(B)/client/opus/k2a_FLP.o \
  $(B)/client/opus/LPC_inv_pred_gain_FLP.o \
  $(B)/client/opus/pitch_analysis_core_FLP.o \
  $(B)/client/opus/scale_copy_vector_FLP.o \
  $(B)/client/opus/scale_vector_FLP.o \
  $(B)/client/opus/schur_FLP.o \
  $(B)/client/opus/sort_FLP.o \
  \
  $(B)/client/info.o \
  $(B)/client/internal.o \
  $(B)/client/opusfile.o \
  $(B)/client/stream.o \
  $(B)/client/http.o
ifdef MINGW
Q3OBJ += \
  $(B)/client/wincerts.o
endif
endif
endif

ifeq ($(NEED_OGG),1)
ifeq ($(USE_INTERNAL_OGG),1)
Q3OBJ += \
  $(B)/client/bitwise.o \
  $(B)/client/framing.o
endif
endif

ifeq ($(USE_CODEC_VORBIS),1)
ifeq ($(USE_INTERNAL_VORBIS),1)
Q3OBJ += \
  $(B)/client/vorbis/analysis.o \
  $(B)/client/vorbis/bitrate.o \
  $(B)/client/vorbis/block.o \
  $(B)/client/vorbis/codebook.o \
  $(B)/client/vorbis/envelope.o \
  $(B)/client/vorbis/floor0.o \
  $(B)/client/vorbis/floor1.o \
  $(B)/client/vorbis/info.o \
  $(B)/client/vorbis/lookup.o \
  $(B)/client/vorbis/lpc.o \
  $(B)/client/vorbis/lsp.o \
  $(B)/client/vorbis/mapping0.o \
  $(B)/client/vorbis/mdct.o \
  $(B)/client/vorbis/psy.o \
  $(B)/client/vorbis/registry.o \
  $(B)/client/vorbis/res0.o \
  $(B)/client/vorbis/sharedbook.o \
  $(B)/client/vorbis/smallft.o \
  $(B)/client/vorbis/synthesis.o \
  $(B)/client/vorbis/vorbisenc.o \
  $(B)/client/vorbis/vorbisfile.o \
  $(B)/client/vorbis/window.o
endif
endif

ifeq ($(USE_INTERNAL_ZLIB),1)
Q3OBJ += \
  $(B)/client/adler32.o \
  $(B)/client/crc32.o \
  $(B)/client/inffast.o \
  $(B)/client/inflate.o \
  $(B)/client/inftrees.o \
  $(B)/client/zutil.o
endif

ifeq ($(HAVE_VM_COMPILED),true)
  ifneq ($(findstring $(ARCH),x86 x86_64),)
    Q3OBJ += \
      $(B)/client/vm_x86.o
  endif
  ifneq ($(findstring $(ARCH),ppc),)
    Q3OBJ += $(B)/client/vm_powerpc.o $(B)/client/vm_powerpc_asm.o
  endif
  ifneq ($(findstring $(ARCH),sparc),)
    Q3OBJ += $(B)/client/vm_sparc.o
  endif
  ifeq ($(ARCH),armv7l)
    Q3OBJ += $(B)/client/vm_armv7l.o
  endif
endif

ifdef MINGW
  Q3OBJ += \
    $(B)/client/win_resource.o \
    $(B)/client/sys_win32.o
else
  Q3OBJ += \
    $(B)/client/sys_unix.o
endif

ifeq ($(PLATFORM),darwin)
  Q3OBJ += \
    $(B)/client/sys_osx.o
endif

ifeq ($(USE_MUMBLE),1)
  Q3OBJ += \
    $(B)/client/libmumblelink.o
endif

ifneq ($(USE_RENDERER_DLOPEN),0)
$(B)/$(CLIENTBIN)$(FULLBINEXT): $(Q3OBJ) $(LIBSDLMAIN)
	$(echo_cmd) "LD $@"
	$(Q)$(CXX) $(CLIENT_CFLAGS) $(CFLAGS) $(CLIENT_LDFLAGS) $(LDFLAGS) $(NOTSHLIBLDFLAGS) \
		-o $@ $(Q3OBJ) \
		$(LIBSDLMAIN) $(CLIENT_LIBS) $(LIBS)

$(B)/renderer_sp_opengl1_$(SHLIBNAME): $(Q3ROBJ) $(JPGOBJ) $(FTOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(Q3ROBJ) $(JPGOBJ) $(FTOBJ) \
		$(THREAD_LIBS) $(LIBSDLMAIN) $(RENDERER_LIBS) $(LIBS)

else
$(B)/$(CLIENTBIN)$(FULLBINEXT): $(Q3OBJ) $(Q3ROBJ) $(JPGOBJ) $(FTOBJ) $(LIBSDLMAIN)
	$(echo_cmd) "LD $@"
	$(Q)$(CXX) $(CLIENT_CFLAGS) $(CFLAGS) $(CLIENT_LDFLAGS) $(LDFLAGS) $(NOTSHLIBLDFLAGS) \
		-o $@ $(Q3OBJ) $(Q3ROBJ) $(JPGOBJ) $(FTOBJ) \
		$(LIBSDLMAIN) $(CLIENT_LIBS) $(RENDERER_LIBS) $(LIBS)

endif

ifneq ($(strip $(LIBSDLMAIN)),)
ifneq ($(strip $(LIBSDLMAINSRC)),)
$(LIBSDLMAIN) : $(LIBSDLMAINSRC)
ifeq ($(PLATFORM),darwin)
	$(LIPO) -extract $(MACOSX_ARCH) $< -o $@
else
	cp $< $@
endif
	$(RANLIB) $@
endif
endif



#############################################################################
# DEDICATED SERVER
#############################################################################

Q3DOBJ = \
  $(B)/ded/sv_bot.o \
  $(B)/ded/sv_client.o \
  $(B)/ded/sv_ccmds.o \
  $(B)/ded/sv_game.o \
  $(B)/ded/sv_init.o \
  $(B)/ded/sv_main.o \
  $(B)/ded/sv_net_chan.o \
  $(B)/ded/sv_snapshot.o \
  $(B)/ded/sv_world.o \
  \
  $(B)/ded/cm_load.o \
  $(B)/ded/cm_patch.o \
  $(B)/ded/cm_polylib.o \
  $(B)/ded/cm_test.o \
  $(B)/ded/cm_trace.o \
  $(B)/ded/cmd.o \
  $(B)/ded/common.o \
  $(B)/ded/cvar.o \
  $(B)/ded/files.o \
  $(B)/ded/md4.o \
  $(B)/ded/msg.o \
  $(B)/ded/net_chan.o \
  $(B)/ded/net_ip.o \
  $(B)/ded/huffman.o \
  \
  $(B)/ded/q_math.o \
  $(B)/ded/q_shared.o \
  \
  $(B)/ded/unzip.o \
  $(B)/ded/ioapi.o \
  $(B)/ded/vm.o \
  $(B)/ded/vm_interpreted.o \
  \
  $(B)/ded/be_aas_bspq3.o \
  $(B)/ded/be_aas_cluster.o \
  $(B)/ded/be_aas_debug.o \
  $(B)/ded/be_aas_entity.o \
  $(B)/ded/be_aas_file.o \
  $(B)/ded/be_aas_main.o \
  $(B)/ded/be_aas_move.o \
  $(B)/ded/be_aas_optimize.o \
  $(B)/ded/be_aas_reach.o \
  $(B)/ded/be_aas_route.o \
  $(B)/ded/be_aas_routealt.o \
  $(B)/ded/be_aas_routetable.o \
  $(B)/ded/be_aas_sample.o \
  $(B)/ded/be_ai_char.o \
  $(B)/ded/be_ai_chat.o \
  $(B)/ded/be_ai_gen.o \
  $(B)/ded/be_ai_goal.o \
  $(B)/ded/be_ai_move.o \
  $(B)/ded/be_ai_weap.o \
  $(B)/ded/be_ai_weight.o \
  $(B)/ded/be_ea.o \
  $(B)/ded/be_interface.o \
  $(B)/ded/l_crc.o \
  $(B)/ded/l_libvar.o \
  $(B)/ded/l_log.o \
  $(B)/ded/l_memory.o \
  $(B)/ded/l_precomp.o \
  $(B)/ded/l_script.o \
  $(B)/ded/l_struct.o \
  \
  $(B)/ded/null_client.o \
  $(B)/ded/null_input.o \
  $(B)/ded/null_snddma.o \
  \
  $(B)/ded/con_log.o \
  $(B)/ded/sys_main.o

ifeq ($(ARCH),x86)
  Q3DOBJ += \
      $(B)/ded/matha.o \
      $(B)/ded/snapvector.o \
      $(B)/ded/ftola.o
endif
ifeq ($(ARCH),x86_64)
  Q3DOBJ += \
      $(B)/ded/snapvector.o \
      $(B)/ded/ftola.o
endif

ifeq ($(USE_INTERNAL_ZLIB),1)
Q3DOBJ += \
  $(B)/ded/adler32.o \
  $(B)/ded/crc32.o \
  $(B)/ded/inffast.o \
  $(B)/ded/inflate.o \
  $(B)/ded/inftrees.o \
  $(B)/ded/zutil.o
endif

ifeq ($(HAVE_VM_COMPILED),true)
  ifneq ($(findstring $(ARCH),x86 x86_64),)
    Q3DOBJ += \
      $(B)/ded/vm_x86.o
  endif
  ifneq ($(findstring $(ARCH),ppc),)
    Q3DOBJ += $(B)/ded/vm_powerpc.o $(B)/ded/vm_powerpc_asm.o
  endif
  ifneq ($(findstring $(ARCH),sparc),)
    Q3DOBJ += $(B)/ded/vm_sparc.o
  endif
  ifeq ($(ARCH),armv7l)
    Q3DOBJ += $(B)/ded/vm_armv7l.o
  endif
endif

ifdef MINGW
  Q3DOBJ += \
    $(B)/ded/win_resource.o \
    $(B)/ded/sys_win32.o \
    $(B)/ded/con_win32.o
else
  Q3DOBJ += \
    $(B)/ded/sys_unix.o \
    $(B)/ded/con_tty.o
endif

ifeq ($(PLATFORM),darwin)
  Q3DOBJ += \
    $(B)/ded/sys_osx.o
endif

$(B)/$(SERVERBIN)$(FULLBINEXT): $(Q3DOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) $(NOTSHLIBLDFLAGS) -o $@ $(Q3DOBJ) $(LIBS)



#############################################################################
## BASEQ3 CGAME
#############################################################################

Q3CGOBJ_ = \
  $(B)/$(BASEGAME)/cgame/cg_main.o \
  $(B)/$(BASEGAME)/cgame/bg_animation.o \
  $(B)/$(BASEGAME)/cgame/bg_misc.o \
  $(B)/$(BASEGAME)/cgame/bg_pmove.o \
  $(B)/$(BASEGAME)/cgame/bg_slidemove.o \
  $(B)/$(BASEGAME)/cgame/bg_lib.o \
  $(B)/$(BASEGAME)/cgame/cg_atmospheric.o \
  $(B)/$(BASEGAME)/cgame/cg_consolecmds.o \
  $(B)/$(BASEGAME)/cgame/cg_draw.o \
  $(B)/$(BASEGAME)/cgame/cg_drawtools.o \
  $(B)/$(BASEGAME)/cgame/cg_effects.o \
  $(B)/$(BASEGAME)/cgame/cg_ents.o \
  $(B)/$(BASEGAME)/cgame/cg_event.o \
  $(B)/$(BASEGAME)/cgame/cg_flamethrower.o \
  $(B)/$(BASEGAME)/cgame/cg_info.o \
  $(B)/$(BASEGAME)/cgame/cg_localents.o \
  $(B)/$(BASEGAME)/cgame/cg_marks.o \
  $(B)/$(BASEGAME)/cgame/cg_newdraw.o \
  $(B)/$(BASEGAME)/cgame/cg_particles.o \
  $(B)/$(BASEGAME)/cgame/cg_players.o \
  $(B)/$(BASEGAME)/cgame/cg_playerstate.o \
  $(B)/$(BASEGAME)/cgame/cg_polybus.o \
  $(B)/$(BASEGAME)/cgame/cg_predict.o \
  $(B)/$(BASEGAME)/cgame/cg_scoreboard.o \
  $(B)/$(BASEGAME)/cgame/cg_servercmds.o \
  $(B)/$(BASEGAME)/cgame/cg_snapshot.o \
  $(B)/$(BASEGAME)/cgame/cg_sound.o \
  $(B)/$(BASEGAME)/cgame/cg_trails.o \
  $(B)/$(BASEGAME)/cgame/cg_view.o \
  $(B)/$(BASEGAME)/cgame/cg_weapons.o \
  $(B)/$(BASEGAME)/ui/ui_shared.o \
  \
  $(B)/$(BASEGAME)/cgame/q_math.o \
  $(B)/$(BASEGAME)/cgame/q_shared.o

Q3CGOBJ = $(Q3CGOBJ_) $(B)/$(BASEGAME)/cgame/cg_syscalls.o
Q3CGVMOBJ = $(Q3CGOBJ_:%.o=%.asm)

ifdef MINGW
$(B)/$(BASEGAME)/cgame_sp_$(SHLIBNAME): $(Q3CGOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(Q3CGOBJ) $(LIBS)
else
$(B)/$(BASEGAME)/cgame.sp.$(SHLIBNAME): $(Q3CGOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(Q3CGOBJ) $(LIBS)
endif
$(B)/$(BASEGAME)/vm/cgame.sp.qvm: $(Q3CGVMOBJ) $(CGDIR)/cg_syscalls.asm $(Q3ASM)
	$(echo_cmd) "Q3ASM $@"
	$(Q)$(Q3ASM) -o $@ $(Q3CGVMOBJ) $(CGDIR)/cg_syscalls.asm


#############################################################################
## BASEQ3 GAME
#############################################################################

Q3GOBJ_ = \
  $(B)/$(BASEGAME)/game/steam.o \
  $(B)/$(BASEGAME)/game/steamshim_child.o \
  $(B)/$(BASEGAME)/game/g_main.o \
  $(B)/$(BASEGAME)/game/ai_cast.o \
  $(B)/$(BASEGAME)/game/ai_cast_characters.o \
  $(B)/$(BASEGAME)/game/ai_cast_debug.o \
  $(B)/$(BASEGAME)/game/ai_cast_events.o \
  $(B)/$(BASEGAME)/game/ai_cast_fight.o \
  $(B)/$(BASEGAME)/game/ai_cast_func_attack.o \
  $(B)/$(BASEGAME)/game/ai_cast_func_boss1.o \
  $(B)/$(BASEGAME)/game/ai_cast_funcs.o \
  $(B)/$(BASEGAME)/game/ai_cast_script_actions.o \
  $(B)/$(BASEGAME)/game/ai_cast_script.o \
  $(B)/$(BASEGAME)/game/ai_cast_script_ents.o \
  $(B)/$(BASEGAME)/game/ai_cast_survival.o \
  $(B)/$(BASEGAME)/game/ai_cast_sight.o \
  $(B)/$(BASEGAME)/game/ai_cast_think.o \
  $(B)/$(BASEGAME)/game/ai_chat.o \
  $(B)/$(BASEGAME)/game/ai_cmd.o \
  $(B)/$(BASEGAME)/game/ai_dmnet.o \
  $(B)/$(BASEGAME)/game/ai_dmq3.o \
  $(B)/$(BASEGAME)/game/ai_main.o \
  $(B)/$(BASEGAME)/game/ai_team.o \
  $(B)/$(BASEGAME)/game/bg_animation.o \
  $(B)/$(BASEGAME)/game/bg_misc.o \
  $(B)/$(BASEGAME)/game/bg_pmove.o \
  $(B)/$(BASEGAME)/game/bg_slidemove.o \
  $(B)/$(BASEGAME)/game/bg_lib.o \
  $(B)/$(BASEGAME)/game/g_active.o \
  $(B)/$(BASEGAME)/game/g_alarm.o \
  $(B)/$(BASEGAME)/game/g_bot.o \
  $(B)/$(BASEGAME)/game/g_client.o \
  $(B)/$(BASEGAME)/game/g_cmds.o \
  $(B)/$(BASEGAME)/game/g_combat.o \
  $(B)/$(BASEGAME)/game/g_items.o \
  $(B)/$(BASEGAME)/game/g_mem.o \
  $(B)/$(BASEGAME)/game/g_misc.o \
  $(B)/$(BASEGAME)/game/g_missile.o \
  $(B)/$(BASEGAME)/game/g_mover.o \
  $(B)/$(BASEGAME)/game/g_props.o \
  $(B)/$(BASEGAME)/game/g_save.o \
  $(B)/$(BASEGAME)/game/g_script_actions.o \
  $(B)/$(BASEGAME)/game/g_script.o \
  $(B)/$(BASEGAME)/game/g_session.o \
  $(B)/$(BASEGAME)/game/g_spawn.o \
  $(B)/$(BASEGAME)/game/g_svcmds.o \
  $(B)/$(BASEGAME)/game/g_survival_buy.o \
  $(B)/$(BASEGAME)/game/g_survival_score.o \
  $(B)/$(BASEGAME)/game/g_survival_misc.o \
  $(B)/$(BASEGAME)/game/g_target.o \
  $(B)/$(BASEGAME)/game/g_team.o \
  $(B)/$(BASEGAME)/game/g_tramcar.o \
  $(B)/$(BASEGAME)/game/g_trigger.o \
  $(B)/$(BASEGAME)/game/g_utils.o \
  $(B)/$(BASEGAME)/game/g_weapon.o \
  \
  $(B)/$(BASEGAME)/game/q_math.o \
  $(B)/$(BASEGAME)/game/q_shared.o

Q3GOBJ = $(Q3GOBJ_) $(B)/$(BASEGAME)/game/g_syscalls.o
Q3GVMOBJ = $(Q3GOBJ_:%.o=%.asm)

ifdef MINGW
$(B)/$(BASEGAME)/qagame_sp_$(SHLIBNAME): $(Q3GOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(Q3GOBJ) $(LIBS)
else
$(B)/$(BASEGAME)/qagame.sp.$(SHLIBNAME): $(Q3GOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(Q3GOBJ) $(LIBS)
endif
$(B)/$(BASEGAME)/vm/qagame.sp.qvm: $(Q3GVMOBJ) $(GDIR)/g_syscalls.asm $(Q3ASM)
	$(echo_cmd) "Q3ASM $@"
	$(Q)$(Q3ASM) -o $@ $(Q3GVMOBJ) $(GDIR)/g_syscalls.asm


#############################################################################
## BASEQ3 UI
#############################################################################

Q3UIOBJ_ = \
  $(B)/$(BASEGAME)/ui/ui_main.o \
  $(B)/$(BASEGAME)/ui/ui_atoms.o \
  $(B)/$(BASEGAME)/ui/ui_gameinfo.o \
  $(B)/$(BASEGAME)/ui/ui_players.o \
  $(B)/$(BASEGAME)/ui/ui_shared.o \
  \
  $(B)/$(BASEGAME)/ui/bg_misc.o \
  $(B)/$(BASEGAME)/ui/bg_lib.o \
  \
  $(B)/$(BASEGAME)/ui/q_math.o \
  $(B)/$(BASEGAME)/ui/q_shared.o

Q3UIOBJ = $(Q3UIOBJ_) $(B)/$(BASEGAME)/ui/ui_syscalls.o
Q3UIVMOBJ = $(Q3UIOBJ_:%.o=%.asm)

ifdef MINGW
$(B)/$(BASEGAME)/ui_sp_$(SHLIBNAME): $(Q3UIOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(Q3UIOBJ) $(LIBS)
else
$(B)/$(BASEGAME)/ui.sp.$(SHLIBNAME): $(Q3UIOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(Q3UIOBJ) $(LIBS)
endif
$(B)/$(BASEGAME)/vm/ui.sp.qvm: $(Q3UIVMOBJ) $(UIDIR)/ui_syscalls.asm $(Q3ASM)
	$(echo_cmd) "Q3ASM $@"
	$(Q)$(Q3ASM) -o $@ $(Q3UIVMOBJ) $(UIDIR)/ui_syscalls.asm




#############################################################################
## STEAM INTEGRATION
#############################################################################

$(B)/$(BASEGAME)/game/%.o: $(STEAMSHIMDIR)/%.c
	$(DO_GAME_CC)

$(B)/$(BASEGAME)/game/%.o: $(STEAMDIR)/%.c
	$(DO_GAME_CC)

#############################################################################
## CLIENT/SERVER RULES
#############################################################################

$(B)/client/%.o: $(ASMDIR)/%.s
	$(DO_AS)

# k8 so inline assembler knows about SSE
$(B)/client/%.o: $(ASMDIR)/%.c
	$(DO_CC) -march=k8

$(B)/client/snd_altivec.o: $(CDIR)/snd_altivec.c
	$(DO_CC_ALTIVEC)

$(B)/client/%.o: $(CDIR)/%.c
	$(DO_CC)

$(B)/client/%.o: $(SDIR)/%.c
	$(DO_CC)

$(B)/client/%.o: $(CMDIR)/%.c
	$(DO_CC)

$(B)/client/%.o: $(BLIBDIR)/%.c
	$(DO_BOT_CC)

$(B)/client/%.o: $(OGGDIR)/src/%.c
	$(DO_CC)

$(B)/client/vorbis/%.o: $(VORBISDIR)/lib/%.c
	$(DO_CC)

$(B)/client/opus/%.o: $(OPUSDIR)/src/%.c
	$(DO_CC)

$(B)/client/opus/%.o: $(OPUSDIR)/celt/%.c
	$(DO_CC)

$(B)/client/opus/%.o: $(OPUSDIR)/silk/%.c
	$(DO_CC)

$(B)/client/opus/%.o: $(OPUSDIR)/silk/float/%.c
	$(DO_CC)

$(B)/client/%.o: $(OPUSFILEDIR)/src/%.c
	$(DO_CC)

$(B)/client/%.o: $(ZDIR)/%.c
	$(DO_CC)

$(B)/client/%.o: $(SDLDIR)/%.c
	$(DO_CC)

$(B)/client/%.o: $(SYSDIR)/%.c
	$(DO_CC)

$(B)/client/%.o: $(SYSDIR)/%.m
	$(DO_CC)

$(B)/client/win_resource.o: $(SYSDIR)/win_resource.rc $(SYSDIR)/win_manifest.xml
	$(DO_WINDRES)

$(B)/splines/%.o: $(SPLDIR)/%.cpp
	$(DO_SPLINE_CXX)

$(B)/renderer/%.o: $(CMDIR)/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(SDLDIR)/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(JPDIR)/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(RDIR)/%.c
	$(DO_REF_CC)

$(B)/renderer/tr_altivec.o: $(RDIR)/tr_altivec.c
	$(DO_REF_CC_ALTIVEC)

$(B)/renderer/%.o: $(FTDIR)/src/autofit/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/base/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/bdf/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/bzip2/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/cache/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/cff/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/cid/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/gxvalid/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/gzip/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/lzw/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/otvalid/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/pcf/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/pfr/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/psaux/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/pshinter/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/psnames/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/raster/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/sfnt/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/smooth/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/tools/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/truetype/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/type1/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/type42/%.c
	$(DO_REF_CC)

$(B)/renderer/%.o: $(FTDIR)/src/winfonts/%.c
	$(DO_REF_CC)

$(B)/ded/%.o: $(ASMDIR)/%.s
	$(DO_AS)

# k8 so inline assembler knows about SSE
$(B)/ded/%.o: $(ASMDIR)/%.c
	$(DO_CC) -march=k8

$(B)/ded/%.o: $(SDIR)/%.c
	$(DO_DED_CC)

$(B)/ded/%.o: $(CMDIR)/%.c
	$(DO_DED_CC)

$(B)/ded/%.o: $(ZDIR)/%.c
	$(DO_DED_CC)

$(B)/ded/%.o: $(BLIBDIR)/%.c
	$(DO_BOT_CC)

$(B)/ded/%.o: $(SYSDIR)/%.c
	$(DO_DED_CC)

$(B)/ded/%.o: $(SYSDIR)/%.m
	$(DO_DED_CC)

$(B)/ded/win_resource.o: $(SYSDIR)/win_resource.rc $(SYSDIR)/win_manifest.xml
	$(DO_WINDRES)

$(B)/ded/%.o: $(NDIR)/%.c
	$(DO_DED_CC)

# Extra dependencies to ensure the git version is incorporated
ifeq ($(USE_GIT),1)
  $(B)/client/cl_console.o : ../.git
  $(B)/client/common.o : ../.git
  $(B)/ded/common.o : ../.git
endif


#############################################################################
## GAME MODULE RULES
#############################################################################

$(B)/$(BASEGAME)/cgame/bg_%.o: $(GDIR)/bg_%.c
	$(DO_CGAME_CC)

$(B)/$(BASEGAME)/cgame/%.o: $(CGDIR)/%.c
	$(DO_CGAME_CC)

$(B)/$(BASEGAME)/cgame/bg_%.asm: $(GDIR)/bg_%.c $(Q3LCC)
	$(DO_CGAME_Q3LCC)

$(B)/$(BASEGAME)/cgame/%.asm: $(CGDIR)/%.c $(Q3LCC)
	$(DO_CGAME_Q3LCC)

$(B)/$(BASEGAME)/game/%.o: $(GDIR)/%.c
	$(DO_GAME_CC)

$(B)/$(BASEGAME)/game/%.asm: $(GDIR)/%.c $(Q3LCC)
	$(DO_GAME_Q3LCC)

$(B)/$(BASEGAME)/ui/bg_%.o: $(GDIR)/bg_%.c
	$(DO_UI_CC)

$(B)/$(BASEGAME)/ui/%.o: $(UIDIR)/%.c
	$(DO_UI_CC)

$(B)/$(BASEGAME)/ui/bg_%.asm: $(GDIR)/bg_%.c $(Q3LCC)
	$(DO_UI_Q3LCC)

$(B)/$(BASEGAME)/ui/%.asm: $(UIDIR)/%.c $(Q3LCC)
	$(DO_UI_Q3LCC)

$(B)/$(BASEGAME)/qcommon/%.o: $(CMDIR)/%.c
	$(DO_SHLIB_CC)

$(B)/$(BASEGAME)/qcommon/%.asm: $(CMDIR)/%.c $(Q3LCC)
	$(DO_Q3LCC)

# Added
$(B)/$(BASEGAME)/cgame/q_math.o: $(CMDIR)/q_math.c
	$(DO_CGAME_CC)
$(B)/$(BASEGAME)/cgame/q_shared.o: $(CMDIR)/q_shared.c
	$(DO_CGAME_CC)
$(B)/$(BASEGAME)/cgame/q_math.asm: $(CMDIR)/q_math.c $(Q3LCC)
	$(DO_CGAME_Q3LCC)
$(B)/$(BASEGAME)/cgame/q_shared.asm: $(CMDIR)/q_shared.c $(Q3LCC)
	$(DO_CGAME_Q3LCC)

# Added
$(B)/$(BASEGAME)/game/q_math.o: $(CMDIR)/q_math.c
	$(DO_GAME_CC)
$(B)/$(BASEGAME)/game/q_shared.o: $(CMDIR)/q_shared.c
	$(DO_GAME_CC)
$(B)/$(BASEGAME)/game/q_math.asm: $(CMDIR)/q_math.c $(Q3LCC)
	$(DO_GAME_Q3LCC)
$(B)/$(BASEGAME)/game/q_shared.asm: $(CMDIR)/q_shared.c $(Q3LCC)
	$(DO_GAME_Q3LCC)

# Added
$(B)/$(BASEGAME)/ui/q_math.o: $(CMDIR)/q_math.c
	$(DO_UI_CC)
$(B)/$(BASEGAME)/ui/q_shared.o: $(CMDIR)/q_shared.c
	$(DO_UI_CC)
$(B)/$(BASEGAME)/ui/q_math.asm: $(CMDIR)/q_math.c $(Q3LCC)
	$(DO_UI_Q3LCC)
$(B)/$(BASEGAME)/ui/q_shared.asm: $(CMDIR)/q_shared.c $(Q3LCC)
	$(DO_UI_Q3LCC)

#############################################################################
# MISC
#############################################################################

OBJ = $(Q3OBJ) $(Q3ROBJ) $(Q3R2OBJ) $(Q3DOBJ) $(JPGOBJ) $(FTOBJ) \
  $(Q3GOBJ) $(Q3CGOBJ) $(Q3UIOBJ) \
  $(Q3GVMOBJ) $(Q3CGVMOBJ) $(Q3UIVMOBJ) 
TOOLSOBJ = $(LBURGOBJ) $(Q3CPPOBJ) $(Q3RCCOBJ) $(Q3LCCOBJ) $(Q3ASMOBJ)
STRINGOBJ = $(Q3R2STRINGOBJ)


copyfiles: release
	@if [ ! -d $(COPYDIR)/$(BASEGAME) ]; then echo "You need to set COPYDIR to where your RTCW data is!"; fi
ifneq ($(BUILD_GAME_SO),0)
  ifneq ($(BUILD_BASEGAME),0)
	-$(MKDIR) -m 0755 $(COPYDIR)/$(BASEGAME)
  endif
endif

ifneq ($(BUILD_CLIENT),0)
	$(INSTALL) $(STRIP_FLAG) -m 0755 $(BR)/$(CLIENTBIN)$(FULLBINEXT) $(COPYBINDIR)/$(CLIENTBIN)$(FULLBINEXT)
  ifneq ($(USE_RENDERER_DLOPEN),0)
	$(INSTALL) $(STRIP_FLAG) -m 0755 $(BR)/renderer_sp_opengl1_$(SHLIBNAME) $(COPYBINDIR)/renderer_sp_opengl1_$(SHLIBNAME)
  else
  endif
endif

ifneq ($(BUILD_SERVER),0)
	@if [ -f $(BR)/$(SERVERBIN)$(FULLBINEXT) ]; then \
		$(INSTALL) $(STRIP_FLAG) -m 0755 $(BR)/$(SERVERBIN)$(FULLBINEXT) $(COPYBINDIR)/$(SERVERBIN)$(FULLBINEXT); \
	fi
endif

ifneq ($(BUILD_GAME_SO),0)
  ifneq ($(BUILD_BASEGAME),0)
	$(INSTALL) $(STRIP_FLAG) -m 0755 $(BR)/$(BASEGAME)/cgame.sp.$(SHLIBNAME) \
                                        $(COPYDIR)/$(BASEGAME)/.
	$(INSTALL) $(STRIP_FLAG) -m 0755 $(BR)/$(BASEGAME)/qagame.sp.$(SHLIBNAME) \
                                        $(COPYDIR)/$(BASEGAME)/.
	$(INSTALL) $(STRIP_FLAG) -m 0755 $(BR)/$(BASEGAME)/ui.sp.$(SHLIBNAME) \
                                        $(COPYDIR)/$(BASEGAME)/.
  endif
endif

clean: clean-debug clean-release

clean-debug:
	@$(MAKE) clean2 B=$(BD)

clean-release:
	@$(MAKE) clean2 B=$(BR)

clean2:
	@echo "CLEAN $(B)"
	@rm -f $(OBJ)
	@rm -f $(OBJ_D_FILES)
	@rm -f $(STRINGOBJ)
	@rm -f $(TARGETS)

toolsclean: toolsclean-debug toolsclean-release

toolsclean-debug:
	@$(MAKE) toolsclean2 B=$(BD)

toolsclean-release:
	@$(MAKE) toolsclean2 B=$(BR)

toolsclean2:
	@echo "TOOLS_CLEAN $(B)"
	@rm -f $(TOOLSOBJ)
	@rm -f $(TOOLSOBJ_D_FILES)
	@rm -f $(LBURG) $(DAGCHECK_C) $(Q3RCC) $(Q3CPP) $(Q3LCC) $(Q3ASM) $(STRINGIFY)

distclean: clean toolsclean
	@rm -rf $(BUILD_DIR)

dist:
	git archive --format zip --output $(CLIENTBIN)-$(VERSION).zip HEAD

#############################################################################
# DEPENDENCIES
#############################################################################

ifneq ($(B),)
  OBJ_D_FILES=$(filter %.d,$(OBJ:%.o=%.d))
  TOOLSOBJ_D_FILES=$(filter %.d,$(TOOLSOBJ:%.o=%.d))
  -include $(OBJ_D_FILES) $(TOOLSOBJ_D_FILES)
endif

.PHONY: all clean clean2 clean-debug clean-release copyfiles \
	debug default dist distclean makedirs \
	release targets \
	toolsclean toolsclean2 toolsclean-debug toolsclean-release \
	$(OBJ_D_FILES) $(TOOLSOBJ_D_FILES)

# If the target name contains "clean", don't do a parallel build
ifneq ($(findstring clean, $(MAKECMDGOALS)),)
.NOTPARALLEL:
endif
