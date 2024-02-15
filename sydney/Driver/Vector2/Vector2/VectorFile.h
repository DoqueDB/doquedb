// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VectorFile.h -- 
// 
// Copyright (c) 2005, 2006, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VECTOR2_VECTORFILE_H
#define __SYDNEY_VECTOR2_VECTORFILE_H

#include "Vector2/Module.h"
#include "Vector2/Data.h"
#include "Vector2/PageManager.h"

#include "Buffer/Page.h"
#include "LogicalFile/OpenOption.h"
#include "Os/Path.h"
#include "Version/Page.h"

//@@#include "ModVector.h"

_SYDNEY_BEGIN

namespace Admin
{
	namespace Verification
	{
		class Progress;
	}
}
namespace Common
{
	class DataArrayData;
}
namespace Trans
{
	class Timestamp;
	class Transaction;
}
namespace Version
{
	class File;
}

_SYDNEY_VECTOR2_BEGIN

class FileID;

//
//	CLASS
//	Vector2::VectorFile --
//
//	NOTES
//	【VectorFileとは】
//	[?] ベクターと物理ファイルの対応をとる
//
//	TRMeisterの基盤->チューニング->Kernel/PhysicalFile->問題点
//	とにかくなんか実行するたびに物理ページのヘッダーと管理ページのfix, unfixが発生する。
//	たとえば、ベクターファイルがある値を挿入するとき、以下のような流れになっている。
//	
//		[ヘッダーページをattachする(1回)]
//			->[キーからPageIDをもとめる]
//				->[そのページが最終ページ以下かチェックする(2回)]
//				->[そのページが使用されているかチェックする(2回)]
//				->[そのページをattachする(1回)]
//	
//	ベクターファイルに1件挿入するたびに、Version::Page::fix が合計6回実行される。(allocatePageが実行されるとさらに3回増える)
//	
//	⇒ 直接Versionを使用するようにベクターファイルを作り直す。
//	
//	TRMeisterの基盤->チューニング->Driver/Vector->問題点
//	1. fetchの内部でdynamic_castを実行している
//	-> これは単純になくせばいい
//	
//	2. getするたびに PhysicalFile::isUsedPage を実行している。PhysicalFile::isUsedPage 内部で2ページattachしてしまうので、非常に遅い。
//	-> 途中のページの存在が保障されていないとなると、この処理ははずせない？
//	-> 根本的にはファイルフォーマットの変更が必要か？
//	-> PhysicalFile::isUsedPage を修正し、キャッシュするようにするか？ isNoVersion()==false時しか効果はないが...
//	
//	3. getするたびに Vector::Object を new している。そもそもこんなクラスいらない。
//	-> やめるしかないが、めんどくさそうなので、一度 new しかものを使いまわすようにするか？
//	
//	4. getするたびにページをattachしている。
//	-> isNoVersion()==falseの時はキャッシュできるが、めんどくさそう
//
//	例えば以下の関数の機能をVectorで実装する。
//	PhysicalFile::File::clear()
//	物理ファイルを生成直後（空）の状態に戻す。
//	PhysicalFile::File::truncate()
//	空き領域管理機能付き物理ファイルまたは
//	物理ページ管理機能付き物理ファイルの場合、
//	物理ファイル末尾にある未使用の物理ページをトランケートする。
//
//	Versionでdiscardableを使ってfixしない。
//	fixとはコピーを作ること。
//	unfixとはコピーに対して行われた変更を実際に書き込むこと。
//
//	【オブジェクトの寿命】
//	LogicalInterface::open()時に、まだattachされてなければnewされる
//	LogicalInterface::close()時に、detachされてdeleteされる
//
//	でもLogicalInterfaceは_Auto_AttachFileでもattachしている。
//
//	【PageManagerとは】
//	各ページを管理。VectorFileの操作に必須。
//
//	【Version::Fileとは】
//	[?]ようわからんけど、PhysicalFileを使わないためには必要？
//	[?]コンストラクタでattach
//	[?]createでcreate
//
class VectorFile : public Common::Object
{
public:
	// コンストラクタ
	VectorFile(const FileID& cFileID_);
	// デストラクタ
	virtual ~VectorFile();

	// 作成する
	void create();
	// 破棄する
	void destroy(const Trans::Transaction& cTransaction_);

	// マウントする
	void mount(const Trans::Transaction& cTransaction_);
	// アンマウントする
	void unmount(const Trans::Transaction& cTransaction_);

	// フラッシュする
	void flush(const Trans::Transaction& cTransaction_);

	// バックアップを開始する
	void startBackup(const Trans::Transaction& cTransaction_,
					 const bool bRestorable_);
	// バックアップを終了する
	void endBackup(const Trans::Transaction& cTransaction_);

	// 障害から回復する
	void recover(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);
	// ある時点に開始された版を最新版とする
	void restore(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);

	// 同期を取る
	void sync(const Trans::Transaction& cTransaction_,
			  bool& incomplete, bool& modified);

	// 移動する
	// cFilePath_をcPath_に変更
	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cPath_);

	// 実体であるOSファイルが存在するか調べる
	bool isAccessible(bool bForce_ = false) const;
	// マウントされているか調べる
	bool isMounted(const Trans::Transaction& cTransaction_) const;

	// 整合性検査をする
	virtual void verify() = 0;
	//virtual void verify(const Trans::Transaction& cTransaction_,
	//			const unsigned int uiTreatment_,
	//			Admin::Verification::Progress& cProgress_) = 0;
	// 整合性検査を開始する
	void startVerification(
		const Trans::Transaction& cTransaction_,
		const unsigned int uiTreatment_,
		Admin::Verification::Progress& cProgress_);
	// 整合性検査を終了する
	void endVerification();

	// オープンする
	void open(const Trans::Transaction& cTransaction_,
			  LogicalFile::OpenOption::OpenMode::Value eOpenMode_);
	// クローズする
	void close();

	// ファイルサイズを得る
	ModUInt64 getSize() const;
	// 登録カウントを得る
	ModUInt32 getCount();
	
	// 指定されたエントリを得る
	virtual bool fetch(ModUInt32 uiKey_,
					   Common::DataArrayData& cTuple_,
					   const int* pField_,
					   //@@const ModVector<int>& vecField_,
					   const ModSize uiFieldCount_) = 0;

	// 次のエントリを得る
	virtual ModUInt32 next(ModUInt32 uiKey_,
						   Common::DataArrayData& cTuple_,
						   const int* pField_,
						   const ModSize uiFieldCount_,
						   bool bGetByBitset_ = false) = 0;
	// 前のエントリを得る
	virtual ModUInt32 prev(ModUInt32 uiKey_,
						   Common::DataArrayData& cTuple_,
						   const int* pField_,
						   const ModSize uiFieldCount_) = 0;

	// 挿入する
	virtual void insert(ModUInt32 uiKey_,
						const Common::DataArrayData& cTuple_) = 0;
	// 削除する
	virtual void expunge(ModUInt32 uiKey_) = 0;
	// 更新する
	virtual void update(ModUInt32 uiKey_,
						const Common::DataArrayData& cTuple_,
						const int* pUpdateField_,
						//@@const ModVector<int>& vecUpdateField_,
						const ModSize uiFieldCount_) = 0;

	// すべてのページをdetachする
	void detachAllPages();

	// ページをリセットする
	virtual void resetPage(Version::Page::Memory& page) = 0;

	// Batch mode?
	bool isBatchMode() {return m_bBatch;}

	// Is the file valid for the key?
	bool isValid(ModUInt32 uiKey_);

protected:
	//
	//	CLASS
	//	Vector2::VectorFile::PageHeader -- ページのヘッダー
	//
	//	NOTES
	//	データページ内のデータ数を管理する。
	//	件数が0件なら管理ページのビットマップの対応するビットを落とす。
	//	->Vector2::PageManagerを参照
	//
	//	【ページの初期化】
	//	カウント数は0。データは0xff。
	//
	//	ちなみに、各バイトが全て0xffなデータはnullを意味する。
	//	(int の -1 も null として扱う。)
	//
	//	【ページの構造】
	//  0		   4		   m_cPageManager.getPageDataSize()(byte)
	//	-------------------------
	//	| m_uiCount | Value ... |
	//	-------------------------
	//
	//	もし件数を管理しなければ、データを削除するたびに、
	//	ヘッダ部分を除いたデータページ全体をスキャンして、
	//	まだデータが入っているか調べる必要がある。
	//
	struct PageHeader
	{
		ModUInt32	m_uiCount;
	};

	// ページヘッダーを得る
	static PageHeader*
	getPageHeader(Version::Page::Memory& page_);
	static const PageHeader*
	getConstPageHeader(const Version::Page::Memory& page_);

	// ページを得る
	Version::Page::Memory&
	attachPage(Version::Page::ID uiPageID_,
			   PageManager::Operation::Value eOperation_);
	
	// キーからページIDと位置を得る
	Version::Page::ID calcPageID(ModUInt32 uiKey_,
								 int& position_);
	// ページIDと位置からキーを得る
	ModUInt32 calcKey(Version::Page::ID uiPageID_, int position_);

	// ページのデータ領域のサイズを得る
	virtual Os::Memory::Size getPageDataSize() const = 0;
	// ページに格納できる要素数を得る
	ModSize getDataPerPage() const
		{ return getPageDataSize() / m_cData.getSize(); }
	
	// FileID
	//[?] constを追加した。
	//コンストラクタにはconstで渡されるし、変更することもなさそうなので。
	const FileID&			m_cFileID;
	// データクラス
	Data					m_cData;
	// バージョンファイル
	Version::File*			m_pVersionFile;
	// ページ管理クラス
	PageManager				m_cPageManager;

	// トランザクション
	//[?] constを追加した。
	//open()にはconstで渡されるし、create()でそれを渡すだけだから。
	const Trans::Transaction* 	m_pTransaction;
	// FIXモード
	Buffer::Page::FixMode::Value	m_eFixMode;

	//整合性検査の途中経過
	Admin::Verification::Progress*	m_pProgress;
	
private:
	// ファイルを作成する
	void create(const Trans::Transaction& cTransaction_);

	// 現在見ているページ
	Version::Page::Memory	m_cCurrentPage;
	
	// パス
	// コピーのコストがかかるし、FileIDから取得できるので保持しない。
	// Os::Path m_cPath;

	// マウントされているかどうかのキャッシュ
	mutable bool m_bMounted;

	// Batch mode?
	bool m_bBatch;
};

_SYDNEY_VECTOR2_END
_SYDNEY_END

#endif // __SYDNEY_VECTOR2_VECTORFILE_H

//
//	Copyright (c) 2005, 2006, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
