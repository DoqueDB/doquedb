// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OtherInformationFile.cpp -- 転置以外のその他情報を格納するファイル
// 
// Copyright (c) 2010, 2011, 2012, 2017, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "FullText2/OtherInformationFile.h"
#include "FullText2/Parameter.h"
#include "FullText2/MessageAll_Class.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "Common/BinaryData.h"
#include "Common/DataArrayData.h"
#include "Common/DoubleData.h"
#include "Common/ObjectIDData.h"
#include "Common/IntegerData.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/UnsignedIntegerArrayData.h"

#include "PhysicalFile/DirectArea.h"

#include "Exception/BadArgument.h"
#include "Exception/VerifyAborted.h"

#include "Schema/File.h"

#include "Os/Memory.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	//
	//	下位ファイルのパス
	//
	ModUnicodeString _VectorPath("Vector");
	ModUnicodeString _VariablePath("Variable");

	//
	//	ファイル分割時の閾値(ファイルサイズ)
	//
	ParameterInteger64 _cMaxFileSize("FullText2_MaxFileSize",
									 ModInt64(3) << 10 << 10 << 10,	// 3G
									 false);
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::OtherInformationFile
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::FileID& cFileID_
//		ファイルID
//	const Os::Path& cPath_
//		パス
//	bool bBatch_
//		バッチモードかどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OtherInformationFile::OtherInformationFile(FullText2::FileID& cFileID_,
										   const Os::Path& cPath_,
										   bool bBatch_)
	: MultiFile(Os::Path()), m_cFileID(cFileID_), m_cParentPath(cPath_),
	  m_pVectorFile(0), m_pVariableFile(0), m_pHeader(0), m_bDelay(false),
	  m_pTransaction(0), m_eFixMode(Buffer::Page::FixMode::Unknown)
{
	// バッチインサートの場合も遅延更新となる
	m_bDelay = (bBatch_ || cFileID_.isDelayed());

	// m_pFieldPosition[]の初期化はattach内で行う
	attach(bBatch_);
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::~OtherInformationFile
//		-- デストラクタ
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
OtherInformationFile::~OtherInformationFile()
{
	detach();
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::getCount
//		-- ファイルに挿入しているタプル数を得る
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	ModUInt32
//		挿入されているタプル数
//
//	EXCEPTIONS
//
ModUInt32
OtherInformationFile::getCount()
{
	ModUInt32 count = 0;
	if (isMounted(*m_pTransaction))
	{
		// KWICのため、FullTextFile::getPropertyが実行される
		// それは1件も挿入されていなくても実行されてしまう
		
		count = m_pVectorFile->getCount();
	}
	return count;
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::create -- ファイルを作成する
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
OtherInformationFile::create()
{
	// ファイルを作成する
	MultiFile::create();

	try
	{
		// ヘッダーを初期化する
		initializeHeader();

		// 変更を確定する
		flushAllPages();
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		try
		{
			// 削除する
			destroy();
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(getLockName(), false);
		}
		_SYDNEY_RETHROW;
	} 
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::verify -- 整合性検査を行う
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	Admin::Verification::Treatment::Value uiTreatment_
//		動作
//	Admin::Verification::Progress& cProgress_
//		経過
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile::verify(const Trans::Transaction& cTransaction_,
							 Admin::Verification::Treatment::Value uiTreatment_,
							 Admin::Verification::Progress& cProgress_)
{
	// 下位ファイルに整合性検査の開始を通知する
	startVerification(cTransaction_, uiTreatment_, cProgress_);
		
	try
	{
		// すべてのデータが読み出せるか確認する
		
		// 読み出すためのデータを作成する
		Common::DataArrayData cTuple;
		ModVector<int> vecGetFields;
		ModVector<Common::DataType::Type>::Iterator i
			= m_vecVectorElements.begin();
		int n = 0;
		for (; i != m_vecVectorElements.end(); ++i, ++n)
		{
			switch (*i)
			{
			case Common::DataType::Double:
				cTuple.pushBack(new Common::DoubleData);
				break;
			case Common::DataType::Integer:
				cTuple.pushBack(new Common::IntegerData);
				break;
			case Common::DataType::UnsignedInteger:
				cTuple.pushBack(new Common::UnsignedIntegerData);
				break;
			case Common::DataType::ObjectID:
				cTuple.pushBack(new Common::ObjectIDData);
				break;
			default:
				;
			}
			vecGetFields.pushBack(n);
		}

		// ベクターファイルに格納されている最大文書ID
		ModUInt32 uiMaxDocumentID = m_pVectorFile->getMaxKey();

		Common::BinaryData cBinaryData;
		Common::UnsignedIntegerArrayData cUnsignedIntegerArrayData;

		ModSize count = 0;

		for (ModUInt32 id = 0; id <= uiMaxDocumentID; ++id)
		{
			// ベクターファイルを読み込む
			m_pVectorFile->get(id, cTuple, vecGetFields);

			if (cTuple.getElement(0)->isNull())
				// 先頭の文書長は必ず格納されている
				// これがnullってことは何も格納されていないってこと
				continue;

			// 登録件数
			++count;

			i = m_vecVectorElements.begin();
			int n = 0;
			int v = 0;
			for (; i < m_vecVectorElements.end(); ++i, ++n)
			{
				const Common::Data& cData = *(cTuple.getElement(n).get());
				if (*i == Common::DataType::ObjectID)
				{
					if (cData.isNull() == false)
					{
						// 可変長を読む

						// ObjectID
						const Common::ObjectIDData& cObjectID =
							_SYDNEY_DYNAMIC_CAST(
								const Common::ObjectIDData&, cData);
						PhysicalFile::DirectArea::ID oid;
						oid.m_uiPageID = cObjectID.getFormerValue();
						oid.m_uiAreaID = cObjectID.getLatterValue();
					
						// Value
						Common::Data* p = 0;
						switch (m_vecVariableElements[v])
						{
						case ValueType::FeatureList:
							p = &cBinaryData;
							break;
						case ValueType::SectionInfo:
							p = &cUnsignedIntegerArrayData;
							break;
						}

						// 読んでみるだけ
						m_pVariableFile->get(oid, *p);
					}

					++v;
				}
			}
		}

		// 登録件数を確認する
		if (count != getCount())
		{
			_SYDNEY_VERIFY_INCONSISTENT(
				cProgress_, m_cFileID.getPath(),
				Message::IllegalEntryCount(getCount(), count));
		}
	}
	catch (Exception::VerifyAborted&)
	{
		;	// 何もしない
	}
	catch (...)
	{
		recoverAllPages();
		endVerification();
		_SYDNEY_RETHROW;
	}

	flushAllPages();
	endVerification();
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::move -- ファイルを移動する
//
//	NOTES
//	移動元と移動先のパスが異なっていることが前提。
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Os::Path& cNewPath_
//		移動先のエリア
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile::move(const Trans::Transaction& cTransaction_,
						   const Os::Path& cNewPath_)
{
	// ここではディレクトリは作らないので、そのエラー処理もいらない
	
	Os::Path path = cNewPath_;
	path.addPart(_VectorPath);
	m_pVectorFile->move(cTransaction_, path);
	if (m_pVariableFile)
	{
		try
		{
			path = cNewPath_;
			path.addPart(_VariablePath);
			m_pVariableFile->move(cTransaction_, path);
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			try
			{
				path = m_cParentPath;
				path.addPart(_VectorPath);
				m_pVectorFile->move(cTransaction_, path);
			}
			catch (...)
			{
				SydErrorMessage << "Recovery failed." << ModEndl;
				Schema::File::setAvailability(getLockName(), false);
			}
			_SYDNEY_RETHROW;
		}
	}
	m_cParentPath = cNewPath_;
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::open -- オープンする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	Buffer::Page::FixMode::Value eFixMode_
//		FIXモード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile::open(const Trans::Transaction& cTransaction_,
						   Buffer::Page::FixMode::Value eFixMode_)
{
	m_pTransaction = &cTransaction_;
	m_eFixMode = eFixMode_;

	MultiFile::open(cTransaction_, eFixMode_);
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::close -- クローズする
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
OtherInformationFile::close()
{
	MultiFile::close();
	
	m_pTransaction = 0;
	m_eFixMode = Buffer::Page::FixMode::Unknown;
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::recoverAllPages -- ページの更新を破棄する
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
OtherInformationFile::recoverAllPages()
{
	m_pHeader = 0;
	MultiFile::recoverAllPages();
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::flushAllPages -- ページの更新を反映する
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
OtherInformationFile::flushAllPages()
{
	m_pHeader = 0;
	MultiFile::flushAllPages();
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		文書ID
//	const ModVector<ModSize>& vecSize_
//		正規化後の文書長
//	const ModVector<ModSize>& vecOriginalSize_
//		正規化前の文書長
//	ModInt32 iUnitNumber_
//		挿入したユニット番号
//	double dblScoreData_
//		スコア調整カラムのスコア値
//	const FeatureList& cFeatureList_
//		特徴語データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile::insert(ModUInt32 uiDocumentID_,
							 const ModVector<ModSize>& vecSize_,
							 const ModVector<ModSize>& vecOriginalSize_,
							 ModInt32 iUnitNumber_,
							 double dblScoreData_,
							 const FeatureList& cFeatureList_)
{
	try
	{
		// 正規化後の文書長を求める
		ModUInt32 uiDocumentLength = 0;
		ModVector<ModSize>::ConstIterator i = vecSize_.begin();
		for (; i != vecSize_.end(); ++i)
			uiDocumentLength += *i;

		// 正規化前の文書長を求める
		ModUInt32 uiOriginalDocumentLength = 0;
		i = vecOriginalSize_.begin();
		for (; i != vecOriginalSize_.end(); ++i)
			uiOriginalDocumentLength += *i;

		// ヘッダーを読み込む
		readHeader();

		// 文書長を更新する
		m_pHeader->m_ulTotalDocumentLength += uiDocumentLength;
		// ユニットの登録件数を更新する
		if (iUnitNumber_ != -1)
			m_pHeader->m_pUnitCount[iUnitNumber_] += 1;

		if (m_bDelay)
		{
			// 遅延更新の時にはエグゼキュータ側の小転置の
			// 最小文書IDと最大文書IDを更新する

			ModUInt32& max = (m_pHeader->m_iIndex == 0) ?
				m_pHeader->m_uiIns0MaxID : m_pHeader->m_uiIns1MaxID;
			ModUInt32& min = (m_pHeader->m_iIndex == 0) ?
				m_pHeader->m_uiIns0MinID : m_pHeader->m_uiIns1MinID;

			if (max < uiDocumentID_)
				max = uiDocumentID_;
			if (min == 0)
				min = uiDocumentID_;
		}
		else
		{
			// 遅延更新ではないときは
			// 大転置の最大文書IDのみを更新する

			if (m_pHeader->m_uiFullMaxID < uiDocumentID_)
				m_pHeader->m_uiFullMaxID = uiDocumentID_;
		}

		// ヘッダーをdirtyにする
		m_pVectorFile->dirtyHeaderPage();

		// 登録するデータを作成する
		Common::DataArrayData cTuple;
		cTuple.reserve(m_vecVectorElements.getSize());

		//【注意】
		//	この並びは attach() と同じ
		
		// 正規化後の文書長

		{
			cTuple.pushBack(
				new Common::UnsignedIntegerData(uiDocumentLength));
		}

		// 正規化前の文書長
		if (m_cFileID.isRoughKwic())
		{
			cTuple.pushBack(
				new Common::UnsignedIntegerData(uiOriginalDocumentLength));
		}

		// ユニット番号
		if (m_cFileID.isDistribute())
		{
			cTuple.pushBack(
				new Common::IntegerData(iUnitNumber_));
		}
		
		// セクションサイズ
		if (m_cFileID.isSectionized())
		{
			// 登録するためのデータを作成する
			Common::UnsignedIntegerArrayData cData;
			cData.reserve(static_cast<int>(vecSize_.getSize()));
			for (i = vecSize_.begin(); i != vecSize_.end(); ++i)
				cData.pushBack(*i);

			// 可変長ファイルに登録する
			PhysicalFile::DirectArea::ID oid
				= m_pVariableFile->insert(cData);

			// ObjectIDDataにする
			cTuple.pushBack(
				new Common::ObjectIDData(oid.m_uiPageID, oid.m_uiAreaID));
		}

		// スコア調整カラム
		if (m_cFileID.isScoreField())
		{
			cTuple.pushBack(
				new Common::DoubleData(dblScoreData_));
		}

		// 特徴語リスト
		if (m_cFileID.isClustering())
		{
			Common::BinaryData cData;
	
			// エリアの最大サイズを得る
			ModSize maxSize = m_pVariableFile->getMaxStorableAreaSize()
				- sizeof(ModUInt32);	// BinaryDataの格納時にサイズを書くので
			// ダンプするサイズを求める
			ModSize size = FeatureSet::getSize(cFeatureList_, maxSize);
			// サイズ分の領域を確保する
			cData.assign(0, size);
			// ダンプする
			FeatureSet::dump(syd_reinterpret_cast<char*>(cData.getValue()),
							 cFeatureList_, maxSize);
			// 値を正規化する
			syd_reinterpret_cast<FeatureSet*>(cData.getValue())->normalize();

			// 可変長ファイルに登録する
			PhysicalFile::DirectArea::ID oid
				= m_pVariableFile->insert(cData);

			// ObjectIDDataにする
			cTuple.pushBack(
				new Common::ObjectIDData(oid.m_uiPageID, oid.m_uiAreaID));
		}

		// ベクターに挿入する
		m_pVectorFile->insert(uiDocumentID_, cTuple);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::expunge -- 削除する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		ROWID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile::expunge(ModUInt32 uiDocumentID_)
{
	try
	{
		// ヘッダーを読み込む
		readHeader();

		// 削除する文書の文書長を得る
		ModUInt32 documentLength = 0;
		getDocumentLength(uiDocumentID_, documentLength);
		// 削除する文書のユニット番号を得る
		ModInt32 iUnitNumber = -1;
		getUnitNumber(uiDocumentID_, iUnitNumber);
		
		// 文書長を更新する
		m_pHeader->m_ulTotalDocumentLength -= documentLength;
		// ユニットの登録件数を更新する
		if (iUnitNumber != -1)
			m_pHeader->m_pUnitCount[iUnitNumber] -= 1;
		
		// ヘッダーをdirtyにする
		m_pVectorFile->dirtyHeaderPage();
		
		int n = 0;
		ModVector<Common::DataType::Type>::Iterator i
			= m_vecVectorElements.begin();
		for (; i != m_vecVectorElements.end(); ++i, ++n)
		{
			if ((*i) == Common::DataType::ObjectID)
			{
				// ベクターから値を取得
				Common::ObjectIDData c;
				m_pVectorFile->get(uiDocumentID_, n, c);

				if (c.isNull())
					// null なので、次へ
					// 現状、null が格納されていることはないけど...
					continue;

				// 可変長ファイルから削除する
				PhysicalFile::DirectArea::ID oid;
				oid.m_uiPageID = c.getFormerValue();
				oid.m_uiAreaID = c.getLatterValue();
				m_pVariableFile->expunge(oid);
			}
		}
				
		// ベクターを削除する
		m_pVectorFile->expunge(uiDocumentID_);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::updateScoreData
//		-- スコア調整カラムのデータを更新する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		文書ID
//	double dblScoreData_
//		スコア調整カラムのデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile::updateScoreData(ModUInt32 uiDocumentID_,
									  double dblScoreData_)
{
	if (m_pFieldPosition[ValueType::ScoreData] == -1)
		// 格納されていない
		return;
	
	// ヘッダーを読み込む
	readHeader();

	Common::DoubleData cData(dblScoreData_);
	
	m_pVectorFile->update(uiDocumentID_,
						  m_pFieldPosition[ValueType::ScoreData],
						  cData);
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::updateFeatureList
//		-- 特徴語リストのデータを更新する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		文書ID
//	const FullText2::FeatureList& cFeatureList_
//		特徴語リスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile::updateFeatureList(ModUInt32 uiDocumentID_,
										const FeatureList& cFeatureList_)
{
	if (m_pFieldPosition[ValueType::FeatureList] == -1)
		// 格納されていない
		return;
	
	// ヘッダーを読み込む
	readHeader();

	// ベクターから値を取得
	Common::ObjectIDData c;
	m_pVectorFile->get(uiDocumentID_,
					   m_pFieldPosition[ValueType::FeatureList], c);
	if (c.isNull() == false)
	{
		// nullじゃないので、可変長ファイルから削除する
		PhysicalFile::DirectArea::ID oid;
		oid.m_uiPageID = c.getFormerValue();
		oid.m_uiAreaID = c.getLatterValue();
		m_pVariableFile->expunge(oid);
	}

	Common::BinaryData cData;

	// エリアの最大サイズを得る
	ModSize maxSize = m_pVariableFile->getMaxStorableAreaSize()
		- sizeof(ModUInt32);	// BinaryDataの格納時にサイズを書くので
	// ダンプするサイズを求める
	ModSize size = FeatureSet::getSize(cFeatureList_, maxSize);
	// サイズ分の領域を確保する
	cData.assign(0, size);
	// ダンプする
	FeatureSet::dump(syd_reinterpret_cast<char*>(cData.getValue()),
					 cFeatureList_, maxSize);
	// 値を正規化する
	syd_reinterpret_cast<FeatureSet*>(cData.getValue())->normalize();
	// 可変長ファイルに登録する
	PhysicalFile::DirectArea::ID oid = m_pVariableFile->insert(cData);

	// ObjectIDDataにする
	c.setValue(oid.m_uiPageID, oid.m_uiAreaID);

	// ベクターファイルの更新する
	m_pVectorFile->update(uiDocumentID_,
						  m_pFieldPosition[ValueType::FeatureList],
						  c);
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::getDocumentLength
//		-- 文書長を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//	 	文書ID
//	ModUInt32& uiDocumentLength_
//		文書長
//
//	RETURN
//	bool
//		値が存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OtherInformationFile::getDocumentLength(ModUInt32 uiDocumentID_,
										ModUInt32& uiDocumentLength_)
{
	// ヘッダーを読み込む
	readHeader();

	// 文書長のフィールド番号は 0 を想定
	m_pVectorFile->get(uiDocumentID_, 0, uiDocumentLength_);
	
	return true;
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::getOriginalLength
//		-- 正規化前の文書長を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//	 	文書ID
//	ModUInt32& uiOriginalLength_
//		正規化前の文書長
//
//	RETURN
//	bool
//		値が存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OtherInformationFile::getOriginalLength(ModUInt32 uiDocumentID_,
										ModUInt32& uiOriginalLength_)
{
	if (m_pFieldPosition[ValueType::OriginalLength] == -1)
		return false;
	
	// ヘッダーを読み込む
	readHeader();

	Common::UnsignedIntegerData cValue;
	m_pVectorFile->get(uiDocumentID_,
					   m_pFieldPosition[ValueType::OriginalLength],
					   cValue);
	if (cValue.isNull())
		return false;

	uiOriginalLength_ = cValue.getValue();

	return true;
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::getUnitNumber
//		-- 挿入したユニット番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//	 	文書ID
//	ModInt32& iUnitNumber_
//		挿入したユニット番号
//
//	RETURN
//	bool
//		値が存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OtherInformationFile::getUnitNumber(ModUInt32 uiDocumentID_,
									ModInt32& iUnitNumber_)
{
	if (m_pFieldPosition[ValueType::UnitNumber] == -1)
		return false;
	
	// ヘッダーを読み込む
	readHeader();

	Common::IntegerData cValue;
	m_pVectorFile->get(uiDocumentID_,
					   m_pFieldPosition[ValueType::UnitNumber],
					   cValue);
	if (cValue.isNull())
		return false;

	iUnitNumber_ = cValue.getValue();

	return true;
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::getScoreData
//		-- スコア調整値を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//	 	文書ID
//	double& dblScoreData_
//	   	スコア調整値
//
//	RETURN
//	bool
//		値が存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OtherInformationFile::getScoreData(ModUInt32 uiDocumentID_,
								   double& dblScoreData_)
{
	if (m_pFieldPosition[ValueType::ScoreData] == -1)
		return false;
	
	// ヘッダーを読み込む
	readHeader();

	Common::DoubleData cValue;
	m_pVectorFile->get(uiDocumentID_,
					   m_pFieldPosition[ValueType::ScoreData],
					   cValue);
	if (cValue.isNull())
		return false;

	dblScoreData_ = cValue.getValue();

	return true;
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::getSectionSize
//		-- セクションサイズ情報を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//	 	文書ID
//	ModVector<ModSize>& vecSectionSize_
//		セクションサイズ情報
//
//	RETURN
//	bool
//		値が存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OtherInformationFile::getSectionSize(ModUInt32 uiDocumentID_,
									 ModVector<ModSize>& vecSectionSize_)
{
	// 中身を消去する
	vecSectionSize_.erase(vecSectionSize_.begin(), vecSectionSize_.end());
	
	if (m_pFieldPosition[ValueType::SectionInfo] == -1)
		return false;
	
	// ヘッダーを読み込む
	readHeader();

	// 可変長フィールドなので、まずObjectIDを取得する
	Common::ObjectIDData cOID;
	m_pVectorFile->get(uiDocumentID_,
					   m_pFieldPosition[ValueType::SectionInfo],
					   cOID);
	if (cOID.isNull())
		return false;

	// 可変長ファイルから読み出す
	Common::UnsignedIntegerArrayData cValue;
	PhysicalFile::DirectArea::ID oid;
	oid.m_uiPageID = cOID.getFormerValue();
	oid.m_uiAreaID = cOID.getLatterValue();
	m_pVariableFile->get(oid, cValue);

	// すべての要素を取り出し、引数にコピーする
	const ModVector<unsigned int>& v = cValue.getValue();
	vecSectionSize_.reserve(v.getSize());
	ModVector<unsigned int>::ConstIterator i = v.begin();
	for (; i != v.end(); ++i)
		vecSectionSize_.pushBack(*i);

	return true;
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::getFeatureSet
//		-- 特徴語セットを得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//	 	文書ID
//	FeaureSetPointer& pFeatureSet_
//		特徴語セットへのポインター
//
//	RETURN
//	bool
//		値が存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OtherInformationFile::getFeatureSet(ModUInt32 uiDocumentID_,
									FeatureSetPointer& pFeatureSet_)
{
	// 中身を消去する
	pFeatureSet_.free();

	if (m_pFieldPosition[ValueType::FeatureList] == -1)
		return false;
	
	// ヘッダーを読み込む
	readHeader();

	// 可変長フィールドなので、まずObjectIDを取得する
	Common::ObjectIDData cOID;
	m_pVectorFile->get(uiDocumentID_,
					   m_pFieldPosition[ValueType::FeatureList],
					   cOID);
	if (cOID.isNull())
		return false;
	
	// 可変長ファイルからエリアを得る
	PhysicalFile::DirectArea::ID oid;
	oid.m_uiPageID = cOID.getFormerValue();
	oid.m_uiAreaID = cOID.getLatterValue();
	PhysicalFile::DirectArea area = m_pVariableFile->attachArea(oid);

	if (!area.isValid())
		return false;

	// エリアから直接ポインターを取り出す
	const FeatureSet* dst =
		syd_reinterpret_cast<const FeatureSet*>(
			syd_reinterpret_cast<const char*>(
				const_cast<const PhysicalFile::DirectArea&>(area).
				operator const void*()) + sizeof(ModUInt32));
	// コピーする
	pFeatureSet_.copy(dst);
	// エリアをdetachする
	area.detach();
	
	return true;
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::getLastDocumentID -- 最終文書IDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
// 	RETURN
//	ModUInt32
//		最終文書ID
//
//	EXCEPTIONS
//
ModUInt32
OtherInformationFile::getLastDocumentID()
{
	// ヘッダーを読み込む
	readHeader();

	// ベクターの最大キーが最終文書IDである
	return m_pVectorFile->getMaxKey();
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::getTotalDocumentLength
//		-- 総文書長を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		総文書長
//
//	EXCEPTIONS
//
ModUInt64
OtherInformationFile::getTotalDocumentLength()
{
	// ヘッダーを読み込む
	readHeader();

	return m_pHeader->m_ulTotalDocumentLength;
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::flip -- 小転置を入れ替える
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
OtherInformationFile::flip()
{
	// ヘッダーを読む
	readHeader();

	if (m_pHeader->m_iProceeding)
		// マージ中ならなにもしない
		return;

	// マージ中にして、小転置を入れ替える
	m_pHeader->m_iProceeding = 1;
	m_pHeader->m_iIndex = (m_pHeader->m_iIndex == 0) ? 1 : 0;

	// ヘッダーを更新したので、dirty にする
	m_pVectorFile->dirtyHeaderPage();
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::isProceeding -- マージ中かどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		マージ中だった場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OtherInformationFile::isProceeding()
{
	// ヘッダーを読む
	readHeader();

	return (m_pHeader->m_iProceeding != 0) ? true : false;
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::mergeCancel -- マージ中断に設定する
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
OtherInformationFile::mergeCancel()
{
	// ヘッダーを読む
	readHeader();

	m_pHeader->m_iProceeding = 2;

	// ヘッダーを更新したので、dirty にする
	m_pVectorFile->dirtyHeaderPage();
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::isCanceled -- マージを中断したかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		キャンセルした場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OtherInformationFile::isCanceled()
{
	// ヘッダーを読む
	readHeader();

	bool result = false;
	if (m_pHeader->m_iProceeding == 2)
	{
		// マージが中断された
		result = true;
		m_pHeader->m_iProceeding = 1;	// マージ中にする
		
		// ヘッダーを更新したので、dirty にする
		m_pVectorFile->dirtyHeaderPage();
	}
	
	return result;
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::mergeDone -- マージの終了を通知する
//
//	NOTES
//
//	ARGUMENTS
//
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile::mergeDone()
{
	// ヘッダーを読む
	readHeader();

	if (m_pHeader->m_iProceeding == 0)
		// マージ中じゃないなら何もしない
		return;

	// マージ対象の小転置の最大文書IDを得る
	ModUInt32 max = (m_pHeader->m_iIndex == 0) ?
		m_pHeader->m_uiIns1MaxID : m_pHeader->m_uiIns0MaxID;

	// 最大文書IDを更新する
	if (m_pHeader->m_uiFullMaxID < max)
		m_pHeader->m_uiFullMaxID = max;

	// 小転置の文書IDの範囲のユニット番号を更新する
	ModUInt32 from = (m_pHeader->m_iIndex == 0) ?
		m_pHeader->m_uiIns1MinID : m_pHeader->m_uiIns0MinID;
	updateUnitNumber(from, max + 1, m_pHeader->m_iInsertUnit);

	// 最大最少文書IDを初期化する
	if (m_pHeader->m_iIndex == 0)
	{
		m_pHeader->m_uiIns1MinID = 0;
		m_pHeader->m_uiIns1MaxID = 0;
	}
	else
	{
		m_pHeader->m_uiIns0MinID = 0;
		m_pHeader->m_uiIns0MaxID = 0;
	}
	
	// マージは終わったので、フラグを戻す
	m_pHeader->m_iProceeding = 0;

	// ヘッダーを更新したので、dirty にする
	m_pVectorFile->dirtyHeaderPage();
}

//
//	FUNCTION
//	FullText2::OtherInformationFile::getIndex
//		-- エグゼキュータ側の小転置の番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		エグゼキュータ側の小転置の番号(0 または 1)
//
//	EXCEPTIONS
//
int
OtherInformationFile::getIndex()
{
	// ヘッダーを読む
	readHeader();

	return m_pHeader->m_iIndex;
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::getFirstDocumentID
//		-- エグゼキュータ側小転置の最少文書IDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//
//	EXCEPTIONS
//
DocumentID
OtherInformationFile::getFirstDocumentID()
{
	// ヘッダーを読む
	readHeader();

	ModUInt32 id = (m_pHeader->m_iIndex == 0) ?
		m_pHeader->m_uiIns0MinID : m_pHeader->m_uiIns1MinID;

	return (id == 0) ? UndefinedDocumentID : id;
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::getMergeFirstDocumentID
//		-- マージデーモン側小転置の最少文書IDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//
//	EXCEPTIONS
//
DocumentID
OtherInformationFile::getMergeFirstDocumentID()
{
	// ヘッダーを読む
	readHeader();

	ModUInt32 id = (m_pHeader->m_iIndex == 0) ?
		m_pHeader->m_uiIns1MinID : m_pHeader->m_uiIns0MinID;

	return (id == 0) ? UndefinedDocumentID : id;
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::getFullMaxID
//		-- 大転置の最大文書IDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::DocumentID
//		大転置の最大文書ID
//
//	EXCEPTIONS
//
DocumentID
OtherInformationFile::getFullMaxID()
{
	// ヘッダーを読む
	readHeader();
	return m_pHeader->m_uiFullMaxID;
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::getIns0MaxID
//		-- 小転置の最大文書IDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::DocumentID
//		小転置の最大文書ID
//
//	EXCEPTIONS
//
DocumentID
OtherInformationFile::getIns0MaxID()
{
	// ヘッダーを読む
	readHeader();
	return m_pHeader->m_uiIns0MaxID;
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::getIns1MaxID
//		-- 小転置の最大文書IDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::DocumentID
//		小転置の最大文書ID
//
//	EXCEPTIONS
//
DocumentID
OtherInformationFile::getIns1MaxID()
{
	// ヘッダーを読む
	readHeader();
	return m_pHeader->m_uiIns1MaxID;
}

//
//	FUCTION public
//	FullText2::OtherInformationFile::getInsertUnit
//		-- 挿入に利用するユニット番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInt32
//		ユニット数
//
//	EXCEPTIONS
//
ModInt32
OtherInformationFile::getInsertUnit()
{
	readHeader();
	return m_pHeader->m_iInsertUnit;
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::setInsertUnit
//		-- 挿入に利用するユニット番号を設定する
//
//	NOTES
//
//	ARGUMENTS
//	ModInt32 iUnitNumber_
//		設定するユニット番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile::setInsertUnit(ModInt32 iUnitNumber_)
{
	readHeader();
	m_pHeader->m_iInsertUnit = iUnitNumber_;
	m_pVectorFile->dirtyHeaderPage();
}

//
//	FUCTION public
//	FullText2::OtherInformationFile::getMaxFileSize
//		-- 1ユニットあたりの最大ファイルサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		最大ファイルサイズ(バイト単位)
//
//	EXCEPTIONS
//
ModUInt64
OtherInformationFile::getMaxFileSize()
{
	readHeader();
	return m_pHeader->m_ulMaxFileSize;
}

//
//	FUCTION public
//	FullText2::OtherInformationFile::updateMaxFileSize
//		-- 1ユニットあたりの最大ファイルサイズを更新する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		更新した最大ファイルサイズ(バイト単位)
//
//	EXCEPTIONS
//
ModUInt64
OtherInformationFile::updateMaxFileSize()
{
	readHeader();

	// 最大ファイルサイズを超えたら、最大ファイルサイズ分加える
	
	m_pHeader->m_ulMaxFileSize += _cMaxFileSize.get();
	m_pVectorFile->dirtyHeaderPage();

	return m_pHeader->m_ulMaxFileSize;
}

//
//	FUNCTION public
//	FullText2::OtherInformationFile::copy -- インスタンスをコピーする
//
//	NOTES
//	読み出し専用の検索時に利用されるのを想定しており、
//	複数のスレッドで同時に更新した場合の動作は不定となる
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::OtherInformationFile*
//		コピーしたファイル
//
//	EXCEPTIONS
//
OtherInformationFile*
OtherInformationFile::copy()
{
	// コピーというメソッドではあるが、実際はコピーではなく、
	// 初期化された新しいインスタンスを返す
	
	// 検索時にしか呼び出されないので、常にバッチモードは false でいい
	OtherInformationFile* pFile = new OtherInformationFile(m_cFileID,
														   m_cParentPath,
														   false);
	if (m_pTransaction)
	{
		pFile->open(*m_pTransaction, m_eFixMode);
	}
	return pFile;
}

//
//	FUNCTION private
//	FullText2::OtherInformationFile::attach -- 物理ファイルをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	bool bBatch_
//		バッチモードか否か
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile::attach(bool bBatch_)
{
	int n = 0;
	Os::Memory::set(m_pFieldPosition, 0xff, sizeof(int) * ValueType::ValueNum);
	
	// ベクターファイルの型を調べる

	//【注意】
	//	この並びは insert() と同じ
	//	m_vecVariableElements は verify の時にしか利用されない
		
	// 正規化後の文書長
	//
	//【注意】
	//	getDocumentLength では、正規化後の文書長のフィールド番号が
	//	0 であることを想定しているので、変更してはならない
	//
	m_vecVectorElements.pushBack(Common::DataType::UnsignedInteger);
	m_pFieldPosition[ValueType::DocumentLength] = n;
	++n;
	
	// 正規化前の文書長
	if (m_cFileID.isRoughKwic())
	{
		m_vecVectorElements.pushBack(Common::DataType::UnsignedInteger);
		m_pFieldPosition[ValueType::OriginalLength] = n;
		++n;
	}
	
	// ユニット番号
	if (m_cFileID.isDistribute())
	{
		m_vecVectorElements.pushBack(Common::DataType::Integer);
		m_pFieldPosition[ValueType::UnitNumber] = n;
		++n;
	}

	// セクションサイズ
	if (m_cFileID.isSectionized())
	{
		m_vecVectorElements.pushBack(Common::DataType::ObjectID);
		m_vecVariableElements.pushBack(ValueType::SectionInfo);
		m_pFieldPosition[ValueType::SectionInfo] = n;
		++n;
	}

	// スコア調整カラム
	if (m_cFileID.isScoreField())
	{
		m_vecVectorElements.pushBack(Common::DataType::Double);
		m_pFieldPosition[ValueType::ScoreData] = n;
		++n;
	}

	// 特徴語リスト
	if (m_cFileID.isClustering())
	{
		m_vecVectorElements.pushBack(Common::DataType::ObjectID);
		m_vecVariableElements.pushBack(ValueType::FeatureList);
		m_pFieldPosition[ValueType::FeatureList] = n;
		++n;
	}
	
	Os::Path path = m_cParentPath;
	path.addPart(_VectorPath);
	m_pVectorFile = new MultiVectorFile(m_cFileID, path,
										m_vecVectorElements, bBatch_);
	MultiFile::pushBackSubFile(m_pVectorFile);
	
	if (m_vecVariableElements.getSize())
	{
		Os::Path path = m_cParentPath;
		path.addPart(_VariablePath);
		m_pVariableFile = new VariableFile(m_cFileID, path, bBatch_);
		MultiFile::pushBackSubFile(m_pVariableFile);
	}
}

//
//	FUNCTION private
//	FullText2::OtherInformationFile::detach -- 物理ファイルをデタッチする
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
OtherInformationFile::detach()
{
	delete m_pVectorFile, m_pVectorFile = 0;
	if (m_pVariableFile)
	{
		delete m_pVariableFile, m_pVariableFile = 0;
	}
	m_vecVectorElements.clear();
	m_pHeader = 0;
}

//
//	FUNCTION private
//	FullText2::OtherInformationFile::readHeader -- ヘッダーを読む
//
//	NOTES
//
//	ARUGENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile::readHeader()
{
	if (m_pHeader == 0)
		m_pHeader = syd_reinterpret_cast<Header*>(
			m_pVectorFile->getSubClassHeader());
}

//
//	FUNCTION private
//	FullText2::OtherInformationFile::initializeHeader -- ヘッダーを初期化する
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
OtherInformationFile::initializeHeader()
{
	readHeader();

	// ヘッダー領域全体を 0 で初期化
	Os::Memory::reset(m_pHeader, m_pVectorFile->getSubClassHeaderSize());

	// 初期値が 0 でないものを設定する
	m_pHeader->m_ulMaxFileSize = _cMaxFileSize.get();

	m_pVectorFile->dirtyHeaderPage();
}

//
//	FUNCTION private
//	FullText2::OtherInformationFile::updateUnitNumber
//		-- マージしたデータのユニット番号を更新する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiFromKey_
//		先頭のキー値
//	ModUInt43 uiEndKey_
//		終端のキー値
//	ModInt32 iUnitNumber_
//		ユニット番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile::updateUnitNumber(ModUInt32 uiFromKey_,
									   ModUInt32 uiEndKey_,
									   ModInt32 iUnitNumber_)
{
	if (m_pFieldPosition[ValueType::UnitNumber] == -1)
		// 格納されていない
		return;
	
	// ヘッダーを読み込む
	readHeader();

	// 更新するためのデータを作成する
	Common::IntegerData cData(iUnitNumber_);
	
	ModUInt32 id = uiFromKey_;
	for (; id < uiEndKey_; ++id)
	{
		// 文書長を取り出し、存在していたら更新する

		ModUInt32 documentLength = 0;
		if (getDocumentLength(id, documentLength) == true)
		{
			// 存在しているので更新する

			m_pVectorFile->update(id,
								  m_pFieldPosition[ValueType::UnitNumber],
								  cData);
		}
	}
}

//
//	FUNCTION private
//	FullText2::OtherInformationFile::startVerification -- 整合性検査を開始する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	Admin::Verification::Treatment::Value uiTreatment_
//		動作
//	Admin::Verification::Progress& cProgress_
//		経過
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile::startVerification(
	const Trans::Transaction& cTransaction_,
	Admin::Verification::Treatment::Value uiTreatment_,
	Admin::Verification::Progress& cProgress_)
{
	int n = 0;
	try
	{
		m_pVectorFile->startVerification(cTransaction_,
										 uiTreatment_,
										 cProgress_);
		++n;
		if (m_pVariableFile)
		{
			m_pVariableFile->startVerification(cTransaction_,
											   uiTreatment_,
											   cProgress_);
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		switch (n) {
		case 1: m_pVectorFile->endVerification();
		default:
			;
		}

		_TRMEISTER_RETHROW;
	}
}

//
//	FUNCTION private
//	FullText2::OtherInformationFile::endVerification -- 整合性検査を終了する
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
OtherInformationFile::endVerification()
{
	m_pVectorFile->endVerification();
	if (m_pVariableFile) m_pVariableFile->endVerification();
}

//
//	Copyright (c) 2010, 2011, 2012, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
