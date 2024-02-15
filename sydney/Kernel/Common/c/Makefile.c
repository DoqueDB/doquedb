/*
 * Makefile.c --- Kernel/Common/c
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
CLIENTSUBDIRS =

/****************************************/
/* following variables MUST be defined  */

MODULE = Common
TARGET_BASE = SyCommon
SRCDIR = ..
HDRDIR = ../$(MODULE)
MESSAGE_DEFINITION =
MESSAGE_TARGET =

LOCAL_EXPORT_HDRDIR = ../../include/$(MODULE)
TOP_EXPORT_HDRDIR = ../../../include/$(MODULE)
TOP_INSTALL_DIR = ../../../c.CONF
PACKAGE_HDRDIR = $(TOP_INSTALL_DIR)/$(PACKAGE_NAME)/include/$(MODULE)

/* above variables MUST be defined      */
/****************************************/

/* headers created for message */
MESSAGE_HDRS =

/* headers installed in tape image */
PACKAGE_HDRS = \
	$(HDRDIR)/ArrayData.h \
	$(HDRDIR)/BinaryData.h \
	$(HDRDIR)/ClassID.h \
	$(HDRDIR)/Common.h \
	$(HDRDIR)/Collation.h \
	$(HDRDIR)/ColumnMetaData.h \
	$(HDRDIR)/CompressedBinaryData.h \
	$(HDRDIR)/CompressedData.h \
	$(HDRDIR)/CompressedStringData.h \
	$(HDRDIR)/Data.h \
	$(HDRDIR)/DataArrayData.h \
	$(HDRDIR)/DataInstance.h \
	$(HDRDIR)/DataOperation.h \
	$(HDRDIR)/DataType.h \
	$(HDRDIR)/DateData.h \
	$(HDRDIR)/DateTimeData.h \
	$(HDRDIR)/DecimalData.h \
	$(HDRDIR)/DefaultData.h \
	$(HDRDIR)/DoubleData.h \
	$(HDRDIR)/ExceptionObject.h \
	$(HDRDIR)/ExecutableObject.h \
	$(HDRDIR)/Externalizable.h \
	$(HDRDIR)/FloatData.h \
	$(HDRDIR)/Integer64Data.h \
	$(HDRDIR)/IntegerArrayData.h \
	$(HDRDIR)/IntegerData.h \
	$(HDRDIR)/LanguageData.h \
	$(HDRDIR)/Module.h \
	$(HDRDIR)/NullData.h \
	$(HDRDIR)/Object.h \
	$(HDRDIR)/ObjectIDData.h \
	$(HDRDIR)/ObjectPointer.h \
	$(HDRDIR)/ResultSetMetaData.h \
	$(HDRDIR)/SafeExecutableObject.h \
	$(HDRDIR)/ScalarData.h \
	$(HDRDIR)/StringArrayData.h \
	$(HDRDIR)/StringData.h \
	$(HDRDIR)/SQLData.h \
	$(HDRDIR)/Thread.h \
	$(HDRDIR)/UnicodeString.h \
	$(HDRDIR)/UnsignedInteger64Data.h \
	$(HDRDIR)/UnsignedIntegerArrayData.h \
	$(HDRDIR)/UnsignedIntegerData.h \
	$(HDRDIR)/WordData.h

/* headers installed in Kernel */
LOCAL_EXPORT_HDRS =

/* headers installed in TOP */
TOP_EXPORT_HDRS1 = \
	$(HDRDIR)/Assert.h \
	$(HDRDIR)/AutoArrayPointer.h \
	$(HDRDIR)/AutoCaller.h \
	$(HDRDIR)/BasicString.h \
	$(HDRDIR)/BitSet.h \
	$(HDRDIR)/CRC.h \
	$(HDRDIR)/Configuration.h \
	$(HDRDIR)/DoubleLinkedList.h \
	$(HDRDIR)/ErrorLevel.h \
	$(HDRDIR)/ErrorMessage.h \
	$(HDRDIR)/ErrorMessageManager.h \
	$(HDRDIR)/ExceptionMessage.h \
	$(HDRDIR)/Functional.h \
	$(HDRDIR)/Functional_Accumulator.h \
	$(HDRDIR)/Functional_Accumulator0.h \
	$(HDRDIR)/Functional_Accumulator1.h \
	$(HDRDIR)/Functional_Accumulator2.h \
	$(HDRDIR)/Functional_Accumulator3.h \
	$(HDRDIR)/Functional_Bind.h \
	$(HDRDIR)/Functional_Bind0.h \
	$(HDRDIR)/Functional_Bind1.h \
	$(HDRDIR)/Functional_Bind2.h \
	$(HDRDIR)/Functional_Bind3.h \
	$(HDRDIR)/Functional_CallAccumulator0.h \
	$(HDRDIR)/Functional_CallAccumulator1.h \
	$(HDRDIR)/Functional_CallAccumulator2.h \
	$(HDRDIR)/Functional_CallAccumulator3.h \
	$(HDRDIR)/Functional_CallFun.h \
	$(HDRDIR)/Functional_CallFun0.h \
	$(HDRDIR)/Functional_CallFun1.h \
	$(HDRDIR)/Functional_CallFun2.h \
	$(HDRDIR)/Functional_CallFun3.h \
	$(HDRDIR)/Functional_CallFun4.h \
	$(HDRDIR)/Functional_Comparator.h \
	$(HDRDIR)/Functional_Comparator0.h \
	$(HDRDIR)/Functional_Comparator1.h \
	$(HDRDIR)/Functional_MemFun.h \
	$(HDRDIR)/Functional_MemFun0.h \
	$(HDRDIR)/Functional_MemFun1.h \
	$(HDRDIR)/Functional_MemFun2.h \
	$(HDRDIR)/Functional_MemFun3.h \
	$(HDRDIR)/Functional_MemFun4.h \
	$(HDRDIR)/HashTable.h \
	$(HDRDIR)/Hasher.h \
	$(HDRDIR)/InputArchive.h \
	$(HDRDIR)/Internal.h \
	$(HDRDIR)/LargeVector.h \
	$(HDRDIR)/MD5.h \
	$(HDRDIR)/Manager.h \
	$(HDRDIR)/Message.h \
	$(HDRDIR)/MessageStream.h \
	$(HDRDIR)/MessageStreamBuffer.h \
	$(HDRDIR)/OutputArchive.h \
	$(HDRDIR)/Parameter.h \
	$(HDRDIR)/Privilege.h \
	$(HDRDIR)/Request.h \
	$(HDRDIR)/Status.h \
	$(HDRDIR)/SystemParameter.h \
	$(HDRDIR)/VectorMap.h \
	$(MESSAGE_HDRS)
	
TOP_EXPORT_HDRS = \
	$(PACKAGE_HDRS) \
	$(TOP_EXPORT_HDRS1)

HDRS = \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS) \
	$(HDRDIR)/SinglyLinkedList.h \
	$(HDRDIR)/StringTest.h \
	$(HDRDIR)/UnicodeCharJapaneseComparisonTable.h

MESSAGE_OBJS =

OBJS = \
	$(MESSAGE_OBJS) \
	ArrayData$O \
	BinaryData$O \
	BitSet$O \
	CRC$O \
	ClassID$O \
	ColumnMetaData$O \
	Common$O \
	CompressedBinaryData$O \
	CompressedData$O \
	CompressedStringData$O \
	Data$O \
	DataArrayData$O \
	DataInstance$O \
	DateData$O \
	DateTimeData$O \
	DefaultData$O \
	DoubleData$O \
	DecimalData$O \
	ErrorLevel$O \
	ErrorMessage$O \
	ErrorMessageManager$O \
	ExceptionObject$O \
	ExecutableObject$O \
	Externalizable$O \
	FloatData$O \
	Hasher$O \
	InputArchive$O \
	Integer64Data$O \
	IntegerArrayData$O \
	IntegerData$O \
	LanguageData$O \
	MD5$O \
	Manager$O \
	Message$O \
	MessageStream$O \
	MessageStreamBuffer$O \
	NullData$O \
	Object$O \
	ObjectIDData$O \
	OutputArchive$O \
	Parameter$O \
	Request$O \
	ResultSetMetaData$O \
	SafeExecutableObject$O \
	ScalarData$O \
	Status$O \
	StringArrayData$O \
	StringData$O \
	SQLData$O \
	SystemParameter$O \
	Thread$O \
	UnicodeString$O \
	UnsignedInteger64Data$O \
	UnsignedIntegerArrayData$O \
	UnsignedIntegerData$O \
	WordData$O

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
	$(MESSAGE_TARGET) \
	$(TARGET)

/*
 * all
 */
AllTarget($(ALLTARGETS))
clientall:: all

ObjectListTarget($(TARGET), $(OBJS), $(TOP_INSTALL_DIR))
clientobjlist:: objlist

/*
 * message
 */
/* MessageTarget($(MESSAGE_TARGET), $(MESSAGE_DEFINITION), $(BUILD_JAR)) */

/*
 * install library and header
 */
InstallHeaderTarget2($(PACKAGE_HDRS), $(TOP_EXPORT_HDRS1), $(TOP_EXPORT_HDRDIR))
/* InstallHeaderTarget($(LOCAL_EXPORT_HDRS), $(LOCAL_EXPORT_HDRDIR)) */
clientinstallh:: installh

/*
 * install header for package
 */
TapeHeaderTarget($(PACKAGE_HDRS), $(PACKAGE_HDRDIR))

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
