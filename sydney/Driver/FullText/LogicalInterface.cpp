// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogicalInteraface.cpp --
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2023 Ricoh Company, Ltd.
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
#include "SyInclude.h"
#include "SyDynamicCast.h"
#include "FullText/LogicalInterface.h"
#include "FullText/DelayIndexFile.h"
#include "FullText/SimpleIndexFile.h"
#include "FullText/OpenOption.h"
#include "FullText/OtherInformationFile0.h"
#ifndef SYD_CPU_SPARC
#include "FullText/OtherInformationFile1.h"
#endif
#include "FullText/OtherInformationFile2.h"
#include "FullText/Parameter.h"
#include "FullText/MessageAll_Class.h"

#include "Inverted/FieldType.h"

#include "Admin/Verification.h"

#include "Common/Assert.h"
#include "Common/BinaryData.h"
#include "Common/DoubleData.h"
#include "Common/DataArrayData.h"
#include "Common/IntegerArrayData.h"
#include "Common/IntegerData.h"
#include "Common/LanguageData.h"
#include "Common/Message.h"
#include "Common/StringData.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/UnsignedIntegerArrayData.h"

#include "Exception/FileNotOpen.h"
#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"
#include "Exception/VerifyAborted.h"
#include "Exception/InvalidSectionData.h"

#include "LogicalFile/TreeNodeInterface.h"

#include "FileCommon/OpenOption.h"
#include "FileCommon/DataManager.h"
#include "FileCommon/NodeWrapper.h"

#include "Schema/File.h"

#include "Os/Limits.h"

#include "ModUnicodeString.h"
#include "ModLanguageSet.h"


_SYDNEY_USING
_SYDNEY_FULLTEXT_USING

namespace
{
	//
	//	CLASS
	//	_$$::_AutoAttachFile
	//
	class _AutoAttachFile
	{
	public:
		_AutoAttachFile(LogicalInterface& cFile_, bool bEstimate = false)
			: m_cFile(cFile_), m_bOwner(false)
			{
				if (m_cFile.isAttached() == false)
				{
					m_cFile.attachFile();
					m_bOwner = true;
				}
			}
		~_AutoAttachFile()
			{
				if (m_bOwner) m_cFile.detachFile();
			}

	private:
		LogicalInterface& m_cFile;
		bool m_bOwner;
	};

	//
	//	VARIABLE local
	//
	const ModUnicodeString _cRoughKwicSize("RoughKwicSize");
	const ModUnicodeString _cSearchTermList("SearchTermList");
	const ModUnicodeString _cUnaParameterKey("UnaParameterKey");
	const ModUnicodeString _cUnaParameterValue("UnaParameterValue");

	//	VARIABLE local
	//	_$$::_KwicMarginScaleFactor -- 
	//	_$$::_KwicMarginScaleFactorForNormalizing -- 
	//
	//	NOTE
	//	マージンを含めたKWIC長(荒いKWIC長)を、
	//	ユーザが指定したKWIC長の何倍にするか？
	//
	ParameterInteger _KwicMarginScaleFactor(
		"FullText_KwicMarginScaleFactor", 2);
	ParameterInteger _KwicMarginScaleFactorForNormalizing(
		"FullText_KwicMarginScaleFactorForNormalizing", 10);

	//	VARIABLE local
	//	_$$::_GetLocationUpperLimitPerTerm
	//
	//	NOTE
	//	1単語あたり何個の位置を取得するか？
	//	ただし、Section検索時は無効(無制限)。
	//
	ParameterInteger _GetLocationUpperLimitPerTerm(
		"FullText_GetLocationUpperLimitPerTerm", 100);

	//	VARIABLE local
	//	_$$::_GetLocationUpperTermLimit
	//
	//	NOTE
	//	何単語分の位置リストを取得するか？
	//	ただし、Section検索時は無効(無制限)。
	//
	ParameterInteger _GetLocationUpperTermLimit(
		"FullText_GetLocationUpperLimitPerTerm", 20);

	//
	//	FUNCTION local
	//	_$$::_convertProjection
	//		-- TreeNodeInterfaceからIntegerArrayDataへ変換する
	//
	void _convertProjection(const LogicalFile::TreeNodeInterface* pNode_,
							const FullText::FileID& cFileID_,
							Common::IntegerArrayData& cProjection_)
	{
		using namespace LogicalFile;
	
		// プロジェクションの指定はTreeNodeInterfaceで行う

		// シフトする数
		int shift = 0;
		if (cFileID_.isLanguage())
			shift += 1;
		if (cFileID_.isScoreField())
			shift += 1;

		FileCommon::ListNodeWrapper node(pNode_);

		// オペランドのサイズ
		int operandSize = node.getSize();

		for (int i = 0; i < operandSize; ++i)
		{
			int n = -1;
			const TreeNodeInterface* pOperand = node.get(i);

			switch (pOperand->getType())
			{
			case LogicalFile::TreeNodeInterface::Field:
				n = FileCommon::DataManager::toInt(pOperand);
				break;
			case LogicalFile::TreeNodeInterface::Score:
				n = Inverted::FieldType::Score + shift;
				break;
			case LogicalFile::TreeNodeInterface::Tf:
				n = Inverted::FieldType::Tf + shift;
				break;
			case LogicalFile::TreeNodeInterface::ClusterID:
				n = Inverted::FieldType::Cluster + shift;
				break;
			case LogicalFile::TreeNodeInterface::RoughKwicPosition:
				n = Inverted::FieldType::RoughKwicPosition + shift;
				break;
			case LogicalFile::TreeNodeInterface::Section:
				n = Inverted::FieldType::Section + shift;
				break;
			case LogicalFile::TreeNodeInterface::FeatureValue:
				n = Inverted::FieldType::FeatureValue + shift;
				break;
			case LogicalFile::TreeNodeInterface::Word:
				n = Inverted::FieldType::Word + shift;
				break;
			case LogicalFile::TreeNodeInterface::Avg:
				{
					if (pOperand->getOperandSize() != 1)
						break;
					const TreeNodeInterface* p = pOperand->getOperandAt(0);
					switch (p->getType())
					{
					case LogicalFile::TreeNodeInterface::FullTextLength:
						n = Inverted::FieldType::AverageLength + shift;
						break;
					case LogicalFile::TreeNodeInterface::CharLength:
						n = Inverted::FieldType::AverageCharLength + shift;
						break;
					case LogicalFile::TreeNodeInterface::WordCount:
						n = Inverted::FieldType::AverageWordCount + shift;
						break;
					}
				}
				break;
			case LogicalFile::TreeNodeInterface::Count:
				n = Inverted::FieldType::Count + shift;
				break;
			}

			if (n == -1)
				_SYDNEY_THROW0(Exception::BadArgument);

			cProjection_.setElement(i, n);
		}
	}
}

//
//	DEFINE
//	_CHECK_OPEN -- ファイルがオープンされているかどうか
//
#define _CHECK_OPEN() \
{ \
	if (isOpened() == false) \
	{ \
		_SYDNEY_THROW0(Exception::FileNotOpen); \
	} \
}

//
//	FUNCTION public
//	FullText::LogicalInterface::LogicalInterface -- コンストラクタ
//
//	NOTES
//
//	ARGUMETNS
//	const LogicalFile::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
LogicalInterface::LogicalInterface(const LogicalFile::FileID& cFileID_)
	: m_cFileID(cFileID_), m_pTransaction(0),
	  m_pIndexFile(0), m_pOtherFile(0),
	  m_pSearchCapsule(0),
	  m_pGetLocationCapsule(0),
	  m_pSearchResult(0),
	  m_eSortParameter(Inverted::SortParameter::None),
	  m_iGetCount(0), m_iLimit(Os::Limits<int>::getMax()), m_iOffset(1),
	  m_pSearchByBitSet(0),
	  m_uiKwicSize(0),
	  m_iGetUnnormalizedCharLengthField(0)
{
	// [YET] 初期化子で行わないのはなぜ？
	m_cFieldMask = 0;
}

//
//	FUNCTION public
//	FullText::LogicalInterface::~LogicalInterface -- デストラクタ
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
LogicalInterface::~LogicalInterface()
{
	delete m_pSearchResult;
}


//
//	FUNCTION public
//	FullText::LogicalInterface::isAccessible
//		-- 実体であるOSファイルが存在するか調べる
//
//	NOTES
//
//	ARGUMENTS
//	bool force
//		強制モードかどうか(default false)
//
//	RETURN
//	bool
//		存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::isAccessible(bool force) const
{
	_AutoAttachFile cAuto(*const_cast<LogicalInterface*>(this));
	return m_pIndexFile->isAccessible(force);
}

//
//	FUNCTION public
//	FullText::LogicalInterface::isMounted -- マウントされているかどうか
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& trans_
//		トランザクション
//
//	RETURN
//	bool
//		マウントされている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::isMounted(const Trans::Transaction& trans) const
{
	_AutoAttachFile cAuto(*const_cast<LogicalInterface*>(this));
	return m_pIndexFile->isMounted(trans);
}

//
//	FUNCTION public
//	FullText::LogicalInterface::getFileID -- ファイルIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
const LogicalFile::FileID&
LogicalInterface::getFileID() const
{
	return m_cFileID;
}

//
//	FUNCTION public
//	FullText::LogicalInterface::getSize -- ファイルサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		ファイルサイズ(byte)
//
//	EXCEPTIONS
//
ModUInt64
LogicalInterface::getSize(const Trans::Transaction& cTrans_)
{
	_AutoAttachFile cAuto(*this);

	ModUInt64 size = 0;
	if (isMounted(cTrans_))
	{
		size = m_pIndexFile->getSize(cTrans_);
		size += m_pOtherFile->getSize(cTrans_);
	}
	return size;
}

//
//	FUNCTION public
//	FullText::LogicalInterface::getCount -- 挿入されているオブジェクト数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInt64
//		オブジェクトの総数
//
//	EXCEPTIONS
//
ModInt64
LogicalInterface::getCount() const
{
	; _CHECK_OPEN();
	ModInt64 count = 0;
	if (isMounted(*m_pTransaction))
	{
		if (m_eOpenMode == LogicalFile::OpenOption::OpenMode::Search)
		{
			// 条件が与えられているので、件数見積もりを行う
			Inverted::SearchCapsule& capsule
				= const_cast<IndexFile*>(m_pIndexFile)->getSearchCapsule(
					const_cast<LogicalInterface*>(this));
			count = static_cast<ModInt64>(capsule.getEstimateCount());

			// SearchCapsuleをクリアする
			const_cast<IndexFile*>(m_pIndexFile)->clearSearchCapsule();
		}
		else
		{
			// 登録件数を返す
			count = m_pIndexFile->getCount();
		}
	}
	return count;
}

//
//	FUNCTION public
//	FullText::LogicalInterface::getOverhead -- オープン時のオーバヘッドを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	double
//		オーバヘッド(秒)
//
//	EXCEPTIONS
//
double
LogicalInterface::getOverhead() const
{
	; _CHECK_OPEN();
	return m_pIndexFile->getFullInvert()->getOverhead();
}

//
//	FUNCTION public
//	FullText::LogicalInterface::getProcessCost
//		-- ひとつのタプルを挿入or取得する時のプロセスコストを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	double
//		プロセスコスト(秒)
//
//	EXCEPTIONS
//
double
LogicalInterface::getProcessCost() const
{
	; _CHECK_OPEN();
	return m_pIndexFile->getFullInvert()->getProcessCost();
}

//
//	FUNCTION public
//	FullText::LogicalInterface::getSearchParameter -- オープンパラメータを得る
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		条件
//	LogicalFile::OpenOption& cOpenOption_
//		条件を設定するオープンパラメータ
//
//	RETURN
//	bool
//		実行できる検索の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::getSearchParameter(
	const LogicalFile::TreeNodeInterface* pCondition_,
	LogicalFile::OpenOption& cOpenOption_) const
{
	if (pCondition_)
	{
		if (m_pIndexFile->getSearchParameter(m_cFileID, pCondition_, cOpenOption_)
			== false)
			return false;

		// オープンモード
		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
									FileCommon::OpenOption::OpenMode::Key),
								FileCommon::OpenOption::OpenMode::Search);
	}
	else
	{
		// スキャンは平均文書長と文書数の取得の時にのみ許される

		// オープンモード
		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
									FileCommon::OpenOption::OpenMode::Key),
								FileCommon::OpenOption::OpenMode::Read);
	}

	// 全文はすべての検索結果をキャッシュしてしまう
	cOpenOption_.setBoolean(_SYDNEY_OPEN_PARAMETER_KEY(
								FileCommon::OpenOption::CacheAllObject::Key), true);

	return true;
}

//
//	FUNCTION public
//	FullText::LogicalInterface::getProjectionParameter
//		-- プロジェクションパラメータを得る
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pNode_
//		プロジェクションするフィールド
//	LogicalFile::OpenOption& cOpenOption_
//		プロジェクションパラメータを設定するオープンオプション
//
//	RETURN
//	bool
//		取得できないフィールドが含まれていた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::getProjectionParameter(
	const LogicalFile::TreeNodeInterface* pNode_,
	LogicalFile::OpenOption& cOpenOption_) const
{
	// TreeNodeInterfaceのプロジェクションをIntegerArrayDataに変換する
	Common::IntegerArrayData cProjection;
	_convertProjection(pNode_, m_cFileID, cProjection);
	
	if (m_cFileID.getProjectionParameter(cProjection, cOpenOption_) == false)
		return false;

	// オープンモード
	int tmp;
	if (cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(
									FileCommon::OpenOption::OpenMode::Key), tmp) == false)
	{
		// まだオープンモードが設定されていないので設定する
		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
									FileCommon::OpenOption::OpenMode::Key),
								FileCommon::OpenOption::OpenMode::Read);
	}

	// 全文はすべての検索結果をキャッシュしてしまう
	cOpenOption_.setBoolean(_SYDNEY_OPEN_PARAMETER_KEY(
								FileCommon::OpenOption::CacheAllObject::Key), true);

	return true;
}

//
//	FUNCTION public
//	FullText::LogicalInterface::getUpdateParameter -- 更新パラメータを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Common::IntegerArrayData& cUpdateField_
//		更新するフィールド
//	LogicalFile::OpenOption& cOpenOption_
//		更新パラメータを設定するオープンオプション
//
//	RETURN
//	bool
//		更新できないフィールドが含まれていた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::getUpdateParameter(
	const Common::IntegerArrayData& cUpdateField_,
	LogicalFile::OpenOption& cOpenOption_) const
{
	if (m_cFileID.getUpdateParameter(cUpdateField_, cOpenOption_) == false)
		return false;

	// オープンモード
	cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
								FileCommon::OpenOption::OpenMode::Key),
							FileCommon::OpenOption::OpenMode::Update);

	return true;
}

//
//	FUNCTION public
//	FullText::LogicalInterface::getSortParameter -- ソート順パラメータを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pNode_
//		ソートキー
//	LogicalFile::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	bool
//		ソートできる場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::getSortParameter(const LogicalFile::TreeNodeInterface* pNode_,
								   LogicalFile::OpenOption& cOpenOption_) const
{
	//	pNode_ は以下のような構造
	//
	//	TreeNodeInterface::OrderBy - operand --> TreeNodeInterface::SortKey
	//
	//	TreeNodeInterface::SortKey - operand --> TreeNodeInterface::Score
	//								 option --> TreeNodeInterface::SortDirection
	//												0: ASC
	//												1: DESC
	//
	//	TreeNodeInterface::Score - operand --> Field 0
	//                                     --> Field 1
	//

	if (pNode_->getType() != LogicalFile::TreeNodeInterface::OrderBy ||
		pNode_->getOperandSize() != 1)
		// ソートキーは１つしか指定できない
		return false;
	
	const LogicalFile::TreeNodeInterface* p = pNode_->getOperandAt(0);
	if (p->getType() != LogicalFile::TreeNodeInterface::SortKey)
		return false;

	// 昇順 or 降順
	int order = 0;	// デフォルトは昇順
	if (p->getOptionSize() != 0 && p->getOptionSize() != 1)
		return false;
	if (p->getOptionSize() == 1)
		order = FileCommon::DataManager::toInt(p->getOptionAt(0));

	if (p->getOperandSize() != 1)
		return false;

	// キー
	p = p->getOperandAt(0);

	switch (p->getType())
	{
	case LogicalFile::TreeNodeInterface::Field:
		{
			// ROWID
			int rowid = Inverted::FieldType::Rowid;
			if (m_cFileID.isLanguage())
				rowid += 1;
			if (m_cFileID.isScoreField())
				rowid += 1;

			if (FileCommon::DataManager::toInt(p) != rowid)
				return false;

			cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
										OpenOption::SortParameter::Key),
									(order == 0) ?
									Inverted::SortParameter::RowIdAsc
									: Inverted::SortParameter::RowIdDesc);
		}
		break;
		
	case LogicalFile::TreeNodeInterface::Score:
	case LogicalFile::TreeNodeInterface::WordDf:
	case LogicalFile::TreeNodeInterface::WordScale:
		{
			if (p->getOperandSize() != 1)
				return false;
			const LogicalFile::TreeNodeInterface* tmp = p->getOperandAt(0);
			if (tmp->getType() != LogicalFile::TreeNodeInterface::Field)
				return false;
			if (FileCommon::DataManager::toInt(tmp) != 0)
				return false;

			if (p->getType() == LogicalFile::TreeNodeInterface::Score)
			{
				// スコア
				cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
											OpenOption::SortParameter::Key),
										(order == 0) ?
										Inverted::SortParameter::ScoreAsc
										: Inverted::SortParameter::ScoreDesc);
			}
			else if (p->getType() == LogicalFile::TreeNodeInterface::WordDf)
			{
				// ワードDF
				cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
											OpenOption::SortParameter::Key),
										(order == 0) ?
										Inverted::SortParameter::DfAsc
										: Inverted::SortParameter::DfDesc);
			}
			else
			{
				// ワードScale
				cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
											OpenOption::SortParameter::Key),
										(order == 0) ?
										Inverted::SortParameter::ScaleAsc
										: Inverted::SortParameter::ScaleDesc);
			}
		}
		break;
		
	default:
		return false;
	}

	return true;
}

//
//	FUNCTION public
//	FullText::LogicalInterface::getLimitParameter -- 取得数と取得位置を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::IntegerArrayData& cSpec_
//		0番目はLIMIT、1番目はOFFSET(1ベース)
//	LogicalFile::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	bool
//		実行できる場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::getLimitParameter(const Common::IntegerArrayData& cSpec_,
									LogicalFile::OpenOption& cOpenOption_) const
{
	int limit;
	int offset = 1;

	if (cSpec_.getCount() == 0 || cSpec_.getCount() > 2)
		return false;

	limit = cSpec_.getElement(0);
	if (cSpec_.getCount() == 2)
		offset = cSpec_.getElement(1);

	if (limit < 0 || offset < 1)
		return false;

	cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
								OpenOption::Limit::Key), limit);
	cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
								OpenOption::Offset::Key), offset);
	return true;
}

//
//	FUNCTION public
//	FullText::LogicalInterface::create -- ファイルを作成する
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

const LogicalFile::FileID&
LogicalInterface::create(const Trans::Transaction& cTransaction_)
{
	m_cFileID.create();

	_AutoAttachFile cAuto(*this);

	m_pIndexFile->create(cTransaction_);
	try
	{
		m_pOtherFile->create(cTransaction_);
	}
	catch (...)
	{
		m_pIndexFile->destroy(cTransaction_);
		_SYDNEY_RETHROW;
	}

	return m_cFileID;
}

//
//	FUNCTION public
//	FullText::LogicalInterface::destroy -- ファイルを破棄する
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
LogicalInterface::destroy(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);

	m_pIndexFile->destroy(cTransaction_);
	m_pOtherFile->destroy(cTransaction_);
}

//
//	FUNCTION public
//	FullText::LogicalInterface::mount -- マウントする
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
const LogicalFile::FileID&
LogicalInterface::mount(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);

	m_pIndexFile->mount(cTransaction_);
	try
	{
		m_pOtherFile->mount(cTransaction_);
	}
	catch (...)
	{
		try
		{
			m_pIndexFile->unmount(cTransaction_);
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(m_cFileID.getLockName(), false);
		}
		_SYDNEY_RETHROW;
	}
	m_cFileID.setMounted(true);
	return m_cFileID;
}

//
//	FUNCTION public
//	FullText::LogicalInterface::unmount -- アンマウントする
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
const LogicalFile::FileID&
LogicalInterface::unmount(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);

	m_pIndexFile->unmount(cTransaction_);
	try
	{
		m_pOtherFile->unmount(cTransaction_);
	}
	catch (...)
	{
		try
		{
			m_pIndexFile->mount(cTransaction_);
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(m_cFileID.getLockName(), false);
		}
		_SYDNEY_RETHROW;
	}

	m_cFileID.setMounted(false);

	return m_cFileID;
}

//
//	FUNCTION public
//	FullText::LogicalInterface::flush -- フラッシュする
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
LogicalInterface::flush(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);

	m_pIndexFile->flush(cTransaction_);
	m_pOtherFile->flush(cTransaction_);
}

//
//	FUNCTION public
//	FullText::LogicalInterface::startBackup -- バックアップを開始する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const bool bRestorable_
//		リストア可能かどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::startBackup(const Trans::Transaction& cTransaction_,
							  const bool bRestorable_)
{
	_AutoAttachFile cAuto(*this);

	m_pIndexFile->startBackup(cTransaction_, bRestorable_);
	try
	{
		m_pOtherFile->startBackup(cTransaction_, bRestorable_);
	}
	catch (...)
	{
		m_pIndexFile->endBackup(cTransaction_);
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	FullText::LogicalInterface::endBackup -- バックアップを終了する
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
LogicalInterface::endBackup(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);

	m_pIndexFile->endBackup(cTransaction_);
	m_pOtherFile->endBackup(cTransaction_);
}

//
//	FUNCTION public
//	FullText::LogicalInterface::recover -- 障害から回復する
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
LogicalInterface::recover(const Trans::Transaction& cTransaction_,
						  const Trans::TimeStamp& cPoint_)
{
	_AutoAttachFile cAuto(*this);

	m_pIndexFile->recover(cTransaction_, cPoint_);
	m_pOtherFile->recover(cTransaction_, cPoint_);
}

//
//	FUNCTION public
//	FullText::LogicalInterface::restore
//		-- ある時点に開始された読取専用トランザクションが参照する版を
//		 最新版とする
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
LogicalInterface::restore(const Trans::Transaction& cTransaction_,
						  const Trans::TimeStamp& cPoint_)
{
	_AutoAttachFile cAuto(*this);

	m_pIndexFile->restore(cTransaction_, cPoint_);
	m_pOtherFile->restore(cTransaction_, cPoint_);
}

//
//	FUNCTION public
//	FullText::LogicalInterface::verify -- 整合性検査を行う
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& trans_
//		トランザクション
//	const Admin::Verification::Treatment::Value treatment_
//		アクション
//	Admin::Verification::Progress& progress_
//		経過
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::verify(const Trans::Transaction& trans_,
						 const Admin::Verification::Treatment::Value treatment_,
						 Admin::Verification::Progress& progress_)
{
	if (isMounted(trans_) == false)
		return;

	_AutoAttachFile cAuto(*this);

	try
	{
		{
			Admin::Verification::Progress progress(progress_.getConnection());
			m_pIndexFile->verify(trans_, treatment_, progress);
			progress_ += progress;
			if (progress.isGood() == false
				&& !(treatment_ & Admin::Verification::Treatment::Continue))
				_SYDNEY_THROW0(Exception::VerifyAborted);
		}
		if (m_pOtherFile->isMounted(trans_) == true)
		{
			Admin::Verification::Progress progress(progress_.getConnection());
			m_pOtherFile->verify(trans_, treatment_, progress);
			progress_ += progress;
			if (progress.isGood() == false
				&& !(treatment_ & Admin::Verification::Treatment::Continue))
				_SYDNEY_THROW0(Exception::VerifyAborted);

			// 転置ファイルのエントリ数をセクション情報ファイルのエントリ数を
			// チェックする
			LogicalFile::OpenOption cOpenOption;

			cOpenOption.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
								   FileCommon::OpenOption::OpenMode::Key),
								   LogicalFile::OpenOption::OpenMode::Read);
			cOpenOption.setBoolean(_SYDNEY_OPEN_PARAMETER_KEY(
								   FileCommon::OpenOption::Estimate::Key), true);

			m_pIndexFile->open(trans_, cOpenOption);
			m_pOtherFile->open(trans_, cOpenOption);

			try
			{
				if (m_pIndexFile->getCount() != m_pOtherFile->getCount())
				{
					Admin::Verification::Progress progress(progress_.getConnection());
					_SYDNEY_VERIFY_INCONSISTENT(progress,
											m_cFileID.getPath(),
											Message::NotEqualEntryCount());
					progress_ += progress;
				}
			}
			catch (...)
			{
				m_pIndexFile->close();
				m_pOtherFile->close();
				_SYDNEY_RETHROW;
			}
			m_pIndexFile->close();
			m_pOtherFile->close();
		}
	}
	catch (Exception::VerifyAborted&)
	{
		// 無視
	}
	catch (...)
	{
		Admin::Verification::Progress progress(progress_.getConnection());
		_SYDNEY_VERIFY_INCONSISTENT(progress,
									m_cFileID.getPath(),
									Message::VerifyAbort());
		progress_ += progress;
	}
}

//
//	FUNCTION public
//	FullText::LogicalInterface::open -- オープンする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const LogicalFile::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::open(const Trans::Transaction& cTransaction_,
					   const LogicalFile::OpenOption& cOpenOption_)
{
	m_pTransaction = &cTransaction_;
	m_iGetCount = 0;

	// オープンモード
	int iValue = cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(
											 FileCommon::OpenOption::OpenMode::Key));
	m_eOpenMode = static_cast<LogicalFile::OpenOption::OpenMode::Value>(iValue);

	// Estimateか
	bool bEstimate = cOpenOption_.getBoolean(_SYDNEY_OPEN_PARAMETER_KEY(
												 FileCommon::OpenOption::Estimate::Key));

	// ファイルをattachする
	attachFile();

	try
	{
		m_pIndexFile->open(cTransaction_, cOpenOption_);
		m_pOtherFile->open(cTransaction_, cOpenOption_);
		if (bEstimate == true)
			return;

		if (m_eOpenMode == LogicalFile::OpenOption::OpenMode::Search)
		{
			// 検索モード
			// ソートパラメータ
			int param;
			if (cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(
											OpenOption::SortParameter::Key), param) == true)
			{
				m_eSortParameter
					= static_cast<Inverted::SortParameter::Value>(param);
			}

			// 結果取得上限
			if (cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(
											OpenOption::Limit::Key), param) == true)
				m_iLimit = param;
			if (cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(
											OpenOption::Offset::Key), param) == true)
				m_iOffset = param;

			// KWIC長
			m_uiKwicSize = getKwicSize(cOpenOption_);

			// 絞り込んだ集合でランキング検索する用のビットセットが
			// 設定されているか？設定されている場合はサポート外
			const Common::Object* p = 0;
			if (cOpenOption_.getObjectPointer(
					_SYDNEY_OPEN_PARAMETER_KEY(
						FileCommon::OpenOption::RankByBitSet::Key), p) == true)
			{
				_SYDNEY_THROW0(Exception::NotSupported);
			}
			
			// 取得フィールドを設定する
			setGetField(cOpenOption_);
		}
		else if (m_eOpenMode == LogicalFile::OpenOption::OpenMode::Read)
		{
			// スキャンモード

			// 取得フィールドを設定する
			setGetField(cOpenOption_);
		}
		else
		{
			// 更新フィールドを設定する
			setPutField(cOpenOption_);
		}
	}
	catch (...)
	{
		detachFile();
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	FullText::LogicalInerface::close -- クローズする
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
LogicalInterface::close()
{
	m_pTransaction = 0;
	m_vecPutField.clear();
	m_pIndexFile->close();
	m_pOtherFile->close();
	if(m_pSearchResult)
		m_pSearchResult->clear();
	reset();
	// ファイルをdetachする
	detachFile();
}

//
//	FUNCTION public
//	FullText::LogicalInterface::get -- データを取得する
//
//	NOTES
//
//	ARGUMENTS
//	Commn::DataArrayData* pTuple_
//		検索結果
//
//	RETURN
//	bool
//		結果が得られた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::get(Common::DataArrayData* pTuple_)
{
	// 初回取得前に検索結果をm_pSearchResultに設定しておくことで、
	// m_pGetTupleの中では、初回の取得かどうかを意識しないで処理できる。

	if (m_iGetCount == 0)
	{
		//
		// 初めての取得
		//

		//
		// SearchCapsuleの設定
		//

		if ( m_pGetTuple != &LogicalInterface::getLength)
		{
			// [NOTE] 長さ等は、条件付きでは取得できないので、
			//  SearchCapsuleを設定することはない。
			//  また、セクション検索とは同時に実行されない。
			//  参照 FieldMask::checkGroupExclussivness()
			
			// 平均文書長と登録文書数はマウントされていなくてもnullや0を返す
			// それ以外は、マウントされていないと何も返せない
			if (isMounted(*m_pTransaction) == false)
				return false;

			// [NOTE] getProperty()ですでに取得済みかもしれないが、
			//  SearchCapsuleはIndexFileで管理されているので、再取得は問題ない。
			m_pSearchCapsule = &m_pIndexFile->getSearchCapsule(this);

			if (m_cFieldMask &
				(1 << _SYDNEY::Inverted::FieldType::Section |
				 1 << _SYDNEY::Inverted::FieldType::RoughKwicPosition))
			{
				// カプセルに知らせる

				// [NOTE] SectionSearchFlag==trueで、
				//  検索時にTea構文がキャッシュされるので、
				//  RoughKwicPositionを取得する時もフラグを立てておく。
				m_pSearchCapsule->setSectionSearchFlag(true);
				m_pGetLocationCapsule = &m_pIndexFile
					->getGetLocationCapsule(m_pSearchCapsule);
				if (!(m_cFieldMask &
					  1 << _SYDNEY::Inverted::FieldType::Section))
				{
					// [NOTE] 位置の取得制限はSection検索時は不可
					m_pGetLocationCapsule->setUpperLimitPerTerm(
						_GetLocationUpperLimitPerTerm.get());
					m_pGetLocationCapsule->setUpperTermLimit(
						_GetLocationUpperTermLimit.get());
				}
				
				if (m_pSearchByBitSet)
				{
					// ビットセットからの絞込みの場合でも、
					// searchCapsuleを実行する必要がある場合がある
					m_bitsetIter = m_pSearchByBitSet->begin();
				}
			}
		}

		//
		// limitの設定
		//

		// [YET] ビットセットの絞り込み条件があった場合に、
		//  それ以前の時点でデータを絞り込みすぎていないか？
	
		ModSize limit;
		if (m_iLimit == Os::Limits<int>::getMax() ||
			m_pSearchByBitSet)
			limit = 0;
		else
			limit = m_iOffset + m_iLimit - 1;

		//
		// 検索
		//
		
		// 単語抽出と検索をexecute()で実行
		// SQL文でwordlistが指定されると、m_sizeWordlistには、
		// wordlist中の単語数が戻る
		if (m_pGetTuple == &LogicalInterface::getWordTuple)
		{
			//
			// Word単位で取得する場合
			//
			
			m_pSearchCapsule->execute(limit,m_eSortParameter,&m_cWordSet);
			m_wordIter = m_cWordSet.begin();
		}
		else if (m_pSearchCapsule)
		{
			//
			// 通常の検索の場合（Word単位の取得と長さ等の取得を除く）
			//
			
			// 最大limit個の検索結果を取得
			// [NOTE] クラスタリング検索時はlimit個のクラスタに含まれる
			//  検索結果を取得するので、検索結果数がlimitを超えることがある。
			// [YET] クラスタリング結果だけModInvertedSearchResultを使ってない。
			m_cCluster.clear();
			delete m_pSearchResult;
			m_pSearchResult = ModInvertedSearchResult::factory(m_cResultType);
			m_pSearchCapsule->execute(limit,
									  m_eSortParameter,
									  m_sizeWordlist,
									  m_pSearchResult,
									  m_cCluster);
			
			// オフセット処理
			m_iGetPos = getGetPos();
		}
		else
		{
			//
			// 長さの取得の場合
			//
			; _TRMEISTER_ASSERT(m_pGetTuple == &LogicalInterface::getLength);
			
			// ここでは何もしない。
		}
	}

	//
	// 取得
	//
	return (this->*m_pGetTuple)(*pTuple_);
}

//
//	FUNCTION public
//	FullText::LogicalInterface::insert -- データを挿入する
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData* pTuple_
//		挿入するデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::insert(Common::DataArrayData* pTuple_)
{
	if (m_eOpenMode == LogicalFile::OpenOption::OpenMode::Search)
	{
		// オープンモードがSearchなのにinsertに来るってことは拡張検索

		// スコアを取得しなくてもランキング検索しなければならない
		if (m_pSearchCapsule == 0)
		{
			m_pSearchCapsule
				= &m_pIndexFile->getSearchCapsule(this);
		}

		// 拡張文書を登録する
		insertExpandDocument(pTuple_);
	}
	else
	{
		// 転置に挿入できる形に整える
		ModUInt32 uiTupleID;
		ModUnicodeString cstrDocument;
		ModVector<ModLanguageSet> vecLanguage;
		ModVector<ModSize> vecSectionOffset;
		convert(pTuple_, uiTupleID,
				cstrDocument, vecLanguage, vecSectionOffset);

		// 挿入する
		ModVector<ModSize> vecOrgOffset = vecSectionOffset;
		ModInvertedFeatureList vecFeature;
		// vecLanguageを渡しているが、言語情報は挿入しない
		m_pIndexFile->insert(cstrDocument,
							 vecLanguage,
							 uiTupleID,
							 vecSectionOffset,
							 vecFeature);
		try
		{
			// その他情報を挿入できる形に整える
			Common::DataArrayData cTuple;
			// 言語情報は挿入しない
			makeInsertOtherInformationTuple(
				vecSectionOffset, pTuple_, vecFeature, cstrDocument,
				cTuple);

			// 挿入する
			m_pOtherFile->insert(uiTupleID, cTuple);
		}
		catch (...)
		{
			try
			{
				m_pIndexFile->expunge(cstrDocument,
									  vecLanguage,
									  uiTupleID,
									  vecOrgOffset);
			}
			catch (...)
			{
				SydErrorMessage << "Recovery failed." << ModEndl;
				Schema::File::setAvailability(
					m_cFileID.getLockName(), false);
			}
			_SYDNEY_RETHROW;
		}
	}
}

//
//	FUNCTION public
//	FullText::LogicalInterface::update -- 更新する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData* pKey_
//		キー
//	Common::DataArrayData* pTuple_
//		更新するデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::update(const Common::DataArrayData* pKey_,
						 Common::DataArrayData* pTuple_)
{
	// 引数チェック
	if (pTuple_->getCount() != static_cast<int>(m_vecPutField.getSize()))
		_SYDNEY_THROW0(Exception::BadArgument);

	bool isScoreOnly = false;
	if (m_vecPutField.getSize() == 1 && m_vecPutField[0] == PutValue::SCORE)
		isScoreOnly = true;

	ModUInt32 uiTupleID;

	ModUnicodeString cstrKeyDocument;
	ModVector<ModLanguageSet> vecKeyLanguage;
	ModVector<ModSize> vecKeySectionOffset;

	ModUnicodeString cstrValueDocument;
	ModVector<ModLanguageSet> vecValueLanguage;
	ModVector<ModSize> vecValueSectionOffset;

	ModVector<ModSize> vecOrgSectionOffset;
	ModInvertedFeatureList vecValueFeature;

	if (isScoreOnly)
	{
		// スコア調整フィールドのみの更新
		int n = (m_cFileID.isLanguage()) ? 3 : 2;
		// ROWIDのみ取り出す
		convertRowidData(pKey_->getElement(n).get(), uiTupleID);
	}
	else
	{
		// キーにはすべてのフィールドが含まれている
		convert(pKey_, uiTupleID, cstrKeyDocument,
				vecKeyLanguage, vecKeySectionOffset);

		// 置き換えるフィールド
		cstrValueDocument = cstrKeyDocument;
		vecValueLanguage = vecKeyLanguage;
		vecValueSectionOffset = vecKeySectionOffset;

		ModVector<PutValue::Type>::Iterator i = m_vecPutField.begin();
		for (int n = 0; i != m_vecPutField.end(); ++i, ++n)
		{
			if ((*i) == PutValue::DOCUMENT)
			{
				convertDocumentData(pTuple_->getElement(n).get(),
									cstrValueDocument,
									vecValueSectionOffset);
			}
			else if ((*i) == PutValue::LANGUAGE)
			{
				convertLanguageData(pTuple_->getElement(n).get(),
									vecValueLanguage);
			}
		}

		// 引数チェック
		if (vecValueLanguage.getSize() != 0
			&& vecValueLanguage.getSize() != vecValueSectionOffset.getSize())
		{
			// 要素数が違うのでエラー
			_SYDNEY_THROW0(Exception::InvalidSectionData);
		}

		vecOrgSectionOffset = vecValueSectionOffset;
		// 更新する
		m_pIndexFile->update(uiTupleID,
							 cstrKeyDocument,
							 vecKeyLanguage,
							 vecKeySectionOffset,
							 cstrValueDocument,
							 vecValueLanguage,
							 vecValueSectionOffset,
							 vecValueFeature);
	}

	try
	{
		// その他情報を更新できる形に整える
		Common::DataArrayData cTuple;
		ModVector<int> vecUpdateField;
		makeUpdateOtherInformationTuple(
			vecValueSectionOffset, pTuple_, vecValueFeature, cstrValueDocument,
			cTuple, vecUpdateField);
		// 更新する
		m_pOtherFile->update(uiTupleID,
							 cTuple,
							 vecUpdateField);
	}
	catch (...)
	{
		try
		{
			if (!isScoreOnly)
			{
				ModInvertedFeatureList vecKeyFeature;
				m_pIndexFile->update(uiTupleID,
									 cstrValueDocument,
									 vecValueLanguage,
									 vecOrgSectionOffset,
									 cstrKeyDocument,
									 vecKeyLanguage,
									 vecKeySectionOffset,
									 vecKeyFeature);
			}
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(m_cFileID.getLockName(), false);
		}
		_SYDNEY_RETHROW;
	}
}

//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::expunge(const Common::DataArrayData* pKey_)
{
	// キーにはすべてのフィールドが含まれている
	ModUInt32 uiTupleID;
	ModUnicodeString cstrKeyDocument;
	ModVector<ModLanguageSet> vecKeyLanguage;
	ModVector<ModSize> vecKeySectionOffset;
	convert(pKey_, uiTupleID, cstrKeyDocument,
			vecKeyLanguage, vecKeySectionOffset);

	// 削除する
	m_pIndexFile->expunge(cstrKeyDocument,
						  vecKeyLanguage,
						  uiTupleID,
						  vecKeySectionOffset);

	try
	{
		m_pOtherFile->expunge(uiTupleID);
	}
	catch (...)
	{
		try
		{
			ModInvertedFeatureList dummy;
			m_pIndexFile->insert(cstrKeyDocument,
								 vecKeyLanguage,
								 uiTupleID,
								 vecKeySectionOffset,
								 dummy);

		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(m_cFileID.getLockName(), false);
		}
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	FullText::LogicalInterface::fetch -- 検索条件を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData* pOption_
//		検索条件
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::fetch(const Common::DataArrayData* pOption_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	FullText::LogicalInterface::mark -- 巻き戻し位置を記録する
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
LogicalInterface::mark()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	FullText::LogicalInterface::rewind -- 巻き戻す
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
LogicalInterface::rewind()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	FullText::LogicalInterface::reset -- 論理ファイルへのカーソルをリセットする
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
LogicalInterface::reset()
{
	m_iGetCount = 0;
	m_cWordSet.clear();
}

//
//	FUNCTION public
//	FullText::LogicalInterface::equals -- 比較
//
//	NOTES
//	自身と引数 pOther_ の比較を行ない、比較結果を返す。
//	※ 同一オブジェクトかをチェックするのではなく、
//		 同じファイルかどうかをチェックする
//
//	ARGUMENTS
//	const Common::Object* pOther_
//		比較対象オブジェクトへのポインタ
//
//	RETURN
//	bool
//		自身と引数 pOther_ が同値ならば true を、
//		そうでなければ false を返す。
//
//	EXCEPTIONS
//	なし
//
bool
LogicalInterface::equals(const Common::Object* pOther_) const
{
	const LogicalInterface* pOther
		= dynamic_cast<const LogicalInterface*>(pOther_);
	if (pOther)
	{
		return toString() == pOther->toString();
	}
	return false;
}

//
//	FUNCTION public
//	FullText::LogicalInterface::sync -- 同期を取る
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& trans
//		トランザクション
//	bool& incomplete
//		処理し残したかどうか
//	bool& modified
//		更新されたかどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::sync(const Trans::Transaction& trans,
					   bool& incomplete, bool& modified)
{
	_AutoAttachFile cAuto(*this);

	m_pIndexFile->sync(trans, incomplete, modified);
	m_pOtherFile->sync(trans, incomplete, modified);
}

//
//	FUNCTION public
//	FullText::LogicalInterface::getProperty -- プロパティを得る
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData* pKey_
//		プロパティの項目を表すキー
//	Common::DataArrayData* pValue_
//		キーに対応するプロパティのバリュー
//
//	RETURN
//
//	EXCEPTIONS
//
void
LogicalInterface::getProperty(Common::DataArrayData* pKey_,
							  Common::DataArrayData* pValue_)
{
	; _TRMEISTER_ASSERT(pKey_ != 0 && pValue_ != 0);
	; _TRMEISTER_ASSERT(m_uiKwicSize > 0);

	// Set search term list
	pKey_->pushBack(new Common::StringData(_cSearchTermList));
	Common::DataArrayData* pSearchTermList = new Common::DataArrayData();
	m_pSearchCapsule = &m_pIndexFile->getSearchCapsule(this);
	m_pSearchCapsule->getSearchTermList(*pSearchTermList, m_cResultType);
	pValue_->pushBack(pSearchTermList);
	
	// Set rough kwic size
	pKey_->pushBack(new Common::StringData(_cRoughKwicSize));
	// [NOTE] SearchTermListの構造が変わったのを機に、
	//  最大単語長は考慮しないことにした。
	//  KWIC長より長い文字列をマークアップする意味はないため。
	pValue_->pushBack(new Common::UnsignedIntegerData(
						  getKwicMarginScaleFactor() * m_uiKwicSize));
	
	// Set una parameter
	pKey_->pushBack(new Common::StringData(_cUnaParameterKey));
	pKey_->pushBack(new Common::StringData(_cUnaParameterValue));
	Common::DataArrayData* pUnaParameterKey = new Common::DataArrayData();
	Common::DataArrayData* pUnaParameterValue = new Common::DataArrayData();
	m_pIndexFile->getFullInvert()->getUnaParameter(*pUnaParameterKey,
												   *pUnaParameterValue);
	pValue_->pushBack(pUnaParameterKey);
	pValue_->pushBack(pUnaParameterValue);
}

//
//	FUNCTION public
//	FullText::LogicalInterface::move -- 移動する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Common::StringArrayData& cArea_
//		エリア
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::move(const Trans::Transaction& cTransaction_,
					   const Common::StringArrayData& cArea_)
{
	Os::Path cOrgPath = m_cFileID.getPath();
	if (cOrgPath != cArea_.getElement(0))
	{
		_AutoAttachFile cAuto(*this);

		m_pIndexFile->move(cTransaction_, cArea_);//,m_cFileID);
		try
		{
			m_pOtherFile->move(cTransaction_, cArea_);
		}
		catch (...)
		{
			try
			{
				Common::StringArrayData cOrgArea;
				cOrgArea.setElement(0, cOrgPath);
				m_pIndexFile->move(cTransaction_, cOrgArea);//,m_cFileID);
			}
			catch (...)
			{
				SydErrorMessage << "Recovery failed." << ModEndl;
				Schema::File::setAvailability(m_cFileID.getLockName(),
											  false);
			}
			_SYDNEY_RETHROW;
		}
		m_cFileID.setPath(cArea_.getElement(0));
	}
}

// FUNCTION public
//	FullText::LogicalInterface::getNoLatchOperation -- ラッチが不要なオペレーションを返す
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	LogicalFile::File::Operation::Value
//
// EXCEPTIONS

LogicalFile::File::Operation::Value
LogicalInterface::
getNoLatchOperation()
{
	return Operation::Open
		| Operation::Close
		| Operation::Reset
		| Operation::GetProcessCost
		| Operation::GetOverhead;
}

// FUNCTION public
//	FullText::LogicalInterface::getCapability -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	LogicalFile::File::Capability::Value
//
// EXCEPTIONS

LogicalFile::File::Capability::Value
LogicalInterface::
getCapability()
{
	// 条件から件数を見積もることができる
	return Capability::EstimateCount;
}

//
//	FUNCTION public
//	FullText::LogicalInterface::toString -- ファイルを識別するための文字列を返す
//
//	NOTES
//	ファイルを識別するための文字列を返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		ファイルを識別するための文字列
//
//	EXCEPTIONS
//
ModUnicodeString
LogicalInterface::toString() const
{
	return m_cFileID.getPath();
}

//
//	FUNCTION public
//	FullText::LogicalInterface::attachFile -- ファイルをattachする
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
LogicalInterface::attachFile()
{
	if (m_cFileID.isDelayed() == true)
	{
		m_pIndexFile = new DelayIndexFile(m_cFileID);
	}
	else
	{
		m_pIndexFile = new SimpleIndexFile(m_cFileID);
	}

	if (m_cFileID.isOtherInformationFile())
	{
#ifdef SYD_CPU_SPARC
		; _SYDNEY_ASSERT(m_cFileID.getVersion() >= FileID::OtherVersion);
		// V2のみ
		m_pOtherFile = new OtherInformationFile2(m_cFileID);
#else
		if (m_cFileID.getVersion() >= FileID::OtherVersion)
		{
			// V2
			m_pOtherFile = new OtherInformationFile2(m_cFileID);
		}
		else
		{
			// V1
			m_pOtherFile = new OtherInformationFile1(m_cFileID);
		}
#endif
	}
	else
	{
		// OtherInformationFile0のメンバー関数は何もしない。
		// いちいちm_pOtherFileがnullかどうか検査するのは
		// わずらわしいので、何もしないクラスOtherInformationFile0を新設
		
		m_pOtherFile = new OtherInformationFile0(m_cFileID);
	}
}

//
//	FUNCTION public
//	FullText::LogicalInterface::detachFile -- ファイルをdetachする
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
LogicalInterface::detachFile()
{
	// [NOTE] SearchCapsule等はIndexFileで管理されている。
	//  SearchCapsule等を使う場所で毎回取得するのは手間なので、
	//  LogicalInterfaceでも保持している。
	//  したがって、IndexFileが初期化される時はSearchCapsule等も初期化する。
	m_pSearchCapsule = 0;
	m_pGetLocationCapsule = 0;
	
	delete m_pIndexFile, m_pIndexFile = 0;
	delete m_pOtherFile, m_pOtherFile = 0;
}

//
//	FUNCTION public
//	FullText::LogicalInterface::isAttached -- ファイルがattachされているかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		attachされいている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::isAttached() const
{
	return m_pIndexFile != 0;
}

//
//	FUNCTION public
//	FullText::LogicalInterface::isOpened -- オープンされているか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		 オープンされている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::isOpened() const
{
	return (isAttached() && m_pTransaction) ? true : false;
}

//
//	FUNCTION public
//	FullText::LoigicalInterface::isModifierValue
//		-- スコア調整用の値があるかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		スコア調整用の値がある場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::isModifierValue()
{
	return m_cFileID.isScoreField();
}

//
//	FUNCTION public
//	FullText::LogicalInterface::getModifierValue
//		-- スコア調整用の値を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//	double& value_
//		スコア調整用の値
//
//	RETURN
//	bool
//		値が存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::getModifierValue(ModUInt32 uiRowID_, double& value_)
{
	bool result = false;
	
	// その他情報ファイルからスコア調整用の値を得る
	Common::DoubleData d;
	m_pOtherFile->get(uiRowID_, m_iGetScoreField, d);
	if (d.isNull() == false)
	{
		value_ = d.getValue();
		result = true;
	}

	return result;
}

//
//	FUNCTION public
//	FullText::LogicalInterface::isFeatureValue
//		-- 特徴語データがあるかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		特徴語データがある場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::isFeatureValue()
{
	// [YET] 特徴語関連の用語の使い方が統一されていない。
	//  isFeatureValue(), getFeatureValue()
	//  makeFeatureFieldData()
	//  Inverted::FeatureSetPointer
	
	return m_cFileID.isClustering();
}

//
//	FUNCTION public
//	FullText::LogicalInterface::getFeatureValue
//		-- 特徴語データを取得する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//	Inverted::FeatureSetPointer& pFeatureSet_
//		特徴語データ
//
//	RETURN
//	bool
//		値が存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::getFeatureValue(ModUInt32 uiRowID_,
								  Inverted::FeatureSetPointer& pFeatureSet_)
{
	bool result = false;
	
	pFeatureSet_.free();
	
	// その他情報ファイルから特徴語データを得る
	//
	//【注意】
	//	特徴語のデータは格納時に、最大エリアサイズ以下になるように
	//	切り詰められている。よって、複数のエリアに跨ることはない
	//
	PhysicalFile::DirectArea area =
		m_pOtherFile->getArea(uiRowID_, m_iGetFeatureField);
	if (area.isValid())
	{
		const Inverted::FeatureSet* dst =
			syd_reinterpret_cast<const Inverted::FeatureSet*>(
				syd_reinterpret_cast<const char*>(
					const_cast<const PhysicalFile::DirectArea&>(area).
					operator const void*()) + sizeof(ModUInt32));
		// コピーする
		pFeatureSet_.copy(dst);

		// エリアのdetach
		area.detach();

		result = true;
	}

	return result;
}

//
//	FUNCTION private
//	FullText::LogicalInterface::convert -- 転置に挿入できる形に整える
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData* pTuple_
//		変換するデータ
//	ModUInt32& uiTupleID
//		取り出したタプルID
//	ModUnicodeString& cstrDocument
//		取り出した全文データ
//	ModVector<ModLanguageSet> vecLanguage
//		取り出した言語情報
//	ModVector<ModSize> vecSectionOffset
//		取り出したセクション位置情報
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::convert(const Common::DataArrayData* pTuple_,
						  ModUInt32& uiTupleID_,
						  ModUnicodeString& cstrDocument_,
						  ModVector<ModLanguageSet>& vecLanguage_,
						  ModVector<ModSize>& vecSectionOffset_)
{
	int n = 0;
	// 全文データ
	convertDocumentData(pTuple_->getElement(n++).get(),
						cstrDocument_, vecSectionOffset_);

	if (m_cFileID.isLanguage())
	{
		// 言語データ
		convertLanguageData(pTuple_->getElement(n++).get(), vecLanguage_);

		if (vecLanguage_.getSize() != 0
			&& vecSectionOffset_.getSize() != vecLanguage_.getSize())
		{
			// 要素数が異なっているのでエラー
			_SYDNEY_THROW0(Exception::InvalidSectionData);
		}
	}

	if (m_cFileID.isScoreField())
		// スコア調整用カラムがあるので、それは飛ばす
		++n;

	// ROWID
	convertRowidData(pTuple_->getElement(n++).get(), uiTupleID_);
}

//
//	FUNCTION private
//	FullText::LogicalInterface::convertDocumentData
//		-- 全文データをコンバートする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data* pData_
//		変換するデータ
//	ModUnicodeString& cstrDocument_
//		全文データ
//	ModVector<ModSize>& vecSectionOffset_
//		セクション位置
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::convertDocumentData(const Common::Data* pData_,
									  ModUnicodeString& cstrDocument_,
									  ModVector<ModSize>& vecSectionOffset_)
{
	cstrDocument_.clear();
	vecSectionOffset_.clear();

	if (pData_->getType() == Common::DataType::String)
	{
		// 文字列

		if (m_cFileID.isArray() == true)
			_SYDNEY_THROW0(Exception::BadArgument);

		const Common::StringData* pStringData
			= _SYDNEY_DYNAMIC_CAST(const Common::StringData*, pData_);
		cstrDocument_ = pStringData->getValue();
		vecSectionOffset_.pushBack(pStringData->getLength());
	}
	else if (pData_->getType() == Common::DataType::Array)
	{
		// 配列

		if (m_cFileID.isArray() == false)
			_SYDNEY_THROW0(Exception::BadArgument);

		const Common::DataArrayData* pArrayData
			= _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, pData_);
		ModSize length = 0;
		for (int i = 0; i < pArrayData->getCount(); ++i)
		{
			const Common::Data* pData = pArrayData->getElement(i).get();
			if (pData->isNull())
			{
				vecSectionOffset_.pushBack(length);
			}
			else if (pData->getType() == Common::DataType::String)
			{
				const Common::StringData* pStringData
					= _SYDNEY_DYNAMIC_CAST(const Common::StringData*, pData);
				cstrDocument_ += pStringData->getValue();
				length += pStringData->getLength();
				vecSectionOffset_.pushBack(length);
			}
			else
			{
				// エラー
				_SYDNEY_THROW0(Exception::BadArgument);
			}
		}
	}
	else
	{
		// エラー
		_SYDNEY_THROW0(Exception::BadArgument);
	}
}

//
//	FUNCTION private
//	FullText::LogicalInterface::convertLanguageData
//		-- 言語データをコンバートする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data* pData_
//		変換するデータ
//	ModVector<ModLanguageSet>& vecLanguage_
//		言語データ
//	bool checkArray_
//		配列かどうかチェックするか？ (default true)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::convertLanguageData(const Common::Data* pData_,
									  ModVector<ModLanguageSet>& vecLanguage_,
									  bool checkArray_)
{
	vecLanguage_.clear();

	if (!pData_->isNull())
	{
		if (pData_->getType() == Common::DataType::Language)
		{
			// 言語情報

			if (checkArray_ && m_cFileID.isArray() == true)
				_SYDNEY_THROW0(Exception::BadArgument);

			const Common::LanguageData* pLanguageData
				= _SYDNEY_DYNAMIC_CAST(const Common::LanguageData*, pData_);
			vecLanguage_.pushBack(pLanguageData->getValue());
		}
		else if (pData_->getType() == Common::DataType::String)
		{
			// 文字列で言語情報が与えられた

			if (checkArray_ && m_cFileID.isArray() == true)
				_SYDNEY_THROW0(Exception::BadArgument);

			const Common::StringData* pStringData
				= _SYDNEY_DYNAMIC_CAST(const Common::StringData*, pData_);
			vecLanguage_.pushBack(ModLanguageSet(pStringData->getValue()));
		}
		else if (pData_->getType() == Common::DataType::Array)
		{
			// 配列

			if (checkArray_ && m_cFileID.isArray() == false)
				_SYDNEY_THROW0(Exception::BadArgument);

			const Common::DataArrayData* pArrayData
				= _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, pData_);
			for (int i = 0; i < pArrayData->getCount(); ++i)
			{
				const Common::Data* pData = pArrayData->getElement(i).get();
				if (pData->isNull())
				{
					vecLanguage_.pushBack(ModLanguageSet());
				}
				else if (pData->getType() == Common::DataType::Language)
				{
					const Common::LanguageData* pLanguageData
						= _SYDNEY_DYNAMIC_CAST(const Common::LanguageData*,
											   pData);
					vecLanguage_.pushBack(pLanguageData->getValue());
				}
				else if (pData->getType() == Common::DataType::String)
				{
					const Common::StringData* pStringData
						= _SYDNEY_DYNAMIC_CAST(const Common::StringData*,
											   pData);
					vecLanguage_.pushBack(
						ModLanguageSet(pStringData->getValue()));
				}
				else
				{
					// エラー
					_SYDNEY_THROW0(Exception::BadArgument);
				}
			}
		}
		else
		{
			// エラー
			_SYDNEY_THROW0(Exception::BadArgument);
		}
	}
}

//
//	FUNCTION private
//	FullText::LogicalInterface::covertRowidData
//		-- ROWIDをコンバートする
//
//	NOTES
//
//	ARUGMENTS
//	const Common::Data* pData_
//		変換するデータ
//	ModUInt32& uiRowID_
//		変換後のROWID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::convertRowidData(const Common::Data* pData_,
								   ModUInt32& uiRowID_)
{
	if (pData_->getType() == Common::DataType::UnsignedInteger)
	{
		const Common::UnsignedIntegerData* pUnsignedIntegerData
			= _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData*, pData_);
		uiRowID_ = pUnsignedIntegerData->getValue();
	}
	else
	{
		// エラー
		_SYDNEY_THROW0(Exception::BadArgument);
	}
}

//
//	FUNCTION private
//	FullText::LogicalInterface::insertExpandDocument -- 拡張文書を登録する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData* pTuple_
//		挿入するタプルデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::insertExpandDocument(const Common::DataArrayData* pTuple_)
{
	if (pTuple_->getCount() == 0)
		_SYDNEY_THROW0(Exception::BadArgument);

	const Common::Data* pData = pTuple_->getElement(0).get();
	if (pData->isNull())
		return;

	// 0 : 全文データ
	// 1 : 言語データ(オプション)

	// まずは言語データを得る
	ModVector<ModLanguageSet> vecLanguage;
	if (pTuple_->getCount() == 2)
	{
		const Common::Data* pLang = pTuple_->getElement(1).get();
		if (!pLang->isNull())
		{
			// 配列かどうかはチェックしない
			convertLanguageData(pLang, vecLanguage, false);
		}
	}

	if (pData->getType() == Common::DataType::String)
	{
		// 文字列
		const Common::StringData* pStringData
			= _SYDNEY_DYNAMIC_CAST(const Common::StringData*, pData);

		// 設定する
		if (vecLanguage.getSize())
		{
			m_pSearchCapsule->pushExpandDocument(
				pStringData->getValue(),
				vecLanguage[0]);
		}
		else
		{
			m_pSearchCapsule->pushExpandDocument(
				pStringData->getValue(),
				m_pIndexFile->getFullInvert()->getDefaultLanguageSet());
		}
	}
	else if (pData->getType() == Common::DataType::Array)
	{
		// 配列

		const Common::DataArrayData* pArrayData
			= _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, pData);
		if (pArrayData->getCount() != static_cast<int>(vecLanguage.getSize())
			&& vecLanguage.getSize() != 0
			&& vecLanguage.getSize() != 1)
		{
			// エラー
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		ModLanguageSet lang;
		for (int i = 0; i < pArrayData->getCount(); ++i)
		{
			if (i < static_cast<int>(vecLanguage.getSize()))
				lang = vecLanguage[i];
			const Common::Data* pData = pArrayData->getElement(i).get();
			if (!pData->isNull())
			{
				if (pData->getType() == Common::DataType::String)
				{
					const Common::StringData* pStringData
						= _SYDNEY_DYNAMIC_CAST(const Common::StringData*,
											   pData);
					if (vecLanguage.getSize())
					{
						m_pSearchCapsule->pushExpandDocument(
							pStringData->getValue(),
							vecLanguage[0]);
					}
					else
					{
						m_pSearchCapsule->pushExpandDocument(
							pStringData->getValue(),
							m_pIndexFile->getFullInvert()->getDefaultLanguageSet());
					}
				}
				else
				{
					// エラー
					_SYDNEY_THROW0(Exception::BadArgument);
				}
			}
		}
	}
}
//
//	FUNCTION private
//	FullText::LogicalInterface::setPutField -- 更新フィールドを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::setPutField(const LogicalFile::OpenOption& cOpenOption_)
{							//    0                   1                     2
	PutValue::Type table1[] = {PutValue::DOCUMENT,PutValue::LANGUAGE,PutValue::SCORE};
	PutValue::Type table2[] = {PutValue::DOCUMENT,PutValue::SCORE};
	PutValue::Type *table;
	int tblSize;

	// [YET] DOCUMENTだけや、DOCUMENTとLANGUAGEだけのテーブルは生成されない？
	//  どこでチェックしているのか不明…

	if (m_cFileID.isLanguage())
	{
		table = table1;
		tblSize = sizeof(table1)/sizeof(*table1);
	}
	else
	{
		table = table2;
		tblSize = sizeof(table2)/sizeof(*table2);
	}

	m_vecPutField.clear();

	// 取得するフィールドを取得する順番にチェックする
	int count = cOpenOption_.getInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(
			FileCommon::OpenOption::TargetFieldNumber::Key));
	for (int i = 0; i < count; ++i)
	{
		int n = cOpenOption_.getInteger(
			_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
				FileCommon::OpenOption::TargetFieldIndex::Key, i));
		if( n < tblSize)
			m_vecPutField.pushBack(table[n]);
	}
}

//
// FUNCTION private
//	FullText::LogicalInterface::setGetField
//		-- 上位層から指定された方法で、検索結果を取得する方法を決定する。
//
// NOTES
//
// ARGUMENTS
// const LogicalFile::OpenOption& cOpenOption_
//
// RETURN
//
// EXCEPTIONS
//
void
LogicalInterface::setGetField(const LogicalFile::OpenOption& cOpenOption_)
{
	// [NOTE] 事前にm_eSortParameterを設定しておく
	
	// 以下を設定する
	// * m_pGetTuple
	//  転置の検索結果をtupleに変換する関数ポインタ。
	// * m_vecGetField
	//	全文が取得するフィールド型を、取得する順番に格納したベクタ。
	//  転置が取得するフィールド型も含む。
	//	[YET] 全文が取得するフィールド型だが、転置で定義された型を使っている。
	// * m_cResultType
	//  転置が取得するフィールド型を表したビットセット。
	//  順番は関係ないのでビットセットで十分。
	// * m_iGet〜Field
	//  その他情報ファイルから取得するデータのフィールド位置
	//  getOtherInfromationField内で設定される。
	// * m_cFieldMask
	//  全文が取得するフィールド型を表したビットセット。
	//  [YET] 実際は、セクション検索かどうかにしか使われていない。
	//   変数名と用途が一致していない。
	// * m_pSearchByBitSet
	//  絞り込み用のビットセット
	//  [YET] m_pGetTupleの設定に必要なのでここで取得しているが、
	//   取得フィールドとは関係ない。

	//
	// 前処理
	//
	
	// 取得するフィールドは固定フィールドのみなので、
	// 固定フィールド用のフィールドマスクを作成
	FieldMask mask(m_cFileID.isLanguage(),m_cFileID.isScoreField());

	//
	// 初期化
	//
	
	int count = cOpenOption_.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(
			FileCommon::OpenOption::TargetFieldNumber::Key));
	m_vecGetField.clear();
	m_vecGetField.reserve(count);
	m_cResultType = 0;
	
	//
	// 取得フィールドの設定
	//
	
	for (int i = 0; i < count; ++i)
	{
		// 取得するフィールドを取得する順番にチェックする

		// フィールド型を取得
		int n = cOpenOption_.getInteger(
			_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
				FileCommon::OpenOption::TargetFieldIndex::Key, i));
		FieldMask::FieldType type = mask.getFieldType(n);

		m_vecGetField.pushBack(type);
		
		switch (type)
		{
		case _SYDNEY::Inverted::FieldType::Rowid:
		case _SYDNEY::Inverted::FieldType::Tf:
		case _SYDNEY::Inverted::FieldType::Score:
			m_pGetTuple = &LogicalInterface::getSearchResultTuple;
			m_cResultType |= 1 << type;
			m_cFieldMask |= 1 << type;
			break;
		case _SYDNEY::Inverted::FieldType::Cluster:		// clusterを得る
			m_pGetTuple = &LogicalInterface::getSearchResultTuple;
			// クラスタリングは、スコアを使ってラフクラスタを生成し、
			// ラフクラスタ内を詳細クラスタリングしているためスコアが必要
			m_cResultType |= 1 << _SYDNEY::Inverted::FieldType::Score;
			m_cFieldMask  |= 1 << _SYDNEY::Inverted::FieldType::Score;
			// ラフクラスタの生成は、スコアの降順にソートされていることが必要
			m_eSortParameter = Inverted::SortParameter::ScoreDesc;
			break;
		case _SYDNEY::Inverted::FieldType::Section:
		case _SYDNEY::Inverted::FieldType::FeatureValue:
		case _SYDNEY::Inverted::FieldType::RoughKwicPosition:
			m_pGetTuple = &LogicalInterface::getSearchResultTuple;
			m_cFieldMask |= 1 << type;
			break;
		case _SYDNEY::Inverted::FieldType::Word:
			m_pGetTuple = &LogicalInterface::getWordTuple;
			m_cFieldMask |= 1 << type;
			break;
		case _SYDNEY::Inverted::FieldType::AverageLength:
		case _SYDNEY::Inverted::FieldType::AverageCharLength:
		case _SYDNEY::Inverted::FieldType::AverageWordCount:
		case _SYDNEY::Inverted::FieldType::Count:
		// 長さの取得
			m_pGetTuple = &LogicalInterface::getLength;
			m_cFieldMask |= 1 << type;
			break;
		case _SYDNEY::Inverted::FieldType::WordDf:
		case _SYDNEY::Inverted::FieldType::WordScale:
			// [NOTE] Wordに含めて返し、単体では値を返さない。
			m_cFieldMask |= 1 << type;
			break;
		default:
			break;
		}
	}
	
	// [NOTE] 新オプティマイザーはスコアでソートしても
	//  スコアを取得するとは限らないので、スコアを取得するように設定する。
	if (m_eSortParameter == Inverted::SortParameter::ScoreDesc ||
		m_eSortParameter == Inverted::SortParameter::ScoreAsc)
	{
		m_cResultType |= 1 << _SYDNEY::Inverted::FieldType::Score;
		m_cFieldMask |= 1 << _SYDNEY::Inverted::FieldType::Score;
	}

	//
	// OtherInformationファイルのフィールドを設定
	//
	
	getOtherInformationField();
	
	//
	// m_pGetTuple の再設定
	//

	if (cOpenOption_.getBoolean(
			_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::GetByBitSet::Key)) == true)
	{
		// ビットセットで取得する場合
		
		m_pGetTuple = &LogicalInterface::getByBitSet;

		// ソート条件をクリアする
		m_eSortParameter = Inverted::SortParameter::None;
		
		// limit, offsetもクリアする
		m_iLimit = Os::Limits<int>::getMax();
		m_iOffset = 1;
	}
	m_pSearchByBitSet = getBitSetForNarrowing(cOpenOption_);
	if ((m_cFieldMask & (1 << _SYDNEY::Inverted::FieldType::Section)) &&
		m_pSearchByBitSet)
	{
		// セクション検索かつビットセットによる絞り込みありの場合
		m_pGetTuple = &LogicalInterface::getSearchByBitSet;

		// [NOTE] getSearchByBitSet()は、WORDLIST条件に必須語が指定されると、
		//  結果が不正になってしまうので、getSearchResultTuple()を使う。
		//  参照：LogicalInterface::getSearchByBitSet(),
		//   Sydney固有の文書検索用の SearchCapsule::search()
		bool value = false;
		if (cOpenOption_.getBoolean(
				_SYDNEY_OPEN_PARAMETER_KEY(
					Inverted::OpenOption::KeyID::EssentialWordList), value)
			== true)
		{
			if (value == true)
			{
				m_pGetTuple = &LogicalInterface::getSearchResultTuple;
			}
		}
	}
}

//
// 検索項目の取得
//
bool
LogicalInterface::getField(FieldMask::FieldType type)
{
	for (ModVector<FieldMask::FieldType>::Iterator iter = m_vecGetField.begin();
		 iter != m_vecGetField.end(); ++iter)
	{
		if ((*iter) == type)
			return true;
	}
	return false;
}

//
//	FUNCTION private
//	FullText::LogicalInterface::getOtherInformationField
//		-- OtherInformationファイルのフィールドを取得
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
LogicalInterface::getOtherInformationField()
{
	// [YET] FileID::checkと実装が重なる
	
	// m_iGetScoreFieldはget()で、
	// m_iGetSectionFieldとm_iGetFeatureFieldはsetGetField()で、
	// それぞれ設定されていたので、
	// m_iGetUnnormalizedCharLengthFieldを追加するタイミングで
	// ここにまとめた。
	
	int k = 0;
	if (m_cFileID.isScoreField())
	{
		m_iGetScoreField = k;
		++k;
	}
	if (m_cFileID.isSectionized())
	{
		m_iGetSectionField = k;
		++k;
	}
	if (m_cFileID.isClustering())
	{
		m_iGetFeatureField = k;
		++k;
	}
	if (m_cFileID.isRoughKwic())
	{
		m_iGetUnnormalizedCharLengthField = k;
		++k;
	}
}

//
//	FUNCTION private
//	FullText::LogicalInterface::makeInsertOtherInformationTuple
//		-- その他情報をファイルに挿入できる形に整える
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<ModSize>& vecSectionOffset_
//		セクション情報
//	const Common::DataArrayData* pTuple_
//		挿入データ
//	const ModInvertedFeatureList& vecFeature_
//		特徴語情報
//	const ModUnicodeString& cstrDocument_
//		正規化前の文字列
//		[NOTE] pTuple_から取得できるが、convert()を呼ぶことを避けるため
//	Common::DataArrayData& cTuple_
//		ファイルに挿入するタプル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::makeInsertOtherInformationTuple(
	const ModVector<ModSize>& vecSectionOffset_,
	const Common::DataArrayData* pTuple_,
	const ModInvertedFeatureList& vecFeature_,
	const ModUnicodeString& cstrDocument_,
	Common::DataArrayData& cTuple_)
{
	cTuple_.clear();

	//
	//	今のところ挿入するデータは、スコア調整値とセクション情報と
	//	特徴語と正規化前データの文字数のみである
	//

	// 先頭列は文字列なので、1から開始。
	int n = 1;
	
	if (m_cFileID.isLanguage())
		// 言語情報があるが索引に格納しない。
		// 列だけ進めておく。
		++n;

	if (m_cFileID.isScoreField())
	{
		// スコア調整値がある
		cTuple_.pushBack(pTuple_->getElement(n));
	}

	if (m_cFileID.isSectionized())
	{
		// セクション情報がある
		cTuple_.pushBack(makeSectionFieldData(vecSectionOffset_));
	}

	if (m_cFileID.isClustering())
	{
		// 特徴語情報がある
		cTuple_.pushBack(makeFeatureFieldData(vecFeature_));
	}

	if (m_cFileID.isRoughKwic())
	{
		// 荒いKWIC情報がある
		// [NOTE] 今のところ、正規化前データの文字数のみ。
		cTuple_.pushBack(
			new Common::UnsignedIntegerData(cstrDocument_.getLength()));
	}
}

//
//	FUNCTION private
//	FullText::LogicalInterface::makeUpdateOtherInformationTuple
//		-- その他情報をファイルに更新できる形に整える
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<ModSize>& vecSectionOffset_
//		セクション情報
//	const Common::DataArrayData* pTuple_
//		挿入データ
//	const ModInvertedFeatureList& vecFeature_
//		特徴語情報
//	const ModUnicodeString& cstrDocument_
//		[NOTE] pTuple_から取得できるが、convert()を呼ぶことを避けるため
//	Common::DataArrayData& cTuple_
//		ファイルに挿入するタプル
//	ModVector<int>& vecUpdateField_
//		更新対象のフィールド番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalInterface::makeUpdateOtherInformationTuple(
	const ModVector<ModSize>& vecSectionOffset_,
	const Common::DataArrayData* pTuple_,
	const ModInvertedFeatureList& vecFeature_,
	const ModUnicodeString& cstrDocument_,
	Common::DataArrayData& cTuple_,
	ModVector<int>& vecUpdateField_)
{
	cTuple_.clear();
	vecUpdateField_.clear();
	
	//
	//	今のところ更新するデータは、スコア調整値とセクション情報と
	//	特長語情報と、正規化前データの文字数のみである
	//
	
	bool isUpdateKey = false;

	ModVector<PutValue::Type>::Iterator i = m_vecPutField.begin();
	for (int j = 0; i != m_vecPutField.end(); ++i, ++j)
	{
		if ((*i) == PutValue::DOCUMENT || (*i) == PutValue::LANGUAGE)
		{
			// キーが更新された
			isUpdateKey = true;
		}
		else if ((*i) == PutValue::SCORE)
		{
			// スコア調整値が更新対象
			cTuple_.pushBack(pTuple_->getElement(j));
			vecUpdateField_.pushBack(0);
		}
	}

	int	n = 0;

	if (m_cFileID.isScoreField())
	{
		// スコア調整カラムがある
		++n;
	}
	
	if (m_cFileID.isSectionized())
	{
		// セクション情報がある
		if (isUpdateKey)
		{
			cTuple_.pushBack(makeSectionFieldData(vecSectionOffset_));
			vecUpdateField_.pushBack(n);
		}
		++n;
	}

	if (m_cFileID.isClustering())
	{
		// 特長語情報がある
		if (isUpdateKey)
		{
			cTuple_.pushBack(makeFeatureFieldData(vecFeature_));
			vecUpdateField_.pushBack(n);
		}
		++n;
	}
	
	if (m_cFileID.isRoughKwic())
	{
		// 荒いKWIC情報がある
		// [NOTE] 今のところ、正規化前データの文字数のみ。
		
		if (isUpdateKey)
		{
			// [YET] LANGUAGEだけが更新されたり、
			//  データが更新されたが文字数が変わらない場合も更新してしまうが、
			//  そういうケースは少ないと思われるので更新してしまう。
			cTuple_.pushBack(
				new Common::UnsignedIntegerData(cstrDocument_.getLength()));
			vecUpdateField_.pushBack(n);
		}
		++n;
	}
}

//
//	FUNCTION private
//	FullText::LogicalInterface::makeSectionFieldData
//		-- その他情報ファイルに挿入するためのセクション情報データを作成する
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<ModSize>& vecSectionOffset_
//		セクションオフセット値
//
//	RETURN
//	Common::Data::Pointer
//		作成したセクション情報データ
//
//	EXCEPTIONS
//
Common::Data::Pointer
LogicalInterface::makeSectionFieldData(
	const ModVector<ModSize>& vecSectionOffset_)
{
	Common::UnsignedIntegerArrayData* pArray
		= new Common::UnsignedIntegerArrayData;
	Common::Data::Pointer p = pArray;
	pArray->reserve(vecSectionOffset_.getSize());
	ModVector<ModSize>::ConstIterator i = vecSectionOffset_.begin();
	for (; i != vecSectionOffset_.end(); ++i)
		pArray->pushBack(*i);
	return p;
}

//
//	FUNCTION private
//	FullText::LogicalInterface::makeFeatureFieldData
//		-- 特長語情報データを作成する
//
//	NOTES
//
//	ARGUMENTS
//	const ModInvertedFeatureList& vecFeature_
//		特長語情報配列
//
//	RETURN
//	Common::Data::Pointer
//		作成した特長語情報データ
//
//	EXCEPTIONS
//
Common::Data::Pointer
LogicalInterface::makeFeatureFieldData(
	const ModInvertedFeatureList& vecFeature_)
{
	Common::BinaryData* pBinary = new Common::BinaryData;
	Common::Data::Pointer p = pBinary;
	
	// エリアの最大サイズを得る
	ModSize maxSize = m_pOtherFile->getMaxStorableAreaSize()
		- sizeof(ModUInt32);	// BinaryDataの格納時にサイズを書くので
	// ダンプ時のサイズを求める
	ModSize size = Inverted::FeatureSet::getSize(vecFeature_, maxSize);
	// サイズ分の領域を確保する
	pBinary->assign(0, size);
	// ダンプする(特徴語の昇順に格納される)
	Inverted::FeatureSet::dump(syd_reinterpret_cast<char*>(pBinary->getValue()),
							   vecFeature_, maxSize);
	// 値を正規化する
	syd_reinterpret_cast<Inverted::FeatureSet*>(pBinary->getValue())->normalize();

	return p;
}

//
//	FUNCTION private
//	FullText::LogicalInterface::hitSection -- ヒットしたセクションを求める
//
//	NOTES
//
//	ARGUMENTS
//	const Common::UnsignedIntegerArrayData& cSectionOffset_
//		その他情報ファイルから取得したセクションオフセット情報
//	const Inverted::GetLocationCapsule::ResultSet& cLocation_
//		検索語がヒットした位置
//	Common::Data::Pointer pHitSection_
//		求めたヒットセクションの番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
bool
LogicalInterface::hitSection(
	const Common::UnsignedIntegerArrayData& cSectionOffset_,
	const Inverted::GetLocationCapsule::ResultSet& cLocation_,
	Common::Data::Pointer pHitSection_)
{

	; _SYDNEY_ASSERT(
		pHitSection_->getType() == Common::DataType::Array &&
		pHitSection_->getElementType() == Common::DataType::Data);

	if (cSectionOffset_.isNull())
		return false;

	Common::DataArrayData* pHitSection
		= _SYDNEY_DYNAMIC_CAST(Common::DataArrayData*,
							   pHitSection_.get());
	pHitSection->clear();

	int i = 0;
	Inverted::GetLocationCapsule::ResultSet::ConstIterator j
		= cLocation_.begin();
	while (i < cSectionOffset_.getCount() && j != cLocation_.end())
	{
		unsigned int v = cSectionOffset_.getElement(i);

		if ((*j).first >= v)
		{
			i++;
			continue;
		}

		pHitSection->pushBack(new Common::UnsignedIntegerData(i+1)); // 1ベース

		while (j != cLocation_.end())
		{
			if ((*j).first < v)
			{
				++j;
				continue;
			}
			break;
		}
		i++;
	}
	return true;
}

//
//	FUNCTION private
//	FullText::LogicalInterface::getFeatureValue
//		-- 特徴語データを取得(タプル格納用)
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//	Common::Data::Pointer pData_
//		特徴語データの格納先
//
//	RETURN
//
//	EXCEPTIONS
//
void
LogicalInterface::getFeatureValue(ModUInt32 uiRowID_,
								  Common::Data::Pointer pData_)
{
	; _SYDNEY_ASSERT(
		pData_->getType() == Common::DataType::Array &&
		pData_->getElementType() == Common::DataType::Data);

	// 初期化1
	Common::DataArrayData* pWordArray
		= _SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pData_.get());
	pWordArray->clear();

	// 特徴語セットの取得
	Inverted::FeatureSetPointer pFeatureSet;
	if (getFeatureValue(uiRowID_, pFeatureSet) == true)
	{
		// 特徴語を取得できた場合
		
		// 初期化2
		ModSize count = pFeatureSet->getCount();
		pWordArray->reserve(count);
		
		// 設定
		const Inverted::FeatureSet::Feature* pFeature = pFeatureSet->first();
		for (ModSize i = 0; i < count; ++i)
		{
			if (i > 0)
			{
				pFeature = pFeature->next();
			}
			
			Common::WordData* pWord = new Common::WordData(
				ModUnicodeString(pFeature->getString(), pFeature->getLength()));
			pWord->setScale(pFeature->getWeight());
			pWordArray->pushBack(pWord);
		}
	}
}

//
//	FUNCTION public
//	FullText::LogicalInterface::detachAllPages -- すべてのファイルをdetachする
//
//	NOTES
//	すべてのファイルを言っても、detachが必要なファイルはその他情報ファイル
//	だけである。
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
LogicalInterface::detachAllPages()
{
	m_pOtherFile->flushAllPages();
}

/////////////////////////////////////////////////////////////////////////////
//  以下は転置ライブラリの検索結果をtupleに変換するメンバー関数群
//


//
//	FUNCTION private
//	FullText::LogicalInterface::getByBitSet -- ビットセットで結果を得る
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData& cTuple_
//		結果を設定する配列
//
//	RETURN
//	bool
//		結果が得られた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::getByBitSet(Common::DataArrayData& cTuple_)
{
	bool result = false;

	if (m_iGetCount == 0)
	{
		// 最初のget()ですべての結果を返す
		Common::Data::Pointer pData = cTuple_.getElement(0);
		; _SYDNEY_ASSERT(pData->getType() == Common::DataType::BitSet);
		Common::BitSet* pBitSet
			= _SYDNEY_DYNAMIC_CAST(Common::BitSet*, pData.get());
		pBitSet->reset();	// クリアする

		if (m_iLimit != Os::Limits<int>::getMax() ||
			m_iOffset != 1)
		{
			// limit offset が指定されている場合は、
			// 1つづつ確認しながら設定する
			
			for (ModSize i = m_iOffset - 1; i < m_pSearchResult->getSize(); ++i)
			{
				ModInvertedDocumentID docid = m_pSearchResult->getDocID(i);
				if (m_pSearchByBitSet)
				{
					// ビットセットからの絞り込み
					// ビットセットの存在をチェックする
					if (m_pSearchByBitSet->test(docid) == false)
						continue;
				}
				if (m_iGetCount++ >= m_iLimit) break;

				pBitSet->set(docid);

				result = true;
			}
		}
		else
		{
			// それ以外の場合は、
			// まとめて and を取る

			// GetByBitSet のときは、必ず rowid の昇順にソートされている
			// それは、GetByBitSet が上から渡されるのは、セレクトリストも
			// ソートもない場合のみで、転置はソートがなければ、rowid 順で
			// 結果を返す。

			ModSize size = m_pSearchResult->getSize();
			Common::BitSet::UnitType unit;
			ModSize pos = 0;
			for (ModSize i = 0; i < size; ++i)
			{
				ModInvertedDocumentID docid = m_pSearchResult->getDocID(i);
				if (pos != (docid / (Common::BitSet::UNIT_SIZE * 8)))
				{
					if (unit.none() == false)
					{
						pBitSet->insertUnitType(pos, unit);
						unit.clear();
					}
					pos = docid / (Common::BitSet::UNIT_SIZE * 8);
				}
				
				unit._bitmap[(docid % (Common::BitSet::UNIT_SIZE * 8)) /
							 (Common::BitSet::SIZEOF_UINT * 8)]
					|= (1 << (docid % (Common::BitSet::SIZEOF_UINT * 8)));
			}
			if (unit.none() == false)
			{
				pBitSet->insertUnitType(pos, unit);
			}

			if (m_pSearchByBitSet)
			{
				pBitSet->operator &= (*m_pSearchByBitSet);
			}

			result = pBitSet->any();

			++m_iGetCount;
		}
	}

	return result;
}

//
//	FUNCTION private
//	FullText::LogicalInterface::m_pGetTuple -- 1つの結果を得る
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData& cTuple_
//		結果を格納する配列
//
//	RETURN
//	bool
//		結果が得られた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::getSearchResultTuple(Common::DataArrayData& cTuple_)
{
	while (m_iGetPos < m_pSearchResult->getSize())
	{
		if (m_pSearchByBitSet)
		{
			// ビットセットからの絞り込み
			// ビットセットの存在をチェックする

			if (m_pSearchByBitSet->test(
				m_pSearchResult->getDocID(m_iGetPos))	== false)
			{
				++m_iGetPos;
				continue;
			}
		}

		// limit処理
		if (m_cCluster.getSize() > 0)
		{
			// クラスタリング検索の場合
			// [NOTE] limitはクラスタ数の上限を意味するので、
			//  出力データ数でlimitの判定はできない。
			//  しかし、limitを超えるクラスタのクラスタIDは取得できないので、
			//  クラスタIDを取得できるデータは出力しても良い。
			// [NOTE] クラスタIDを取得すると同時に、検索結果も
			//  ソートされることがあるので、タプルを作る前に取得しておく。
			if (getClusterID(m_iGetPos) == false) break;
		}
		else
		{
			if (m_iGetCount >= m_iLimit) break;
		}
		++m_iGetCount;
		
		int e = 0;
		for (ModVector<FieldMask::FieldType>::Iterator iter = m_vecGetField.begin();
			 iter != m_vecGetField.end(); ++iter)
		{
			Common::Data::Pointer p = cTuple_.getElement(e++);
			switch (*iter)
			{
			case _SYDNEY::Inverted::FieldType::Rowid:
				; _SYDNEY_ASSERT(p->getType()
								 == Common::DataType::UnsignedInteger);
				_SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData*, p.get())
					->setValue(m_pSearchResult
							   ->getDocID(m_iGetPos));
				break;
			case _SYDNEY::Inverted::FieldType::Score:
				; _SYDNEY_ASSERT(p->getType() == Common::DataType::Double);
				_SYDNEY_DYNAMIC_CAST(Common::DoubleData*, p.get())
					->setValue(m_pSearchResult
							   ->getScore(m_iGetPos));
				break;
			case _SYDNEY::Inverted::FieldType::Section:
				{
					ModSize dummy(0);
					const Inverted::GetLocationCapsule::ResultSet& result
						= m_pGetLocationCapsule->execute(
							m_pSearchResult->getDocID(m_iGetPos), dummy);
					Common::UnsignedIntegerArrayData cSection;
					m_pOtherFile->get(m_pSearchResult->getDocID(m_iGetPos),
									  m_iGetSectionField,
									  cSection);
					if (hitSection(cSection, result, p) == false)
						p->setNull();
				}
				break;
			case _SYDNEY::Inverted::FieldType::Tf:
				{
					// 格納先の初期化
					Common::DataArrayData* vectTF
						= _SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, p.get());
					vectTF->clear();

					// TFリストの取得
					const ModInvertedVector<ModPair<ModUInt32,ModUInt32> >* tf
						= m_pSearchResult->getTF(m_iGetPos);

					// 格納
					ModUInt32 j = 0;
					for (ModSize i = 0 ; i < tf->getSize(); ++i)
					{
						// 各検索語を順番に処理する
						
						ModPair<ModUInt32,ModUInt32> pair = tf->at(i);
						while (j++ < pair.first)
						{
							// 検索語番号と一致するまで0で埋める。
							vectTF->pushBack(
								new Common::UnsignedIntegerData(0));
						}

						// TFを格納する。
						vectTF->pushBack(
							new Common::UnsignedIntegerData(pair.second));
					}

					/////////////////////////////////////////////////
					// wordlistで指定された単語数まで０で埋める
					/////////////////////////////////////////////////
					while (j++ < m_sizeWordlist)
					{
						vectTF->pushBack(new Common::UnsignedIntegerData(0));
					}
				}
				break;
			case _SYDNEY::Inverted::FieldType::Cluster:
				{
					; _SYDNEY_ASSERT(p->getType() == Common::DataType::Integer);
					// クラスタ番号は1-baseで返す。内部的には0-base。
					// 参照
					//  Inverted::SearchResultSet::initializeClusterIDList()
					_SYDNEY_DYNAMIC_CAST(Common::IntegerData*, p.get())
						->setValue(m_cCluster[m_iGetPos] + 1);
					break;
				}
			case _SYDNEY::Inverted::FieldType::FeatureValue:
				{
					getFeatureValue(m_pSearchResult->getDocID(m_iGetPos), p);
					break;
				}
			case _SYDNEY::Inverted::FieldType::RoughKwicPosition:
				{
					; _SYDNEY_ASSERT(p->getType() == Common::DataType::Integer);
					
					ModSize uiPosition = 0;
					
					// 取得関数はgetDocIDだが、RowIDに変換された値が入ってる
					ModUInt32 uiTupleID = static_cast<ModUInt32>(
						m_pSearchResult->getDocID(m_iGetPos));
					ModSize uiUnnormalizedCharLength = 0;
					double dAdjustFactor = getAdjustFactor(
						uiTupleID, uiUnnormalizedCharLength);
					if (dAdjustFactor > 0.0)
					{
						ModSize uiTermCount = 0;
						const Inverted::GetLocationCapsule::ResultSet& result
							= m_pGetLocationCapsule->execute(uiTupleID,
															 uiTermCount);
						uiPosition = getKwicPosition(
							result, uiTermCount,
							getAdjustKwicSize(dAdjustFactor));
						uiPosition = adjustRoughKwicPosition(
							uiPosition, dAdjustFactor,
							uiUnnormalizedCharLength);
					}
					
					_SYDNEY_DYNAMIC_CAST(Common::IntegerData*, p.get())->
						setValue(static_cast<int>(uiPosition));
					break;
				}
			} //switch
		} //for
		++m_iGetPos;
		return true;
	} //while
	return false;
}

//
//	FUNCTION private
//	FullText::LogicalInterface::getWordTuple -- 1つの結果を得る
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData& cTuple_
//		結果を格納する配列
//
//	RETURN
//	bool
//		結果が得られた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::getWordTuple(Common::DataArrayData& cTuple_)
{
	int offset = 1;
	while (m_wordIter != m_cWordSet.end())
	{
		if (m_iGetCount == 0 && offset++ < m_iOffset)
		{
			++m_wordIter;
			continue;
		}
		if (m_iGetCount++ >= m_iLimit) break;

		int e = 0;
		ModVector<FieldMask::FieldType>::Iterator i = m_vecGetField.begin();
		for (; i != m_vecGetField.end(); ++i)
		{
			if( *i == _SYDNEY::Inverted::FieldType::Word)
			{
				Common::Data::Pointer p = cTuple_.getElement(e++);
				; _SYDNEY_ASSERT(p->getType()
								 == Common::DataType::Word);
				_SYDNEY_DYNAMIC_CAST(Common::WordData*, p.get())
					->operator =(*m_wordIter);
			}
		}

		++m_wordIter;
		return true;
	}
	return false;
}

//
//	FUNCTION private
//	FullText::LogicalInterface::getLength -- 文書長を得る
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData& cTuple_
//		結果を格納する配列
//
//	RETURN
//	bool
//		結果が得られた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::getLength(Common::DataArrayData& cTuple_)
{
	bool result = false;
	if (m_iGetCount == 0)
	{
		bool bGet = false;
		ModInt64 len = 0;
		ModInt64 count = 0;

		if (isMounted(*m_pTransaction) == true)
		{
			try
			{
				// 総文書長を得る
				len = static_cast<ModInt64>(
					m_pIndexFile->getTotalDocumentLength());
				// 文書数を得る
				count = m_pIndexFile->getCount();
			}
			catch (...)
			{
				m_pIndexFile->recoverAllPages();
				_SYDNEY_RETHROW;
			}
			m_pIndexFile->flushAllPages();

			bGet = true;
		}
		m_iGetCount++;
		int e = 0;
		ModVector<FieldMask::FieldType>::Iterator i = m_vecGetField.begin();
		for (; i != m_vecGetField.end(); ++i)
		{
			switch (*i)
			{
			case _SYDNEY::Inverted::FieldType::AverageLength:
			case _SYDNEY::Inverted::FieldType::AverageCharLength:
			case _SYDNEY::Inverted::FieldType::AverageWordCount:
				{
					Common::Data::Pointer p = cTuple_.getElement(e++);
					if (bGet)
					{
						// v16.5から DoubleData ではなく
						// UnsignedIntegerData になった
					
						if (p->getType() != Common::DataType::UnsignedInteger)
							_SYDNEY_THROW0(Exception::BadArgument);
						_SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData*,
											 p.get())
							->setValue(static_cast<unsigned int>(len / count));
					}
					else
					{
						p->setNull();
					}
				}
				break;
			case _SYDNEY::Inverted::FieldType::Count:
				{
					Common::Data::Pointer p = cTuple_.getElement(e++);
					if (p->getType() != Common::DataType::UnsignedInteger)
						_SYDNEY_THROW0(Exception::BadArgument);
					_SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData*, p.get())
						->setValue(static_cast<unsigned int>(count));
				}
				break;
			}
		}
		result = true;
	}

	return result;
}


//
//	FUNCTION private
//	FullText::LogicalInterface::getSearchByBitSet -- 1つの結果を得る
//
//	NOTES
//	SearchByBitSetでの検索でかつ、ROWIDとセクション情報を取得する場合
//	セクション情報を取得しない場合には、このメソッドは呼んではいけない。
//	[YET] 関数名ではセクション検索時のみということがわからない。
//
//	ARGUMENTS
//	Common::DataArrayData& cTuple_
//		結果を格納する配列
//
//	RETURN
//	bool
//		結果が得られた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LogicalInterface::getSearchByBitSet(Common::DataArrayData& cTuple_)
{
	; _TRMEISTER_ASSERT(m_pSearchByBitSet != 0);
	
	bool result = false;

	int offset = 1;
	while (m_bitsetIter != m_pSearchByBitSet->end())
	{
		// 位置情報を取得する
		//
		//	【注意】
		//		GetLocationCapsuleの検索では削除されたものは考慮していない
		//		しかし、削除されたものは来るはずがないので、このままでいい

		ModSize dummy(0);
		const Inverted::GetLocationCapsule::ResultSet& r
			= m_pGetLocationCapsule->execute(*m_bitsetIter, dummy);
		if (r.getSize() == 0)
		{
			// ヒットしなかったので次
			++m_bitsetIter;
			continue;
		}

		if (m_iGetCount == 0 && offset++ < m_iOffset)
		{
			++m_bitsetIter;
			continue;
		}
		if (m_iGetCount++ >= m_iLimit) break;

		int e = 0;
		ModVector<FieldMask::FieldType>::Iterator i = m_vecGetField.begin();
		for (; i != m_vecGetField.end(); ++i)
		{
			switch (*i)
			{
			case _SYDNEY::Inverted::FieldType::Rowid:
				{
					Common::Data::Pointer p = cTuple_.getElement(e++);
					; _SYDNEY_ASSERT(p->getType()
									 == Common::DataType::UnsignedInteger);
					_SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData*, p.get())
						->setValue(*m_bitsetIter);
				}
				break;
			case _SYDNEY::Inverted::FieldType::Score:
				{
					Common::Data::Pointer p = cTuple_.getElement(e++);
					; _SYDNEY_ASSERT(p->getType() == Common::DataType::Double);
					_SYDNEY_DYNAMIC_CAST(Common::DoubleData*, p.get())
						->setValue(m_pSearchResult
							   ->getScore(m_iGetPos));
				}
				break;
			case _SYDNEY::Inverted::FieldType::Section:
				{
					Common::UnsignedIntegerArrayData cSection;
					m_pOtherFile->get(*m_bitsetIter,
									  m_iGetSectionField,
									  cSection);
					Common::Data::Pointer p = cTuple_.getElement(e++);
					if (hitSection(cSection, r, p) == false)
					{
						// 削除されている
						m_iGetCount--;
						break;
					}
					result = true;
				}
				break;
			}
		}

		++m_bitsetIter;

		if (result) break;
	}

	return result;
}

//
//	FUNCTION private
//	FullText::LogicalInterface::getKwicSize --
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::OpenOption& cOpenOption_
//
//	RETURN
//	ModSize
//
//	EXCEPTIONS
//
ModSize
LogicalInterface::getKwicSize(
	const LogicalFile::OpenOption& cOpenOption_) const
{
	ModSize uiSize = 0;
	
	int param;
	if (cOpenOption_.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(
				Inverted::OpenOption::KeyID::KwicSize), param) == true)
	{
		// [YET] ハイライトエリアのサイズはCONTAINS述語で指定されるが、
		//  使われるのはFullText。InvertedとFullTextの分割がうまくいってない。
		//  Inverted::OpenOption::setContains を参照
		
		// [NOTE] 1以上の値が入っている。
		//  Inverted::OpenOption::setContains を参照
		; _TRMEISTER_ASSERT(param > 0);
		uiSize = static_cast<ModSize>(param);
	}
	
	return uiSize;
}

//
//	FUNCTION private
//	FullText::LogicalInterface::getKwicMarginScaleFactor --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModSize
LogicalInterface::getKwicMarginScaleFactor() const
{
	; _TRMEISTER_ASSERT(_KwicMarginScaleFactor.get() > 0);
	; _TRMEISTER_ASSERT(_KwicMarginScaleFactorForNormalizing.get() > 0);
	
	return (m_cFileID.isRoughKwic() == true) ?
		static_cast<ModSize>(_KwicMarginScaleFactorForNormalizing.get()) :
		static_cast<ModSize>(_KwicMarginScaleFactor.get());
}

//
//	FUNCTION private
//	FullText::LogicalInterface::getAdjustFactor --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
double
LogicalInterface::getAdjustFactor(ModUInt32 uiTupleID_,
								  ModSize& uiUnnormalizedCharLength_)
{
	// [NOTE] まとめて取得してVectorに格納しておくと、
	//  クラスタIDを追加取得するたびにソートし直さなくてはならない。
	
	double dFactor = 1.0;
	
	if (m_cFileID.isRoughKwic() == true)
	{
		// 荒いKWICに関する情報がある場合
		
		// 索引に格納されている索引語の位置は、
		// 正規化後の文字位置や単語位置の場合がある。
		// 一方、上位から得られるKWICサイズや、
		// 上位に返す荒いKWICの開始位置などは、
		// 正規化前の文字列または文字位置が基準になっているので、
		// これらを変換する必要がある。

		// 単語索引の場合
		//  係数 = 文書内の総単語数 / 正規化前文字列長

		// 正規化索引の場合
		//  係数 = 正規化後の文字列長 / 正規化前文字列長
		
		// 正規化前文字列長
		; _TRMEISTER_ASSERT(m_pOtherFile != 0);
		Common::UnsignedIntegerData d;
		m_pOtherFile->get(uiTupleID_, m_iGetUnnormalizedCharLengthField, d);
		; _SYDNEY_ASSERT(d.isNull() == false);
		uiUnnormalizedCharLength_ = d.getValue();

		if (getKwicMarginScaleFactor() * m_uiKwicSize <
			uiUnnormalizedCharLength_)
		{
			// 単語数or正規化後文字列長
			; _TRMEISTER_ASSERT(m_pIndexFile != 0);
			ModSize uiDocumentLength =
				m_pIndexFile->getDocumentLength(uiTupleID_);
		
			// 係数の計算
			; _SYDNEY_ASSERT(uiUnnormalizedCharLength_ > 0);
			dFactor = static_cast<double>(
				uiDocumentLength) / uiUnnormalizedCharLength_;
		}
		else
		{
			// 荒いKWIC領域より対象データの方が小さいので、
			// 荒いKWIC領域には先頭を返せばよい。係数を0にしておく。
			dFactor = 0.0;
		}
	}
	else
	{
		// 荒いKWICに関する情報がないということは、
		// オリジナルの文字列長が格納されている。
		uiUnnormalizedCharLength_ =
			m_pIndexFile->getDocumentLength(uiTupleID_);

		if (getKwicMarginScaleFactor() * m_uiKwicSize >=
			uiUnnormalizedCharLength_)
		{
			dFactor = 0.0;
		}
	}
	
	return dFactor;
}

//
//	FUNCTION private
//	FullText::LogicalInterface::getKwicPosition
//		-- KWICの開始位置を計算
//
//	NOTES
//
//	ARGUMENTS
//	const Inverted::GetLocationCapsule::ResultSet& vecResult_
//		各索引語の位置。
//		正規化後の文字位置かどうか、単語位置かどうか、は索引に依存する。
//	ModSize uiTermCount_
//		索引語の異なり数
//	ModSize uiKwicSize_
//		ユーザが指定したKWIC長。
//		索引語の位置が、正規化後の文字位置かどうか、単語位置かどうか、
//		によって補正された値が与えられる。
//
//	RETURN
//	ModSize
//		荒いKWICの開始位置
//
//	EXCEPTIONS
//
ModSize
LogicalInterface::getKwicPosition(
	const Inverted::GetLocationCapsule::ResultSet& vecResult_,
	ModSize uiTermCount_,
	ModSize uiKwicSize_) const
{
	// 荒いKWICの中心部分となるシードKWICを取得する。
	
	// シードKWICの開始位置は、シードKWICに含まれる索引語の中で、
	// 最も先頭に出現する索引語の位置を返す。
	// シードKWICは、最も多くの相異なる索引語を含む範囲の中で、
	// 最も前方の範囲が選択される。
	
	// シードKWICの開始位置を示す索引語の開始位置
	ModSize uiPosition = 0;
	// シードKWICに含まれる異なり索引語数
	ModSize uiHitCount = 0;

	// iterator of tail
	Inverted::GetLocationCapsule::ResultSet::ConstIterator i =
		vecResult_.begin();
	ModSize uiTail = 0;
	ModSize uiTailPatternID = 0;
	// iterator of top
	Inverted::GetLocationCapsule::ResultSet::ConstIterator j = i;
	// hit count
	ModSize uiCurrentHitCount = 0;
	ModVector<ModSize> vecHitCount(uiTermCount_, ModSize(0));
	
	for (; i != vecResult_.end(); ++i)
	{
		// [NOTE] The head location of the tail pattern in the seed kwic.
		//  The location is 0-base,
		//  see Inverted::GetLocationCapsule::search() for details.
		uiTail = (*i).first;
		uiTailPatternID = (*i).second.second;
		
		// 新しい索引語を追加
		if (vecHitCount[uiTailPatternID]++ == 0)
		{
			++uiCurrentHitCount;
		}
		
		// シードKWICからはみ出した索引語を削除
		while (uiTail - (*j).first + 1 > uiKwicSize_ && j != i)
		{
			; _TRMEISTER_ASSERT(vecHitCount[(*j).second.second] > 0);
			if (--vecHitCount[(*j++).second.second] == 0)
			{
				; _TRMEISTER_ASSERT(uiCurrentHitCount > 0);
				--uiCurrentHitCount;
			}
		}

		// シードKWICを更新
		if (uiCurrentHitCount > uiHitCount)
		{
			; _TRMEISTER_ASSERT(uiPosition <= (*j).first);
			uiPosition = (*j).first;
			uiHitCount = uiCurrentHitCount;
			
			if (uiHitCount == uiTermCount_)
			{
				break;
			}
		}
	}

	return uiPosition;
}

//
//	FUNCTION private
//	FullText::LogicalInterface::adjustKwicSize --
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiKwicSize_
//		ユーザが指定したKWIC長
//
//	RETURN
//
//	EXCEPTIONS
//
ModSize
LogicalInterface::getAdjustKwicSize(double dAdjustFactor_) const
{
	; _TRMEISTER_ASSERT(dAdjustFactor_ > 0);
	
	// 正規化前文字列長から、正規化後文字列長or単語数に変換する。
	ModSize uiKwicSize = static_cast<ModSize>(dAdjustFactor_ * m_uiKwicSize);
	return (uiKwicSize == 0) ? 1 : uiKwicSize;
}

//
//	FUNCTION private
//	FullText::LogicalInterface::adjustRoughKwicPosition -- 位置の調整
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiPosition_
//		シードKWICの先頭位置
//
//	RETURN
//	ModSize
//
//	EXCEPTIONS
//
ModSize
LogicalInterface::adjustRoughKwicPosition(
	ModSize uiPosition_,
	double dAdjustFactor_,
	ModSize uiUnnormalizedCharLength_) const
{
	; _TRMEISTER_ASSERT(dAdjustFactor_ > 0);

	// 正規化位置or単語位置から戻すので、係数の逆数を掛け合わせる。
	ModSize uiPosition = static_cast<ModSize>(uiPosition_ / dAdjustFactor_);
	
	// マージンを追加
	; _TRMEISTER_ASSERT(getKwicMarginScaleFactor() >= 1);
	ModSize uiMargin = (getKwicMarginScaleFactor() - 1) * m_uiKwicSize / 2;
	
	// [NOTE] オフセットは0-baseで返す。
	//  FTSInvertedは1-baseだが、0-baseに変換済み。
	//  参照 ModInvertedTokenizer::tokenize(), GetLocationCapsule::search()
	ModSize result = 0;
	if (uiPosition > uiMargin)
	{
		result = uiPosition - uiMargin;

		// ラフKWICが文書の末尾からはみ出ていたら、開始位置を前に移動する
		ModSize uiRoughKwicSize = getKwicMarginScaleFactor() * m_uiKwicSize;
		// 参照: getAdjustFactor()
		; _TRMEISTER_ASSERT(uiUnnormalizedCharLength_ > uiRoughKwicSize);
		result = ModMin(uiUnnormalizedCharLength_ - uiRoughKwicSize,
						result);
	}
	return result;
}

//
//	FUNCTION private
//	FullText::LogicalInterface::getBitSetForNarrowing
//		-- 検索結果絞り込み用のビットセットを取得
//
//	NOTES
//	このビットセットは検索結果を絞り込むのに使う。
//	検索結果のうち、ビットが立っているデータだけを返す。
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
const Common::BitSet*
LogicalInterface::getBitSetForNarrowing(
	const LogicalFile::OpenOption& cOpenOption_) const
{
	const Common::BitSet* pBitSet = 0;
	
	const Common::Object* p = 0;
	if (cOpenOption_.getObjectPointer(
			_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::SearchByBitSet::Key), p) == true)
	{
		// ビットセットによる絞り込みがある場合
		 pBitSet = _SYDNEY_DYNAMIC_CAST(const Common::BitSet*, p);
	}
	
	return pBitSet;
}
	
//
//	FUNCTION private
//	FullText::LogicalInterface::getGetPos
//		-- 取得開始位置を取得する
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModSize
LogicalInterface::getGetPos()
{
	// [NOTE] getClusterID()が非constなので、これも非const。
	
	ModSize uiPos = 0;
	
	if (m_cCluster.getSize() > 0)
	{
		// クラスタリング検索の場合

		if (m_iOffset > 1)
		{
			// オフセット分のクラスタをスキップ
			int iClusterCount = 1;
			for (ModSize i = 0; getClusterID(i + 1) == true; ++i)
			{
				if (m_cCluster[i] != m_cCluster[i+1])
				{
					if (++iClusterCount == m_iOffset)
					{
						uiPos = i + 1;
						break;
					}
				}
			}
			if (iClusterCount < m_iOffset)
			{
				// クラスタ数がオフセット未満の場合
				uiPos = ModInt32Max;
			}
		}
	}
	else
	{
		// 非クラスタリング検索、または、検索結果が0件の場合
		; _TRMEISTER_ASSERT(m_iOffset > 0);
		uiPos = m_iOffset - 1;
	}

	return uiPos;
}

//
//	FUNCTION private
//	FullText::LogicalInterface::getClusterID
//		-- クラスタIDを取得する
//
//	NOTES
//	クラスタIDの未取得分を取得する際に、未取得分に対応した検索結果が
//	ソートされる。
//
//	ARGUMENTS
//	ModSize uiPos_
//		取得したいクラスタIDの位置
//		検索結果集合の位置(0-base)に対応する。
//
//	RETURN
//	bool
//		クラスタIDが取得できた
//
//	EXCEPTIONS
//
bool
LogicalInterface::getClusterID(ModSize uiPos_)
{
	; _TRMEISTER_ASSERT(m_pSearchCapsule != 0);
	; _TRMEISTER_ASSERT(m_cCluster.getSize() > 0);
	; _TRMEISTER_ASSERT(m_cCluster.getSize() == m_pSearchResult->getSize());

	return m_pSearchCapsule->getCluster(m_cCluster, m_pSearchResult, uiPos_);
}

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
