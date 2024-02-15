/*
 * Makefile.c --- Kernel/Execution/Iterator/c
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
SUBMODULE = Iterator
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
LOCAL_EXPORT_HDRS = \
	$(HDRDIR)/Array.h			\
	$(HDRDIR)/Base.h			\
	$(HDRDIR)/BitSet.h			\
	$(HDRDIR)/CascadeInput.h	\
	$(HDRDIR)/Declaration.h		\
	$(HDRDIR)/EmptyNull.h		\
	$(HDRDIR)/Exists.h			\
	$(HDRDIR)/File.h			\
	$(HDRDIR)/ForwordingIterator.h	\	
	$(HDRDIR)/Filter.h			\
	$(HDRDIR)/Input.h			\
	$(HDRDIR)/Loop.h			\
	$(HDRDIR)/MergeSort.h		\
	$(HDRDIR)/Module.h			\
	$(HDRDIR)/NestedLoop.h		\
	$(HDRDIR)/Tuples.h			\
	$(HDRDIR)/UnionDistinct.h

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(MESSAGE_HDRS)

HDRS = \
	$(TOP_EXPORT_HDRS)			\
	$(LOCAL_EXPORT_HDRS)  		\
	$(HDRDIR)/Class.h			\
	$(HDRDIR)/Nadic.h			\
	$(HDRDIR)/OperandElement.h

MESSAGE_OBJS =

OBJS = \
	$(MESSAGE_OBJS)			\
	Array$O					\
	Base$O					\
	BitSet$O				\
	CascadeInput$O			\
	Class$O					\
	EmptyNull$O				\
	Exists$O				\
	File$O					\
	Filter$O				\
	Input$O					\
	Loop$O					\
	MergeSort$O				\
	NestedLoop$O			\
	OperandElement$O		\
	Tuples$O				\
	UnionDistinct$O

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

ObjectListTargetN($(TARGET), $(LOCAL_INSTALL_DIR))
AddObjectListTarget($(TARGET), $(OBJS))

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
