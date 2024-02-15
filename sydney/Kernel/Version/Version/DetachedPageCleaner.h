// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DetachedPageCleaner.h --
//		参照済バージョンページ記述子破棄スレッドに関するクラス定義、関数宣言
// 
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_VERSION_DETACHEDPAGECLEANER_H
#define	__SYDNEY_VERSION_DETACHEDPAGECLEANER_H

#include "Version/Module.h"

#include "Buffer/DaemonThread.h"

_SYDNEY_BEGIN
_SYDNEY_VERSION_BEGIN

//	CLASS
//	Version::DetachedPageCleaner --
//		参照済バージョンページ記述子破棄スレッドを表すクラス
//
//	NOTES
//		参照済バージョンページ記述子破棄スレッドとは、
//		バージョンファイルごとに、その参照されていない
//		バージョンページ記述子のページ更新トランザクションリストを
//		空にすることを試み、空にできれば、バージョンページ記述子を
//		破棄する常駐型スレッドである

class DetachedPageCleaner
	: public	Buffer::DaemonThread
{
public:
	// コンストラクター
	DetachedPageCleaner(unsigned int timeout, bool enable);
	// デストラクター
	~DetachedPageCleaner();

private:
	// スレッドが繰り返し実行する関数
	void					repeatable();
};

//	FUNCTION private
//	Version::DetachedPageCleaner::DetachedPageCleaner --
//		参照済バージョンページ記述子破棄スレッドを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		timeout
//			参照済バージョンページ記述子の破棄の処理の実行間隔(単位ミリ秒)
//		bool				enable
//			true
//				参照済バージョンページ記述子の破棄を可能にする
//			false
//				参照済バージョンページ記述子の破棄を不可にする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
DetachedPageCleaner::DetachedPageCleaner(unsigned int timeout, bool enable)
	: Buffer::DaemonThread(timeout, enable)
{}

//	FUNCTION private
//	Version::DetachedPageCleaner::~DetachedPageCleaner --
//		参照済バージョンページ記述子破棄スレッドを表すクラスのデストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
DetachedPageCleaner::~DetachedPageCleaner()
{}

_SYDNEY_VERSION_END
_SYDNEY_END

#endif	// __SYDNEY_VERSION_DETACHEDPAGECLEANER_H

//
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
