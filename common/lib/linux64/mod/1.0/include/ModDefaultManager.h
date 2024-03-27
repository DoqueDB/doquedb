// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModDefaultManager.h -- ModDefaultManager のクラス定義
// 
// Copyright (c) 1997, 2002, 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModDefaultManager_H__
#define __ModDefaultManager_H__

#include "ModCommonDLL.h"
#include "ModTypes.h"
#include "ModMemoryHandle.h"
#include "ModManager.h"
#include "ModObject.h"

//
// CLASS
// ModDefaultMemoryHandle -- デフォルトで利用されるメモリハンドルクラス
// NOTES
// デフォルトで利用されるメモリハンドルクラスである。
// 全体で使うメモリハンドルである。
// この場合はメモリハンドルとモジュール名クラスの派生クラスであり、
// メモリ管理用ネゴシエーション関数、マネージャ名を返す関数を実装している。
// メモリハンドル自身のメモリは特別にModMemoryHandleで定義されているため
// ModDefaultObjectのサブクラスとしてはいけない。
// 

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModDefaultMemoryHandle
	: public	ModMemoryHandle
{
public:
    // コンストラクタ
    ModDefaultMemoryHandle();
    ModDefaultMemoryHandle(ModSize limitSize_);
    // デストラクタ
	ModCommonDLL
    ~ModDefaultMemoryHandle();
    // マネージャ名を返す
    const char* getName() const;

private:
	ModCommonDLL
    static const char* name;
};

//
// FUNCTION public
// ModDefaultMemoryHandle::ModDefaultMemoryHandle -- コンストラクタ
//
// NOTES
//	デフォルトメモリハンドルのデフォルトコンストラクタである。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	その他
//		ModMemoryHandle::ModMemoryHandleの例外参照(初期化前のみModCommonInitialize::checkAndInitializeの例外、等)
//

inline
ModDefaultMemoryHandle::ModDefaultMemoryHandle()
{ }

//
// FUNCTION public
// ModDefaultMemoryHandle::ModDefaultMemoryHandle -- デフォルトメモリハンドルコンストラクタ
//
// NOTES
//	デフォルトメモリハンドルのコンストラクタである。
//	引数に、モジュールごとのメモリ獲得の限界値を指定する。
//
// ARGUMENTS
//	ModSize limitSize_
//		メモリ割り当ての限界値
//
// RETURN
//	なし
//
// EXCEPTIONS
//	その他
//		ModMemoryHandle::ModMemoryHandleの例外参照(初期化前のみModCommonInitialize::checkAndInitializeの例外、等)
//

inline
ModDefaultMemoryHandle::ModDefaultMemoryHandle(ModSize limitSize_)
	: ModMemoryHandle(limitSize_)
{ }

//
// FUNCTION public
// ModDefaultMemoryHandle::getName -- モジュールマネージャ名を返す
//
// NOTES
//     	デフォルトマネージャの名前をもつ静的領域へのポインタを返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	デフォルトマネージャという名前へのポインタ
//
// EXCEPTIONS
//	なし
//

inline
const char*
ModDefaultMemoryHandle::getName() const
{
    return this->name;
}

// CLASS
// ModDefaultManager -- デフォルトのメモリハンドルを管理する仮想的マネージャクラス
// NOTES
//	デフォルトで利用する仮想的なマネージャである。
//	MOD全体で利用するデフォルトメモリハンドルModDefaultMemoryHandleを管理する
//	従来はModManager <ModDefaultMemoryHandle>のままであったが、
//	1回だけ呼び出して初期化の処理を行なうdoInitializeを用意するために、
//	クラスに変更した。
//	他から自由に呼び出せるinitializeは、内部ではModCommonInitializeを
//	呼び出しているだけで何回でも呼び出せるが、doInitializeは一回だけ
//	呼び出されるようにCommonInitializeが制御している。

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

class ModDefaultManager
	: public	ModManager<ModDefaultMemoryHandle>
{
    friend class ModCommonInitialize;
private:
    static void doInitialize();
    static void doTerminate();
};

//
// TYPEDEF
// ModDefaultObject -- デフォルトのオブジェクトクラス
// NOTES
//	デフォルトで利用するオブジェクトクラスを型定義したもの。
//	MODで作成するクラスは特に理由がない限り、本クラスのサブクラスとして
//	作成する。
#if MOD_CONF_MEMORY_MANAGEMENT == 3
class ModCommonDLL ModDefaultObject
	: public ModObject <ModDefaultManager>
{
public:
	void* operator new(size_t size);
	void* operator new(size_t size, void* address);

	void  operator delete(void* address);
	void  operator delete(void* pointer, void* address);
};
#else
typedef ModObject <ModDefaultManager> ModDefaultObject;
#endif

//
// TYPEDEF
//	ModStandardManager -- ModDefaultManagerの別名を定義する
// NOTES
//	ModDefaultManagerの別名により、汎用標準モジュールでのソース変更を
//	簡単化するための型。
//
typedef ModDefaultManager ModStandardManager;

//
// TYPEDEF
//	ModOsManager -- ModDefaultManagerの別名を定義する
// NOTES
//	ModDefaultManagerの別名により、汎用OSモジュールでのソース変更を
//	簡単化するための型。
//	汎用OSモジュールには、仮想OSドライバのほか、ファイル、ソケット、メモリ、
//	スレッド、ミューテックス、条件変数、アーカイバ、シリアライザなどが
//	ある。
typedef ModDefaultManager ModOsManager;

#endif	// __ModDefaultManager_H__

//
// Copyright (c) 1997, 2002, 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
