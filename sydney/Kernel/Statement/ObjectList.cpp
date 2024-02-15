// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectList.cpp -- ObjectList
// 
// Copyright (c) 1999, 2002, 2004, 2005, 2006, 2010, 2013, 2023 Ricoh Company, Ltd.
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
	const char moduleName[] = "Statement";
	const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Statement/SQLParser.h"
#include "Statement/Type.h"
#include "Statement/IntegerValue.h"
#include "Statement/ObjectList.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModOstrStream.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

//
//	FUNCTION public
//	Statement::ObjectConnection::ObjectList
//		-- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	なし
//
//	RETURN
//
//	EXCEPTIONS
//	なし
//
ObjectList::ObjectList(ObjectType::Type eType_)
	: Object(eType_, 0)
{
}

//
//	FUNCTION public
//		Statement::ObjectList::getCount -- Object の個数を得る
//
//	NOTES
//		リストの要素の個数を得る実装を提供
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		int
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
int
ObjectList::getCount() const
{
	return m_vecpElements.getSize();
}

//
//	FUNCTION public
//		Statement::ObjectList::getAt -- Object を得る
//
//	NOTES
//		リストの要素を得る実装を提供
//
//	ARGUMENTS
//		int iAt_
//
//	RETURN
//		Object*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Statement::Object*
ObjectList::getAt(int iAt_) const
{
	return m_vecpElements[iAt_];
}

//
//	FUNCTION public
//		Statement::ObjectList::setAt -- Object を設定する
//
//	NOTES
//		リストの要素を設定する実装を提供
//
//	ARGUMENTS
//		int iAt_
//		Object* pObject_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ObjectList::setAt(int iAt_, Object* pObject_)
{
	; _SYDNEY_ASSERT(0 <= iAt_);
	if ( iAt_ < static_cast<int>(m_vecpElements.getSize()) ) {
		m_vecpElements[iAt_] = (pObject_);//範囲内
	} else {
		m_vecpElements.pushBack(pObject_);//範囲外
	}
}

//
//	FUNCTION public
//		Statement::ObjectList::append -- Object を加える
//
//	NOTES
//		リストの要素を加える実装を提供
//
//	ARGUMENTS
//		Object* pObject_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ObjectList::append(Object* pObject_)
{
	m_vecpElements.pushBack(pObject_);
}

// FUNCTION public
//	Plan::ObjectList::insert -- オブジェクト要素を先頭に加える
//
// NOTES
//
// ARGUMENTS
//	Object* pObject_
//	
// RETURN
//	なし
//
// EXCEPTIONS

void
ObjectList::insert(Object* pObject_)
{
	m_vecpElements.pushFront(pObject_);
}

// FUNCTION public
//	Statement::ObjectList::merge -- オブジェクトリストを追加する
//
// NOTES
//	コピー元の要素の所有権も移るので注意
//
// ARGUMENTS
//	ObjectList& cList_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ObjectList::
merge(ObjectList& cList_)
{
	m_vecpElements.insert(m_vecpElements.end(),
						  cList_.m_vecpElements.begin(), cList_.m_vecpElements.end());
	// コピー元の要素の所有権も移す
	cList_.m_vecpElements.clear();
}

// SQL文で値を得る
//virtual
ModUnicodeString
ObjectList::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream cStream;
	if (int n = getCount()) {
		int i = 0;
		do {
			if (i) cStream << ", ";
			if (getAt(i)) {
				cStream << getAt(i)->toSQLStatement(bForCascade_);
			} else {
				cStream << "(0)";
			}
		} while (++i < n);
	}
	return cStream.getString();
}

//
//	Copyright (c) 1999, 2002, 2004, 2005, 2006, 2010, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

