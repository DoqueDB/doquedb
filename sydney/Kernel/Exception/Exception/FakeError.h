// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FakeError.h -- 擬似エラー
// 
// Copyright (c) 2001, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_EXCEPTION_FAKEERROR_H
#define __TRMEISTER_EXCEPTION_FAKEERROR_H

#include "Exception/Module.h"
#include "ModCriticalSection.h"
#include "ModParameter.h"
#include "ModCharString.h"
#include "ModMap.h"
#include "ModVector.h"

_TRMEISTER_BEGIN

namespace Exception
{

//
//	CLASS
//	Exception::FakeError --
//
//	NOTES
//	異常系テストのために、擬似的にエラーを発生させるためのクラス
//
class SYD_EXCEPTION_FUNCTION FakeError 
{
public:
	//
	//	STRUCT
	//	Exception::FakeError::Condition
	//
	//	NOTES
	//	1つの条件を表す構造体
	//
	struct Condition
	{
		//
		//	ENUM
		//	Exception::FakeError::Type -- 条件判定で使用するもの
		//
		struct Type
		{
			enum Value
			{
				None,
				Count
			};
		};

		//
		//	ENUM
		//	Exception::FakeError::Compare -- 条件の比較演算子
		//
		struct Compare
		{
			enum Value
			{
				None,
				Equal,
				LessThan,
				LessThanEqual,
				GreaterThan,
				GreaterThanEqual
			};
		};

		//
		//	ENUM
		//	Exception::FakeError::Operator -- 論理演算子
		//
		struct Operator
		{
			enum Value
			{
				None,
				And,
				Or
			};
		};

		// コンストラクタ
		Condition()
			: m_eType(Type::None),
			  m_eCompare(Compare::None),
			  m_eOperator(Operator::None),
			  m_iCount(0), m_iParameterCount(0) {}

		//デストラクタ
		~Condition();

		//パースする
		void parse(char* pszString_);
		void parse(char* pszString_, char szChar_);

		// 条件を満たしているかチェックする
		bool check();

		//すべての条件を削除する
		void clear();

		Type::Value			m_eType;
		Compare::Value		m_eCompare;
		Operator::Value		m_eOperator;

		//関数名
		ModCharString		m_cstrFunctionName;

		// 現在の呼び出し回数
		int					m_iCount;
		// パラメータに記述されている呼び出し回数
		int					m_iParameterCount;

		ModVector<Condition*>	m_vecpChild;
	};

	//
	//	TYPEDEF
	//	Exception::FakeError::Map
	//
	//	NOTES
	//	関数名をキーに条件を格納するマップ
	//
	typedef ModMap<ModCharString, ModVector<Condition*>, ModLess<ModCharString> > Map;

	//
	//	FUNCTION public
	//	Exception::FakeError::check -- エラーを起こす位置かどうか調べる
	//
	//	ARGUMENTS
	//	const char* pszFunctionName_
	//		関数名(エラー名)
	//
	static bool check(const char* pszFunctionName_);

	//
	//	FUNCTION public
	//	Exception::FakeError::reset -- リセットし、パラメータを読み直す
	//
	//	ARGUMENTS
	//	なし
	//
	static void reset();

	// 文字切り出しを行う
	static const char* token(const char* pszString_);

	// マップに条件を登録する
	static void insert(Condition* pCondition_);

private:
	// パラメータの内容をパースする
	static void parse(const char* pszParameter_);

	// 排他制御用のクリティカルセクション
	static ModCriticalSection m_cCriticalSection;
	// 初期化されたか
	static bool m_bInitialized;
	//条件
	static Condition* m_pCondition;
	//関数名がキーのマップ
	static Map m_mapCondition;
};

}

//
//	MACRO
//	_TRMEISTER_FAKE_ERROR
//
//	NOTES
//	擬似エラーが発生した場合に、例外を投げる
//
#ifdef SYD_FAKE_ERROR
#define _TRMEISTER_FAKE_ERROR(f, e)	\
{ \
	if (Exception::FakeError::check(f)) throw e; \
}
#else
#define _TRMEISTER_FAKE_ERROR(f, e)
#endif
#define _SYDNEY_FAKE_ERROR _TRMEISTER_FAKE_ERROR

_TRMEISTER_END

#endif //__TRMEISTER_EXCEPTION_FAKEERROR_H

//
//	Copyright (c) 2001, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
