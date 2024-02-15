// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AlterAreaAction.h --
// 
// Copyright (c) 2000, 2002, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_ALTERAREAACTION_H
#define __SYDNEY_STATEMENT_ALTERAREAACTION_H

#include "Common/Common.h"
#include "Statement/Object.h"


_SYDNEY_BEGIN

namespace Statement
{

class AreaElementList;

//
//	CLASS
//	Statement::AlterAreaAction --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION AlterAreaAction	: public Statement::Object
{
public:
	//constructor
	AlterAreaAction()
		: Object(ObjectType::AlterAreaAction)
	{}
	//コンストラクタ(2)
	AlterAreaAction(const int iActType_, AreaElementList* pcAreaElem_);
	//デストラクタ
	virtual ~AlterAreaAction();

	//
	//	動作識別子
	//
	enum ActionType
	{
		Unknown = 0,
		SingleModify,	//非配列変更用
		ElemAryModify,	//配列要素変更用
		FullAryModify	//配列全要素変更用
	};

	//
	//	アクセサ
	//

	//動作識別用アクセサ
	int getActionType() const;
	void setActionType(const int iActType_);

	//AreaElementList アクセサ
	AreaElementList* getAreaElementList() const;
	void setAreaElementList(AreaElementList* pcAreaElem_);

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

protected:

private:
	//代入オペレータは使用しない
	AlterAreaAction& operator= (const AlterAreaAction& cOther_);

};

} // namespace Statement

_SYDNEY_END

#endif //__SYDNEY_STATEMENT_ALTERAREAACTION_H

//
//	Copyright (c) 2000, 2002, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
