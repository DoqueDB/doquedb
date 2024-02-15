// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectSelection.h --
// 
// Copyright (c) 2000, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_OBJECTSELECTION_H
#define __SYDNEY_STATEMENT_OBJECTSELECTION_H

#include "Statement/Module.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class Object;

//
//	CLASS
//	ObjectSelection -- 選択の Node
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION ObjectSelection : public Statement::Object
{
protected:
	//コンストラクタ
	//(ObjectListクラスはオブジェクト化しない)
	ObjectSelection(ObjectType::Type eType_);

	//
	//	アクセサ実装
	//
	//識別子アクセサ
	int getObjectType() const;
	void setObjectType(int iType_);

	//Object アクセサ
	Object* getObject() const;
	void setObject(Object* pcObject_);

	// Scaler アクセサ
	int	getScaler() const;
	void setScaler(int iValue_);

private:
	//代入オペレータは使用しない
	ObjectSelection& operator= (const ObjectSelection& cOther_);
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_OBJECTSELECTION_H

//
//	Copyright (c) 2000, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
