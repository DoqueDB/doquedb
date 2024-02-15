/*
 * Makefile.c --- Kernel/Schema/c
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

MODULE = Schema
TARGET_BASE = SySchema
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
MESSAGE_HDRS = \
	$(HDRDIR)/MessageAll_Class.h \
	$(HDRDIR)/MessageAll_Number.h \
	$(HDRDIR)/MessageArgument.h \
	$(HDRDIR)/MessageFormat_English.h \
	$(HDRDIR)/MessageFormat_Japanese.h \
	$(HDRDIR)/MessageNumber_DatabasePathNotExist.h \
	$(HDRDIR)/MessageNumber_FileNotFound.h \
	$(HDRDIR)/MessageNumber_IndexTupleNotFound.h \
	$(HDRDIR)/MessageNumber_MetaIndexNotMatch.h \
	$(HDRDIR)/MessageNumber_MetaIndexTupleNotFound.h \
	$(HDRDIR)/MessageNumber_NotNullIntegrityViolation.h \
	$(HDRDIR)/MessageNumber_SequenceCorrectFailed.h \
	$(HDRDIR)/MessageNumber_SequenceCreateFailed.h \
	$(HDRDIR)/MessageNumber_SequenceNotExist.h \
	$(HDRDIR)/MessageNumber_SequenceReadFailed.h \
	$(HDRDIR)/MessageNumber_SequenceValueNotMatch.h \
	$(HDRDIR)/MessageNumber_TupleCountNotMatch.h \
	$(HDRDIR)/MessageNumber_TupleValueNotMatch.h \
	$(HDRDIR)/MessageNumber_VerifyFinished.h \
	$(HDRDIR)/MessageNumber_VerifyStarted.h \
	$(HDRDIR)/MessageNumber_VerifyTupleFinished.h \
	$(HDRDIR)/MessageNumber_VerifyTupleOnTheWay.h \
	$(HDRDIR)/MessageNumber_VerifyTupleStarted.h \
	$(HDRDIR)/Message_DatabasePathNotExist.h \
	$(HDRDIR)/Message_FileNotFound.h \
	$(HDRDIR)/Message_IndexTupleNotFound.h \
	$(HDRDIR)/Message_MetaIndexNotMatch.h \
	$(HDRDIR)/Message_MetaIndexTupleNotFound.h \
	$(HDRDIR)/Message_NotNullIntegrityViolation.h \
	$(HDRDIR)/Message_SequenceCorrectFailed.h \
	$(HDRDIR)/Message_SequenceCreateFailed.h \
	$(HDRDIR)/Message_SequenceNotExist.h \
	$(HDRDIR)/Message_SequenceReadFailed.h \
	$(HDRDIR)/Message_SequenceValueNotMatch.h \
	$(HDRDIR)/Message_TupleCountNotMatch.h \
	$(HDRDIR)/Message_TupleValueNotMatch.h \
	$(HDRDIR)/Message_VerifyFinished.h \
	$(HDRDIR)/Message_VerifyStarted.h \
	$(HDRDIR)/Message_VerifyTupleFinished.h \
	$(HDRDIR)/Message_VerifyTupleOnTheWay.h \
	$(HDRDIR)/Message_VerifyTupleStarted.h

/* headers installed in Kernel */
LOCAL_EXPORT_HDRS = \
	$(HDRDIR)/Area.h \
	$(HDRDIR)/AreaContent.h \
	$(HDRDIR)/AreaContentMap.h \
	$(HDRDIR)/AreaMap.h \
	$(HDRDIR)/AutoRWLock.h \
	$(HDRDIR)/Cascade.h \
	$(HDRDIR)/CascadeMap.h \
	$(HDRDIR)/Column.h \
	$(HDRDIR)/Constraint.h \
	$(HDRDIR)/Default.h \
	$(HDRDIR)/FileMap.h \
	$(HDRDIR)/Function.h \
	$(HDRDIR)/FunctionMap.h \
	$(HDRDIR)/Hint.h \
	$(HDRDIR)/Index.h \
	$(HDRDIR)/IndexMap.h \
	$(HDRDIR)/Key.h \
	$(HDRDIR)/ObjectMap.h \
	$(HDRDIR)/Partition.h \
	$(HDRDIR)/PartitionMap.h \
	$(HDRDIR)/Privilege.h \
	$(HDRDIR)/PrivilegeMap.h \
	$(HDRDIR)/Redo.h \
	$(HDRDIR)/Reorganize.h \
	$(HDRDIR)/SystemFile.h \
	$(HDRDIR)/SystemFileSub.h \
	$(HDRDIR)/SystemTable.h \
	$(HDRDIR)/SystemTable_Area.h \
	$(HDRDIR)/SystemTable_AreaContent.h \
	$(HDRDIR)/SystemTable_Cascade.h \
	$(HDRDIR)/SystemTable_Database.h \
	$(HDRDIR)/SystemTable_File.h \
	$(HDRDIR)/SystemTable_Function.h \
	$(HDRDIR)/SystemTable_Index.h \
	$(HDRDIR)/SystemTable_Partition.h \
	$(HDRDIR)/SystemTable_Privilege.h \
	$(HDRDIR)/SystemTable_Table.h \
	$(HDRDIR)/Table.h \
	$(HDRDIR)/TableMap.h \
	$(HDRDIR)/Undo.h \
	$(HDRDIR)/Utility.h \
	$(HDRDIR)/VirtualTable.h

/* headers installed in TOP */
TOP_EXPORT_HDRS1 = \
	$(HDRDIR)/AreaCategory.h \
	$(HDRDIR)/Database.h \
	$(HDRDIR)/Externalizable.h \
	$(HDRDIR)/Field.h \
	$(HDRDIR)/File.h \
	$(HDRDIR)/FileID.h \
	$(HDRDIR)/Hold.h \
	$(HDRDIR)/Identity.h \
	$(HDRDIR)/LogData.h \
	$(HDRDIR)/Manager.h \
	$(HDRDIR)/Meta.h \
	$(HDRDIR)/Module.h \
	$(HDRDIR)/Object.h \
	$(HDRDIR)/ObjectID.h \
	$(HDRDIR)/ObjectName.h \
	$(HDRDIR)/OpenOption.h \
	$(HDRDIR)/PathParts.h \
	$(HDRDIR)/TupleID.h

TOP_EXPORT_HDRS2 = \
	$(MESSAGE_HDRS)

TOP_EXPORT_HDRS = \
	$(TOP_EXPORT_HDRS1) \
	$(TOP_EXPORT_HDRS2)

HDRS = \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS) \
	$(HDRDIR)/AccessFile.h \
	$(HDRDIR)/AccessFullText.h \
	$(HDRDIR)/AccessLobFile.h \
	$(HDRDIR)/ArrayFile.h \
	$(HDRDIR)/ArrayIndex.h \
	$(HDRDIR)/AutoLatch.h \
	$(HDRDIR)/AutoLogicalLogFile.h \
	$(HDRDIR)/BitmapFile.h \
	$(HDRDIR)/BitmapIndex.h \
	$(HDRDIR)/BtreeFile.h \
	$(HDRDIR)/BtreeIndex.h \
	$(HDRDIR)/ColumnMap.h \
	$(HDRDIR)/ConstraintMap.h \
	$(HDRDIR)/DatabaseMap.h \
	$(HDRDIR)/Debug.h \
	$(HDRDIR)/ErrorRecovery.h \
	$(HDRDIR)/FakeError.h \
	$(HDRDIR)/FieldMap.h \
	$(HDRDIR)/FileReflect.h \
	$(HDRDIR)/FileVerify.h \
	$(HDRDIR)/FullTextFile.h \
	$(HDRDIR)/FullTextIndex.h \
	$(HDRDIR)/KdTreeFile.h \
	$(HDRDIR)/KdTreeIndex.h \
	$(HDRDIR)/KeyMap.h \
	$(HDRDIR)/LobFile.h \
	$(HDRDIR)/NameParts.h \
	$(HDRDIR)/ObjectSnapshot.h \
	$(HDRDIR)/Operation.h \
	$(HDRDIR)/Parameter.h \
	$(HDRDIR)/PathParts.h \
	$(HDRDIR)/RecordFile.h \
	$(HDRDIR)/Recovery.h \
	$(HDRDIR)/ReorganizeArea.h \
	$(HDRDIR)/ReorganizeCascade.h \
	$(HDRDIR)/ReorganizeColumn.h \
	$(HDRDIR)/ReorganizeConstraint.h \
	$(HDRDIR)/ReorganizeDatabase.h \
	$(HDRDIR)/ReorganizeExecutor.h \
	$(HDRDIR)/ReorganizeFunction.h \
	$(HDRDIR)/ReorganizeIndex.h \
	$(HDRDIR)/ReorganizePartition.h \
	$(HDRDIR)/ReorganizePrivilege.h \
	$(HDRDIR)/ReorganizeTable.h \
	$(HDRDIR)/Sequence.h \
	$(HDRDIR)/SessionID.h \
	$(HDRDIR)/SystemDatabase.h \
	$(HDRDIR)/SystemTable_Column.h \
	$(HDRDIR)/SystemTable_Constraint.h \
	$(HDRDIR)/SystemTable_Field.h \
	$(HDRDIR)/SystemTable_Key.h \
	$(HDRDIR)/TemporaryDatabase.h \
	$(HDRDIR)/TreeNode.h \
	$(HDRDIR)/VectorFile.h

MESSAGE_OBJS = \
	Message_DatabasePathNotExist$O \
	Message_FileNotFound$O \
	Message_IndexTupleNotFound$O \
	Message_MetaIndexNotMatch$O \
	Message_MetaIndexTupleNotFound$O \
	Message_NotNullIntegrityViolation$O \
	Message_SequenceCorrectFailed$O \
	Message_SequenceCreateFailed$O \
	Message_SequenceNotExist$O \
	Message_SequenceReadFailed$O \
	Message_SequenceValueNotMatch$O \
	Message_TupleCountNotMatch$O \
	Message_TupleValueNotMatch$O \
	Message_VerifyFinished$O \
	Message_VerifyStarted$O \
	Message_VerifyTupleFinished$O \
	Message_VerifyTupleOnTheWay$O \
	Message_VerifyTupleStarted$O

OBJS = \
	$(MESSAGE_OBJS) \
	AccessFile$O \
	AccessFullText$O \
	AccessLobFile$O \
	Area$O \
	AreaCategory$O \
	AreaContent$O \
	AreaContentMap$O \
	AreaMap$O \
	ArrayFile$O \
	ArrayIndex$O \
	AutoLatch$O \
	BitmapFile$O \
	BitmapIndex$O \
	BtreeFile$O \
	BtreeIndex$O \
	Cascade$O \
	CascadeMap$O \
	Column$O \
	ColumnMap$O \
	Constraint$O \
	ConstraintMap$O \
	Database$O \
	DatabaseMap$O \
	Debug$O \
	Default$O \
	Externalizable$O \
	Field$O \
	FieldMap$O \
	File$O \
	FileID$O \
	FileImport$O \
	FileMap$O \
	FileReflect$O \
	FileVerify$O \
	FullTextFile$O \
	FullTextIndex$O \
	Function$O \
	FunctionMap$O \
	Hint$O \
	Hold$O \
	Identity$O \
	Index$O \
	IndexMap$O \
	KdTreeFile$O \
	KdTreeIndex$O \
	Key$O \
	KeyMap$O \
	LobFile$O \
	LogData$O \
	Manager$O \
	Meta$O \
	Object$O \
	ObjectID$O \
	ObjectName$O \
	ObjectSnapshot$O \
	OpenOption$O \
	Partition$O \
	PartitionMap$O \
	Privilege$O \
	PrivilegeMap$O \
	RecordFile$O \
	Recovery$O \
	Redo$O \
	Reorganize$O \
	ReorganizeArea$O \
	ReorganizeCascade$O \
	ReorganizeColumn$O \
	ReorganizeConstraint$O \
	ReorganizeDatabase$O \
	ReorganizeExecutor$O \
	ReorganizeFunction$O \
	ReorganizeIndex$O \
	ReorganizePartition$O \
	ReorganizePrivilege$O \
	ReorganizeTable$O \
	Schema$O \
	Sequence$O \
	SystemDatabase$O \
	SystemFile$O \
	SystemFileSub$O \
	SystemTable$O \
	Table$O \
	TableMap$O \
	TemporaryDatabase$O \
	TreeNode$O \
	TupleID$O \
	Undo$O \
	Utility$O \
	VectorFile$O \
	VirtualTable$O

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
InstallHeaderTarget2($(TOP_EXPORT_HDRS1), $(TOP_EXPORT_HDRS2), $(TOP_EXPORT_HDRDIR))
InstallHeaderTarget($(LOCAL_EXPORT_HDRS), $(LOCAL_EXPORT_HDRDIR))

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
