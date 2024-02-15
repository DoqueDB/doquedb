// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IntegerArray.h --
// 
// Copyright (c) 2001, 2002, 2003, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_INTEGERARRAY_H
#define __SYDNEY_STATEMENT_INTEGERARRAY_H

#include "Common/Common.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

//
//	CLASS
//	Statement::IntegerArray --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION IntegerArray : public Statement::Object
{
public:
	//コンストラクタ
	IntegerArray();
	//デストラクタ
	virtual ~IntegerArray();

    //値を取得する
    int getValue(const int iIdx_) const;

#ifdef OBSOLETE    
    //値を設定する
    void setValue(const int iIdx_, const int iValue_);
#endif
    void pushBack(const int iValue_);

    //要素数を取得する
    int getSize() const;

#ifdef OBSOLETE
    //オペレータ
    int operator[](int iIdx_);
    const int operator[](const int iIdx_) const;
#endif

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
};

//	FUNCTION public
//	Statement::IntegerArray::copy -- 自身をコピーする
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

inline
Object*
IntegerArray::copy() const
{
	return new IntegerArray(*this);
}

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_INTEGERARRAY_H

//
//	Copyright (c) 2001, 2002, 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
