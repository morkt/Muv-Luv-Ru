CXX = g++
MSVC = cl //nologo
BOOSTDIR = C:/usr/boost_1_55_0
ROOTDIR = ../..

INCLUDES = -I$(BOOSTDIR) -I$(ROOTDIR)/sys++ -I$(ROOTDIR)/extlib
DEFS = -D_WIN32_WINNT=0x0501 -D_WIN32_IE=0x0500 -DNOMINMAX -DNDEBUG
UNICODE_DEFS = -DUNICODE -D_UNICODE
CXXDEFS = -Wno-unused-local-typedefs
CXXFLAGS = -Wall -W -pipe -std=c++11 -march=i686 -O2 $(DEFS) $(CXXDEFS) $(INCLUDES) -IC:/usr/include
LDFLAGS = -static-libgcc -static-libstdc++
LDLIBS =
MSVCFLAGS = //EHsc //O2 //GS- //wd4482 $(DEFS) $(UNICODE_DEFS) -D_USING_V110_SDK71_ $(INCLUDES) -IC:/usr/VC/include
MSVCLDFLAGS = //SUBSYSTEM:WINDOWS,5.01
ZLIB = C:/usr/VC/lib/zlibstat.lib
PNGLIB = C:/usr/VC/lib/libpng.lib
MSVCLIBS = user32.lib Comdlg32.lib Shell32.lib Shlwapi.lib Ole32.lib Gdi32.lib $(ROOTDIR)/sys++/sys++.lib $(ZLIB) $(PNGLIB)
OBJECTS =  xami.obj xami-config.obj xami-progress.obj xami-extract.obj xami-create.obj \
	   xami-popup.obj logcontrol.obj ami-reader.obj xami-util.obj mltcomp.obj mltwrite.obj \
	   fileutil.obj png-convert.obj logcontrol.obj stringutil.obj
RESOURCES = xami-main.rc
scrcomp: UNICODE_DEFS=

.SUFFIXES: .o .obj .cc .rc .res .exe

.PHONY: tags

all: xami

xami: $(OBJECTS) $(RESOURCES:.rc=.res)
	$(MSVC) $(MSVCFLAGS) $^ //Fe$@.exe //link $(MSVCLDFLAGS) $(MSVCLIBS)

scrcomp: scrcomp.obj mltcomp.obj stringutil.obj
	$(MSVC) $^ //Fe$@.exe

#xami: $(OBJECTS:.obj=.o) $(RESOURCES:.rc=.o)
#	$(CXX) -s -mwindows $(LDFLAGS) $^ -o $@

xami.obj: xami.cc xami.hpp xami-config.hpp logcontrol.hpp windres.h
xami-config.obj: xami-config.cc xami-config.hpp xami-util.hpp
xami-extract.obj: xami-extract.cc xami.hpp xami-progress.hpp ami-archive.hpp fileutil.hpp
xami-create.obj: xami-create.cc xami.hpp xami-progress.hpp ami-archive.hpp mltcomp.hpp fileutil.hpp
xami-progress.obj: xami-progress.cc xami-progress.hpp xami.hpp windres.h
ami-reader.obj: ami-reader.cc ami-archive.hpp xami-util.hpp
mltcomp.obj: mltcomp.cc mltcomp.hpp

tags:
	ctags *.cc *.tcc *.hpp *.h

.cc.obj:
	$(MSVC) $(MSVCFLAGS) -c $<

.rc.o:
	windres -c 65001 $< -o $@

.rc.res:
	rc //nologo //c65001 $<

clean:
	rm -f *.o *.obj *.res xami.exe
