//-*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CriticalSectionManager.h -- クリティカルセクショマネージャ (Linux のみビルド対象)
// 
// Copyright (c) 2018, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_OS_CRITICALSECTIONMANAGER_H
#define	__TRMEISTER_OS_CRITICALSECTIONMANAGER_H

#include "Os/Module.h"

#include "ModCharString.h"

#include <boost/unordered_set.hpp>
#include <boost/thread/mutex.hpp>

class ModUnicodeString;

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

class CriticalSection;


//	CLASS
//	Os::CriticalSectionManager -- クリティカルセクションを管理するクラス
//
//	NOTES
//  Linux のみで有効
//  ロック中か否かにかかわらず存在するクリティカルセクションインスタンスをすべて管理する

class CriticalSectionManager
{
public:
	typedef boost::unordered_set<const CriticalSection *> CriticalSectionSet;

	// クリティカルセクションを管理マップに登録する
	static void add(const CriticalSection* pCriticalSection);

	// クリティカルセクションを管理マップから除く
	static void remove(const CriticalSection* pCriticalSection);

	// クリティカルセクションのログを出力する
	static void printOut();

private:

	// ミューテックスを得る
	static boost::mutex& getMutex();

	// インスタンスを得る
	static CriticalSectionManager* getInstance();

	// ログの出力先パラメータを読み込む
	static bool loadOutputPath(ModUnicodeString& cstrPath);

	// 出力先のパスが有効か調べる
	static void checkLogPath(const ModUnicodeString& cstrPath);

	// コンストラクタ
	CriticalSectionManager(ModUInt32 iTableSize);

	// ログを出力する
	void printOut(const ModUnicodeString& cPath, const ModCharString& cTime);

	// クリティカルセクションを管理するハッシュセット
	CriticalSectionSet m_cCriticalSectionSet;
};

#endif	// __TRMEISTER_OS_CRITICALSECTIONMANAGER_H

_TRMEISTER_OS_END
_TRMEISTER_END

//
// Copyright (c) 2018, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
