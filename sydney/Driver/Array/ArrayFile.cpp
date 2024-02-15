// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ArrayFile.cpp --
// 
// Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Array";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Array/Algorithm.h"
#include "Array/ArrayFile.h"
#include "Array/FakeError.h"
#include "Array/LockManager.h"
#include "Array/Parameter.h"
#include "Array/Condition.h"
#include "Array/MessageAll_Class.h"

#include "Common/Assert.h"

#include "Exception/Unexpected.h"
#include "Exception/VerifyAborted.h"


_SYDNEY_USING
_SYDNEY_ARRAY_USING

namespace
{
	//
	//	VARIABLE local
	//	_$$::_iCachePageSize -- ページをキャッシュする数
	//
	ParameterInteger _iCachePageSize("Array_CachePageSize", 5);

	//
	//	VARIABLE local
	//	_$$::_uiHeaderPageID -- The PageID of HeaderPage
	//
	PhysicalFile::PageID _uiHeaderPageID(0);
}

//
//	FUNCTION public
//	Array::ArrayFile::ArrayFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const Array::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ArrayFile::ArrayFile(const FileID& cFileID_)
	: File(_iCachePageSize.get()), m_cFileID(cFileID_),
	  m_cDataTree(cFileID_), m_cNullDataTree(cFileID_),
	  m_cNullArrayTree(cFileID_), m_bTreeInitialize(false),
	  m_pHeaderPageInstance(0),
	  m_uiSearchPageID(PhysicalFile::ConstValue::UndefinedPageID),
	  m_iSearchEntryPosition(-1),
	  m_uiMarkPageID(PhysicalFile::ConstValue::UndefinedPageID),
	  m_iMarkEntryPosition(-1),
	  m_pCondition(0)
{
	// 物理ファイルをattachする
	attach(cFileID_, cFileID_.getPageSize());
}

//
//	FUNCTION public
//	Array::ArrayFile::~ArrayFile -- デストラクタ
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
ArrayFile::~ArrayFile()
{
	if (m_pHeaderPageInstance) delete m_pHeaderPageInstance;
	// 物理ファイルをdetachする
	detach();
}

//
//	FUNCTION public
//	Array::ArrayFile::open --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	
//	EXCEPTIONS
//
void
ArrayFile::open(const Trans::Transaction& cTransaction_,
				LogicalFile::OpenOption::OpenMode::Value eOpenMode_)
{
	File::open(cTransaction_, eOpenMode_);

	if (m_bTreeInitialize == false)
	{
		m_cDataTree.initialize();
		m_cNullDataTree.initialize();
		m_cNullArrayTree.initialize();
		m_bTreeInitialize = true;
	}
}

//
//	FUNCTION public
//	Array::ArrayFile::close -- クローズする
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
ArrayFile::close()
{
	// Detach search page.
	m_pSearchPage = 0;

	// Reset search status.
	if (m_uiSearchPageID != PhysicalFile::ConstValue::UndefinedPageID)
		eraseLock(m_uiSearchPageID);
	m_uiSearchPageID = PhysicalFile::ConstValue::UndefinedPageID;

	// Detach header page.
	m_pHeaderPage = 0;
	
	File::close();
}

//
//	FUNCTION public
//	Array::ArrayFile::getEstimateCount -- 検索結果件数を見積もる
//
//	NOTES
//
//	ARGUMENTS
//	Array::Condition* pCondition_
//		検索条件
//
//	RETURN
//	ModUInt32
//		概算の検索件数
//
//	EXCEPTIONS
//
ModUInt32
ArrayFile::getEstimateCount(Condition* pCondition_)
{
	ModSize tupleCount = 0;

	if (pCondition_->isValid() == true)
	{
		// Set Tree.
		Tree* pTree = getTree(pCondition_->getTreeType());
	
		// Estimate the number of the entries.
		ModUInt32 entryCount = 0;
		if (pCondition_->isFetch() == true)
		{
			// フェッチでの見積もり
			entryCount = getEstimateCountForFetch(pTree, pCondition_);
		}
		else
		{
			// 検索での見積もり
			entryCount = getEstimateCountForSearch(pTree, pCondition_);
		}

		// 平均エントリ数で割る必要があるが、
		// 同じ値が入っているわけではないので、平均エントリ数が4以上の場合、
		// 平均エントリ数の半分で割ることにする
		const ModSize uiAverage = static_cast<ModSize>(
			pTree->getAverageEntryCount(
				getHeaderPage()->getTupleCount(),
				getHeaderPage()->getOneEntryTupleCount()) / 2);
	
		if (uiAverage > 1)
		{
			tupleCount = entryCount / uiAverage;
		}
		else
		{
			// The tuple's count must be smaller than the entry's count.
			tupleCount = entryCount;
		}
	}
	return (tupleCount == 0) ? 1 : tupleCount;
}

//
//	FUNCTION public
//	Array::ArrayFile::search -- Search the candidate page
//
//	NOTES
//	Maybe, the searched page includes the entry which satisfies the condition.
//
//	ARGUMENTS
//	Array::Condition* pCondition_
//		検索条件
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ArrayFile::search(Condition* pCondition_)
{
	using namespace LogicalFile;

	// Set the condition.
	m_pCondition = pCondition_;

	// Search a page.
	m_pSearchPage = 0;
	if (m_pCondition->isValid() == true)
	{
		Tree* pTree = getTree(m_pCondition->getTreeType());
		if (pTree->getCount() > 0)
		{
			const Condition::Cond& cLower = m_pCondition->getLowerCondition();
			if (cLower.m_eMatch == TreeNodeInterface::Undefined)
			{
				// Get the leftmost page.
				m_pSearchPage = attachPage(
					pTree, pTree->getLeftLeafPageID(), DataPage::PagePointer());
			}
			else
			{
				// In NullDataTree and NullArrayTree, NOT search, JUST scan.
				; _SYDNEY_ASSERT(pTree->getType() == Tree::Type::Data);

				// Get a candidate page which satisfies the condition.

				const Compare& cCompare = cLower.getKeyCompare(pTree);
				m_pSearchPage = getLeafPage(
					pTree, cLower.m_pBuffer, cCompare, cLower.m_eMatch);
			}
		}
	}

	//  Initialize search status.
	if (m_uiSearchPageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		eraseLock(m_uiSearchPageID);
	}
	if (m_pSearchPage)
	{
		m_uiSearchPageID = m_pSearchPage->getID();
		insertLock(m_uiSearchPageID);
	}
	else
	{
		// No page in tree.
		m_uiSearchPageID = PhysicalFile::ConstValue::UndefinedPageID;
	}
	m_iSearchEntryPosition = -1;

	// Initialize mark status.
	m_uiMarkPageID = PhysicalFile::ConstValue::UndefinedPageID;
	m_iMarkEntryPosition = -1;
}

//
//	FUNCTION public
//	Array::ArrayFile::get -- 取得する
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData& cData_
//		取得した結果
//	unsigned int& uiTupleID_
//		タプルID
//
//	RETURN
//	bool
//		結果が得られた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
ArrayFile::get(Common::DataArrayData& cData_,
			   unsigned int& uiTupleID_)
{
	if (m_pCondition->isValid() == false)
	{
		// Unknown condiiton.
		return false;
	}

	Tree* pTree = getTree(m_pCondition->getTreeType());
	if (pTree->getCount() == 0)
	{
		// No data.
		return false;
	}
	
	// Set next data to m_pSearchEntryBuffer.
	// If next data does not exist, UndefinedPageID is set to m_uiSearchPageID.
	next(pTree);
	if (m_uiSearchPageID == PhysicalFile::ConstValue::UndefinedPageID)
	{
		// Not found the data.
		return false;
	}

	// Get the data.
	uiTupleID_ = pTree->getRowID(m_pSearchEntryBuffer, cData_);

	return true;
}

//
//	FUNCTION public
//	Array::ArrayFile::getByBitSet -- ビットセットで得る
//
//	NOTES
//
//	ARGUMENTS
//	int iFieldID_
//		フィールド番号
//	Common::BitSet& cBitSet_
//		設定するビットセット
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ArrayFile::getByBitSet(Common::BitSet& cBitSet_)
{
	if (m_pCondition->isValid() == true)
	{
		Tree* pTree = getTree(m_pCondition->getTreeType());
		while (true)
		{
			// Set next data.
			next(pTree);
			if (m_uiSearchPageID == PhysicalFile::ConstValue::UndefinedPageID)
			{
				// Not found the data.
				break;
			}

			// Get the data by BitSet.
			pTree->getRowIDByBitSet(m_pSearchEntryBuffer, cBitSet_);
		}
	}
}

//
//	FUNCTION public
//	Array::ArrayFile::checkEntry --
//		Check whether the entry which satisfies the conditions exists.
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
ArrayFile::checkEntry(Condition* pCondition_,
					  unsigned int uiRowID_, unsigned int index_)
{
	using namespace LogicalFile;
	
	; _SYDNEY_ASSERT(pCondition_ != 0);

	if (pCondition_->getTreeType() == Tree::Type::Undefined)
	{
		// ここに来るのは、空の配列が挿入されている場合
		//
		// 空の配列の場合、LogicalInterface::open() で
		// count, fetch とも 0 になり、ただ new しただけの
		// Condition が渡されることになる
		//
		// 空の配列は検査のしようがないので、常に true とする

		return true;
	}

	bool result = false;
	if (pCondition_->isValid() == true)
	{
		Tree* pTree = getTree(pCondition_->getTreeType());

#ifdef DEBUG
		// Check the condition.
		// Key is defined when the Tree is DataTree.
		if (pTree->getType() == Tree::Type::Data)
		{
			const Condition::Cond& cLower = pCondition_->getLowerCondition();
			const Condition::Cond& cUpper = pCondition_->getUpperCondition();
			const Compare& cLowerCompare = cLower.getKeyCompare(pTree);
			; _SYDNEY_ASSERT(cLowerCompare(cUpper.m_pBuffer, cLower.m_pBuffer)
							 == 0);
			; _SYDNEY_ASSERT(
				cLower.m_eMatch == TreeNodeInterface::GreaterThanEquals &&
				cUpper.m_eMatch == TreeNodeInterface::LessThanEquals);
		}
#endif

		// Create a search entry.
		Common::DataArrayData cSearchData;
		pTree->makeDataArrayData(cSearchData);
		pTree->setKey(cSearchData, pCondition_->getLowerCondition().m_pBuffer);
		pTree->setRowID(cSearchData, uiRowID_);
		pTree->setIndex(cSearchData, index_);
		ModSize size;
		AutoPointer<ModUInt32> p = pTree->makeLeafEntry(cSearchData, size);

		// Find the entry.

		// Condition is specified, but this is an unique search.
		// Because RowID and Index is also specified.
		const Compare& cCompare = Condition::getCompare(pTree);
		m_pSearchPage = getLeafPage(pTree, p, cCompare);
		if (m_pSearchPage != 0)
		{
			; _SYDNEY_ASSERT(m_pSearchPage->isLeaf() == true);
			DataPage::Iterator i = m_pSearchPage->find(p, cCompare);
			if (i != m_pSearchPage->end())
			{
				result = true;
			}
		}
	}
	return result;
}

//
//	FUNCTION public
//	Array::ArrayFile::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cTuple_
//		挿入するデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ArrayFile::insert(const Common::DataArrayData& cTuple_)
{
	// Check the tuple.
	if (checkTuple(cTuple_) == false)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// Get the RowID.
	const unsigned int uiRowID =
		getRowID(cTuple_, FileID::FieldPosition::RowID);
	// Not get the array of the value here.
	// See checkTuple() for details.

	//
	// Insert
	//
	if (cTuple_.getElement(FileID::FieldPosition::Array)->isNull() == true)
	{
		// For NullArray
		Common::DataArrayData cNullArrayData;
		Tree* pTree = getTree(Tree::Type::NullArray);
		initializeDataArrayData(cNullArrayData, pTree, uiRowID);

		insertOneData(pTree, cNullArrayData);
	}
	else
	{
		// Get the array of the Value.
		const Common::DataArrayData& cArray =
			getArray(cTuple_, FileID::FieldPosition::Array);

		// For Data and NullData
		Common::DataArrayData cData;
		Tree* pTree = getTree(Tree::Type::Data);
		initializeDataArrayData(cData, pTree, uiRowID);

		Common::DataArrayData cNullData;
		pTree = getTree(Tree::Type::NullData);
		initializeDataArrayData(cNullData, pTree, uiRowID);

		const int count = cArray.getCount();
		for (int index = 0; index < count; ++index)
		{
			Common::DataArrayData* pTemp = 0;
			setDataArrayData(pTemp, pTree, cData, cNullData, cArray, index);
			insertOneData(pTree, *pTemp);
		}
	}
	
	; _ARRAY_FAKE_ERROR(ArrayFile::insert);

	getHeaderPage()->incrementTupleCount();
}

//
//	FUNCTION public
//	Array::ArrayFile::update --
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cTuple_
//	const Common::DataArrayData& cKey_
//
//	RETURN
//
//	EXCEPTIONS
//
void
ArrayFile::update(const Common::DataArrayData& cTuple_,
				  const Common::DataArrayData& cKey_)
{
	// Check the key.
	if (checkTuple(cTuple_) == false || checkKey(cKey_) == false)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	const unsigned int uiRowID =
		getRowID(cTuple_, FileID::FieldPosition::RowID);

	//
	// Update
	//

	if (cTuple_.getElement(FileID::FieldPosition::Array)->isNull() == true &&
		cKey_.getElement(FileID::FieldPosition::Array)->isNull() == true)
	{
		// Both of the tuples are NullArray entries.
		// So nothing to do.
		return;
	}
	if (cTuple_.getElement(FileID::FieldPosition::Array)->isNull() == true ||
		cKey_.getElement(FileID::FieldPosition::Array)->isNull() == true)
	{
		// One array is null,
		// so all the data of the other array are inserted or expunged.
		// NOT need to update only the differences.
		
		expunge(cTuple_);

		; _ARRAY_FAKE_ERROR(ArrayFile::update);

		const Common::Data& cNewData
			= *(cKey_.getElement(FileID::FieldPosition::Array));

		Common::DataArrayData cNewTuple;
		cNewTuple.reserve(FileID::FieldCount);
		setArray(cNewTuple, FileID::FieldPosition::Array, cNewData);
		setRowID(cNewTuple, FileID::FieldPosition::RowID, uiRowID);

		insert(cNewTuple);
		return;
	}

	const Common::DataArrayData& cArray =
		getArray(cTuple_, FileID::FieldPosition::Array);
	const Common::DataArrayData& cNewArray =
		getArray(cKey_, FileID::FieldPosition::Array);
	
	Common::DataArrayData cData;
	Tree* pTree = getTree(Tree::Type::Data);
	initializeDataArrayData(cData, pTree, uiRowID);

	Common::DataArrayData cNullData;
	pTree = getTree(Tree::Type::NullData);
	initializeDataArrayData(cNullData, pTree, uiRowID);

	const int count = cArray.getCount();
	const int newCount = cNewArray.getCount();
	for (int index = 0; index < count; ++index)
	{
		// Check each data.
		// If the data and the new data is same, not update.
		
		if (index >= newCount)
		{
			// Expunge the current entry.

			Common::DataArrayData* pTemp = 0;
			setDataArrayData(pTemp, pTree, cData, cNullData, cArray, index);
			expungeOneData(pTree, *pTemp);
			continue;
		}

		// Update the entry.
		const Common::Data::Pointer pElement = cArray.getElement(index);
		const Common::Data::Pointer pNewElement = cNewArray.getElement(index);
		if (pElement->isNull() == true)
		{
			if (pNewElement->isNull() == true)
			{
				// Both of the elements are same.
				continue;
			}
			else
			{
				pTree = getTree(Tree::Type::NullData);
				setDataArrayData(cNullData, pTree, pElement, index);
				expungeOneData(pTree, cNullData);

				; _ARRAY_FAKE_ERROR(ArrayFile::update);

				pTree = getTree(Tree::Type::Data);
				setDataArrayData(cData, pTree, pNewElement, index);
				insertOneData(pTree, cData);
			}
		}
		else
		{
			if (pNewElement->isNull() == true)
			{
				pTree = getTree(Tree::Type::Data);
				setDataArrayData(cData, pTree, pElement, index);
				expungeOneData(pTree, cData);

				; _ARRAY_FAKE_ERROR(ArrayFile::update);

				pTree = getTree(Tree::Type::NullData);
				setDataArrayData(cNullData, pTree, pNewElement, index);
				insertOneData(pTree, cNullData);
			}
			else
			{
				if (pElement.get()->equals(pNewElement.get())
					== true)
				{
					// Both of the elements are same.
					continue;
				}
				else
				{
					pTree = getTree(Tree::Type::Data);
					setDataArrayData(cData, pTree, pElement, index);
					expungeOneData(pTree, cData);

					; _ARRAY_FAKE_ERROR(ArrayFile::update);

					// NOT need to change the index.
					pTree->setKey(cData, pNewElement);
					insertOneData(pTree, cData);
				}
			}
		}
	} // for

	// Insert the new added entries.
	for (int index = count; index < newCount; ++index)
	{
		Common::DataArrayData* pTemp = 0;
		setDataArrayData(pTemp, pTree, cData, cNullData, cNewArray, index);
		insertOneData(pTree, *pTemp);
	}

	// NOT need to change the count of the tuples.
}

//
//	FUNCTION public
//	Array::ArrayFile::expunge --
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cTuple_
//
//	RETURN
//
//	EXCEPTIONS
//
void
ArrayFile::expunge(const Common::DataArrayData& cTuple_)
{
	// [YET!] insertと処理が同じ。呼び出す関数が異なるだけ。
	
	// Check the tuple.
	if (checkTuple(cTuple_) == false)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// RowIDを得る
	const unsigned int uiRowID =
		getRowID(cTuple_, FileID::FieldPosition::RowID);

	//
	// Expunge
	//
	if (cTuple_.getElement(FileID::FieldPosition::Array)->isNull() == true)
	{
		// For NullArray
		Common::DataArrayData cNullArrayData;
		Tree* pTree = getTree(Tree::Type::NullArray);
		initializeDataArrayData(cNullArrayData, pTree, uiRowID);

		expungeOneData(pTree, cNullArrayData);
	}
	else
	{
		// For Data and NullData

		const Common::DataArrayData& cArray =
			getArray(cTuple_, FileID::FieldPosition::Array);
		
		Common::DataArrayData cData;
		Tree* pTree = getTree(Tree::Type::Data);
		initializeDataArrayData(cData, pTree, uiRowID);

		Common::DataArrayData cNullData;
		pTree = getTree(Tree::Type::NullData);
		initializeDataArrayData(cNullData, pTree, uiRowID);

		const int count = cArray.getCount();
		for (int index = 0; index < count; ++index)
		{
			Common::DataArrayData* pTemp = 0;
			setDataArrayData(pTemp, pTree, cData, cNullData, cArray, index);
			expungeOneData(pTree, *pTemp);
		}
	}

	; _ARRAY_FAKE_ERROR(ArrayFile::expunge);

	getHeaderPage()->decrementTupleCount();
}

//
//	FUNCTION public
//	Array::ArrayFile::verify -- 整合性チェック
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
ArrayFile::verify()
{
	if (m_bTreeInitialize == false)
	{
		m_cDataTree.initialize();
		m_cNullDataTree.initialize();
		m_cNullArrayTree.initialize();
		m_bTreeInitialize = true;
	}
	
	verifyOneTree(getTree(Tree::Type::Data));
	verifyOneTree(getTree(Tree::Type::NullData));
	verifyOneTree(getTree(Tree::Type::NullArray));
}

//
//	FUNCTION public
//	Array::ArrayFile::mark -- マークする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		マークできた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
ArrayFile::mark()
{
	if (m_iSearchEntryPosition == -1)
		return false;

	m_uiMarkPageID = m_uiSearchPageID;
	m_iMarkEntryPosition = m_iSearchEntryPosition;
	Os::Memory::copy(m_pMarkEntryBuffer, m_pSearchEntryBuffer,
					 FileID::MAX_SIZE * sizeof(ModUInt32));

	return true;
}

//
//	FUNCTION public
//	Array::ArrayFile::rewind -- リワインドする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		リワインドできた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
ArrayFile::rewind()
{
	if (m_iMarkEntryPosition == -1)
		return false;

	// ロック情報を削除する -> 次のget()で検索が実行される
	eraseLock(m_uiSearchPageID);

	m_pSearchPage = 0;
	m_uiSearchPageID = m_uiMarkPageID;
	m_iSearchEntryPosition = m_iMarkEntryPosition;
	Os::Memory::copy(m_pSearchEntryBuffer, m_pMarkEntryBuffer,
					 FileID::MAX_SIZE * sizeof(ModUInt32));

	return true;
}

//
//	FUNCTION public
//	Array::ArrayFile::flushAllPages -- 全ページをフラッシュする
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
ArrayFile::flushAllPages()
{
	// 検索中ページをdetachする
	m_pSearchPage = 0;
	// ヘッダーをデタッチする
	m_pHeaderPage = 0;
	// 全ページをフラッシュする
	File::flushAllPages();
	// ロック情報を削除する
	if (m_vecPageID.getSize())
		LockManager::erase(m_cFileID.getLockName(), m_vecPageID);
	// 更新ページIDを初期化する
	m_vecPageID.clear();
}

//
//	FUNCTION public
//	Array::ArrayFile::recoverAllPages -- 全ページを元に戻す
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
ArrayFile::recoverAllPages()
{
	if (getTransaction().isNoVersion() || doVerify())
	{
		// 検索中ページをdetachする
		m_pSearchPage = 0;
	}
	// ヘッダーをデタッチする
	m_pHeaderPage = 0;
	// 全ページをフラッシュする
	File::recoverAllPages();
	// 更新ページIDを初期化する
	m_vecPageID.clear();
}

//
//	FUNCTION public
//	Array::ArrayFile::attachPage -- ページを得る
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		ページID
//	const Array::DataPage::PagePointer& pParent_
//		親ページ
//	Buffer::Page::FixMode::Value eFixMode_
//		FIXモード(default Buffer::Page::FixMode::Unknown)
//
//	RETURN
//	Array::DataPage::PagePointer
//		ページ
//
//	EXCEPTIONS
//
DataPage::PagePointer
ArrayFile::attachPage(Tree* pTree_,
					  PhysicalFile::PageID uiPageID_,
					  const DataPage::PagePointer& pParent_,
					  Buffer::Page::FixMode::Value eFixMode_)
{
	if (uiPageID_ == PhysicalFile::ConstValue::UndefinedPageID)
		return DataPage::PagePointer();

	// Set page.
	DataPage::PagePointer pPage
		= _SYDNEY_DYNAMIC_CAST(DataPage*, File::findMap(uiPageID_));
	if (pPage == 0)
	{
		// Not found in attaching pages.
		
		pPage = _SYDNEY_DYNAMIC_CAST(DataPage*, popPage());
		if (pPage == 0)
		{
			// No page in cache, so new.
			
			pPage = new DataPage(*this);
		}

		// Set Tree.
		pPage->setTree(pTree_);

		// Set PhysicalPage.
		// File::m_eFixMode is used when eFixMode_ is equal to Unknown.
		PhysicalFile::Page* pPhysicalPage =
			attachPhysicalPage(uiPageID_, eFixMode_);
		pPage->setPhysicalPage(pPhysicalPage);
		
		// Attach page.
		File::attachPage(pPage);
	}

	// Set parent page.
	pPage->setParentPage(pParent_);

	return pPage;
}

//
//	FUNCTION public
//	Array::ArrayFile::allocatePage -- 
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPrevPageID_
//	PhysicalFile::PageID uiNextPageID_
//	const Array::DataPage::PagePointer& pParent_
//
//	RETURN
//	Array::DataPage::PagePointer
//
//	EXCEPTIONS
//
DataPage::PagePointer
ArrayFile::allocatePage(Tree* pTree_,
						PhysicalFile::PageID uiPrevPageID_,
						PhysicalFile::PageID uiNextPageID_,
						const DataPage::PagePointer& pParent_)
{
	// Get a PhysicalFile's page from free pages if exists.
	PhysicalFile::Page* p = File::getFreePage();
	if (p == 0)
	{
		p = File::allocatePage();
	}

	// Get a page from cache if exists.
	DataPage::PagePointer pPage = _SYDNEY_DYNAMIC_CAST(DataPage*, popPage());
	if (pPage == 0)
	{
		pPage = new DataPage(*this);
	}

	// Set Tree.
	pPage->setTree(pTree_);

	// Attach the page.
	pPage->setPhysicalPage(p, uiPrevPageID_, uiNextPageID_);
	File::attachPage(pPage);

	// Set the parent of the page.
	pPage->setParentPage(pParent_);

	return pPage;
}

//
//	FUNCTION public
//	Array::ArrayFile::findParentPage -- 親ページを検索する
//
//	NOTES
//
//	ARGUMENTS
//	Tree* pTree_
//		Tree
//	const ModUInt32* pValue_
//		キーデータ The begin of the entries in the child page
//	PhysicalFile::PageID uiChildPageID_
//		親を探している子ページ
//
//	RETURN
//	Array::DataPage::PagePointer
//		親ページ
//
//	EXCEPTIONS
//
DataPage::PagePointer
ArrayFile::findParentPage(Tree* pTree_,
						  const ModUInt32* pBeginEntry_,
						  PhysicalFile::PageID uiChildPageID_)
{
	; _SYDNEY_ASSERT(uiChildPageID_ !=
					 PhysicalFile::ConstValue::UndefinedPageID);
	// The child page is not root.
	; _SYDNEY_ASSERT(pTree_->getRootPageID() != uiChildPageID_);

	const Compare& cCompare = Condition::getCompare(pTree_);
	PhysicalFile::PageID uiPageID = pTree_->getRootPageID();

	DataPage::PagePointer pNodePage;
	do
	{
		// Search the node from root to leaf.

		// Attach the candidate of the parent page.
		if (uiPageID == PhysicalFile::ConstValue::UndefinedPageID)
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}
		pNodePage = attachPage(pTree_, uiPageID, pNodePage);
		; _SYDNEY_ASSERT(pNodePage->getCount() != 0);

		// Check the candidate.
		if (pNodePage->isLeaf() == true)
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		// Get the child, min(x | x >= k).
		// This is an unique search, so it had better to use supremum.
		DataPage::Iterator i =
			pNodePage->getIteratorTowardLeafUpperBound(pBeginEntry_, cCompare);
		uiPageID = pTree_->getPageID(*i);

		// When uiPageID_ corresponds with uiChildPageID_,
		// the parent page has to store the pBeginEntry_.
		; _SYDNEY_ASSERT(uiPageID != uiChildPageID_
						 || cCompare(*i, pBeginEntry_) == 0);
	}
	while (uiPageID != uiChildPageID_);
	return pNodePage;
}

//
//	FUNCTION pribate
//	Array::ArrayFile::getTree --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
Tree*
ArrayFile::getTree(Tree::Type::Value eTreeType_)
{
	// Attach header page.
	getHeaderPage();
	
	Tree* pTree = 0;
	switch(eTreeType_)
	{
	case Tree::Type::Data:
		pTree = &m_cDataTree;
		break;
	case Tree::Type::NullData:
		pTree = &m_cNullDataTree;
		break;
	case Tree::Type::NullArray:
		pTree = &m_cNullArrayTree;
		break;
	default:
		; _SYDNEY_ASSERT(false);
		break;
	}
	return pTree;
}

//
//	FUNCTION public
//	Array::ArrayFile::create -- ファイルを作成する
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
ArrayFile::create()
{
	// まず下位を呼ぶ
	File::create();
	try
	{
		// ヘッダーページを初期化する
		initializeHeaderPage();
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		recoverAllPages();
		File::destroy();
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Array::ArrayFile::dirtyHeaderPage -- ヘッダーページをdirtyにする
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
ArrayFile::dirtyHeaderPage()
{
	getHeaderPage()->dirty();
}

//
//	FUNCTION public
//	Array::ArrayFile::getHeaderPage -- ヘッダーページを得る
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::Page* pPhysicalPage_ (default 0)
//		ヘッダーページの物理ページ
//
//	RETURN
//	Array::HeaderPage::PagePointer
//		ヘッダーページ
//
//	EXCEPTIONS
//
HeaderPage::PagePointer
ArrayFile::getHeaderPage(PhysicalFile::Page* pPhysicalPage_)
{
	if (m_pHeaderPageInstance == 0)
	{
		// For the first time
		m_pHeaderPageInstance = new HeaderPage(*this);
	}

	if (m_pHeaderPage == 0)
	{
		// The HeaderPage has been detached or this is the first time.
		
		// Get the PhysicalFile's page.
		if (pPhysicalPage_ == 0)
		{
			pPhysicalPage_ = attachPhysicalPage(_uiHeaderPageID);
		}
		
		// Set the PhysicalFile's page to the HeaderPage.
		m_pHeaderPage = m_pHeaderPageInstance;
		m_pHeaderPage->setPhysicalPage(pPhysicalPage_);

		// Set Tree's Header.
		m_cDataTree.setHeader(m_pHeaderPage->getDataHeader());
		m_cNullDataTree.setHeader(m_pHeaderPage->getNullDataHeader());
		m_cNullArrayTree.setHeader(m_pHeaderPage->getNullArrayHeader());
	}
	return m_pHeaderPage;
}

//
//	FUNCTION private
//	Array::ArrayFile::insertOneData --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
ArrayFile::insertOneData(Tree* pTree_, const Common::DataArrayData& cData_)
{
	// Make an entry.
	ModSize size;
	AutoPointer<ModUInt32> p = pTree_->makeLeafEntry(cData_, size);

	// Get a page.
	const Compare& cCompare = Condition::getCompare(pTree_);
	DataPage::PagePointer pLeafPage = getLeafPage(pTree_, p, cCompare);
	if (pLeafPage == 0)
	{
		// LeafPage does NOT exist. It means root does NOT exist too.
		// Create root. And root double as leaf because there is just one page.
		pLeafPage = createRootPage(pTree_);
	}
	
	// Insert the entry.
	pLeafPage->insertEntry(p, size);
	pTree_->incrementCount();
}

//
//	FUNCTION private
//	Array::ArrayFile::expungeOneData --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
ArrayFile::expungeOneData(Tree* pTree_, const Common::DataArrayData& cData_)
{
	// Make an entry.
	ModSize size;
	AutoPointer<ModUInt32> p = pTree_->makeLeafEntry(cData_, size);

	// Get a page.
	const Compare& cCompare = Condition::getCompare(pTree_);
	DataPage::PagePointer pLeafPage = getLeafPage(pTree_, p, cCompare);
	if (pLeafPage == 0)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// Expunge the entry.
	pLeafPage->expungeEntry(p, size);
	pTree_->decrementCount();
}

//
//	FUNCTION private
//	Array::ArrayFile::verifyOneTree -- 整合性チェック
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
ArrayFile::verifyOneTree(Tree* pTree_)
{
	m_uiVerifyCount = 0;
	
	DataPage::PagePointer pRootPage = getRootPage(pTree_);
	if (pRootPage != 0)
	{
		if (pRootPage->getNextPageID()
			!= PhysicalFile::ConstValue::UndefinedPageID
			|| pRootPage->getPrevPageID()
			!= PhysicalFile::ConstValue::UndefinedPageID)
		{
			_SYDNEY_VERIFY_INCONSISTENT(getProgress(),
										getPath(),
										Message::IllegalRootPageID());
			_SYDNEY_THROW0(Exception::VerifyAborted);
		}
		
		// ページをverify
		pRootPage->verify();
	}

	// 左端リーフページのチェック
	if (pTree_->getLeftLeafPageID()
		!= PhysicalFile::ConstValue::UndefinedPageID)
	{
		DataPage::PagePointer pPage
			= attachPage(pTree_,
						 pTree_->getLeftLeafPageID(),
						 DataPage::PagePointer());
		if (pPage->isLeaf() != true
			|| pPage->getPrevPageID()
			!= PhysicalFile::ConstValue::UndefinedPageID)
		{
			_SYDNEY_VERIFY_INCONSISTENT(getProgress(),
										getPath(),
										Message::IllegalLeftLeafPageID());
			_SYDNEY_THROW0(Exception::VerifyAborted);
		}
	}

	// 右端リーフページのチェック
	if (pTree_->getRightLeafPageID()
		!= PhysicalFile::ConstValue::UndefinedPageID)
	{
		DataPage::PagePointer pPage
			= attachPage(pTree_,
						 pTree_->getRightLeafPageID(),
						 DataPage::PagePointer());
		if (pPage->isLeaf() != true
			|| pPage->getNextPageID()
			!= PhysicalFile::ConstValue::UndefinedPageID)
		{
			_SYDNEY_VERIFY_INCONSISTENT(getProgress(),
										getPath(),
										Message::IllegalLeftLeafPageID());
			_SYDNEY_THROW0(Exception::VerifyAborted);
		}
	}

	// エントリ数のチェック
	if (m_uiVerifyCount != pTree_->getCount())
	{
		_SYDNEY_VERIFY_INCONSISTENT(
			getProgress(),
			getPath(),
			Message::DiscordKeyNum(pTree_->getCount(), m_uiVerifyCount));
		_SYDNEY_THROW0(Exception::VerifyAborted);
	}
}

//
//	FUNCTION private
//	Array::ArrayFile::insertLock -- ロック情報を登録する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		登録するページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ArrayFile::insertLock(PhysicalFile::PageID uiPageID_)
{
	if (getTransaction().isNoVersion() == true)
	{
		LockManager::insert(m_cFileID.getLockName(),
							uiPageID_,
							this);
	}
}

//
//	FUNCTION private
//	Array::ArrayFile::eraseLock -- ロック情報を削除する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		登録するページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ArrayFile::eraseLock(PhysicalFile::PageID uiPageID_)
{
	if (getTransaction().isNoVersion() == true)
	{
		LockManager::erase(m_cFileID.getLockName(),
						   uiPageID_,
						   this);
	}
}

//
//	FUNCTION private
//	Array::ArrayFile::checkLock -- ロック情報が存在するか
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		登録するページID
//
//	RETURN
//	bool
//		エントリが存在している場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
ArrayFile::checkLock(PhysicalFile::PageID uiPageID_)
{
	bool result = true;
	if (getTransaction().isNoVersion() == true)
	{
		result = LockManager::check(m_cFileID.getLockName(),
									uiPageID_,
									this);
	}
	return result;
}

//
//	FUNCTION private
//	Array::ArrayFile::detachPage -- ページをデタッチする
//
//	NOTES
//
//	ARGUMENTS
//	Array::Page* pPage_
//		デタッチするページ
//
//	RETURN
//
//	EXCEPTIONS
//
void
ArrayFile::detachPage(Page* pPage_)
{
	// HeaderPage and DataPage are managed independently.

	if (pPage_->getID() == _uiHeaderPageID)
	{
		// For HeaderPage
		
		PhysicalFile::Page::UnfixMode::Value mode =
			PhysicalFile::Page::UnfixMode::NotDirty;
		if (pPage_->isDirty() == true)
		{
			// Prepare for detach.
			pPage_->preFlush();
			mode = PhysicalFile::Page::UnfixMode::Dirty;
		}
		m_pPhysicalFile->detachPage(pPage_->m_pPhysicalPage, mode);
	}
	else
	{
		// For DataPage
		File::detachPage(pPage_);
	}
}

//
//	FUNCTION private
//	Array::ArrayFile::initializeHeaderPage -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
ArrayFile::initializeHeaderPage()
{
	// Get a PhysicalFile's page.
	PhysicalFile::Page* p = File::allocatePage();
	if (p->getID() != _uiHeaderPageID)
	{
		// This must be the first time to allocate a PhysicalFile's page.
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	// Initialize the HeaderPage.
	HeaderPage::PagePointer pPage = getHeaderPage(p);
	pPage->initialize();
}

//
//	FUNCTION private
//	Array::ArrayFile::getLeafPage -- リーフページを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Tree* pTree_
//	const ModUInt32* pBuffer_
//		検索するためのデータ
//	const Compare& cCompare_
//		
//	TreeNodeInterface::Type eType_
//		This value is used when the bUniqueSearch_ is false.
//
//	RETURN
//	Array::DataPage::PagePointer
//		リーフページ
//
//	EXCEPTIONS
//
DataPage::PagePointer
ArrayFile::getLeafPage(Tree* pTree_,
					   const ModUInt32* pBuffer_,
					   const Compare& cCompare_,
					   LogicalFile::TreeNodeInterface::Type eType_)
{
	using namespace LogicalFile;
	
	DataPage::PagePointer pPage = getRootPage(pTree_);
	if (pPage != 0)
	{
		// Search a leaf from root to leaf.
		
		ModSize stepCount = pTree_->getStepCount();
		ModSize c = 1;
		while (pPage->isLeaf() == false)
		{
			; _SYDNEY_ASSERT(pPage->getCount() != 0);
		
			// Get a child.
			DataPage::Iterator i;
			if (pTree_->isForUniqueSearch(cCompare_) == true
				|| eType_ == TreeNodeInterface::GreaterThan)
			{
				i = pPage->getIteratorTowardLeafUpperBound(pBuffer_, cCompare_);
			}
			else
			{
				i = pPage->getIteratorTowardLeafLowerBound(pBuffer_, cCompare_);
			}
		
			// Get the child's fix mode and the parent page.
			DataPage::PagePointer pParent = pPage;
			Buffer::Page::FixMode::Value eFixMode = getFixMode();
			if (eFixMode == Buffer::Page::FixMode::ReadOnly)
			{
				// ReadOnly means never changing the parents,
				// so NOT need to set the parent.
				pParent = DataPage::PagePointer();
			}
			if (++c != stepCount)
			{
				// The child is node.
			
				// Even if File::m_eFixMode is the type of update,
				// the node is not always changed.
				eFixMode = Buffer::Page::FixMode::ReadOnly;
			}

			// Attach the child.
			PhysicalFile::PageID uiPageID = pTree_->getPageID(*i);
			pPage = attachPage(pTree_, uiPageID, pParent, eFixMode);
		}
	}
	return pPage;
}

//
//	FUNCTION private
//	Array::ArrayFile::getRootPage -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	Array::DataPage::PagePointer
//
//	EXCEPTIONS
//
DataPage::PagePointer
ArrayFile::getRootPage(Tree* pTree_)
{
	DataPage::PagePointer pPage;

	PhysicalFile::PageID uiPageID = pTree_->getRootPageID();
	if (uiPageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// Root exists!
		
		// Get fix mode.
		Buffer::Page::FixMode::Value eFixMode = Buffer::Page::FixMode::Unknown;
		ModSize stepCount = pTree_->getStepCount();
		if (stepCount != 1)
		{
			// Root does NOT double as Leaf.
			
			// Even if File::m_eFixMode is the type of update,
			// it is not always changed.
			eFixMode = Buffer::Page::FixMode::ReadOnly;
		}
		
		// Attach page.
		// File::m_eFixMode is used when eFixMode is equal to Unknown.
		pPage = attachPage(pTree_, uiPageID, DataPage::PagePointer(), eFixMode);
	}
	return pPage;
}

//
//	FUNCTION private
//	Array::ArrayFile::setRootPage --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
DataPage::PagePointer
ArrayFile::createRootPage(Tree* pTree_)
{
	DataPage::PagePointer pPage =
		allocatePage(pTree_,
					 PhysicalFile::ConstValue::UndefinedPageID,
					 PhysicalFile::ConstValue::UndefinedPageID,
					 DataPage::PagePointer());
	// There is only one page, so root double as leaf.
	pPage->setLeaf();

	// Set the page to HeaderPage.
	HeaderPage::PagePointer pHeaderPage = getHeaderPage();
	
	pTree_->setRootPageID(pPage->getID());
	pTree_->setLeftLeafPageID(pPage->getID());
	pTree_->setRightLeafPageID(pPage->getID());
	pTree_->incrementStepCount();

	return pPage;
}

//
//	FUNCTION private
//	Array::ArrayFile::next -- 次の検索結果を設定する
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
ArrayFile::next(Tree* pTree_)
{
	using namespace LogicalFile;

	// Check the result of search() or the previous result of this function.
	// UndefinedPageID means the next entry does NOT exist.
	if (m_uiSearchPageID == PhysicalFile::ConstValue::UndefinedPageID)
	{
		return;
	}

	// Check the candidate page
	// which is the rusult of search() or
	// in which the previous searched entry is included.
	if (m_pSearchPage == 0)
	{
		// The previous searched page has been detached.
		m_pSearchPage = attachPreviousSearchPage(pTree_);
		if (m_pSearchPage == 0)
		{
			return;
		}
	}
	; _SYDNEY_ASSERT(m_pSearchPage->isLeaf() == true);

	int iSearchEntryPosition = m_iSearchEntryPosition;
	while (true)
	{
		// Get the entry.
		DataPage::Iterator i = getSearchEntry(pTree_, iSearchEntryPosition);
		if (iSearchEntryPosition == -1)
		{
			// NOT found any entry.
			break;
		}

		// Compare with Upper condition.
		if (m_pCondition->isUpperConditionSatisfied(*i) == false)
		{
			// NOT found any entry.
			// Becuase the current or later entries are larger than
			// the upper condition.
			eraseLock(m_uiSearchPageID);
			m_uiSearchPageID = PhysicalFile::ConstValue::UndefinedPageID;
			break;
		}

		// Compare with Other condition.
		if (m_pCondition->isOtherConditionMatch(*i) == true)
		{
			// FOUND the entry!
			copyEntry(m_pSearchEntryBuffer, i);
			m_iSearchEntryPosition = iSearchEntryPosition;
			break;
		}
	}
}

//
//	FUNCTION private
//	Array::ArrayFile::attachPreviousSearchPage --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
DataPage::PagePointer
ArrayFile::attachPreviousSearchPage(Tree* pTree_)
{
	DataPage::PagePointer pLeafPage;
	if (checkLock(m_uiSearchPageID) == true)
	{
		// The page has been locked.

		pLeafPage = attachPage(
			pTree_, m_uiSearchPageID, DataPage::PagePointer());
		; _SYDNEY_ASSERT(pLeafPage != 0 && pLeafPage->isLeaf() == true);
	}
	else
	{
		// The page has been updated, so search again.

		const Compare& cCompare = Condition::getCompare(pTree_);
		pLeafPage = getLeafPage(pTree_, m_pSearchEntryBuffer, cCompare);
		if (pLeafPage != 0)
		{
			; _SYDNEY_ASSERT(pLeafPage->isLeaf() == true &&
							 pLeafPage->getCount() != 0);
			// Success to get the candidate page.
			
			// Get the PageID.
			m_uiSearchPageID = pLeafPage->getID();
			// Insert the lock.
			insertLock(m_uiSearchPageID);

			// Set the position of the previous searched entry.
			DataPage::Iterator i =
				pLeafPage->find(m_pSearchEntryBuffer, cCompare);
			if (i == pLeafPage->end())
			{
				// [YET!] なんでUnexpectedなのか？
				_SYDNEY_THROW0(Exception::Unexpected);
			}
			m_iSearchEntryPosition = static_cast<int>(i - pLeafPage->begin());
		}
	}
	return pLeafPage;
}

//
//	FUNCTION private
//	Array::ArrayFile::getSearchEntry --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
DataPage::Iterator
ArrayFile::getSearchEntry(Tree* pTree_, int& iSearchEntryPosition_)
{
	using namespace LogicalFile;
	
	; _SYDNEY_ASSERT(m_pSearchPage != 0);
	; _SYDNEY_ASSERT(m_pSearchPage->isLeaf() == true);

	// Get the candidate iterator.
	DataPage::Iterator i;
	if (iSearchEntryPosition_ == -1)
	{
		// For the first time
		
		const Condition::Cond& cLower = m_pCondition->getLowerCondition();
		if (cLower.m_eMatch == TreeNodeInterface::Undefined)
		{
			i = m_pSearchPage->begin();
		}
		else
		{
			// NullDataTree and NullArrayTree is only scanned.
			; _SYDNEY_ASSERT(pTree_->getType() == Tree::Type::Data);

			const Compare& cCompare = cLower.getKeyCompare(pTree_);
			if (cLower.m_eMatch == TreeNodeInterface::GreaterThanEquals)
			{
				i = m_pSearchPage->getIteratorTowardLeafLowerBound(
					cLower.m_pBuffer, cCompare);
			}
			else
			{
				i = m_pSearchPage->getIteratorTowardLeafUpperBound(
					cLower.m_pBuffer, cCompare);
			}
		}
	}
	else
	{
		// Get last position.
		i = m_pSearchPage->begin() + iSearchEntryPosition_;

		// Set next position.
		; _SYDNEY_ASSERT(i != m_pSearchPage->end());
		++i;
	}

	// Check the iterator.
	if (i == m_pSearchPage->end())
	{
		if ((m_pSearchPage = attachNextPage(pTree_)) != 0)
		{
			i = m_pSearchPage->begin();
		}
	}

	// Set the entry postion.
	if (m_pSearchPage == 0)
	{
		// Not found any entry.
		; _SYDNEY_ASSERT(m_uiSearchPageID ==
						 PhysicalFile::ConstValue::UndefinedPageID);
		iSearchEntryPosition_ = -1;
	}
	else
	{
		; _SYDNEY_ASSERT(i != m_pSearchPage->end());
		iSearchEntryPosition_ = static_cast<int>(i - m_pSearchPage->begin());
	}
	return i;
}

//
//	FUNCTION private
//	Array::ArrayFile::attachNextPage --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
DataPage::PagePointer
ArrayFile::attachNextPage(Tree* pTree_)
{
	; _SYDNEY_ASSERT(m_pSearchPage != 0);
	; _SYDNEY_ASSERT(m_uiSearchPageID !=
					 PhysicalFile::ConstValue::UndefinedPageID);
	; _SYDNEY_ASSERT(checkLock(m_uiSearchPageID) == true);

	DataPage::PagePointer page = DataPage::PagePointer();

	eraseLock(m_uiSearchPageID);

	// Get next PageID.
	m_uiSearchPageID = m_pSearchPage->getNextPageID();
	if (m_uiSearchPageID == PhysicalFile::ConstValue::UndefinedPageID)
	{
		return page;
	}

	// Attach the page.
	page = attachPage(pTree_, m_uiSearchPageID, DataPage::PagePointer());
	insertLock(m_uiSearchPageID);

	return page;
}

//
//	FUNCTION private
//	Array::ArrayFile::copyEntry -- リーフエントリをコピーする
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32* dst
//		コピー先のバッファ
//	DataPage::Iterator i_
//		コピーするエントリへのイテレータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ArrayFile::copyEntry(ModUInt32* dst, DataPage::Iterator i_)
{
	ModSize size = static_cast<ModSize>(*(i_ + 1) - *i_) * sizeof(ModUInt32);
	Os::Memory::copy(dst, *i_, size);
}

//
//	FUNCTION private
//	Array::ArrayFile::getEstimateCountForSearch
//		-- 検索での結果件数の見積もり
//
//	NOTES
//
//	ARUGMENTS
//	const Tree* pTree_
//	Array::Condition* pCondition_
//		検索条件
//
//	RETURN
//	ModUInt32
//		見積もった結果件数
//
//	EXCEPTIONS
//
ModUInt32
ArrayFile::getEstimateCountForSearch(Tree* pTree_, Condition* pCondition_)
{
	using namespace LogicalFile;
	
	; _SYDNEY_ASSERT(pTree_->getType() == pCondition_->getTreeType());
	
	// Check the type of tree.
	switch(pTree_->getType())
	{
	case Tree::Type::Data:
		break;
	case Tree::Type::NullData:
	case Tree::Type::NullArray:
		// Get all the data in the tree.
		return pTree_->getCount();
	default:
		// Unexpected.
		; _SYDNEY_ASSERT(false);
		return 0;
	}

	// The case of Tree::Type::Data
	; _SYDNEY_ASSERT(pTree_->getType() == Tree::Type::Data);
	
	if (pCondition_->isValid() == false)
		// 検索結果が0件の条件
		return 0;

	// 上限と下限の条件を得る
	const Condition::Cond& cLower = pCondition_->getLowerCondition();
	const Condition::Cond& cUpper = pCondition_->getUpperCondition();
	if (cLower.m_eMatch == TreeNodeInterface::Undefined &&
		cUpper.m_eMatch == TreeNodeInterface::Undefined)
	{
		// Get all the data in the tree.
		return pTree_->getCount();
	}

	// 全登録数
	ModUInt32 count = pTree_->getCount();
	if (count == 0)
		return 0;

	// 比較クラス
	const Compare& cCompare = cLower.getKeyCompare(pTree_);

	//
	//	【注意】
	//	登録件数が各ページに均一に分布していると仮定している。
	//
	
	// 検索での見積もりは、下限と上限のエントリをチェックし、
	// エントリが異なっているところで、見積もる。
	
	// ルートページを得る
	PhysicalFile::PageID uiPageID = pTree_->getRootPageID();
	
	; _SYDNEY_ASSERT(uiPageID != PhysicalFile::ConstValue::UndefinedPageID);
	DataPage::PagePointer pPage	= attachPage(pTree_, uiPageID,
											 DataPage::PagePointer(),
											 Buffer::Page::FixMode::ReadOnly);

	DataPage::Iterator l;
	DataPage::Iterator u;
		
	// ページ内のヒット件数を得る
	ModSize n = getHitCount(pPage, pCondition_, cCompare, l, u);

	while (true)
	{
		if (pPage->isLeaf())
		{
			// リーフなので、件数そのまま
			count = n;
			break;
		}
		else if (l == u)
		{
			// この段では違いがないので、1つ下の段を調べる
			count /= pPage->getCount();
			uiPageID = pTree_->getPageID(*u);

			; _SYDNEY_ASSERT(uiPageID !=
							 PhysicalFile::ConstValue::UndefinedPageID);
			pPage = attachPage(pTree_, uiPageID,
							   DataPage::PagePointer(),
							   Buffer::Page::FixMode::ReadOnly);
			
			// ページ内のヒット件数を得る
			n = getHitCount(pPage, pCondition_, cCompare, l, u);
		}
		else if ((u - l) == 1)
		{
			count /= pPage->getCount();
			
			// 隣り合うページがヒットしたので、
			// 下位ページをそれぞれ検索する

			PhysicalFile::PageID pid;
			
			// 下限を検索
			pid = pTree_->getPageID(*l);
			DataPage::PagePointer p0
				= attachPage(pTree_, pid,
							 DataPage::PagePointer(),
							 Buffer::Page::FixMode::ReadOnly);
			DataPage::Iterator l0;
			DataPage::Iterator u0;
			ModSize n0 = getHitCount(p0, pCondition_, cCompare, l0, u0, true);

			// 上限を検索
			pid = pTree_->getPageID(*u);
			DataPage::PagePointer p1
				= attachPage(pTree_, pid,
							 DataPage::PagePointer(),
							 Buffer::Page::FixMode::ReadOnly);
			DataPage::Iterator l1;
			DataPage::Iterator u1;
			ModSize n1 = getHitCount(p1, pCondition_, cCompare, l1, u1);

			n = n0 + n1;
			
			if (n0 != 0 && n1 == 0)
			{
				pPage = p0;
				l = l0;
				u = u0;
			}
			else if (n0 == 0)
			{
				// 下限側の戻り値が 0 の場合は、iterator は不正値なので、
				// 上限側を利用する
				
				pPage = p1;
				l = l1;
				u = u1;
			}
			else
			{
				// 両方ヒットしたのでここで見積もる
				
				if (p0->isLeaf())
				{
					count = n;
				}
				else
				{
					count /= p0->getCount();
					
					if (n == 1)
						count /= 2;
					else
						count = ((count == 0) ? 1 : count) * (n - 1);
				}
				break;
			}
		}
		else
		{
			// 上限と下限に違いがあるので、ヒット件数を見積もる

			count /= pPage->getCount();
			
			if (n == 1)
				count /= 2;
			else
				count = ((count == 0) ? 1 : count) * (n - 1);
			break;
		}
	}

	return (count == 0) ? 1 : count;
}
	
//
//	FUNCTION private
//	Array::ArrayFile::getEstimateCountForFetch
//		-- フェッチでの結果件数の見積もり
//
//	NOTES
//
//	ARUGMENTS
//	const Tree* pTree_
//	Array::Condition* pCondition_
//		検索条件
//
//	RETURN
//	ModUInt32
//		見積もった結果件数
//
//	EXCEPTIONS
//
ModUInt32
ArrayFile::getEstimateCountForFetch(Tree* pTree_, Condition* pCondition_)
{
	; _SYDNEY_ASSERT(pTree_->getType() == pCondition_->getTreeType());
	
	// Check the type of tree.
	switch(pTree_->getType())
	{
	case Tree::Type::Data:
		break;
	case Tree::Type::NullData:
	case Tree::Type::NullArray:
	default:
		// Unexpected.
		; _SYDNEY_ASSERT(false);
		return 0;
	}

	// The case of Tree::Type::Data
	; _SYDNEY_ASSERT(pTree_->getType() == Tree::Type::Data);
	
	//
	//	【注意】
	//	登録件数が各ページに均一に分布していると仮定している。
	//
	
	// フェッチでの見積もりはルートノードから同じ値のエントリを数を数え、
	// 同じ値のエントリが複数あったところで見積もる
	// つまり、取得する可能性のあるタプル数の上限を返す。

	// 全登録数
	ModUInt32 count = pTree_->getCount();
	if (count == 0)
		return 0;

	// NOT Unique Search, because this function is estimating count.
	// If unique search, the result is 1 at most.
	// In such case, NOT need to estimate it.
	const Compare& cCompare = Condition::getKeyCompare(pTree_);

	// ルートページを得る
	PhysicalFile::PageID uiPageID = pTree_->getRootPageID();
	; _SYDNEY_ASSERT(uiPageID != PhysicalFile::ConstValue::UndefinedPageID);

	while (uiPageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// ページを得る
		DataPage::PagePointer pPage
			= attachPage(pTree_,uiPageID,
						 DataPage::PagePointer(),
						 Buffer::Page::FixMode::ReadOnly);

		// 先頭要素から同じエントリ数をチェックしていく
		DataPage::Iterator i = pPage->begin();
		DataPage::Iterator j = pPage->begin();
		if (j != pPage->end())
			++j;
		
		bool same = false;
		ModUInt32 d = 1;
		for (; j != pPage->end(); ++i, ++j)
		{
			if (cCompare(*i, *j) == 0)
			{
				// 前後で同じエントリ
				same = true;
			}
			else
			{
				// 前後で異なるエントリ
				++d;
			}
		}

		if (same == true || pPage->isLeaf())
		{
			// 同じエントリがあるか、リーフなので、
			// このページの異なり数で割る
			count /= d;
			break;
		}
		else
		{
			// For Node
			
			// この段では違いがないので、1つ下の段を調べる
			count /= pPage->getCount();

			i = pPage->begin();
			i += pPage->getCount() / 2;	// 中間点を調べる
			uiPageID = pTree_->getPageID(*i);
		}
	}

	// See getEstimateCountForSearch for the reason of this correction.
	return (count == 0) ? 1 : count;
}

//
//	FUNCTION private
//	Array::ArrayFile::checkTuple -- 
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cTuple_
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
ArrayFile::checkTuple(const Common::DataArrayData& cTuple_) const
{
	// Check the count.
	if (cTuple_.getCount() != FileID::FieldCount)
	{
		return false;
	}

	// For ObjectID
	// Not check it, because ObjectID is NOT stored in this file.

	// For Array
	const Common::Data& cArray =
		*(cTuple_.getElement(FileID::FieldPosition::Array));
	if (cArray.getType() != Common::DataType::Array &&
		cArray.isNull() == false)
	{
		// cArray is null when the data is null implicitly.
		// When the data is null explicitly, the type of cArray is Array. 
		return false;
	}

	// For RowID
	const Common::Data& cRowID =
		*(cTuple_.getElement(FileID::FieldPosition::RowID));
	if (cRowID.getType() != Common::DataType::UnsignedInteger)
	{
		return false;
	}

	return true;
}

//
//	FUNCTION private
//	Array::ArrayFile::checkKey -- 
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cKey_
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
ArrayFile::checkKey(const Common::DataArrayData& cKey_) const
{
	// Check the count.
	if (cKey_.getCount() != 1)
	{
		return false;
	}

	// For Array
	const Common::Data& cArray = *(cKey_.getElement(0));
	if (cArray.getType() != Common::DataType::Array &&
		cArray.isNull() == false)
	{
		return false;
	}

	return true;
}

//
//	FUNCTION private
//	Array::ArrayFile::getRowID -- 
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cTuple_
//	int iPosition_
//
//	RETURN
//	unsigned int
//		RowID
//
//	EXCEPTIONS
//
unsigned int
ArrayFile::getRowID(const Common::DataArrayData& cTuple_,
					int iPosition_) const
{
	return _SYDNEY_DYNAMIC_CAST(
		const Common::UnsignedIntegerData&,
		*(cTuple_.getElement(iPosition_))).getValue();
}

//
//	FUNCTION private
//	Array::ArrayFile::setRowID -- 
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData& cTuple_
//	int iPosition_
//	unsigned int uiRowID_
//
//	RETURN
//
//	EXCEPTIONS
//
void
ArrayFile::setRowID(Common::DataArrayData& cTuple_,
					int iPosition_,
					unsigned int uiRowID_)
{
	Common::Data::Pointer pRowID = new Common::UnsignedIntegerData(uiRowID_);
	cTuple_.setElement(iPosition_, pRowID);
}

//
//	FUNCTION private
//	Array::ArrayFile::getArray -- 
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cTuple_
//	int iPosition_
//
//	RETURN
//	const Common::DataArrayData&
//		Value's Array
//
//	EXCEPTIONS
//
const Common::DataArrayData&
ArrayFile::getArray(const Common::DataArrayData& cTuple_,
					int iPosition_) const
{
	return _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&,
								*(cTuple_.getElement(iPosition_)));
}

//
//	FUNCTION private
//	Array::ArrayFile::setArray -- 
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData& cTuple_
//	int iPosition_
//	unsigned int uiRowID_
//
//	RETURN
//
//	EXCEPTIONS
//
void
ArrayFile::setArray(Common::DataArrayData& cTuple_,
					int iPosition_,
					const Common::Data& cData_)
{
	Common::Data::Pointer pArray;

	if (cData_.isNull() == true)
	{
		pArray = new Common::DataArrayData();
		pArray->setNull(true);
	}
	else
	{
		const Common::DataArrayData& cArray =
			_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&, cData_);
		pArray = new Common::DataArrayData(cArray);
	}
	cTuple_.setElement(iPosition_, pArray);
}

//
//	FUNCTION private
//	Array::ArrayFile::initializeDataArrayData -- Initialize DataArrayData
//
//	NOTES
//
//	ARUGMENTS
//	Common::DataArrayData& cData_
//	const Tree* pTree_
//	unsigned int uiRowID_
//
//	RETURN
//
//	EXCEPTIONS
//
void
ArrayFile::initializeDataArrayData(Common::DataArrayData& cData_,
								   const Tree* pTree_,
								   unsigned int uiRowID_) const
{
	pTree_->makeDataArrayData(cData_);
	pTree_->setRowID(cData_, uiRowID_);
}

//
//	FUNCTION private
//	Array::ArrayFile::setDataArrayData -- Set to DataArrayData
//
//	NOTES
//
//	ARUGMENTS
//	Common::DataArrayData& cData_
//	const Tree* pTree_
//	const Common::Data::Pointer pElement_
//	unsigned int uiIndex_
//
//	RETURN
//
//	EXCEPTIONS
//
void
ArrayFile::setDataArrayData(Common::DataArrayData& cData_,
							const Tree* pTree_,
							const Common::Data::Pointer pElement_,
							unsigned int uiIndex_) const
{
	pTree_->setKey(cData_, pElement_);
	pTree_->setIndex(cData_, uiIndex_);
}

//
//	FUNCTION private
//	Array::ArrayFile::setDataArrayData -- Set to DataArrayData
//
//	NOTES
//
//	ARUGMENTS
//	Common::DataArrayData& cTemp_
//	Tree*& cTree_
//	const Common::DataArrayData& cData_
//	const Common::DataArrayData& cNullData_
//	const Common::DataArrayData& cArray_
//	unsigned int uiIndex_
//
//	RETURN
//
//	EXCEPTIONS
//
void
ArrayFile::setDataArrayData(Common::DataArrayData*& pTemp_,
							Tree*& pTree_,
							Common::DataArrayData& cData_,
							Common::DataArrayData& cNullData_,
							const Common::DataArrayData& cArray_,
							unsigned int uiIndex_)
{
	Common::Data::Pointer pElement = cArray_.getElement(uiIndex_);
	if (pElement->isNull() == true)
	{
		pTree_ = getTree(Tree::Type::NullData);
		pTemp_ = &cNullData_;
	}
	else
	{
		pTree_ = getTree(Tree::Type::Data);
		pTemp_ = &cData_;
	}

	setDataArrayData(*pTemp_, pTree_, pElement, uiIndex_);
}

//
//	FUNCTION private
//	Array::DataFile::getHitCount -- ページ内の人件数を確認する
//
//	NOTES
//
//	ARGUMENTS
//	Array::DataPage::PagePointer pPage_
//		検索するページ
//	Array::Condition* pCondition_
//		検索条件
//	const Array::Compare& cCompare_
//		比較クラス
//	Array::DataPage::Iterator& l_
//		下限
//	Array::DataPage::Iterator& u_
//		上限
//		
//	RETURN
//	ModSize
//		ヒット件数
//
//	EXCEPTIONS
//
ModSize
ArrayFile::getHitCount(DataPage::PagePointer pPage_,
					   Condition* pCondition_,
					   const Compare& cCompare_,
					   DataPage::Iterator& l_,
					   DataPage::Iterator& u_,
					   bool isLower_)
{
	using namespace LogicalFile;
	
	// 上限と下限の条件を得る
	const Condition::Cond& cLower = pCondition_->getLowerCondition();
	const Condition::Cond& cUpper = pCondition_->getUpperCondition();
	
	// 下限を検索
	if (cLower.m_eMatch == TreeNodeInterface::Undefined)
	{
		// 下限がない
		l_ = pPage_->begin();
	}
	else
	{
		if (cLower.m_eMatch == TreeNodeInterface::GreaterThan)
		{
			// upper_boundで検索する
			l_ = Algorithm::upperBound(pPage_->begin(), pPage_->end(),
									   cLower.m_pBuffer, cCompare_);
		}
		else
		{
			// lower_boundで検索する
			l_ = Algorithm::lowerBound(pPage_->begin(), pPage_->end(),
									   cLower.m_pBuffer, cCompare_);
		}

		// 隣り合う2ページの下限部分の場合、ヒットしなかったら終わり
		if (isLower_ && l_ == pPage_->end())
			return 0;
		
		// その１つ前
		if (l_ != pPage_->begin() && pPage_->isLeaf() == false)
			--l_;
	}

	// 上限を検索
	if (cUpper.m_eMatch == TreeNodeInterface::Undefined)
	{
		// 上限がない
		u_ = pPage_->end();
	}
	else
	{
		if (cUpper.m_eMatch != TreeNodeInterface::LessThan)
		{
			// upper_boundで検索する
			u_ = Algorithm::upperBound(pPage_->begin(), pPage_->end(),
									   cUpper.m_pBuffer, cCompare_);
		}
		else
		{
			// lower_boundで検索する
			u_ = Algorithm::lowerBound(pPage_->begin(), pPage_->end(),
									   cUpper.m_pBuffer, cCompare_);
		}
	}
	// その1つ前
	if (u_ != pPage_->begin())
		--u_;

	if (l_ > u_)
		// ヒットしない
		return 0;

	ModSize n = (u_ - l_) + 1;

	if (n >= 1)
	{
		// B木の比較はPadSpaceで行われるが、LIKEはNoPadでする必要がある
		// そのため、例えば、'abc%' は、'abc' <= x < 'abd' ではなく、
		// 'abb' < x < 'abd' となり、多く見積もられすぎてしまう
		// よって、上限と下限に違いがあったところで、OtherConditionも
		// 評価し、その誤差を軽減する

		if (pPage_->isLeaf())
		{
			// リーフなので、全部 OtherCondition を評価する
			
			n = 0;
			DataPage::Iterator i = l_;
			while (i <= u_)
			{
				if (pCondition_->isOtherConditionMatch(*i) == true)
				{
					++n;
				}

				++i;
			}
		}
		else if (isLower_)
		{
			// ノードだけど、下限側のページなので、全部評価する

			while (l_ <= u_)
			{
				if (pCondition_->isOtherConditionMatch(*l_) == true)
					break;

				++l_;
				--n;
			}
		}
		else if (n > 2)
		{
			// ノードなので、u_-2 まで評価する

			while (n > 2)
			{
				if (pCondition_->isOtherConditionMatch(*l_) == true)
					break;

				++l_;
				--n;
			}
		}
	}

	return n;
}

//
//	Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
