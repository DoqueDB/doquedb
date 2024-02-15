// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MountDatabaseStatement.cpp --
// 
// Copyright (c) 2000, 2002, 2004, 2009, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Statement";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Statement/DatabaseCreateOptionList.h"
#include "Statement/Identifier.h"
#include "Statement/IntegerValue.h"
#include "Statement/Literal.h"
#include "Statement/MountDatabaseStatement.h"
#include "Statement/OptionalAreaParameter.h"
#include "Statement/Type.h"

#include "Common/Assert.h"

#include "ModAutoPointer.h"
#include "ModUnicodeOstrStream.h"

#if 0
#include "Analysis/MountDatabaseStatement.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace
{

enum
{
	f_Name,
	f_List,
	f_Area,
	f_UsingSnapshot,
	f_WithRecovery,
	f_DiscardLogicalLog,
	f_MasterUrl,
	f__end_index
};

}

//	FUNCTION public
//	Statement::MountDatabaseStatement::MountDatabaseStatement --
//		コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Identifier*	identifier
//			マウントするデータベースの識別子
//		Statement::DatabaseCreateOptionList*	optionList
//			マウントするデータベースの生成オプション
//		Statement::OptionalAreaParameter*	areaParameter
//			マウントするデータベースのエリア指定
//		bool			usingSnapshot
//			USING SNAPSHOT の指定の有無
//		bool			withRecovery
//			WITH RECOVER の指定の有無
//		bool			discardLogicalLog
//			DISCARD LOGICALLOG の指定の有無
//
//	RETURN
//		なし
//
//	EXCEPTIONS

MountDatabaseStatement::MountDatabaseStatement(
	Identifier* identifier,
	DatabaseCreateOptionList* optionList,
	OptionalAreaParameter* areaParameter,
	bool usingSnapshot, bool withRecovery, bool discardLogicalLog)
	: ObjectConnection(
		ObjectType::MountDatabaseStatement, f__end_index, Object::System)
{
	setDatabaseName(identifier);
	setOptionList(optionList);
	setAreaParameter(areaParameter);
	setUsingSnapshot(usingSnapshot);
	setWithRecovery(withRecovery);
	setDiscardLogicalLog(discardLogicalLog);
}

//	FUNCTION public
//	Statement::MountDatabaseStatement::getDatabaseName --
//		マウントするデータベースの識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた識別子
//
//	EXCEPTIONS
//		なし

Identifier*
MountDatabaseStatement::getDatabaseName() const
{
	return static_cast<Identifier*>(getObject(f_Name));
}

//	FUNCTION private
//	Statement::MountDatabaseStatement::setDatabaseName --
//		マウントするデータベースの識別子を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Identifier*	p
//			設定するデータベースの識別子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
MountDatabaseStatement::setDatabaseName(Identifier* p)
{
	setObject(f_Name, p);
}

//	FUNCTION public
//	Statement::MountDatabaseStatement::getOptionList --
//		マウントするデータベースの生成オプションを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた生成オプション
//
//	EXCEPTIONS
//		なし

DatabaseCreateOptionList*
MountDatabaseStatement::getOptionList() const
{
	return static_cast<DatabaseCreateOptionList*>(getObject(f_List));
}

//	FUNCTION private
//	Statement::MountDatabaseStatement::setOptionList --
//		マウントするデータベースの生成オプションを設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::DatabaseCreateOptionList*	p
//			設定するデータベースの生成オプション
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
MountDatabaseStatement::setOptionList(DatabaseCreateOptionList* p)
{
	setObject(f_List, p);
}

//	FUNCTION public
//	Statement::MountDatabaseStatement::getAreaParameter --
//		マウントするデータベースのエリア指定を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたエリア指定
//
//	EXCEPTIONS
//		なし

OptionalAreaParameter*
MountDatabaseStatement::getAreaParameter() const
{
	return static_cast<OptionalAreaParameter*>(getObject(f_Area));
}

//	FUNCTION private
//	Statement::MountDatabaseStatement::setAreaParameter --
//		マウントするデータベースのエリア指定を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::OptionalAreaParameter*	p
//			設定するデータベースのエリア指定
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
MountDatabaseStatement::setAreaParameter(OptionalAreaParameter* p)
{
	setObject(f_Area, p);
}

//	FUNCTION public
//	Statement::MountDatabaseStatement::isUsingSnapshot --
//		マウントするデータベースの USING SNAPSHOT の指定の有無を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた指定の有無
//
//	EXCEPTIONS
//		なし

bool
MountDatabaseStatement::isUsingSnapshot() const
{
	const IntegerValue* v =
		static_cast<IntegerValue*>(getObject(f_UsingSnapshot));
	; _SYDNEY_ASSERT(v);
	return v->getValue();
}

//	FUNCTION private
//	Statement::MountDatabaseStatement::setUsingSnapshot --
//		マウントするデータベースの USING SNAPSHOT の指定の有無を設定する
//
//	NOTES
//
//	ARGUMENTS
//		bool		v
//			設定する指定の有無
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
MountDatabaseStatement::setUsingSnapshot(bool v)
{
	ModAutoPointer<IntegerValue> p(new IntegerValue(v));
	; _SYDNEY_ASSERT(p.isOwner());
	setObject(f_UsingSnapshot, p.get()), p.release();
}

//	FUNCTION public
//	Statement::MountDatabaseStatement::isWithRecovery --
//		マウントするデータベースの WITH RECOVERY の指定の有無を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた指定の有無
//
//	EXCEPTIONS
//		なし

bool
MountDatabaseStatement::isWithRecovery() const
{
	const IntegerValue* v =
		static_cast<IntegerValue*>(getObject(f_WithRecovery));
	; _SYDNEY_ASSERT(v);
	return v->getValue();
}

//	FUNCTION private
//	Statement::MountDatabaseStatement::setWithRecovery --
//		マウントするデータベースの WITH RECOVERY の指定の有無を設定する
//
//	NOTES
//
//	ARGUMENTS
//		bool		v
//			設定する指定の有無
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
MountDatabaseStatement::setWithRecovery(bool v)
{
	ModAutoPointer<IntegerValue> p(new IntegerValue(v));
	; _SYDNEY_ASSERT(p.isOwner());
	setObject(f_WithRecovery, p.get()), p.release();
}

//	FUNCTION public
//	Statement::MountDatabaseStatement::isDiscardLogicalLog --
//		マウントするデータベースの DISCARD LOGICALLOG の指定の有無を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた指定の有無
//
//	EXCEPTIONS
//		なし

bool
MountDatabaseStatement::isDiscardLogicalLog() const
{
	const IntegerValue* v =
		static_cast<IntegerValue*>(getObject(f_DiscardLogicalLog));
	; _SYDNEY_ASSERT(v);
	return v->getValue();
}

//	FUNCTION private
//	Statement::MountDatabaseStatement::setDiscardLogicalLog --
//		マウントするデータベースの DISCARD LOGICALLOG の指定の有無を設定する
//
//	NOTES
//
//	ARGUMENTS
//		bool		v
//			設定する指定の有無
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
MountDatabaseStatement::setDiscardLogicalLog(bool v)
{
	ModAutoPointer<IntegerValue> p(new IntegerValue(v));
	; _SYDNEY_ASSERT(p.isOwner());
	setObject(f_DiscardLogicalLog, p.get()), p.release();
}

//	FUNCTION public
//	Statement::MountDatabaseStatement::getMasterUrl --
//		マスターデータベースのURLを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		マスターデータベースのURL
//
//	EXCEPTIONS
//		なし

Literal*
MountDatabaseStatement::getMasterUrl() const
{
	return _SYDNEY_DYNAMIC_CAST(Literal*, getObject(f_MasterUrl));
}

//	FUNCTION private
//	Statement::MountDatabaseStatement::setMasterUrl --
//		マスターデータベースのURLを設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Literal* url_
//			マスターデータベースのURL
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
MountDatabaseStatement::setMasterUrl(Literal* url_)
{
	setObject(f_MasterUrl, url_);
}

//
//	FUNCTION public
//	Statement::MountDatabaseStatement::toSQLStatement -- SQL文を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		文字列表記
//
//	EXCEPTIONS
//
ModUnicodeString
MountDatabaseStatement::toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream s;
	
	s << "mount ";

	// データベース名
	if (getDatabaseName() == 0)
	{
		s << "(null)";
	}
	else
	{
		s << getDatabaseName()->toSQLStatement(bForCascade_);
	}

	// 生成オプション
	if (getOptionList() != 0)
	{
		s << " " << getOptionList()->toSQLStatement(bForCascade_);
	}

	// エリア指定
	if (getAreaParameter() != 0)
	{
		s << " " << getAreaParameter()->toSQLStatement(bForCascade_);
	}

	// USING SNAPSHOT
	if (isUsingSnapshot())
	{
		s << " using snapshot";
	}

	// WITH RECOVERY
	if (isWithRecovery())
	{
		s << " with recovery";
	}

	// DISCARD LOGICALLOG
	if (isDiscardLogicalLog())
	{
		s << " with discard logicallog";
	}

	// AS SLAVE FROM MASTER 'trmeister://...'
	if (getMasterUrl())
	{
		s << " as slave from master "
		  << getMasterUrl()->toSQLStatement(bForCascade_);
	}

	return ModUnicodeString(s.getString());
}

#if 0
namespace
{
	Analysis::MountDatabaseStatement _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
MountDatabaseStatement::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2004, 2009, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
