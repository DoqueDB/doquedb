/*
 * Makefile.c --- Kernel/Execution/Operator/c
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

SUBDIRS =

/****************************************/
/* following variables MUST be defined  */

MODULE = Execution
SUBMODULE = Operator
SRCDIR = ..
HDRDIR = ../../$(MODULE)/$(SUBMODULE)
MESSAGE_DEFINITION =
MESSAGE_TARGET =

LOCAL_EXPORT_HDRDIR = ../../../include/$(MODULE)/$(SUBMODULE)
TOP_EXPORT_HDRDIR = ../../../../include/$(MODULE)/$(SUBMODULE)
TOP_INSTALL_DIR = ../../../../c.CONF
LOCAL_INSTALL_DIR = ../../c.CONF

/* above variables MUST be defined      */
/****************************************/

/* headers created for message */
MESSAGE_HDRS =

/* headers installed in Kernel */
LOCAL_EXPORT_HDRS =				\
	$(HDRDIR)/Assign.h			\
	$(HDRDIR)/BitSet.h			\
	$(HDRDIR)/CheckCancel.h		\
	$(HDRDIR)/Clear.h			\	
	$(HDRDIR)/Declaration.h		\
	$(HDRDIR)/FileFetch.h		\
	$(HDRDIR)/FileGetProperty.h	\
	$(HDRDIR)/FileOperation.h	\
	$(HDRDIR)/Generator.h		\
	$(HDRDIR)/Iterate.h			\
	$(HDRDIR)/Limit.h			\
	$(HDRDIR)/Locator.h			\
	$(HDRDIR)/Locker.h			\
	$(HDRDIR)/Logger.h			\
	$(HDRDIR)/Output.h			\
	$(HDRDIR)/Module.h			\
	$(HDRDIR)/SetNull.h			\
	$(HDRDIR)/SystemColumn.h	\
	$(HDRDIR)/Thread.h			\
	$(HDRDIR)/Throw.h			\
	$(HDRDIR)/Transaction.h		\
	$(HDRDIR)/UndoLog.h

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(MESSAGE_HDRS)

HDRS = \
	$(TOP_EXPORT_HDRS)			\
	$(LOCAL_EXPORT_HDRS)  		\
	$(HDRDIR)/Class.h

MESSAGE_OBJS =

OBJS = \
	$(MESSAGE_OBJS)			 \
	Assign$O				 \
	BitSet$O				 \
	CheckCancel$O			 \
	Class$O					 \
	Clear$O					 \	
	FileFetch$O				 \
	FileGetProperty$O		 \
	FileOperation$O			 \
	Generator$O				 \
	Iterate$O				 \
	Limit$O					 \
	Locator$O				 \
	Locker$O				 \
	Logger$O				 \
	Output$O				 \
	SetNull$O				 \
	SystemColumn$O			 \
	Thread$O				 \
	Throw$O					 \
	Transaction$O			 \
	UndoLog$O

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
EXTRALOCALCFLAGS = \
	-I../..

/********************************************************/

TARGET = $(SUBMODULE).ol

ALLTARGETS = \
	$(MESSAGE_TARGET) \
	$(TARGET)

/*
 * all
 */
AllTarget($(ALLTARGETS))

ObjectListTarget($(TARGET), $(OBJS), $(LOCAL_INSTALL_DIR))

InstallHeaderTarget($(LOCAL_EXPORT_HDRS), $(LOCAL_EXPORT_HDRDIR))

/*
 * clean
 */
CleanTarget($(ALLTARGETS))
CleanTarget($(OBJS))

#include "Makefile.h"

/*
  Copyright (c) 1999, 2023 Ricoh Company, Ltd.
  All rights reserved.
*/
