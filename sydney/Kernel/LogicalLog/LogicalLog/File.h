// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.h -- 論理ログファイル関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2009, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOGICALLOG_FILE_H
#define __SYDNEY_LOGICALLOG_FILE_H

#include "LogicalLog/Module.h"
#include "LogicalLog/LSN.h"
#include "LogicalLog/VersionNumber.h"

#include "Buffer/File.h"
#include "Common/Object.h"
#include "Os/Memory.h"

#include "ModVector.h"

_SYDNEY_BEGIN

namespace Buffer {
	class Page;
}

_SYDNEY_LOGICALLOG_BEGIN

class Log;
class File;
class SubFile;

namespace Format {
	struct FileHeader;
	struct LogHeader;
}

//	CLASS
//	LogicalLog::File -- 論理ログファイル記述子
//
//	NOTES

class File
	: public	Common::Object
{
public:
	//	CLASS
	//	LogicalLog::File::Category --
	//		論理ログファイルの種別を表すクラス
	//
	//	NOTES

	struct Category
	{
		//	ENUM
		//	LogicalLog::File::Category::Value --
		//		論理ログファイルの種別を表す値の列挙型
		//
		//	NOTES

		enum Value
		{
			// 不明
			Unknown =		0,
			// システム用
			System,
			// 通常のデータベース用
			Database,
			// 値の数
			ValueNum
		};
	};

	//	CLASS
	//	LogicalLog::File::StorageStrategy --
	//		論理ログファイルのファイル格納戦略を表すクラス
	//
	//	NOTES

	struct StorageStrategy
	{
		// コンストラクター
		SYD_LOGICALLOG_FUNCTION
		StorageStrategy();
		// デストラクター
		~StorageStrategy();

		// マウントされているか
		bool					_mounted;
		// 読取専用か
		bool					_readOnly;
		// バージョンページサイズ(B 単位)
		Os::Memory::Size		_pageSize;
		// 実体である OS ファイルの絶対パス名
		Os::Path				_path;
		// エクステンションサイズ(B 単位)
		Os::File::Size			_extensionSize;
		// カテゴリ
		Category::Value			_category;
		// 回復指定が完全か
		bool					_recoveryFull;
	};

	// コンストラクター
	SYD_LOGICALLOG_FUNCTION
	File(const StorageStrategy& storageStrategy);
	// デストラクター
	SYD_LOGICALLOG_FUNCTION
	virtual ~File();

	// 生成する
	SYD_LOGICALLOG_FUNCTION
	void					create();
	// 破棄する
	SYD_LOGICALLOG_FUNCTION
	void					destroy();
	// マウントする
	SYD_LOGICALLOG_FUNCTION
	void					mount();
	// アンマウントする
	SYD_LOGICALLOG_FUNCTION
	void					unmount();
	// パスを変更する
	SYD_LOGICALLOG_FUNCTION
	void					rename(const Os::Path& path);
	// トランケートする
	SYD_LOGICALLOG_FUNCTION
	void					truncate();
	// ローテートする
	SYD_LOGICALLOG_FUNCTION
	void					rotate(bool force = false);
	// 古い論理ログを削除する
	SYD_LOGICALLOG_FUNCTION
	void					discard(LSN lsn = NoLSN);
	// 回復処理を行う
	SYD_LOGICALLOG_FUNCTION
	void					recover();
	
	// 末尾のログシーケンス番号を得る
	SYD_LOGICALLOG_FUNCTION
	LSN						getLastLSN();
	// 次のログシーケンス番号を得る
	SYD_LOGICALLOG_FUNCTION
	LSN						getNextLSN(LSN lsn);
	// 前のログシーケンス番号を得る
	SYD_LOGICALLOG_FUNCTION
	LSN						getPrevLSN(LSN lsn);

	// ある論理ログを読み出す
	SYD_LOGICALLOG_FUNCTION
	const Log*				read(LSN lsn);
	// ログを書く
	SYD_LOGICALLOG_FUNCTION
	LSN						append(Log& log, LSN masterLSN);
	// 指定された論理ログまでフラッシュする
	SYD_LOGICALLOG_FUNCTION
	void					flush(LSN lsn = NoLSN);

	// 同期処理が完了しているかどうか
	SYD_LOGICALLOG_FUNCTION
	bool					isSyncDone();
	// 同期処理が完了しているかを設定する
	SYD_LOGICALLOG_FUNCTION
	bool					setSyncDone(bool done_);

	// 論理ログヘッダーファイルの親ディレクトリのパスを得る
	const Os::Path&			getPath() const;

	// 構成する OS ファイルが存在するか調べる
	SYD_LOGICALLOG_FUNCTION
	bool					isAccessible() const;
	// マウントされているか
	SYD_LOGICALLOG_FUNCTION
	bool					isMounted() const;
	// 読取専用か
	SYD_LOGICALLOG_FUNCTION
	bool					isReadOnly() const;

	// 回復設定がチェックポイントか
	SYD_LOGICALLOG_FUNCTION
	bool					isRecoveryFull() const;
	// 回復設定を設定する
	SYD_LOGICALLOG_FUNCTION
	void					setRecoveryFull(bool isFull_);

	// バージョンを得る
	SYD_LOGICALLOG_FUNCTION
	VersionNumber::Value	getVersion();

	// マスターのLSNを設定する
	SYD_LOGICALLOG_FUNCTION
	void					setMasterLSN(LSN masterLSN);
	// マスターのLSNを得る
	SYD_LOGICALLOG_FUNCTION
	LSN						getMasterLSN();

private:
	// データファイルを確保する
	SubFile*				attachBody(LSN lsn = NoLSN, bool attach = true);
	// すべてのデータファイルをdetachする
	void					detachBody();

	// ページサイズを得る
	Os::Memory::Size		getPageSize() const { return m_pageSize; }

	// サブファイル名を作成する
	void makeSubFileName(Os::Path& cFileName_,
						 const Os::Path& cPath_, int rotate_);

	// 論理ログファイルの実体ファイル
	SubFile*				m_pHeader;
	SubFile*				m_pBody;	// 最新
	LSN						m_bodyLSN;	// 最新の先頭のLSN
	ModVector<ModPair<LSN, SubFile*> >
							m_vecpOld;	// 過去

	// パス
	Os::Path				m_path;
	// ページサイズ
	Os::Memory::Size		m_pageSize;
	// ファイル拡張サイズ
	Os::File::Size			m_extensionSize;
	// カテゴリ
	Category::Value			m_category;
	// 回復指定が完全か
	bool					m_recoveryFull;

	// サブファイルのディレクトリ名
	ModUnicodeString		m_cSubDirName;
	// サブファイルのファイル名の固定部分
	ModUnicodeString		m_cSubFileName;
	// サブファイルのサフィックス
	ModUnicodeString		m_cSubFileSuffix;
};

//	FUNCTION public
//	LogicalLog::File::getPath -- 実体である OS ファイルの絶対パス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた絶対パス名
//
//	EXCEPTIONS
//		なし

inline
const Os::Path&
File::getPath() const
{
	return m_path;
}

//	FUNCTION public
//	LogicalLog::File::StorageStrategy::~StorageStrategy --
//		ファイル格納戦略を表すクラスのデストラクター
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

inline
File::StorageStrategy::~StorageStrategy()
{}

_SYDNEY_LOGICALLOG_END
_SYDNEY_END

#endif //__SYDNEY_LOGICALLOG_FILE_H

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2005, 2009, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
