// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VectorFile.cpp -- 
// 
// Copyright (c) 2005, 2006, 2007, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Vector2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Vector2/VectorFile.h"
#include "Vector2/FileID.h"

#include "Common/Assert.h"
#include "Version/Configuration.h"
#include "Version/File.h"

#include "Exception/FileManipulateError.h"
#include "Exception/EntryNotFound.h"

_SYDNEY_USING
_SYDNEY_VECTOR2_USING

//
//	FUNCTION public
//	Vector2::VectorFile::File -- コンストラクタ
//
//	NOTES
//	[?] createには、FileIDが渡されないのでここで設定する。
//	[?] またcreateで、VesrionFileのcreateも呼ばれるのでここで設定する。
//	[?] 他に必要な事は？
//
//	ARGUMENTS
//	const Vector2::FileID&		cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
VectorFile::VectorFile(const FileID& cFileID_)
	: m_cFileID(cFileID_),
	  m_pTransaction(0), m_eFixMode(Buffer::Page::FixMode::Unknown),
	  m_bMounted(false), m_pProgress(0),
	  m_bBatch(false)
{
	m_cPageManager.setVectorFile(this);
	 
	Version::File::StorageStrategy cStorageStrategy;
	Version::File::BufferingStrategy cBufferingStrategy;

	
	// Lob::File::attach()から引用

	//
	//	物理ファイル格納戦略を設定する
	//

	// マウントされているか
	cStorageStrategy._mounted = cFileID_.isMounted();
	// 読み取り専用か
	cStorageStrategy._readOnly = cFileID_.isReadOnly();
	// ページサイズ
	cStorageStrategy._pageSize = cFileID_.getPageSize();

	// パスのコピーはコストがかかるので廃止
	const Os::Path& cPath = cFileID_.getPath();

	// マスタデータファイルの親ディレクトリの絶対パス名
	cStorageStrategy._path._masterData = cPath;
	if (cFileID_.isTemporary() == false)
	{
		// バージョンログファイルの親ディレクトリの絶対パス名
		cStorageStrategy._path._versionLog = cPath;
		// 同期ログファイルの親ディレクトリの絶対パス名
		cStorageStrategy._path._syncLog = cPath;
	}

	// マスタデータファイルの最大ファイルサイズ(B 単位)
	cStorageStrategy._sizeMax._masterData = 0;
	// バージョンログファイルの最大ファイルサイズ(B 単位)
	cStorageStrategy._sizeMax._versionLog = 0;
	// 同期ログファイルの最大ファイルサイズ(B 単位)
	cStorageStrategy._sizeMax._syncLog = 0;

	// マスタデータファイルのエクステンションサイズ(B 単位)
	cStorageStrategy._extensionSize._masterData = 0;
 	// バージョンログファイルのエクステンションサイズ(B 単位)
	cStorageStrategy._extensionSize._versionLog = 0;
	// 同期ログファイルのエクステンションサイズ(B 単位)
	cStorageStrategy._extensionSize._syncLog = 0;
	
 	//
 	//	バッファリング戦略を設定する
 	//
 	if (cFileID_.isTemporary())
 	{
 		// 一時なら
 		cBufferingStrategy._category
 			= Buffer::Pool::Category::Temporary;
 	}
 	else if (cFileID_.isReadOnly())
 	{
 		// 読み取り専用なら
 		cBufferingStrategy._category
 			= Buffer::Pool::Category::ReadOnly;
 	}
 	else
 	{
 		// その他
 		cBufferingStrategy._category
 			= Buffer::Pool::Category::Normal;
 	}

	//	PhysicalFile::File::File()から引用
	m_pVersionFile
		= Version::File::attach(cStorageStrategy,
								cBufferingStrategy,
								m_cFileID.getLockName());

	m_cPageManager.setVersionFile(m_pVersionFile);

	// データクラスを設定する
	m_cData.setType(m_cFileID);
	//@@m_cData.setType(m_cFileID.getFieldType());
}

//
//	FUNCTION public
//	Vector2::VectorFile::~File -- デストラクタ
//
//	NOTES
//		キャッシュしているページの開放
//		バージョンファイル記述子の破棄
//	[?] 他には？
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
VectorFile::~VectorFile()
{
	Version::File::detach(m_pVersionFile, true);
}


//
//	FUNCTION public
//	Vector2::VectorFile::create -- 物理ファイルを生成する
//
//	NOTES
//	物理ファイルを生成する。
//	物理ファイルは、バージョンファイルを用いて実装される。
//	この関数を呼び出すことにより、マスタデータファイルと
//	バージョンログファイルが生成される。
//
//	物理ファイル生成時には、
//		1. 物理ファイルヘッダ
//		2. 先頭の空き領域管理表／物理ページ表
//		3. 先頭の物理ページ
//	の3つのバージョンページが確保される。
//
//	※ 先頭の物理ページはバージョンページ確保するだけで、
//	　 まだ“未使用の物理ページ”である。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::FileManipulateError
//		物理ファイルを生成できなかった
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//

void
VectorFile::create()
{
	create(*m_pTransaction);
}

//
//	FUNCTION public
//	Vector2::VectorFile::destroy --	物理ファイルを消去する
//
//	NOTES
//	物理ファイルをディスク上から消去する。
//	物理ファイルは、バージョンファイルを用いて実装される。
//	この関数を呼び出すことにより、
//	マスタデータファイル、バージョンログファイル、
//	同期ログファイルがディスク上から消去される。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::FileManipulateError
//		物理ファイルを消去できなかった
//
void
VectorFile::destroy(const Trans::Transaction& cTransaction_)
{
	m_pVersionFile->destroy(cTransaction_);
}

//
//	FUNCTION public
//	Vector2::VectorFile::mount -- 物理ファイルをマウントする
//
//	NOTES
//	物理ファイルをマウントする。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
VectorFile::mount(const Trans::Transaction&	cTransaction_)
{
	m_pVersionFile->mount(cTransaction_);
}

//
//	FUNCTION public
//	Vector2::VectorFile::unmount --
//		物理ファイルをアンマウントする
//
//	NOTES
//	物理ファイルをアンマウントする。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
VectorFile::unmount(const Trans::Transaction&	cTransaction_)
{
	m_pVersionFile->unmount(cTransaction_);
	m_bMounted = false;
}

//
//	FUNCTION public
//	Vector2::VectorFile::flush --
//		物理ファイルをフラッシュする
//
//	NOTES
//	物理ファイルをフラッシュする。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
VectorFile::flush(const Trans::Transaction&	cTransaction_)
{
	m_pVersionFile->flush(cTransaction_);
}

//
//	FUNCTION pubilc
//	Vector2::VectorFile::startBackup --
//		物理ファイルに対してバックアップ開始を通知する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const bool					Restorable_ = true
//		版が最新版になるように変更可能とするかどうか
//			true  : バックアップされた内容をリストアしたとき、
//			        あるタイムスタンプの表す時点に開始された
//			        版管理するトランザクションの参照する版が
//			        最新版になるように変更可能にする。
//			false : バックアップされた内容をリストアしたとき、
//			        バックアップ開始時点に障害回復可能にする。
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
VectorFile::startBackup(const Trans::Transaction&	cTransaction_,
						const bool					bRestorable_)
{
	m_pVersionFile->startBackup(cTransaction_, bRestorable_);
}

//
//	FUNCTION pubilc
//	Vector2::VectorFile::endBackup --
//		物理ファイルに対してバックアップ終了を通知する
//
//	NOTES
//	物理ファイルに対してバックアップ終了を通知する。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
VectorFile::endBackup(const Trans::Transaction&	cTransaction_)
{
	m_pVersionFile->endBackup(cTransaction_);
}

//
//	FUNCTION public
//	Vector2::VectorFile::recover --
//		物理ファイルを障害回復する
//
//	NOTES
//	物理ファイルを障害回復する。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const Trans::TimeStamp&		Point_
//		バージョンファイルを戻す時点のタイムスタンプ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
VectorFile::recover(const Trans::Transaction&	cTransaction_,
					const Trans::TimeStamp&		cPoint_)
{
	m_pVersionFile->recover(cTransaction_, cPoint_);
}

//
//	FUNCTION public
//	Vector2::VectorFile::restore --
//		あるタイムスタンプの表す時点に開始された
//		版管理するトランザクションの参照する版が
//		最新版になるようにバージョンファイルを変更する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const Trans::TimeStamp&		Point_
//		このタイムスタンプの表す時点に開始された
//		版管理するトランザクションの参照する版が
//		最新版になるようにバージョンファイルを変更する
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
VectorFile::restore(const Trans::Transaction&	cTransaction_,
					const Trans::TimeStamp&		cPoint_)
{
	m_pVersionFile->restore(cTransaction_, cPoint_);
}

//	FUNCTION public
//	Vector2::VectorFile::sync -- 物理ファイルの同期を取る
//
//	NOTES
//	物理ファイルの同期を取る。
//	==== v12.0(1.0.12.5) 〜 ===========================
//		空き領域管理機能付き物理ファイルまたは
//		物理ページ管理機能付き物理ファイルの場合、
//		Version::File::sync()を呼び出す前に、
//		物理ファイル末尾にある未使用の物理ページを
//		トランケートする。
//	===================================================
//
//	ARGUMENTS
//		Trans::Transaction&	cTransaction
//			物理ファイルの同期を取る
//			トランザクションのトランザクション記述子
//		bool&				incomplete
//			true
//				今回の同期処理で物理ファイルを持つ
//				オブジェクトの一部に処理し残しがある
//			false
//				今回の同期処理で物理ファイルを持つ
//				オブジェクトを完全に処理してきている
//
//				同期処理の結果、物理ファイルを処理し残したかを設定する
//		bool&				modified
//			true
//				今回の同期処理で物理ファイルを持つ
//				オブジェクトの一部が既に更新されている
//			false
//				今回の同期処理で物理ファイルを持つ
//				オブジェクトはまだ更新されていない
//
//				同期処理の結果、物理ファイルが更新されたかを設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	Exception::FileManipulateError
//		物理ファイルの同期を取れなかった
//
void
VectorFile::sync(const Trans::Transaction&	cTransaction_,
				 bool&						incomplete,
				 bool&						modified)
{
	m_pVersionFile->sync(cTransaction_, incomplete, modified);
}

//
//	FUNCTION public
//	Vector2::VectorFile::move --
//		物理ファイルを移動する
//
//	NOTES
//	物理ファイルを移動する。
//
//	ARGUMENTS
//	const Trans::Transaction&					cTransaction_
//		トランザクション記述子への参照
//	const Os::Path&								cPath_
//		
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
VectorFile::move(const Trans::Transaction&	cTransaction_,
				 const Os::Path& 			cPath_)
{
	//Lob(Btree2も同じ)をコピー
	//PhysicalFileからVersionFileへの変更を追加
	// パスを持たないので毎回取得する。
	if (Os::Path::compare(m_cFileID.getPath(), cPath_)
		== Os::Path::CompareResult::Unrelated)
	{
		// ファイルが一時ファイルか調べる
		const bool temporary =
			(m_pVersionFile->getBufferingStrategy()._category
			 == Buffer::Pool::Category::Temporary);

		// 新しいパス名を設定する
		Version::File::StorageStrategy::Path cPath;
		cPath._masterData = cPath_;
		; _SYDNEY_ASSERT(m_pVersionFile);
		if (!temporary) {
			cPath._versionLog = cPath_;
			cPath._syncLog = cPath_;
		}

		m_pVersionFile->move(cTransaction_, cPath);

		//m_cPath = cPath_;
	}
}

//
//	FUNCTION public
//	Vector2::VectorFile::isAccessible --
//		物理ファイルを構成する OS ファイルが存在するか調べる
//
//	NOTES
//
//	ARGUMENTS
//	bool	Force_ = false
//		OSファイルの存在を実際に調べるかどうか
//			true  : OSファイルの存在を実際に調べる
//			false : OSファイルの存在を必要に応じて調べる
//
//	RETURN
//	bool
//		生成されているかどうか
//			true  : 生成されている
//			false : 生成されていない
//
//	EXCEPTIONS
//	なし
//
bool
VectorFile::isAccessible(bool	bForce_) const
{
	return m_pVersionFile->isAccessible(bForce_);
}

//	FUNCTION public
//	Vector2::VectorFile::isMounted --
//		物理ファイルがマウントされているか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&		trans
//			マウントされているか調べる
//			トランザクションのトランザクション記述子
//
//	RETURN
//		true
//			マウントされている
//		false
//			マウントされていない
//
//	EXCEPTIONS
//		なし
//
bool
VectorFile::isMounted(const Trans::Transaction& cTransaction_) const
{
	; _SYDNEY_ASSERT(m_pVersionFile);
	if (m_bMounted == false)
	{
		// falseだったら下位に聞く
		// trueだったらdetachするまでfalseになることはない
		
		m_bMounted = m_pVersionFile->isMounted(cTransaction_);
	}
	return m_bMounted;
}

//
//	FUNCTION public
//	Vector2::VectorFile::verify -- 整合性検査をする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	const unsigned int				uiTreatment_
//		整合性検査の検査方法（“可能ならば修復するか”など）
//		const Admin::Verification::Treatment::Valueを
//		const unsigned intにキャストした値
//	Admin::Verification::Progress&	cProgress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::MemoryExhaust
//		メモリ不足のため、バージョンページをフィックスできなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//

void
VectorFile::verify()
//VectorFile::verify(const Trans::Transaction& cTransaction_,
//				   const unsigned int uiTreatment_,
//				   Admin::Verification::Progress& cProgress_)
{}

//
//	FUNCTION public
//	Vector2::VectorFile::startVerification -- 整合性検査を開始する
//
//	NOTES
//	LogicalInterface::verifyはopenされずに呼ばれるので
//	open + verify固有の処理 を実行する
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	Admin::Verification::Treatment::Value uiTreatment_
//		整合性検査で矛盾を見つけたときの処置を表す値
//	Admin::Verification::Progress& cProgress_
//		整合性検査の経過を表すクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VectorFile::startVerification(const Trans::Transaction& cTransaction_,
						Admin::Verification::Treatment::Value uiTreatment_,
						Admin::Verification::Progress& cProgress_)
{
	// PhysicalFileのverifyとBtree2のverifyはいろいろな処理をしているが
	// Vector2はそのまま呼ぶだけで十分
	// また、overall=falseで問題ないらしい
	m_pVersionFile->startVerification(
		cTransaction_,
		uiTreatment_,
		cProgress_,
		false);

	m_pTransaction = &cTransaction_;
	m_pProgress = &cProgress_;
	// Vector2のデータは訂正できるほど冗長ではないのでReadOnly
	m_eFixMode = Buffer::Page::FixMode::ReadOnly;

	m_cPageManager.open(cTransaction_,
						m_eFixMode,
						m_cFileID.getPageDataSize());
}

//
//	FUNCTION public
//	Vector2::VectorFile::endVerification -- 整合性検査を終了する
//
//	NOTES
//	LogicalInterface::verifyはcloseされずに終了するので
//	close + verify固有の処理 を実行する
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
VectorFile::endVerification()
{
	// closeと異なり、_AutoDetachPage後に呼ばれるのでdetachは不要

	m_cPageManager.close();

	// PhysicalFileのverifyとBtree2のverifyはいろいろな処理をしているが	
	// Vector2はそのまま呼ぶだけで十分
	m_pVersionFile->endVerification(*m_pTransaction, *m_pProgress);

	m_pTransaction = 0;
	m_pProgress = 0;
	m_eFixMode = Buffer::Page::FixMode::Unknown;
}

//
//	FUNCTION public
//	Vector2::VectorFile::open -- オープンする
//
//	NOTES
//	[?] 
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	LogicalFile::OpenOption::OpenMode::Value eOpenMode_
//		オープンモード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VectorFile::open(
	const Trans::Transaction&					cTransaction_,
	LogicalFile::OpenOption::OpenMode::Value	eOpenMode_)
{
	// FixModeを求める
	if (eOpenMode_ == LogicalFile::OpenOption::OpenMode::Update)
	{
		m_eFixMode = Buffer::Page::FixMode::Write;
	}
	else if (eOpenMode_ == LogicalFile::OpenOption::OpenMode::Batch)
	{
		m_eFixMode = Buffer::Page::FixMode::Write;
		m_pVersionFile->setBatch(true);
		m_bBatch = true;
	}
	else
	{
		m_eFixMode = Buffer::Page::FixMode::ReadOnly;
	}

	// トランザクションを保存する
	m_pTransaction = &cTransaction_;

	m_cPageManager.open(cTransaction_,
						m_eFixMode,
						m_cFileID.getPageDataSize());
}

//
//	FUNCTION public
//	Vector2::VectorFile::close -- クローズする
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
VectorFile::close()
{
	detachAllPages();
	m_cPageManager.close();
	
	m_pTransaction = 0;
	m_eFixMode = Buffer::Page::FixMode::Unknown;
	m_pVersionFile->setBatch(false);
	m_bBatch = false;
}

//
//	FUNCTION public
//	Vector2::VectorFile::getSize --
//		物理ファイルの実体であるバージョンファイルを構成する
//		OS ファイルの総サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		求めたサイズ(B 単位)
//
//	EXCEPTIONS
//
ModUInt64
VectorFile::getSize() const
{
	; _SYDNEY_ASSERT(m_pVersionFile);
	return m_pVersionFile->getSize();
}

//
//	FUNCTION public
//	Vector2::VectorFile::getCount -- 登録カウントを得る
//
//	NOTES
//	[?] physicalfile/fileにない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		求めたサイズ(B 単位)
//
//	EXCEPTIONS
//
//
ModUInt32
VectorFile::getCount()
{
	return m_cPageManager.getCount();
}

//
//	FUNCTION public
//	Vector2::VectorFile::detachAllPages -- すべてのページをdetachする
//
//	NOTES
//	??生成されている全物理ページ記述子を破棄する。
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
VectorFile::detachAllPages()
{
	m_cPageManager.detachManagePage();
	m_cCurrentPage.unfix(false);
}

//
//	FUNCTION public
//	Vector2::VectorFile::isValid --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
bool
VectorFile::isValid(ModUInt32 uiKey_)
{
	// [NOTE] calcPageID() is not a const function.
	
	int dummy;
	Version::Page::ID uiPageID = calcPageID(uiKey_, dummy);
	return (uiPageID <= m_cPageManager.getMaxPageID());
}

//
//	FUNCTION protected static
//	Vector2::VectorFile::getPageHeader -- ページヘッダーを得る
//	Vector2::VectorFile::getConstPageHeader -- ページヘッダーを得る
//
//	NOTES
//
//	ARGUMENTS
//	Version::Page::Memory& page_
//		取り出すページ
//
//	RETURN
//	Vector2::VectorFile::PageHeader*
//	const Vector2::VectorFile::PageHeader*
//		ページヘッダー
//
//	EXCEPTIONS
//
VectorFile::PageHeader*
VectorFile::getPageHeader(Version::Page::Memory& page_)
{
	return syd_reinterpret_cast<PageHeader*>(static_cast<char*>(page_));
}
const VectorFile::PageHeader*
VectorFile::getConstPageHeader(const Version::Page::Memory& page_)
{
	return syd_reinterpret_cast<const PageHeader*>(static_cast<const char*>(page_));
}

//
//	FUNCTION protected
//	Vector2::VectorFile::attachPage -- ページを取得する
//
//	NOTES
//	FIXモードの変更はないことが前提
//
//	ARGUMENTS
//	Version::Page::ID uiPageID_
//		取得するページID
//	Vector2::PageManager::Operation::Value eOperation_
//		オペレーションモード
//
//	RETURN
//	Version::Page::Memory&
//		取得したページイメージ
//
//	EXCEPTIONS
//
Version::Page::Memory&
VectorFile::attachPage(Version::Page::ID uiPageID_,
					   PageManager::Operation::Value eOperation_)
{
	if (m_cCurrentPage.getPageID() != uiPageID_)
	{
		if (m_cCurrentPage.isOwner())
			// すでにattachされているページがあればunifxする
			m_cCurrentPage.unfix(false);
		
		// ページをattachする
		m_cCurrentPage = m_cPageManager.attach(uiPageID_,
											   eOperation_);
	}
	return m_cCurrentPage;
}

//
//	FUNCTION protected
//	Vector2::VectorFile::calcPageID -- キーからページIDと位置を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32	uiKey_
//		キーの値(ROWID)
//	int&		position_
//		データページにおけるキーの位置を、0から始まる順番で返す
//
//	RETURN
//	Version::Page::ID
//		データページのページIDを返す
//
//	EXCEPTIONS
//	なし
//
Version::Page::ID
VectorFile::calcPageID(ModUInt32 uiKey_, int& position_)
{
	// データページ1ページあたりのデータ数
	ModSize dataPerPage = getDataPerPage();

	// ページ内の位置
	position_ = uiKey_ % dataPerPage;

	// 管理ページがない場合のページID
	Version::Page::ID pageID = uiKey_ / dataPerPage;
	// 管理ページ分を足しこむ
	// 管理ページはbit 0を使わないので1引く。最後にヘッダの分を1足す。
	return pageID + pageID / (m_cPageManager.getPagePerTable() - 1) + 1;
}

//
//	FUNCTION protected
//	Vector2::VectorFile::calcKey -- ページIDと位置からキーを得る
//
//	NOTES
//
//	ARGUMENTS
//	Version::Page::ID uiPageID_
//		キーを含むページのID
//	int position_
//		キーのページ内での位置
//
//	RETURN
//	ModUInt32
//		キー
//
//	EXCEPTIONS
//
ModUInt32
VectorFile::calcKey(Version::Page::ID uiPageID_, int position_)
{
	// 管理ページ1ページあたりのページ数
	ModSize pagePerTable = m_cPageManager.getPagePerTable();

	// 管理ページを除いたページID
	uiPageID_ = uiPageID_ - uiPageID_ / pagePerTable - 1;
	// (直前のページまでのデータ数) * (そのページでの位置)
	// ページIDは、0から始まるのでちょうどページ数 - 1を表す。
	// ページ内の位置もキーも、0から始まるのでちょうどそのまま足すだけでよい
	return uiPageID_ * getDataPerPage() + position_;
}

//
//	FUNCTION private
//	Vector2::VectorFile::create -- ファイルを作成する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
VectorFile::create(const Trans::Transaction& cTransaction_)
{
	// バージョンファイルを生成する
	m_pVersionFile->create(cTransaction_);

	try
	{
		// ヘッダーの初期化
		m_cPageManager.initialize(cTransaction_);
		m_cPageManager.detachManagePage();
	}
	catch (...)
	{
		// 失敗したのでファイルを削除する
		m_pVersionFile->destroy(cTransaction_);
		_SYDNEY_RETHROW;
	}
}

//
//	Copyright (c) 2005, 2006, 2007, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
