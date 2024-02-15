/*
 * Makefile.c --- Kernel/Analysis/c
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

SUBDIRS = \
	Function  \
	Interface \
	Operation \
	Predicate \
	Procedure \
	Query	  \
	Value

/****************************************/
/* following variables MUST be defined  */

MODULE = Analysis
TARGET_BASE = SyAnalys
SRCDIR = ..
HDRDIR = ../$(MODULE)
MESSAGE_DEFINITION =
MESSAGE_TARGET =

LOCAL_EXPORT_HDRDIR = ../../include/$(MODULE)
TOP_EXPORT_HDRDIR = ../../../include/$(MODULE)
TOP_INSTALL_DIR = ../../../c.CONF

/* above variables MUST be defined      */
/****************************************/

/* Object list from subdirs */
SUBDIR_OBJECT_LIST = \
	Function.ol		 \
	Interface.ol	 \
	Operation.ol	 \
	Predicate.ol	 \
	Procedure.ol	 \
	Query.ol		 \
	Value.ol

/* headers created for message */
MESSAGE_HDRS =

/* headers installed in Kernel */
#ifdef USE_OLDER_VERSION
LOCAL_EXPORT_HDRS1 = \
	$(HDRDIR)/Analyzer.h \
	$(HDRDIR)/Argument.h \
	$(HDRDIR)/BulkSpecification.h \
	$(HDRDIR)/ColumnInfoSetByName.h \
	$(HDRDIR)/ContainsOperand.h \
	$(HDRDIR)/ContainsOperand_FreeText.h \
	$(HDRDIR)/ContainsOperand_Pattern.h \
	$(HDRDIR)/ContainsOperand_And.h \
	$(HDRDIR)/ContainsOperand_Or.h \
	$(HDRDIR)/ContainsOperand_AndNot.h \
	$(HDRDIR)/ContainsOperand_SpecialPattern.h \
	$(HDRDIR)/ContainsOperand_Synonym.h \
	$(HDRDIR)/ContainsOperand_Weight.h \
	$(HDRDIR)/ContainsOperand_Within.h \
	$(HDRDIR)/ContainsOperand_Word.h \
	$(HDRDIR)/ContainsOperand_WordList.h \
	$(HDRDIR)/ContainsPredicate.h \
	$(HDRDIR)/CrossJoin.h \
	$(HDRDIR)/DeleteStatement_Base.h \
/*	$(HDRDIR)/DeleteStatement_Positioned.h */\
	$(HDRDIR)/DeleteStatement_Searched.h \
/*	$(HDRDIR)/DerivedColumn.h */\
	$(HDRDIR)/Environment.h \
	$(HDRDIR)/ExistsJoin.h \
	$(HDRDIR)/Expand.h \
	$(HDRDIR)/GroupByClause.h \
	$(HDRDIR)/GroupingColumnReferenceList.h \
	$(HDRDIR)/InPredicate.h \
	$(HDRDIR)/InPredicate_Value.h \
	$(HDRDIR)/InsertStatement_Base.h \
	$(HDRDIR)/InsertStatement_Bulk_ColumnList.h \
	$(HDRDIR)/InsertStatement_Bulk_NoColumnList.h \
	$(HDRDIR)/InsertStatement_Default.h \
	$(HDRDIR)/InsertStatement_SubQuery_ColumnList.h \
	$(HDRDIR)/InsertStatement_SubQuery_NoColumnList.h \
	$(HDRDIR)/ItemReference_NoQualifier.h \
	$(HDRDIR)/ItemReference_Qualifier.h \
	$(HDRDIR)/LikePredicate.h \
	$(HDRDIR)/Literal.h \
	$(HDRDIR)/Module.h

LOCAL_EXPORT_HDRS2 = \
	$(HDRDIR)/QualifiedJoin_Base.h \
	$(HDRDIR)/QualifiedJoin_ColumnList.h \
	$(HDRDIR)/QualifiedJoin_Condition.h \
	$(HDRDIR)/QualifiedJoin_Natural.h \
	$(HDRDIR)/QueryExpression_Base.h \
	$(HDRDIR)/QueryExpression_NoSetOperator.h \
	$(HDRDIR)/QueryExpression_SetOperator.h \
	$(HDRDIR)/QuerySpecification.h \
	$(HDRDIR)/Recovery_Delete.h \
	$(HDRDIR)/Recovery_Insert.h \
	$(HDRDIR)/Recovery_Update.h \
	$(HDRDIR)/Recovery_UndoExpunge.h \
	$(HDRDIR)/Recovery_UndoUpdate.h \
	$(HDRDIR)/Reorganize_Import.h \
	$(HDRDIR)/SelectList_Asterisk.h \
	$(HDRDIR)/SelectList_List.h \
	$(HDRDIR)/SelectSubList_DerivedColumn.h \
	$(HDRDIR)/SelectSubList_Identifier.h \
/*	$(HDRDIR)/SelectSubList_SubQuery.h */\
	$(HDRDIR)/SimilarPredicate.h \
	$(HDRDIR)/TableExpression.h \
	$(HDRDIR)/TablePrimary_BaseTable.h \
	$(HDRDIR)/TablePrimary_DerivedTable.h \
	$(HDRDIR)/TablePrimary_JoinedTable.h \
	$(HDRDIR)/UpdateStatement_Base.h \
/*	$(HDRDIR)/UpdateStatement_Positioned.h */\
	$(HDRDIR)/UpdateStatement_Searched.h \
	$(HDRDIR)/UndoLog_Base.h \
	$(HDRDIR)/UndoLog_Delete.h \
	$(HDRDIR)/UndoLog_Insert.h \
	$(HDRDIR)/UndoLog_Update.h \
	$(HDRDIR)/Utility.h

LOCAL_EXPORT_HDRS3 = \
	$(HDRDIR)/ValueExpression.h \
	$(HDRDIR)/ValueExpression_And.h \
	$(HDRDIR)/ValueExpression_Arithmetic_Dyadic.h \
	$(HDRDIR)/ValueExpression_Arithmetic_Monadic.h \
	$(HDRDIR)/ValueExpression_ArrayConstructor.h \
	$(HDRDIR)/ValueExpression_ArrayReference.h \
	$(HDRDIR)/ValueExpression_Between.h \
	$(HDRDIR)/ValueExpression_CharLength.h \
	$(HDRDIR)/ValueExpression_Comparison_Row.h \
	$(HDRDIR)/ValueExpression_Comparison_Scalar.h \
	$(HDRDIR)/ValueExpression_Concatenate.h \
	$(HDRDIR)/ValueExpression_Contains.h \
	$(HDRDIR)/ValueExpression_Exists.h \
	$(HDRDIR)/ValueExpression_ExpandSynonym.h \
	$(HDRDIR)/ValueExpression_FullText.h \
	$(HDRDIR)/ValueExpression_FullTextLength.h \
	$(HDRDIR)/ValueExpression_Function_Niladic.h \
	$(HDRDIR)/ValueExpression_Function_Set.h \
	$(HDRDIR)/ValueExpression_In.h \
	$(HDRDIR)/ValueExpression_IsNull.h \
	$(HDRDIR)/ValueExpression_ItemReference.h \
	$(HDRDIR)/ValueExpression_Kwic.h \
	$(HDRDIR)/ValueExpression_Like.h \
	$(HDRDIR)/ValueExpression_Literal.h \
	$(HDRDIR)/ValueExpression_Normalize.h \
	$(HDRDIR)/ValueExpression_Not.h \
	$(HDRDIR)/ValueExpression_Null.h \
	$(HDRDIR)/ValueExpression_Or.h \
	$(HDRDIR)/ValueExpression_Overlay.h \
	$(HDRDIR)/ValueExpression_PlaceHolder.h \
	$(HDRDIR)/ValueExpression_RowConstructor.h \
	$(HDRDIR)/ValueExpression_SubString.h \
	$(HDRDIR)/ValueExpression_TableConstructor.h \
	$(HDRDIR)/ValueExpression_WordCount.h

LOCAL_EXPORT_HDRS = \
	$(LOCAL_EXPORT_HDRS1) \
	$(LOCAL_EXPORT_HDRS2) \
	$(LOCAL_EXPORT_HDRS3)

#else
LOCAL_EXPORT_HDRS = \
	$(HDRDIR)/Declaration.h \
	$(HDRDIR)/Module.h
#endif

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(MESSAGE_HDRS)

HDRS = \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS)

MESSAGE_OBJS =

#ifdef USE_OLDER_VERSION
OBJS1 = \
	Analyzer$O \
	BulkSpecification$O \
	ColumnInfoSetByName$O \
	ContainsOperand_FreeText$O \
	ContainsOperand_Pattern$O \
	ContainsOperand_And$O \
	ContainsOperand_Or$O \
	ContainsOperand_AndNot$O \
	ContainsOperand_SpecialPattern$O \
	ContainsOperand_Synonym$O \
	ContainsOperand_Weight$O \
	ContainsOperand_Within$O \
	ContainsOperand_Word$O \
	ContainsOperand_WordList$O \
	ContainsPredicate$O \
	CrossJoin$O \
	DeleteStatement_Base$O \
/*	DeleteStatement_Positioned$O */\
	DeleteStatement_Searched$O \
/*	DerivedColumn$O */\
	Environment$O \
	ExistsJoin$O \
	Expand$O \
	GroupByClause$O \
	GroupingColumnReferenceList$O \
	InPredicate$O \
	InPredicate_Value$O \
	InsertStatement_Base$O \
	InsertStatement_Bulk_ColumnList$O \
	InsertStatement_Bulk_NoColumnList$O \
	InsertStatement_Default$O \
	InsertStatement_SubQuery_ColumnList$O \
	InsertStatement_SubQuery_NoColumnList$O \
	ItemReference_NoQualifier$O \
	ItemReference_Qualifier$O \
	LikePredicate$O \
	Literal$O \
	QualifiedJoin_Base$O \
	QualifiedJoin_ColumnList$O \
	QualifiedJoin_Condition$O \
	QualifiedJoin_Natural$O \
	QueryExpression_Base$O \
	QueryExpression_NoSetOperator$O \
	QueryExpression_SetOperator$O \
	QuerySpecification$O \
	Recovery_Delete$O \
	Recovery_Insert$O \
	Recovery_Update$O \
	Recovery_UndoExpunge$O \
	Recovery_UndoUpdate$O \
	Reorganize_Import$O \
	SelectList_Asterisk$O \
	SelectList_List$O \
	SelectSubList_DerivedColumn$O \
	SelectSubList_Identifier$O \
/*	SelectSubList_SubQuery$O */\
	SimilarPredicate$O \
	TableExpression$O \
	TablePrimary_BaseTable$O \
	TablePrimary_DerivedTable$O \
	TablePrimary_JoinedTable$O \
	UpdateStatement_Base$O \
/*	UpdateStatement_Positioned$O */\
	UpdateStatement_Searched$O \
	UndoLog_Base$O \
	UndoLog_Delete$O \
	UndoLog_Insert$O \
	UndoLog_Update$O \
	Utility$O

OBJS2 = \
	ValueExpression_And$O \
	ValueExpression_Arithmetic_Dyadic$O \
	ValueExpression_Arithmetic_Monadic$O \
	ValueExpression_ArrayConstructor$O \
	ValueExpression_ArrayReference$O \
	ValueExpression_Between$O \
	ValueExpression_CharLength$O \
	ValueExpression_Comparison_Row$O \
	ValueExpression_Comparison_Scalar$O \
	ValueExpression_Concatenate$O \
	ValueExpression_Contains$O \
	ValueExpression_Exists$O \
	ValueExpression_ExpandSynonym$O \
	ValueExpression_FullText$O \
	ValueExpression_FullTextLength$O \
	ValueExpression_Function_Niladic$O \
	ValueExpression_Function_Set$O \
	ValueExpression_In$O \
	ValueExpression_IsNull$O \
	ValueExpression_ItemReference$O \
	ValueExpression_Kwic$O \
	ValueExpression_Like$O \
	ValueExpression_Literal$O \
	ValueExpression_Normalize$O \
	ValueExpression_Not$O \
	ValueExpression_Null$O \
	ValueExpression_Or$O \
	ValueExpression_Overlay$O \
	ValueExpression_PlaceHolder$O \
	ValueExpression_RowConstructor$O \
	ValueExpression_SubString$O \
	ValueExpression_TableConstructor$O \
	ValueExpression_WordCount$O

OBJS = \
	$(MESSAGE_OBJS) \
	$(OBJS1) \
	$(OBJS2)

#else
OBJS = \
	$(MESSAGE_OBJS)
#endif

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

ALLTARGETS =	 	  \
	objlist-r		  \
	$(MESSAGE_TARGET) \
	$(TARGET)

/*
 * all
 */
AllTarget($(ALLTARGETS))

#ifdef USE_OLDER_VERSION
ObjectListTarget2SubDir($(TARGET), $(OBJS1), $(OBJS2), $(SUBDIR_OBJECT_LIST), $(TOP_INSTALL_DIR))
#else
ObjectListTargetSubDir($(TARGET), $(OBJS), $(SUBDIR_OBJECT_LIST), $(TOP_INSTALL_DIR))
#endif

/*
 * message
 */
/* MessageTarget($(MESSAGE_TARGET), $(MESSAGE_DEFINITION), $(BUILD_JAR)) */

/*
 * install library and header
 */
/* InstallHeaderTarget($(TOP_EXPORT_HDRS), $(TOP_EXPORT_HDRDIR)) */
#ifdef USE_OLDER_VERSION
InstallHeaderTarget($(LOCAL_EXPORT_HDRS1), $(LOCAL_EXPORT_HDRDIR))
InstallHeaderTarget($(LOCAL_EXPORT_HDRS2), $(LOCAL_EXPORT_HDRDIR))
InstallHeaderTarget($(LOCAL_EXPORT_HDRS3), $(LOCAL_EXPORT_HDRDIR))
#else
InstallHeaderTarget($(LOCAL_EXPORT_HDRS), $(LOCAL_EXPORT_HDRDIR))
#endif

/*
 * clean
 */
CleanTarget($(MESSAGE_TARGET) $(TARGET))
#ifdef USE_OLDER_VERSION
CleanTarget($(OBJS1))
CleanTarget($(OBJS2))
#endif

/*
 * no Makefile.h
 */
MAKEFILE_H =

/*
    Copyright (c) 1999, 2023 Ricoh Company, Ltd.
    All rights reserved.
*/
