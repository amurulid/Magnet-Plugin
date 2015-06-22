#-
# ==========================================================================
# Copyright (c) 2011 Autodesk, Inc.
# All rights reserved.
# 
# These coded instructions, statements, and computer programs contain
# unpublished proprietary information written by Autodesk, Inc., and are
# protected by Federal copyright law. They may not be disclosed to third
# parties or copied or duplicated in any form, in whole or in part, without
# the prior written consent of Autodesk, Inc.
# ==========================================================================
#+

ifndef INCL_BUILDRULES

#
# Include platform specific build settings
#

TOP := ..
include $(TOP)/buildrules

#
# Always build the local plug-in when make is invoked from the
# directory.
#
all : plugins

endif

#
# Variable definitions
#

SRCDIR := $(TOP)/finalproject
DSTDIR := $(TOP)/finalproject

finalproject_SOURCES  := $(TOP)/finalproject/finalproject.cpp
finalproject_OBJECTS  := $(TOP)/finalproject/finalproject.o
finalproject_PLUGIN   := $(DSTDIR)/finalproject.$(EXT)
finalproject_MAKEFILE := $(DSTDIR)/Makefile

#
# Include the optional per-plugin Makefile.inc
#
#    The file can contain macro definitions such as:
#       {pluginName}_EXTRA_CFLAGS
#       {pluginName}_EXTRA_C++FLAGS
#       {pluginName}_EXTRA_INCLUDES
#       {pluginName}_EXTRA_LIBS
-include $(SRCDIR)/Makefile.inc


#
# Set target specific flags.
#

$(finalproject_OBJECTS): CFLAGS   := $(CFLAGS)   $(finalproject_EXTRA_CFLAGS) -no-ipo -no-ip -restrict -openmp
$(finalproject_OBJECTS): C++FLAGS := $(C++FLAGS) $(finalproject_EXTRA_C++FLAGS)
$(finalproject_OBJECTS): INCLUDES := $(INCLUDES) $(finalproject_EXTRA_INCLUDES)

depend_finalproject:     INCLUDES := $(INCLUDES) $(finalproject_EXTRA_INCLUDES)

$(finalproject_PLUGIN):  LFLAGS   := $(LFLAGS) $(finalproject_EXTRA_LFLAGS)
$(finalproject_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lFoundation $(finalproject_EXTRA_LIBS)

#
# Rules definitions
#

.PHONY: depend_finalproject clean_finalproject Clean_finalproject

$(finalproject_PLUGIN): $(finalproject_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_finalproject :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(finalproject_SOURCES)

clean_finalproject:
	-rm -f $(finalproject_OBJECTS)

Clean_finalproject:
	-rm -f $(finalproject_MAKEFILE).bak $(finalproject_OBJECTS) $(finalproject_PLUGIN)

plugins: $(finalproject_PLUGIN)
depend:	 depend_finalproject
clean:	 clean_finalproject
Clean:	 Clean_finalproject

# DO NOT DELETE

../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/maya/MIOStream.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/maya/MIOStreamFwd.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/maya/MPxDeformerNode.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/maya/MItGeometry.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/maya/MFnPlugin.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/maya/MDataBlock.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/maya/MDataHandle.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/maya/MPoint.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/maya/MTimer.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/maya/MFnMesh.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/maya/MPointArray.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/maya/MFnTypedAttribute.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/maya/MFnMeshData.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/maya/MMeshIntersector.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/maya/MThreadUtils.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/tbb/parallel_for.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/tbb/task.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/tbb/tbb_stddef.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/tbb/tbb_config.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/tbb/tbb_machine.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/tbb/partitioner.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/tbb/aligned_space.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/tbb/atomic.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/tbb/blocked_range.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/tbb/tbb_exception.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/tbb/tbb_allocator.h
../finalproject/finalproject.o: /usr/autodesk/maya2015-x64/include/tbb/blocked_range.h
