/*
 * Makefile.c --- Kernel/DPlan/c
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

SUBDIRS =		\
	Candidate	\
	Relation	\
	Predicate	\
	Scalar

/****************************************/
/* following variables MUST be defined  */

MODULE = DPlan
TARGET_BASE = SyDPlan
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
	Candidate.ol	 \
	Predicate.ol	\
	Relation.ol		 \
	Scalar.ol 

/* headers created for message */
MESSAGE_HDRS =

/* headers installed in Kernel */
LOCAL_EXPORT_HDRS =			\
	$(HDRDIR)/Declaration.h \
	$(HDRDIR)/Manager.h		\
	$(HDRDIR)/Module.h

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(MESSAGE_HDRS)

HDRS = \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS)

MESSAGE_OBJS =

OBJS =					\
	Manager$O

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

ALLTARGETS = \
	objlist-r		  \
	$(MESSAGE_TARGET) \
	$(TARGET)

/*
 * all
 */
AllTarget($(ALLTARGETS))

ObjectListTargetSubDir($(TARGET), $(OBJS), $(SUBDIR_OBJECT_LIST), $(TOP_INSTALL_DIR))

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
