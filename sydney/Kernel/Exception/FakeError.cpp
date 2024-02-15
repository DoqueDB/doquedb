// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FakeError.cpp --
// 
// Copyright (c) 2001, 2005, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Exception";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Exception/FakeError.h"
#include "Exception/AutoCriticalSection.h"
#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"

#include "ModParameter.h"
#include "ModParameterSource.h"

namespace
{
//
//	VARIABLE
//	_$$::_pszParameterName
//			-- パラメータ名
//
const char* _pszParameterName = "Exception_FakeError";

//
//	以下の3つの変数は Common::SystemParameter.cpp にも同じものが存在する。
//	いずれは統合するべきである
//

//
//	VARIABLE
//	_$$::_pszSourceParent
//			-- デフォルトの親パス名
//
const char* _pszSourceParent = "TRMeister";

//
//	VARIABLE
//	_$$::_pszEnvironment
//			-- パラメータファイルを指定する環境変数の名前
//
const char* _pszEnvironment = "SYDPARAM";

//
//	VARIABLE
//	_$$::_pszSystemEnvironment
//			-- システムパラメータファイルを指定する環境変数の名前
//
const char* _pszSystemEnvironment = "SYDSYSPARAM";

}

_TRMEISTER_USING

using namespace Exception;

//
//	VARIABLE private
//	Exception::FakeError::m_cCriticalSection
//
//	NOTES
//	排他制御用のクリティカルセクション
//
ModCriticalSection
FakeError::m_cCriticalSection;

//
//	VARIABLE private
//	Exception::FakeError::m_bInitialized
//
//	NOTES
//	初期化されたかどうか
//
bool
FakeError::m_bInitialized = false;

//
//	VARIABLE private
//	Exception::FakeError::m_pCondition
//
//	NOTES
//	条件ツリー
//
FakeError::Condition*
FakeError::m_pCondition = 0;

//
//	VARIABLE private
//	Exception::FakeError::m_mapCondition
//
//	NOTES
//	条件を関数名をキーにマップに格納したもの
//
FakeError::Map
FakeError::m_mapCondition;

//
//	FUNCTION public
//	Exception::FakeError::check -- 擬似エラーを発生させるかどうかチェックする
//
//	NOTES
//	擬似エラーを発生させるかどうかチェックする。
//
//	ARGUMENTS
//	const char* pszFunctionName_
//		チェックする関数名
//
//	RETURN
//	bool
//		擬似エラーが発生した場合は true、それ以外の場合は false
//
//	EXCEPTIONS
//	なし
//
bool
FakeError::check(const char* pszFunctionName_)
{
	bool bResult = false;

	AutoCriticalSection cAuto(m_cCriticalSection);

	if (m_bInitialized == false)
	{
		// 初期化されていないのでする
		parse(_pszParameterName);
	}

	if (m_pCondition)
	{
		//マップを関数名で検索する
		Map::Iterator i = m_mapCondition.find(pszFunctionName_);
		if (i != m_mapCondition.end())
		{
			ModVector<Condition*>::Iterator j = (*i).second.begin();
			for (; j != (*i).second.end(); ++j)
			{
				(*j)->m_iCount++;
			}

			// 条件を満たしているかチェックする
			bResult = m_pCondition->check();
		}
	}

	return bResult;
}

//
//	FUNCTION public
//	Exception::FakeError::reset -- リセットし、パラメータを読み直す
//
//	NOTES
//	リセットし、パラメータを読み直す。
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
void
FakeError::reset()
{
	AutoCriticalSection cAuto(m_cCriticalSection);

	//以前の条件を削除する
	delete m_pCondition;

	// パラメータを読み直す
	parse(_pszParameterName);
}

//
//	FUNCTION public
//	Exception::FakeError::token -- 文字切り出しを行う
//
//	NOTES
//	文字切り出しを行う
//
//	ARGUMENTS
//	const char* pszString_
//		パースする文字列
//
//	RETURN
//	const char*
//		切り出した文字列
//
//	EXCEPTIONS
//	なし
//
const char*
FakeError::token(const char* pszString_)
{
	static const char* s = "";
	static char buff[1024];
	if (pszString_) s = pszString_;
	char* p = buff;
	*p = 0;
	bool bEnd = false;
	bool bKeyword = false;
	while (*s)
	{
		switch (*s)
		{
		case ' ':
		case '\t':
			if (p != buff)
				bEnd = true;
			else
				s++;
			break;
		case '=':
			if (p != buff)
				bEnd = true;
			else
			{
				*p++ = *s++;
				if (*s == '=')
					*p++ = *s++;
				bKeyword = true;
			}
			break;
		case '<':
		case '>':
			if (p != buff)
				bEnd = true;
			else
			{
				*p++ = *s++;
				if (*s == '=')
					*p++ = *s++;
				bKeyword = true;
			}
			break;
		case '&':
			if (p != buff)
				bEnd = true;
			else
			{
				*p++ = *s++;
				if (*s == '&')
					*p++ = *s++;
				bKeyword = true;
			}
			break;
		case ',':
			if (p != buff)
				bEnd = true;
			else
			{
				*p++ = *s++;
				if (*s == ',')
					*p++ = *s++;
				bKeyword = true;
			}
			break;
		case '(':
		case ')':
			if (p != buff)
				bEnd = true;
			else
			{
				*p++ = *s++;
				bKeyword = true;
			}
			break;
		default:
			if (bKeyword == true)
				bEnd = true;
			else
				*p++ = *s++;
		}
		if (bEnd) break;
	}
	*p = 0;

	return buff;
}

//
//	FUNCTION public
//	Exception::FakeError::insert -- 条件を登録する
//
//	NOTES
//
//	ARGUMENTS
//	Condition* pCondition_
//		登録する条件
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FakeError::insert(Condition* pCondition_)
{
	m_mapCondition[pCondition_->m_cstrFunctionName].pushBack(pCondition_);
}

//
//	FUNCTION private
//	Exception::FakeError::parse -- パラメータをパースする
//
//	NOTES
//	パラメータをパースし、関数名とエラー呼び出し回数をセットする
//
//	ARGUMENTS
//	const char* pszParameter_
//		パラメータ名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FakeError::parse(const char* pszParameter_)
{
	ModUnicodeString strParameter;
	const bool result =
#if MOD_CONF_REGISTRY == 0
		ModParameter(
			ModParameterSource(
				0, _pszEnvironment, 0, _pszSystemEnvironment, 0),
			ModTrue).getUnicodeString(strParameter, pszParameter_);
			
#endif
#if MOD_CONF_REGISTRY == 1
		ModParameter::getString(
			ModUnicodeString("HKEY_LOCAL_MACHINE\\SOFTWARE\\Ricoh\\TRMeister"),
			ModUnicodeString(pszParameter_, ModOs::Process::getEncodingType()),
			strParameter);
#endif

	if (result) {
		const char* pszValue = strParameter.getString();
		char* buff = new char[strParameter.getStringBufferSize()];
		ModCharTrait::copy(buff, pszValue);
		m_pCondition = new Condition;
		m_pCondition->parse(buff);
		delete [] buff;
	}

	// 初期化終了
	m_bInitialized = true;
}

//
//	FUNCTION public
//	Exception::FakeError::Condition::~Condition -- デストラクタ
//
//	NOTES
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
FakeError::Condition::~Condition()
{
	clear();
}

//
//	FUNCTION public
//	Exception::FakeError::Condition::parse -- パースする
//
//	NOTES
//
//	ARGUMENTS
//	char* pszString_
//		パースする文字列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FakeError::Condition::parse(char* pszString_)
{
	//まず、論理演算子を切り出す

	//ORを探す
	char* p = pszString_;
	while (*p)
	{
		if (*p == ',')
			break;
		p++;
	}
	if (*p != 0)
	{
		//ORがあったので、この条件ノードはOR
		m_eOperator = Operator::Or;

		//下位の条件をパースする
		parse(pszString_, ',');
	}
	else
	{
		p = pszString_;
		while (*p)
		{
			if (*p == '&')
				break;
			p++;
		}
		if (*p != 0)
		{
			//ANDがあったので、この条件ノードはAND
			m_eOperator = Operator::And;

			//下位の条件をパースする
			parse(pszString_, '&');
		}
		else
		{
			//ORもANDもないので、関数名と条件をパースする
			int state = 0;
			p = pszString_;
			const char* t;
			while (*(t = token(p)))
			{
				// 今現在は以下のフォーマットしかサポートしていない
				// ・"FunctionName count = ?"
				// ・"FunctionName count = (? ? ...)"

				if (state == 0)
				{
					// このステートは関数名のとき
					m_cstrFunctionName = t;
					state++;
				}
				else if (state == 1)
				{
					// このステートは条件の左辺のとき
					if (ModCharTrait::compare(t, "count") == 0)
					{
						m_eType = Type::Count;
					}
					else
					{
						throw NotSupported(moduleName, srcFile, __LINE__);
					}
					state++;
				}
				else if (state == 2)
				{
					// このステートは条件の演算子
					if (ModCharTrait::compare(t, "==") == 0
						|| ModCharTrait::compare(t, "=") == 0)
					{
						m_eCompare = Compare::Equal;
					}
					else if (ModCharTrait::compare(t, "<") == 0)
					{
						m_eCompare = Compare::LessThan;
					}
					else if (ModCharTrait::compare(t, "<=") == 0)
					{
						m_eCompare = Compare::LessThanEqual;
					}
					else if (ModCharTrait::compare(t, ">") == 0)
					{
						m_eCompare = Compare::GreaterThan;
					}
					else if (ModCharTrait::compare(t, ">=") == 0)
					{
						m_eCompare = Compare::GreaterThanEqual;
					}
					else
					{
						throw NotSupported(moduleName, srcFile, __LINE__);
					}
					state++;
				}
				else if (state == 3)
				{
					// このステートは条件値
					if (m_eType== Type::Count)
					{
						if (ModCharTrait::compare(t, "(") == 0)
						{
							m_eOperator = Operator::Or;
							state++;
						}
						else 
						{
							m_iParameterCount = ModCharTrait::toInt(t);
							//マップに登録する
							FakeError::insert(this);
							state = 5;
						}
					}
					else
					{
						throw NotSupported(moduleName, srcFile, __LINE__);
					}
				}
				else if (state == 4)
				{
					if (ModCharTrait::compare(t, ")") == 0) 
					{
						state++;
					}
					else
					{
						Condition* cond = new Condition;
						cond->m_eType = Type::Count;
						cond->m_eCompare = Compare::Equal;
						cond->m_cstrFunctionName = this->m_cstrFunctionName;
						cond->m_iParameterCount = ModCharTrait::toInt(t);
						//マップに登録する
						FakeError::insert(cond);
						m_vecpChild.pushBack(cond);
					}
				}
				else if (state >= 5)
				{
					// このステートには来ない
					throw NotSupported(moduleName, srcFile, __LINE__);
				}
				p = 0;
			}

			if (state == 0)
			{
				// 何も実行しない
				;
			}
			else if (state == 1)
			{
				// デフォルトではカウントが1の場合
				m_eType = Type::Count;
				m_eCompare = Compare::Equal;
				m_iParameterCount = 1;
			}
			else if (state != 5)
			{
				// エラー
				throw BadArgument(moduleName, srcFile, __LINE__);
			}

//			//マップに登録する
//			FakeError::insert(this);
		}
	}
}

//
//	FUNCTION public
//	Exception::FakeError::Condition::parse -- パースする
//
//	NOTES
//
//	ARGUMENTS
//	char* pszString_
//		パースする文字列
//	char szChar_
//		処理する論理演算子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FakeError::Condition::parse(char* pszString_, char szChar_)
{
	char* s = pszString_;
	bool next = false;
	char* p = s;
	while (*p)
	{
		if (*p == szChar_)
			break;
		p++;
	}
	while (*s)
	{
		if (*p)
			next = true;
		else
			next = false;

		*p = 0;
		Condition* pCondition = new Condition;
		pCondition->parse(s);

		m_vecpChild.pushBack(pCondition);

		s = p;
		if (next == true)
		{
			s++;
			p++;
			while (*p)
			{
				if (*p == szChar_)
					break;
				p++;
			}
		}
	}
}

//
//	FUNCTION public
//	Exception::FakeError::Condition::check -- 条件を満たしているかチェックする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		満たしている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool
FakeError::Condition::check()
{
	bool bResult = false;

	switch (m_eOperator)
	{
	case Operator::Or:
		//ORの時は m_vecpChild のうちどれかが true だったら true
		{
			ModVector<Condition*>::Iterator i = m_vecpChild.begin();
			for (; i != m_vecpChild.end(); ++i)
			{
				if ((*i)->check() == true)
				{
					(*i)->clear();	//ORの時は一度ヒットした条件は二度と使用しない
					bResult = true;
					break;
				}
			}
		}
		break;
	case Operator::And:
		//ANDの時は m_vecpChild すべてが true だったら true
		{
			ModVector<Condition*>::Iterator i = m_vecpChild.begin();
			for (; i != m_vecpChild.end(); ++i)
			{
				if ((*i)->check() == false)
				{
					break;
				}
			}
			if (i == m_vecpChild.end())
				bResult = true;
		}
		break;
	case Operator::None:
		// 自分が ture なら true
		switch (m_eCompare)
		{
		case Compare::Equal:
			bResult = (m_iParameterCount == m_iCount);
			break;
		case Compare::LessThan:
			bResult = (m_iParameterCount > m_iCount);
			break;
		case Compare::LessThanEqual:
			bResult = (m_iParameterCount >= m_iCount);
			break;
		case Compare::GreaterThan:
			bResult = (m_iParameterCount < m_iCount);
			break;
		case Compare::GreaterThanEqual:
			bResult = (m_iParameterCount <= m_iCount);
			break;
		}
		break;
	}

	return bResult;
}

//
//	FUNCTION public
//	Exception::FakeError::Condition::clear -- すべての条件を削除する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	ない
//
void
FakeError::Condition::clear()
{
	ModVector<Condition*>::Iterator i = m_vecpChild.begin();
	for (; i != m_vecpChild.end(); ++i)
	{
		delete *i;
	}
	m_eType = Type::None;
	m_eCompare = Compare::None;
	m_eOperator = Operator::None;
	m_cstrFunctionName = "";
	m_iCount = 0;
	m_iParameterCount = 0;
	m_vecpChild.clear();
}

//
//	Copyright (c) 2001, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
