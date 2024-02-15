// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.h --
//		物理ファイル関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PHYSICALFILE_FILE_H
#define __SYDNEY_PHYSICALFILE_FILE_H

#include "Common/Object.h"

#include "Common/BitSet.h"
#include "Common/DoubleLinkedList.h"

#include "Version/File.h"
#include "Version/Page.h"

#include "PhysicalFile/Module.h"
#include "PhysicalFile/Types.h"
#include "PhysicalFile/Page.h"

#include "PhysicalFile/DirectArea.h"

#include "Os/CriticalSection.h"


_SYDNEY_BEGIN

namespace Common
{
	class ObjectIDData;
}
	
_SYDNEY_PHYSICALFILE_BEGIN

//
//	CLASS
//	PhysicalFile::File --
//		物理ファイル記述子の基本クラス
//
//	NOTES
//	物理ファイル記述子の基本クラス。
//
class File : public Common::Object
{
	friend class Manager;
	friend class Page;
	friend class AreaManagePage;
	friend class PageManagePage;
	friend class NonManagePage;
	friend class Content;
	friend class AreaManageFile;

public:

	//
	//	ENUM public
	//	PhysicalFile::File::Vers --
	//		物理ファイルバージョン
	//
	//	NOTES
	//	物理ファイルバージョン。
	//
	enum Vers
	{
		// 初期バージョン
		Version1 = 0,
		// バージョン数
		VersionNum,
		// 現在のバージョン
		CurrentVersion = VersionNum - 1
	};

	//
	//	STRUCT public
	//	PhysicalFile::File::StorageStrategy --
	//		物理ファイル格納戦略
	//
	//	NOTES
	//	物理ファイル格納戦略。
	//
	struct StorageStrategy
	{
		// 物理ファイルタイプ
		// 以下のいずれかを指定する。
		// 1. 空き領域管理機能付き物理ファイル(PhysicalFile::AreaManageType)
		// 2. 物理ページ管理機能付き物理ファイル(PhysicalFile::PageManageType)
		// 3. 管理機能なし物理ファイル(PhysicalFile::NonManageType)
		// 4. 物理ページ管理機能付き物理ファイル (PhysicalFile::PageManageType2)
		Type							m_PhysicalFileType;

		// 物理ページ内の使用率上限 [％]
		// 1〜100％で指定する。デフォルト値は
		// PhysicalFile::ConstValue::DefaultPageUseRateを指定する。
		// 空き領域管理機能付き物理ファイルの場合には省略不可。
		unsigned int					m_PageUseRate;

		Version::File::StorageStrategy	m_VersionFileInfo;

	}; // end of struct PhysicalFile::File::StorageStrategy

	//
	//	STRUCT public
	//	PhysicalFile::File::BufferingStrategy --
	//		バッファリング戦略
	//
	//	NOTES
	//	バッファリング戦略。
	//
	struct BufferingStrategy
	{
		Version::File::BufferingStrategy	m_VersionFileInfo;
	};

	//
	// メンバ関数
	//

	// 物理ファイルを生成する
	SYD_PHYSICALFILE_FUNCTION
	void create(const Trans::Transaction&	Transaction_);

	// 物理ファイルを消去する
	SYD_PHYSICALFILE_FUNCTION
	virtual void destroy(const Trans::Transaction&	Transaction_);

	// 物理ファイルを移動する
	SYD_PHYSICALFILE_FUNCTION
	void move(const Trans::Transaction&						Transaction_,
			  const Version::File::StorageStrategy::Path&	FilePath_);

	// 構成する OS ファイルが存在するか調べる
	SYD_PHYSICALFILE_FUNCTION
	bool
	isAccessible(bool Force_ = false) const;
	// マウントされているか調べる
	SYD_PHYSICALFILE_FUNCTION
	bool
	isMounted(const Trans::Transaction& trans) const;

	// バッチインサートフラグを設定する
	SYD_PHYSICALFILE_FUNCTION
	void
	setBatch(bool batch_) { m_VersionFile->setBatch(batch_); };

	// 実体である OS ファイルの総サイズを得る
	SYD_PHYSICALFILE_FUNCTION
	FileSize
	getSize() const;
#ifdef OBSOLETE
	// 実体であるバージョンファイルが使用するサイズを得る
	FileSize
	getVersionSize(const Trans::Transaction& trans);
#endif
	// 使用中の総バージョンページサイズを得る
	SYD_PHYSICALFILE_FUNCTION
	virtual FileSize
	getUsedSize(const Trans::Transaction& trans);
#ifdef OBSOLETE
	// 確保中の総バージョンページサイズを得る
	SYD_PHYSICALFILE_FUNCTION
	virtual FileSize
	getTotalSize(const Trans::Transaction& trans);
#endif

	// 物理ページを確保する(detachPageAllを呼ぶこと)
	SYD_PHYSICALFILE_FUNCTION
	PageID
		allocatePage(
			const Trans::Transaction&	Transaction_,
			const PageID				PageID_ =
											ConstValue::UndefinedPageID);

	// 物理ページを確保する(detachPageAllを呼ぶこと)
	SYD_PHYSICALFILE_FUNCTION
	Page* allocatePage2(const Trans::Transaction& cTransaction_,
						Buffer::Page::FixMode::Value eFixMode_,
						PageID cPageID_ = ConstValue::UndefinedPageID,
						Buffer::ReplacementPriority::Value ePriority
						= Buffer::ReplacementPriority::Low);

	// 物理ページを解放する(detachPageAllを呼ぶこと)
	SYD_PHYSICALFILE_FUNCTION
	void freePage(const Trans::Transaction&	Transaction_,
				  const PageID				PageID_);

	// 物理ページを解放する(detachPageAllを呼ぶこと)
	SYD_PHYSICALFILE_FUNCTION
	void freePage2(const Trans::Transaction&	Transaction_,
				   Page*&						pPage_);

	// 物理ページを再利用する(detachPageAllを呼ぶこと)
	SYD_PHYSICALFILE_FUNCTION
	PageID reusePage(const Trans::Transaction&	Transaction_,
					 const PageID				PageID_);

	// 物理ファイルを空の状態にする
	SYD_PHYSICALFILE_FUNCTION
	void clear(const Trans::Transaction&	Transaction_,
			   const bool					Force_ = false);

	// 物理ページ記述子を生成する(detachPageAllを呼ぶこと)
	SYD_PHYSICALFILE_FUNCTION
	virtual Page*
		attachPage(
			const Trans::Transaction&			Transaction_,
			const Buffer::Page::FixMode::Value	FixMode_,
			const Buffer::ReplacementPriority::Value
												ReplacementPriority_ =
										Buffer::ReplacementPriority::Low);

	// 物理ページ記述子を生成する(detachPageAllを呼ぶこと)
	SYD_PHYSICALFILE_FUNCTION
	virtual Page*
		attachPage(
			const Trans::Transaction&			Transaction_,
			const PageID						PageID_,
			const Buffer::Page::FixMode::Value	FixMode_,
			const Buffer::ReplacementPriority::Value
												ReplacementPriority_ =
										Buffer::ReplacementPriority::Low);

	// 物理ページ記述子を破棄する
	SYD_PHYSICALFILE_FUNCTION
	virtual void
		detachPage(
			Page*&					Page_,
			Page::UnfixMode::Value	UnfixMode_ = Page::UnfixMode::Omit,
			const bool				SavePage_ = false);

	// 物理ページ記述子を破棄せずにページ内容を更新する
	SYD_PHYSICALFILE_FUNCTION
	void savePage(Page*			Page_,
				  const bool	Dirty_ = false);

	// 生成されている全物理ページ記述子を破棄する
	SYD_PHYSICALFILE_FUNCTION
	virtual void detachPageAll();

	// 空き領域管理機能付き物理ファイルの物理ページ記述子を破棄し、
	// ページ内容を元に戻す
	SYD_PHYSICALFILE_FUNCTION
	virtual void recoverPage(const Trans::Transaction&	Transaction_,
							 Page*&						Page_);

	// 物理ページ管理機能付き物理ファイルの物理ページ記述子を破棄し、
	// ページ内容を元に戻す
	SYD_PHYSICALFILE_FUNCTION
	virtual void recoverPage(Page*&	Page_);

	// 生成されている空き領域管理機能付き物理ファイルの
	// 全物理ページ記述子を破棄し、ページ内容を元に戻す
	SYD_PHYSICALFILE_FUNCTION
	virtual void
		recoverPageAll(const Trans::Transaction&	Transaction_);

	// 生成されている物理ページ管理機能付き物理ファイルの
	// 全物理ページ記述子を破棄し、ページ内容を元に戻す
	SYD_PHYSICALFILE_FUNCTION
	virtual void recoverPageAll();

	// 物理ページを高速検索可能な閾値を返す [byte]
	SYD_PHYSICALFILE_FUNCTION
	virtual PageSize getPageSearchableThreshold() const;

	// 物理ページを検索する(detachPageAllを呼ぶこと)
	SYD_PHYSICALFILE_FUNCTION
	virtual PageID
		searchFreePage(
			const Trans::Transaction&	Transaction_,
			const PageSize				Size_,
			const PageID				PageID_,
			const bool					IsUnuseArea_,
			const AreaNum				AreaNum_ = 1);

	// 物理ページを検索する(detachPageAllを呼ぶこと)
	SYD_PHYSICALFILE_FUNCTION
	virtual Page*
		searchFreePage2(
			const Trans::Transaction&		Transaction_,
			PageSize						Size_,
			Buffer::Page::FixMode::Value	eFixMode_,
			PageID							PageID_,
			bool							IsUnuseArea_,
			AreaNum							AreaNum_ = 1);

	// 物理ページデータサイズを返す
	// Get the size of the space which the other module can use.
	SYD_PHYSICALFILE_FUNCTION
	static PageSize
		getPageDataSize(const Type				PhysicalFileType_,
						const Os::Memory::Size	VersionPageSize_,
						const AreaNum			AreaNum_ = 1);

	// 物理ページデータサイズを返す
	SYD_PHYSICALFILE_FUNCTION
	virtual PageSize
		getPageDataSize(const AreaNum	AreaNum_ = 1) const = 0;

	// 使用中の物理ページかどうかをチェックする(detachPageAllを呼ぶこと)
	SYD_PHYSICALFILE_FUNCTION
	bool isUsedPage(const Trans::Transaction&	Transaction_,
					const PageID				PageID_);

	// 物理ファイル内で使用中と未使用の物理ページ数を取り出す
	SYD_PHYSICALFILE_FUNCTION
	void fetchOutPageNum(const Trans::Transaction&	Transaction_,
						 PageNum&					UsedPageNum_,
						 PageNum&					UnusePageNum_);

	// 物理ファイル内で使用中の物理ページ数を返す
	SYD_PHYSICALFILE_FUNCTION
	PageNum
		getUsedPageNum(const Trans::Transaction&	Transaction_);

	// 物理ファイル内で未使用の物理ページ数を返す
	SYD_PHYSICALFILE_FUNCTION
	PageNum
		getUnusePageNum(const Trans::Transaction&	Transaction_);

	// 先頭の使用中の物理ページの識別子を返す(detachPageAllを呼ぶこと)
	SYD_PHYSICALFILE_FUNCTION
	virtual PageID
		getTopPageID(const Trans::Transaction&	Transaction_);

	// 最後の使用中の物理ページの識別子を返す(detachPageAllを呼ぶこと)
	SYD_PHYSICALFILE_FUNCTION
	virtual PageID
		getLastPageID(const Trans::Transaction&	Transaction_);

	// 次の使用中の物理ページの識別子を返す(detachPageAllを呼ぶこと)
	SYD_PHYSICALFILE_FUNCTION
	virtual PageID
		getNextPageID(const Trans::Transaction&	Transaction_,
					  const PageID				PageID_);

	// 前の使用中の物理ページの識別子を返す(detachPageAllを呼ぶこと)
	SYD_PHYSICALFILE_FUNCTION
	virtual PageID
		getPrevPageID(const Trans::Transaction&	Transaction_,
					  const PageID				PageID_);
#ifdef OBSOLETE
	// 利用者が確保済のおおよその領域サイズを返す
	SYD_PHYSICALFILE_FUNCTION
	virtual FileSize
		getAllocatedSize(const Trans::Transaction&	Transaction_) = 0;
#endif // OBSOLETE
	// 参照しているバージョンファイル記述子が同じかどうかを調べる
	SYD_PHYSICALFILE_FUNCTION
	bool equals(const File*	DstFile_) const;

	//
	// ↓↓↓↓↓ for DirectAreaFile ↓↓↓↓↓
	//

	// Allocate DirectArea
	SYD_PHYSICALFILE_FUNCTION
	virtual DirectArea allocateArea(const Trans::Transaction&	cTransaction_,
									AreaSize					uiSize_);

	// Free DirectArea
	SYD_PHYSICALFILE_FUNCTION
	virtual void freeArea(const Trans::Transaction&			cTransaction_,
						  const DirectArea::ID&				cID_,
						  Admin::Verification::Progress*	pProgress_ = 0);

	// Attach DirectArea
	SYD_PHYSICALFILE_FUNCTION
	virtual DirectArea attachArea(const Trans::Transaction&	cTransaction_,
								  const DirectArea::ID&		cID_,
								  Buffer::Page::FixMode::Value	eFixMode_);
	
	// Detach All attached DirectAreas
	SYD_PHYSICALFILE_FUNCTION
	virtual void detachAllAreas();

	// Recover All attached DirectAreas
	SYD_PHYSICALFILE_FUNCTION
	virtual void recoverAllAreas();

	// Get the max size of storable DirectArea.
	SYD_PHYSICALFILE_FUNCTION
	virtual AreaSize getMaxStorableAreaSize() const;

	// Convert ObjectIDData to DirectArea::ID.
	SYD_PHYSICALFILE_FUNCTION
	static void
	convertToDirectAreaID(const Common::ObjectIDData&	cObjectID_,
						  DirectArea::ID&				cID_);

	// Convert DirectArea::ID to ObjectIDData.
	SYD_PHYSICALFILE_FUNCTION
	static void
	convertToObjectID(const DirectArea::ID&		cID_,
					  Common::ObjectIDData&		cObjectID_);

	//
	// These are public, but not used by other than this module.
	//

	// Get VersionFile for DirectAreaPage::changeFixMode and PhysicalFileTest.
	SYD_PHYSICALFILE_FUNCTION
	Version::File* getVersionFile() const { return m_VersionFile; };
	
	//
	// ↑↑↑↑↑ for DirectAreaFile ↑↑↑↑↑
	//

	//
	// 運用管理のためのメソッド
	//

	// 物理ファイルをマウントする
	SYD_PHYSICALFILE_FUNCTION
	void mount(const Trans::Transaction&	Transaction_);

	// 物理ファイルをアンマウントする
	SYD_PHYSICALFILE_FUNCTION
	void unmount(const Trans::Transaction&	Transaction_);

	// 物理ファイルをフラッシュする
	SYD_PHYSICALFILE_FUNCTION
	void flush(const Trans::Transaction&	Transaction_);

	// 物理ファイルに対してバックアップ開始を通知する
	SYD_PHYSICALFILE_FUNCTION
	void startBackup(const Trans::Transaction&	Transaction_,
					 const bool					Restorable_ = true);

	// 物理ファイルに対してバックアップ終了を通知する
	SYD_PHYSICALFILE_FUNCTION
	void endBackup(const Trans::Transaction&	Transaction_);

	// 物理ファイルを障害回復する
	SYD_PHYSICALFILE_FUNCTION
	void recover(const Trans::Transaction&	Transaction_,
				 const Trans::TimeStamp&	Point_);

	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	SYD_PHYSICALFILE_FUNCTION
	void restore(const Trans::Transaction&	Transaction_,
				 const Trans::TimeStamp&	Point_);

	// 同期を取る
	SYD_PHYSICALFILE_FUNCTION
	void
	sync(const Trans::Transaction& trans, bool& incomplete, bool& modified);

	//
	// 整合性検査のためのメソッド
	//

	// 整合性検査開始を指示する
	SYD_PHYSICALFILE_FUNCTION
	void
		startVerification(
			const Trans::Transaction&		Transaction_,
			const unsigned int				Treatment_,
			Admin::Verification::Progress&	Progress_);

	// 使用中の物理ページを通知する
	SYD_PHYSICALFILE_FUNCTION
	void
		notifyUsePage(const Trans::Transaction&			Transaction_,
					  Admin::Verification::Progress&	Progress_,
					  const PageID						PageID_,
					  const AreaNum						AreaNum_ = 0,
					  const AreaID*						AreaIDs_ = 0);

	// 整合性検査終了を指示する
	SYD_PHYSICALFILE_FUNCTION
	void endVerification(const Trans::Transaction&		Transaction_,
						 Admin::Verification::Progress&	Progress_);

	// 物理ページ記述子を生成する（バージョンページの整合性検査付き）
	SYD_PHYSICALFILE_FUNCTION
	virtual Page*
		verifyPage(const Trans::Transaction&			Transaction_,
				   const Buffer::Page::FixMode::Value	FixMode_,
				   Admin::Verification::Progress&		Progress_);

	// 物理ページ記述子を生成する（バージョンページの整合性検査付き）
	SYD_PHYSICALFILE_FUNCTION
	Page* verifyPage(const Trans::Transaction&			Transaction_,
					 const PageID						PageID_,
					 const Buffer::Page::FixMode::Value	FixMode_,
					 Admin::Verification::Progress&		Progress_);

	//
	// ↓↓↓↓↓ for DirectAreaFile ↓↓↓↓↓
	//

	// Verify DirectArea
	SYD_PHYSICALFILE_FUNCTION
	virtual DirectArea verifyArea(
		const Trans::Transaction&			cTransaction_,
		const DirectArea::ID&				cID_,
		Buffer::Page::FixMode::Value		eFixMode_,
		Admin::Verification::Progress&		cProgress_);

	//
	// ↑↑↑↑↑ for DirectAreaFile ↑↑↑↑↑
	//

	// バッファリング戦略を得る
	BufferingStrategy
	getBufferingStrategy() const;

#ifdef DEBUG

	SYD_PHYSICALFILE_FUNCTION
	bool truncate(const Trans::Transaction&	Trans_);

	SYD_PHYSICALFILE_FUNCTION
	virtual Version::Page::ID getTableID(const PageID	PageID_);

	SYD_PHYSICALFILE_FUNCTION
	virtual void
		getTableHeader(
			const Trans::Transaction&	Transaction_,
			const Version::Page::ID		TableVersionPageID_,
			PageNum&					UsedPageNum_,
			PageNum&					UnusePageNum_,
			PageNum*					PageNumByUnuseAreaRate_,
			PageNum*					PageNumByFreeAreaRate_);

	SYD_PHYSICALFILE_FUNCTION
	virtual void
		getTableBitmap(
			const Trans::Transaction&	Transaction_,
			const Version::Page::ID		TableVersionPageID_,
			unsigned char*				BitmapBuffer_);

	//
	// ↓↓↓↓↓ for DirectAreaFile ↓↓↓↓↓
	//

	// Get parent index value.
	SYD_PHYSICALFILE_FUNCTION
	virtual AreaSize
	getParentIndexValue(const Trans::Transaction&	cTransaction_,
						PageID						uiPageID_);

	// Set parent index value.
	SYD_PHYSICALFILE_FUNCTION
	virtual void
	setParentIndexValue(const Trans::Transaction&	cTransaction_,
						PageID						uiPageID_,
						AreaSize					uiSize_);
	
	// Set the number of Area in the Page
	SYD_PHYSICALFILE_FUNCTION
	virtual void setLeafHeaderNumber(const Trans::Transaction&	cTransaction_,
									 PageID						uiPageID_,
									 AreaNum					uiNum_);
	// Set the free space offset in the Page
	SYD_PHYSICALFILE_FUNCTION
	virtual void setLeafHeaderOffset(const Trans::Transaction&	cTransaction_,
									 PageID						uiPageID_,
									 AreaOffset					uiOffset_);

	// Set the AreaID in the Page
	SYD_PHYSICALFILE_FUNCTION
	virtual void setLeafIndexAreaID(const Trans::Transaction&	cTransaction_,
									PageID						uiPageID_,
									AreaID						uiAreaID_,
									AreaNum						uiIndex_);
	// Set the area offset in the Page
	SYD_PHYSICALFILE_FUNCTION
	virtual void setLeafIndexOffset(const Trans::Transaction&	cTransaction_,
									PageID						uiPageID_,
									AreaOffset					uiOffset_,
									AreaNum						uiIndex_);
	//
	// ↑↑↑↑↑ for DirectAreaFile ↑↑↑↑↑
	//
#endif // DEBUG

	// 物理ファイルが管理しているページを確定する
	SYD_PHYSICALFILE_FUNCTION
	void unfixVersionPage(bool dirty);

	// dirtyではないページを解放する。ただしiCacheSize_分はとっておく
	SYD_PHYSICALFILE_FUNCTION
	bool unfixNotDirtyPage(ModSize iCacheSize_ = 0);

	//
	//	CLASS
	//	PhysicalFile::File::VersionPage -- Version::Page::Memoryのラッパークラス
	//
	//	NOTES
	//	This is changed from protected to public for DirectAreaTree.
	//
	class VersionPage : public Version::Page::Memory
	{
	public:
		// コンストラクタ
		VersionPage();
		// デストラクタ
		~VersionPage();

		// 代入演算子
		VersionPage& operator = (const VersionPage& src);
		VersionPage& operator = (const Version::Page::Memory& src);

		// 参照カウンタ
		int _reference;
		
		VersionPage* _next;
		VersionPage* _prev;
	};

	//
	//	CLASS
	//	PhysicalFile::File::PagePointer -- VersionPageの参照カウンタを管理する
	//
	//	NOTES
	//	This is changed from protected to public for DirectAreaTree.
	//
	class PagePointer
	{
	public:
		// コンストラクタ
		PagePointer(VersionPage* pPage_ = 0) : m_pPage(pPage_) { increment(); }
		// デストラクタ
		~PagePointer() { decrement(); }
		// コピーコンストラクタ
		PagePointer(const PagePointer& cPointer_)
			: m_pPage(cPointer_.get())
		{
			increment();
		}

		// 代入演算子
		PagePointer& operator = (const PagePointer& cPointer_)
		{
			if (get() != cPointer_.get())
			{
				decrement();
				m_pPage = cPointer_.get();
				increment();
			}
			return *this;
		}
		// 代入演算子
		PagePointer& operator = (VersionPage* pPage_)
		{
			if (get() != pPage_)
			{
				decrement();
				m_pPage = pPage_;
				increment();
			}
			return *this;
		}

		// ページを得る
		VersionPage* get() const { return m_pPage; }

		// オペレータ
		operator bool () const { return get() != 0; }
		VersionPage* operator -> () { return get(); }
		VersionPage& operator * () { return *get(); }

	private:
		// 参照カウンタを増やす
		void increment()
		{
			if (m_pPage) m_pPage->_reference++;
		}
		// 参照カウンタを減らす
		void decrement()
		{
			if (m_pPage) m_pPage->_reference--;
		}

		// ページ
		mutable VersionPage* m_pPage;
	};
	
protected:

	//
	//	CLASS
	//	PhysicalFile::File::AutoUnfix -- バージョンページをunfixする
	//
	class AutoUnfix
	{
	public:
		// コンストラクタ
		AutoUnfix(File* pFile_) : m_pFile(pFile_), dirty(false) {}
		// デストラクタ
		~AutoUnfix()
		{
			if (m_pFile) m_pFile->unfixVersionPage(dirty);
		}

		// 成功
		void success()
		{
			dirty = true;
		}

	private:
		File* m_pFile;
		bool dirty;
	};

	//
	// メンバ関数
	//

	// コンストラクタ
	File(const StorageStrategy&		FileStorageStrategy_,
		 const BufferingStrategy&	BufferingStrategy_,
		 const Lock::FileName*		LockName_,
		 bool						batch_);

	// デストラクタ
	virtual ~File();

	// キャッシュされている物理ページ記述子を返す
	Page* getCachedPage(const PageID	PageID_) const;

public:
	// Covert PhysicalFile::PageID to Version::Page::ID.
	virtual Version::Page::ID
		convertToVersionPageID(const PageID	PageID_) const;

protected:
	// Get managed VersionPage
	PagePointer
		fixVersionPage(
			const Trans::Transaction&			Transaction_,
			Version::Page::ID					VersionPageID_,
			Buffer::Page::FixMode::Value		FixMode_,
			Buffer::ReplacementPriority::Value	ReplacementPriority_ =
										Buffer::ReplacementPriority::Low);

	// Get attached page.
	virtual Page* getAttachedPage(
			const Trans::Transaction&					cTransaction_,
			const PageID								uiPageID_,
			const Buffer::Page::FixMode::Value			eFixMode_,
			const Buffer::ReplacementPriority::Value*	pPriority_,
			Admin::Verification::Progress*				pProgress_);
	
	//
	// 整合性検査のためのメソッド
	//

	// アタッチ中の物理ページが存在するかどうかを知らせる
	bool existAttachPage() const;

public:	
	// 可能であれば修復するように指定されているかどうかを知らせる
	bool isCorrect() const;

	// Get file path.
	ModUnicodeString getFilePath() const { return m_FilePath; };

	
	//
	// データメンバ
	//

	// 版を破棄可能な生成フィックスモード
	static Buffer::Page::FixMode::Value		DiscardableAllocateFixMode;

	// 版を破棄可能な書き込みフィックスモード
	static Buffer::Page::FixMode::Value		DiscardableWriteFixMode;

protected:
	// バージョンファイル記述子
	Version::File*							m_VersionFile;

	// バージョンページサイズ [byte]
	Os::Memory::Size						m_VersionPageSize;

	// バージョンページデータサイズ [byte]
	// （バージョンページ中で、PhysicalFileが使えるサイズ）
	Os::Memory::Size						m_VersionPageDataSize;

	// 公開領域最大サイズ [byte]
	PageSize								m_UserAreaSizeMax;

	// 物理ページ記述子のリンク操作の排他制御用のラッチ
	mutable Os::CriticalSection				_latch;

	// 直前にアタッチした物理ページ記述子
	Page*									m_Page;

	// 直前にデタッチした物理ページ記述子
	Page*									m_DetachPage;

	//
	// 整合性検査のためのデータメンバ
	//

	// 物理ページ識別子を記録するためのビットマップ
	Common::BitSet							m_PageIDs;

	// 自身が管理している物理ページ数
	PageNum									m_ManagePageNum;

	// 自身が管理している物理ページのうちの最終物理ページの識別子
	PageID									m_LastManagePageID;

	// 整合性検査の検査方法（“可能ならば修復するか”など）
	// Admin::Verification::Treatment::Valueを
	// unsigned intにキャストした値
	unsigned int							m_Treatment;

	// 整合性検査中であるかどうか
	bool									m_Check;

	// 使用中の物理ページを通知されたかどうか
	bool									m_NotifiedUsePage;

	// 物理ファイル格納位置（ディレクトリパス）
	ModUnicodeString						m_FilePath;

	//
	//	TYPEDEF
	//	PhysicalFile::File::PageList
	//
	typedef Common::DoubleLinkedList<VersionPage>	PageList;

private:

	//
	// メンバ関数
	//

	// 物理ファイル生成時の初期化
	virtual void initialize(const Trans::Transaction&	Trans_,
							void*						FileHeader_);

	// 物理ファイル生成を取り消す
	void undoCreate(const Trans::Transaction&	Transaction_);

	// 物理ファイルの利用可能性を設定する
	void setAvailability(const bool	Availability_);

	// 物理ページ記述子を生成する
	virtual Page*
		attachPage(
			const Trans::Transaction&					Transaction_,
			File*										File_,
			const PageID								PageID_,
			const Buffer::Page::FixMode::Value			FixMode_,
			const Buffer::ReplacementPriority::Value	ReplacementPriority_);

	// Allocate Page instance.
	virtual Page* allocatePageInstance(
		const Trans::Transaction&			cTransaction_,
		PageID								uiPageID_,
		Buffer::Page::FixMode::Value		eFixMode_,
		Admin::Verification::Progress*		pProgress_ = 0,
		Buffer::ReplacementPriority::Value	eReplacementPriority_
										= Buffer::ReplacementPriority::Low) = 0;

protected:
	// 物理ページアタッチのリトライのための処理を行う
	bool
	retryStepSelf(bool others = true);

private:
#ifdef OBSOLETE
	bool
	retryStepOthers();
#endif

	// 物理ページを割り当てる
	PageID assignPage(const Trans::Transaction&	Transaction_,
					  const void*				FileHeader_,
					  const PageNum				TotalPageNum_,
					  const PageNum				UnusePageNum_);

	//	物理ページ追加のための準備を行う
	virtual void
	prepareAppendPage(const Trans::Transaction&				Trans_,
					  void*									FileHeader_,
					  const PageID							LastPageID_,
					  const Buffer::Page::FixMode::Value	AllocateFixMode_);

	//	物理ページ追加のための準備を行う
	virtual void
	prepareAppendPage(const Trans::Transaction&				Trans_,
					  void*									FileHeader_,
					  const PageID							LastPageID_,
					  const PageID							PageID_,
					  const Buffer::Page::FixMode::Value	AllocateFixMode_,
					  const Buffer::Page::FixMode::Value	WriteFixMode_);

	// トランケートする
	virtual void truncate(const Trans::Transaction& trans, bool& modified);

	// 最後の使用中の物理ページの識別子を返す
	virtual PageID getLastPageID(const Trans::Transaction&	Transaction_,
								 PageNum					usedPageNum_,
								 PageNum					unusePageNum_);

	// 空き領域管理表／物理ページ表ヘッダから
	// 「使用中の物理ページ数」と「未使用の物理ページ数」を取り出す
	virtual void
		fetchOutPageNumFromManageTable(
			const void*	TablePointer_,
			PageNum&	UsedPageNum_,
			PageNum&	UnusePageNum_) const;

	// 空き領域管理表／物理ページ表ヘッダの
	// 「使用中の物理ページ数」を更新する
	virtual void updateUsedPageNum(void*		TablePointer_,
								   const int	AddNum_);

	// 空き領域管理表／物理ページ表ヘッダの
	// 「未使用の物理ページ数」を更新する
	virtual void updateUnusePageNum(void*		TablePointer_,
									const int	AddNum_);

	// 使用中の物理ページかどうかをチェックする
	virtual bool isUsedPage(const void*		TablePointer_,
							const PageID	PageID_) const;

	// 1つの空き領域管理表／物理ページ表で管理可能な物理ページ数を返す
	virtual PageNum getPagePerManageTable() const;

	// 次に割り当てる物理ページを検索する
	virtual PageID
		searchNextAssignPage(const Trans::Transaction&	Transaction_,
							 const PageNum				TotalPageNum_);
	PageID searchNextAssignPage2(const Trans::Transaction&	Transaction_,
								 const PageNum				TotalPageNum_);
	virtual PageID
		searchNextAssignPage(const Trans::Transaction&	Trans_,
							 const void*				FileHeader_,
							 const PageNum				ManagePageNum);

	// 次に割り当てる物理ページを検索する
	virtual PageID
		searchNextAssignPage(const void*	TablePointer_) const;

	// 物理ページを初期化する
	virtual void initializePage(void*	PagePointer_);

	// 空き領域管理表／物理ページ表数を返す
	virtual PageNum getManageTableNum(const PageID	PageID_) const;

	// 空き領域管理表／物理ページ表のバージョンページ識別子を返す
	virtual Version::Page::ID	getManageTableVersionPageID(
		const PageID	PageID_) const;

	// 空き領域管理表／物理ページ表を更新する
	virtual void updateManageTable(void*			TablePointer_,
								   const PageID		PageID_,
								   const PageNum	PageNum_,
								   const bool		ForReuse_,
								   const void*		PagePointer_ = 0);

	// 空き領域管理表内の領域率ビットマップを更新する
	virtual void updateAreaBitmap(void*					TablePointer_,
								  const PageID			PageID_,
								  const unsigned char	BitmapValue_);

	// 物理ページ表内の物理ページ使用状態ビットマップを更新する
	virtual void updatePageBitmap(void*			TablePointer_,
								  const PageID	PageID_,
								  const bool	BitON_);

	// 未使用領域率別／空き領域率別の物理ページ数配列を更新する
	virtual void updatePageArray(void*				TablePointer_,
								 const bool			ByUnuseAreaRate_,
								 const unsigned int	AreaRateValue_,
								 const bool			Increment_);

	// 木構造を更新する ※ PageManageFile2
	virtual void updateTree(const Trans::Transaction&	Trans_,
							void*						FileHeader_,
							void*						Leaf_,
							const PageID				PageID_,
							const bool					IsUse_,
							const bool					AppendPage_,
							const bool					Discardable_);

	// ノードの全ビット OFF ※ PageManageFile2
	virtual void clearNode(void*	Node_) const;

	// ノードのビット ON/OFF ※ PageManageFile2
	virtual void updateNode(const Trans::Transaction&	Trans_,
							void*						FileHeader_,
							const PageID				PageID_,
							const bool					BitON_,
							const bool					Discardable_);

	// ノードとリーフをクリアする ※ PageManageFile2
	virtual void clearTree(const Trans::Transaction&	Trans_,
						   const PageNum				TotalPageNum_);

	//
	// 整合性検査のためのメソッド
	//

	// 物理ページ記述子を生成する（バージョンページの整合性検査付き）
	Page*
		verifyPage(
			const Trans::Transaction&			Transaction_,
			File*								File_,
			const PageID						PageID_,
			const Buffer::Page::FixMode::Value	FixMode_,
			Admin::Verification::Progress&		Progress_);

	// アタッチ中の物理ページに対応するバージョンページの整合性検査をする
	virtual void
		verifyAttachedPage(
			const Trans::Transaction&		Transaction_,
			Admin::Verification::Progress&	Progress_) const;

	// 物理ページが過去にアタッチされたかどうかを確認する
	bool isAttachedPage(const PageID	PageID_) const;

	// 整合性検査の前処理を行う
	virtual void
		initializeVerification(
			const Trans::Transaction&		Transaction_,
			const unsigned int				Treatment_,
			Admin::Verification::Progress&	Progress_);

	// 整合性検査の後処理を行う
	virtual void terminateVerification();

	// 物理エリア識別子を記録するためのビットマップの列を初期化する
	virtual void initializeAreaIDBitmap(const PageNum	ManagePageNum_);

	// 物理エリア識別子を記録するためのビットマップの列を解放する
	virtual void terminateAreaIDBitmap(const PageNum	ManagePageNum_);

	// 物理エリア識別子を記録するためのビットマップを設定する
	virtual void setAreaIDBitmap(const PageID	PageID_,
								 const AreaNum	AreaNum_,
								 const AreaID*	AreaIDs_);

public:
	// 整合性検査のためにバージョンページをフィックスする
	// （リトライ付き）
	static Version::Page::Memory
		fixVersionPage(
			const Trans::Transaction&			Transaction_,
			File*								PhysicalFile_,
			const Version::Page::ID				VersionPageID_,
			const Buffer::Page::FixMode::Value	FixMode_,
			Admin::Verification::Progress&		Progress_);

protected:
	// Version::Page::Memoryのキャッシュ
	PageList						m_cPageList;

private:

	// 整合性検査のために、物理ファイルヘッダをフィックスする
	virtual void
		verifyFileHeader(const Trans::Transaction&			Transaction_,
						 Admin::Verification::Progress&	Progress_);

	// 整合性検査のために、
	// すべての空き領域管理表／物理ページ表をフィックスする
	virtual void verifyAllTable(const Trans::Transaction&		Transaction_,
								Admin::Verification::Progress&	Progress_);

	// 整合性検査のために、すべてのノードをフィックスする
	virtual void verifyAllPages(const Trans::Transaction&		cTransaction_,
								Admin::Verification::Progress&	cProgress_);

	// 利用者と自身の物理ページの使用状況が一致するかどうかをチェックする
	virtual void
		correspondUsePage(const Trans::Transaction&			Transaction_,
						  Admin::Verification::Progress&	Progress_);

	// 物理ページの使用状況を修復する
	virtual void correctUsePage(const Trans::Transaction&		Transaction_,
								const PageID					PageID_,
								void*							TableTop_,
								Admin::Verification::Progress&	Progress_);

	// 利用者と自身の物理エリアの使用状況が一致するかどうかをチェックする
	virtual void
		correspondUseArea(const Trans::Transaction&			Transaction_,
						  Admin::Verification::Progress&	Progress_);

	// 物理ファイルの整合性検査を行う
	virtual void
		checkPhysicalFile(const Trans::Transaction&			Transaction_,
						  Admin::Verification::Progress&	Progress_) = 0;

	// ヘッダーページを得る
	VersionPage* getHeaderPage(const Trans::Transaction& cTransaction_,
							   Buffer::Page::FixMode::Value eFixMode_);

	// バージョンページを得る
	PagePointer getVersionPage(const Trans::Transaction& cTransaction_,
							   Version::Page::ID uiVersionPageID_,
							   Buffer::Page::FixMode::Value eFixMode_,
							   Buffer::ReplacementPriority::Value
												   eReplacementPriority_);

	// ランダムで管理テーブルを列挙する
	static ModSize getLookManageTable(	PageNum				PagePerTable_,
										Version::Page::ID	FirstTableID_,
										Version::Page::ID	LastTableID_,
										Version::Page::ID*	LookPages_,
										ModSize				Size_);

	//
	// データメンバ
	//

	// 物理ファイルタイプ
	Type							m_Type;
#ifdef OBSOLETE
	// アタッチ中の物理ファイル記述子へのリンク
	File*							m_Next;
	File*							m_Prev;
#endif
	// バッファプール種別
	Buffer::Pool::Category::Value	m_BufferingCategory;

	//
	// 整合性検査のためのデータメンバ
	//

	// バージョンファイルのstartVerification()を呼んだかどうか
	//     true  : 既に呼んだ
	//     false : まだ呼んでいない
	bool							m_VersionFileStarted;

	// isMountedのキャッシュ
	mutable bool							m_bMounted;

protected:
	// 物理ファイルヘッダーのキャッシュ
	VersionPage*					m_pHeaderPage;
	
}; // end of class PhysicalFile::File

//
//	FUNCTION protected
//	PhysicalFile::File::convertToVersionPageID --
//		物理ページ識別子からバージョンページ識別子へ変換する
//
//	NOTES
//	物理ページ識別子からバージョンページ識別子へ変換し、返す。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		物理ページ識別子
//
//	RETURN
//	Version::Page::ID
//		バージョンページ識別子
//
//	EXCEPTIONS
//	なし
//
// virtual
inline
Version::Page::ID
File::convertToVersionPageID(const PageID	PageID_) const
{
	// バージョンページ識別子を求めて返す
	return PageID_ + this->getManageTableNum(PageID_) + 1;
	//                      物理ファイルヘッダの分 → ~~~
}

//
//	FUNCTION private
//	PhysicalFile::File::getManageTableNum --
//		空き領域管理表／物理ページ表数を返す
//
//	NOTES
//	物理ファイル先頭から引数PageID_で示される物理ページまでに
//	いくつ空き領域管理表／物理ページ表が存在するかを返す。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		物理ページ識別子
//
//	RETURN
//	PhysicalFile::PageNum
//		空き領域管理表／物理ページ表数
//
//	EXCEPTIONS
//	なし
//
inline
PageNum
File::getManageTableNum(const PageID	PageID_) const
{
	return PageID_ / this->getPagePerManageTable() + 1;
}

//	FUNCTION public
//	PhysicalFile::File::getBufferingStrategy -- バッファリング戦略を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたバッファリング戦略を表すクラス
//
//	EXCEPTIONS

inline
File::BufferingStrategy
File::getBufferingStrategy() const
{
	BufferingStrategy strategy;
	strategy.m_VersionFileInfo = m_VersionFile->getBufferingStrategy();

	return strategy;
}

//	CLASS
//	PhysicalFile::FileHeader --
//		物理ファイルヘッダクラス
//
//	NOTES
//	物理ファイルヘッダは下図のような物理構造となっている。
//	各情報はいずれも4バイトで記録されている。
//
//	管理機能なし物理ファイル	NonManageFile
//
//		┌──────────────────┐　　　　　┬
//		│　　　 物理ファイルバージョン 　　　│
//		├──────────────────┤　物理ファイルヘッダ
//		│　　　　 物理ファイルタイプ 　　　　│
//		├──────────────────┤　　　　　┴
//		│　　　　　　　　　　　　　　　　　　│
//		＝　　　　　　　　　　　　　　　　　　＝
//
//	空き領域管理機能付き物理ファイル	AreaManageFile
//	物理ページ管理機能つき物理ファイル	PageManageFile
//	物理ページ管理機能つき物理ファイル	PageManageFile2
//
//		┌──────────────────┐　　　　　┬
//		│　　　 物理ファイルバージョン 　　　│　　　　　│
//		├──────────────────┤　　　　　│
//		│　　　　 物理ファイルタイプ 　　　　│
//		├──────────────────┤　物理ファイルヘッダ
//		│　　　　使用中の物理ページ数　　　　│
//		├──────────────────┤　　　　　│
//		│　　　　未使用の物理ページ数　　　　│　　　　　│
//		├──────────────────┤　　　　　┴
//		│　　　　　　　　　　　　　　　　　　│
//		＝　　　　　　　　　　　　　　　　　　＝
//
//	物理エリア管理機能付き物理ファイル	DirectAreaFile
//
//		┌──────────────────┐　　　　　┬
//		│　　　 物理ファイルバージョン 　　　│　　　　　│
//		├──────────────────┤
//		│　　　　 物理ファイルタイプ 　　　　│　物理ファイルヘッダ
//		├──────────────────┤　
//		│　　　管理している物理ページ数　　　│　　　　　│
//		├──────────────────┤　　　　　┴
//		│　　　　　　　　　　　　　　　　　　│
//		＝　　　　　　　　　　　　　　　　　　＝

class FileHeader
{
public:

	// 物理ファイルヘッダのバージョンページ識別子
	static const Version::Page::ID	VersionPageID;

	//	STRUCT public
	//	PhysicalFile::FileHeader::Item_Common --
	//		全物理ファイルの物理ファイルヘッダに記録されている共通項目
	//
	//	NOTES

	struct Item_Common {

		File::Vers	m_Version;		// 物理ファイルバージョン
		Type		m_FileType;		// 物理ファイルタイプ
	};

	//	STRUCT public
	//	PhysicalFile::FileHeader::Item_Type1 --
	//		物理ファイルヘッダに記録されている項目（タイプ 1）
	//
	//	NOTES
	//	空き領域管理機能付き物理ファイル	AreaManageFile
	//	物理ページ管理機能付き物理ファイル	PageManageFile
	//	物理ページ管理機能つき物理ファイル	PageManageFile2

	struct Item_Type1 {

		Item_Common	m_Common;		// 全物理ファイル共通項目
		PageNum		m_UsedPageNum;	// 使用中の物理ページ数
		PageNum		m_UnusePageNum;	// 未使用の物理ページ数
	};

	//	STRUCT public
	//	PhysicalFile::FileHeader::Item_type2 --
	//		物理ファイルヘッダに記録されている項目（タイプ 2 ）
	//
	//	NOTES
	//	物理エリア管理機能付き物理ファイル	DirectAreaFile
	//
	//	管理している物理ページ数には、ルート、ノード、非木構造管理リーフを含む

	struct Item_Type2 {
	
		Item_Common	m_Common;			// 全物理ファイル共通項目
		PageNum		m_ManagePageNum;	// 管理している物理ページ数
	};

	//
	// メンバ関数
	//

	//
	//	common
	//

	// 物理ファイルヘッダを初期化する
	static void initialize(void*	FileHeaderPointer_,
						   Type		FileType_);

	// 物理ファイルヘッダの記録サイズを返す [byte]
	static Os::Memory::Size getSize(const Type	FileType_);

	//
	//	for type 1
	//

	// 使用中の物理ページ数を更新する
	static PageNum updateUsedPageNum(void*		FileHeaderPointer_,
									 const int	AddNum_);

	// 未使用の物理ページ数を更新する
	static PageNum updateUnusePageNum(void*		FileHeaderPointer_,
									  const int	AddNum_);

	// 使用中／未使用物理ページ数を取り出す
	static void fetchOutPageNum(const void*	FileHeaderPointer_,
								PageNum&	usedPageNum_,
								PageNum&	unusePageNum_);

	// 使用中の物理ページ数を返す
	static PageNum getUsedPageNum(const void*	FileHeaderPointer_);

	// 未使用の物理ページ数を返す
	static PageNum getUnusePageNum(const void*	FileHeaderPointer_);

	//
	//	for type 2
	//

	// Set the number of managed pages
	static void setManagePageNum(void*			pFileHeaderPointer_,
								 const PageNum	uiNum_);

	// 管理している物理ページ数を返す
	static PageNum getManagePageNum(const void*	pFileHeaderPointer_);

}; // end of class PhysicalFile::FileHeader

_SYDNEY_PHYSICALFILE_END
_SYDNEY_END

#endif //__SYDNEY_PHYSICALFILE_FILE_H

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
