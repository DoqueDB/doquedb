// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Buffer.h --	バッファに関する処理を行うクラス関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_CHECKPOINT_BUFFER_H
#define	__SYDNEY_CHECKPOINT_BUFFER_H

#include "Checkpoint/Module.h"

_SYDNEY_BEGIN
_SYDNEY_CHECKPOINT_BEGIN

//	CLASS
//	Checkpoint::Buffer --
//		チェックポイント処理のうち、バッファに関する処理を行うクラス
//
//	NOTES
//		このクラスは new することはないはずなので、
//		Common::Object の子クラスにしない

class Buffer
{
	friend class Executor;
private:
	// 直前のチェックポイント処理の終了時にダーティなバッファのうち、
	// 現在もダーティなものをすべてフラッシュする
	static bool				flush(bool onlyMarked);
};

_SYDNEY_CHECKPOINT_END
_SYDNEY_END

#endif	// __SYDNEY_CHECKPOINT_BUFFER_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
