/*
 * Makefile.c --- Kernel/Plan/Predicate/c
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

MODULE = Plan
SUBMODULE = Predicate
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
	$(HDRDIR)/Between.h			\
	$(HDRDIR)/Combinator.h		\
	$(HDRDIR)/Comparison.h		\
	$(HDRDIR)/Contains.h		\
	$(HDRDIR)/Declaration.h		\
	$(HDRDIR)/Exists.h			\
	$(HDRDIR)/In.h				\
	$(HDRDIR)/IsSubstringOf.h	\
	$(HDRDIR)/Like.h			\
	$(HDRDIR)/Module.h			\
	$(HDRDIR)/NeighborIn.h		\
	$(HDRDIR)/Similar.h			\
	$(HDRDIR)/Impl/Base.h	

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(MESSAGE_HDRS)

IMPL_HDRS = \
	$(HDRDIR)/Impl/BetweenImpl.h		\
	$(HDRDIR)/Impl/CheckUnknownImpl.h	\
	$(HDRDIR)/Impl/CheckedImpl.h		\
	$(HDRDIR)/Impl/ChosenImpl.h			\
	$(HDRDIR)/Impl/CombinatorImpl.h		\
	$(HDRDIR)/Impl/ComparisonImpl.h		\
	$(HDRDIR)/Impl/ContainsImpl.h		\
	$(HDRDIR)/Impl/ExistsImpl.h			\
	$(HDRDIR)/Impl/FetchImpl.h			\
	$(HDRDIR)/Impl/InImpl.h				\
	$(HDRDIR)/Impl/IsSubstringOf.h		\
	$(HDRDIR)/Impl/LikeImpl.h			\
	$(HDRDIR)/Impl/NeighborInImpl.h		\
	$(HDRDIR)/Impl/SimilarImpl.h

HDRS =								\
	$(TOP_EXPORT_HDRS)				\
	$(LOCAL_EXPORT_HDRS)			\
	$(HDRDIR)/CheckUnknown.h		\
	$(HDRDIR)/CheckedInterface.h	\
	$(HDRDIR)/ChosenInterface.h		\
	$(IMPL_HDRS)

MESSAGE_OBJS =

IMPL_OBJS = \
	BetweenImpl$O			\
	CheckUnknownImpl$O		\
	CheckedImpl$O			\
	ChosenImpl$O			\
	CombinatorImpl$O		\
	ComparisonImpl$O		\
	ContainsImpl$O			\
	ExistsImpl$O			\
	FetchImpl$O				\
	InImpl$O				\
	IsSubstringOfImpl$O		\
	LikeImpl$O				\
	NeighborInImpl$O		\
	SimilarImpl$O

OBJS =						\
	$(MESSAGE_OBJS)			\
	Between$O				\
	CheckedInterface$O		\
	CheckUnknown$O			\
	ChosenInterface$O		\
	Combinator$O			\
	Comparison$O			\
	Contains$O				\
	Exists$O				\
	Fetch$O					\
	In$O					\
	IsSubstringOf$O			\
	Like$O					\
	NeighborIn$O			\
	Similar$O				\
	$(IMPL_OBJS)

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
