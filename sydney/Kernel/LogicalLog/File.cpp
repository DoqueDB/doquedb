// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.cpp -- 論理ログファイル関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2009, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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
#include "LogicalLog/File.h"
#include "LogicalLog/Format.h"
#include "LogicalLog/Log.h"
#include "LogicalLog/Manager.h"
#include "LogicalLog/SubFile.h"
#include "LogicalLog/VersionNumber.h"

#include "Buffer/AutoPool.h"
#include "Buffer/Memory.h"
#include "Buffer/Page.h"
#include "Checkpoint/Daemon.h"
#include "Common/Assert.h"
#include "Common/Message.h"
#include "Exception/BadDataPage.h"
#include "Exception/LogFileCorrupted.h"
#include "Exception/FakeError.h"

#include "ModAutoPointer.h"
#include "ModOsDriver.h"
#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_LOGICALLOG_USING

namespace
{

namespace _File
{
	//	VARIABLE
	//	$$$::_File::_HeaderName --
	//		ヘッダーファイルのファイル名
	//
	//	NOTES

	ModUnicodeString _HeaderName("LOGICALLOG.SYD");

}

}

//	FUNCTION public
//	Manager::initialize --
//		マネージャーを初期化する
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

// static
void
Manager::initialize()
{}

//	FUNCTION public
//	Manager::terminate --
//		マネージャーの後処理を行う
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

// static
void
Manager::terminate()
{
	Configuration::reset();
}

//	FUNCTION public
//	LogicalLog::File::File --
//		論理ログファイル記述子を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		LogicalLog::File::StorageStrategy&	storageStrategy
//			論理ログファイルのファイル格納戦略
//
//	RETURN
//		なし
//
//	EXCEPTIONS

File::File(const StorageStrategy& storageStrategy)
	: m_pHeader(0), m_pBody(0), m_bodyLSN(NoLSN),
	  m_cSubDirName("LOGICALLOGDIR"),
	  m_cSubFileName("LOGFILE"), m_cSubFileSuffix(".SYD")
{
	// メンバー変数を設定する
	
	m_path = storageStrategy._path;
	m_pageSize = Buffer::Page::correctSize(storageStrategy._pageSize);
	m_category = storageStrategy._category;
	m_extensionSize =
		(storageStrategy._extensionSize + m_pageSize - 1) &	~(m_pageSize - 1);
	m_recoveryFull = storageStrategy._recoveryFull;

	// ヘッダーファイルを設定する
	//
	//【注意】
	//	まだデータファイルは確保しない。

	Os::Path p = m_path;
	p.addPart(_File::_HeaderName);
	m_pHeader = new SubFile(p, m_pageSize,
							storageStrategy._mounted,
							storageStrategy._readOnly,
							m_extensionSize, 0);

}

//	FUNCTION public
//	LogicalLog::File::~File --
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

File::~File()
{
	// 論理ログファイルの実体であるSubFileを破棄する

	detachBody();

	// ヘッダーファイルのインスタンスも破棄する
	
	delete m_pHeader, m_pHeader = 0;
}

//	FUNCTION public
//	LogicalLog::File::create -- 生成する
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
File::create()
{
	// 論理ログのバージョン番号
	
	VersionNumber::Value v = (m_category == Category::System) ?
		VersionNumber::First : VersionNumber::Current;
	
	// ヘッダーファイルのみを作成する

	m_pHeader->create(0, 0, NoLSN, v);

}

//	FUNCTION public
//	LogicalLog::File::destroy -- 破棄する
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
File::destroy()
{
	if (m_pHeader->isMounted() &&
		m_pHeader->getVersion() >= VersionNumber::Second)
	{
		SubFile* p = 0;

		try {
			p = attachBody();
		} catch (...) {}	// エラーは無視する

		while (p != 0)
		{
			// 今のデータファイルを保存

			SubFile* n = p;
			
			// 先頭のLSNを得る

			LSN lsn = p->getFirstLSN();

			// １つ古いデータファイルを得る
			
			try {
				p = 0;
				p = attachBody(lsn - 1);
			} catch (...) {}	// エラーは無視する
			
			// 破棄する
			
			n->destroy();
		}
	}

	// ヘッダーファイルを破棄する

	m_pHeader->destroy();
	
	// データファイルをdetachする
	
	detachBody();
}

//	FUNCTION public
//	LogicalLog::File::mount -- マウントする
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
File::mount()
{
	// ヘッダーファイルをマウントする

	m_pHeader->mount();

	// データファイルをマウントする
	
	if (m_pBody != 0 && m_pHeader != m_pBody)
	{
		m_pBody->mount();
	}

	ModVector<ModPair<LSN, SubFile*> >::Iterator i = m_vecpOld.begin();
	for (; i != m_vecpOld.end(); ++i)

		(*i).second->mount();
}

//	FUNCTION public
//	LogicalLog::File::unmount -- アンマウントする
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
File::unmount()
{
	// データファイルをアンマウントする

	if (m_pBody != 0 && m_pHeader != m_pBody)
	{
		m_pBody->unmount();
	}

	ModVector<ModPair<LSN, SubFile*> >::Iterator i = m_vecpOld.begin();
	for (; i != m_vecpOld.end(); ++i)

		(*i).second->unmount();

	// ヘッダーファイルをアンマウントする
		
	m_pHeader->unmount();

	// データファイルをdetachする
	
	detachBody();
}

//	FUNCTION public
//	LogicalLog::File::rename -- パスを変更する
//
//	NOTES
//		論理ログファイルが格納されている親パスを変更する
//		ファイル名が変わるわけではない
//
//	ARGUMENTS
//		const Os::Path& path
//			親パス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::rename(const Os::Path& path)
{
	if (m_pHeader->getVersion() >= VersionNumber::Second)
	{
		SubFile* p = attachBody();
		int r = m_pHeader->getRotate();

		while (p != 0)
		{
			Os::Path sp;
			makeSubFileName(sp, path, r);

			// リネームする
			
			p->rename(sp);

			// １つ古いデータファイルを得る
			
			p = attachBody(p->getFirstLSN() - 1);
			--r;
		}
	}

	// ヘッダーファイルをrenameする

	Os::Path p = path;
	p.addPart(_File::_HeaderName);
	m_pHeader->rename(p);

	// メンバー変数を変更する
	
	m_path = path;
}

//	FUNCTION public
//	LogicalLog::File::truncate -- トランケートする
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
File::truncate()
{
	// データファイルをトランケートすることはない
	// トランケートされる可能性があるのは、ヘッダーファイルだけである
	
	// 論理ログのバージョン番号
	
	VersionNumber::Value v = (m_category == Category::System) ?
		VersionNumber::First : VersionNumber::Current;
	
	if (m_pHeader->truncate(v) == true && v == VersionNumber::Current)
	{

		// 最新のバージョンなのにトランケートされたということは、
		// このトランケートによって、旧バージョンから新バージョンへ
		// コンバートされたということなので、ローテートを実施する

		if (m_pBody)
		{
			// 新しくデータファイルを作る必要があるので、ポインターをクリアする

			; _SYDNEY_ASSERT(m_pHeader == m_pBody);
			
			m_pBody = 0;
			m_bodyLSN = NoLSN;
		}

		rotate(true);
	}
}

//	FUNCTION public
//	LogicalLog::File::rotate -- ローテートする
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
File::rotate(bool force)
{
	if (m_pHeader->getVersion() < VersionNumber::Second)

		// バージョンが古い場合は、ローテートしない

		return;

	if (!force && 
		(m_pBody == 0 || m_pBody->isRotateThreshold() == false))

		// 強制実行モードではなく、かつ、
		// ログファイルをアタッチしていないか、
		// 論理ログの大きさが閾値に達していない場合は、
		// ローテートは実施しない

		return;

	// 現在のローテート番号を得る
	int n = m_pHeader->getRotate();

	// 次
	++n;

	// ローテートを実施する

	// ローテートするファイルの先頭のLSN

	LSN lsn = 0;
	LSN last = 0;
	LSN masterLSN = NoLSN;

	// まずは、これまでのデータファイルをフラッシュし、
	// 過去ログ配列の先頭に追加する

	if (m_pBody)
	{
		// 次の生成される論理ログのログシーケンス番号
		lsn = m_pBody->getNextLSN();
		// 現在の論理ログの末尾のログシーケンス番号
		last = m_pBody->getLastLSN();
		// マスター側の論理ログのログシーケンス番号
		masterLSN = m_pBody->getMasterLSN();
		// すべてをフラッシュする
		m_pBody->flush();

		// インスタンスをベクターに格納する
		m_vecpOld.pushFront(ModPair<LSN, SubFile*>(m_pBody->getFirstLSN(),
												   m_pBody));
		m_pBody = 0;

		// 古いファイルが３つ以上あるなら、２つ以下になるまで削除する
		while (m_vecpOld.getSize() > 2)
		{
			ModVector<ModPair<LSN, SubFile*> >::Iterator i = m_vecpOld.end();
			--i;

			delete (*i).second;
			m_vecpOld.erase(i);
		}
	}

	; _TRMEISTER_FAKE_ERROR("LogicalLog::File::rotate1", 0);

	// 新しいファイルを確保する

	Os::Path sub;
	makeSubFileName(sub, m_path, n);
	m_pBody = new SubFile(sub, m_pageSize,
						  m_pHeader->isMounted(), m_pHeader->isReadOnly(),
						  m_extensionSize, n);

	// ファイルの実態を作成する

	m_pBody->create(lsn, last, masterLSN, VersionNumber::Current);
	m_bodyLSN = m_pBody->getFirstLSN();
	
	; _TRMEISTER_FAKE_ERROR("LogicalLog::File::rotate2", 0);

	// ヘッダーファイルにローテートしたことを記録する

	m_pHeader->rotate();

	// ローテートしたことをログに出力する
	SydMessage << "Create: " << sub << ModEndl;
}

//	FUNCTION public
//	LogicalLog::File::discard -- 古い論理ログを削除する
//
//	NOTES
//
//	ARGUMENTS
//		LogicalLog::LSN		lsn
//			このログシーケンス番号の表す論理ログが含まれるファイルより、
//			2世代以上古いファイルをすべて削除する
//
//	RETRUN
//		なし
//
//	EXCEPTIONS

void
File::discard(LSN lsn)
{
	if (m_pHeader->getVersion() < VersionNumber::Second)
		
		// 旧バージョンの論理ログはディスカードできない

		return;

	// 必要なファイルを得る
	// このファイルの２つ前から不要
	
	SubFile* p = attachBody(lsn);

	for (int i = 0; i < 2; ++i)
	{
		if (p == 0)
			break;
		
		// 先頭のLSNを得る

		LSN l = p->getFirstLSN();

		// 1つ古いデータファイルを得る

		p = attachBody(l - 1);
	}

	// これより古いものはすべて削除する

	LSN saveLSN = 0;

	while (p != 0)
	{
		// 先頭のLSNを得る

		LSN l = p->getFirstLSN();

		// 取っておく
		
		if (saveLSN == 0) saveLSN = l;
		
		// 破棄する
			
		SydMessage << "Delete: " << p->getPath() << ModEndl;
		p->destroy();

		// １つ古いデータファイルを得る
			
		p = attachBody(l - 1);
	}

	if (saveLSN != 0)
	{
		// m_vecpOld から削除した部分を削除する

		ModVector<ModPair<LSN, SubFile*> >::Iterator i = m_vecpOld.begin();
		ModVector<ModPair<LSN, SubFile*> >::Iterator s = m_vecpOld.end();
		for (; i != m_vecpOld.end(); ++i)
		{
			if ((*i).first > saveLSN)

				// 新しいログなので飛ばす
			
				continue;

			// ここから古いログ
			
			if ((*i).first == saveLSN)

				// 先頭のイテレータを記憶する

				s = i;
			
			// 破棄済みなのでオブジェクトも破棄する
		
			delete (*i).second;
		}

		// ベクターから削除

		m_vecpOld.erase(s, m_vecpOld.end());
	}
}

//	FUNCTION public
//	LogicalLog::File::recover -- 回復処理を実行する
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
File::recover()
{
	m_pHeader->recover();
}

//	FUNCTION public
//	LogicalLog::File::flush -- 指定された論理ログまでフラッシュする
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
File::flush(LSN lsn)
{
	if (m_pBody != 0 && m_pHeader != m_pBody)
	{

		// 外部からのフラッシュ要求時には、
		// ヘッダーファイルをフラッシュする必要はない
		// ヘッダーファイルは適切なタイミングで自らフラッシュを実行している
		// ここでは、末尾のデータファイルのみフラッシュする

		m_pBody->flush(lsn);
	}
	else
	{
		
		// ヘッダーファイルをフラッシュする

		m_pHeader->flush(lsn);
	}
}

//
//	FUNCTION public
//	LogicalLog::File::isSyncDone -- 同期処理が完了しているかどうか
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
File::isSyncDone()
{
	// ヘッダーファイルから取得する
	
	return m_pHeader->isSyncDone();
}

//
//	FUNCTION public
//	LogicalLog::File::setSyncDone -- 同期処理が完了しているかを設定する
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
File::setSyncDone(bool done_)
{
	// ヘッダーファイルに設定する
	
	return m_pHeader->setSyncDone(done_);
}

//	FUNCTION public
//	LogicalLog::File::isAccessible --
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

bool
File::isAccessible() const
{
	return m_pHeader->isAccessible();
}

//	FUNCTION public
//	LogicalLog::File::isMounted --
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

bool
File::isMounted() const
{
	//【注意】	マウントされかつ構成する OS ファイルが存在することを確認する

	return m_pHeader->isMounted();
}

//	FUNCTION public
//	LogicalLog::File::isReadOnly -- 論理ログファイルは読取専用か
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

bool
File::isReadOnly() const
{
	return m_pHeader->isReadOnly();
}

//	FUNCTION public
//	LogicalLog::File::isRecoveryFull -- 回復設定が完全か
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			回復設定が完全である
//		false
//			回復設定がチェックポイントである
//
//	EXCEPTIONS
//		なし

bool
File::isRecoveryFull() const
{
	return m_recoveryFull;
}

//	FUNCTION public
//	LogicalLog::File::setRecoveryFull -- 回復設定を設定する
//
//	NOTES
//
//	ARGUMENTS
//		bool isFull_
//			回復設定を完全にする場合はtrue、
//			回復指定をチェックポイントにする場合はfalseを設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::setRecoveryFull(bool isFull_)
{
	m_recoveryFull = isFull_;
}

//	FUNCTION public
//	LogicalLog::File::getLastLSN -- 末尾のログシーケンス番号を得る
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

LSN
File::getLastLSN()
{
	// 必要ならデータファイルをアタッチする

	SubFile* p = attachBody();
	LSN lastLSN = p->getLastLSN();

	if (lastLSN != NoLSN && p->getFirstLSN() > lastLSN)
	{
		// このデータファイルより前
		// 前のデータファイルがあるかチェックする

		p = attachBody(lastLSN);

		if (p == 0)

			// 存在しない
			lastLSN = NoLSN;
	}

	return lastLSN;
}

//	FUNCTION public
//	LogicalLog::File::getNextLSN -- あるログシーケンス番号の次を求める
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
File::getNextLSN(LSN lsn)
{
	// 必要ならデータファイルをアタッチする

	SubFile* p = attachBody(lsn);

	return p->getNextLSN(lsn);
}

//	FUNCTION public
//	LogicalLog::File::getPrevLSN -- あるログシーケンス番号の前を求める
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
File::getPrevLSN(LSN lsn)
{
	// 必要ならデータファイルをアタッチする

	SubFile* p = attachBody(lsn);

	LSN prevLSN = p->getPrevLSN(lsn);

	if (prevLSN != NoLSN && p->getFirstLSN() > prevLSN)
	{
		// このデータファイルより前
		// 前のデータファイルがあるかチェックする

		p = attachBody(prevLSN);

		if (p == 0)
				
			// 存在しない
			prevLSN = NoLSN;
	}

	return prevLSN;
}

//	FUNCTION public
//	LogicalLog::File::append -- 論理ログを追記する
//
//	NOTES
//
//	ARGUMENTS
//		LogicalLog::Log&	log
//			追記する論理ログ
//
//	RETURN
//		追記された論理ログのログシーケンス番号
//
//	EXCEPTIONS

LSN
File::append(Log& log, LSN masterLSN)
{
	// 必要ならデータファイルをアタッチする

	SubFile* p = attachBody();

	// データファイルに論理ログを追記する
	
	LSN lsn = p->append(log, masterLSN);

	if (p->isRotateThreshold() && Checkpoint::Daemon::isExecuting() == false)
	{
		// 論理ログをローテートする閾値を超えたので、
		// チェックポイントスレッドを起動する

		Checkpoint::Daemon::wakeup(Checkpoint::Daemon::Category::Executor);
	}

	return lsn;
}

//	FUNCTION public
//	LogicalLog::File::read -- 論理ログを読み込む
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
File::read(LSN lsn)
{
	// 必要ならデータファイルをアタッチする

	SubFile* p = attachBody(lsn);

	if (p == 0)
		return 0;

	return p->read(lsn);
}

//	FUNCTION public
//	LogicalLog::File::getVersion -- バージョン番号を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		LogicalLog::VersionNumber::Value
//			バージョン番号
//
//	EXCEPTIONS

VersionNumber::Value
File::getVersion()
{
	return m_pHeader->getVersion();
}

//	FUNCTION public
//	LogicalLog::File::setMasterLSN -- マスター側のLSNを設定する
//
//	NOTES
//
//	ARGUMENTS
//		LSN masterLSN
//			マスター側のLSN
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::setMasterLSN(LSN masterLSN)
{
	//【注意】	マスターデータベースをコピーし、スレーブデータベースとして
	//			マウントする時に利用する
	
	// 必要ならデータファイルをアタッチする

	SubFile* p = attachBody();
	p->setMasterLSN(masterLSN);
}

//	FUNCTION public
//	LogicalLog::File::getMasterLSN -- マスター側のログシーケンス番号を得る
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

LSN
File::getMasterLSN()
{
	// 必要ならデータファイルをアタッチする

	SubFile* p = attachBody();
	return p->getMasterLSN();
}

//	FUNCTION private
//	LogicalLog::File::attachBody -- データファイルをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//		LogicalLog::LSN lsn
//			ログシーケンス番号
//		bool attach
//			データファイルがアタッチされていない場合にアタッチするかどうか
//
//	RETURN
//		サブファイルへのポインター。ファイルが存在していない場合はnull
//
//	EXCEPTIONS

SubFile*
File::attachBody(LSN lsn, bool attach)
{
	// バージョンを確認する
	if (m_pHeader->getVersion() < VersionNumber::Second)
	{
		// 古いバージョンなので、データファイルはヘッダーファイルと同じ
		if (m_pBody == 0) m_pBody = m_pHeader;
		return m_pBody;
	}

	if (m_pBody == 0)
	{
		if (attach == false)

			// 新たな attach は行わない

			return 0;
		
		// 最新のデータファイルを調べる
		// どのローテート番号のファイルをアタッチすればいいか確認

		int r = m_pHeader->getRotate();
		if (r == SubFile::RotateNone)
		{
			// まだローテートが一度も実行されていないので、実行する
				
			rotate(true);
		}
		else
		{
			// 該当するローテート番号のファイルをアタッチする
				
			Os::Path sub;
			makeSubFileName(sub, m_path, r);

			m_pBody = new SubFile(sub, m_pageSize,
								  m_pHeader->isMounted(),
								  m_pHeader->isReadOnly(),
								  m_extensionSize, r);
			m_bodyLSN = m_pBody->getFirstLSN();
		}
	}

	if (lsn == NoLSN || m_bodyLSN <= lsn)
	{
		// NoLSNの場合は、一番新しいデータファイルなので、m_pBody のままでいい
		// m_pBodyの先頭より大きなLSNの場合も m_pBody になる

		return m_pBody;
	}
	else
	{
		// 最新のデータファイルではないので、
		// 過去の論理ログを１つ１つ辿っていく

		int r = m_pHeader->getRotate();
		--r;

		// すでに attach している中にないか確認する

		ModVector<ModPair<LSN, SubFile*> >::Iterator i = m_vecpOld.begin();
		for (; i != m_vecpOld.end(); ++i, --r)
		{
			if ((*i).first <= lsn)

				// このファイルである
				
				return (*i).second;
		}

		if (attach == false)

			// 新たな attach は行わない

			return 0;
		
		// まだ attach していないので、新しいものから順番に attach していく

		while (r != 0)
		{
			// ローテート番号のファイルをアタッチする

			Os::Path sub;
			makeSubFileName(sub, m_path, r);

			// ファイルが存在するかチェックする

			if (Os::File::access(sub, Os::File::AccessMode::File) == false)
				
				// 存在しない

				break;

			ModPair<LSN, SubFile*> e;
			e.second = new SubFile(sub, m_pageSize,
								   m_pHeader->isMounted(),
								   m_pHeader->isReadOnly(),
								   m_extensionSize, r);

			// このファイルの先頭のLSNを求める
			
			e.first = e.second->getFirstLSN();

			m_vecpOld.pushBack(e);

			if (e.first <= lsn)

				// このファイルである

				return e.second;

			// 次へ
			--r;
		}
	}

	// 該当するデータファイルが見つからなかった
		
	return 0;
}

//
//	FUNCTION private
//	LogicalLog::File::detach -- すべてのファイルをdetachする
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::detachBody()
{
	// 論理ログファイルの実体であるSubFileを破棄する
	
	if (m_pHeader == m_pBody) {

		// 古いバージョンの論理ログまたは、システムの論理ログは、
		// ヘッダーファイルとデータファイルが分かれていない
		
		m_pBody = 0;
		
	} else {

		// 新しい論理ログはヘッダーとデータとが別ファイル
		
		if (m_pBody) delete m_pBody, m_pBody = 0;
		ModVector<ModPair<LSN, SubFile*> >::Iterator i = m_vecpOld.begin();
		for (; i != m_vecpOld.end(); ++i)
			delete (*i).second;
		m_vecpOld.clear();
	}
}

//
//	FUNCTION private
//	LogicalLog::File::makeSubFileName -- サブファイル名を作成する
//
//	NOTES
//
//	ARGUMENTS
//	Os::Path& cFileName_
//		作成されたファイル名
//	const Os::Path& cPath_
//		親フォルダーのパス
//	int rotate_
//		ローテート番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::makeSubFileName(Os::Path& cFileName_, const Os::Path& cPath_, int rotate_)
{
	cFileName_ = cPath_;
	cFileName_.addPart(m_cSubDirName);
	ModUnicodeOstrStream s;
	s << m_cSubFileName << rotate_ << m_cSubFileSuffix;
	cFileName_.addPart(s.getString());
}

//	FUNCTION public
//	LogicalLog::File::StorageStrategy::StorageStrategy --
//		ファイル格納戦略を表すクラスのコンストラクター
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

File::StorageStrategy::StorageStrategy()
	: _mounted(true),
	  _readOnly(false),
	  _pageSize(Configuration::PageSize::get()),
	  _extensionSize(Configuration::ExtensionSize::get()),
	  _category(Category::Unknown),
	  _recoveryFull(false)
{}

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2009, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
