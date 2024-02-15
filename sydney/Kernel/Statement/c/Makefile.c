/*
 * Makefile.c --- Kernel/Statement/c
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

.SUFFIXES: .lemon

SUBDIRS =

/****************************************/
/* following variables MUST be defined  */

MODULE = Statement
TARGET_BASE = SyStmt
SRCDIR = ..
HDRDIR = ../$(MODULE)
MESSAGE_DEFINITION =
MESSAGE_TARGET =

LOCAL_EXPORT_HDRDIR = ../../include/$(MODULE)
TOP_EXPORT_HDRDIR = ../../../include/$(MODULE)
TOP_INSTALL_DIR = ../../../c.CONF

/* above variables MUST be defined      */
/****************************************/

/* headers created for message */
MESSAGE_HDRS =

LEMON_HDRS = \
	$(HDRDIR)/SQLParserL.h

/* headers installed in Kernel */
LOCAL_EXPORT_HDRS1 = \
	$(HDRDIR)/AllType.h \
	$(HDRDIR)/AlterAreaAction.h \
	$(HDRDIR)/AlterAreaStatement.h \
	$(HDRDIR)/AlterCascadeStatement.h \
	$(HDRDIR)/AlterDatabaseAttribute.h \
	$(HDRDIR)/AlterDatabaseAttributeList.h \
	$(HDRDIR)/AlterDatabaseStatement.h \
	$(HDRDIR)/AlterIndexAction.h \
	$(HDRDIR)/AlterIndexStatement.h \
	$(HDRDIR)/AlterPartitionStatement.h \
	$(HDRDIR)/AlterTableAction.h \
	$(HDRDIR)/AlterTableStatement.h \
	$(HDRDIR)/AreaDataDefinition.h \
	$(HDRDIR)/AreaDefinition.h \
	$(HDRDIR)/AreaElementList.h \
	$(HDRDIR)/BulkSpecification.h \
	$(HDRDIR)/CascadeDefinition.h \
	$(HDRDIR)/CheckpointStatement.h \
	$(HDRDIR)/ColumnConstraintDefinition.h \
	$(HDRDIR)/ColumnConstraintDefinitionList.h \
	$(HDRDIR)/ColumnDefinition.h \
	$(HDRDIR)/ColumnName.h \
	$(HDRDIR)/ColumnNameList.h \
	$(HDRDIR)/CommitStatement.h \
	$(HDRDIR)/ContainsOperand.h \
	$(HDRDIR)/ContainsOperandList.h \
	$(HDRDIR)/ContainsOption.h \
	$(HDRDIR)/ContainsPredicate.h \
	$(HDRDIR)/CrossJoin.h \
	$(HDRDIR)/DataValue.h \
	$(HDRDIR)/DatabaseCreateOption.h \
	$(HDRDIR)/DatabaseCreateOptionList.h \
	$(HDRDIR)/DatabaseDefinition.h \
	$(HDRDIR)/DatabasePathElement.h \
	$(HDRDIR)/DatabasePathElementList.h \
	$(HDRDIR)/DeclareStatement.h \
	$(HDRDIR)/DeleteStatement.h \
	$(HDRDIR)/DerivedColumn.h \
	$(HDRDIR)/DisconnectStatement.h \
	$(HDRDIR)/DropAreaStatement.h \
	$(HDRDIR)/DropCascadeStatement.h \
	$(HDRDIR)/DropDatabaseStatement.h \
	$(HDRDIR)/DropFunctionStatement.h \
	$(HDRDIR)/DropIndexStatement.h \
	$(HDRDIR)/DropPartitionStatement.h \
	$(HDRDIR)/DropTableStatement.h \
	$(HDRDIR)/EndBackupStatement.h \
	$(HDRDIR)/EndExplainStatement.h \
	$(HDRDIR)/ExistsJoin.h \
	$(HDRDIR)/Expand.h \
	$(HDRDIR)/ExplainOption.h \
	$(HDRDIR)/ExplainStatement.h

LOCAL_EXPORT_HDRS2 = \
	$(HDRDIR)/FunctionDefinition.h \
	$(HDRDIR)/GrantStatement.h \
	$(HDRDIR)/GroupByClause.h \
	$(HDRDIR)/GroupingColumnReference.h \
	$(HDRDIR)/GroupingColumnReferenceList.h \
	$(HDRDIR)/GroupingElementList.h \
	$(HDRDIR)/HavingClause.h \
	$(HDRDIR)/Hint.h \
	$(HDRDIR)/HintElement.h \
	$(HDRDIR)/HintElementList.h \
	$(HDRDIR)/Identifier.h \
	$(HDRDIR)/IdentifierList.h \
	$(HDRDIR)/IndexDefinition.h \
	$(HDRDIR)/InPredicate.h \
	$(HDRDIR)/InsertColumnsAndSource.h \
	$(HDRDIR)/InsertStatement.h \
	$(HDRDIR)/IntegerArray.h \
	$(HDRDIR)/IntegerValue.h \
	$(HDRDIR)/IsolationLevel.h \
	$(HDRDIR)/ItemReference.h \
	$(HDRDIR)/JoinType.h \
	$(HDRDIR)/LikePredicate.h \
	$(HDRDIR)/LimitSpecification.h \
	$(HDRDIR)/Literal.h \
	$(HDRDIR)/LogicalLogOption.h \
	$(HDRDIR)/MountDatabaseStatement.h \
	$(HDRDIR)/MoveDatabaseStatement.h \
	$(HDRDIR)/ObjectConnection.h \
	$(HDRDIR)/ObjectList.h \
	$(HDRDIR)/ObjectSelection.h \
	$(HDRDIR)/OptionalAreaParameter.h \
	$(HDRDIR)/OptionalAreaParameterList.h \
	$(HDRDIR)/ParameterDeclaration.h \
	$(HDRDIR)/ParameterDeclarationList.h \
	$(HDRDIR)/PartitionDefinition.h \
	$(HDRDIR)/QualifiedJoin.h \
	$(HDRDIR)/QueryExpression.h \
	$(HDRDIR)/QueryOperator.h \
	$(HDRDIR)/QuerySpecification.h \
	$(HDRDIR)/QueryTerm.h

LOCAL_EXPORT_HDRS3 = \
	$(HDRDIR)/ReturnsClause.h \
	$(HDRDIR)/ReturnStatement.h \
	$(HDRDIR)/RevokeStatement.h \
	$(HDRDIR)/RollbackStatement.h \
	$(HDRDIR)/RoutineBody.h \
	$(HDRDIR)/SelectList.h \
	$(HDRDIR)/SelectStatement.h \
	$(HDRDIR)/SelectSubList.h \
	$(HDRDIR)/SelectSubListList.h \
	$(HDRDIR)/SelectTargetList.h \
	$(HDRDIR)/SetTransactionStatement.h \
	$(HDRDIR)/SimilarPredicate.h \
	$(HDRDIR)/SortSpecification.h \
	$(HDRDIR)/SortSpecificationList.h \
	$(HDRDIR)/StartBackupStatement.h \
	$(HDRDIR)/StartExplainStatement.h \
	$(HDRDIR)/StartTransactionStatement.h \
	$(HDRDIR)/StringValue.h \
	$(HDRDIR)/SyncStatement.h \
	$(HDRDIR)/TableConstraintDefinition.h \
	$(HDRDIR)/TableCorrelationSpec.h \
	$(HDRDIR)/TableDefinition.h \
	$(HDRDIR)/TableElementList.h \
	$(HDRDIR)/TableExpression.h \
	$(HDRDIR)/TablePrimary.h \
	$(HDRDIR)/TableReferenceList.h \
	$(HDRDIR)/TransactAccMode.h \
	$(HDRDIR)/TransactionMode.h \
	$(HDRDIR)/TransactionModeList.h \
	$(HDRDIR)/TreeWalker.h \
	$(HDRDIR)/UnmountDatabaseStatement.h \
	$(HDRDIR)/UpdateSetClause.h \
	$(HDRDIR)/UpdateSetClauseList.h \
	$(HDRDIR)/UpdateStatement.h \
	$(HDRDIR)/Utility.h \
	$(HDRDIR)/ValueExpression.h \
	$(HDRDIR)/ValueExpressionList.h \
	$(HDRDIR)/VariableName.h \
	$(HDRDIR)/VerifyOptionList.h \
	$(HDRDIR)/VerifyStatement.h \
	$(HDRDIR)/XA_CommitStatement.h \
	$(HDRDIR)/XA_EndStatement.h \
	$(HDRDIR)/XA_ForgetStatement.h \
	$(HDRDIR)/XA_Identifier.h \
	$(HDRDIR)/XA_PrepareStatement.h \
	$(HDRDIR)/XA_RecoverStatement.h \
	$(HDRDIR)/XA_RollbackStatement.h \
	$(HDRDIR)/XA_StartStatement.h

LOCAL_EXPORT_HDRS = \
	$(LOCAL_EXPORT_HDRS1) \
	$(LOCAL_EXPORT_HDRS2) \
	$(LOCAL_EXPORT_HDRS3)

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(MESSAGE_HDRS) \
	$(HDRDIR)/AreaOption.h \
	$(HDRDIR)/DLL.h \
	$(HDRDIR)/Module.h \
	$(HDRDIR)/Object.h \
	$(HDRDIR)/SQLParser.h \
	$(HDRDIR)/SQLScanner.h \
	$(HDRDIR)/SQLWrapper.h \
	$(HDRDIR)/Token.h \
	$(HDRDIR)/Type.h \
	$(HDRDIR)/TypeList.h

HDRS = \
	$(LEMON_HDRS) \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS) \
	 $(HDRDIR)/Externalizable.h

MESSAGE_OBJS =

LEMON_OBJS = \
	SQLParserL$O

OBJS1 = \
	AlterAreaAction$O \
	AlterAreaStatement$O \
	AlterCascadeStatement$O \
	AlterDatabaseAttribute$O \
	AlterDatabaseAttributeList$O \
	AlterDatabaseStatement$O \
	AlterIndexAction$O \
	AlterIndexStatement$O \
	AlterPartitionStatement$O \
	AlterTableAction$O \
	AlterTableStatement$O \
	AreaDataDefinition$O \
	AreaDefinition$O \
	AreaElementList$O \
	AreaOption$O \
	BulkSpecification$O \
	CascadeDefinition$O \
	CheckpointStatement$O \
	ColumnConstraintDefinition$O \
	ColumnConstraintDefinitionList$O \
	ColumnDefinition$O \
	ColumnName$O \
	ColumnNameList$O \
	CommitStatement$O \
	ContainsOperand$O \
	ContainsOperandList$O \
	ContainsOption$O \
	ContainsPredicate$O \
	CrossJoin$O \
	DataValue$O \
	DatabaseCreateOption$O \
	DatabaseCreateOptionList$O \
	DatabaseDefinition$O \
	DatabasePathElement$O \
	DatabasePathElementList$O \
	DeclareStatement$O \
	DeleteStatement$O \
	DerivedColumn$O \
	DisconnectStatement$O \
	DropAreaStatement$O \
	DropCascadeStatement$O \
	DropDatabaseStatement$O \
	DropFunctionStatement$O \
	DropIndexStatement$O \
	DropPartitionStatement$O \
	DropTableStatement$O \
	EndBackupStatement$O \
	EndExplainStatement$O \
	ExistsJoin$O \
	Expand$O \
	ExplainOption$O \
	ExplainStatement$O \
	FunctionDefinition$O \
	GetInstance$O \
	GrantStatement$O \
	GroupByClause$O \
	GroupingColumnReference$O \
	GroupingColumnReferenceList$O \
	GroupingElementList$O \
	HavingClause$O \
	Hint$O \
	HintElement$O \
	HintElementList$O \
	Identifier$O \
	IdentifierList$O \
	IndexDefinition$O \
	InPredicate$O \
	InsertColumnsAndSource$O \
	InsertStatement$O \
	IntegerArray$O \
	IntegerValue$O \
	IsolationLevel$O \
	ItemReference$O \
	LikePredicate$O \
	LimitSpecification$O \
	Literal$O
OBJS2 = \
	MountDatabaseStatement$O \
	MoveDatabaseStatement$O \
	Object$O \
	ObjectConnection$O \
	ObjectList$O \
	ObjectSelection$O \
	OptionalAreaParameter$O \
	OptionalAreaParameterList$O \
	ParameterDeclaration$O \
	ParameterDeclarationList$O \
	PartitionDefinition$O \
	QualifiedJoin$O \
	QueryExpression$O \
	QueryOperator$O \
	QuerySpecification$O \
	QueryTerm$O \
	ReturnsClause$O \
	ReturnStatement$O \
	RevokeStatement$O \
	RollbackStatement$O \
	RoutineBody$O \
	SQLParser$O \
	SQLScanner$O \
	SQLWrapper$O \
	SelectList$O \
	SelectStatement$O \
	SelectSubList$O \
	SelectSubListList$O \
	SelectTargetList$O \
	SetTransactionStatement$O \
	SimilarPredicate$O \
	SortSpecification$O \
	SortSpecificationList$O \
	StartBackupStatement$O \
	StartExplainStatement$O \
	StartTransactionStatement$O \
	Statement$O \
	StringValue$O \
	SyncStatement$O \
	TableConstraintDefinition$O \
	TableCorrelationSpec$O \
	TableDefinition$O \
	TableElementList$O \
	TableExpression$O \
	TablePrimary$O \
	TableReferenceList$O \
	Token$O \
	TransactAccMode$O \
	TransactionMode$O \
	TransactionModeList$O \
	TreeWalker$O \
	Type$O \
	UnmountDatabaseStatement$O \
	UpdateSetClause$O \
	UpdateSetClauseList$O \
	UpdateStatement$O \
	Utility$O \
	ValueExpression$O \
	ValueExpressionList$O \
	VariableName$O \
	VerifyStatement$O \
	XA_CommitStatement$O \
	XA_EndStatement$O \
	XA_ForgetStatement$O \
	XA_Identifier$O \
	XA_PrepareStatement$O \
	XA_RecoverStatement$O \
	XA_RollbackStatement$O \
	XA_StartStatement$O

OBJS3 = \
	$(MESSAGE_OBJS) \
	$(LEMON_OBJS)

OBJS = \
	$(OBJS1) \
	$(OBJS2) \
	$(OBJS3)

LEMON_SRCDIR = $(SRCDIR)/LemonSrc
LEMON_TARGET = \
	$(SRCDIR)/SQLParserL.cpp

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

#if defined(OS_WINDOWSNT4_0) || defined(OS_WINDOWS98)
$(LEMON_TARGET):: $(LEMON_SRCDIR)/$$(@B).lemon
	-$(SYDLOCALTOOL)\lemon\exe\lemon $(LEMON_SRCDIR)/$(@B).lemon
	perl $(LEMON_SRCDIR)\fixlemon.pl $(LEMON_SRCDIR)/$(@B).c $(LEMON_SRCDIR)/$(@B).cpp
	$(CP) $(LEMON_SRCDIR)/$(@B).cpp $@
	$(CP) $(LEMON_SRCDIR)/$(@B).h $(LEMON_HDRS)
#endif

/*
 * all
 */
AllTarget($(ALLTARGETS))

ObjectListTarget3($(TARGET), $(OBJS1), $(OBJS2), $(OBJS3), $(TOP_INSTALL_DIR))

/*
 * message
 */
/* MessageTarget($(MESSAGE_TARGET), $(MESSAGE_DEFINITION), $(BUILD_JAR)) */

/*
 * install library and header
 */
InstallHeaderTarget($(TOP_EXPORT_HDRS), $(TOP_EXPORT_HDRDIR))
InstallHeaderTarget($(LOCAL_EXPORT_HDRS1), $(LOCAL_EXPORT_HDRDIR))
InstallHeaderTarget($(LOCAL_EXPORT_HDRS2), $(LOCAL_EXPORT_HDRDIR))
InstallHeaderTarget($(LOCAL_EXPORT_HDRS3), $(LOCAL_EXPORT_HDRDIR))

/*
 * clean
 */
CleanTarget($(ALLTARGETS))
CleanTarget($(OBJS1))
CleanTarget($(OBJS2))
CleanTarget($(OBJS3))

$(LEMON_OBJS): $(LEMON_HDRS)

#include "Makefile.h"

/*
  Copyright (c) 1999, 2023 Ricoh Company, Ltd.
  All rights reserved.
*/
