// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileSynchronizer.h --
//		バージョンファイル同期スレッドに関するクラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2003, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_CHECKPOINT_FILESYNCHRONIZER_H
#define	__SYDNEY_CHECKPOINT_FILESYNCHRONIZER_H

#include "Checkpoint/Module.h"

#include "Buffer/DaemonThread.h"
#include "Schema/ObjectID.h"

_SYDNEY_BEGIN
_SYDNEY_CHECKPOINT_BEGIN

//	CLASS
//	Checkpoint::FileSynchronizer --
//		バージョンファイル同期スレッドを表すクラス
//
//	NOTES
//		バージョンファイル同期スレッドとは、
//		バージョンファイルごとに、バージョンログファイル中の
//		できる限り新しい版をマスタデータファイルへ上書きし、
//		その版以前のものをすべて破棄する常駐型スレッドである

class FileSynchronizer
	: public	_SYDNEY::Buffer::DaemonThread
{
public:
	// コンストラクター
	FileSynchronizer(unsigned int timeout, bool enable);
	// デストラクター
	~FileSynchronizer();

	// データベースを同期処理の候補にする
	static void
	enter(Schema::ObjectID::Value dbID);

	// 今回の同期処理では実行しないデータベースを登録する
	static void
	skip(Schema::ObjectID::Value dbID);

	// 同期処理の準備をする
	static void
	prepare();

private:
	// スレッドが繰り返し実行する関数
	void					repeatable();
};

//	FUNCTION public
//	Checkpoint::FileSynchronizer::FileSynchronizer --
//		バージョンファイル同期スレッドを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		timeout
//			バージョンファイルの同期の処理の実行間隔(単位ミリ秒)
//		bool				enable
//			true
//				バージョンファイルの同期を可能にする
//			false
//				バージョンファイルの同期を不可にする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
FileSynchronizer::FileSynchronizer(unsigned int timeout, bool enable)
	: _SYDNEY::Buffer::DaemonThread(timeout, enable)
{}

//	FUNCTION public
//	Checkpoint::FileSynchronizer::~FileSynchronizer --
//		バージョンファイル同期スレッドを表すクラスのデストラクター
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
FileSynchronizer::~FileSynchronizer()
{}

_SYDNEY_CHECKPOINT_END
_SYDNEY_END

#endif	// __SYDNEY_CHECKPOINT_FILESYNCHRONIZER_H

//
// Copyright (c) 2001, 2002, 2003, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
