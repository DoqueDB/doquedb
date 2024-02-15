// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AlterDatabaseStatement.cpp --
// 
// Copyright (c) 2000, 2002, 2003, 2009, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#include "Statement/Type.h"
#include "Statement/Identifier.h"
#include "Statement/IntegerValue.h"
#include "Statement/DatabaseCreateOption.h"
#include "Statement/AlterDatabaseAttribute.h"
#include "Statement/AlterDatabaseAttributeList.h"
#include "Statement/AreaOption.h"
#include "Statement/AlterDatabaseStatement.h"
#include "Statement/LogicalLogOption.h"
#if 0
#include "Analysis/AlterDatabaseStatement.h"
#endif

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING

namespace
{
	// メンバのm_vecpElements内でのindex
	enum
	{
		f_DatabaseName,
		f_AlterOption,
		f_ReplicationType,
		f__end_index
	};

}

using namespace Statement;

//
//	FUNCTION public
//	Statement::AlterDatabaseStatement::AlterDatabaseStatement
//		-- コンストラクタ(2)
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
AlterDatabaseStatement::AlterDatabaseStatement(Identifier* pcIdent_,
											   AlterDatabaseAttributeList* pcAttribute_)
	: Object(ObjectType::AlterDatabaseStatement, f__end_index, Object::System, true)
{
	setDatabaseName(pcIdent_);
	setAlterDatabaseOption(pcAttribute_);
}

//
//	FUNCTION public
//	Statement::AlterDatabaseStatement::~AlterDatabaseStatement -- デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
AlterDatabaseStatement::~AlterDatabaseStatement()
{
}

//
//	FUNCTION public
//	Statement::AlterDatabaseStatement::getDatabaseName
//		-- Database 名を取得する
//
//	NOTES
//	Database 名を取得する
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Identifier*
//		Database 名
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
Identifier*
AlterDatabaseStatement::getDatabaseName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_DatabaseName];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::AlterDatabaseStatement::setDatabaseName
//		-- Database 名を設定する
//
//	NOTES
//	Database 名を設定する
//
//	ARGUMENTS
//	Identifer* pcIdent_
//		Database 名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
AlterDatabaseStatement::setDatabaseName(Identifier* pcIdent_)
{
	m_vecpElements[f_DatabaseName] = pcIdent_;
}

//
//	FUNCTION public
//	Statement::AlterDatabaseStatement::getAlterDatabaseOption
//		-- AlterDatabaseAttributeList を取得する
//
//	NOTES
//	AlterDatabaseAttributeList を取得する
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	AlterDatabaseAttributeList*
//		AlterDatabaseAttributeList
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
AlterDatabaseAttributeList*
AlterDatabaseStatement::getAlterDatabaseOption() const
{
	AlterDatabaseAttributeList* pResult = 0;
	Object* pObj = m_vecpElements[f_AlterOption];
	if ( pObj && ObjectType::AlterDatabaseAttributeList == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(AlterDatabaseAttributeList*, pObj);
	return pResult;
}

//	FUNCTION public
//	Statement::AlterDatabaseStatement::getAlterDatabaseOption --
//		あるオプションが指定されているか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Statement::AlterDatabaseAttribute::AttributeType	type
//			指定されているか調べるオプションの種類
//
//	RETURN
//		Boolean::True
//			真の意味で指定されている
//		Boolean::False
//			偽の意味で指定されている
//		Boolean::Unknown
//			指定されていない
//
//	EXCEPTIONS

Boolean::Value
AlterDatabaseStatement::getAlterDatabaseOption(
	AlterDatabaseAttribute::AttributeType type) const
{
	Boolean::Value	result = Boolean::Unknown;

	if (getAlterDatabaseOption() == 0)
		return result;
	
	const AlterDatabaseAttributeList& list = *getAlterDatabaseOption();
	const int n = list.getCount();

	for (int i = 0; i < n; ++i) {
		; _SYDNEY_ASSERT(list.getDatabaseAttributeAt(i));
		const AlterDatabaseAttribute& attr = *list.getDatabaseAttributeAt(i);

		if (attr.getAttributeType() != type)
			// 調べたい種類ではないので、次へ
			continue;

		switch (attr.getAttributeType()) {
		case AlterDatabaseAttribute::ReadWrite:
			result =
				(attr.getAttribute() == DatabaseCreateOption::ReadWrite) ?
				Boolean::True : Boolean::False;
			break;

		case AlterDatabaseAttribute::Online:
			result =
				(attr.getAttribute() == DatabaseCreateOption::Online ||
				 attr.getAttribute() == DatabaseCreateOption::OnlineWithDiscardLogicalLog) ?
				Boolean::True : Boolean::False;
			break;

		case AlterDatabaseAttribute::RecoveryFull:
			result =
				(attr.getAttribute() == DatabaseCreateOption::RecoveryFull) ?
				Boolean::True : Boolean::False;
			break;

		case AlterDatabaseAttribute::SuperUserMode:
			result =
				(attr.getAttribute() == DatabaseCreateOption::SuperUser) ?
				Boolean::True : Boolean::False;
			break;
		}

		break;
	}

	return result;
}

//
//	FUNCTION public
//	Statement::AlterDatabaseStatement::setAlterDatabaseOption
//		-- AlterDatabaseAttributeList を設定する
//
//	NOTES
//	AlterDatabaseAttributeList を設定する
//
//	ARGUMENTS
//	AlterDatabaseAttributeList* pcAttribute_
//		AlterDatabaseAttributeList
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
AlterDatabaseStatement::setAlterDatabaseOption(AlterDatabaseAttributeList* pcAttribute_)
{
	m_vecpElements[f_AlterOption] = pcAttribute_;
}

//	FUNCTION public
//	Statement::AlterDatabaseStatement::isDiscardLogicalLog
//		WITH DISCARD LOGICALLOGが指定されているかどうか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		bool
//			指定されている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS

bool
AlterDatabaseStatement::isDiscardLogicalLog() const
{
	bool result = false;

	const AlterDatabaseAttributeList& list = *getAlterDatabaseOption();
	const int n = list.getCount();

	for (int i = 0; i < n; ++i) {
		; _SYDNEY_ASSERT(list.getDatabaseAttributeAt(i));
		const AlterDatabaseAttribute& attr = *list.getDatabaseAttributeAt(i);

		if (attr.getAttributeType() == AlterDatabaseAttribute::Online &&
			attr.getAttribute()
			== DatabaseCreateOption::OnlineWithDiscardLogicalLog)
		{

			// 指定されている
			
			result = true;
			break;
		}
		
	}

	return result;
}

//
//	FUNCTION public
//	Statement::AlterDatabaseStatement::getReplicationType --
//		レプリケーションの動作種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Statement::AlterDatabaseStatement::ReplicationType
//			レプリケーションの動作種別
//
//	EXCEPTIONS
//		なし
//
AlterDatabaseStatement::ReplicationType
AlterDatabaseStatement::getReplicationType() const
{
	const IntegerValue* v = _SYDNEY_DYNAMIC_CAST(
		IntegerValue*, getElement(f_ReplicationType, ObjectType::IntegerValue));
	if (v == 0)
		return Unknown;
	return static_cast<ReplicationType>(v->getValue());
}

//
//	FUNCTION public
//	Statement::AlterDatabaseStatement::setReplicationType --
//		レプリケーションの動作種別を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::AlterDatabaseStatement::ReplicationType
//			設定するレプリケーションの動作種別
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//
void
AlterDatabaseStatement::setReplicationType(ReplicationType eType_)
{
	setElement(f_ReplicationType, new IntegerValue(eType_));
}

//
//	FUNCTION public
//	Statement::AlterDatabaseStatement::copy -- 自身をコピーする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Statement::Object*
//
//	EXCEPTIONS
//	なし
//
Object*
AlterDatabaseStatement::copy() const
{
	return new AlterDatabaseStatement(*this);
}

//
//	FUNCTION public
//	Statement::AlterDatabaseStatement::toSQLStatement
//		-- SQL文で値を得る
//
//	NOTES
//
//	ARGUMNENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		文字列表記
//
//	EXCEPTIONS
//
ModUnicodeString
AlterDatabaseStatement::toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream s;

	s << "alter database ";
	if (getDatabaseName())
		s << getDatabaseName()->toSQLStatement(bForCascade_);
	else
		s << "(null)";
	if (getAlterDatabaseOption())
		s << " " << getAlterDatabaseOption()->toSQLStatement(bForCascade_);
	switch (getReplicationType())
	{
	case StartSlave:
		s << " start slave";
		break;
	case StopSlave:
		s << " stop slave";
		break;
	case SetToMaster:
		s << " set to master";
		break;
	default:
		break;
	}

	return ModUnicodeString(s.getString());
}

#if 0
namespace
{
	Analysis::AlterDatabaseStatement _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
AlterDatabaseStatement::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2003, 2009, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
