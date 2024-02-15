// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SydTestException.h -- SydTest専用の例外を表すクラス
// 
// Copyright (c) 2001, 2002, 2003, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SYDTEST_SYDTESTEXCEPTION_H
#define __SYDNEY_SYDTEST_SYDTESTEXCEPTION_H

#include "Common/Common.h"
#include "Common/Internal.h"

_SYDNEY_BEGIN

namespace SydTest
{

//
//	CLASS
//	SydTest::SydTestException -- SydTest専用の例外を表すクラス
//
//	NOTES
//  Sydney専用の例外を表すクラス
//
class SydTestException
{
public:

	enum Description {
		Nope = 0,
		Fatal, 
		BadSyntax, 
		FileNotFound,
		InitializeFailed,
		TagNotFound,
		TagAlreadyExists,
		Requested,
		Boundary
	};

	//コンストラクタ
	SydTestException();
	SydTestException(Description eDescription_);
	SydTestException(int iBrokenLine_);
	//デストラクタ
	virtual ~SydTestException();
	//マニピュレータ
	void setDescription(Description eDescription_);
	void setLine(int iBrokenLine_);
	//アクセサ
	Description getDescription() const;
	int getLine() const;

private:
	Description	m_eDescription; // エラーコード
	int	m_iBrokenLine; // m_eDescriptionがBadSyntaxだったとき、その行番号
};

//
//	FUNCTION public
//  SydTestException::SydTestException -- コンストラクタ(1)
//
//	NOTES
//  m_eDescriptionおよびm_iBrokenLineはそれぞれNopeと0に設定される。
//
//	ARGUMENTS
//    なし
//
//	RETURN
//    なし
//
inline
SydTestException::SydTestException()
: m_eDescription(Nope), m_iBrokenLine(0)
{
}

//
//	FUNCTION public
//  SydTestException::SydTestException -- コンストラクタ(2)
//
//	NOTES
//  コンストラクタ
//
//	ARGUMENTS
//    eDescription_
//      スクリプトのエラーコード
//
//	RETURN
//    なし
//
inline
SydTestException::SydTestException(Description eDescription_)
: m_eDescription(eDescription_), m_iBrokenLine(0)
{
}

//
//	FUNCTION public
//  SydTestException::SydTestException -- コンストラクタ(3)
//
//	NOTES
//  コンストラクタ
//
//	ARGUMENTS
//    eDescription_
//      スクリプトのエラーコード
//    iBrokenLine_
//      スクリプト失敗箇所の行番号
//
//	RETURN
//    なし
//
inline
SydTestException::SydTestException(int iBrokenLine_)
: m_eDescription(BadSyntax), m_iBrokenLine(0)
{
}

//
//	FUNCTION public
//  SydTestException::~SydTestException -- デストラクタ
//
//	NOTES
//  デストラクタ
//
//	ARGUMENTS
//    なし
//
//	RETURN
//    なし
//
inline
SydTestException::~SydTestException()
{
}

//
//	FUNCTION public
//  SydTestException::setDescription -- エラーコードをセットする
//
//	NOTES
//  エラーコードをセットする
//
//	ARGUMENTS
//    eDescription_
//      エラーコード
//
//	RETURN
//    なし
//
inline
void
SydTestException::setDescription(Description eDescription_)
{
	m_eDescription = eDescription_;
}

//
//	FUNCTION public
//  SydTestException::setLine -- スクリプト失敗行の番号をセットする
//
//	NOTES
//  スクリプト失敗箇所の行番号をセットする
//
//	ARGUMENTS
//    iBrokenLine_
//      スクリプト失敗箇所の行番号
//
//	RETURN
//    なし
//
inline
void
SydTestException::setLine(int iBrokenLine_)
{
	m_eDescription = BadSyntax;
	m_iBrokenLine = iBrokenLine_;
}

//
//	FUNCTION public
//  SydTestException::getDescription -- エラーコードを得る
//
//	NOTES
//  エラーコードを得る
//
//	ARGUMENTS
//    なし
//
//	RETURN
//    Description
//      エラーコード
//
inline
SydTestException::Description
SydTestException::getDescription() const
{
	return m_eDescription;
}

//
//	FUNCTION public
//  SydTestException::getLine -- スクリプト失敗行の番号を得る
//
//	NOTES
//  スクリプト失敗箇所の行番号を得る
//
//	ARGUMENTS
//    なし
//
//	RETURN
//    int
//      スクリプト失敗箇所の行番号
//
inline
int
SydTestException::getLine() const
{
	return m_iBrokenLine;
}

}

_SYDNEY_END

#endif // __SYDNEY_SYDTEST_SYDTESTEXCEPTION_H

//
//	Copyright (c) 2001, 2002, 2003, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
