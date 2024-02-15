/*
 * Makefile.c --- Driver/Btree/c
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

MODULE = Btree
TARGET_BASE = SyDrvBtr
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
	$(HDRDIR)/MessageNumber_DiscordKeyNum.h \
	$(HDRDIR)/MessageNumber_DiscordNextLeaf.h \
	$(HDRDIR)/MessageNumber_DiscordRootNode.h \
	$(HDRDIR)/MessageNumber_ExistLastObject.h \
	$(HDRDIR)/MessageNumber_ExistTopObject.h \
	$(HDRDIR)/MessageNumber_IllegalFileVersion.h \
	$(HDRDIR)/MessageNumber_IllegalKeyInfoIndex.h \
	$(HDRDIR)/MessageNumber_IllegalNextNodePageID.h \
	$(HDRDIR)/MessageNumber_IllegalTreeDepth.h \
	$(HDRDIR)/MessageNumber_KeyInfoIndexNotEqualZero.h \
	$(HDRDIR)/MessageNumber_NotUnique.h \
	$(HDRDIR)/MessageNumber_VerifyFailed.h \
	$(HDRDIR)/Message_DiscordDelegateKey.h \
	$(HDRDIR)/Message_DiscordKeyNum.h \
	$(HDRDIR)/Message_DiscordNextLeaf.h \
	$(HDRDIR)/Message_DiscordRootNode.h \
	$(HDRDIR)/Message_ExistLastObject.h \
	$(HDRDIR)/Message_ExistTopObject.h \
	$(HDRDIR)/Message_IllegalFileVersion.h \
	$(HDRDIR)/Message_IllegalKeyInfoIndex.h \
	$(HDRDIR)/Message_IllegalNextNodePageID.h \
	$(HDRDIR)/Message_IllegalTreeDepth.h \
	$(HDRDIR)/Message_KeyInfoIndexNotEqualZero.h \
	$(HDRDIR)/Message_NotUnique.h \
	$(HDRDIR)/Message_VerifyFailed.h

/* headers installed in Driver */
LOCAL_EXPORT_HDRS = \
	$(HDRDIR)/FetchHint.h \
	$(HDRDIR)/File.h \
	$(HDRDIR)/FileParameter.h \
	$(HDRDIR)/KeyPosType.h \
	$(HDRDIR)/NullBitmap.h \
	$(HDRDIR)/OpenParameter.h \
	$(HDRDIR)/PageVector.h \
	$(HDRDIR)/SearchHint.h

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(HDRDIR)/FileOption.h \
	$(HDRDIR)/FileDriver.h \
	$(HDRDIR)/Module.h \
	$(HDRDIR)/OpenOption.h \
	$(MESSAGE_HDRS)

HDRS = \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS) \
	$(HDRDIR)/AreaObject.h \
	$(HDRDIR)/Config.h \
	$(HDRDIR)/Estimator.h \
	$(HDRDIR)/FileDriver.h \
	$(HDRDIR)/FileInformation.h \
	$(HDRDIR)/Hint.h \
	$(HDRDIR)/HintTools.h \
	$(HDRDIR)/KeyInformation.h \
	$(HDRDIR)/MultiSearchConditionItem.h \
	$(HDRDIR)/NodePageHeader.h \
	$(HDRDIR)/OpenOptionAnalyzer.h \
	$(HDRDIR)/TreeFile.h \
	$(HDRDIR)/UseInfo.h \
	$(HDRDIR)/ValueFile.h \
	$(HDRDIR)/Version.h

MESSAGE_OBJS = \
	Message_DiscordDelegateKey$O \
	Message_DiscordKeyNum$O \
	Message_DiscordNextLeaf$O \
	Message_DiscordRootNode$O \
	Message_ExistLastObject$O \
	Message_ExistTopObject$O \
	Message_IllegalFileVersion$O \
	Message_IllegalKeyInfoIndex$O \
	Message_IllegalNextNodePageID$O \
	Message_IllegalTreeDepth$O \
	Message_KeyInfoIndexNotEqualZero$O \
	Message_NotUnique$O \
	Message_VerifyFailed$O

OBJS = \
	$(MESSAGE_OBJS) \
	AreaObject$O \
	Btree$O \
	Config$O \
	DBGetFileDriver$O \
	Estimator$O \
	FetchHint$O \
	File$O \
	FileDriver$O \
	FileInformation$O \
	FileParameter$O \
	File_Expunge$O \
	File_Fetch$O \
	File_Index$O \
	File_Insert$O \
	File_Like$O \
	File_Search$O \
	File_SimpleKey_Expunge$O \
	File_SimpleKey_Fetch$O \
	File_SimpleKey_Insert$O \
	File_SimpleKey_Search$O \
	File_SimpleKey_Update$O \
	File_SimpleKey_Verify$O \
	File_Update$O \
	File_Variable$O \
	File_Verify$O \
	HintTools$O \
	KeyInformation$O \
	KeyPosType$O \
	MultiSearchConditionItem$O \
	NodePageHeader$O \
	NullBitmap$O \
	OpenOptionAnalyzer$O \
	OpenParameter$O \
	SearchHint$O \
	TreeFile$O \
	UseInfo$O \
	ValueFile$O

#ifdef SYD_DLL
EXPORTFLAGS = \
	-DSYD_BTREE_EXPORT_FUNCTION \
	-DSYD_EXPORT_FUNCTION
#endif
EXTRACFLAGS = \
	$(EXPORTFLAGS)
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
InstallHeaderTarget($(LOCAL_EXPORT_HDRS), $(LOCAL_EXPORT_HDRDIR))
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
