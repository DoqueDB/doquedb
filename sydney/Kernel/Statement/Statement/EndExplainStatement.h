// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// EndExplainStatement.h --
// 
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_ENDEXPLAINSTATEMENT_H
#define __SYDNEY_STATEMENT_ENDEXPLAINSTATEMENT_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

//
//	CLASS
//	Statement::EndExplainStatement --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION EndExplainStatement  : public Object
{
public:
	//コンストラクタ(1)
	EndExplainStatement();
	//デストラクタ
	virtual ~EndExplainStatement();

	//自身をコピーする
	Object* copy() const;

protected:
private:
	// 代入オペレーターは使わない
	EndExplainStatement& operator=(const EndExplainStatement& cOther_);

};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_ENDEXPLAINSTATEMENT_H

//
//	Copyright (c) 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
