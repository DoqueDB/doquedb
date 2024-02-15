// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// JapaneseBlocker3 -- 日本語用のブロック化器
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_JAPANESEBLOCKER3_H
#define __SYDNEY_FULLTEXT2_JAPANESEBLOCKER3_H

#include "FullText2/Module.h"
#include "FullText2/JapaneseBlocker2.h"

#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::JapaneseBlocker3
//		-- 基本的には JapaneseBlocker2と同じだが、
//		   「々」を記号ではなく漢字として処理する
//
//	NOTES
//
class JapaneseBlocker3 : public JapaneseBlocker2
{
public:
	// コンストラクタ
	JapaneseBlocker3();
	// デストラクタ
	virtual ~JapaneseBlocker3();

protected:
	// ブロック種別を得る
	ModSize getBlock(ModUnicodeChar c_) const;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_JAPANESEBLOCKER3_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
