// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogicalLogOption.h --
// 
// Copyright (c) 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_LOGICALLOGOPTION_H
#define __SYDNEY_STATEMENT_LOGICALLOGOPTION_H

#include "Statement/Module.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

namespace LogicalLogOption {

	// 
	// 論理ログに対するオプション
	//
	enum Type {
		
		None = 0,			// オプション指定なし
		Discard,			// 不要部分の破棄

		ValueNum
	};
}

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_LOGICALLOGOPTION_H

//
//	Copyright (c) 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
