// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenMP.h --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_UTILITY_OPENMP_H
#define __SYDNEY_UTILITY_OPENMP_H

#include "Utility/Module.h"

#include "Os/AutoCriticalSection.h"
#include "Exception/Object.h"

_TRMEISTER_BEGIN
_TRMEISTER_UTILITY_BEGIN

//
//	CLASS
//	Utility::OpenMP -- 
//
//	NOTES
//	本クラスは OpenMP による並列処理実行時の例外を処理するクラスである
//
class SYD_UTILITY_FUNCTION OpenMP
{
public:
	// コンストラクタ
	OpenMP();
	// デストラクタ
	virtual ~OpenMP();

	// マルチスレッドで実行するメソッドを実行する
	void run(bool bParallel_ = true);

	// マルチスレッド実行の前処理を行うメソッド
	//
	// シングルスレッドで実行される
	// 必要ならサブクラスで実装する
	//
	virtual void prepare();
	
	// マルチスレッドで実行するメソッド
	//
	virtual void parallel() = 0;
	
	// マルチスレッド実行の後処理を行うメソッド
	//
	// シングルスレッドで実行される
	// 必要ならサブクラスで実装する
	//
	virtual void dispose();

	//
	// 以下のメソッドは、prepare(), parallel(), dispose() 内でのみ利用可能
	//
	
	// 現在のスレッドチームの全スレッド数を得る
	int getThreadSize();
	// 自スレッドのスレッド番号を得る
	int getThreadNum();

protected:
	// 例外が発生しているか
	bool isException();
	// 例外を設定する
	void setException(const Exception::Object& e);

	// メンバ変数排他制御用のラッチ(サブクラスで利用してもいい)
	Os::CriticalSection m_cLatch;

private:
	// 関数を実行する
	void runPrepare();
	void runParallel();
	void runDispose();
	
	// 例外が発生していたら再送する
	void rethrowException();

	// 例外を保存する
	Exception::Object m_cException;
	// 例外が発生したかどうか
	bool m_bException;
	// 例外が発生したスレッドのスレッドID(表示用)
	unsigned int m_uiMessageThreadID;
};

_TRMEISTER_UTILITY_END
_TRMEISTER_END

#endif //__TRMEISTER_UTILITY_OPENMP_H

//
//  Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
