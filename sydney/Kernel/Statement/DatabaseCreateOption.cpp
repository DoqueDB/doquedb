// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DatabaseCreateOption.cpp --
// 
// Copyright (c) 2000, 2002, 2012, 2013, 2023 Ricoh Company, Ltd.
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
#include "Statement/DatabaseCreateOption.h"
#include "Statement/IntegerValue.h"
#if 0
#include "Analysis/DatabaseCreateOption.h"
#endif

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING

namespace
{
	namespace _ReadWriteMode
	{
		ModUnicodeString	_cReadOnly("read only");
		ModUnicodeString	_cReadWrite("read write");
	}
	namespace _OnlineMode
	{
		ModUnicodeString	_cOnline("online");
		ModUnicodeString	_cOffline("offline");
		ModUnicodeString	_cOnlineWith("online with discard logicallog");
	}
	namespace _RecoveryMode
	{
		ModUnicodeString	_cRecoveryFull("recovery full");
		ModUnicodeString	_cRecoveryCheckpoint("recovery checkpoint");
	}
	namespace _SuperUserMode
	{
		ModUnicodeString	_cSuperUser("user system");
		ModUnicodeString	_cAllUser("user all");
	}
}

using namespace Statement;

//
//	FUNCTION public
//	Statement::DatabaseCreateOption::DatabaseCreateOption -- コンストラクタ(2)
//
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
DatabaseCreateOption::DatabaseCreateOption(OptionType iOptionType, Object* pcOption_)
	: ObjectSelection(ObjectType::DatabaseCreateOption)
{
	setOptionType(iOptionType);
	setOption(pcOption_);
}

//
//	FUNCTION public
//	Statement::DatabaseCreateOption::~DatabaseCreateOption -- デストラクタ
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
DatabaseCreateOption::~DatabaseCreateOption()
{
}

//
//	FUNCTION public
//	Statement::DatabaseCreateOption::copy -- 自身をコピーする
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
DatabaseCreateOption::copy() const
{
	return new DatabaseCreateOption(*this);
}

//
//	FUNCTION public
//	Statement::DatabaseCreateOption::toSQLStatement
//		-- SQL文で値を得る
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
DatabaseCreateOption::toSQLStatement(bool bForCascade_ /* = false */) const
{
	Object* p = getOption();
	if (p == 0)
		// ここにくることはないけど、念のため
		return ModUnicodeString("null");

	if (p->getType() == ObjectType::IntegerValue)
	{
		IntegerValue* n = _SYDNEY_DYNAMIC_CAST(IntegerValue*, p);
		return getOptionName(static_cast<OptionType>(getOptionType()),
							 n->getValue());
	}

	return p->toSQLStatement(bForCascade_);
}

//
//	FUNCTION public static
//	Statement::DatabaseCreateOption::getOptionName
//		-- オプションの文字列表記を得る
//
//	NOTES
//
//	ARGUMENTS
//	Statement::DatabaseCreateOption::iOptionType_
//		オプションタイプ
//	int iValue_
//		オプション値
//
//	RETURN
//	ModUncodeString
//		オプションの文字列表記
//
//	EXCEPTIONS
//
ModUnicodeString
DatabaseCreateOption::getOptionName(OptionType iOptionType_,
									int iValue_)
{
	ModUnicodeString r;
	
	switch (iOptionType_)
	{
	case ReadWriteOption:
		switch (iValue_)
		{
		case ReadOnly:
			r = _ReadWriteMode::_cReadOnly;
			break;
		case ReadWrite:
			r = _ReadWriteMode::_cReadWrite;
			break;
		}
		break;
	case OnlineOption:
		switch (iValue_)
		{
		case Online:
			r = _OnlineMode::_cOnline;
			break;
		case Offline:
			r = _OnlineMode::_cOffline;
			break;
		case OnlineWithDiscardLogicalLog:
			r = _OnlineMode::_cOnlineWith;
			break;
		}
		break;
	case RecoveryOption:
		switch (iValue_)
		{
		case RecoveryFull:
			r = _RecoveryMode::_cRecoveryFull;
			break;
		case RecoveryCheckpoint:
			r = _RecoveryMode::_cRecoveryCheckpoint;
			break;
		}
		break;
	case UserModeOption:
		switch (iValue_)
		{
		case SuperUser:
			r = _SuperUserMode::_cSuperUser;
			break;
		case AllUser:
			r = _SuperUserMode::_cAllUser;
			break;
		}
		break;
	}

	if (r.getLength() == 0)
	{
		ModUnicodeOstrStream s;
		s << "???";
		r = s.getString();
	}

	return r;
}

#if 0
namespace
{
	Analysis::DatabaseCreateOption _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
DatabaseCreateOption::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
