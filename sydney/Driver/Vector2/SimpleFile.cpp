// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SimpleFile.cpp -- 
// 
// Copyright (c) 2005, 2006, 2009, 2023 Ricoh Company, Ltd.
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
#include "SyInclude.h"
#include "Vector2/SimpleFile.h"
#include "Vector2/Types.h"
#include "Vector2/FileID.h"
#include "Vector2/MessageAll_Class.h"

#include "Admin/Verification.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/Message.h"

#include "Schema/File.h"

#include "Exception/Unexpected.h"
#include "Exception/EntryNotFound.h"
#include "Exception/UniquenessViolation.h"
#include "Exception/VerifyAborted.h"

_SYDNEY_USING
_SYDNEY_VECTOR2_USING

//
//	FUNCTION public
//	Vector2::SimpleFile::SimpleFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const Vector2::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SimpleFile::SimpleFile(const Vector2::FileID& cFileID_)
	: VectorFile(cFileID_)
{
}

//
//	FUNCTION public
//	Vector2::SimpleFile::~SimpleFile -- デストラクタ
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
SimpleFile::~SimpleFile()
{
}

//
//	FUNCTION public
//	Vector2::SimpleFile::verify --
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
SimpleFile::verify()
//SimpleFile::verify(const Trans::Transaction& cTransaction_,
//				   const unsigned int uiTreatment_,
//				   Admin::Verification::Progress& cProgress_)
{
	// 各ページのエントリ
	ModUInt32 count = 0;
	// 各ページのエントリの合計
	ModUInt32 total = 0;
	// 管理テーブルページの間隔
	ModUInt32 interval = m_cPageManager.getPagePerTable();
	// ヘッダに記録されている最大ページ
	ModUInt32 uiMaxPageID = m_cPageManager.getMaxPageID();

	// iはページIDを指す。またi=0はヘッダなので1から調べる
	ModUInt32 uiPageID = 1;
	for (; uiPageID <= uiMaxPageID; ++uiPageID)
	{
		if (uiPageID % interval != 0)
		{
			// データページの場合

			// データページの取得
			// 上書きの代入は以前のページがunfixされない(2005/01/11時点)
			// なのでループの中で宣言して、毎回デコンストラクタする
			Admin::Verification::Progress cProgress(
				m_pProgress->getConnection());
			Version::Page::Memory page = m_cPageManager.verify(uiPageID,
															   cProgress);
			*m_pProgress += cProgress;
			// Vector2のverifyはファイルを訂正できないため常にReadなので
			// 引数に渡す必要がない
			//page = m_cPageManager.verify(uiPageID,
			//							 PageManager::Operation::Read,
			//							 *m_pProgress);

			if (cProgress.isGood() == false)
			{
				// 取得したpageが不正な場合
				_SYDNEY_VERIFY_INCONSISTENT(
					cProgress,
					m_cFileID.getPath(),
					Message::GettingPageFailed(uiPageID, uiMaxPageID));
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}

			// データページの走査
			for (int i = 0; i < static_cast<int>(getDataPerPage()); ++i)
			{
				// データ領域を得る
				const char* data
					= getConstPageData(page) + i * m_cData.getSize();

				if (m_cData.isNull(data) == false)
					// nullでなければインクリメントする
					++count;
			}

			if (count != getConstPageHeader(page)->m_uiCount
				|| static_cast<bool>(count) != m_cPageManager.getBit(uiPageID))
			{
				// エントリ数やページの使用状況に食い違いがあった
				_SYDNEY_VERIFY_INCONSISTENT(*m_pProgress,
											m_cFileID.getPath(),
											Message::CorruptObjectCount(
												static_cast<int>(uiPageID),
												static_cast<int>(
													getConstPageHeader(page)->m_uiCount),
												static_cast<int>(count),
												static_cast<int>(
													m_cPageManager.getBit(uiPageID))));
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}

			total += count;
			count = 0;
		}
	}

	// 最大ページを含む管理ページの最後のページ番号
	ModUInt32 uiLastPageID = (uiMaxPageID / interval) * interval + (interval - 1);
	for (; uiPageID <= uiLastPageID; ++uiPageID )
	{
		// 最大ページ以降にallocate済みのページがあれば
		// ページがfixできてしまい矛盾が判明すると思ったが
		// そもそもallocateしていないページはfixできずにassertにひっかかる？

		//page = m_cPageManager.verify(uiPageID,
		//							 *m_pProgress);
		//if (page.isOwner() == true)
		//{
		//	// 最大ページ以降なのでページを取得できないはず
		//	// <- PageManager::verifyの実装より
		//	_SYDNEY_VERIFY_INCONSISTENT(*m_pProgress,
		//								m_cFileID.getPath(),
		//								Message::CorruptMaxPage(
		//									static_cast<int>(uiPageID),
		//									static_cast<int>(uiMaxPageID)));
		//	_SYDNEY_THROW0(Exception::VerifyAborted);
		//}
		
		if (m_cPageManager.getBit(uiPageID) == true)
		{
			// 最大ページ以降なのでビットは立っていないはず
			_SYDNEY_VERIFY_INCONSISTENT(*m_pProgress,
										m_cFileID.getPath(),
										Message::CorruptPageUsage(
											static_cast<int>(uiPageID),
											static_cast<int>(uiMaxPageID)));
			_SYDNEY_THROW0(Exception::VerifyAborted);
		}
	}

	if (total != m_cPageManager.getCount())
	{
		// 各ページから計上したエントリ数とヘッダのエントリ数は同じはず
		_SYDNEY_VERIFY_INCONSISTENT(*m_pProgress,
									m_cFileID.getPath(),
									Message::CorruptTotalObjectCount(
										static_cast<int>(m_cPageManager.getCount()),
										static_cast<int>(total)));
		_SYDNEY_THROW0(Exception::VerifyAborted);
	}
}

//
//	FUNCTION public
//	Vector2::SimpleFile::fetch -- 指定されたエントリを得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiKey_
//		キー
//	Common::DataArrayData& cTuple_
//		取得した値
//	const ModVector<int>& vecField_
//		取得するフィールドの番号(1から始まる)
//
//	RETURN
//	bool
//		キーが存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
SimpleFile::fetch(ModUInt32 uiKey_,
				  Common::DataArrayData& cTuple_,
				  const int* pField_,
				  //@@const ModVector<int>& vecField_,
				  const ModSize uiFieldCount_)
{
	; _SYDNEY_ASSERT(uiFieldCount_ != 0);
	//@@; _SYDNEY_ASSERT(vecField_.getSize() != 0);
	; _SYDNEY_ASSERT(cTuple_.getCount() == (int)uiFieldCount_);
	//@@; _SYDNEY_ASSERT(cTuple_.getCount() == (int)vecField_.getSize());

	// キーのデータが格納されているページを計算する
	int position;
	Version::Page::ID uiPageID = calcPageID(uiKey_, position);

	// ページを得る
	Version::Page::Memory& page = attachPage(uiPageID,
											 PageManager::Operation::Read);
	if (page.isOwner() == false)
	{
		// ページが存在しない
		return false;
	}

	// ページヘッダーを得る
	const PageHeader* header = getConstPageHeader(page);
	// データ領域を得る
	const char* data = getConstPageData(page) + position * m_cData.getSize();

	// データを取得する
	return m_cData.getData(data, cTuple_, pField_, uiFieldCount_);
	//@@return m_cData.getData(data, cTuple_, vecField_, uiFieldCount_);
	//return m_cData.getData(data, cTuple_, vecField_);
}

//
//	FUNCTION public
//	Vector2::VectorFile::next -- 次のエントリを得る
//
//	NOTES
//	取得フィールドの指定がない場合は、次のエントリを取得するだけ。
//	指定がある場合は、エントリだけでなくフィールドの値も取得する。
//
//	ARGUMENTS
//	ModUInt32 uiKey_
//		前回取得したKey
//		初めてエントリを取得する場合はIllegalKeyが与えられる。
//	Common::DataArrayData& cTuple_
//		取得結果を入れる配列
//	const ModVector<int>& vecField_
//		取得するフィールドを指定する配列
//
//	RETURN
//	ModUInt32
//		今回取得したKey
//		取得できなかった場合はIllegalKeyを返す。
//
//	EXCEPTIONS
//
ModUInt32
SimpleFile::next(ModUInt32 uiKey_,
				 Common::DataArrayData& cTuple_,
				 const int* pField_,
				 const ModSize uiFieldCount_,
				 bool bGetByBitset_)
{
	; _SYDNEY_ASSERT(cTuple_.getCount() == (int)uiFieldCount_ ||
					 bGetByBitset_ == true);

	bool bFound = false;

	Version::Page::ID uiPageID; // 取得するエントリが含まれるページ
	int position; // ページ内でのエントリの位置

	// まずページとエントリ位置の初期状態を設定する

	if (uiKey_ == IllegalKey)
	{
		// 最初のnextである。先頭ページを取得する
		uiPageID = m_cPageManager.next(0);
		position = 0;

		if (uiPageID > m_cPageManager.getMaxPageID())
			// データが入っているページがない
			return IllegalKey;
	}
	else
	{
		// 現在キーのページIDを求める
		Version::Page::ID uiPrevID = calcPageID(uiKey_, position);
		// 次のキーのページIDと位置を求める
		uiPageID = calcPageID(uiKey_ + 1, position);
		if (uiPageID != uiPrevID)
		{
			// 一つ次のデータが一つ次のページに格納されるので、
			// 現在のページ以降でデータを含むページからエントリは得られる

			uiPageID = m_cPageManager.next(uiPrevID);
			position = 0;

			if (uiPageID > m_cPageManager.getMaxPageID())
				// 現在のページ以降にデータを含むページがない
				return IllegalKey;
		}
	}

	while (bFound == false)
	{
		// 次にページの中をしらべる

		// ページを得る
		Version::Page::Memory& page = attachPage(uiPageID,
												 PageManager::Operation::Read);
		if (page.isOwner() == false)
			// ありえない
			_SYDNEY_THROW0(Exception::Unexpected);

		// ページヘッダーを得る
		const PageHeader* header = getConstPageHeader(page);

		for (; position < static_cast<int>(getDataPerPage()); ++position)
		{
			// データ領域を得る
			const char* data
				= getConstPageData(page) + position * m_cData.getSize();

			// nullかどうかチェックする
			if (m_cData.isNull(data) == false)
			{
				// 見つかった
				bFound = true;

				if (uiFieldCount_ != 0 && bGetByBitset_ == false)
					// 指定されたフィールドを取得する
					m_cData.getData(data, cTuple_, pField_, uiFieldCount_);

				break;
			}
		}

		if (position == static_cast<int>(getDataPerPage()))
		{
			// このページにデータはあったが、初期位置より後ろにはなかった

			// 次のページ
			uiPageID = m_cPageManager.next(uiPageID);
			position = 0;

			if (uiPageID > m_cPageManager.getMaxPageID())
				// 現在のページ以降にデータを含むページがない
				return IllegalKey;
		}
	}

	return calcKey(uiPageID, position);
}


//
//	FUNCTION private
//	Vector2::VectorFile::prev -- 前のエントリを得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiKey_
//		前回取得したKey
//		初めてエントリを取得する場合は最大値を与える
//	Common::DataArrayData& cTuple_
//		取得結果を入れる配列
//	const ModVector<int>& vecField_
//		取得するフィールドを指定する配列
//
//	RETURN
//	ModUInt32
//		今回取得したKey
//		Keyが取得できなかった場合はIllegalIDを返す
//
//	EXCEPTIONS
//
ModUInt32
SimpleFile::prev(ModUInt32 uiKey_,
				 Common::DataArrayData& cTuple_,
				 const int* pField_,
				 const ModSize uiFieldCount_)
{
	; _SYDNEY_ASSERT(uiFieldCount_ != 0);
	; _SYDNEY_ASSERT(cTuple_.getCount() == (int)uiFieldCount_);

	bool bFound = false;

	Version::Page::ID uiPageID; // 取得するエントリが含まれるページ
	int position; // ページ内でのエントリの位置
	
	if (uiKey_ == IllegalKey)
	{
		// 最初のprevである。allocate済みの最大ページを取得する
		uiPageID = m_cPageManager.getMaxPageID();
		position = static_cast<int>(getDataPerPage() - 1);

		if (uiPageID == 0)
			// データが入っているページがない
			return IllegalKey;
	}
	else if (uiKey_ == 0)
	{
		// これより前のエントリはない
		return IllegalKey;
	}
	else
	{
		// 現在キーのページIDを求める
		Version::Page::ID uiPrevID = calcPageID(uiKey_, position);
		// 前のキーの位置を求める
		uiPageID = calcPageID(uiKey_ - 1, position);
		if (uiPageID != uiPrevID)
		{
			// 一つ前のデータが一つ前のページに格納されるので、
			// 現在のページ以前でデータを含むページからエントリは得られる
				
			uiPageID = m_cPageManager.prev(uiPrevID);
			position = static_cast<int>(getDataPerPage() - 1);
			if (uiPageID == 0)
			{
				// 現在のページ以前にデータを含むページがない
				return IllegalKey;
			}
		}
	}

	while (bFound == false)
	{
		// 次にページの中をしらべる

		// ページを得る
		Version::Page::Memory& page = attachPage(uiPageID,
												 PageManager::Operation::Read);
		if (page.isOwner() == false)
		{
			// ありえない
			_SYDNEY_THROW0(Exception::Unexpected);
		}

		// ページヘッダーを得る
		const PageHeader* header = getConstPageHeader(page);

		for (; position != -1; --position)
		{
			// データ領域を得る
			const char* data
				= getConstPageData(page) + position * m_cData.getSize();

			// nullかどうかチェックする
			if (m_cData.isNull(data) == false)
			{
				// 見つかった
				bFound = true;
				if (uiFieldCount_)
				//@@if (vecField_.getSize())
					m_cData.getData(data, cTuple_, pField_, uiFieldCount_);
					//@@m_cData.getData(data, cTuple_, vecField_, uiFieldCount_);
					//m_cData.getData(data, cTuple_, vecField_);
				break;
			}
		}

		if (position == -1)
		{
			// このページにデータはあったが、初期位置より後ろにはなかった

			// 前のページ
			uiPageID = m_cPageManager.prev(uiPageID);
			position = static_cast<int>(getDataPerPage() - 1);
			if (uiPageID == 0)
			{
				// 現在のページ以前にデータを含むページがない
				return IllegalKey;
			}
		}
	}

	return calcKey(uiPageID, position);
}

//
//	FUNCTION public
//	Vector2::SimpleFile::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	ModUint32 uiKey_
//		キーの値(ROWID)
//	const Common::DataArrayData& cValue_
//		挿入するデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SimpleFile::insert(ModUInt32 uiKey_,
				   const Common::DataArrayData&	cValue_)
{
	// FileIdはキーの分を除いているので追加する
	; _SYDNEY_ASSERT(cValue_.getCount() == (int)m_cFileID.getFieldCount() + 1);
	
	// 書き込む位置
	int position;
	// 書き込むページのID
	Version::Page::ID uiPageID = calcPageID(uiKey_, position);
	// 書き込むページ
	Version::Page::Memory& page
		= attachPage(uiPageID, PageManager::Operation::Insert);

	if (page.isOwner() == false)
	{
		// ページが得られなかった(ありえない)
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	// ページヘッダーを得る
	PageHeader* header = getPageHeader(page);
	// データ領域を得る
	char* data = getPageData(page) + position * m_cData.getSize();

	if (m_cData.isNull(data) == false)
	{
		// キーの指すデータがすでに存在する
		_SYDNEY_THROW0(Exception::UniquenessViolation);
	}
	
	// 書き換えるのでdirtyにする
	page.dirty();
	// データを格納する
	m_cData.dump(data, cValue_);

	int step = 0;

	try
	{
		if (header->m_uiCount == 0)
		{
			// このページには初めてデータを挿入する
			m_cPageManager.on(uiPageID);
		}
		++step;
	
		// ページのカウントをインクリメントする
		header->m_uiCount++;
		++step;
	
		// ファイルのカウントをインクリメントする
		m_cPageManager.incrementCount();
	}
	catch (...)
	{
		try
		{
			switch (step)
			{
			case 2:
				header->m_uiCount--;
			case 1:
				if (header->m_uiCount == 0) m_cPageManager.off(uiPageID);
			case 0:
				m_cData.reset(data);
			}
		}
		catch (...)
		{
			// リカバリーに失敗した
			SydErrorMessage << "recovery failed." << ModEndl;
			// データベースの利用可能性をOFFにする
			Schema::File::setAvailability(m_cFileID.getLockName(), false);
		}
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Vector2::SimpleFile::expunge -- 削除する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiKey_
//		削除するデータを特定するためのキー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SimpleFile::expunge(ModUInt32 uiKey_)
{
	// 削除する位置
	int position;
	// 削除するデータを含むページのID
	Version::Page::ID uiPageID = calcPageID(uiKey_, position);
	// 削除するデータを含むページ
	Version::Page::Memory& page
		= attachPage(uiPageID, PageManager::Operation::Expunge);
	if (page.isOwner() == false)
	{
		// キーの指すデータを含むページが存在しない
		ModUnicodeOstrStream stream;
		stream << uiKey_;
		_SYDNEY_THROW1(Exception::EntryNotFound, stream.getString());
	}

	// ページヘッダーを得る
	PageHeader* header = getPageHeader(page);
	// m_uiCountはデクリメントされるので 0 以外である必要があるが、
	// isNullではないデータの存在により 0 以外であることが保障される
	//if (header->m_uiCount == 0)
	//{
	//	// キーの指すデータを含むページにデータが入っていない
	//	_SYDNEY_THROW0(Exception::Unexpected);
	//}
	
	// データ領域を得る
	char* data = getPageData(page) + position * m_cData.getSize();

	if (m_cData.isNull(data))
	{
		// キーの指すデータが存在しない。
		ModUnicodeOstrStream stream;
		stream << uiKey_;
		_SYDNEY_THROW1(Exception::EntryNotFound, stream.getString());
	}

	// 書き換えるのでdirtyにする
	page.dirty();

	int step = 0;
	try
	{
		// ページのカウントをデクリメントする
		header->m_uiCount--;
		++step;
		
		if (header->m_uiCount == 0)
		{
			// 0件になったらページを未使用にする
			m_cPageManager.off(uiPageID);
		}
		++step;

		// ファイルのカウントをデクリメントする
		m_cPageManager.decrementCount();
		++step;
		
		// 削除する
		m_cData.reset(data);
	}
	catch (...)
	{
		try
		{
			switch (step)
			{
			case 3:
				m_cPageManager.incrementCount();
			case 2:
				if (header->m_uiCount == 0) m_cPageManager.on(uiPageID);
			case 1:
				header->m_uiCount++;
			}
		}
		catch (...)
		{
			// リカバリーに失敗した
			SydErrorMessage << "recovery failed." << ModEndl;
			// データベースの利用可能性をOFFにする
			Schema::File::setAvailability(m_cFileID.getLockName(), false);
		}
	}
}

//
//	FUNCTION public
//	Btree2::VectorFile::update -- 更新する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiKey_
//		削除するデータを特定するためのキー
//	const Common::DataArrayData& cTuple_
//		削除対象フィールドのデータ
//	const ModVector<int>& cUpdateField_
//		削除対象フィールドのフィールド番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SimpleFile::update(ModUInt32 uiKey_,
				   const Common::DataArrayData& cTuple_,
				   const int* pUpdateField_,
				   const ModSize uiFieldCount_)
{
	if (uiFieldCount_ == 0)
	//@@if (cUpdateField_.getSize() == 0)
		// 更新対象がないので何もしない
		return;

	; _SYDNEY_ASSERT(cTuple_.getCount() == (int)uiFieldCount_);
	//@@; _SYDNEY_ASSERT(cTuple_.getCount() == (int)cUpdateField_.getSize());
	// FileIdはキーの分を除いているので追加する
	; _SYDNEY_ASSERT(cTuple_.getCount() <= (int)m_cFileID.getFieldCount() + 1);
	
	// 書き込む位置
	int position;
	// 書き込むデータを含むページのID
	Version::Page::ID uiPageID = calcPageID(uiKey_, position);
	// 書き込むデータを含むページ
	Version::Page::Memory& page
		= attachPage(uiPageID, PageManager::Operation::Update);
	if (page.isOwner() == false)
	{
		// そのデータを含むページは存在しない
		ModUnicodeOstrStream stream;
		stream << uiKey_;
		_SYDNEY_THROW1(Exception::EntryNotFound, stream.getString());
	}

	// ページヘッダーを得る
	PageHeader* header = getPageHeader(page);
	// データ領域を得る
	char* data = getPageData(page) + position * m_cData.getSize();

	// 書き換えるのでdirtyにする
	page.dirty();
	// 更新する
	m_cData.update(data, cTuple_, pUpdateField_, uiFieldCount_);
	//@@m_cData.update(data, cTuple_, cUpdateField_, uiFieldCount_);
	//m_cData.update(data, cTuple_, cUpdateField_);
}

//
//	FUNCTION public
//	Vector2::SimpleFile::reset -- ページを初期化する
//
//	NOTES
//
//	ARGUMENTS
//	Version::Page::Memory& page_
//		初期化するページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SimpleFile::resetPage(Version::Page::Memory& page_)
{
	//
	//	【注意】
	//	SimpleFileにはOIDしか格納しない
	//	OIDには0xffffffは存在しないので、0xffで初期化する
	//
	
	page_.dirty();
	
	PageHeader* pHeader = getPageHeader(page_);
	pHeader->m_uiCount = 0;
	
	char* pData = getPageData(page_);
	Os::Memory::set(pData, 0xff, getPageDataSize());
}

//
//	FUNCTION private static
//	Vector2::SimpleFile::getPageData -- データ領域の先頭を得る
//	Vector2::SimpleFile::getConstPageData -- データ領域の先頭を得る
//
//	NOTES
//
//	ARGUMENTS
//	Version::Page::Memory& page_
//		取り出すページ
//
//	RETURN
//	char*
//	const char*
//		データ領域の先頭
//
//	EXCEPTIONS
//
char*
SimpleFile::getPageData(Version::Page::Memory& page_)
{
	return static_cast<char*>(page_) + sizeof(PageHeader);
}
const char*
SimpleFile::getConstPageData(const Version::Page::Memory& page_)
{
	return static_cast<const char*>(page_) + sizeof(PageHeader);
}

//
//	Copyright (c) 2005, 2006, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
