/*
 * Makefile.c --- Kernel/Plan/c
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

SUBDIRS =	   \
	AccessPlan \
	Candidate  \
	File	   \
	Interface  \
	Order	   \
	Predicate  \
	Relation   \
	Sql		   \	
	Scalar	   \
	Tree	   \
	Utility

/****************************************/
/* following variables MUST be defined  */

MODULE = Plan
TARGET_BASE = SyPlan
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
	AccessPlan.ol \
	Candidate.ol  \
	File.ol		  \
	Interface.ol  \
	Order.ol	  \
	Predicate.ol  \
	Relation.ol	  \
	Scalar.ol	  \
	Sql.ol		  \	
	Tree.ol		  \
	Utility.ol

/* headers created for message */
MESSAGE_HDRS =

/* headers installed in Kernel */
#ifdef USE_OLDER_VERSION
LOCAL_EXPORT_HDRS1 = \
/*	$(HDRDIR)/AssignNode_Default.h */\
	$(HDRDIR)/AssignNode_Identity.h \
	$(HDRDIR)/AssignNode_RowID.h \
	$(HDRDIR)/AutoLatch.h \
	$(HDRDIR)/Boolean.h \
	$(HDRDIR)/Collection.h \
	$(HDRDIR)/Collection_File.h \
	$(HDRDIR)/Collection_OnMemory.h \
	$(HDRDIR)/Collection_Sorted.h \
	$(HDRDIR)/Collection_SortedFile.h \
	$(HDRDIR)/Collection_Stream.h \
	$(HDRDIR)/ColumnInfo.h \
	$(HDRDIR)/ColumnNode_Get.h \
	$(HDRDIR)/ColumnNode_Put.h \
	$(HDRDIR)/ColumnNode_Put_Copy.h \
	$(HDRDIR)/ColumnNode_Put_Default.h \
	$(HDRDIR)/ColumnType.h \
	$(HDRDIR)/Configuration.h \
	$(HDRDIR)/ConstantNode.h \
	$(HDRDIR)/Cost.h \
/*	$(HDRDIR)/DateTimeNode.h */\
	$(HDRDIR)/Declaration.h \
/*	$(HDRDIR)/DerivedColumnNode.h */\
	$(HDRDIR)/Dyadic.h \
	$(HDRDIR)/FileInfo.h \
	$(HDRDIR)/FunctionInterface_Dyadic.h \
	$(HDRDIR)/FunctionInterface_Monadic.h \
	$(HDRDIR)/FunctionInterface_Monadic_Set.h \
	$(HDRDIR)/FunctionInterface_Nadic.h \
	$(HDRDIR)/FunctionInterface_Quadic.h \
	$(HDRDIR)/FunctionInterface_Triadic.h \
	$(HDRDIR)/FunctionNode_ArrayConstructor.h \
	$(HDRDIR)/FunctionNode_Avg.h \
	$(HDRDIR)/FunctionNode_Case.h \
	$(HDRDIR)/FunctionNode_Cast.h \
	$(HDRDIR)/FunctionNode_CharLength.h \
	$(HDRDIR)/FunctionNode_CheckNull.h \
	$(HDRDIR)/FunctionNode_Coalesce.h \
	$(HDRDIR)/FunctionNode_ContainsAnd.h \
	$(HDRDIR)/FunctionNode_ContainsAndNot.h \
	$(HDRDIR)/FunctionNode_ContainsOr.h \
	$(HDRDIR)/FunctionNode_ContainsSynonym.h \
	$(HDRDIR)/FunctionNode_ContainsWithin.h \
/*	$(HDRDIR)/FunctionNode_Copy.h */\
	$(HDRDIR)/FunctionNode_Count.h \
	$(HDRDIR)/FunctionNode_CurrentDatetime.h \
	$(HDRDIR)/FunctionNode_Distinct.h \
	$(HDRDIR)/FunctionNode_Dyadic_Numeric.h \
	$(HDRDIR)/FunctionNode_ElementReference.h \
	$(HDRDIR)/FunctionNode_ExpandSynonym.h \
	$(HDRDIR)/FunctionNode_FreeText.h \
	$(HDRDIR)/FunctionNode_FullTextLength.h \
	$(HDRDIR)/FunctionNode_Kwic_NoLanguage.h \
	$(HDRDIR)/FunctionNode_Kwic_WithLanguage.h \
	$(HDRDIR)/FunctionNode_Max.h \
	$(HDRDIR)/FunctionNode_Min.h \
	$(HDRDIR)/FunctionNode_Monadic_Numeric.h \
	$(HDRDIR)/FunctionNode_Normalize.h \
	$(HDRDIR)/FunctionNode_Overlay_NoLength.h \
	$(HDRDIR)/FunctionNode_Overlay_WithLength.h \
	$(HDRDIR)/FunctionNode_Pattern.h \
	$(HDRDIR)/FunctionNode_SpecialPattern.h \
	$(HDRDIR)/FunctionNode_StringConcatenate.h \
	$(HDRDIR)/FunctionNode_SubString_NoLength.h \
	$(HDRDIR)/FunctionNode_SubString_WithLength.h

LOCAL_EXPORT_HDRS2 = \
	$(HDRDIR)/FunctionNode_Sum.h \
	$(HDRDIR)/FunctionNode_Tf.h \
	$(HDRDIR)/FunctionNode_Weight.h \
	$(HDRDIR)/FunctionNode_Word.h \
	$(HDRDIR)/FunctionNode_WordCount.h \
	$(HDRDIR)/FunctionNode_WordList.h \
	$(HDRDIR)/FunctionNode_Wrapper.h \
	$(HDRDIR)/Joining.h \
	$(HDRDIR)/Kwic.h \
	$(HDRDIR)/LimitSpecification.h \
	$(HDRDIR)/Locator.h \
	$(HDRDIR)/LogData.h \
	$(HDRDIR)/Manager.h \
/*	$(HDRDIR)/Manipulation.h */\
	$(HDRDIR)/Module.h \
	$(HDRDIR)/Monadic.h \
	$(HDRDIR)/Nadic.h \
	$(HDRDIR)/Node.h \
	$(HDRDIR)/Object.h \
	$(HDRDIR)/ObjectPointer.h \
	$(HDRDIR)/ObjectSet.h \
	$(HDRDIR)/OperandInfo.h \
	$(HDRDIR)/OperandIterator.h \
	$(HDRDIR)/Operation.h \
	$(HDRDIR)/OperationInterface_Dyadic.h \
	$(HDRDIR)/OperationInterface_Monadic.h \
	$(HDRDIR)/OperationNode_LocatorAppend.h \
	$(HDRDIR)/OperationNode_LocatorReplace.h \
	$(HDRDIR)/OperationNode_LocatorTruncate.h \
	$(HDRDIR)/Overlay.h \
	$(HDRDIR)/PageUnit.h \
	$(HDRDIR)/ParameterNode.h \
	$(HDRDIR)/PredicateInterface.h \
/*	$(HDRDIR)/PredicateInterface_Dyadic_Predicate.h */\
	$(HDRDIR)/PredicateInterface_Dyadic_Row.h \
	$(HDRDIR)/PredicateInterface_Dyadic_Scalar.h \
	$(HDRDIR)/PredicateInterface_Monadic_Predicate.h \
/*	$(HDRDIR)/PredicateInterface_Monadic_Row.h */\
	$(HDRDIR)/PredicateInterface_Monadic_Scalar.h \
	$(HDRDIR)/PredicateInterface_Nadic_Predicate.h \
	$(HDRDIR)/PredicateInterface_Niladic.h \
/*	$(HDRDIR)/PredicateInterface_Triadic_Row.h */\
	$(HDRDIR)/PredicateInterface_Triadic_Scalar.h

LOCAL_EXPORT_HDRS3 = \
	$(HDRDIR)/PredicateNode_And.h \
	$(HDRDIR)/PredicateNode_Between.h \
	$(HDRDIR)/PredicateNode_Comparison_Row.h \
	$(HDRDIR)/PredicateNode_Comparison_Scalar.h \
	$(HDRDIR)/PredicateNode_Contains.h \
	$(HDRDIR)/PredicateNode_Exists.h \
/*	$(HDRDIR)/PredicateNode_In.h */\
	$(HDRDIR)/PredicateNode_Like.h \
	$(HDRDIR)/PredicateNode_Not.h \
	$(HDRDIR)/PredicateNode_Null.h \
	$(HDRDIR)/PredicateNode_Or.h \
/*	$(HDRDIR)/PredicateNode_Quantified_Comparison.h */ \
	$(HDRDIR)/PredicateNode_Similar.h \
	$(HDRDIR)/Quadic.h \
	$(HDRDIR)/RelationInfo.h \
	$(HDRDIR)/RelationInterface.h \
	$(HDRDIR)/RelationInterface_Dyadic.h \
	$(HDRDIR)/RelationInterface_Filter.h \
/*	$(HDRDIR)/RelationInterface_Merge.h */\
	$(HDRDIR)/RelationInterface_Monadic.h \
	$(HDRDIR)/RelationInterface_Nadic.h \
	$(HDRDIR)/RelationInterface_NestedLoop.h \
	$(HDRDIR)/RelationInterface_Niladic.h \
	$(HDRDIR)/RelationNode_Aggregation.h \
	$(HDRDIR)/RelationNode_CartesianProduct.h \
	$(HDRDIR)/RelationNode_Check.h \
	$(HDRDIR)/RelationNode_Check_Constraint.h \
	$(HDRDIR)/RelationNode_Check_Distinct.h \
	$(HDRDIR)/RelationNode_Collecting.h \
	$(HDRDIR)/RelationNode_Collecting_Limited.h \
	$(HDRDIR)/RelationNode_Distinct.h \
	$(HDRDIR)/RelationNode_Doubling.h \
/*	$(HDRDIR)/RelationNode_Except_All.h */\
/*	$(HDRDIR)/RelationNode_Except_Distinct.h */\
	$(HDRDIR)/RelationNode_ExistsJoin.h \
	$(HDRDIR)/RelationNode_ExistsThetaJoin.h \
	$(HDRDIR)/RelationNode_Expand.h \
	$(HDRDIR)/RelationNode_Grouping.h \
/*	$(HDRDIR)/RelationNode_HashJoin.h */\
/*	$(HDRDIR)/RelationNode_IndexJoin.h */\
	$(HDRDIR)/RelationNode_Input.h \
/*	$(HDRDIR)/RelationNode_Intersect_All.h */\
/*	$(HDRDIR)/RelationNode_Intersect_Distinct.h */\
	$(HDRDIR)/RelationNode_Limit.h \
	$(HDRDIR)/RelationNode_Lock.h \
	$(HDRDIR)/RelationNode_Lock_Delete.h \
	$(HDRDIR)/RelationNode_Lock_Insert.h \
	$(HDRDIR)/RelationNode_Lock_Update.h \
	$(HDRDIR)/RelationNode_Log.h \
	$(HDRDIR)/RelationNode_Log_Delete.h \
	$(HDRDIR)/RelationNode_Log_Delete_Undo.h \
	$(HDRDIR)/RelationNode_Log_Insert.h \
	$(HDRDIR)/RelationNode_Log_Update.h \
	$(HDRDIR)/RelationNode_Log_Update_Undo.h \
/*	$(HDRDIR)/RelationNode_MergeJoin.h */\
/*	$(HDRDIR)/RelationNode_MergeJoin_Distinct.h */\
	$(HDRDIR)/RelationNode_NeedCollecting.h \
/*	$(HDRDIR)/RelationNode_NonJoin.h */\
	$(HDRDIR)/RelationNode_NotExistsJoin.h \
	$(HDRDIR)/RelationNode_Ntimes.h

LOCAL_EXPORT_HDRS4 = \
	$(HDRDIR)/RelationNode_Once.h \
	$(HDRDIR)/RelationNode_Output.h \
	$(HDRDIR)/RelationNode_Output_Top.h \
	$(HDRDIR)/RelationNode_Projection.h \
	$(HDRDIR)/RelationNode_QualifiedJoin_Base.h \
	$(HDRDIR)/RelationNode_QualifiedJoin_Full.h \
	$(HDRDIR)/RelationNode_QualifiedJoin_Inner.h \
	$(HDRDIR)/RelationNode_QualifiedJoin_Left.h \
	$(HDRDIR)/RelationNode_QualifiedJoin_Right.h \
	$(HDRDIR)/RelationNode_SimpleAggregation.h \
	$(HDRDIR)/RelationNode_Sort.h \
	$(HDRDIR)/RelationNode_Sort_Distinct.h \
	$(HDRDIR)/RelationNode_Table_Delete.h \
	$(HDRDIR)/RelationNode_Table_Get.h \
	$(HDRDIR)/RelationNode_Table_Import.h \
	$(HDRDIR)/RelationNode_Table_Insert.h \
	$(HDRDIR)/RelationNode_Table_Put.h \
	$(HDRDIR)/RelationNode_Table_Simple.h \
	$(HDRDIR)/RelationNode_Table_Update.h \
	$(HDRDIR)/RelationNode_ThetaJoin.h \
	$(HDRDIR)/RelationNode_Undo_Delete.h \
	$(HDRDIR)/RelationNode_Undo_Insert.h \
	$(HDRDIR)/RelationNode_Undo_Update.h \
	$(HDRDIR)/RelationNode_Union.h \
	$(HDRDIR)/RelationNode_UpdateData.h \
	$(HDRDIR)/RelationNode_VirtualTable.h \
	$(HDRDIR)/ResultInfo.h \
	$(HDRDIR)/RowInfo.h \
	$(HDRDIR)/RowNode.h \
	$(HDRDIR)/ScalarInterface.h \
	$(HDRDIR)/ScalarInterface_Niladic.h \
	$(HDRDIR)/SchemaInfo.h \
	$(HDRDIR)/SerialPage.h \
	$(HDRDIR)/SortSpecification.h \
	$(HDRDIR)/Status.h \
	$(HDRDIR)/StatusInfo.h \
	$(HDRDIR)/SubString.h \
	$(HDRDIR)/TableInfo.h \
	$(HDRDIR)/Triadic.h \
	$(HDRDIR)/Tuple.h \
	$(HDRDIR)/TypeDef.h \
	$(HDRDIR)/UndoLog.h \
	$(HDRDIR)/Variable.h

LOCAL_EXPORT_HDRS = \
	$(LOCAL_EXPORT_HDRS1) \
	$(LOCAL_EXPORT_HDRS2) \
	$(LOCAL_EXPORT_HDRS3) \
	$(LOCAL_EXPORT_HDRS4)

#else

LOCAL_EXPORT_HDRS = \
	$(HDRDIR)/Declaration.h	\
	$(HDRDIR)/Manager.h \
	$(HDRDIR)/Module.h

#endif

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(MESSAGE_HDRS)

#ifdef USE_OLDER_VERSION
HDRS = \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS) \
	$(HDRDIR)/AssignNode_ObjectID.h \
	$(HDRDIR)/BulkExecutor.h \
	$(HDRDIR)/BulkFile.h \
	$(HDRDIR)/BulkParameter.h \
	$(HDRDIR)/BulkParser.h \
	$(HDRDIR)/BulkSeparator.h \
	$(HDRDIR)/BulkWriter.h \
	$(HDRDIR)/Collection_Bulk.h \
	$(HDRDIR)/Collection_Distinct.h \
	$(HDRDIR)/Collection_Map.h \
	$(HDRDIR)/Collection_Partitioning.h \
	$(HDRDIR)/FakeError.h \
	$(HDRDIR)/FieldInfo.h \
	$(HDRDIR)/FieldNode.h \
	$(HDRDIR)/FileAccess.h \
	$(HDRDIR)/FileAccess_Check.h \
	$(HDRDIR)/FileAccess_Delete.h \
	$(HDRDIR)/FileAccess_Except.h \
	$(HDRDIR)/FileAccess_Fetch.h \
	$(HDRDIR)/FileAccess_FetchForSearch.h \
	$(HDRDIR)/FileAccess_File.h \
	$(HDRDIR)/FileAccess_Get.h \
	$(HDRDIR)/FileAccess_GetLocator.h \
	$(HDRDIR)/FileAccess_Insert.h \
	$(HDRDIR)/FileAccess_Lock.h \
	$(HDRDIR)/FileAccess_Merge.h \
	$(HDRDIR)/FileAccess_Put.h \
	$(HDRDIR)/FileAccess_Scan.h \
	$(HDRDIR)/FileAccess_Search.h \
	$(HDRDIR)/FileAccess_SearchContains.h \
	$(HDRDIR)/FileAccess_UndoExpunge.h \
	$(HDRDIR)/FileAccess_UndoUpdate.h \
	$(HDRDIR)/FileAccess_Union.h \
	$(HDRDIR)/FileAccess_Update.h \
	$(HDRDIR)/FileAccessPlan.h \
	$(HDRDIR)/FileAccessPlan_Check.h \
	$(HDRDIR)/FileAccessPlan_Delete.h \
	$(HDRDIR)/FileAccessPlan_Import.h \
	$(HDRDIR)/FileAccessPlan_Insert.h \
	$(HDRDIR)/FileAccessPlan_Join.h \
	$(HDRDIR)/FileAccessPlan_Get.h \
	$(HDRDIR)/FileAccessPlan_Put.h \
	$(HDRDIR)/FileAccessPlan_Retrieve.h \
	$(HDRDIR)/FileAccessPlan_Scan.h \
	$(HDRDIR)/FileAccessPlan_Search.h \
	$(HDRDIR)/FileAccessPlan_Simple.h \
	$(HDRDIR)/FileAccessPlan_Undo.h \
	$(HDRDIR)/FileAccessPlan_UndoExpunge.h \
	$(HDRDIR)/FileAccessPlan_UndoUpdate.h \
	$(HDRDIR)/FileAccessPlan_Undo_Delete.h \
	$(HDRDIR)/FileAccessPlan_Undo_Insert.h \
	$(HDRDIR)/FileAccessPlan_Undo_Update.h \
	$(HDRDIR)/FileAccessPlan_Update.h \
	$(HDRDIR)/FunctionInfo.h \
	$(HDRDIR)/PredicateInterface_IndexFile.h \
	$(HDRDIR)/PredicateNode_In_File.h \
	$(HDRDIR)/PredicateNode_Not_File.h \
	$(HDRDIR)/RelationNode_Collecting_PartitioningLimited.h \
	$(HDRDIR)/RelationNode_OuterJoin.h \
	$(HDRDIR)/RelationNode_Table_Retrieve.h

#else

HDRS = \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS)

#endif

MESSAGE_OBJS =

#ifdef USE_OLDER_VERSION
OBJS1 = \
	$(MESSAGE_OBJS) \
/*	AssignNode_Default$O */\
	AssignNode_Identity$O \
	AssignNode_ObjectID$O \
	AssignNode_RowID$O \
	AutoLatch$O \
	Boolean$O \
	BulkExecutor$O \
	BulkFile$O \
	BulkParameter$O \
	BulkParser$O \
	BulkSeparator$O \
	BulkWriter$O \
	Collection$O \
	Collection_Bulk$O \
	Collection_Distinct$O \
	Collection_File$O \
	Collection_Map$O \
	Collection_OnMemory$O \
	Collection_Partitioning$O \
	Collection_Sorted$O \
	Collection_SortedFile$O \
	Collection_Stream$O \
	ColumnInfo$O \
	ColumnNode_Get$O \
	ColumnNode_Put$O \
	ColumnNode_Put_Copy$O \
	ColumnNode_Put_Default$O \
	ColumnType$O \
	Configuration$O \
	ConstantNode$O \
	Cost$O \
/*	DateTimeNode$O */\
/*	DerivedColumnNode$O */\
	FieldInfo$O \
	FieldNode$O \
	FileAccess$O \
	FileAccess_Check$O \
	FileAccess_Delete$O \
	FileAccess_Except$O \
	FileAccess_Fetch$O \
	FileAccess_FetchForSearch$O \
	FileAccess_File$O \
	FileAccess_Get$O \
	FileAccess_GetLocator$O \
	FileAccess_Insert$O \
	FileAccess_Lock$O \
	FileAccess_Merge$O \
	FileAccess_Put$O \
	FileAccess_Scan$O \
	FileAccess_Search$O \
	FileAccess_SearchContains$O \
	FileAccess_UndoExpunge$O \
	FileAccess_UndoUpdate$O \
	FileAccess_Union$O \
	FileAccess_Update$O

OBJS2 = \
	FileAccessPlan$O \
	FileAccessPlan_Check$O \
	FileAccessPlan_Delete$O \
	FileAccessPlan_Import$O \
	FileAccessPlan_Insert$O \
	FileAccessPlan_Join$O \
	FileAccessPlan_Get$O \
	FileAccessPlan_Put$O \
	FileAccessPlan_Retrieve$O \
	FileAccessPlan_Scan$O \
	FileAccessPlan_Search$O \
	FileAccessPlan_Simple$O \
	FileAccessPlan_Undo$O \
	FileAccessPlan_UndoExpunge$O \
	FileAccessPlan_UndoUpdate$O \
	FileAccessPlan_Undo_Delete$O \
	FileAccessPlan_Undo_Insert$O \
	FileAccessPlan_Undo_Update$O \
	FileAccessPlan_Update$O \
	FileInfo$O \
	FunctionInfo$O \
	FunctionInterface_Dyadic$O \
	FunctionInterface_Monadic$O \
	FunctionInterface_Monadic_Set$O \
	FunctionInterface_Nadic$O \
	FunctionInterface_Quadic$O \
	FunctionInterface_Triadic$O \
	FunctionNode_ArrayConstructor$O \
	FunctionNode_Avg$O \
	FunctionNode_Case$O \
	FunctionNode_Cast$O \
	FunctionNode_CharLength$O \
	FunctionNode_CheckNull$O \
	FunctionNode_Coalesce$O \
	FunctionNode_ContainsAnd$O \
	FunctionNode_ContainsAndNot$O \
	FunctionNode_ContainsOr$O \
	FunctionNode_ContainsSynonym$O \
	FunctionNode_ContainsWithin$O \
/*	FunctionNode_Copy$O */\
	FunctionNode_Count$O \
	FunctionNode_CurrentDatetime$O \
	FunctionNode_Distinct$O \
	FunctionNode_Dyadic_Numeric$O \
	FunctionNode_ElementReference$O \
	FunctionNode_ExpandSynonym$O \
	FunctionNode_FreeText$O \
	FunctionNode_FullTextLength$O \
	FunctionNode_Kwic_NoLanguage$O \
	FunctionNode_Kwic_WithLanguage$O \
	FunctionNode_Max$O \
	FunctionNode_Min$O \
	FunctionNode_Monadic_Numeric$O \
	FunctionNode_Normalize$O \
	FunctionNode_Overlay_NoLength$O \
	FunctionNode_Overlay_WithLength$O \
	FunctionNode_Pattern$O \
	FunctionNode_SpecialPattern$O \
	FunctionNode_StringConcatenate$O \
	FunctionNode_SubString_NoLength$O \
	FunctionNode_SubString_WithLength$O \
	FunctionNode_Sum$O \
	FunctionNode_Tf$O \
/*	FunctionNode_User$O */\
	FunctionNode_Weight$O \
	FunctionNode_Word$O \
	FunctionNode_WordCount$O \
	FunctionNode_WordList$O \
	FunctionNode_Wrapper$O

OBJS3 = \
	Joining$O \
	Kwic$O \
	LimitSpecification$O \
	Locator$O \
	LogData$O \
	Manager$O \
/*	Manipulation$O */\
	Node$O \
	Object$O \
	ObjectSet$O \
	Operation$O \
	OperationInterface_Dyadic$O \
	OperationInterface_Monadic$O \
	OperationNode_LocatorAppend$O \
	OperationNode_LocatorReplace$O \
	OperationNode_LocatorTruncate$O \
	Overlay$O \
	PageUnit$O \
	ParameterNode$O \
	PredicateInterface$O \
/*	PredicateInterface_Dyadic_Predicate$O */\
	PredicateInterface_Dyadic_Row$O \
	PredicateInterface_Dyadic_Scalar$O \
	PredicateInterface_IndexFile$O \
	PredicateInterface_Monadic_Predicate$O \
/*	PredicateInterface_Monadic_Row$O */\
	PredicateInterface_Monadic_Scalar$O \
	PredicateInterface_Nadic_Predicate$O \
	PredicateInterface_Niladic$O \
/*	PredicateInterface_Triadic_Row$O */\
	PredicateInterface_Triadic_Scalar$O \
	PredicateNode_And$O \
	PredicateNode_Between$O \
	PredicateNode_Comparison_Row$O \
	PredicateNode_Comparison_Scalar$O \
	PredicateNode_Contains$O \
	PredicateNode_Exists$O \
/*	PredicateNode_In$O */\
	PredicateNode_In_File$O \
	PredicateNode_Like$O \
	PredicateNode_Not$O \
	PredicateNode_Not_File$O \
	PredicateNode_Null$O \
	PredicateNode_Or$O \
/*	PredicateNode_Quantified_Comparison$O */ \
	PredicateNode_Similar$O

OBJS4 = \
	RelationInfo$O \
	RelationInterface$O \
	RelationInterface_Dyadic$O \
	RelationInterface_Filter$O \
/*	RelationInterface_Merge$O */\
	RelationInterface_Monadic$O \
	RelationInterface_Nadic$O \
	RelationInterface_NestedLoop$O \
	RelationNode_Aggregation$O \
	RelationNode_CartesianProduct$O \
	RelationNode_Check$O \
	RelationNode_Check_Constraint$O \
	RelationNode_Check_Distinct$O \
	RelationNode_Collecting$O \
	RelationNode_Collecting_Limited$O \
	RelationNode_Collecting_PartitioningLimited$O \
	RelationNode_Distinct$O \
	RelationNode_Doubling$O \
/*	RelationNode_Except_All$O */\
/*	RelationNode_Except_Distinct$O */\
	RelationNode_ExistsJoin$O \
	RelationNode_ExistsThetaJoin$O \
	RelationNode_Expand$O \
	RelationNode_Grouping$O \
/*	RelationNode_HashJoin$O */\
/*	RelationNode_IndexJoin$O */\
	RelationNode_Input$O \
/*	RelationNode_Intersect_All$O */\
/*	RelationNode_Intersect_Distinct$O */\
	RelationNode_Limit$O \
	RelationNode_Lock$O \
	RelationNode_Lock_Delete$O \
	RelationNode_Lock_Insert$O \
	RelationNode_Lock_Update$O \
	RelationNode_Log$O \
	RelationNode_Log_Delete$O \
	RelationNode_Log_Delete_Undo$O \
	RelationNode_Log_Insert$O \
	RelationNode_Log_Update$O \
	RelationNode_Log_Update_Undo$O \
/*	RelationNode_MergeJoin$O */\
/*	RelationNode_MergeJoin_Distinct$O */\
	RelationNode_NeedCollecting$O \
/*	RelationNode_NonJoin$O */\
	RelationNode_NotExistsJoin$O \
	RelationNode_Ntimes$O \
	RelationNode_Once$O \
	RelationNode_OuterJoin$O \
	RelationNode_Output$O \
	RelationNode_Output_Top$O \
	RelationNode_Projection$O \
	RelationNode_QualifiedJoin_Base$O \
	RelationNode_QualifiedJoin_Full$O \
	RelationNode_QualifiedJoin_Inner$O \
	RelationNode_QualifiedJoin_Left$O \
	RelationNode_QualifiedJoin_Right$O \
	RelationNode_SimpleAggregation$O \
	RelationNode_Sort$O \
	RelationNode_Sort_Distinct$O \
	RelationNode_Table_Delete$O \
	RelationNode_Table_Get$O \
	RelationNode_Table_Import$O \
	RelationNode_Table_Insert$O \
	RelationNode_Table_Put$O \
	RelationNode_Table_Retrieve$O \
	RelationNode_Table_Simple$O \
	RelationNode_Table_Update$O \
	RelationNode_ThetaJoin$O \
	RelationNode_Undo_Delete$O \
	RelationNode_Undo_Insert$O \
	RelationNode_Undo_Update$O \
	RelationNode_Union$O \
	RelationNode_UpdateData$O \
	RelationNode_VirtualTable$O

OBJS5 = \
	ResultInfo$O \
	RowInfo$O \
	RowNode$O \
	ScalarInterface$O \
/*	ScalarInterface_Niladic$O */\
	SerialPage$O \
	Status$O \
	StatusInfo$O \
	SortSpecification$O \
	SubString$O \
	TableInfo$O \
	Tuple$O \
	TypeDef$O \
	UndoLog$O \
	Variable$O

OBJS = \
	$(OBJS1) \
	$(OBJS2) \
	$(OBJS3) \
	$(OBJS4) \
	$(OBJS5)

#else

OBJS = \
	$(MESSAGE_OBJS) \
	Manager$O

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

ALLTARGETS = \
	objlist-r		  \
	$(MESSAGE_TARGET) \
	$(TARGET)

/*
 * all
 */
AllTarget($(ALLTARGETS))

#ifdef USE_OLDER_VERSION
ObjectListTarget5SubDir($(TARGET), $(OBJS1), $(OBJS2), $(OBJS3), $(OBJS4), $(OBJS5), $(SUBDIR_OBJECT_LIST), $(TOP_INSTALL_DIR))
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
InstallHeaderTarget($(LOCAL_EXPORT_HDRS4), $(LOCAL_EXPORT_HDRDIR))
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
CleanTarget($(OBJS3))
CleanTarget($(OBJS4))
CleanTarget($(OBJS5))
#else
CleanTarget($(OBJS))
#endif

#include "Makefile.h"

/*
  Copyright (c) 1999, 2023 Ricoh Company, Ltd.
  All rights reserved.
*/
