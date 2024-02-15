// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SubFile.cpp -- 論理ログサブファイル関連の関数定義
// 
// Copyright (c) 2009, 2012, 2013, 2014, 2015, 2017, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "LogicalLog";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"

#include "LogicalLog/Configuration.h"
#include "LogicalLog/SubFile.h"
#include "LogicalLog/Format.h"
#include "LogicalLog/Log.h"
#include "LogicalLog/Manager.h"
#include "LogicalLog/VersionNumber.h"

#include "Buffer/AutoPool.h"
#include "Buffer/Memory.h"
#include "Buffer/Page.h"
#include "Common/Assert.h"
#include "Common/Message.h"
#include "Common/Thread.h"
#include "Exception/BadDataPage.h"
#include "Exception/LogFileCorrupted.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_LOGICALLOG_USING

namespace
{

namespace _SubFile
{
	namespace _Header
	{
		//	CONST
		//	$$$::_SubFile::_Header::_multiplexCount --
		//		ファイルヘッダを安全のためになん重に記録するか
		//
		//	NOTES

		const unsigned int		_multiplexCount = 2;

	}
}

}

//	FUNCTION public
//	LogicalLog::SubFile::SubFile --
//		論理ログファイル記述子を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const Os::Path& path
//			論理ログサブファイルのパス
//		Os::Memory::Size pageSize
//			ページサイズ
//		bool mounted
//			マウントされているかどうか
//		bool readOnly
//			読み取り専用かどうか
//		Os::File::Size extensionSize
//			拡張サイズ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

SubFile::SubFile(const Os::Path& path, Os::Memory::Size pageSize,
				 bool mounted, bool readOnly,
				 Os::File::Size extensionSize, int rotate)
	: m_syncDone(false)
{
	// 論理ログファイルをバッファリングするための
	// バッファプール記述子を得る
	//
	//【注意】	使用するバッファプールの種別は必ず論理ログデータ用になる

	Buffer::AutoPool pool(
		Buffer::Pool::attach(Buffer::Pool::Category::LogicalLog));

	// 論理ログサブファイルの実体である
	// バッファファイルのバッファファイル記述子を得る
	//
	//【注意】	論理ログサブファイルはページの読み込み時に、
	//			必ず CRC を使った読み込み内容の検証を行う

	m_pBufFile = Buffer::File::attach(
		*pool, path, pageSize, mounted, readOnly, false);
	; _SYDNEY_ASSERT(m_pBufFile);

	// 与えられたエクステンションサイズを切り上げて、
	// ページサイズの倍数にする

	m_iExtensionSize =
		(extensionSize + getPageSize() - 1) & ~(getPageSize() - 1);

	// 残りのメンバーの初期化を行う

	m_last = m_next = m_flushedLimit = NoLSN;
	m_version = VersionNumber::Unknown;
	m_top = NoLSN;
	m_subfile = rotate;
	m_masterLSN = NoLSN;
}

//	FUNCTION public
//	LogicalLog::SubFile::create -- 生成する
//
//	NOTES
//
//	ARGUMENTS
//		LSN top
//			このファイルの先頭のLSN
//		LSN last
//			このファイルの先頭のログの１つ前のLSN
//		LSN masterLSN
//			マスター側のLSN
//		VersionNumber::Value
//			このファイルのバージョン番号
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SubFile::create(LSN top, LSN last, LSN masterLSN, VersionNumber::Value v)
{
	if (top == 0)
		top = getPageSize() * _SubFile::_Header::_multiplexCount;
	
	// 論理ログサブファイルの実体であるバッファファイルを生成する

	m_pBufFile->create(true);

	// バッファファイルに論理ログファイルヘッダを
	// 記録する領域を確保し、初期化する
	
	m_pBufFile->extend(getPageSize() * _SubFile::_Header::_multiplexCount);

	//【注意】	writeHeader時にAllocateでfixしているので、
	//			ここではAllocateでfixしない

	// 次に生成される論理ログのログシーケンス番号を求めておく

	m_next = m_flushedLimit = top;
	m_last = last;
	m_top = top - getPageSize() * _SubFile::_Header::_multiplexCount;
	m_version = v;
	m_masterLSN = masterLSN;

	// ファイルヘッダを書き出す

	writeHeader();
}

//	FUNCTION public
//	LogicalLog::SubFile::destroy -- 破棄する
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

void
SubFile::destroy()
{
	// 論理ログファイルの実体である
	// バッファファイルを破棄する

	m_pBufFile->destroy();

	// 論理ログファイルヘッダ情報を初期化する

	m_next = m_last = m_flushedLimit = NoLSN;
	m_top = NoLSN;
}

//	FUNCTION public
//	LogicalLog::SubFile::mount -- マウントする
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

void
SubFile::mount()
{
	// 論理ログファイルの実体である
	// バッファファイルをマウントする

	m_pBufFile->mount(true);

	// 論理ログファイルヘッダは初期化されているはず

	; _SYDNEY_ASSERT(m_next == NoLSN);
}

//	FUNCTION public
//	LogicalLog::SubFile::unmount -- アンマウントする
//
//	NOTES
//		マウントされていない
//		論理ログファイルをアンマウントしてもエラーにならない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SubFile::unmount()
{
	if (m_pBufFile->isMounted() && !m_pBufFile->isReadOnly())
	
		// 論理ログファイルの内容を書き出す
		
		flush();
	
	// 論理ログファイルの実体である
	// バッファファイルをアンマウントする

	m_pBufFile->unmount();

	// 論理ログファイルヘッダを初期化する

	m_next = m_last = m_flushedLimit = NoLSN;
	m_top = NoLSN;
}

//	FUNCTION public
//	LogicalLog::SubFile::truncate -- トランケートする
//
//	NOTES
//
//	ARGUMENTS
//		LogicalLog::VersionNumber::Value
//			バージョン番号
//
//	RETURN
//		bool
//			トランケートされた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS

bool
SubFile::truncate(VersionNumber::Value v)
{
	bool result = false;
	
	// 必要があれば、論理ログファイルヘッダを読み出す

	readHeader();

	// 新しいバージョンはトランケートしない
	
	if (m_version >= VersionNumber::Second)
		return result;

	// 論理ログが存在していたら、トランケートする
	
	if (m_next != (m_top + getPageSize() * _SubFile::_Header::_multiplexCount)
		|| m_version != v) {

		// 論理ログファイルヘッダ情報を
		// 論理ログがひとつもないことを表すように設定する
		//
		// バージョン情報はトランケート後のバージョンを設定する
		// 旧バージョンの論理ログも一度トランケートされると、
		// 新パージョンにコンバートされる

		m_next = m_flushedLimit =
			getPageSize() * _SubFile::_Header::_multiplexCount;
		m_last = m_top = 0;
		m_version = v;
		m_subfile = 0;
		m_masterLSN = NoLSN;

		// ファイルヘッダを書き出す

		writeHeader();

		result = true;
	}

	// 論理ログファイルヘッダより後の部分をすべてトランケートする

	m_pBufFile->truncate(getPageSize() * _SubFile::_Header::_multiplexCount);

	return result;
}

//	FUNCTION public
//	LogicalLog::SubFile::rotate -- ローテートする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		int
//			サブファイル番号
//
//	EXCEPTIONS

int
SubFile::rotate()
{
	// 必要があれば、論理ログファイルヘッダを読み出す

	readHeader();

	// 古いバージョンはローテートしない
	
	if (m_version < VersionNumber::Second)
		return NoRotate;

	// サブファイル番号をインクリメントする
	++m_subfile;

	// ファイルヘッダを書き出す
	writeHeader();

	return m_subfile;
}

//	FUNCTION public
//	LogicalLog::SubFile::recover -- 回復処理を実行する
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

void
SubFile::recover()
{
#ifdef SYD_OS_WINDOWS
	//【注意】
	//
	//	論理ログのヘッダーページはファイルシステム内のデータ領域に格納され、
	//	ファイルサイズなどのメタ情報はファイルシステム内のメタ領域に格納される
	//	システムが異常終了した場合などに、OSがフラッシュするタイミング等の
	//	問題で、ヘッダーの内容とメタ情報が異なっていることがある
	//
	//	これまで報告されたものの多くは、ヘッダーには論理ログが格納されていると
	//	記録されているが、ファイルはすでにトランケートされているという状況で、
	//	トランケートされているという状況が正しいというものである
	//
	//	そこで、ファイルサイズが8KBで、ヘッダーに論理ログが格納されているという
	//	状況になった場合、ファイルシステムのメタ情報を信じて、
	//	ヘッダーを初期化することとする
	//
	//	なお、VersionNumber::Second 以降の論理ログは、トランケートされることは
	//	はないので、対象外とする
	
	// 必要があれば、論理ログファイルヘッダを読み出す

	readHeader();

	if (m_version == VersionNumber::First &&
		m_pBufFile->getSize()
		== static_cast<Os::File::Size>(
			getPageSize() * _SubFile::_Header::_multiplexCount) &&
		m_last != NoLSN) {

		// 論理ログファイルヘッダ情報を
		// 論理ログがひとつもないことを表すように設定する

		m_next = m_flushedLimit =
			getPageSize() * _SubFile::_Header::_multiplexCount;
		m_last = m_top = 0;

		// ファイルヘッダを書き出す

		writeHeader();
		
		// ログに出力する
		
		SydMessage << "Recovered LogicalLog.(1)" << ModEndl;
	}
	else if (m_version == VersionNumber::First)
	{
		// 上記以外にも最後の方のページが読めないことがある
		// 最後の論理ログが読めなかったら、先頭の論理ログからすべて読み出し、
		// 読めるところまでとする
		//
		// ただし、このリカバリが実行できるのは、古いバージョンの論理ログのみ

		bool recover = false;
		try
		{
			// 最後のログを得る
			ModAutoPointer<const Log> log = read(m_last);
		}
		catch (Exception::BadDataPage&)
		{
			Common::Thread::resetErrorCondition();
			
			// 読めなかった
			recover = true;
		}

		if (recover)
		{
			// 先頭から読み進め、読めるところまで

			LSN nextLSN = getPageSize() * _SubFile::_Header::_multiplexCount;
			LSN lastLSN = NoLSN;

			try
			{
				while (nextLSN < m_next)
				{
					ModAutoPointer<const Log> log = read(nextLSN);
					if (log == 0)
						return;
					
					lastLSN = nextLSN;
					nextLSN = getNextLSN(nextLSN);
				}
			}
			catch (Exception::BadDataPage&)
			{
				Common::Thread::resetErrorCondition();
			
				// ここまでとする
			}

			// ファイルヘッダを書き出す

			m_next = m_flushedLimit = nextLSN;
			m_last = lastLSN;

			writeHeader();
			
			// ログに出力する

			SydMessage << "Recovered LogicalLog.(2)" << ModEndl;
		}
	}
#endif
}

//	FUNCTION private
//	LogicalLog::SubFile::extend -- ログファイルを大きくする
//
//	NOTES
//
//	ARGUMENTS
//		Os::File::Size		offset
//			拡張後のファイルサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SubFile::extend(Os::File::Offset offset)
{
	// 必要があれば、論理ログファイルヘッダを読み出す

	readHeader();

	if (m_pBufFile->getSize() < static_cast<Os::File::Size>(offset)) {

		// 指定されたオフセットを切り上げて、ファイル拡張単位の倍数にする

		offset = (offset + m_iExtensionSize - 1) & ~(m_iExtensionSize - 1);

		// 論理ログファイルの実体であるバッファファイルを拡張する

		m_pBufFile->extend(offset);
	}
}

//	FUNCTION private
//	LogicalLog::SubFile::readHeader -- 論理ログファイルヘッダを読み出す
//
//	NOTES
//		論理ログファイルヘッダを読み出し、
//		論理ログファイル記述子にその情報を設定する
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SubFile::readHeader()
{
	if (m_next != NoLSN)

		// すでに論理ログファイルヘッダは読み出されている

		return;

	unsigned int i = 0;
	do {
		try {
			// 多重化されている論理ログファイルヘッダを読み出す

			const Buffer::Memory& memory =
				Buffer::Page::fix(*m_pBufFile, getPageSize() * i,
								  Buffer::Page::FixMode::ReadOnly);
			const Format::FileHeader& header =
				*static_cast<const Format::FileHeader*>(
					static_cast<const void*>(memory));

			if (header._versionNumber > VersionNumber::Current)
			{
				// 自分が知っている最新バージョンより新しいものは読めない
				
				_SYDNEY_THROW0(Exception::LogFileCorrupted);
			}
			
			// 正常なものを読み出せれば、ファイルヘッダ情報を記述子に設定する

			m_next = m_flushedLimit = header._next;
			m_last = header._last;
			m_version = header._versionNumber;
			m_syncDone = header._fileSyncDone;
			m_subfile = header._subfile;
			m_top = header._top;
			
			if (m_version >= VersionNumber::Third)
			{
				// マスターのLSNが格納されているのはV3以降
				
				m_masterLSN = header._masterLSN;
			}
			else 
			{
				m_masterLSN = NoLSN;
			}

			return;

		} catch (Exception::BadDataPage&) {

			// このファイルヘッダは正しく記録されていないので、
			// 次のファイルヘッダを読み出せるか調べてみる

			continue;

		} catch (...) {
			_SYDNEY_RETHROW;
		}
	} while (++i < _SubFile::_Header::_multiplexCount) ;

	// 多重化されているファイルヘッダがすべて正しく記録されていなかった

	_SYDNEY_THROW0(Exception::LogFileCorrupted);
}

//	FUNCTION private
//	LogicalLog::SubFile::writeHeader -- 論理ログファイルヘッダを書き込む
//
//	NOTES
//		論理ログファイル記述子に保持するファイルヘッダ情報を
//		論理ログファイルヘッダへ書き込む
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SubFile::writeHeader()
{
	unsigned int i = 0;
	do {
		// ファイルヘッダを記録しているバッファページをフィックスする
		//
		//【注意】	読み出し時には、多重化されているページのうち、１ページ
		//			読めればOKである。しかし、書き出し時にFixModeをWriteで、
		//			fixすると、現在のページを読み込んでしまい、結果として、
		//			多重化されているページすべてが正しく読み込める必要があった
		//			
		//			そこで、FixModeをWriteからAllocateに変更し、
		//			現在のページの内容は読み込まずに上書きするように変更した

		Buffer::Memory	memory(
			Buffer::Page::fix(*m_pBufFile, getPageSize() * i,
							  Buffer::Page::FixMode::Allocate));

		// バッファページ上の
		// ファイルヘッダ情報を現状に合わせて更新する

		Format::FileHeader& header =
			*static_cast<Format::FileHeader*>(static_cast<void*>(memory));

		Os::Memory::reset(&header, sizeof(header));

		if (m_version == VersionNumber::Second)
		{
			// 一度ヘッダーをフラッシュすると、バージョンアップする
			
			m_version = VersionNumber::Third;
		}

		header._versionNumber = m_version;
		header._last = m_last;
		header._next = m_flushedLimit;
		header._fileSyncDone = m_syncDone;
		header._subfile = m_subfile;
		header._top = m_top;
		header._masterLSN = m_masterLSN;

		// 同期的にアンフィックスする
		//
		//【注意】	多重化されているファイルヘッダを、
		//			ひとつひとつ完全にディスクへ書き出してから、
		//			次のファイルヘッダをディスクへ書き出す
		//
		//			そのため、少なくともひとつは整合性の保証された
		//			ファイルヘッダが残る(最新でない可能性はある)

		memory.unfix(true, false);

	} while (++i < _SubFile::_Header::_multiplexCount) ;
}

//	FUNCTION private
//	LogicalLog::SubFile::readLogHeader -- ある論理ログのヘッダを読み出す
//
//	NOTES
//
//	ARGUMENTS
//		LogicalLog::LSN		lsn
//			ヘッダを読み出す論理ログのログシーケンス番号
//		LogicalLog::Format::LogHeader&	logHeader
//			読み出したヘッダ
//
//	RETURN
//		データが記録されている領域の先頭をログシーケンス番号の形式で表したもの
//
//	EXCEPTIONS

LSN
SubFile::readLogHeader(LSN lsn, Format::LogHeader& logHeader)
{
	// このファイルの先頭に補正する
	lsn -= m_top;
	
	// 論理ログファイルのページサイズを求める

	const Os::Memory::Size	pageSize = getPageSize();

	// 指定されたログシーケンス番号の論理ログが記録されている
	// バッファページのうち、先頭のもののファイル内オフセットと、
	// 先頭のもののどこから論理ログが記録されているか求める

	const Os::Memory::Size start =
		static_cast<Os::Memory::Size>(lsn % pageSize);
	Os::File::Offset pageOffset = lsn - start;

	// 得られたオフセットのバッファページをフィックスする

	const Buffer::Memory& memory0 =
		Buffer::Page::fix(*m_pBufFile, pageOffset,
						  Buffer::Page::FixMode::ReadOnly);

	if (start + sizeof(Format::LogHeader) <= memory0.getSize()) {

		// 論理ログヘッダが先頭のページに存在するので、
		// 先頭のページから複写する

		(void) Os::Memory::copy(&logHeader,
								static_cast<const char*>(memory0) + start,
								sizeof(Format::LogHeader));

		return pageOffset + start + sizeof(Format::LogHeader) + m_top;
	}

	// 論理ログヘッダが先頭と次のページにまたがっているので、
	// まず、次のページをフィックスする

	const Buffer::Memory& memory1 =
		Buffer::Page::fix(*m_pBufFile, pageOffset += pageSize,
						  Buffer::Page::FixMode::ReadOnly);

	// 先頭と次のページから複写する

	const Os::Memory::Size size = memory0.getSize() - start;
	(void) Os::Memory::copy(&logHeader,
							static_cast<const char*>(memory0) + start, size);
	(void) Os::Memory::copy(syd_reinterpret_cast<char*>(&logHeader) + size,
							memory1, sizeof(Format::LogHeader) - size);

	return pageOffset + sizeof(Format::LogHeader) - size + m_top;
}

//	FUNCTION public
//	LogicalLog::SubFile::flush -- 指定された論理ログまでフラッシュする
//
//	NOTES
//
//	ARGUMENTS
//		LogicalLog::LSN		lsn
//			ファイルの先頭からこのログシーケンス番号の表す
//			論理ログまでフラッシュする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SubFile::flush(LSN lsn)
{
	// 必要があれば、論理ログファイルヘッダを読み出す

	readHeader();

	// 指定されたログシーケンス番号の次のログシーケンス番号を得る

	LSN limit = getNextLSN(lsn);
	if (limit == NoLSN)

		// 指定されたログシーケンス番号は有効なものでないので、
		// 次に割り振られるログシーケンス番号を求めておく

		limit = m_next;

	// これまでフラッシュ済の論理ログを超えて
	// フラッシュしようとしているか調べる

	if (m_flushedLimit < limit) {

		// 求めたログシーケンス番号が表す位置を含む
		// バッファページまでフラッシュする

		const Os::Memory::Size	pageSize = getPageSize();
		m_pBufFile->flush((limit - m_top) + (pageSize - 1) & ~(pageSize - 1));

		// ヘッダ情報を更新する

		m_flushedLimit = limit;
	}

	// ファイルヘッダを書き出す

	writeHeader();
}

//	FUNCTION public
//	LogicalLog::SubFile::getFirstLSN -- 先頭のログシーケンス番号を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		LogicalLog::NoLSN 以外の値
//			得られたログシーケンス番号
//	EXCEPTIONS

LSN
SubFile::getFirstLSN()
{
	// 必要があれば、論理ログファイルヘッダを読み出す

	readHeader();

	return m_top + getPageSize() * _SubFile::_Header::_multiplexCount;
}

//	FUNCTION public
//	LogicalLog::SubFile::getNextLSN -- あるログシーケンス番号の次を求める
//
//	NOTES
//
//	ARGUMENTS
//		LogicalLog::LSN		lsn
//			このログシーケンス番号の次を求める
//
//	RETURN
//		得られたログシーケンス番号
//
//	EXCEPTIONS

LSN
SubFile::getNextLSN(LSN lsn)
{
	if (lsn == NoLSN)
		return NoLSN;

	// 必要があれば、論理ログファイルヘッダを読み出す

	readHeader();

	if (lsn < getFirstLSN() || lsn > m_last)

		// 指定されたログシーケンス番号は存在しない論理ログを表している

		return NoLSN;

	// 指定されたログシーケンス番号の論理ログのヘッダを読み出す

	Format::LogHeader	logHeader;
	(void) readLogHeader(lsn, logHeader);

	return getNextLSN(lsn, logHeader._size);
}

//	FUNCTION private
//	LogicalLog::SubFile::getNextLSN -- あるログシーケンス番号の次を求める
//
//	NOTES
//
//	ARGUMENTS
//		LogicalLog::LSN		lsn
//			このログシーケンス番号の次を求める
//		Os::Memory::Size	size
//			次を求めるログシーケンス番号の表す
//			論理ログのデータ部分のサイズ(B 単位)
//
//	RETURN
//		得られたログシーケンス番号
//
//	EXCEPTIONS
//		なし

LSN
SubFile::getNextLSN(LSN lsn, Os::Memory::Size size)
{
	// このファイルの先頭に補正する

	lsn -= m_top;
	
	// 論理ログファイルのページサイズと
	// ページに格納可能なデータのサイズを求める

	const Os::Memory::Size	pageSize = getPageSize();
	const Os::Memory::Size	contentSize =
		Buffer::Page::getContentSize(pageSize);

	// 指定されたログシーケンス番号の論理ログが記録されている
	// バッファページのうち、先頭のもののファイル内オフセットと、
	// 先頭のもののどこから論理ログが記録されているか求める

	const Os::Memory::Size start =
		static_cast<Os::Memory::Size>(lsn % pageSize);
	Os::File::Offset pageOffset = lsn - start;

	// 指定されたログシーケンス番号の論理ログが記録されている
	// 最後の物理ページのファイル内オフセットを求める

	size += start + sizeof(Format::LogHeader);
	for (; size > contentSize; size -= contentSize)
		pageOffset += pageSize;

	return pageOffset + size + m_top;
}

//	FUNCTION public
//	LogicalLog::SubFile::getPrevLSN -- あるログシーケンス番号の前を求める
//
//	NOTES
//
//	ARGUMENTS
//		LogicalLog::LSN		lsn
//			このログシーケンス番号の前を求める
//
//	RETURN
//		得られたログシーケンス番号
//
//	EXCEPTIONS

LSN
SubFile::getPrevLSN(LSN lsn)
{
	if (lsn == NoLSN)
		return NoLSN;

	// 必要があれば、論理ログファイルヘッダを読み出す

	readHeader();

	if (lsn < getFirstLSN() || lsn > m_last)

		// 指定されたログシーケンス番号は存在しない論理ログを表している

		return NoLSN;

	// 指定されたログシーケンス番号の論理ログのヘッダを読み出す

	Format::LogHeader logHeader;
	(void) readLogHeader(lsn, logHeader);

	return logHeader._prevLSN;
}

//	FUNCTION public
//	LogicalLog::SubFile::append -- 論理ログを追記する
//
//	NOTES
//
//	ARGUMENTS
//		LogicalLog::Log&	log
//			追記する論理ログ
//		LSN					masterLSN
//			マスター側のLSN
//
//	RETURN
//		追記された論理ログのログシーケンス番号
//
//	EXCEPTIONS

LSN
SubFile::append(Log& log, LSN masterLSN)
{
	// 必要があれば、論理ログファイルヘッダを読み出す

	readHeader();

	// 与えられた論理ログを記録するために領域がいくら必要か求める

	const Os::Memory::Size size = sizeof(Format::LogHeader) + log.getSize();
	; _SYDNEY_ASSERT(size);

	// 与えられた論理ログにログシーケンス番号を割り当てる

	log.setLSN(m_next);

	// 論理ログファイルのページサイズと
	// ページに格納可能なデータのサイズを求める

	const Os::Memory::Size	pageSize = getPageSize();
	const Os::Memory::Size	contentSize =
		Buffer::Page::getContentSize(pageSize);

	// 末尾の論理ログが記録されているバッファページのうち、
	// 先頭のもののファイル内オフセットと、
	// 先頭のもののどこから論理ログが記録されているか求める

	Os::Memory::Offset start =
		static_cast<Os::Memory::Offset>((m_next - m_top) % pageSize);
	Os::File::Offset pageOffset = (m_next - m_top) - start;

	// 与えられた論理ログを記録可能なようにファイルを拡張する

	extend(pageOffset + ((start + size - 1) / contentSize + 1) * pageSize);

	// 論理ログファイルの末尾のページをフィックスし、
	// 論理ログのうち、可能なぶんを書き出す

	Buffer::Memory	memory(
		Buffer::Page::fix(*m_pBufFile, pageOffset, (start) ?
						  Buffer::Page::FixMode::Write :
						  Buffer::Page::FixMode::Allocate));

	Os::Memory::Size n =
		log.serialize(
			m_last, static_cast<char*>(memory) + start,
			contentSize - start);

	memory.unfix(true);
	Os::Memory::Size rest =	size - n;

	if (rest) {
		start = 0;

		// 新たなページを確保しつつ、残りのぶんを書き出す

		do {
			Buffer::Memory	memory(
				Buffer::Page::fix(*m_pBufFile, pageOffset += pageSize,
								  Buffer::Page::FixMode::Allocate));

			n = log.serialize(memory, contentSize, size - rest);

			memory.unfix(true);

		} while (rest -= n) ;
	}

	// 新しい最後と次のログシーケンス番号を求める

	m_last = m_next;
	m_next = pageOffset + start + n + m_top;
	if (masterLSN != NoLSN)
		m_masterLSN = masterLSN;

	return m_last;
}

//	FUNCTION public
//	LogicalLog::SubFile::read -- 論理ログを読み込む
//
//	NOTES
//
//	ARGUMENTS
//		LogicalLog::LSN		lsn
//			読み込む論理ログのログシーケンス番号
//
//	RETURN
//		0 以外の値
//			読み込んだ論理ログを表すクラスを格納する領域の先頭アドレス
//		0
//			指定されたログシーケンス番号の論理ログは存在しない
//
//	EXCEPTIONS

const Log*
SubFile::read(LSN lsn)
{
	if (lsn == NoLSN)
		return 0;

	// 必要があれば、論理ログファイルヘッダを読み出す

	readHeader();

	if (lsn < getFirstLSN() || lsn > m_last)

		// 指定されたログシーケンス番号は存在しない論理ログを表している

		return 0;

	// ログヘッダをまず読み出す

	Format::LogHeader logHeader;
	LSN dataLSN = readLogHeader(lsn, logHeader);

	// このファイルの先頭に補正する

	dataLSN -= m_top;

	// 読み出す論理ログを格納する領域を確保する

	ModAutoPointer<Log> log(new Log(logHeader._size));

	// 読み出す論理ログのログシーケンス番号を設定する

	log->setLSN(lsn);

	// 論理ログファイルのページサイズと
	// ページに格納可能なデータのサイズを求める

	const Os::Memory::Size	pageSize = getPageSize();
	const Os::Memory::Size	contentSize =
		Buffer::Page::getContentSize(pageSize);

	// 指定されたログシーケンス番号の論理ログのデータが記録されている
	// バッファページのうち、先頭のもののファイル内オフセットと、
	// 先頭のもののどこからデータが記録されているか求める

	const Os::Memory::Offset start =
		static_cast<Os::Memory::Offset>(dataLSN % pageSize);
	Os::File::Offset pageOffset = dataLSN - start;

	// データが記録されている先頭のページをフィックスし、
	// データのうち、記録されているものを読み込む

	char* p = *log;
	const char* end = p + logHeader._size;

	const Buffer::Memory& memory =
		Buffer::Page::fix(*m_pBufFile, pageOffset,
						  Buffer::Page::FixMode::ReadOnly);

	Os::Memory::Size size = static_cast<Os::Memory::Size>(end - p);
	if (size > contentSize - start)
		size = contentSize - start;

	(void) Os::Memory::copy(p, static_cast<const char*>(memory) + start, size);

	// 次のページをフィックスして、残りの部分を読み込む

	while ((p += size) < end) {
		const Buffer::Memory& memory =
			Buffer::Page::fix(*m_pBufFile, pageOffset += pageSize,
							  Buffer::Page::FixMode::ReadOnly);

		size = static_cast<Os::Memory::Size>(end - p);
		if (size > contentSize)
			size = contentSize;

		(void) Os::Memory::copy(p, memory, size);
	}

	return log.release();
}

//
//	FUNCTION public
//	LogicalLog::SubFile::isSyncDone -- 同期処理が完了しているかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		完了している場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
SubFile::isSyncDone()
{
	// 必要があれば、論理ログファイルヘッダを読み出す
	readHeader();

	return m_syncDone;
}

//
//	FUNCTION public
//	LogicalLog::SubFile::setSyncDone -- 同期処理が完了しているかを設定する
//
//	NOTES
//
//	ARGUMENTS
//	bool
//		完了している場合はtrue、それ以外の場合はfalse
//
//	RETURN
//	bool
//		値が変更された場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
SubFile::setSyncDone(bool done_)
{
	// 必要があれば、論理ログファイルヘッダを読み出す

	readHeader();

	bool updated = false;
	if (m_syncDone != done_)
	{
		// フラグが異なっている
		
		updated = true;
		m_syncDone = done_;

		// フラッシュする

		flush();
	}

	return updated;
}

//	FUNCTION public
//	LogicalLog::SubFile::isRotateThreshold
//		-- ローテートを行うべき閾値を超えているかどうか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		bool
//		  	閾値を超えている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
SubFile::isRotateThreshold()
{
	// 必要があれば、論理ログファイルヘッダを読み出す

	readHeader();

	Os::File::Size t = Configuration::RotateThreshold::get();

	return (m_version >= VersionNumber::Second && t != 0) ?
		((m_next - m_top) >= t) : false;
}

//
//	FUNCTION public
//	LogicalLog::SubFile::setMasterLSN
//		-- マスター側のLSNを設定する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SubFile::setMasterLSN(LSN masterLSN)
{
	// 必要があれば、論理ログファイルヘッダを読み出す

	readHeader();

	m_masterLSN = masterLSN;
}

//
//	FUNCTION public
//	LogicalLog::SubFile::getMasterLSN
//		-- マスター側のLSNを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//		LSN
//			マスター側のLSN
//
//	EXCEPTIONS
//
LSN
SubFile::getMasterLSN()
{
	// 必要があれば、論理ログファイルヘッダを読み出す

	readHeader();

	return m_masterLSN;
}

//
//	Copyright (c) 2009, 2012, 2013, 2014, 2015, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
