/*
 * Makefile.c --- Driver/Bitmap/c
 * 
 * Copyright (c) 1997, 2023 Ricoh Company, Ltd.
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

MODULE = Bitmap
TARGET_BASE = SyDrvBmp
SRCDIR = ..
HDRDIR = ../$(MODULE)
MESSAGE_DEFINITION = $(SRCDIR)/MessageDefinition.xml
MESSAGE_TARGET = $(MESSAGE_DEFINITION)_

LOCAL_EXPORT_HDRDIR = ../../include/$(MODULE)
TOP_EXPORT_HDRDIR = ../../../include/$(MODULE)
TOP_INSTALL_DIR = ../../../c.CONF
VERINFODIR=..$(S)..$(S)..$(S)version$(S)c.CONF
VERINFO_INCL=..$(S)..$(S)..$(S)version$(S)include

/* above variables MUST be defined      */
/****************************************/

/* headers created for message */
MESSAGE_HDRS = \
	$(HDRDIR)/MessageAll_Class.h \
	$(HDRDIR)/MessageAll_Number.h \
	$(HDRDIR)/MessageArgument.h \
	$(HDRDIR)/MessageFormat_English.h \
	$(HDRDIR)/MessageFormat_Japanese.h \
	$(HDRDIR)/MessageNumber_DiscordDelegateKey.h \
	$(HDRDIR)/MessageNumber_DiscordBitNum.h \
	$(HDRDIR)/MessageNumber_IllegalBtreeValue.h \
	$(HDRDIR)/MessageNumber_IllegalKeyIndex.h \
	$(HDRDIR)/MessageNumber_IllegalLastRowID.h \
	$(HDRDIR)/MessageNumber_IllegalLeftLeafPageID.h \
	$(HDRDIR)/MessageNumber_IllegalRightLeafPageID.h \
	$(HDRDIR)/MessageNumber_IllegalRootPageID.h \
	$(HDRDIR)/MessageNumber_VerifyFailed.h \
	$(HDRDIR)/Message_DiscordDelegateKey.h \
	$(HDRDIR)/Message_DiscordBitNum.h \
	$(HDRDIR)/Message_IllegalBtreeValue.h \
	$(HDRDIR)/Message_IllegalKeyIndex.h \
	$(HDRDIR)/Message_IllegalLastRowID.h \
	$(HDRDIR)/Message_IllegalLeftLeafPageID.h \
	$(HDRDIR)/Message_IllegalRightLeafPageID.h \
	$(HDRDIR)/Message_IllegalRootPageID.h \
	$(HDRDIR)/Message_VerifyFailed.h

/* headers installed in Driver */
LOCAL_EXPORT_HDRS = 

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(HDRDIR)/Data.h \
	$(HDRDIR)/FileDriver.h \
	$(HDRDIR)/FileID.h \
	$(HDRDIR)/LogicalInterface.h \
	$(HDRDIR)/Module.h \
	$(MESSAGE_HDRS)

HDRS = \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS) \
	$(HDRDIR)/Algorithm.h \
	$(HDRDIR)/AutoPointer.h \
	$(HDRDIR)/BitmapFile.h \
	$(HDRDIR)/BitmapIterator.h \
	$(HDRDIR)/BitmapPage.h \
	$(HDRDIR)/BtreeFile.h \
	$(HDRDIR)/BtreePage.h \
	$(HDRDIR)/Compare.h \
	$(HDRDIR)/CompressedBitmapFile.h \
	$(HDRDIR)/CompressedBitmapIterator.h \
	$(HDRDIR)/Condition.h \
	$(HDRDIR)/DirPage.h \
	$(HDRDIR)/File.h \
	$(HDRDIR)/HeaderPage.h \
	$(HDRDIR)/IDBlock.h \
	$(HDRDIR)/LongBitmapIterator.h \
	$(HDRDIR)/MiddleBitmapIterator.h \
	$(HDRDIR)/NodePage.h \
	$(HDRDIR)/NormalBitmapFile.h \
	$(HDRDIR)/NormalBitmapIterator.h \
	$(HDRDIR)/OpenOption.h \
	$(HDRDIR)/Page.h \
	$(HDRDIR)/PagePointer.h \
	$(HDRDIR)/Parameter.h \
	$(HDRDIR)/QueryNode.h \
	$(HDRDIR)/QueryNodeAnd.h \
	$(HDRDIR)/QueryNodeOperator.h \
	$(HDRDIR)/QueryNodeOr.h \
	$(HDRDIR)/QueryNodeTerm.h \
	$(HDRDIR)/SearchGroupBy.h \
	$(HDRDIR)/ShortBitmapIterator.h

MESSAGE_OBJS = \
	Message_DiscordDelegateKey$O \
	Message_DiscordBitNum$O \
	Message_IllegalBtreeValue$O \
	Message_IllegalKeyIndex$O \
	Message_IllegalLastRowID$O \
	Message_IllegalLeftLeafPageID$O \
	Message_IllegalRightLeafPageID$O \
	Message_IllegalRootPageID$O \
	Message_VerifyFailed$O

OBJS = \
	$(MESSAGE_OBJS) \
	Bitmap$O \
	BitmapFile$O \
	BitmapIterator$O \
	BitmapPage$O \
	BtreeFile$O \
	BtreePage$O \
	Compare$O \
	CompressedBitmapFile$O \
	CompressedBitmapIterator$O \
	Condition$O \
	Data$O \
	DirPage$O \
	File$O \
	FileDriver$O \
	FileID$O \
	HeaderPage$O \
	IDBlock$O \
	LogicalInterface$O \
	LongBitmapIterator$O \
	MiddleBitmapIterator$O \
	NodePage$O \
	NormalBitmapFile$O \
	NormalBitmapIterator$O \
	OpenOption$O \
	Page$O \
	QueryNode$O \
	QueryNodeAnd$O \
	QueryNodeOperator$O \
	QueryNodeOr$O \
	QueryNodeTerm$O \
	SearchGroupBy$O \
	ShortBitmapIterator$O

#ifdef SYD_DLL
EXPORTFLAGS = \
	-DSYD_BITMAP_EXPORT_FUNCTION \
	-DSYD_EXPORT_FUNCTION
#endif
EXTRACFLAGS = \
	$(EXPORTFLAGS) \
	-I..$(S)..$(S)..$(S)Kernel$(S)include
EXTRALDFLAGS =
EXTRAPURIFYFLAGS =
EXTRAQUANTIFYFLAGS =
EXTRAPURECOVFLAGS =
EXTRALOCALCFLAGS =

KERNEL_LIBS = \
	SyKernel$L

LDLIBS = \
	$(KERNEL_LIBS) \
	SyDrvCom$L \
	$(MODLIBS)

/********************************************************/

#ifdef SYD_C_MS7_0
FORCEMULTIPLE = /force:multiple /* ModObjectがなぜか多重定義になってしまう */
#endif
EXTRALOCALDLLFLAGS =  \
	$(FORCEMULTIPLE)
RCLOCALFLAGS = /i $(VERINFO_INCL)

TARGET = $P$(TARGET_BASE)$L
DLLTARGET = $P$(TARGET_BASE)$D
OLTARGET = $(MODULE).ol

ALLTARGETS = \
	$(MESSAGE_TARGET) \
	$(TARGET)

/*
 * all
 */
AllTarget($(ALLTARGETS))

LibraryTarget($(TARGET), $(OBJS))

DLLTarget($(DLLTARGET), $(RESOURCE) $(OBJS) $(TARGET_EXPORT))

ObjectListTarget($(OLTARGET), $(OBJS), $(TOP_INSTALL_DIR))

/*
 * message
 */
MessageTarget($(MESSAGE_TARGET), $(MESSAGE_DEFINITION), $(BUILD_JAR))

/*
 * install library and header
 */
InstallHeaderTarget($(TOP_EXPORT_HDRS), $(TOP_EXPORT_HDRDIR))
/* InstallHeaderTarget($(LOCAL_EXPORT_HDRS), $(LOCAL_EXPORT_HDRDIR)) */
InstallLibraryTarget($(TARGET), $(TOP_INSTALL_DIR))
InstallDLLTarget($(DLLTARGET), $(TOP_INSTALL_DIR))

/*
 * clean
 */
CleanTarget($(ALLTARGETS))
CleanTarget($(DLLTARGET))
CleanTarget($(OBJS))
CleanTarget($(TARGET_BASE).exp)

#if defined(OS_WINDOWSNT4_0) || defined(OS_WINDOWS98)
ResourceTarget($(RESOURCE), $(VERINFODIR)\$(MODULE).rc)
#endif

#include "Makefile.h"

$(MESSAGE_HDRS): $(MESSAGE_TARGET)
$(MESSAGE_OBJS): $(MESSAGE_TARGET)

/*
  Copyright (c) 1997, 2023 Ricoh Company, Ltd.
  All rights reserved.
*/
