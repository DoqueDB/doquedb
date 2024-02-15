// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DirectIterator.h -- 代表オブジェクトクラスを格納するファイルを走査するイテレーター
// 
// Copyright (c) 2001, 2003, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD_DIRECTITERATOR_H
#define __SYDNEY_RECORD_DIRECTITERATOR_H

#include "Common/Common.h"
#include "Common/DataArrayData.h"	// Common::DataArrayData::Pointer のため
#include "Common/Object.h"
#include "Common/DoubleLinkedList.h"
#include "PhysicalFile/Types.h"
#include "PhysicalFile/File.h"
#include "Record/Module.h"
#include "Record/DirectFile.h"

_SYDNEY_BEGIN

namespace PhysicalFile
{
class Page;
}

namespace Trans
{
class Transaction;
}

namespace Admin
{
	namespace Verification
	{
		class Progress;
	}
}

_SYDNEY_RECORD_BEGIN

class TargetFields;

//	CLASS
//	Record::DirectIterator --
//		DirectFile内を走査するイテレーターを表すクラス
//
//	NOTES

class DirectIterator : public Common::Object
{
public:
	struct SearchPage
	{
		enum Value
		{
			Read = 0,				// 探すだけ
			Replace,				// 見つけたページをアタッチした状態にする
			Verify,					// 整合性検査用
			ValueNum
		};
	};

	struct Operation
	{
		enum Value
		{
			Read = 0,				// 読み込み
			Write,					// 書き込み
			Insert,					// 新規挿入
			Expunge,				// 破棄
			Verify,					// 整合性検査
			Batch,					// バッチモードの挿入
			ValueNum
		};
	};

	// コンストラクター
	DirectIterator(DirectFile& cFile_);
	DirectIterator(DirectFile& cFile_,
				   Tools::ObjectID iFirstID_,
				   Tools::ObjectID iLastID_);
	// デストラクター
	~DirectIterator();

	// イテレーターを移動する
	bool seek(Tools::ObjectID iObjectID_, bool bKeepAttach_ = false, Operation::Value eValue_ = Operation::Read);
//	bool seek(Tools::ObjectID iObjectID_, bool bKeepAttach_ = false);
	// イテレーターを次に進める
	bool next();
#ifdef OBSOLETE
	// イテレーターを最初に設定する
	bool reset();
#endif //OBSOLETE
	// イテレーターを無効化する
	void invalidate();

	// イテレーターが有効かを得る
	bool isValid() const;

#ifdef OBSOLETE
	// イテレーターが指しているページIDを得る
	PhysicalFile::PageID getPageID() const;
#endif //OBSOLETE

	// StartIDを上書きする
	void setStartObjectID(const Tools::ObjectID& iFirstID_);
	// LastIDを上書きする
	void setEndObjectID(const Tools::ObjectID& iLastID_);

	// イテレーターが指しているオブジェクトのオブジェクトIDを得る
	Tools::ObjectID getObjectID() const;

	// イテレーターが指している位置の1つ前/後の使用中オブジェクトIDを得る
	Tools::ObjectID getNextObjectID(SearchPage::Value eSearchPage_ = SearchPage::Read);
	Tools::ObjectID getPrevObjectID(SearchPage::Value eSearchPage_ = SearchPage::Read);

	// イテレーターが指している位置の後の未使用オブジェクトIDを得る
	Tools::ObjectID getNextFreeObjectID(SearchPage::Value eSearchPage_ = SearchPage::Read);

	// イテレーターが指している位置の1つ後の使用中オブジェクトIDをVerifyモードで得る
	Tools::ObjectID getNextObjectIDVerify(Admin::Verification::Progress* pProgress_);

	// オブジェクトを読み込む
	void read(const TargetFields* pTarget_,
			  DirectFile::DataPackage& cData_);
	// イテレーターの位置のnull bitmapと可変長オブジェクトIDだけを得る
	void readObjectHeader(DirectFile::DataPackage& cData_);
	// イテレーターの位置の可変長オブジェクトIDだけを得る
	Tools::ObjectID readVariableID();

	// オブジェクトを挿入する
	Tools::ObjectID insert(DirectFile::DataPackage& cData_,
						   Tools::ObjectID iObjectID_ = Tools::m_UndefinedObjectID,
						   bool bUseFreeObjectID_ = false);

	// オブジェクトを更新する
	void update(DirectFile::DataPackage& cData_,
				const TargetFields* pTarget_);

	// オブジェクトを削除する
	void expunge(Tools::ObjectID iFreeObjectID_);

	//////////////////////////
	// 整合性検査用のメソッド
	//////////////////////////
	// イテレーターを次のページに進める
	PhysicalFile::PageID nextPage(Admin::Verification::Progress* pProgress_ = 0);
	// イテレーターの位置にオブジェクトがあるかを得る
	bool isExist();

	// イテレーターが載っているページの整合性検査を行う
	ModSize verifyPageData(Admin::Verification::Treatment::Value iTreatment_,
						   Admin::Verification::Progress& cProgress_);
	// 削除オブジェクトIDの整合性検査を行う
	Tools::ObjectID verifyFreeObjectID(Admin::Verification::Treatment::Value iTreatment_,
									   Admin::Verification::Progress& cProgress_,
									   Tools::ObjectID iFreeID_);

	// イテレーターの場所のページ記述子を得る
	void				attachPage(Operation::Value eValue_,
								   Admin::Verification::Progress* pProgress_ = 0);
	// イテレーターの場所のページ記述子を解放する
	void				detachPage(Operation::Value eValue_);

	// イテレーターの場所のページ記述子を放棄（デタッチしない）
	void releasePage() { m_pPage = 0; }

	////////////////////////////////////
	// 内部関数で使用する定義だが
	// .cppの中でファイルスコープの定義を
	// 使うためにpublicに宣言するもの
	////////////////////////////////////

	// ページヘッダー
	struct PageHeader
	{
		ModSize			m_iObjectNumber;
		ModSize			m_iObjectPerPage;
		Tools::BitMap	m_cBitMap;

		bool isExist(ModSize i_) {return m_cBitMap.test(i_);}
		void setInsert(ModSize i_) {m_cBitMap.set(i_);++m_iObjectNumber;}
		void setExpunge(ModSize i_) {m_cBitMap.reset(i_);--m_iObjectNumber;}
	};

	// AreaIDのイテレーター
	typedef int AreaID;
	class AreaIDIterator
	{
	public:
		virtual bool hasNext() const = 0;
		virtual AreaID getCurrent() = 0;
		virtual AreaID getNext() = 0;
		virtual AreaID reset() = 0;
	};

	// AreaIDIteratorのサブクラスを定義する
	// 昇順:
	class AreaIDAscendingIterator : public AreaIDIterator
	{
	public:
		AreaIDAscendingIterator(AreaID iMax_)
			: m_iCurrent(0), m_iMax(iMax_) {}

		void setCurrent(AreaID iCurrent_) { m_iCurrent = iCurrent_; }
		bool hasNext() const { return m_iCurrent < m_iMax; }
		AreaID getCurrent() { return m_iCurrent; }
		AreaID getNext() { return ++m_iCurrent; }
		AreaID reset() { return m_iCurrent = 0; }

	private:
		AreaID m_iCurrent;
		AreaID m_iMax;
	};

	// 降順:
	class AreaIDDescendingIterator : public AreaIDIterator
	{
	public:
		AreaIDDescendingIterator(AreaID iMax_)
			: m_iCurrent(0), m_iMax(iMax_) {}

		void setCurrent(AreaID iCurrent_) { m_iCurrent = iCurrent_; }
		bool hasNext() const { return m_iCurrent >= 0; }
		AreaID getCurrent() { return m_iCurrent; }
		AreaID getNext() { return --m_iCurrent; }
		AreaID reset() { return m_iCurrent = m_iMax; }

	private:
		AreaID m_iCurrent;
		AreaID m_iMax;
	};

	// ヘッダーのオブジェクト数を調べる関数
	typedef bool (*CheckObjectNumber)(PageHeader& cHeader_);

	// ヘッダーのビットマップを調べる関数
	typedef bool (*CheckBitmap)(PageHeader& cHeader_, AreaID i_);

	// ページIDを探す関数
	typedef PhysicalFile::PageID
		(DirectIterator::* GetPageID)(PhysicalFile::PageID PageID_);

private:
	struct Direction
	{
		enum Value
		{
			Forward = 0,			// IDの昇順
			Backward,				// IDの降順
			ValueNum
		};
	};

	//
	//	CLASS
	//	Record::DirectIterator::Page -- 物理ページのラッパクラス
	//
	class Page
	{
	public:
		Page(PhysicalFile::Page* pPage_)
			: m_pPage(pPage_), _prev(0), _next(0) {}
		~Page() {}

		PhysicalFile::Page* m_pPage;
		Page* _prev;
		Page* _next;
	};

	//////////////
	// 内部関数 //
	//////////////

	PhysicalFile::Page* attachPage(PhysicalFile::PageID iPageID_,
								   Operation::Value eValue_,
								   Admin::Verification::Progress* pProgress_ = 0);

	void				detachPage(PhysicalFile::Page*& pPage_,
								   Operation::Value eValue_);

	// ページのヘッダを読み書きする
	static const char*	readHeader(PhysicalFile::Page& cPage_,
								   DirectIterator::PageHeader& cPageHeader_);
	static char*		writeHeader(PhysicalFile::Page& cPage_,
									const DirectIterator::PageHeader& cPageHeader_);
	static const char*	skipHeader(PhysicalFile::Page& cPage_,
								   const DirectIterator::PageHeader& cPageHeader_);

	void				readHeader();
	void				writeHeader();
	void				skipHeader();

	// オブジェクトの情報を読み書きする
	const char*			readObjectHeader(const char* pPointer_,
										 DirectFile::DataPackage& cData_);
	char*				writeObjectHeader(char* pPointer_,
										  const DirectFile::DataPackage& cData_);

	// さまざまな条件でオブジェクトIDを探す関数
	Tools::ObjectID		searchObjectID(AreaIDIterator& cIterator_,
									   CheckBitmap cCheckBitmap_,
									   CheckObjectNumber cCheckObjectNumber_,
									   GetPageID cGetPageID_,
									   SearchPage::Value eSearchPage_,
									   Admin::Verification::Progress* pProgress_ = 0);

	// 次のページIDを得る
	PhysicalFile::PageID getNextPageID(PhysicalFile::PageID uiCurrentPageID_);
	// 前のページIDを得る
	PhysicalFile::PageID getPrevPageID(PhysicalFile::PageID uiCurrentPageID_);

	// キャッシュしているページを検索する
	PhysicalFile::Page* getCachePage(PhysicalFile::PageID uiPageID_);
	// キャッシュをクリアする
	void clearCachePage();

	////////////////////
	// データメンバー //
	////////////////////

	// イテレーターが走査するファイル
	DirectFile& m_cFile;

	// イテレーターが指しているオブジェクトID
	Tools::ObjectID m_iObjectID;
	// イテレーターが指しているオブジェクトが載っているページIDとページ内位置
	PhysicalFile::PageID m_iPageID;
	PhysicalFile::AreaID m_iAreaID;
	// イテレーターが指しているオブジェクトが載っているページ記述子
	PhysicalFile::Page* m_pPage;
	// そのときのオペレーション
	Operation::Value m_eOperation;

	union {
		char*		m_pPointer;
		const char*	m_pConstPointer;
	};

	// イテレーションの方向
	Direction::Value m_eDirection;

	// イテレーションを開始するID
	Tools::ObjectID m_iStartObjectID;
	// イテレーションを終了するID
	Tools::ObjectID m_iEndObjectID;

	// 現在指しているページのヘッダ情報
	PageHeader m_cPageHeader;

	// キャッシュ
	typedef Common::DoubleLinkedList<Page> PageList;
	PageList m_cPageList;

	// Cache page for batch mode
	PhysicalFile::Page* m_pBatchCachePage;

	// fetchかどうか
	bool m_bFetch;

	// AreaIDのイテレータ
	AreaIDAscendingIterator m_cAscendingIterator;
	AreaIDDescendingIterator m_cDescendingIterator;
};

_SYDNEY_RECORD_END
_SYDNEY_END

#endif // __SYDNEY_RECORD_DIRECTITERATOR_H

//
//	Copyright (c) 2001, 2003, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
