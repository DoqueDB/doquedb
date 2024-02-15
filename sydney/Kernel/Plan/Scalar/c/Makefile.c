/*
 * Makefile.c --- Kernel/Plan/Scalar/c
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
SUBMODULE = Scalar
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
	$(HDRDIR)/Argument.h			\
	$(HDRDIR)/Aggregation.h			\	
	$(HDRDIR)/Base.h				\
	$(HDRDIR)/Case.h				\
	$(HDRDIR)/Choice.h				\
	$(HDRDIR)/DataType.h			\
	$(HDRDIR)/Declaration.h			\
	$(HDRDIR)/Field.h				\
	$(HDRDIR)/Function.h			\
	$(HDRDIR)/Generator.h			\
	$(HDRDIR)/Invoke.h				\
	$(HDRDIR)/Module.h				\
	$(HDRDIR)/Spatial.h				\
	$(HDRDIR)/Subquery.h			\
	$(HDRDIR)/Value.h

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(MESSAGE_HDRS)

HDRS = \
	$(TOP_EXPORT_HDRS)					\
	$(LOCAL_EXPORT_HDRS)				\
	$(HDRDIR)/Arithmetic.h				\
	$(HDRDIR)/Cast.h					\
	$(HDRDIR)/CheckedField.h			\
	$(HDRDIR)/FullText.h				\
	$(HDRDIR)/Locator.h					\
	$(HDRDIR)/Operation.h				\
	$(HDRDIR)/UpdateField.h				\
	$(HDRDIR)/Impl/AggregationImpl.h	\
	$(HDRDIR)/Impl/ArithmeticImpl.h		\
	$(HDRDIR)/Impl/ArrayImpl.h			\
	$(HDRDIR)/Impl/CaseImpl.h			\
	$(HDRDIR)/Impl/CastImpl.h			\
	$(HDRDIR)/Impl/ChoiceImpl.h			\
	$(HDRDIR)/Impl/FieldImpl.h			\
	$(HDRDIR)/Impl/FullTextImpl.h		\
	$(HDRDIR)/Impl/FunctionImpl.h		\
	$(HDRDIR)/Impl/GeneratorImpl.h		\
	$(HDRDIR)/Impl/InvokeImpl.h			\
	$(HDRDIR)/Impl/KwicImpl.h			\
	$(HDRDIR)/Impl/LengthImpl.h			\
	$(HDRDIR)/Impl/LocatorImpl.h		\
	$(HDRDIR)/Impl/OperationImpl.h		\
	$(HDRDIR)/Impl/PatternImpl.h		\	
	$(HDRDIR)/Impl/SpatialImpl.h		\
	$(HDRDIR)/Impl/StringImpl.h			\
	$(HDRDIR)/Impl/SubqueryImpl.h		\
	$(HDRDIR)/Impl/UpdateFieldImpl.h	\
	$(HDRDIR)/Impl/ValueImpl.h

MESSAGE_OBJS =

OBJS = \
	$(MESSAGE_OBJS)				\
	Aggregation$O				\
	Arithmetic$O				\
	Base$O						\
	Case$O						\
	Cast$O						\
	CheckedField$O				\
	Choice$O					\
	DataType$O					\
	Field$O						\
	FullText$O					\
	Function$O					\
	Generator$O					\
	Invoke$O					\
	Locator$O					\
	Operation$O					\
	Spatial$O					\
	Subquery$O					\
	UpdateField$O				\
	Value$O						\
	AggregationImpl$O			\
	ArithmeticImpl$O			\
	ArrayImpl$O					\
	CaseImpl$O					\
	CastImpl$O					\
	ChoiceImpl$O				\
	FieldImpl$O					\
	FullTextImpl$O				\
	GeneratorImpl$O				\
	InvokeImpl$O				\
	KwicImpl$O					\
	LengthImpl$O				\
	LocatorImpl$O				\
	OperationImpl$O				\
	PatternImpl$O				\	
	SpatialImpl$O				\
	SubqueryImpl$O				\
	UpdateFieldImpl$O			\
	ValueImpl$O

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
