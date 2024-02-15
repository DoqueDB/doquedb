/*
 * Makefile.c --- Kernel/Execution/Function/c
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
SUBMODULE = Function
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
LOCAL_EXPORT_HDRS =					\
	$(HDRDIR)/Aggregation.h			\
	$(HDRDIR)/Cast.h				\
	$(HDRDIR)/Coalesce.h			\
	$(HDRDIR)/Declaration.h			\
	$(HDRDIR)/Factory.h				\
	$(HDRDIR)/Invoke.h				\
	$(HDRDIR)/Kwic.h				\
	$(HDRDIR)/Module.h

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(MESSAGE_HDRS)

HDRS = \
	$(TOP_EXPORT_HDRS)				\
	$(LOCAL_EXPORT_HDRS)			\
	$(HDRDIR)/Arithmetic.h			\
	$(HDRDIR)/Cardinality.h			\
	$(HDRDIR)/Case.h				\
	$(HDRDIR)/CharJoin.h			\
	$(HDRDIR)/Choice.h				\
	$(HDRDIR)/Class.h				\
	$(HDRDIR)/Concatenate.h			\
	$(HDRDIR)/Copy.h				\
	$(HDRDIR)/CurrentTimestamp.h	\
	$(HDRDIR)/ElementReference.h	\
	$(HDRDIR)/ExpandSynonym.h		\
	$(HDRDIR)/Length.h				\
	$(HDRDIR)/Normalize.h			\
	$(HDRDIR)/Overlay.h				\
	$(HDRDIR)/SubString.h

MESSAGE_OBJS =

OBJS = \
	$(MESSAGE_OBJS)		\
	Aggregation$O		\
	Arithmetic$O		\
	Cardinality$O		\
	Case$O				\
	Cast$O				\
	CharJoin$O			\
	Choice$O			\
	Class$O				\
	Coalesce$O			\
	Concatenate$O		\
	Copy$O				\
	CurrentTimestamp$O	\
	ElementReference$O	\
	ExpandSynonym$O		\
	Factory$O			\
	Invoke$O			\
	Kwic$O				\
	Length$O			\
	Normalize$O			\
	Overlay$O			\
	SubString$O

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
