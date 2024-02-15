// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SortParameter.h --
// 
// Copyright (c) 2004-2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_SORTPARAMETER_H
#define __SYDNEY_INVERTED_SORTPARAMETER_H

#include "Inverted/Module.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

namespace SortParameter
{
	// ソート種別
	enum Value
	{
		None,
		ScoreDesc,	// スコア降順
		ScoreAsc,	// スコア昇順
		RowIdDesc,	// ROWID降順
		RowIdAsc,	// ROWID昇順
		ScaleDesc,	// スケール降順
		ScaleAsc,	// スケール昇順
		DfDesc,		// DF降順
		DfAsc,		// DF昇順
		TermNoDesc,	// 単語番号順降順
		TermNoAsc	// 単語番号順昇順
	};
}



_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_SORTPARAMETER_H

//
//	Copyright (c) 2004-2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
