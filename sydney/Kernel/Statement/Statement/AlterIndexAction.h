// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AlterIndexAction.h --
// 
// Copyright (c) 2000, 2002, 2005, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_ALTERINDEXACTION_H
#define __SYDNEY_STATEMENT_ALTERINDEXACTION_H

#include "Common/Common.h"
#include "Statement/Object.h"


_SYDNEY_BEGIN

namespace Statement
{

class AreaOption;

//
//	CLASS
//	Statement::AlterIndexAction --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION AlterIndexAction	 : public Statement::Object
{
public:
	//constructor
	AlterIndexAction()
		: Object(ObjectType::AlterIndexAction)
	{}
	//コンストラクタ(2)
	AlterIndexAction(const int iActType_, Object* pcAreaOpt_ = 0);
	//デストラクタ
	virtual ~AlterIndexAction();

	//
	//	動作識別子
	//
	enum ActionType
	{
		Unknown = 0,
		SetArea,
		DropArea,
		Rename,
		Online,
		Offline,
		ValueNum
	};

	//
	//	アクセサ
	//

	//動作識別子アクセサ
	int getActionType() const;
	void setActionType(const int iActType_);

	//AreaOperation アクセサ
	Object* getAction() const;
	void setAction(Object* pcAction_);

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

protected:

private:
	//代入オペレータは使用しない
	AlterIndexAction& operator= (const AlterIndexAction& cOther_);

};

}

_SYDNEY_END

#endif //__SYDNEY_STATEMENT_ALTERINDEXACTION_H

//
//	Copyright (c) 2000, 2002, 2005, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
