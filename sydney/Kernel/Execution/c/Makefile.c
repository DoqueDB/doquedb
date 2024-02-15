/*
 * Makefile.c --- Kernel/Execution/c
 * 
 * Copyright (c) 1999, 2023 Ricoh Company, Ltd.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

SUBDIRS = \
	Action     \
	Collection \
	Control	   \
	Function   \
	Interface  \
	Iterator   \
	Operator   \
	Parallel   \
	Predicate  \
	Utility	   \
/*	V1Impl */  \
	V2Impl

/****************************************/
/* following variables MUST be defined  */

MODULE = Execution
TARGET_BASE = SyExec
SRCDIR = ..
HDRDIR = ../$(MODULE)
MESSAGE_DEFINITION =
MESSAGE_TARGET =

LOCAL_EXPORT_HDRDIR = ../../include/$(MODULE)
TOP_EXPORT_HDRDIR = ../../../include/$(MODULE)
TOP_INSTALL_DIR = ../../../c.CONF

/* above variables MUST be defined      */
/****************************************/

/* Object list from subdirs */
SUBDIR_OBJECT_LIST = \
	Action.ol		 \
	Collection.ol	 \
	Control.ol		 \
	Function.ol		 \
	Interface.ol	 \
	Iterator.ol		 \
	Operator.ol		 \
	Parallel.ol		 \
	Predicate.ol	 \
	Utility.ol		 \
/*	V1Impl.ol	 */  \
	V2Impl.ol

/* headers created for message */
MESSAGE_HDRS =

/* headers installed in Kernel */
LOCAL_EXPORT_HDRS = \
	$(HDRDIR)/Declaration.h		\
	$(HDRDIR)/Executor.h		\
	$(HDRDIR)/Externalizable.h	\
	$(HDRDIR)/Manager.h			\
	$(HDRDIR)/Module.h			\
	$(HDRDIR)/Program.h

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(MESSAGE_HDRS)

HDRS = \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS)

MESSAGE_OBJS =

OBJS = \
	$(MESSAGE_OBJS)			\
	Execution$O				\
	Executor$O				\
	Externalizable$O		\
	Manager$O				\
	Program$O

#ifdef SYD_DLL
EXPORTFLAGS = \
	-DSYD_KERNEL_EXPORT_FUNCTION
#endif
EXTRACFLAGS = \
	$(EXPORTFLAGS)
EXTRALDFLAGS =
EXTRAPURIFYFLAGS =
EXTRAQUANTIFYFLAGS =
EXTRAPURECOVFLAGS =
EXTRALOCALCFLAGS =

/********************************************************/

TARGET = $(MODULE).ol

ALLTARGETS =		  \
	objlist-r		  \
	$(MESSAGE_TARGET) \
	$(TARGET)

/*
 * all
 */
AllTarget($(ALLTARGETS))

ObjectListTargetN($(TARGET), $(TOP_INSTALL_DIR))
AddObjectListTarget($(TARGET), $(OBJS))
AddObjectListTargetSubDir($(TARGET), $(SUBDIR_OBJECT_LIST))
     
/*
 * message
 */
/* MessageTarget($(MESSAGE_TARGET), $(MESSAGE_DEFINITION), $(BUILD_JAR)) */

/*
 * install library and header
 */
/* InstallHeaderTarget($(TOP_EXPORT_HDRS), $(TOP_EXPORT_HDRDIR)) */
InstallHeaderTarget($(LOCAL_EXPORT_HDRS), $(LOCAL_EXPORT_HDRDIR))

/*
 * clean
 */
CleanTarget($(MESSAGE_TARGET) $(TARGET))
CleanTarget($(OBJS))

#include "Makefile.h"

/*
  Copyright (c) 1999, 2023 Ricoh Company, Ltd.
  All rights reserved.
*/
