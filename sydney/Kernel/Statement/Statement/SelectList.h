// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::SelectList -- SelectList
// 
// Copyright (c) 1999, 2002, 2004, 2008, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_SELECTLIST_H
#define __SYDNEY_STATEMENT_SELECTLIST_H

#include "Statement/Object.h"

_SYDNEY_BEGIN

namespace Statement
{
	class SelectSubListList;
	class ValueExpression;
	
//
//	CLASS
//		SelectList -- SelectList
//
//	NOTES
//		SelectList
//
class SYD_STATEMENT_FUNCTION SelectList : public Statement::Object
{
public:
	// コンストラクタ (1)
	SelectList();
	// コンストラクタ (2)
	explicit SelectList(SelectSubListList* pSelectSubListList_);

	// アクセサ
	// SelectSubListList を得る
	SelectSubListList* getSelectSubListList() const;
	// SelectSubListList を設定する
	void setSelectSubListList(SelectSubListList* pSelectSubListList_);

	// select * の * を表わす
	static SelectList* asterisk();
	// *か
	bool isAsterisk() const;

	//自身をコピーする
	Object* copy() const;

	// Analyzerを得る
#ifdef USE_OLDER_VERSION
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

protected:
private:
	// 代入オペレーターは使わない
	SelectList& operator=(const SelectList& cOther_);

	// メンバ変数
};

} // namescape Statement

_SYDNEY_END

#endif // __SYDNEY_STATEMENT_SELECTLIST_H

//
// Copyright (c) 1999, 2002, 2004, 2008, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
