// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Duration.h -- ロックの持続期間関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_LOCK_DURATION_H
#define	__SYDNEY_LOCK_DURATION_H

#include "Lock/Module.h"

class ModMessageStream;
class ModOstream;

_SYDNEY_BEGIN
_SYDNEY_LOCK_BEGIN

namespace Duration
{
	//	ENUM
	//	Lock::Duration::Value -- ロックの持続期間の値を表す列挙型
	//
	//	NOTES

	/*【考察】		現在、指定が有効なのは Pulse のみ */

	enum Value
	{
		// かかった時点で即はずれる
		// ロック数には意味がない

		Pulse =		0,
		Instant =	Pulse,					// 同上

		Statement,

		// カーソルが移動した時点で明示的にはずす
		// ロック数がオーバーフローした時点で Inside になる

		Cursor,
		Short =		Cursor,					// 同上

		// トランザクション終了時にはずれる
		// ロック数には意味がない

		Inside,
		Middle =	Inside,					// 同上

		// ユーザーが明示的にはずす
		// ただし、トランザクション中にははずすことはできない
		// ロック数がオーバーフローする場合、ロックできない

		User,
		Long =		User,					// 同上

		ValueNum							// 値の種類数
	};
}

_SYDNEY_LOCK_END
_SYDNEY_END

// Durationをメッセージに出すための関数
ModMessageStream& operator<<(ModMessageStream& cStream_, _SYDNEY::Lock::Duration::Value eValue_);
ModOstream& operator<<(ModOstream& cStream_, _SYDNEY::Lock::Duration::Value eValue_);

#endif	// __SYDNEY_LOCK_DURATION_H

//
// Copyright (c) 2000, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
