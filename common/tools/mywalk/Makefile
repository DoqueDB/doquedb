OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\MyWalk.exe" \
	"$(OUTDIR)\MyWalkClient.exe"


CLEAN :
	-@erase "$(INTDIR)\MyWalk.obj"
	-@erase "$(INTDIR)\MyWalkClient.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc70.idb"
	-@erase "$(OUTDIR)\MyWalk.exe"
	-@erase "$(OUTDIR)\MyWalkClient.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

LINK32=link.exe
STDLIB = wsock32.lib advapi32.lib
LINK32_FLAGS=$(STDLIB) /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\MyWalk.pdb" /machine:I386 /out:"$(OUTDIR)\MyWalk.exe" 
LINK32_OBJS= \
	"$(INTDIR)\MyWalk.obj" \
	"$(INTDIR)\StdAfx.obj"

"$(OUTDIR)\MyWalk.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

LINK32_FLAGS=$(STDLIB) /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\MyWalkClient.pdb" /machine:I386 /out:"$(OUTDIR)\MyWalkClient.exe" 
LINK32_OBJS= \
	"$(INTDIR)\MyWalkClient.obj" \
	"$(INTDIR)\StdAfx.obj"

"$(OUTDIR)\MyWalkClient.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE=.\MyWalk.cpp

"$(INTDIR)\MyWalk.obj" : $(SOURCE) "$(INTDIR)"

SOURCE=.\MyWalkClient.cpp

"$(INTDIR)\MyWalkClient.obj" : $(SOURCE) "$(INTDIR)"

SOURCE=.\StdAfx.cpp
CPP_SWITCHES=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /c 

"$(INTDIR)\StdAfx.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<
