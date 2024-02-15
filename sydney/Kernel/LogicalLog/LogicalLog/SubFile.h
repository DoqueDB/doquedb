// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SubFile.h -- 論理ログサブファイル関連のクラス定義、関数宣言
// 
// Copyright (c) 2009, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOGICALLOG_SUBFILE_H
#define __SYDNEY_LOGICALLOG_SUBFILE_H

#include "LogicalLog/Module.h"
#include "LogicalLog/LSN.h"
#include "LogicalLog/File.h"
#include "LogicalLog/VersionNumber.h"

#include "Buffer/File.h"
#include "Common/Object.h"
#include "Os/Memory.h"

_SYDNEY_BEGIN

namespace Buffer {
	class Page;
}

_SYDNEY_LOGICALLOG_BEGIN

class Log;
class File;

namespace Format {
	struct LogHeader;
}

//	CLASS
//	LogicalLog::SubFile -- 論理ログサブファイル記述子
//
//	NOTES

class SubFile
	: public	Common::Object
{
public:
	enum {
		// ローテートが無効であることを表現する数
		NoRotate = -1,
		// ローテートがまだ一度も実施されていないことを表現する数
		RotateNone = 0
	};
	
	// コンストラクター
	SubFile(const Os::Path& path, Os::Memory::Size pageSize,
			bool mounted, bool readOnly,
			Os::File::Size extensionSize, int subfile);
	// デストラクター
	~SubFile();

	// 生成する
	void					create(LSN top, LSN last, LSN master,
								   VersionNumber::Value v);
	// 破棄する
	void					destroy();
	// マウントする
	void					mount();
	// アンマウントする
	void					unmount();
	// トランケートする
	bool					truncate(VersionNumber::Value v);
	// ローテートする
	int						rotate();
	// 回復処理を行う
	void					recover();
	
	// 実体である OS ファイルの名前を変更する
	void					rename(const Os::Path& path);

	// 先頭のログシーケンス番号を得る
	LSN						getFirstLSN();
	// 末尾のログシーケンス番号を得る
	LSN						getLastLSN();
	// 次のログシーケンス番号を得る
	LSN						getNextLSN();
	// 次のログシーケンス番号を得る
	LSN						getNextLSN(LSN lsn);
	// 前のログシーケンス番号を得る
	LSN						getPrevLSN(LSN lsn);
	// マスター側のログシーケンス番号を得る
	LSN						getMasterLSN();

	// ある論理ログを読み出す
	const Log*				read(LSN lsn);
	// ログを書く
	LSN						append(Log& log, LSN masterLSN);
	// 指定された論理ログまでフラッシュする
	void					flush(LSN lsn = NoLSN);

	// 実体である OS ファイルの絶対パス名を得る
	const Os::Path&			getPath() const;
	// ページサイズを得る
	Os::Memory::Size		getPageSize() const;

	// 構成する OS ファイルが存在するか調べる
	bool					isAccessible() const;
	// マウントされているか
	bool					isMounted() const;
	// 読取専用か
	bool					isReadOnly() const;

	// バージョンを得る
	VersionNumber::Value 	getVersion();
	// ローテート番号を得る
	int						getRotate();
	
	// 同期処理が完了しているかどうか
	bool					isSyncDone();
	// 同期処理が完了しているかを設定する
	bool					setSyncDone(bool done_);

	// ローテートを行うべき閾値を超えているかどうか
	bool					isRotateThreshold();

	// マスターのLSNを設定する
	void					setMasterLSN(LSN masterLSN);

private:
	// ログファイルを大きくする
	void					extend(Os::File::Offset offset);

	// 論理ログファイルヘッダを読み出す
	void					readHeader();
	// 論理ログファイルヘッダを書き込む
	void					writeHeader();

	// 論理ログヘッダを読み出す
	LSN						readLogHeader(LSN lsn,
										  Format::LogHeader& logHeader);

	// 次のログシーケンス番号を得る
	LSN						getNextLSN(LSN lsn, Os::Memory::Size size);

	// 論理ログファイルの実体であるバッファファイルのバッファファイル記述子
	Buffer::File*			m_pBufFile;
	// 一度に拡張するサイズ(B 単位)
	Os::File::Size			m_iExtensionSize;
	// 末尾の論理ログのログシーケンス番号
	LSN						m_last;
	// 次に生成される論理ログのログシーケンス番号
	LSN						m_next;
	// フラッシュしていない先頭の論理ログのログシーケンス番号
	LSN						m_flushedLimit;
	// 同期処理が完了しているかどうか
	bool					m_syncDone;
	// このファイルの先頭のログシーケンス番号
	LSN						m_top;
	// バージョン番号
	VersionNumber::Value	m_version;
	// サブファイル番号
	int						m_subfile;
	// マスターのログシーケンス番号
	LSN						m_masterLSN;
};

//	FUNCTION public
//	LogicalLog::SubFile::~SubFile --
//		論理ログファイル記述子を表すクラスのデストラクター
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
SubFile::~SubFile()
{
	// 論理ログファイルの実体である
	// バッファファイルのバッファファイル記述子を破棄する

	Buffer::File::detach(m_pBufFile);
}

//	FUNCTION public
//	LogicalLog::SubFile::rename --
//		論理ログファイルの実体である OS ファイルの名前を変更する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			論理ログファイルの実体である OS ファイルの新しい名前
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
SubFile::rename(const Os::Path& path)
{
	// 論理ログファイルの実体である
	// OS ファイルを与えられた名前に変更する

	m_pBufFile->rename(path);
}

//	FUNCTION public
//	LogicalLog::SubFile::getLastLSN -- 末尾のログシーケンス番号を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		LogicalLog::NoLSN 以外の値
//			得られたログシーケンス番号
//		LogicalLog::NoLSN
//			ひとつも論理ログが格納されていない
//	EXCEPTIONS

inline
LSN
SubFile::getLastLSN()
{
	// 必要があれば、論理ログファイルヘッダを読み出す

	readHeader();

	return m_last;
}

//	FUNCTION public
//	LogicalLog::SubFile::getNextLSN -- 次のログシーケンス番号を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		LogicalLog::NoLSN 以外の値
//			得られたログシーケンス番号
//		LogicalLog::NoLSN
//			ひとつも論理ログが格納されていない
//	EXCEPTIONS

inline
LSN
SubFile::getNextLSN()
{
	// 必要があれば、論理ログファイルヘッダを読み出す

	readHeader();

	return m_next;
}

//	FUNCTION public
//	LogicalLog::SubFile::getPath -- 実体である OS ファイルの絶対パス名を得る
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
SubFile::getPath() const
{
	return m_pBufFile->getPath();
}

//	FUNCTION public
//	LogicalLog::SubFile::getPageSize -- ページサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたページサイズ(B 単位)
//
//	EXCEPTIONS
//		なし

inline
Os::Memory::Size
SubFile::getPageSize() const
{
	return m_pBufFile->getPageSize();
}

//	FUNCTION public
//	LogicalLog::SubFile::isAccessible --
//		論理ログファイルを構成する OS ファイルが存在するか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			存在する
//		false
//			存在しない
//
//	EXCEPTIONS

inline
bool
SubFile::isAccessible() const
{
	return m_pBufFile->isAccessible();
}

//	FUNCTION public
//	LogicalLog::SubFile::isMounted --
//		論理ログファイルがマウントされているか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			マウントされている
//		false
//			マウントされていない
//
//	EXCEPTIONS

inline
bool
SubFile::isMounted() const
{
	//【注意】	マウントされかつ構成する OS ファイルが存在することを確認する

	return m_pBufFile->isMountedAndAccessible();
}

//	FUNCTION public
//	LogicalLog::SubFile::isReadOnly -- 論理ログファイルは読取専用か
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			読取専用である
//		false
//			読取専用でない
//
//	EXCEPTIONS
//		なし

inline
bool
SubFile::isReadOnly() const
{
	return m_pBufFile->isReadOnly();
}

//	FUNCTION public
//	LogicalLog::SubFile::getVersion -- バージョン番号を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		LogicalLog::VersionNumber::Value
//			バージョン番号
//	EXCEPTIONS

inline
VersionNumber::Value
SubFile::getVersion()
{
	// 必要があれば、論理ログファイルヘッダを読み出す

	readHeader();

	return m_version;
}

//	FUNCTION public
//	LogicalLog::SubFile::getRotate -- ローテート番号を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		int
//			ローテート番号
//	EXCEPTIONS

inline
int
SubFile::getRotate()
{
	// 必要があれば、論理ログファイルヘッダを読み出す

	readHeader();

	return m_subfile;
}

_SYDNEY_LOGICALLOG_END
_SYDNEY_END

#endif //__SYDNEY_LOGICALLOG_SUBFILE_H

//
//	Copyright (c) 2009, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
