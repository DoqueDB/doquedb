// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SectionFile.cpp --
// 
// Copyright (c) 2003, 2005, 2006, 2008, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "FullText/SectionFile.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/ObjectIDData.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/UnsignedIntegerArrayData.h"

#include "FileCommon/OpenOption.h"
#include "FileCommon/FileOption.h"

#include "Btree/File.h"
#include "Btree/OpenOption.h"

#include "Exception/BadArgument.h"

#include "ModOsDriver.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT_USING

namespace
{
	//
	//	VARIABLE local
	//	_$$::_Directory -- ディレクトリ
	//
	ModUnicodeString _Directory("SectInfo");
}

//
//	FUNCTION public
//	FullText::SectionFile::SectionFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SectionFile::SectionFile(FullText::FileID& cFileID_)
	: m_cFileID(cFileID_)
{
	setNewPath(cFileID_.getPath());
	m_pBtreeFile = new Btree::File(cFileID_.getSection());
}

//
//	FUNCTION public
//	FullText::SectionFile::~SectionFile -- デストラクタ
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
SectionFile::~SectionFile()
{
	delete m_pBtreeFile;
}

//
//	FUNCTION public
//	FullText::SectionFile::create -- ファイルを作成する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	const LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
void
SectionFile::create(const Trans::Transaction& cTransaction_)
{
	m_cFileID.setSection(m_pBtreeFile->create(cTransaction_));
}

//	FUNCTION public
//	FullText::SectionFile::destroy -- ファイルを破棄する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
SectionFile::destroy(const Trans::Transaction& cTransaction_)
{
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく削除する
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	m_pBtreeFile->destroy(cTransaction_);
	ModOsDriver::File::rmAll(m_cPath, ModTrue);
}

//
//	FUNCTION public
//	FullText::SectionFile::mount -- ファイルをマウントする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SectionFile::mount(const Trans::Transaction& cTransaction_)
{
	m_cFileID.setSection(m_pBtreeFile->mount(cTransaction_));
}

//
//	FUNCTION public
//	FullText::SectionFile::unmount -- ファイルをアンマウントする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SectionFile::unmount(const Trans::Transaction& cTransaction_)
{
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかくアンマウントする
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	m_cFileID.setSection(m_pBtreeFile->unmount(cTransaction_));
}

//
//	FUNCTION public
//	FullText::SectionFile::recover -- 障害から回復する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Trans::TimeStamp& cPoint_
//		チェックポイント
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SectionFile::recover(const Trans::Transaction& cTransaction_,
					 const Trans::TimeStamp& cPoint_)
{
	if (isMounted(cTransaction_))
	{
		m_pBtreeFile->recover(cTransaction_, cPoint_);

		if (!isAccessible())
		{
			// リカバリの結果
			// 実体である OS ファイルが存在しなくなったので、
			// サブディレクトを削除する

			Os::Directory::remove(m_cPath);
		}
	}
}

//
//	FUNCTION public
//	FullText::SectionFile::open -- ファイルをオープンする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const LogicalFile::OpenOption& cOption_
//		オープンオプション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SectionFile::open(const Trans::Transaction& cTransaction_,
					const LogicalFile::OpenOption& cOption_)
{
	// オープンモード
	int iOpenMode = cOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(
		FileCommon::OpenOption::OpenMode::Key));
	// 見積りモードかどうか
	bool bEstimate = cOption_.getBoolean(_SYDNEY_OPEN_PARAMETER_KEY(
		FileCommon::OpenOption::Estimate::Key));

	// オープンする
	open(cTransaction_, iOpenMode, bEstimate);
}

//
//	FUNCTION public
//	FullText::SectionFile::open -- ファイルをオープンする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	int iOpenMode_
//		オープンモード
//	bool bEstimate_
//		見積モードかどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SectionFile::open(const Trans::Transaction& cTransaction_,
					int iOpenMode_,
					bool bEstimate_)
{
	LogicalFile::OpenOption cOpenOption;

	if (iOpenMode_ == LogicalFile::OpenOption::OpenMode::Update ||
		iOpenMode_ == LogicalFile::OpenOption::OpenMode::Batch)
	{
		// オープンモード
		cOpenOption.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
			FileCommon::OpenOption::OpenMode::Key),
								 LogicalFile::OpenOption::OpenMode::Update);
	}
	else if (iOpenMode_ == LogicalFile::OpenOption::OpenMode::Read
				 || iOpenMode_ == LogicalFile::OpenOption::OpenMode::Search)
	{
		// オープンモード
		cOpenOption.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
			FileCommon::OpenOption::OpenMode::Key),
								 LogicalFile::OpenOption::OpenMode::Read);
		// 検索用のパラメータを設定する
		cOpenOption.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
			FileCommon::OpenOption::ReadSubMode::Key),
								 FileCommon::OpenOption::ReadSubMode::Fetch);
		cOpenOption.setInteger(_SYDNEY_BTREE_OPEN_PARAMETER_KEY(
			Btree::OpenOption::FetchFieldNumber::Key), 1);
		cOpenOption.setInteger(_SYDNEY_BTREE_OPEN_PARAMETER_FORMAT_KEY(
			Btree::OpenOption::FetchFieldIndex::Key, 0), 1);
	}

	// 見積りモードかどうか
	cOpenOption.setBoolean(_SYDNEY_OPEN_PARAMETER_KEY(
		FileCommon::OpenOption::Estimate::Key), bEstimate_);

	m_pBtreeFile->open(cTransaction_, cOpenOption);
}

//
//	FUNCTION public
//	FullText::SectionFile::close -- ファイルをクローズする
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
SectionFile::close()
{
	m_pBtreeFile->close();
	m_cKey.clear();
	m_cData.clear();
	m_cTuple.clear();
}

//
//	FUNCTION public
//	FullText::SectionFile::move -- ファイルを移動する
//
//	NOTES
//	移動元と移動先のパスが異なっていることが前提。
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Common::StringArrayData& cArea_
//		移動先のエリア
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SectionFile::move(const Trans::Transaction& cTransaction_,
				  const Common::StringArrayData& cArea_)
{
	// 現在のパスを得る
	Os::Path cOrgPath = m_cPath;

	// 新しいパス
	Os::Path cPath = getNewPath(cArea_.getElement(0));

	bool accessible = isAccessible();
	int step = 0;
	try
	{
		Common::StringArrayData cAreaPath;
		cAreaPath.setElement(0, cPath);
		m_pBtreeFile->move(cTransaction_, cAreaPath);
		step++;
		if (accessible)
			// 古いディレクトリを削除する
			Os::Directory::remove(cOrgPath);
		step++;
	}
	catch (...)
	{
		switch (step)
		{
		case 1:
			{
				Common::StringArrayData cAreaPath;
				cAreaPath.setElement(0, cOrgPath);
				m_pBtreeFile->move(cTransaction_, cAreaPath);
			}
		case 0:
			if (accessible)
				Os::Directory::remove(cPath);
		}
		_SYDNEY_RETHROW;
	}

	// 新しいパスを設定する
	setNewPath(cArea_.getElement(0));
}

//
//	FUNCTION public
//	FullText::SectionFile::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiTupleID_
//		タプルID
//	const Common::Data& cSectionOffset_
//		セクション情報
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SectionFile::insert(ModUInt32 uiTupleID_,
					const Common::Data& cSectionOffset_)
{
	if (m_cTuple.getCount() == 0) {
		m_cTuple.reserve(3);
		m_cTuple.pushBack(new Common::ObjectIDData);	// OID
		m_cTuple.pushBack(new Common::UnsignedIntegerData); // ROWID
		m_cTuple.pushBack(new Common::DataArrayData);
	} else {
		; _SYDNEY_ASSERT(m_cTuple.getCount() == 3);
		m_cTuple.getElement(0)->setNull();
	}
	_SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData&,
						 *(m_cTuple.getElement(1).get())).setValue(uiTupleID_);
	convertSectionData(cSectionOffset_, m_cTuple.getElement(2));

	m_pBtreeFile->insert(&m_cTuple);
}

//
//	FUNCTION public
//	FullText::SectionFile::update -- 更新する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiTupleID_
//		タプルID
//	cosnt Common::Data& cSectionOffset_
//		セクション情報
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SectionFile::update(ModUInt32 uiTupleID_,
					const Common::Data& cSectionOffset_)
{
	makeKey(uiTupleID_);

	if (m_cData.getCount() == 0) {
		m_cData.reserve(2);
		m_cData.pushBack(new Common::UnsignedIntegerData);
		m_cData.pushBack(new Common::DataArrayData);
	}
	_SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData&,
						 *(m_cData.getElement(0).get())).setValue(uiTupleID_);
	convertSectionData(cSectionOffset_, m_cData.getElement(1));

	m_pBtreeFile->update(&m_cKey, &m_cData,
						 Btree::File::UpdateSearchTarget::Key);
}

//
//	FUNCTION public
//	FullText::SectionFile::expunge -- 削除する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiTupleID_
//		タプルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SectionFile::expunge(ModUInt32 uiTupleID_)
{
	makeKey(uiTupleID_);

	m_pBtreeFile->expunge(&m_cKey,
							Btree::File::UpdateSearchTarget::Key);
}

//
//	FUNCTION public
//	FullText::SectionFile::get -- セクション情報を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiTupleID_
//		タプルID
//	Common::Data& cSectionOffset_
//		セクション情報
//
//	RETURN
//	bool
//		ヒットした場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
SectionFile::get(ModUInt32 uiTupleID_,
				 Common::Data& cSectionOffset_)
{
	makeKey(uiTupleID_);

	m_pBtreeFile->fetch(&m_cKey);

	if (m_cTuple.getCount() == 0) {
		m_cTuple.reserve(3);
		m_cTuple.pushBack(new Common::ObjectIDData);	// OID
		m_cTuple.pushBack(new Common::UnsignedIntegerData); // ROWID
		m_cTuple.pushBack(new Common::DataArrayData);
	} else {
		; _SYDNEY_ASSERT(m_cTuple.getCount() == 3);
		m_cTuple.getElement(0)->setNull();
	}
	if (m_pBtreeFile->get(&m_cTuple) == false)
		// 検索でヒットしなかった -> 削除されている
		return false;

	// 0: ObjectID(ObjectIDData)
	// 1: ROWID(UnsignedIntegerData)
	// 2: セクション区切り(UnsignedIntegerDataを要素に持つDataArrayData)

	const Common::DataArrayData& s
		= _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&,
							   *(m_cTuple.getElement(2).get()));
	Common::UnsignedIntegerArrayData& d
		= _SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerArrayData&,
							   cSectionOffset_);

	d.clear();
	d.reserve(s.getCount());
	for (int i = 0; i < s.getCount(); ++i)
	{
		const Common::UnsignedIntegerData& p
			= _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData&,
								   *(s.getElement(i).get()));
		d.pushBack(p.getValue());
	}

	return true;
}

//
//	FUNCTION private
//	FullText::SectionFile::makeKey -- キーを作成する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiTupleID_
//		タプルID
//
//	RETURN
//	なり
//
//	EXCEPTIONS
//
void
SectionFile::makeKey(ModUInt32 uiTupleID_)
{
	// 0: ROWID(UnsignedIntegerData)
	if (m_cKey.getCount() == 0) {
		m_cKey.reserve(1);
		m_cKey.pushBack(new Common::UnsignedIntegerData);
	}
	_SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData&,
						 *(m_cKey.getElement(0).get())).setValue(uiTupleID_);
}

//
//	FUNCTION public
//	FullText::SectionFile::getNewPath -- 新しいパス名を作成する
//
//	NOTES
//
//	ARGUMENTS
//	const Os::Path& cParent_
//		新しい親パス
//
//	RETURN
//	Os::Path
//		新しいパス名
//
//	EXCEPTIONS
//
Os::Path
SectionFile::getNewPath(const Os::Path& cParent_)
{
	Os::Path cNewPath = cParent_;
	cNewPath.addPart(_Directory);
	return cNewPath;
}

//
//	FUNCTION public
//	FullText::SectionFile::setNewPath -- 新しいパス名をFileIDに設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Os::Path& cParent_
//		新しい親パス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SectionFile::setNewPath(const Os::Path& cParent_)
{
	setPath(getNewPath(cParent_));
}

//
//	FUNCTION private
//	FullText::SectionFile::convertSectionData
//		-- セクション情報をB木に挿入できる形にコンバートする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cSectionOffset_
//		元のデータ
//	Common::Data::Pointer pNewSectionOffset_
//		変換後のデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SectionFile::convertSectionData(const Common::Data& cSectionOffset_,
								Common::Data::Pointer pNewSectionOffset_)
{
	//【注意】
	//	FullTextの中では、セクションデータは UnsignedIntegerArrayData で
	//	扱われている。しかし、古いものはセクションデータの格納にB木を利用
	//	しており、B木は UnsignedIntegerArrayData は受け付けないので、
	//	UnsignedIntegerDataを要素にもつDataArrayDataにコンバートする

	; _TRMEISTER_ASSERT(cSectionOffset_.getType()
						== Common::DataType::Array &&
						cSectionOffset_.getElementType()
						== Common::DataType::UnsignedInteger);
	; _TRMEISTER_ASSERT(pNewSectionOffset_->getType()
						== Common::DataType::Array &&
						pNewSectionOffset_->getElementType()
						== Common::DataType::Data);

	const Common::UnsignedIntegerArrayData& pSrc =
		_SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerArrayData&,
							 cSectionOffset_);
	Common::DataArrayData& pDst =
		_SYDNEY_DYNAMIC_CAST(Common::DataArrayData&,
							 *(pNewSectionOffset_.get()));

	pDst.clear();
	pDst.reserve(pSrc.getCount());

	for (int i = 0; i < pSrc.getCount(); ++i)
	{
		pDst.pushBack(new Common::UnsignedIntegerData(pSrc.getElement(i)));
	}
}

//
//	FUNCTION private
//	FullText::SectionFile::setPath -- パス名を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Os::Path& cPath_
//		パス名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SectionFile::setPath(const Os::Path& cPath_)
{
	m_cFileID.getSection().setString(
		_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Area::Key),
		cPath_);
	m_cPath = cPath_;
}

//
//	Copyright (c) 2003, 2005, 2006, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
