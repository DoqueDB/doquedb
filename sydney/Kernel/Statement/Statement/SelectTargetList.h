// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::SelectTargetList -- SelectTargetList
// 
// Copyright (c) 1999, 2002, 2004, 2008, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_SELECTTARGETLIST_H
#define __SYDNEY_STATEMENT_SELECTTARGETLIST_H

#include "Statement/ObjectList.h"

_SYDNEY_BEGIN

namespace Statement
{

	class VariableName;
	
//
//	CLASS
//		SelectTargetList -- SelectTargetList
//
//	NOTES
//		SelectTargetList
//
class SYD_STATEMENT_FUNCTION SelectTargetList : public Statement::ObjectList
{
public:
	// コンストラクタ (1)
	SelectTargetList();
	// コンストラクタ (2)
	explicit SelectTargetList(VariableName* pVariableName_);

	VariableName* getVariableNameAt(int iAt_) const;



	//自身をコピーする
	Object* copy() const;

	
	// Analyzerを得る
//	virtual const Analysis::Analyzer* getAnalyzer() const;
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

protected:
private:
	// 代入オペレーターは使わない
	SelectTargetList& operator=(const SelectTargetList& cOther_);

	// メンバ変数
};

} // namescape Statement

_SYDNEY_END

#endif // __SYDNEY_STATEMENT_SELECTTARGETLIST_H

//
// Copyright (c) 1999, 2002, 2004, 2008, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
