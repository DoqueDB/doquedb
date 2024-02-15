// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectConnection.h --
// 
// Copyright (c) 2000, 2002, 2003, 2005, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_OBJECTCONNECTION_H
#define __SYDNEY_STATEMENT_OBJECTCONNECTION_H

#include "Common/Common.h"
#include "Common/Assert.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

	class Object;

//	CLASS
//	Statement::ObjectConnection --
//
//	NOTES

class ObjectConnection  : public Statement::Object
{
protected:
	//constructor
	ObjectConnection(ObjectType::Type eType_)
		: Object(eType_)
	{}
	//コンストラクタ
	SYD_STATEMENT_FUNCTION
	ObjectConnection(ObjectType::Type eType_,
					 unsigned int iNum_,
					 Generate eGen_ = Statement::Object::Undefine,
					 bool bLogRec_ = false);

	// アクセサ実装
	// Object を得る
	Statement::Object* getObject(int iIndex_) const;
	// Object 設定
	void setObject(int iIndex_, Statement::Object* pcObject_);

	// 値 を得る
	SYD_STATEMENT_FUNCTION
	int	getScaler(int iIndex_) const;
	// 値 を設定する
	SYD_STATEMENT_FUNCTION
	void setScaler(int iIndex_, int iValue_);

private:
	// 代入オペレーターは使わない
	ObjectConnection& operator=(const ObjectConnection& cOther_);
};

_SYDNEY_STATEMENT_END

//
//	FUNCTION public
//	Statement::ObjectConnection::getObject
//		-- Object 取得
//
//	NOTES
//	Object 取得
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Object*
//		Object
//
//	EXCEPTIONS
//	なし
//
inline Statement::Object*
Statement::ObjectConnection::getObject(int iIndex_) const
{
	return m_vecpElements[iIndex_];
}

//
//	FUNCTION public
//	Statement::ObjectConnection::setObject
//		-- Object 設定
//
//	NOTES
//	Object 設定
//
//	ARGUMENTS
//	Object* pcObject_
//		Object
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
inline void
Statement::ObjectConnection::setObject(int iIndex_, Statement::Object* pcObject_)
{
	_SYDNEY_ASSERT(m_vecpElements[iIndex_] == 0);
	m_vecpElements[iIndex_] = pcObject_;
}

_SYDNEY_END

#endif //__SYDNEY_STATEMENT_OBJECTCONNECTION_H

//
//	Copyright (c) 2000, 2002, 2003, 2005, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
