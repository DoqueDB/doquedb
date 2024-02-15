// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TypeList.h -- 構文要素の種類
// 
// Copyright (c) 2000, 2002, 2007, 2023 Ricoh Company, Ltd.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// enumと型名文字列の両方に共通したクラスリスト
// TypeDefを#defineしてから呼ぶこと。
// Objectは書かない

TypeDef(StringValue)
TypeDef(IntegerValue)
TypeDef(DataValue)
TypeDef(Identifier)
TypeDef(Literal)
TypeDef(ColumnDefinition)
TypeDef(ColumnName)
TypeDef(ColumnNameList)
#ifdef OBSOLETE
TypeDef(CursorName)
#endif
TypeDef(DeleteStatement)
TypeDef(DerivedColumn)
#ifdef OBSOLETE
TypeDef(FromClause)
#endif
TypeDef(GroupByClause)
TypeDef(GroupingColumnReference)
TypeDef(GroupingColumnReferenceList)
TypeDef(HavingClause)
TypeDef(Hint)
TypeDef(HintElement)
TypeDef(HintElementList)
TypeDef(IndexDefinition)
TypeDef(InsertColumnsAndSource)
TypeDef(InsertStatement)
TypeDef(ItemReference)
#ifdef OBSOLETE
TypeDef(NamedColumnsJoin)
#endif
TypeDef(QualifiedJoin)
TypeDef(QueryExpression)
TypeDef(QueryOperator)
TypeDef(QueryTerm)
TypeDef(QuerySpecification)
TypeDef(SelectList)
TypeDef(SelectStatement)
TypeDef(SelectSubList)
TypeDef(SelectSubListList)
TypeDef(SortSpecification)
TypeDef(SortSpecificationList)
#ifdef OBSOLETE
TypeDef(SqlStatementList)
#endif
TypeDef(TableConstraintDefinition)
TypeDef(TableCorrelationSpec)
TypeDef(TableDefinition)
TypeDef(TableElementList)
TypeDef(TableExpression)
TypeDef(TablePrimary)
TypeDef(TableReferenceList)
TypeDef(UpdateSetClause)
TypeDef(UpdateSetClauseList)
TypeDef(UpdateStatement)
TypeDef(ValueExpression)
TypeDef(ValueExpressionList)
#ifdef OBSOLETE
TypeDef(WhereClause)
#endif
TypeDef(DropTableStatement)
TypeDef(AreaDefinition)
TypeDef(AreaElementList)
TypeDef(AreaDataDefinition)
TypeDef(AreaOption)
TypeDef(DropAreaStatement)
TypeDef(AlterTableAction)
TypeDef(AlterTableStatement)
TypeDef(AlterAreaAction)
TypeDef(AlterAreaStatement)
TypeDef(AlterDatabaseStatement)
TypeDef(AlterIndexAction)
TypeDef(AlterIndexStatement)
TypeDef(DropDatabaseStatement)
TypeDef(DropIndexStatement)
TypeDef(StartTransactionStatement)
TypeDef(SetTransactionStatement)
TypeDef(TransactionMode)
TypeDef(TransactionModeList)
TypeDef(TransactAccMode)
TypeDef(IsolationLevel)
TypeDef(CommitStatement)
TypeDef(RollbackStatement)
#ifndef NEED_INSTANCE
TypeDef(ObjectConnection)
TypeDef(ObjectSelection)
TypeDef(ObjectList)
#endif
TypeDef(AlterDatabaseAttribute)
TypeDef(AlterDatabaseAttributeList)
TypeDef(DatabaseCreateOption)
TypeDef(DatabaseCreateOptionList)
TypeDef(DatabasePathElement)
TypeDef(DatabasePathElementList)
TypeDef(OptionalAreaParameter)
TypeDef(DatabaseDefinition)
TypeDef(StartBackupStatement)
TypeDef(EndBackupStatement)
TypeDef(MountDatabaseStatement)
TypeDef(UnmountDatabaseStatement)
TypeDef(MoveDatabaseStatement)
TypeDef(OptionalAreaParameterList)
TypeDef(VerifyStatement)
TypeDef(IntegerArray)
TypeDef(SQLWrapper)
TypeDef(LikePredicate)
TypeDef(ColumnConstraintDefinition)
TypeDef(ColumnConstraintDefinitionList)
TypeDef(CrossJoin)
TypeDef(ExistsJoin)
TypeDef(InPredicate)
TypeDef(GroupingElementList)
TypeDef(ContainsPredicate)
#ifndef NEED_INSTANCE
TypeDef(ContainsOperand)
#endif
TypeDef(ContainsOperandList)
TypeDef(Expand)
#ifdef OBSOLETE
TypeDef(ExpandOrder)
#endif
TypeDef(LimitSpecification)
TypeDef(SyncStatement)
TypeDef(CheckpointStatement)
TypeDef(SimilarPredicate)
TypeDef(BulkSpecification)
#ifndef NEED_INSTANCE
TypeDef(BatchInsertStatement)
	// BatchInsertStatementはTypeのみの定義なのでAllType.hには追加しない
#endif
TypeDef(XA_CommitStatement)
TypeDef(XA_Identifier)
TypeDef(XA_EndStatement)
TypeDef(XA_ForgetStatement)
TypeDef(XA_PrepareStatement)
TypeDef(XA_RecoverStatement)
TypeDef(XA_RollbackStatement)
TypeDef(XA_StartStatement)
TypeDef(GrantStatement)
TypeDef(RevokeStatement)
TypeDef(IdentifierList)
	// ↓以下はTypeのみの定義なのでAllType.hには追加しない
#ifndef NEED_INSTANCE
TypeDef(TemporaryTableDefinition)
TypeDef(DropTemporaryTableStatement)
TypeDef(TemporaryInsertStatement)
TypeDef(TemporaryUpdateStatement)
TypeDef(TemporaryDeleteStatement)
#endif
	// ↑以上はTypeのみの定義なのでAllType.hには追加しない
TypeDef(ExplainOption)
TypeDef(ExplainStatement)
TypeDef(StartExplainStatement)
TypeDef(EndExplainStatement)
TypeDef(ContainsOption)
	// ↓以下はTypeのみの定義なのでAllType.hには追加しない
#ifndef NEED_INSTANCE
TypeDef(PrepareStatement)
TypeDef(ErasePrepareStatement)
TypeDef(ExecutePrepareStatement)
#endif
	// ↑以上はTypeのみの定義なのでAllType.hには追加しない
TypeDef(DisconnectStatement)
	// ↓以下はTypeのみの定義なのでAllType.hには追加しない
#ifndef NEED_INSTANCE
TypeDef(TemporaryIndexDefinition)
TypeDef(DropTemporaryIndexStatement)
#endif
	// ↑以上はTypeのみの定義なのでAllType.hには追加しない
TypeDef(DeclareStatement)
TypeDef(SelectTargetList)
TypeDef(VariableName)

TypeDef(CascadeDefinition)
TypeDef(AlterCascadeStatement)
TypeDef(DropCascadeStatement)
TypeDef(PartitionDefinition)
TypeDef(AlterPartitionStatement)
TypeDef(DropPartitionStatement)
TypeDef(FunctionDefinition)
TypeDef(DropFunctionStatement)
TypeDef(ParameterDeclaration)
TypeDef(ParameterDeclarationList)
TypeDef(RoutineBody)
TypeDef(ReturnsClause)
TypeDef(ReturnStatement)

//【注意】	新規に型を追加したときは AllType.h にも追加すること
//			既存の型を削除すると、既存のファイルが読めなくなる
//			新規の型は必ず末尾に追加すること

//
//	Copyright (c) 2000, 2002, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
