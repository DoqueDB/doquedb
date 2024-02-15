/*
 * Makefile.c --- Kernel/PhysicalFile/c
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

MODULE = PhysicalFile
TARGET_BASE = SyPhFile
SRCDIR = ..
HDRDIR = ../$(MODULE)
MESSAGE_DEFINITION = $(SRCDIR)/MessageDefinition.xml
MESSAGE_TARGET = $(MESSAGE_DEFINITION)_

LOCAL_EXPORT_HDRDIR = ../../include/$(MODULE)
TOP_EXPORT_HDRDIR = ../../../include/$(MODULE)
TOP_INSTALL_DIR = ../../../c.CONF

/* above variables MUST be defined      */
/****************************************/

/* headers created for message */
MESSAGE_HDRS1 = \
	$(HDRDIR)/MessageAll_Class.h \
	$(HDRDIR)/MessageAll_Number.h \
	$(HDRDIR)/MessageArgument.h \
	$(HDRDIR)/MessageFormat_English.h \
	$(HDRDIR)/MessageFormat_Japanese.h \
	$(HDRDIR)/MessageNumber_CanNotCorrectAreaUseSituation.h \
	$(HDRDIR)/MessageNumber_CanNotCorrectPageUseSituation.h \
	$(HDRDIR)/MessageNumber_CanNotFixAreaManageTable.h \
	$(HDRDIR)/MessageNumber_CanNotFixHeaderPage.h \
	$(HDRDIR)/MessageNumber_CanNotFixPageTable.h \
	$(HDRDIR)/MessageNumber_CorrectedAreaUseSituation.h \
	$(HDRDIR)/MessageNumber_CorrectedPageUseSituation.h \
	$(HDRDIR)/MessageNumber_DiscordAreaUseSituation1.h \
	$(HDRDIR)/MessageNumber_DiscordAreaUseSituation2.h \
	$(HDRDIR)/MessageNumber_DiscordFreeAreaRate.h \
	$(HDRDIR)/MessageNumber_DiscordManagePageNum.h \
	$(HDRDIR)/MessageNumber_DiscordManagePageNumInTable.h \
	$(HDRDIR)/MessageNumber_DiscordPageArray.h \
	$(HDRDIR)/MessageNumber_DiscordPageUseSituation1.h \
	$(HDRDIR)/MessageNumber_DiscordPageUseSituation2.h \
	$(HDRDIR)/MessageNumber_DiscordPageUseSituation3.h \
	$(HDRDIR)/MessageNumber_DiscordUnuseAreaRate.h \
	$(HDRDIR)/MessageNumber_DiscordUnusePageNumInTable.h \
	$(HDRDIR)/MessageNumber_DiscordUsePageNum.h \
	$(HDRDIR)/MessageNumber_DiscordUsePageNumInTable.h

MESSAGE_HDRS2 = \
	$(HDRDIR)/MessageNumber_ExistDuplicateArea.h \
	$(HDRDIR)/MessageNumber_ExistProtrusiveArea.h \
	$(HDRDIR)/MessageNumber_InitializeFailed.h \
	$(HDRDIR)/MessageNumber_NotManagePage.h \
	$(HDRDIR)/MessageNumber_CanNotFixNode.h \
	$(HDRDIR)/MessageNumber_DiscordAreaUseSituation3.h \
	$(HDRDIR)/MessageNumber_CorrectedAreaUseSituation2.h \
	$(HDRDIR)/MessageNumber_DiscordLeafNodeFreeAreaRate.h \
	$(HDRDIR)/MessageNumber_CorrectedLeafNodeFreeAreaRate.h \
	$(HDRDIR)/MessageNumber_DiscordNodeFreeAreaRate.h \
	$(HDRDIR)/MessageNumber_CorrectedNodeFreeAreaRate.h \
	$(HDRDIR)/MessageNumber_DiscordNodeFreeArea.h \
	$(HDRDIR)/MessageNumber_CorrectedNodeFreeArea.h \
	$(HDRDIR)/MessageNumber_DiscordLeafHeader.h \
	$(HDRDIR)/MessageNumber_DisorderedLeafIndexKey.h \
	$(HDRDIR)/MessageNumber_DisorderedLeafIndexOffset.h \
	$(HDRDIR)/MessageNumber_DiscordAreaOffset.h \
	$(HDRDIR)/MessageNumber_CorrectedAreaUseSituation3.h \
	$(HDRDIR)/MessageNumber_DiscordLeafHeaderOffset.h \
	$(HDRDIR)/MessageNumber_CorrectedLeafHeaderOffset.h \
	$(HDRDIR)/MessageNumber_DiscordLeafIndexOffset.h

MESSAGE_HDRS3 = \
	$(HDRDIR)/Message_CanNotCorrectAreaUseSituation.h \
	$(HDRDIR)/Message_CanNotCorrectPageUseSituation.h \
	$(HDRDIR)/Message_CanNotFixAreaManageTable.h \
	$(HDRDIR)/Message_CanNotFixHeaderPage.h \
	$(HDRDIR)/Message_CanNotFixPageTable.h \
	$(HDRDIR)/Message_CorrectedAreaUseSituation.h \
	$(HDRDIR)/Message_CorrectedPageUseSituation.h \
	$(HDRDIR)/Message_DiscordAreaUseSituation1.h \
	$(HDRDIR)/Message_DiscordAreaUseSituation2.h \
	$(HDRDIR)/Message_DiscordFreeAreaRate.h \
	$(HDRDIR)/Message_DiscordManagePageNum.h \
	$(HDRDIR)/Message_DiscordManagePageNumInTable.h \
	$(HDRDIR)/Message_DiscordPageArray.h \
	$(HDRDIR)/Message_DiscordPageUseSituation1.h \
	$(HDRDIR)/Message_DiscordPageUseSituation2.h \
	$(HDRDIR)/Message_DiscordPageUseSituation3.h \
	$(HDRDIR)/Message_DiscordUnuseAreaRate.h \
	$(HDRDIR)/Message_DiscordUnusePageNumInTable.h \
	$(HDRDIR)/Message_DiscordUsePageNum.h \
	$(HDRDIR)/Message_DiscordUsePageNumInTable.h

MESSAGE_HDRS4 = \
	$(HDRDIR)/Message_ExistDuplicateArea.h \
	$(HDRDIR)/Message_ExistProtrusiveArea.h \
	$(HDRDIR)/Message_InitializeFailed.h \
	$(HDRDIR)/Message_NotManagePage.h \
	$(HDRDIR)/Message_CanNotFixNode.h \
	$(HDRDIR)/Message_DiscordAreaUseSituation3.h \
	$(HDRDIR)/Message_CorrectedAreaUseSituation2.h \
	$(HDRDIR)/Message_DiscordLeafNodeFreeAreaRate.h \
	$(HDRDIR)/Message_CorrectedLeafNodeFreeAreaRate.h \
	$(HDRDIR)/Message_DiscordNodeFreeAreaRate.h \
	$(HDRDIR)/Message_CorrectedNodeFreeAreaRate.h \
	$(HDRDIR)/Message_DiscordNodeFreeArea.h \
	$(HDRDIR)/Message_CorrectedNodeFreeArea.h \
	$(HDRDIR)/Message_DiscordLeafHeader.h \
	$(HDRDIR)/Message_DisorderedLeafIndexKey.h \
	$(HDRDIR)/Message_DisorderedLeafIndexOffset.h \
	$(HDRDIR)/Message_DiscordAreaOffset.h \
	$(HDRDIR)/Message_CorrectedAreaUseSituation3.h \
	$(HDRDIR)/Message_DiscordLeafHeaderOffset.h \
	$(HDRDIR)/Message_CorrectedLeafHeaderOffset.h \
	$(HDRDIR)/Message_DiscordLeafIndexOffset.h

MESSAGE_HDRS = \
	$(MESSAGE_HDRS1) \
	$(MESSAGE_HDRS2) \
	$(MESSAGE_HDRS3) \
	$(MESSAGE_HDRS4)

/* headers installed in Kernel */
LOCAL_EXPORT_HDRS =

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(HDRDIR)/Area.h \
	$(HDRDIR)/DirectArea.h \
	$(HDRDIR)/Content.h \
	$(HDRDIR)/File.h \
	$(HDRDIR)/Manager.h \
	$(HDRDIR)/Module.h \
	$(HDRDIR)/Page.h \
	$(HDRDIR)/Types.h

HDRS = \
	$(MESSAGE_HDRS) \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS) \
	$(HDRDIR)/AreaManageFile.h \
	$(HDRDIR)/DirectAreaFile.h \
	$(HDRDIR)/DirectAreaPage.h \
	$(HDRDIR)/DirectAreaTree.h \
	$(HDRDIR)/AreaManagePage.h \
	$(HDRDIR)/BitmapTable.h \
	$(HDRDIR)/NonManageFile.h \
	$(HDRDIR)/NonManagePage.h \
	$(HDRDIR)/PageManageFile.h \
	$(HDRDIR)/PageManageFile2.h \
	$(HDRDIR)/PageManagePage.h

MESSAGE_OBJS = \
	Message_CanNotCorrectAreaUseSituation$O \
	Message_CanNotCorrectPageUseSituation$O \
	Message_CanNotFixAreaManageTable$O \
	Message_CanNotFixHeaderPage$O \
	Message_CanNotFixPageTable$O \
	Message_CorrectedAreaUseSituation$O \
	Message_CorrectedPageUseSituation$O \
	Message_DiscordAreaUseSituation1$O \
	Message_DiscordAreaUseSituation2$O \
	Message_DiscordFreeAreaRate$O \
	Message_DiscordManagePageNum$O \
	Message_DiscordManagePageNumInTable$O \
	Message_DiscordPageArray$O \
	Message_DiscordPageUseSituation1$O \
	Message_DiscordPageUseSituation2$O \
	Message_DiscordPageUseSituation3$O \
	Message_DiscordUnuseAreaRate$O \
	Message_DiscordUnusePageNumInTable$O \
	Message_DiscordUsePageNum$O \
	Message_DiscordUsePageNumInTable$O \
	Message_ExistDuplicateArea$O \
	Message_ExistProtrusiveArea$O \
	Message_InitializeFailed$O \
	Message_NotManagePage$O \
	Message_CanNotFixNode$O \
	Message_DiscordAreaUseSituation3$O \
	Message_CorrectedAreaUseSituation2$O \
	Message_DiscordLeafNodeFreeAreaRate$O \
	Message_CorrectedLeafNodeFreeAreaRate$O \
	Message_DiscordNodeFreeAreaRate$O \
	Message_CorrectedNodeFreeAreaRate$O \
	Message_DiscordNodeFreeArea$O \
	Message_CorrectedNodeFreeArea$O \
	Message_DiscordLeafHeader$O \
	Message_DisorderedLeafIndexKey$O \
	Message_DisorderedLeafIndexOffset$O \
	Message_DiscordAreaOffset$O \
	Message_CorrectedAreaUseSituation3$O \
	Message_DiscordLeafHeaderOffset$O \
	Message_CorrectedLeafHeaderOffset$O \
	Message_DiscordLeafIndexOffset$O

OBJS = \
	$(MESSAGE_OBJS) \
	Area$O \
	DirectArea$O \
	AreaManageFile$O \
	DirectAreaFile$O \
	DirectAreaPage$O \
	DirectAreaTree$O \
	AreaManagePage$O \
	BitmapTable$O \
	Content$O \
	File$O \
	Manager$O \
	NonManageFile$O \
	NonManagePage$O \
	Page$O \
	PageManageFile$O \
	PageManageFile2$O \
	PageManagePage$O \
	PhysicalFile$O

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

ObjectListTarget($(TARGET), $(OBJS), $(TOP_INSTALL_DIR))

/*
 * message
 */
MessageTarget($(MESSAGE_TARGET), $(MESSAGE_DEFINITION), $(BUILD_JAR))

/*
 * install library and header
 */
InstallHeaderTarget($(TOP_EXPORT_HDRS), $(TOP_EXPORT_HDRDIR))
InstallHeaderTarget($(MESSAGE_HDRS1), $(TOP_EXPORT_HDRDIR))
InstallHeaderTarget($(MESSAGE_HDRS2), $(TOP_EXPORT_HDRDIR))
InstallHeaderTarget($(MESSAGE_HDRS3), $(TOP_EXPORT_HDRDIR))
InstallHeaderTarget($(MESSAGE_HDRS4), $(TOP_EXPORT_HDRDIR))
/* InstallHeaderTarget($(LOCAL_EXPORT_HDRS), $(LOCAL_EXPORT_HDRDIR)) */

/*
 * clean
 */
CleanTarget($(ALLTARGETS))
CleanTarget($(OBJS))

#include "Makefile.h"

$(MESSAGE_HDRS): $(MESSAGE_TARGET)
$(MESSAGE_OBJS): $(MESSAGE_TARGET)

/*
  Copyright (c) 1999, 2023 Ricoh Company, Ltd.
  All rights reserved.
*/
