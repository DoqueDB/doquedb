// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SQLDispatchEntry.cpp -- SQL 文の種類からその SQL 文の実行の仕方に関する
//						   情報を取得するための関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2006, 2007, 2011, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Session";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Server/SQLDispatchEntry.h"

#include "Exception/Unexpected.h"
#include "Statement/Object.h"
#include "Statement/Type.h"

_SYDNEY_USING
_SYDNEY_SERVER_USING

namespace
{

const Server::SQLDispatchEntry tbl[] =
{
	//
	//	ObjectType,
	//	Target Module, Transaction Permission,
	//	OK in R/O transaction?, OK on R/O database?, OK on OFFLINE database?,
	//	OK in Slave dabase?,
	//	Logged in database log?, Logged in system log?,
	//	Privilege Category, Privilege Value
	//
	{
		Statement::ObjectType::QueryExpression,
		Module::Optimizer,	Permission::Any,
		Boolean::True,		Boolean::True,		Boolean::False,
		Boolean::True,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::Reference,	Common::Privilege::Reference::Select
	},
	/*
	{
		Statement::ObjectType::QuerySpecification,
		Module::Optimizer,	Permission::Any,
		Boolean::True,		Boolean::True,		Boolean::False,
		Boolean::True,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::Reference,	Common::Privilege::Reference::Select
	},
	{
		Statement::ObjectType::SelectStatement,
		Module::Optimizer,	Permission::Any,
		Boolean::True,		Boolean::True,		Boolean::False,
		Boolean::True,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::Reference,	Common::Privilege::Reference::Select
	},
	*/
	{
		Statement::ObjectType::InsertStatement,
		Module::Optimizer,	Permission::Any,
		Boolean::False,		Boolean::Unknown,		Boolean::False,
		Boolean::False,
		Boolean::True,		Boolean::False,
		Common::Privilege::Category::Data,	Common::Privilege::Data::Insert
	},
	{
		Statement::ObjectType::BatchInsertStatement,
		Module::Optimizer,	Permission::Any,
		Boolean::False,		Boolean::False,		Boolean::False,
		Boolean::False,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::Data,	Common::Privilege::Data::Insert
	},
	{
		Statement::ObjectType::TemporaryInsertStatement,
		Module::Optimizer,	Permission::Any,
		Boolean::False,		Boolean::True,		Boolean::False,
		Boolean::True,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::Reference,	Common::Privilege::Reference::TemporaryTable
	},
	{
		Statement::ObjectType::DeleteStatement,
		Module::Optimizer,	Permission::Any,
		Boolean::False,		Boolean::Unknown,		Boolean::False,
		Boolean::False,
		Boolean::True,		Boolean::False,
		Common::Privilege::Category::Data,	Common::Privilege::Data::Delete
	},
	{
		Statement::ObjectType::TemporaryDeleteStatement,
		Module::Optimizer,	Permission::Any,
		Boolean::False,		Boolean::True,		Boolean::False,
		Boolean::True,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::Reference,	Common::Privilege::Reference::TemporaryTable
	},
	{
		Statement::ObjectType::UpdateStatement,
		Module::Optimizer,	Permission::Any,
		Boolean::False,		Boolean::Unknown,		Boolean::False,
		Boolean::False,
		Boolean::True,		Boolean::False,
		Common::Privilege::Category::Data,	Common::Privilege::Data::Update
	},
	{
		Statement::ObjectType::TemporaryUpdateStatement,
		Module::Optimizer,	Permission::Any,
		Boolean::False,		Boolean::True,		Boolean::False,
		Boolean::True,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::Reference,	Common::Privilege::Reference::TemporaryTable
	},
	{
		Statement::ObjectType::ValueExpression,
		Module::Optimizer,	Permission::Any,
		Boolean::True,		Boolean::True,		Boolean::True,
		Boolean::True,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::Reference,	Common::Privilege::Reference::Select
	},
	{
		Statement::ObjectType::DatabaseDefinition,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::Unknown,	Boolean::Unknown,
		Boolean::False,
		Boolean::False,		Boolean::True,
		Common::Privilege::Category::SuperUser,	Common::Privilege::SuperUser::CreateDatabase
	},
	{
		Statement::ObjectType::TableDefinition,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::False,		Boolean::False,
		Boolean::False,
		Boolean::True,		Boolean::False,
		Common::Privilege::Category::Database,	Common::Privilege::Database::CreateTable
	},
	{
		Statement::ObjectType::TemporaryTableDefinition,
		Module::Schema,		Permission::Any,
		Boolean::False,		Boolean::True,		Boolean::False,
		Boolean::True,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::Reference,	Common::Privilege::Reference::TemporaryTable
	},
	{
		Statement::ObjectType::IndexDefinition,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::False,		Boolean::False,
		Boolean::False,
		Boolean::True,		Boolean::False,
		Common::Privilege::Category::Data,	Common::Privilege::Data::CreateIndex
	},
	{
		Statement::ObjectType::AreaDefinition,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::False,		Boolean::False,
		Boolean::False,
		Boolean::True,		Boolean::False,
		Common::Privilege::Category::Database,	Common::Privilege::Database::CreateArea
	},
	{
		Statement::ObjectType::AlterDatabaseStatement,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::True,		Boolean::True,
		Boolean::True,
		Boolean::False,		Boolean::True,
		Common::Privilege::Category::System,	Common::Privilege::System::AlterDatabase
	},
	{
		Statement::ObjectType::MoveDatabaseStatement,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::False,		Boolean::False,
		Boolean::False,
		Boolean::False,		Boolean::True,
		Common::Privilege::Category::System,	Common::Privilege::System::AlterDatabase
	},
	{
		Statement::ObjectType::AlterTableStatement,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::False,		Boolean::False,
		Boolean::False,
		Boolean::True,		Boolean::False,
		Common::Privilege::Category::Database,	Common::Privilege::Database::AlterTable
	},
	{
		Statement::ObjectType::AlterIndexStatement,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::False,		Boolean::False,
		Boolean::False,
		Boolean::True,		Boolean::False,
		Common::Privilege::Category::Database,	Common::Privilege::Database::AlterIndex
	},
	{
		Statement::ObjectType::AlterAreaStatement,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::False,		Boolean::False,
		Boolean::False,
		Boolean::True,		Boolean::False,
		Common::Privilege::Category::Database,	Common::Privilege::Database::AlterArea
	},
	{
		Statement::ObjectType::DropDatabaseStatement,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::False,		Boolean::False,
		Boolean::True,
		Boolean::False,		Boolean::True,
		Common::Privilege::Category::System,	Common::Privilege::System::DropDatabase
	},
	{
		Statement::ObjectType::DropTableStatement,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::False,		Boolean::False,
		Boolean::False,
		Boolean::True,		Boolean::False,
		Common::Privilege::Category::Database,	Common::Privilege::Database::DropTable
	},
	{
		Statement::ObjectType::DropTemporaryTableStatement,
		Module::Schema,		Permission::Any,
		Boolean::False,		Boolean::True,		Boolean::False,
		Boolean::True,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::Reference,	Common::Privilege::Reference::TemporaryTable
	},
	{
		Statement::ObjectType::DropIndexStatement,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::False,		Boolean::False,
		Boolean::False,
		Boolean::True,		Boolean::False,
		Common::Privilege::Category::Data,	Common::Privilege::Data::DropIndex
	},
	{
		Statement::ObjectType::DropAreaStatement,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::False,		Boolean::False,
		Boolean::False,
		Boolean::True,		Boolean::False,
		Common::Privilege::Category::Database,	Common::Privilege::Database::DropArea
	},
	{
		Statement::ObjectType::MountDatabaseStatement,
		Module::Admin,		Permission::Implicitly,
		Boolean::False,		Boolean::Unknown,	Boolean::Unknown,
		Boolean::True,
		Boolean::False,		Boolean::True,
		Common::Privilege::Category::SuperUser,	Common::Privilege::SuperUser::Mount
	},
	{
		Statement::ObjectType::UnmountDatabaseStatement,
		Module::Admin,		Permission::Implicitly,
		Boolean::False,		Boolean::True,		Boolean::True,
		Boolean::False,
		Boolean::True,		Boolean::True,
		Common::Privilege::Category::System,	Common::Privilege::System::Unmount
	},
	{
		Statement::ObjectType::StartBackupStatement,
		Module::Admin,		Permission::Explicitly,
		Boolean::True,		Boolean::True,		Boolean::False,
		Boolean::True,
		Boolean::True,		Boolean::False,
		Common::Privilege::Category::System,	Common::Privilege::System::Backup
	},
	{
		Statement::ObjectType::EndBackupStatement,
		Module::Admin,		Permission::Explicitly,
		Boolean::True,		Boolean::True,		Boolean::False,
		Boolean::True,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::System,	Common::Privilege::System::Backup
	},
	{
		Statement::ObjectType::VerifyStatement,
		Module::Admin,		Permission::Any,
		Boolean::True,		Boolean::True,		Boolean::False,
		Boolean::True,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::Database,	Common::Privilege::Database::Verify
	},
	{
		Statement::ObjectType::StartTransactionStatement,
		Module::Server,		Permission::Never,
		Boolean::False,		Boolean::Unknown,	Boolean::False,
		Boolean::False,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::Reference,	Common::Privilege::Reference::Transaction
	},
	{
		Statement::ObjectType::SetTransactionStatement,
		Module::Server,		Permission::Never,
		Boolean::False,		Boolean::Unknown,	Boolean::False,
		Boolean::False,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::Reference,	Common::Privilege::Reference::Transaction
	},
	{
		Statement::ObjectType::CommitStatement,
		Module::Server,		Permission::Explicitly,
		Boolean::False,		Boolean::True,		Boolean::False,
		Boolean::False,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::Reference,	Common::Privilege::Reference::Transaction
	},
	{
		Statement::ObjectType::RollbackStatement,
		Module::Server,		Permission::Explicitly,
		Boolean::False,		Boolean::True,		Boolean::False,
		Boolean::False,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::Reference,	Common::Privilege::Reference::Transaction
	},
	{
		Statement::ObjectType::GrantStatement,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::False,		Boolean::False,
		Boolean::False,
		Boolean::True,		Boolean::False,
		Common::Privilege::Category::System,	Common::Privilege::System::Grant
	},
	{
		Statement::ObjectType::RevokeStatement,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::False,		Boolean::False,
		Boolean::False,
		Boolean::True,		Boolean::False,
		Common::Privilege::Category::System,	Common::Privilege::System::Revoke
	},
	{
		Statement::ObjectType::ExplainStatement,
		Module::Server,		Permission::Any,
		Boolean::True,		Boolean::True,		Boolean::False,
		Boolean::True,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::Reference,	Common::Privilege::Reference::Select
	},
	{
		Statement::ObjectType::StartExplainStatement,
		Module::Server,		Permission::Any,
		Boolean::True,		Boolean::True,		Boolean::False,
		Boolean::True,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::Reference,	Common::Privilege::Reference::Select
	},
	{
		Statement::ObjectType::EndExplainStatement,
		Module::Server,		Permission::Any,
		Boolean::True,		Boolean::True,		Boolean::False,
		Boolean::True,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::Reference,	Common::Privilege::Reference::Select
	},
	{
		Statement::ObjectType::TemporaryIndexDefinition,
		Module::Schema,		Permission::Any,
		Boolean::False,		Boolean::True,		Boolean::False,
		Boolean::True,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::Reference,	Common::Privilege::Reference::TemporaryTable
	},
	{
		Statement::ObjectType::DropTemporaryIndexStatement,
		Module::Schema,		Permission::Any,
		Boolean::False,		Boolean::True,		Boolean::False,
		Boolean::True,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::Reference,	Common::Privilege::Reference::TemporaryTable
	},
	{
		Statement::ObjectType::DeclareStatement,
		Module::Optimizer,	Permission::Any,
		Boolean::True,		Boolean::True,		Boolean::True,
		Boolean::True,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::Reference,	Common::Privilege::Reference::TemporaryTable
	},
	{
		Statement::ObjectType::CascadeDefinition,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::False,		Boolean::False,
		Boolean::False,
		Boolean::True,		Boolean::False,
		Common::Privilege::Category::Database,	Common::Privilege::Database::CreateCascade
	},
	{
		Statement::ObjectType::PartitionDefinition,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::False,		Boolean::False,
		Boolean::False,
		Boolean::True,		Boolean::False,
		Common::Privilege::Category::Database,	Common::Privilege::Database::CreatePartition
	},
	{
		Statement::ObjectType::FunctionDefinition,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::False,		Boolean::False,
		Boolean::False,
		Boolean::False,		Boolean::False,
		Common::Privilege::Category::Data,	Common::Privilege::Data::CreateFunction
	},
	{
		Statement::ObjectType::AlterCascadeStatement,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::False,		Boolean::False,
		Boolean::False,
		Boolean::True,		Boolean::False,
		Common::Privilege::Category::Database,	Common::Privilege::Database::AlterCascade
	},
	{
		Statement::ObjectType::AlterPartitionStatement,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::False,		Boolean::False,
		Boolean::False,
		Boolean::True,		Boolean::False,
		Common::Privilege::Category::Database,	Common::Privilege::Database::AlterPartition
	},
	{
		Statement::ObjectType::DropCascadeStatement,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::False,		Boolean::False,
		Boolean::False,
		Boolean::True,		Boolean::False,
		Common::Privilege::Category::Database,	Common::Privilege::Database::DropCascade
	},
	{
		Statement::ObjectType::DropPartitionStatement,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::False,		Boolean::False,
		Boolean::False,
		Boolean::True,		Boolean::False,
		Common::Privilege::Category::Database,	Common::Privilege::Database::DropPartition
	},
	{
		Statement::ObjectType::DropFunctionStatement,
		Module::Schema,		Permission::Implicitly,
		Boolean::False,		Boolean::False,		Boolean::False,
		Boolean::False,
		Boolean::True,		Boolean::False,
		Common::Privilege::Category::Data,	Common::Privilege::Data::DropFunction
	}
};

}

//	FUNCTION public
//	Server::SQLDispatchTable::getEntry --
//		指定された SQL 文に関するエントリを探す
//
//	NOTES
//
//	ARGUMENTS
//		int iType_
//			この SQL 文に関するエントリを探す
//
//	RETURN
//		エントリのリファレンス
//
//	EXCEPTIONS

const SQLDispatchEntry&
SQLDispatchTable::getEntry(int iType_)
{
	unsigned int i = 0;
	const unsigned int n = sizeof(tbl) / sizeof(tbl[0]);

	do {
		if (tbl[i].m_iID == iType_)
			return tbl[i];
	} while (++i < n) ;

	_SYDNEY_THROW0(Exception::Unexpected);
}

//	FUNCTION public
//	Server::SQLDispatchEntry::isXATransactionNeeded --
//		暗黙のトランザクション実行時に分散トランザクションの発行が必要かどうか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Boolean::True
//			必要
//		Boolean::False
//			必要なし
//
//	EXCEPTIONS

Boolean::Value
SQLDispatchEntry::isXATransactionNeeded() const
{
	// 分散トランザクションで実行可能なものは
	// select、insert、delete、update、verify のみ
	// そのうち、暗黙のトランザクション実行時に
	// 分散トランザクションを実行する必要があるのは、
	// insert、delete、update のみ
	
	return (m_eModule == Module::Optimizer &&
			_executableInsideReadOnlyTransaction == Boolean::False)
		? Boolean::True : Boolean::False;
}

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2006, 2007, 2011, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
