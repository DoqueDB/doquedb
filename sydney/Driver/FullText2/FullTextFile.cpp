// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FullTextFile.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2016, 2017, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"
#include "FullText2/FullTextFile.h"

#include "FullText2/GetCount.h"
#include "FullText2/GetResult.h"
#include "FullText2/ExpungeMP.h"
#include "FullText2/FakeError.h"
#include "FullText2/FieldMask.h"
#include "FullText2/FieldType.h"
#include "FullText2/File.h"
#include "FullText2/IDVectorFile.h"
#include "FullText2/InsertMP.h"
#include "FullText2/InvertedSection.h"
#include "FullText2/ListManager.h"
#include "FullText2/ListManagerWithWhiteList.h"
#include "FullText2/MergeReserve.h"
#include "FullText2/OpenOption.h"
#include "FullText2/Parameter.h"
#include "FullText2/SearchInformationArray.h"
#include "FullText2/SearchInformationConcatinate.h"
#include "FullText2/Tokenizer.h"
#include "FullText2/Query.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/DoubleData.h"
#include "Common/LanguageData.h"
#include "Common/Message.h"
#include "Common/StringData.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/WordData.h"

#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"
#include "Exception/SQLSyntaxError.h"
#include "Exception/VerifyAborted.h"

#include "LogicalFile/Estimate.h"

#include "Os/AutoCriticalSection.h"
#include "Os/Limits.h"

#include "Schema/File.h"

#include "Trans/AutoLatch.h"
#include "Trans/Transaction.h"

#include "FileCommon/OpenOption.h"

#include "Utility/ModTerm.h"
#include "Utility/TermResourceManager.h"
#include "Utility/UNA.h"

#include "ModInvertedCoder.h"

#include "ModAlgorithm.h"
#include "ModLanguageSet.h"
#include "ModUnicodeCharTrait.h"
#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

#define _CHECK_TYPE(data, type) \
			if ((data).getType() != Common::DataType::type) \
				_SYDNEY_THROW0(Exception::BadArgument)

namespace
{
	//
	//  CLASS
	//  _$$::_AutoAttachFile
	//
	class _AutoAttachFile
	{
	public:
		_AutoAttachFile(FullTextFile& cFile_, bool bVector_ = false)
			: m_cFile(cFile_), m_bOwner(false)
			{
				if (m_cFile.isAttached() == false)
				{
					m_cFile.attach(bVector_);
					m_bOwner = true;
				}
			}
		~_AutoAttachFile()
			{
				if (m_bOwner) m_cFile.detach();
			}

	private:
		FullTextFile& m_cFile;
		bool m_bOwner;
	};

	//
	//  CLASS
	//  _$$::_AutoDetachPage
	//
	class _AutoDetachPage
	{
	public:
		_AutoDetachPage(FullTextFile* pFile_)
			: m_pFile(0)
			{
				if (pFile_->isNoVersion()) m_pFile = pFile_;
			}
		~_AutoDetachPage()
			{
				recover();
			}
		void flush()
			{
				if (m_pFile) m_pFile->flushAllPages();
			}
		void recover()
			{
				if (m_pFile) m_pFile->recoverAllPages();
			}

	private:
		FullTextFile* m_pFile;
	};

	namespace _Path
	{
		// パス
		ModUnicodeString _cRowID("RowID");
		ModUnicodeString _cDocID("DocID");
		
		ModUnicodeString _cSection("F");
	}

	namespace _UnaParam
	{
		ModUnicodeString _cMaxWordLen("maxwordlen");
		ModUnicodeString _cDoNorm("donorm");
		ModUnicodeString _cCompound("compound");
		ModUnicodeString _cStem("stem");
		ModUnicodeString _cCarriage("carriage");
		ModUnicodeString _cSpace("space");

		ModUnicodeString _cTrue("true");
		ModUnicodeString _cFalse("false");
		ModUnicodeString _cTwo("2");
		ModUnicodeString _cZero("0");
	}

	namespace _Term
	{
		ModUnicodeString _cUNA("@UNARSCID:");
		ModUnicodeString _cTERM("@TERMRSCID:");
	}

	namespace _Property
	{
		ModUnicodeString _cRoughKwicSize("RoughKwicSize");
		ModUnicodeString _cSearchTermList("SearchTermList");
		ModUnicodeString _cUnaParameterKey("UnaParameterKey");
		ModUnicodeString _cUnaParameterValue("UnaParameterValue");
		
		ModUnicodeString _cUnaRscID("UnaRscID");
		ModUnicodeString _cDefaultLanguageSet("DefaultLanguageSet");
	}

	//
	//	KWICで返す長さを入力値の何倍にするか
	//
	// N-gramで正規化なし
	ParameterInteger _KwicMarginScaleFactor(
		"FullText_KwicMarginScaleFactor", 2);
	// オリジナル文書長あり
	ParameterInteger _KwicMarginScaleFactorForNormalizing(
		"FullText_KwicMarginScaleFactorForNormalizing", 10);
	// 正規化、かつ、オリジナル文書長なし
	ParameterInteger _KwicMarginScaleFactorForNoRoughKwic(
		"FullText_KwicMarginScaleFactorForNoRoughKwic", 20);

	//
	//	バッチインサートのサイズ
	//
#ifdef SYD_ARCH64
	ParameterInteger64 _BatchSizeMax(
		"Inverted_BatchSizeMax", ModInt64(512 << 20), false);
#else
	ParameterInteger64 _BatchSizeMax(
		"Inverted_BatchSizeMax", ModInt64(128 << 20), false);
#endif

	// バッチインサート時に出力するログの間隔
	ParameterInteger _BatchReportStep(
		"FullText2_BatchReportStep", 10000);

	// tea構文のコマンド
	ModUnicodeString _cContains("#contains");
	ModUnicodeString _cSingle("single");
	ModUnicodeString _cCat("cat");
	ModUnicodeString _cAnd("and");
	ModUnicodeString _cOr("or");

	//
	//	WordDataをソートするクラス
	//
	class _ScaleLess
	{
	public:
		ModBoolean operator() (const Common::WordData& x,
							   const Common::WordData& y)
			{ return (x.getScale() < y.getScale()) ? ModTrue : ModFalse; }
	};
	class _ScaleGreater
	{
	public:
		ModBoolean operator() (const Common::WordData& x,
							   const Common::WordData& y)
			{ return (x.getScale() > y.getScale()) ? ModTrue : ModFalse; }
	};
	class _DfLess
	{
	public:
		ModBoolean operator() (const Common::WordData& x,
							   const Common::WordData& y)
			{ return (x.getDocumentFrequency() < y.getDocumentFrequency())
				  ? ModTrue : ModFalse; }
	};
	class _DfGreater
	{
	public:
		ModBoolean operator() (const Common::WordData& x,
							   const Common::WordData& y)
			{ return (x.getDocumentFrequency() > y.getDocumentFrequency())
				  ? ModTrue : ModFalse; }
	};

	//	CONST
	//	$$$::_BitCount
	//		-- 1バイト中の立っているビットの数を引くテーブル
	//
	//	NOTES

	const int _BitCountTable[] =
	{
		0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
		1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
		1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
		1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
		3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
	};

	// ビット数を得る
	int _bitCount(unsigned int n_)
	{
		int size = 0;
		for (int i = 0; i < sizeof(int); ++i)
		{
			size += _BitCountTable[n_ & 0xff];
			n_ >>= 8;
		}
		return size;
	}

	// ビット位置をフィールド番号の配列に変換する
	ModVector<int> _convertField(int n_)
	{
		ModVector<int> ret;
		if (n_ == -1)
			return ret;
		
		unsigned int n = static_cast<unsigned int>(n_);
		int count = _bitCount(n);
		ret.reserve(count);
		
		int c = 0;
		unsigned int k = 1;
		for (int i = 0; i < 32; ++i)
		{
			if (n & k)
			{
				ret.pushBack(i);
				if (++c == count)
					break;
			}
			k <<= 1;
		}
		return ret;
	}
}

//
//	FUNCTION public
//	FullText2::FullTextFile::FullTextFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
FullTextFile::FullTextFile(FileID& cFileID_)
	: InvertedFile(cFileID_, cFileID_.getPath()),
	  m_pTransaction(0),
	  m_eOpenMode(LogicalFile::OpenOption::OpenMode::Unknown),
	  m_eFixMode(Buffer::Page::FixMode::Unknown),
	  m_bBatch(false), m_iMergeElement(-1),
	  m_pRowIDVectorFile(0), m_pDocIDVectorFile(0),
	  m_pIdCoder(0), m_pFrequencyCoder(0), m_pLengthCoder(0),
	  m_pLocationCoder(0), m_pWordIdCoder(0), m_pWordFrequencyCoder(0),
	  m_pWordLengthCoder(0), m_pWordLocationCoder(0),
	  m_lBatchSize(0), m_eSearchType(OpenOption::Type::None),
	  m_pQuery(0), m_pResultSet(0), m_pSearchInfo(0),
	  m_uiRowID(Os::Limits<ModUInt32>::getMax()), m_bEqual(false),
	  m_bScore(false), m_bTfList(false), m_bLocation(false),
	  m_bGetWord(false), m_bCluster(false), m_bGetByBitSet(false),
	  m_bFirst(true), m_bSearch(false), m_bPrepare(false),
	  m_eSortParameter(OpenOption::SortParameter::None),
	  m_uiGetCount(0),
	  m_eAdjustMethod(AdjustMethod::Unknown), m_fClusteredLimit(0),
	  m_eCombineMethod(Query::CombineMethod::Tf), m_uiKwicMarginScaleFactor(0),
	  m_bOnlyScoreField(false), m_iTermCount(0), m_iBatchCount(0),
	  m_pNarrowing(0), m_pRanking(0), m_uiRankingDocumentCount(0),
	  m_iNextKeyNumber(0)
{
}

//
//	FUNCTION public
//	FullText2::FullTextFile::~FullTextFile -- デストラクタ
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
FullTextFile::~FullTextFile()
{
	detach();	// ねんのため
}

//
//	FUNCTION public
//	FullText2::FullTextFile::isAccessible
//		-- 実体である OS ファイルが存在するか調べる
//
//	NOTES
//
//	ARGUMENTS
//	bool force (default false)
//		強制モードかどうか
//
//	RETURN
//	bool
//		存在する場合は true 、それ以外の場合は false
//
//	EXCEPTIONS
//
bool
FullTextFile::isAccessible(bool force_) const
{
	_AutoAttachFile cAuto(*const_cast<FullTextFile*>(this), true);
	return m_pRowIDVectorFile->isAccessible(force_);
}

//
//	FUNCTION public
//	FullText2::FullTextFile::isMounted
//		-- マウントされているか調べる
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	bool
//		マウントされている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FullTextFile::isMounted(const Trans::Transaction& cTransaction_) const
{
	_AutoAttachFile cAuto(*const_cast<FullTextFile*>(this), true);
	return m_pRowIDVectorFile->isMounted(cTransaction_);
}

//
//	FUNCTION public
//	FullText2::FullTextFile::getSize -- ファイルサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		ファイルサイズ
//
//	EXCEPTIONS
//
ModUInt64
FullTextFile::getSize()
{
	_AutoAttachFile cAuto(*this);
	return InvertedFile::getSize();
}

//
//	FUNCTION public
//	FullText2::FullTextFile::getCount
//		-- 挿入されているタプル数を得る
//		   検索条件が与えられている場合には、結果件数を返す
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInt64
//		件数
//
//	EXCEPTIONS
//
ModInt64
FullTextFile::getCount()
{
	ModInt64 count = 0;
	if (isMounted(*m_pTransaction))
	{
		_AutoDetachPage cAuto(this);
	
		if (m_cstrCondition.getLength() != 0)
		{
			// 検索条件が指定されているので、
			// その条件をもとに検索結果件数を見積もる

			const ModUnicodeChar* pTea = m_cstrCondition;
			ModAutoPointer<Query> pQuery = makeQuery(false, false, false,
													 pTea);
		
			count = pQuery->getEstimateCount(getSearchInformation(),
											 m_vecpListManager,
											 pTea);
		}
		else
		{
			// 検索条件が与えられていないので、
			// 登録件数を設定する

			count = m_pRowIDVectorFile->getCount();
		}
		
		cAuto.flush();
	}

	return count;
}

//
//	FUNCTION public
//	FullText2::FullTextFile::getOverhead
//		-- 論理ファイルオープン時のオーバヘッドコストを得る
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	double
//		オーバヘッド(秒)
//
//	EXCEPTIONS
//
double
FullTextFile::getOverhead()
{
	if (isMounted(*m_pTransaction) &&
		m_eFixMode ==  Buffer::Page::FixMode::ReadOnly)
	{
		// 転置のオーバヘッドは以下のように求める
		//
		//  検索語の個数 x 検索でアタッチするページを得るコスト
		//

		// B木を検索するコスト
		//  (だいたい3ページだけど1ページ500エントリぐらいあるので、500で割る)
		double bteeCost = File::getOverhead(m_cFileID.getBtreePageSize())
			* 3 / 500.0;

		// ページあたりの転置リスト数(だいたい1ページ10リストぐらいかな)
		double listCount = 10.0;

		// 検索でいくつのリストを触るか？
		// 0 の場合は、てきとうに 3 にする
		double usedList = (m_iTermCount == 0) ? 3 : m_iTermCount;

		return File::getOverhead(m_cFileID.getLeafPageSize())
			* usedList / listCount + bteeCost;
	}
	else
	{
		// 1ページのコストを返す
		return File::getOverhead(m_cFileID.getLeafPageSize());
	}
}

//
//	FUNCTION public
//	FullText2::FullTextFile::getProcessCost
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
FullTextFile::getProcessCost()
{
	// すべての検索結果はメモリー上にあるので、それの転送速度
	return static_cast<double>(sizeof(ModUInt32))
		/ LogicalFile::Estimate::getTransferSpeed(
			LogicalFile::Estimate::Memory);
}

//
//	FUNCTION public
//	FullText2::FullTextFile::destroy -- ファイルを破棄する
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
FullTextFile::destroy(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);
	InvertedFile::destroy(cTransaction_);
}

//
//	FUNCTION public
//	FullText2::FullTextFile::mount -- ファイルをマウントする
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
FullTextFile::mount(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);
	InvertedFile::mount(cTransaction_);
}

//
//	FUNCTION public
//	FullText2::FullTextFile::unmount -- ファイルをアンマウントする
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
FullTextFile::unmount(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);
	InvertedFile::unmount(cTransaction_);
}

//
//	FUNCTION public
//	FullText2::FullTextFile::flush -- ファイルをフラッシュする
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
FullTextFile::flush(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);
	InvertedFile::flush(cTransaction_);
}

//
//	FUNCTION public
//	FullText2::FullTextFile::startBackup -- バックアップを開始する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const bool bRestorable_
//		リストアフラグ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::startBackup(const Trans::Transaction& cTransaction_,
						  const bool bRestorable_)
{
	_AutoAttachFile cAuto(*this);
	InvertedFile::startBackup(cTransaction_, bRestorable_);
}

//
//	FUNCTION public
//	FullText2::FullTextFile::endBackup -- バックアップを終了する
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
FullTextFile::endBackup(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);
	InvertedFile::endBackup(cTransaction_);
}

//
//	FUNCTION public
//	FullText2::FullTextFile::recover -- 障害から回復する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Trans::TimeStamp& cPoint_
//		回復ポイント
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::recover(const Trans::Transaction& cTransaction_,
					  const Trans::TimeStamp& cPoint_)
{
	_AutoAttachFile cAuto(*this);
	InvertedFile::recover(cTransaction_, cPoint_);
}

//
//	FUNCTION public
//	FullText2::FullTextFile::restore
//		-- ある時点に開始された読取専用トランザクションが参照する版を最新とする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Trans::TimeStamp& cPoint_
//		回復ポイント
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::restore(const Trans::Transaction& cTransaction_,
					  const Trans::TimeStamp& cPoint_)
{
	_AutoAttachFile cAuto(*this);
	InvertedFile::restore(cTransaction_, cPoint_);
}

//
//	FUNCTION public
//	FullText2::FullTextFile::verify -- 整合性を検査する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Admin::Verification::Treatment::Value eTreatment_
//		処理方法
//	Admin::Verification::Progress& cProgress_
//		経過報告クラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::verify(const Trans::Transaction& cTransaction_,
					 const Admin::Verification::Treatment::Value eTreatment_,
					 Admin::Verification::Progress& cProgress_)
{
	_AutoAttachFile cAuto(*this);

	try
	{
		InvertedFile::verify(cTransaction_, eTreatment_, cProgress_);
	}
	catch (Exception::VerifyAborted&)
	{
		// 何もしない
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		unsetCoder(); 
		_SYDNEY_RETHROW;
	}

	// verify 中に転置リストが確保されるが、転置リストには必ず圧縮器が必要
	// そのため、ここで解放する必要あり
 
	unsetCoder(); 
}

//
//	FUNCTIION public
//	FullText2::FullTextFile::open -- ファイルをオープンする
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
FullTextFile::open(const Trans::Transaction& cTransaction_,
				   const LogicalFile::OpenOption& cOption_)
{
	OpenOption cOpenOption(const_cast<LogicalFile::OpenOption&>(cOption_));

	// オープンモードを確認する
	// オープンモードの設定は LogicalInterface で行っている
	//
	// FileCommon::OpenOption::OpenMode::Update -- 更新
	// FileCommon::OpenOption::OpenMode::Search -- 検索
	// FileCommon::OpenOption::OpenMode::Read   -- 固定値の取得
	// FileCommon::OpenOption::OpenMode::Batch  -- バッチモード

	int openMode;
	if (cOption_.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key),
			openMode) == false)
	{
		// 通常ありえない
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	// トランザクションを保存する
	m_pTransaction = &cTransaction_;
	// オープンモードを保存する
	m_eOpenMode
		= static_cast<LogicalFile::OpenOption::OpenMode::Value>(openMode);

	try
	{
		switch (m_eOpenMode)
		{
		case LogicalFile::OpenOption::OpenMode::Search:
			// 検索
			m_eFixMode = Buffer::Page::FixMode::ReadOnly;
		
			openForSearch(cOpenOption);
		
			break;

		case LogicalFile::OpenOption::OpenMode::Read:
			// 固定値の取得
			m_eFixMode = Buffer::Page::FixMode::ReadOnly;
		
			openForRead(cOpenOption);
		
			break;

		case LogicalFile::OpenOption::OpenMode::Update:
			// 通常の更新
			m_eFixMode = Buffer::Page::FixMode::Write |
				Buffer::Page::FixMode::Discardable;
		
			openForUpdate(cOpenOption);
		
			break;

		case LogicalFile::OpenOption::OpenMode::Batch:
			// バッチ更新
			m_eFixMode = Buffer::Page::FixMode::Write |
				Buffer::Page::FixMode::Discardable;
			m_bBatch = true;
			m_iBatchCount = 0;

			SydMessage << "Start FullText Batch Insert. ("
					   << m_cFileID.getPath() << ")" << ModEndl;
		
			openForUpdate(cOpenOption);
		
			break;
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// クローズする
		close();
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	FullText2::FullTextFile::close -- ファイルをクローズする
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
FullTextFile::close()
{
	if (m_bBatch)
	{
		// 残りをマージする

		ModVector<InvertedSection*>::Iterator i = m_vecpSection.begin();
		for (; i != m_vecpSection.end(); ++i)
		{
			// マージする
			(*i)->syncMerge(true);
		}
		
		SydMessage << "End FullText Batch Insert." << ModEndl;
	}

	clearTokenizer();
	delete m_pSearchInfo, m_pSearchInfo = 0;
	unsetCoder();
	clearMP();
	
	flushAllPages();
	
	// すべてのファイルをクローズする
	InvertedFile::close();
	
	m_pTransaction = 0;
	m_eOpenMode = LogicalFile::OpenOption::OpenMode::Unknown;
	m_eFixMode = Buffer::Page::FixMode::Unknown;
	m_bBatch = false;
	m_lBatchSize = 0;

	m_cstrCondition.clear();
	m_eSearchType = OpenOption::Type::None;

	m_bEqual = false;
	
	m_bScore = false;
	m_bTfList = false;
	m_bLocation = false;
	m_bGetWord = false;
	m_bCluster = false;
	m_bGetByBitSet = false;
	m_bFirst = true;
	m_bSearch = false;
	m_bPrepare = false;

	m_vecGetField.clear();
	m_eSortParameter = OpenOption::SortParameter::None;
	m_uiGetCount = 0;
	m_eAdjustMethod = AdjustMethod::Unknown;
	m_vecClusterScale.clear();
	m_eCombineMethod = Query::CombineMethod::None;
	m_vecKwicSize.clear();
	m_uiKwicMarginScaleFactor = 0;
	m_vecUpdateField.clear();
	m_bOnlyScoreField = false;
	m_iTermCount = 0;
	m_iBatchCount = 0;
	delete m_pNarrowing, m_pNarrowing = 0;
	delete m_pRanking, m_pRanking = 0;
	m_uiRankingDocumentCount = 0;
	m_vecExpandDocument.clear();
	m_vecExpandLanguage.clear();
	m_vecLocationField.clear();
	
	detach();
}

//
//	FUNCTION public
//	FullText2::FullTextFile::get -- データを取得する
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData& cTuple_
//		取得したデータを格納する配列
//
//	RETURN
//	bool
//		取得できた場合はtrue、それ以外の場合はfalse
//
//	EXCEPIONS
//
bool
FullTextFile::get(Common::DataArrayData& cTuple_)
{
	if (m_eOpenMode == LogicalFile::OpenOption::OpenMode::Unknown)
		// オープンされていない
		return false;
	
	//
	//【注意】
	//	他のファイルドライバーと違い、検索結果をすべてキャッシュする
	//	全文索引は get が呼ばれるたびにページをflushするようなことはしない
	//
	
	bool result = false;
	
	if (m_eOpenMode == LogicalFile::OpenOption::OpenMode::Search)
	{
		if (isMounted(*m_pTransaction))
		{
			// 検索の準備を行う
			prepareSearch();
		
			if (m_bGetWord)
			{
				result = getForWord(cTuple_);
			}
			else if (m_bEqual)
			{
				result = getForEqual(cTuple_);
			}
			else
			{
				result = getForSearch(cTuple_);
			}
		}
	}
	else if (m_eOpenMode == LogicalFile::OpenOption::OpenMode::Read)
	{
		result = getForRead(cTuple_);
	}
	else
	{
		// ありえない
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	return result;
}

//
//	FUNCTION public
//	FullText2::FullTextFile::reset -- 取得時のイテレータをリセットする
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
FullTextFile::reset()
{
	if (m_pTransaction->isNoVersion() == true)
	{
		// 版管理を利用する読み取り専用トランザクションでのみ実行可
		// それ以外の場合はエラー
		
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// 初めから取得
	m_bFirst = true;
}

//
//	FUNCTION public
//	FullText2::FullTextFile::insert --- データを挿入する
//
//	NOTES
//
//	ARGUMETNS
//	const Common::DataArrayData& cTuple_
//		挿入するデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::insert(const Common::DataArrayData& cTuple_)
{
	if (m_eOpenMode == LogicalFile::OpenOption::OpenMode::Search)
	{
		// オープンモードがSearchなのにinsertに来るってことは
		// 適合性フィードバック

		pushExpandDocument(cTuple_);
	}
	else
	{
		// 通常の挿入

		_AutoDetachPage cAuto(this);
		
		// 文書ID <-> ROWID変換のベクターファイルが作成されていなければ作成する
		createVector();

		int keyCount = m_cFileID.getKeyCount();

		// 新しい DocID を得る
		DocumentID uiDocID = getNewDocumentID();

		try
		{
			// ROWID
			ModUInt32 uiRowID = getRowID(cTuple_);

			// 複合索引の場合は、マルチスレッドで挿入する
			InsertMP cMP(*this, cTuple_, uiDocID);
			cMP.run((keyCount > 1) ? true : false);

			// DocID <-> ROWID をベクターに登録する
			insertVector(uiDocID, uiRowID);

			if (m_bBatch)
			{
				++m_iBatchCount;

				if ((m_iBatchCount % _BatchReportStep.get()) == 0)
				{
					SydMessage << "FullText Batch : " << m_iBatchCount
							   << ModEndl;
				}
			}
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
#ifdef SYD_FAKE_ERROR
			FakeErrorMessage << "FullTextFile::insert" << ModEndl;
#endif
			// 変更を破棄する
			cAuto.recover();
			
			try
			{
				// エラーが発生したので、登録した分を削除する

				ModVector<int>::Iterator i = m_vecSuccessKeyNumber.begin();
				for (; i != m_vecSuccessKeyNumber.end(); ++i)
				{
					int key = *i;

					// 削除できる形に整える
					ModVector<ModUnicodeString> vecDocument;
					ModVector<ModLanguageSet> vecLanguage;
					double dummy;

					convertData(key, cTuple_,
								vecDocument, vecLanguage, dummy);

					// 削除する
					getInvertedSection(key)->expunge(
						vecDocument, vecLanguage, uiDocID);
				}

				// 変更を確定させる
				cAuto.flush();
			}
			catch (...)
			{
				// エラー処理に失敗したので、利用可能性をOFFにする
				SydErrorMessage << "Recovery failed." << ModEndl;
				Schema::File::setAvailability(
					m_cFileID.getLockName(), false);
			}

			clearMP();
			_SYDNEY_RETHROW;
		}

		clearMP();

		// 必要ならマージする(バッチインサート時のみ)
		checkAndMerge();
		
		// 変更を確定させる
		cAuto.flush();
	}
}

//
//	FUNCTION public
//	FullText2::FullTextFile::update
//		-- データの更新を行う
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cKey_
//		キー値
//	const Common::DataArrayData& cTuple_
//		更新データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::update(const Common::DataArrayData& cKey_,
					 const Common::DataArrayData& cTuple_)
{
	_AutoDetachPage cAuto(this);

	// 文書ID <-> ROWID変換のベクターファイルが作成されていなければ作成する
	createVector();

	if (m_bOnlyScoreField)
	{
		// スコア調整カラムのみ

		; _SYDNEY_ASSERT(cTuple_.getCount() == 1);

		// ROWIDを得る
		ModUInt32 uiRowID = getRowID(cKey_);

		// 文書IDへ変換する
		DocumentID uiDocID;
		if (m_pRowIDVectorFile->get(uiRowID, uiDocID) == false)
		{
			// 存在しないのは通常ありえないが、
			// サポートの都合でファイルを削除したりするとあり得る
			// よって、エラーにはせずメッセージのみとする

			SydErrorMessage << "Entry not found. : ROWID = "
							<< uiRowID << ModEndl;
			return;
		}

		// スコアをdoubleにする
		double dblScore;
		convertScore(cTuple_.getElement(0).get(), dblScore);
		
		ModVector<InvertedSection*>::Iterator i = m_vecpSection.begin();
		for (; i != m_vecpSection.end(); ++i)
		{
			// 更新する
			
			(*i)->updateScoreData(uiDocID, dblScore);
		}
	}
	else
	{
		// 通常の更新なので、削除してから挿入する

		// 挿入するためのデータを作成する
		Common::DataArrayData cInsert;
		cInsert.setValue(cKey_.getValue());
		int n = 0;
		ModVector<int>::Iterator i = m_vecUpdateField.begin();
		for (; i != m_vecUpdateField.end(); ++i, ++n)
		{
			// 更新後のデータで上書きする
			cInsert.setElement(*i, cTuple_.getElement(n));
		}

		// 削除する
		expunge(cKey_);

		try
		{
			// 挿入する
			insert(cInsert);
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
#ifdef SYD_FAKE_ERROR
			FakeErrorMessage << "FullTextFile::update" << ModEndl;
#endif
			// 変更を破棄する
			cAuto.recover();
			
			try
			{
				// 削除したものを登録する
				insert(cKey_);
			}
			catch (...)
			{
				// エラー処理に失敗したので、利用可能性をOFFにする
				SydErrorMessage << "Recovery failed." << ModEndl;
				Schema::File::setAvailability(m_cFileID.getLockName(), false);
			}
			
			_SYDNEY_RETHROW;
		}
 
		// 必要ならマージする(バッチインサート時のみ)
		checkAndMerge();
	}

	cAuto.flush();
}

//
//	FUNCTION public
//	FullText2::FullTextFile::expunge
//		-- データの削除を行う
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cKey_
//		キーデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::expunge(const Common::DataArrayData& cKey_)
{
	_AutoDetachPage cAuto(this);

	// 文書ID <-> ROWID変換のベクターファイルが作成されていなければ作成する
	createVector();

	int keyCount = m_cFileID.getKeyCount();

	// RowID を文書IDに変換する
	DocumentID uiDocID = UndefinedDocumentID;
	ModUInt32 uiRowID = getRowID(cKey_);
	if (m_pRowIDVectorFile->get(uiRowID, uiDocID) == false)
	{
		// 存在しないのは通常ありえないが、
		// サポートの都合でファイルを削除したりするとあり得る
		// よって、エラーにはせずメッセージのみとする

		SydErrorMessage << "Entry not found. : ROWID = "
						<< uiRowID << ModEndl;
		return;
	}

	try
	{
		// 複合索引の場合は、マルチスレッドで削除する
		ExpungeMP cMP(*this, cKey_, uiDocID);
		cMP.run((keyCount > 1) ? true : false);
		
		// DocID <-> ROWID をベクターから削除する
		expungeVector(uiDocID, uiRowID);

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
#ifdef SYD_FAKE_ERROR
		FakeErrorMessage << "FullTextFile::expunge" << ModEndl;
#endif
		// 変更を破棄する
		cAuto.recover();
		
		try
		{
			// エラーが発生したので、削除した分をrollbackする

			ModVector<int>::Iterator i = m_vecSuccessKeyNumber.begin();
			for (; i != m_vecSuccessKeyNumber.end(); ++i)
			{
				int key = *i;
				
				// 削除できる形に整える
				ModVector<ModUnicodeString> vecDocument;
				ModVector<ModLanguageSet> vecLanguage;
				double dummy;

				convertData(key, cKey_,
							vecDocument, vecLanguage, dummy);

				// 削除を取り消す
				getInvertedSection(key)->expungeRollBack(
					vecDocument, vecLanguage, uiDocID);
			}
			
			// 変更を確定させる
			cAuto.flush();
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(m_cFileID.getLockName(), false);
		}

		clearMP();
		_SYDNEY_RETHROW;
	}

	clearMP();
	
	try
	{
		ModVector<InvertedSection*>::Iterator j = m_vecpSection.begin();
		
		// すべての削除を確定する
		
		for (int i = 0; i < keyCount; ++i, ++j)
		{
			// 削除を確定する
			bool needMerge = (*j)->expungeCommit();

			if (needMerge == true)
			{
				// マージが必要なので登録する

				MergeReserve::pushBack(m_cFileID.getLockName(), i);
			}
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		SydErrorMessage << "Recovery failed." << ModEndl;
		Schema::File::setAvailability(m_cFileID.getLockName(), false);
		
		_SYDNEY_RETHROW;
	}

	// 必要ならマージする(バッチインサート時のみ)
	checkAndMerge();
	
	// 変更を確定させる
	cAuto.flush();
}

//
//	FUNCTION public
//	FullText2::FullTextFile::sync
//		-- 同期処理を実行する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& trans
//		トランザクション
//	bool& incomplete
//		同期すべき版がまだ残っている場合はtrue、それ以外の場合はfalse
//	bool& modified
//		ファイルを更新した場合はtrue、それ以外の場合はfalse
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::sync(const Trans::Transaction& trans,
				   bool& incomplete, bool& modified)
{
	_AutoAttachFile cAuto(*this);
	InvertedFile::sync(trans, incomplete, modified);
}

//
//	FUNCTION public
//	FullText2::FullTextFile::move
//		-- ファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Os::Path& cNewPath_
//		移動先のパス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::move(const Trans::Transaction& cTransaction_,
				   const Os::Path& cNewPath_)
{
	Os::Path cOrgPath = m_cFileID.getPath();

	// 新旧のパスを比較する
	
	int r = Os::Path::compare(cNewPath_, cOrgPath);
	if (r == Os::Path::CompareResult::Identical)
	{
		// 同じパスなので、何もしない
		return;
	}
	
	_AutoAttachFile cAuto(*this);

	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく移動する
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	bool accessible = (isAccessible() &&
					   r == Os::Path::CompareResult::Unrelated);
	int step = 0;
	int i = 0;
	Os::Path path;
	
	try
	{
		// RowID -> 文書ID のベクターファイル
		path = cNewPath_;
		path.addPart(_Path::_cRowID);
		m_pRowIDVectorFile->move(cTransaction_, path);

		++step;

		// 文書ID -> RowID のベクターファイル
		path = cNewPath_;
		path.addPart(_Path::_cDocID);
		m_pDocIDVectorFile->move(cTransaction_, path);

		++step;

		// セクションファイル
		int keyCount = m_cFileID.getKeyCount();
		for (; i != keyCount; ++i)
		{
			path = cNewPath_;
			ModUnicodeOstrStream s;
			s << _Path::_cSection << i;
			path.addPart(s.getString());

			m_vecpSection[i]->move(cTransaction_, path);
		}

		m_cFileID.setPath(cNewPath_);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
#ifdef SYD_FAKE_ERROR
		FakeErrorMessage << "FullTextFile::move (step="
						 << step << ")" << ModEndl;
#endif
		try
		{
			switch (step)
			{
			case 2:
				{
					// セクションファイル
					while (i > 0)
					{
						--i;
						
						path = cOrgPath;
						ModUnicodeOstrStream s;
						s << _Path::_cSection << i;
						path.addPart(s.getString());

						m_vecpSection[i]->move(cTransaction_, path);
					}

					// 文書ID -> ROWID のベクターファイル
					path = cOrgPath;
					path.addPart(_Path::_cDocID);
					m_pDocIDVectorFile->move(cTransaction_, path);
				}
			case 1:
				{
					// ROWID -> 文書ID のベクターファイル
					path = cOrgPath;
					path.addPart(_Path::_cRowID);
					m_pRowIDVectorFile->move(cTransaction_, path);
				}
			}
			
			if (accessible)
				rmdir(cNewPath_);
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
//	FullText2::FullTextFile::getProperty
//		-- プロパティを取得する
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData& cKey_
//		キー
//	Common::DataArrayData& cValue_
//		バリュー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::getProperty(Common::DataArrayData& cKey_,
						  Common::DataArrayData& cValue_)
{
	if (m_bLocation == false)
		// 位置情報を取得しないのに、getPropertyは呼ばれない
		_SYDNEY_THROW0(Exception::BadArgument);
	
	// 検索の準備を行う
	prepareSearch();
		
	cKey_.clear();
	cKey_.reserve(4);
	cValue_.clear();
	cValue_.reserve(4);
	
	// 検索語リスト
	cKey_.pushBack(new Common::StringData(_Property::_cSearchTermList));
	Common::DataArrayData* pSearchTermList = new Common::DataArrayData();
	m_pResultSet->getSearchTerm(*(m_vecLocationField.begin()),	// 先頭のみ
								*pSearchTermList);
	cValue_.pushBack(pSearchTermList);

	if (m_vecKwicSize.getSize() == 0)
		// 検索語だけを取得するモードなので、以下のデータは不要
		return;

	// KWICのサイズ
	cKey_.pushBack(new Common::StringData(_Property::_cRoughKwicSize));
	Common::DataArrayData* s = new Common::DataArrayData;
	ModVector<ModPair<int, ModSize> >::Iterator i = m_vecKwicSize.begin();
	for (; i != m_vecKwicSize.end(); ++i)
	{
		s->setElement((*i).first,
					  new Common::UnsignedIntegerData(
						  (*i).second * m_uiKwicMarginScaleFactor));
	}
	cValue_.pushBack(s);

	//
	//	UNAのパラメータ
	//
	cKey_.pushBack(new Common::StringData(_Property::_cUnaParameterKey));
	cKey_.pushBack(new Common::StringData(_Property::_cUnaParameterValue));
	Common::DataArrayData* pUnaParameterKey = new Common::DataArrayData;
	Common::DataArrayData* pUnaParameterValue = new Common::DataArrayData;
	
	// リソース番号
	pUnaParameterKey->pushBack(new Common::StringData(_Property::_cUnaRscID));
	pUnaParameterValue->pushBack(new Common::UnsignedIntegerData(
									 m_cFileID.getResourceID()));
	// デフォルト言語
	pUnaParameterKey->pushBack(new Common::StringData(
								   _Property::_cDefaultLanguageSet));
	pUnaParameterValue->pushBack(new Common::LanguageData(
									 m_cFileID.getDefaultLanguage()));
	// UNAのパラメータ
	UNA::ModNlpAnalyzer::Parameters param;
	makeUnaParameter(param);
	UNA::ModNlpAnalyzer::Parameters::Iterator j = param.begin();
	for (; j != param.end(); ++j)
	{
		pUnaParameterKey->pushBack(new Common::StringData((*j).first));
		pUnaParameterValue->pushBack(new Common::StringData((*j).second));
	}
	cValue_.pushBack(pUnaParameterKey);
	cValue_.pushBack(pUnaParameterValue);
}

//
//	FUNCTION public
//	FullText2::FullTextFile::attach -- ファイルを attach する
//
//	NOTES
//
//	ARGUMENTS
//	bool bVector_ (defualt false)
//		ベクターのみかどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::attach(bool bVector_)
{
	try
	{
		// ベクターファイルを attach する
		attachVector();
		
		if (bVector_ != true)
		{
			// セクションファイルを attach する
			attachSection();
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		detach();
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	FullText2::FullTextFile::detach -- すべてのファイルを detach する
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
FullTextFile::detach()
{
	if (m_pRowIDVectorFile)
		delete m_pRowIDVectorFile, m_pRowIDVectorFile = 0;
	if (m_pDocIDVectorFile)
		delete m_pDocIDVectorFile, m_pDocIDVectorFile = 0;
	
	ModVector<InvertedSection*>::Iterator i = m_vecpSection.begin();
	for (; i != m_vecpSection.end(); ++i)
	{
		if (*i) delete *i;
	}
	m_vecpSection.clear();
	InvertedFile::clearSubFile();
}

//
//	FUNCTION public
//	FullText2::FullTextFile::isAttached -- ファイルが attach されているか否か
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		ファイルがアタッチされている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FullTextFile::isAttached()
{
	return (m_pRowIDVectorFile != 0);
}

//
//	FUNCTION public
//	FullText2::FullTextFile::isNoVersion
//		-- 版管理を利用しないトランザクションかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		版管理を使用しないトランザクションの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FullTextFile::isNoVersion()
{
	bool r = true;
	if (m_pTransaction && m_pTransaction->isNoVersion() == false)
		r = false;
	return r;
}

//
//	FUNCTION public
//	FullText2::FullTextFile::flushAllPages
//		-- すべてのページの変更を確定する
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
FullTextFile::flushAllPages()
{
	// まずは、ページを保持しているかも知れないオブジェクトを破棄する
	delete m_pQuery, m_pQuery = 0;
	delete m_pResultSet, m_pResultSet = 0;
	
	clearListManager();

	// 検索前処理はやり直す必要あり
	m_bPrepare = false;
	
	// 基底クラスを呼び出す
	InvertedFile::flushAllPages();
}

//
//	FUNCTION public
//	FullText2::FullTextFile::recoverAllPages
//		-- すべてのページの変更を破棄する
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
FullTextFile::recoverAllPages()
{
	// まずは、ページを保持しているかも知れないオブジェクトを破棄する
	delete m_pQuery, m_pQuery = 0;
	delete m_pResultSet, m_pResultSet = 0;
	
	clearListManager();
	
	// 検索前処理はやり直す必要あり
	m_bPrepare = false;
	
	// 基底クラスを呼び出す
	InvertedFile::recoverAllPages();
}

//
//	FUNCTION public
//	FullText2::FullTextFile::openForMerge
//		-- マージのためにオープンする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	nt iElement_
//		要素番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::openForMerge(const Trans::Transaction& cTransaction_,
						   int iElement_)
{
	// トランザクションを保存する
	m_pTransaction = &cTransaction_;
	// マージ対象のセクションの要素番号を保存する
	m_iMergeElement = iElement_;
	
	// ラッチする
	Trans::AutoLatch latch(const_cast<Trans::Transaction&>(*m_pTransaction),
						   m_cFileID.getLockName());

	try
	{
		// ベクターと該当する要素番号のセクションのみattachする
		attachVector();
		attachSection(iElement_);

		// 通常のopenでは、トランザクションを保存したりするが、
		// マージの場合には、不要なので、余計なことはしない
		
		m_vecpSection[iElement_]->openForMerge(cTransaction_);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		detach();
		m_pTransaction = 0;
		m_iMergeElement = -1;
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	FullText2::FullTextFile::closeForMerge
//		-- マージのためのクローズ
//
//	NOTES
//
//	ARGUMENTS
//	bool success_
//
//	RETURN
//	なし
//
//	EXCETPIONS
//
void
FullTextFile::closeForMerge(bool success_)
{
	// ラッチする
	Trans::AutoLatch latch(const_cast<Trans::Transaction&>(*m_pTransaction),
						   m_cFileID.getLockName());

	try
	{
		m_vecpSection[m_iMergeElement]->closeForMerge(success_);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		m_iMergeElement = -1;
		unsetCoder();
		detach();
		_SYDNEY_RETHROW;
	}
	m_iMergeElement = -1;
	unsetCoder();
	detach();
}

//
//	FUNCTION public
//	FullText2::FullTextFile::mergeList
//		-- １つの転置リストをマージする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		未処理の転置リストがある場合にはtrue、それ以外の場合にはfalse
//
//	EXCEPTIONS
//
bool
FullTextFile::mergeList()
{
	// ラッチする
	Trans::AutoLatch latch(const_cast<Trans::Transaction&>(*m_pTransaction),
						   m_cFileID.getLockName());

	return m_vecpSection[m_iMergeElement]->mergeList();
}

//
//	FUNCTION public
//	FullText2::FullTextFile::getIdCoder
//		-- 文書ID用の圧縮器を得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		索引単位
//
//	RETURN
//	ModInvertedCoder*
//		圧縮器
//
//	EXCEPTIONS
//
ModInvertedCoder*
FullTextFile::getIdCoder(const ModUnicodeString& cstrKey_)
{
	setCoder();
	return (cstrKey_.getLength() == 0) ? m_pWordIdCoder : m_pIdCoder;
}

//
//	FUNCTION public
//	FullText2::FullTextFile::getFrequencyCoder
//		-- 頻度用の圧縮器を得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		索引単位
//
//	RETURN
//	ModInvertedCoder*
//		圧縮器
//
//	EXCEPTIONS
//
ModInvertedCoder*
FullTextFile::getFrequencyCoder(const ModUnicodeString& cstrKey_)
{
	setCoder();
	return (cstrKey_.getLength() == 0) ?
		m_pWordFrequencyCoder : m_pFrequencyCoder;
}

//
//	FUNCTION public
//	FullText2::FullTextFile::getLengthCoder
//		-- データ長用の圧縮器を得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		索引単位
//
//	RETURN
//	ModInvertedCoder*
//		圧縮器
//
//	EXCEPTIONS
//
ModInvertedCoder*
FullTextFile::getLengthCoder(const ModUnicodeString& cstrKey_)
{
	setCoder();
	return (cstrKey_.getLength() == 0) ? m_pWordLengthCoder : m_pLengthCoder;
}

//
//	FUNCTION public
//	FullText2::FullTextFile::getLocationCoder
//		-- 出現位置用の圧縮器を得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		索引単位
//
//	RETURN
//	ModInvertedCoder*
//		圧縮器
//
//	EXCEPTIONS
//
ModInvertedCoder*
FullTextFile::getLocationCoder(const ModUnicodeString& cstrKey_)
{
	setCoder();
	return (cstrKey_.getLength() == 0) ?
		m_pWordLocationCoder : m_pLocationCoder;
}

//
//	FUNCTION public
//	FullText2::FullTextFile::getTokenizer -- トークナイザーを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::Tokenizer::AutoPointer
//		トークナイザー
//
//	EXCEPTIONS
//
Tokenizer::AutoPointer
FullTextFile::getTokenizer()
{
	//【注意】
	//
	//	得られたトークナイザーはMT-safeではない

	Os::AutoCriticalSection cAuto(m_cLatch);
	
	if (m_vecpTokenizer.getSize() == 0)
	{
		ModAutoPointer<UNA::ModNlpAnalyzer> p;

		if (m_cFileID.isNormalized() == true ||
			m_cFileID.getIndexingType() != IndexingType::Ngram ||
			m_cFileID.isClustering() == true)
		{
			// UNAのアナライザーを得る
			p = Utility::Una::Manager::getModNlpAnalyzer(
				m_cFileID.getResourceID());

			// 動作を設定する
			UNA::ModNlpAnalyzer::Parameters param;
			makeUnaParameter(param);
			p->prepare(param);
		}

		// トークナイザーを作成する
		m_vecpTokenizer.pushBack(
			Tokenizer::createTokenizer(*this,
									   p.release(),
									   m_cFileID.getTokenizeParameter()));

	}

	Tokenizer::AutoPointer p = m_vecpTokenizer.getFront();
	m_vecpTokenizer.popFront();

	return p;
}

//
//	FUNCTION public
//	FullText2::FullTextFile::pushTokenizer -- 不要になったトークナイザ—を戻す
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::Tokenizer* p_
//		不要になったトークナイザ—
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::pushTokenizer(Tokenizer* p_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	m_vecpTokenizer.pushBack(p_);
}

//
//	FUNCTION public
//	FullText2::FullTextFile::getTerm -- 質問処理器を得る
//
// 	NOTES
//	得られたインスタンスは呼び出し側で解放すること
//
//	ARGUEMNTS
//	const ModUnicodeString& cExtractor_
//		質問処理器のパラメータ
//
//	RETURN
//	Utility::ModTerm*
//		質問処理器
//
//	EXCEPTIONS
//
Utility::ModTerm*
FullTextFile::getTerm(const ModUnicodeString& cExtractor_)
{
	const ModUnicodeChar* p = cExtractor_;
	if (cExtractor_.getLength() == 0)
		p = m_cFileID.getExtractor();

	ModSize id = 0;
	ModSize unaid = 0;

	// @TERMRSCID:?
	const ModUnicodeChar* n = ModUnicodeCharTrait::find(p, _Term::_cTERM);
	if (n)
	{
		n += _Term::_cTERM.getLength();
		id = ModUnicodeCharTrait::toUInt(n);
	}

	// @UNARSCID:?
	n = ModUnicodeCharTrait::find(p, _Term::_cUNA);
	if (n)
	{
		n += _Term::_cUNA.getLength();
		unaid = ModUnicodeCharTrait::toUInt(n);
	}
	else
	{
		unaid = m_cFileID.getResourceID();
	}

	// UNAのアナライザーを得る
	ModAutoPointer<UNA::ModNlpAnalyzer> anal
		= Utility::Una::Manager::getModNlpAnalyzer(unaid);

	// 動作を設定する
	UNA::ModNlpAnalyzer::Parameters param;
	makeUnaParameter(param);

	// 次に質問処理器のリソースを得る
	const Utility::ModTermResource* resource
		= Utility::TermResourceManager::get(id);


	// 質問処理器を作成する
	return new Utility::ModTerm(resource, anal.release(), param);
}

//
//	FUNCTION public
//	FullText2::FullTextFile::getTermResource -- 質問処理器のリソースを得る
//
// 	NOTES
//
//	ARGUEMNTS
//	なし
//
//	RETURN
//	const Utility::ModTermResource*
//		質問処理器のリソース
//
//	EXCEPTIONS
//
const Utility::ModTermResource*
FullTextFile::getTermResource()
{
	const ModUnicodeChar* p = m_cFileID.getExtractor();

	ModSize id = 0;
	ModSize unaid = 0;

	// @TERMRSCID:?
	const ModUnicodeChar* n = ModUnicodeCharTrait::find(p, _Term::_cTERM);
	if (n)
	{
		n += _Term::_cTERM.getLength();
		id = ModUnicodeCharTrait::toUInt(n);
	}

	// 質問処理器のリソースを返す
	return Utility::TermResourceManager::get(id);
}

//
//	FUNCTION public
//	FullText2::FullTextFile::addBatchSize
//		-- バッチ用のメモリを消費したサイズを加算する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiSize_
//		消費したサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::addBatchSize(ModSize uiSize_)
{
	m_lBatchSize += static_cast<ModInt64>(uiSize_);
}

//
//	FUNCTION public
//	FullText2::FullTextFile::getKeyNumber -- 処理するキー番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		キー番号。それ以上存在しない場合は -1 を返す
//
//	EXCEPTIONS
//
int
FullTextFile::getKeyNumber()
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	int key = m_iNextKeyNumber;
	if (key >= m_cFileID.getKeyCount())
		return -1;
	
	++m_iNextKeyNumber;
	
	return key;
}

//
//	FUNCTION public
//	FullText2::FullTextFile::pushSuccessKeyNumber
//		-- 更新が成功したキー番号を設定する
//
//	NOTES
//
//	ARGUMENTS
//	int key_
//		成功したキー番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::pushSuccessKeyNumber(int key_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	m_vecSuccessKeyNumber.pushBack(key_);
}

//
//	FUNCTION public
//	FullText2::FullTextFile::convertData
//		-- 転置ファイルで扱えるデータに変換する
//
//	NOTES
//	変換できるのは挿入データとキーデータである
//	更新のバリューデータは変換できない
//
//	ARGUMETNS
//	int n_
//		要素番号
//	const Common::DataArrayData& cTuple_
//		変換元のデータ
//	ModVector<ModUnicodeString>& vecDocument_
//		変換後の文書データ
//	ModVector<ModLanguageSet>& vecLanguage_
//		変換後の言語データ
//	double& dblScore_
//		変換後のスコア調整値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::convertData(int n_, const Common::DataArrayData& cTuple_,
						  ModVector<ModUnicodeString>& vecDocument_,
						  ModVector<ModLanguageSet>& vecLanguage_,
						  double& dblScore_)
{
	//	0: String or StringArray		文字列
	//	1: Language or LanguageArray	言語情報
	//	2: Double						スコア調整

	const Common::Data* pStringData = 0;
	const Common::Data* pLanguageData = 0;
	const Common::Data* pScoreData = 0;

	{
#define _CHECK_COUNT(size)	\
		if (cTuple_.getCount() <= size) _SYDNEY_THROW0(Exception::BadArgument)
	
		// cTuple_ からの要素取得は MT-safe ではないので、
		// ここでまとめて行う
		
		Os::AutoCriticalSection cAuto(m_cLatch);

		int c = m_cFileID.getKeyCount();
	
		// 文字列
		_CHECK_COUNT(n_);
		pStringData = cTuple_.getElement(n_).get();
	
		if (m_cFileID.isLanguage())
		{
			// 言語データ
			_CHECK_COUNT(c);
			pLanguageData = cTuple_.getElement(c).get();
			++c;
		}

		if (m_cFileID.isScoreField())
		{
			// スコア調整値
			_CHECK_COUNT(c);
			pScoreData = cTuple_.getElement(c).get();
			++c;
		}
		
#undef _CHECK_COUNT
	}
	
	// 文書データ
	convertString(pStringData, vecDocument_);

	if (pLanguageData)
	{
		// 言語データ
		convertLanguage(pLanguageData, vecLanguage_);
	}
	if (vecLanguage_.getSize() == 0)
	{
		// 言語列がない or 言語列が null の場合にも
		// デフォルトの言語を1つは設定する

		vecLanguage_.pushBack(
			ModLanguageSet(m_cFileID.getDefaultLanguage()));
	}

	if (pScoreData)
	{
		// スコア調整値
		convertScore(pScoreData, dblScore_);
	}
}

//
//	FUNCTION private
//	FullText2::FullTextFile::openForSearch
//		-- 検索のためにファイルをオープンする
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::openForSearch(OpenOption& cOpenOption_)
{
	// 初期化する
	m_bFirst = true;
	
	// 最低限必要なベクターファイルのみattachする
	attach(true);

	try
	{
		_AutoDetachPage cAuto(this);
	
		// コスト見積りに来るかもしれないので、検索語数を設定しておく
		m_iTermCount = cOpenOption_.getTermCount();
		
		if (cOpenOption_.isEstimate())
		{
			// コスト取得モードの場合は検索はしない
			
		} 	
		else if (cOpenOption_.getSearchType() == OpenOption::Type::Equal)
		{
			// 整合性検査のための検索
			//
			// 整合性検査のための検索はROWIDしか取得しない
			// FileID::getProjectionParameter でチェック済み
			// よって、ここで検索まで実行してしまう

			m_bEqual = true;
			m_bPrepare = true;

			// すべてのファイルを attach する
			attachSection();

			// 必要なデータをOpenOptionから取得する
			ModVector<ModUnicodeString> vecDoc
				= cOpenOption_.getSectionValue();
			ModVector<ModLanguageSet> vecLang
				= cOpenOption_.getSectionLanguage();
			ModUInt32 uiRowID = cOpenOption_.getRowID();

			m_uiRowID = Os::Limits<ModUInt32>::getMax();
		
			// ROWID -> 文書IDへ変換する
			DocumentID uiDocumentID;
			if (m_pRowIDVectorFile->get(uiRowID, uiDocumentID) == false)
			{
				// 存在しない
				return;
			}

			// 検索する
			//
			// 複合索引の場合はどうする？
			//
			if (m_vecpSection[0]->check(vecDoc, vecLang, uiDocumentID) == false)
			{
				// 存在しない
				return;
			}

			// 存在する
			m_uiRowID = uiRowID;
		}
		else
		{
			// ソート条件を設定する
			setSortParameter(cOpenOption_);

			// 取得するフィールドを設定する
			setProjectionParameter(cOpenOption_);
		
			// 検索に必要な情報を取得する
			saveSearchInfo(cOpenOption_);
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object)
#else
	catch (...)
#endif
	{
		close();
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION private
//	FullText2::FullTextFile::openForRead
//		-- 固定値の取得のためにオープンする
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::openForRead(OpenOption& cOpenOption_)
{
	// 初期化する
	m_bFirst = true;
	
	// 最低限必要なベクターファイルのみattachする
	attach(true);

	// 取得するフィールドを設定する
	setProjectionParameter(cOpenOption_);
}

//
//	FUNCTION private
//	FullText2::FullTextFile::openForUpdate
//		-- 更新のためにオープンする
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::openForUpdate(OpenOption& cOpenOption_)
{
	// セクションをattachすると内部でページが読み込まれるので、
	// ページのdetachが必要
	
	// すべてのセクションをattachする
	attach();
	
	// 更新パラメータを設定する
	setUpdateParameter(cOpenOption_);
}

//
//	FUNCTION private
//	FullText2::FullTextFile::saveSearchInfo
//		-- 検索に必要な情報を保存する
//
//	NOTES
//
//	ARGUMENTS
// 	FullText2::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::saveSearchInfo(OpenOption& cOpenOption_)
{
	// 検索条件
	m_cstrCondition = cOpenOption_.getCondition();
	if (m_cstrCondition.getLength() == 0)
	{
		// 検索条件がないのはありえない
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	// 検索タイプ
	m_eSearchType = cOpenOption_.getSearchType();

	if (m_bGetWord &&
		m_eSearchType != OpenOption::Type::FreeText)
	{
		// 単語取得はfreetextまたはwordlistが必要
		_TRMEISTER_THROW0(Exception::NotSupported);
	}
	
	// ビットセットによる絞り込み
	const Common::BitSet* pBitSet = cOpenOption_.getBitSetForNarrowing();
	if (pBitSet)
	{
		//【注意】
		//	マルチスレットで検索するためにはランダムアクセスする必要がある
		//	そのため、BitSet ではなく、配列にしている
		
		m_pNarrowing = new Common::LargeVector<DocumentID>();
		m_pNarrowing->reserve(pBitSet->count());

		if (isMounted(*m_pTransaction))
		{
			Common::BitSet::ConstIterator i = pBitSet->begin();
			Common::BitSet::ConstIterator e = pBitSet->end();
			for (; i != e; ++i)
			{
				// このビットセットはROWIDの集合なので、
				// 文書IDに変換する

				ModUInt32 uiRowID = *i;
				DocumentID uiDocID = 0;

				if (m_pRowIDVectorFile->get(uiRowID, uiDocID) == false)
				{
					// 存在しないので、次へ
					continue;
				}

				m_pNarrowing->pushBack(uiDocID);
			}

			// 昇順にソートする
			ModSort(m_pNarrowing->begin(), m_pNarrowing->end(),
					ModLess<DocumentID>());
		}
	}

	// ビットセットで絞った集合での検索
	pBitSet = cOpenOption_.getBitSetForRanking();
	m_uiRankingDocumentCount = 0;
	if (pBitSet)
	{
		m_pRanking = new Common::BitSet();

		if (isMounted(*m_pTransaction))
		{
			Common::BitSet::ConstIterator i = pBitSet->begin();
			Common::BitSet::ConstIterator e = pBitSet->end();
			for (; i != e; ++i)
			{
				// このビットセットはROWIDの集合なので、
				// 文書IDに変換する

				ModUInt32 uiRowID = *i;
				DocumentID uiDocID = 0;

				if (m_pRowIDVectorFile->get(uiRowID, uiDocID) == false)
				{
					// 存在しないので、次へ
					continue;
				}

				m_pRanking->set(uiDocID);
				++m_uiRankingDocumentCount;
			}
		}

		// 最後の要素として UndefinedDocumentID を加える
		m_pRanking->set(UndefinedDocumentID);
	}
}

//
//	FUNCTION private
//	FullText2::FullTextFile::prepareSearch
//		-- 検索の準備を行う
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
FullTextFile::prepareSearch()
{
	if (m_bPrepare == false)
	{
		// 検索クラスを得る
		const ModUnicodeChar* pTea = m_cstrCondition;
		m_pQuery = makeQuery(isScore(), isTfList(), isLocation(), pTea);

		// パースする
		const ModUnicodeChar* p2 = pTea;
		SearchInformation& cSearchInfo = getSearchInformation();
		m_pQuery->parse(cSearchInfo, m_vecpListManager, pTea);

		if (m_bGetWord == false)
		{
			// 検索結果クラスのインスタンスを得る
			m_pResultSet
				= new ResultSet(m_vecGetField, cSearchInfo, m_bGetByBitSet);
		}

		// 位置情報が必要かどうか
		if (m_bLocation)
		{
			if (m_eSearchType == OpenOption::Type::FreeText)
			{
				// 位置情報を取得するために、検索条件クラスから
				// 検索文字列を得る必要がある

				p2 = m_pQuery->getConditionForLocation();
			}

			// 位置情報を取得するためにもう一度パースする

			ModVector<int>::Iterator i = m_vecLocationField.begin();
			for (; i != m_vecLocationField.end(); ++i)
			{
				const ModUnicodeChar* p3 = p2;
				
				m_pResultSet->setTermLeafNode(
					*i, m_pQuery->parseForLocation(
						cSearchInfo, m_vecpListManager[*i], p3));
			}

		}

		if (m_bTfList)
		{
			// TF値を取得するので、必要な情報を設定する

			if (m_pQuery->getTermNodeForTfList().getSize() == 0)
			{
				// wordlist がないのでエラー
				_SYDNEY_THROW0(Exception::NotSupported);
			}
		
			m_pResultSet->setTfListNode(m_pQuery->getTermNodeForTfList());
		}
	
		if (m_bCluster)
		{
			// 現時点(2010/09/08)では内積値合成方法は平均のみ
		
			m_pResultSet->setClusterParameter(
				m_fClusteredLimit,
				ResultSet::ClusterCombiner::Ave,
				m_vecClusterScale);
		}

		if (m_vecKwicSize.getSize())
		{
			// 粗いKWICのサイズを設定する

			ModVector<ModPair<int, ModSize> >::Iterator i
				= m_vecKwicSize.begin();
			for (; i != m_vecKwicSize.end(); ++i)
			{
				m_pResultSet->setKwicParameter((*i).first, (*i).second,
											   m_uiKwicMarginScaleFactor);
			}
		}

		m_bPrepare = true;
	}
}

//
//	FUNCTION private
//	FullText2::FullTextFile::makeQuery
//		-- 検索クラスを用意する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::Query*
//		検索クラス
//
//	EXCEPTIONS
//
Query*
FullTextFile::makeQuery(bool bScore_, bool bTfList_, bool bLocation_,
						const ModUnicodeChar*& pTea_)
{
	// 以下のパターンがある
	// #contains[single,<field number>,
	//			 <average length>,<document frequency>,...](...)
	// #contains[cat,[<field number>,...],[<scale>,...],
	//			 <average length>,<document frequency>,...](...)
	// #contains[(and|or),<combiner>,[<field number>,...],
	//			 [<scale>,...],[<average length>,...],
	//			 [<document frequency>,...],...](...)
	//
	// 最後の ... は以下の通り
	//		<calculator>,<combiner>,<expand limit>,
	//		<extractor>,<score function>,<clustered limit>

	if (_cContains.compare(pTea_, _cContains.getLength()) != 0)
		_TRMEISTER_THROW1(Exception::SQLSyntaxError, pTea_);
	pTea_ += _cContains.getLength();
	
	if (*pTea_ == '[')
		++pTea_;		// '[' の分
	else
		_TRMEISTER_THROW1(Exception::SQLSyntaxError, pTea_);

	//
	//	スコアの合成方法を求める
	//
	ModUnicodeString cScoreCombiner;
	bool bCombiner = false;
	
	if (_cSingle.compare(pTea_, _cSingle.getLength()) == 0)
	{
		pTea_ += _cSingle.getLength();
		m_eCombineMethod = Query::CombineMethod::None;
	}
	else if (_cCat.compare(pTea_, _cCat.getLength()) == 0)
	{
		pTea_ += _cCat.getLength();
		m_eCombineMethod = Query::CombineMethod::Tf;
	}
	else if (_cOr.compare(pTea_, _cOr.getLength()) == 0)
	{
		pTea_ += _cOr.getLength();
		m_eCombineMethod = Query::CombineMethod::ScoreOr;

		// 合成方法を得る
		bCombiner = true;
	}		
	else if (_cAnd.compare(pTea_, _cAnd.getLength()) == 0)
	{
		pTea_ += _cAnd.getLength();
		m_eCombineMethod = Query::CombineMethod::ScoreAnd;
		
		// 合成方法を得る
		bCombiner = true;
	}
	else
		_TRMEISTER_THROW1(Exception::SQLSyntaxError, pTea_);
	
	if (*pTea_ == ',')
		++pTea_;		// ',' の分
	else
		_TRMEISTER_THROW1(Exception::SQLSyntaxError, pTea_);

	if (bCombiner)
	{
		Query::getToken(cScoreCombiner, pTea_);
		if (OpenOption::checkScoreCombiner(cScoreCombiner) == false)
			_TRMEISTER_THROW1(Exception::SQLSyntaxError, cScoreCombiner);
	}

	// フィールド番号
	ModVector<int> vecField;
	parseField(vecField, pTea_);

	// スケールを得る
	ModVector<Query::ScaleData> vecScale;
	parseScale(vecField, vecScale, pTea_);
	
	// 検索条件クラスのインスタンスを得る
	ModAutoPointer<Query> pQuery = new Query(bScore_, bTfList_, bLocation_);

	// スコアの合成方法を設定する
	pQuery->setCombineParameter(m_eCombineMethod,
								cScoreCombiner,
								vecScale);

	// 検索情報クラスを得る
	SearchInformation& cSearchInfo = getSearchInformation();

	// 平均文書長をセットする
	setAverageLength(vecField, cSearchInfo, pTea_);
	// 総文書数をセットする
	setDocumentFrequency(vecField, cSearchInfo, pTea_);

	//	パラメータ番号とパラメータの関係
	//
	//	0		calculator
	//	1		combiner
	//	2		expand limit
	//	3		extractor
	//	4		score function
	//	5		clustered limit

	int n = 0;
	ModUnicodeString s;

	// 拡張語の上限 expand limit
	ModSize maxTerm = 0;
	// 抽出指定 extractor
	ModUnicodeString cExtractor;
	
	while (*pTea_ != '(')
	{
		Query::getToken(s, pTea_);

		switch (n)
		{
		case 0:
			// スコア計算器
			pQuery->setScoreCalculator(s);
			break;
		case 1:
			// スコア合成器
			pQuery->setScoreCombiner(s);
			break;
		case 2:
			// 拡張語数の上限
			if (s.getLength())
				maxTerm  = ModUnicodeCharTrait::toUInt(s);
			break;
		case 3:
			// 抽出パラメータ
			cExtractor = s;
			break;
		case 4:
			// スコア関数
			if (s.getLength())
				m_eAdjustMethod = OpenOption::getAdjustMethod(s);
			break;
		case 5:
			// クラスターの閾値
			if (s.getLength())
				m_fClusteredLimit = ModUnicodeCharTrait::toFloat(s);
			break;
		}

		++n;
	}

	if (m_eSearchType == OpenOption::Type::FreeText)
	{
		// 質問処理器を得る
		Utility::ModTerm* pTerm = getTerm(cExtractor);

		if (m_bGetWord)
		{
			// 単語取得なので、Limitを設定する
			if (m_uiLimit != Os::Limits<ModSize>::getMax())
				pTerm->maxTerm1
					= ModMax(m_uiLimit + m_uiOffset, pTerm->maxTerm1);
			if (maxTerm != 0)
				pTerm->maxTerm2	= maxTerm;
		}

		// 検索クラスに設定する
		pQuery->setTerm(pTerm);
	}

	if (m_vecExpandDocument.getSize())
	{
		// 適合性フィードバックのデータを設定する

		pQuery->setExpandDocument(m_vecExpandDocument,
								  m_vecExpandLanguage);
	}

	// リスト管理クラスを得る
	setListManager();

	return pQuery.release();
}

//
//	FUNCTION private
//	FullText2::FullTextFile::setListManager
//		-- リスト管理クラスを設定する
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
FullTextFile::setListManager()
{
	if (m_vecpListManager.getSize() == 0 && m_vecpSection.getSize() != 0)
	{
		// リスト管理クラスを設定する

		m_vecpListManager.reserve(m_vecpSection.getSize());
		ModVector<InvertedSection*>::Iterator i = m_vecpSection.begin();
		for (; i != m_vecpSection.end(); ++i)
		{
			// 利用するセクションの ListManager のみを設定する
			
			ListManager* p = 0;
			
			if (*i != 0)
			{
				p = (*i)->getListManager();
				
				if (m_pRanking)
				{
					// 絞った集合でのランキング検索
				
					p = new ListManagerWithWhiteList(*this, p, *m_pRanking);
				}
			}
			
			m_vecpListManager.pushBack(p);
		}
	}
}

//
//	FUNCTION private
//	FullText2::FullTextFile::clearListManager
//		-- リスト管理クラスをクリアする
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
FullTextFile::clearListManager()
{
	ModVector<ListManager*>::Iterator i = m_vecpListManager.begin();
	for (; i != m_vecpListManager.end(); ++i)
	{
		if (*i != 0) delete (*i);
	}
	m_vecpListManager.clear();
}

//
//	FUNCTION private
//	FullText2::FullTextFile::attachVector -- ベクターファイルをattachする
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
FullTextFile::attachVector()
{
	Os::Path path;

	// RowID -> 文書ID を attach する
	path = m_cPath;
	path.addPart(_Path::_cRowID);
	m_pRowIDVectorFile = new IDVectorFile(m_cFileID, path, m_bBatch);
	if (m_pTransaction) m_pRowIDVectorFile->open(*m_pTransaction, m_eFixMode);
	InvertedFile::pushBackSubFile(m_pRowIDVectorFile);

	// 文書ID -> RowID を attach する
	path = m_cPath;
	path.addPart(_Path::_cDocID);
	m_pDocIDVectorFile = new IDVectorFile(m_cFileID, path, m_bBatch);
	if (m_pTransaction) m_pDocIDVectorFile->open(*m_pTransaction, m_eFixMode);
	InvertedFile::pushBackSubFile(m_pDocIDVectorFile);
}

//
//	FUNCTION private
//	FullText::FullTextFile::attachSection
//		-- 転置ファイルセクションを attach する
//
//	NOTES
//
//	ARGUMENTS
//	int n_
//		attach する転置ファイルセクションの要素番号
//		最大キーより大きな数の場合には、すべてのセクションをattachする
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::attachSection(int n_)
{
	int keyCount = m_cFileID.getKeyCount();

	if (m_vecpSection.getSize() == 0)
	{
		m_vecpSection.assign(keyCount, 0);
	}

	if (n_ >= keyCount)
	{
		// すべての要素を attach する

		Os::Path path;

		for (int i = 0; i < keyCount; ++i)
		{
			if (m_vecpSection[i])
				// すでに attach されている
				continue;
			
			path = m_cPath;
			ModUnicodeOstrStream s;
			s << _Path::_cSection << i;
			path.addPart(s.getString());

			ModAutoPointer<InvertedSection> p
				= new InvertedSection(*this, path, m_bBatch);
			if (m_pTransaction) p->open(*m_pTransaction, m_eFixMode);
			InvertedFile::pushBackSubFile(p.get());
			m_vecpSection[i] = p.release();
		}
	}
	else
	{
		// 特定の要素のみを attach する
		
		if (m_vecpSection[n_] == 0)
		{
			// まだ attach されていない
		
			Os::Path path = m_cPath;
			ModUnicodeOstrStream s;
			s << _Path::_cSection << n_;
			path.addPart(s.getString());
		
			ModAutoPointer<InvertedSection> p
				= new InvertedSection(*this, path, m_bBatch);
			if (m_pTransaction) p->open(*m_pTransaction, m_eFixMode);
			InvertedFile::pushBackSubFile(p.get());
			m_vecpSection[n_] = p.release();
		}
	}
}

//
//  FUNCTION private
// 	FullText2::FullTextFile::setCoder -- 圧縮器を設定する
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
FullTextFile::setCoder()
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	
	if (m_pIdCoder == 0)
	{
		m_pIdCoder
			= ModInvertedCoder::create(m_cFileID.getIdCoder());
		m_pFrequencyCoder
			= ModInvertedCoder::create(m_cFileID.getFrequencyCoder());
		m_pLengthCoder
			= ModInvertedCoder::create(m_cFileID.getLengthCoder());
		m_pLocationCoder
			= ModInvertedCoder::create(m_cFileID.getLocationCoder());
		m_pWordIdCoder
			= ModInvertedCoder::create(m_cFileID.getWordIdCoder());
		m_pWordFrequencyCoder
			= ModInvertedCoder::create(m_cFileID.getWordFrequencyCoder());
		m_pWordLengthCoder
			= ModInvertedCoder::create(m_cFileID.getWordLengthCoder());
		m_pWordLocationCoder
			= ModInvertedCoder::create(m_cFileID.getWordLocationCoder());
	}
}

//
//  FUNCTION private
//  FullText2::FullTextFile::unsetCoder -- 圧縮器を破棄する
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
FullTextFile::unsetCoder()
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	
	delete m_pIdCoder, m_pIdCoder = 0;
	delete m_pFrequencyCoder, m_pFrequencyCoder = 0;
	delete m_pLengthCoder, m_pLengthCoder = 0;
	delete m_pLocationCoder, m_pLocationCoder = 0;
	delete m_pWordIdCoder, m_pWordIdCoder = 0;
	delete m_pWordFrequencyCoder, m_pWordFrequencyCoder = 0;
	delete m_pWordLengthCoder, m_pWordLengthCoder = 0;
	delete m_pWordLocationCoder, m_pWordLocationCoder = 0;
}

//
//	FUNCTION private
//	FullText2::FullTextFile::makeUnaParameter
//		-- UNAの解析器用のパラメータを作成する
//
//	NOTES
//
//	ARGUMENTS
//	UNA::ModNlpAnalyzer::Parameters& p;
//		UNA解析器用のパラメータ
//
// 	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::makeUnaParameter(UNA::ModNlpAnalyzer::Parameters& p)
{
	ModUnicodeOstrStream s;
	s << m_cFileID.getMaxWordLength();
	p.insert(_UnaParam::_cMaxWordLen, s.getString());

	p.insert(_UnaParam::_cDoNorm,
			 m_cFileID.isNormalized() ? _UnaParam::_cTrue : _UnaParam::_cFalse);
	p.insert(_UnaParam::_cCompound, _UnaParam::_cTrue);
	p.insert(_UnaParam::_cStem,
			 m_cFileID.isStemming() ? _UnaParam::_cTrue : _UnaParam::_cFalse);
	p.insert(_UnaParam::_cCarriage,
			 m_cFileID.isCarriage() ? _UnaParam::_cTrue : _UnaParam::_cFalse);

	//
	// 空白の処理は 0 が AsIs (リソースのデフォルト) で、
	// 2 が DeleteSpace である
	//
	p.insert(_UnaParam::_cSpace,
			 m_cFileID.isDeleteSpace() ? _UnaParam::_cTwo : _UnaParam::_cZero);
}

//
//	FUNCTION private
//	FullText2::FullTextFile::pushExpandDocument -- 拡張文書を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cTuple_
//		挿入するタプルデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::pushExpandDocument(const Common::DataArrayData& cTuple_)
{
	if (cTuple_.getCount() == 0)
		_TRMEISTER_THROW0(Exception::BadArgument);

	const Common::Data* pData = cTuple_.getElement(0).get();
	if (pData->isNull())
		return;

	// 0 : 全文データ
	// 1 : 言語データ(オプション)

	ModVector<ModUnicodeString> vecDocument;
	ModVector<ModLanguageSet> vecLanguage;

	// 全文データを得る
	pData = cTuple_.getElement(0).get();
	convertString(pData, vecDocument);
	
	// 次に言語データを得る
	if (cTuple_.getCount() == 2)
	{
		const Common::Data* pLang = cTuple_.getElement(1).get();
		if (!pLang->isNull())
		{
			convertLanguage(pLang, vecLanguage);
		}
	}

	if (vecLanguage.getSize() == 0)
	{
		// デフォルト言語を追加する
		vecLanguage.pushBack(ModLanguageSet(m_cFileID.getDefaultLanguage()));
	}

	if (vecDocument.getSize() != vecLanguage.getSize() &&
		vecLanguage.getSize() != 1)
	{
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	// 適合性フィードバックを設定する
	ModVector<ModUnicodeString>::Iterator i = vecDocument.begin();
	ModVector<ModLanguageSet>::Iterator j = vecLanguage.begin();
	for (; i != vecDocument.end(); ++i)
	{
		m_vecExpandDocument.pushBack(*i);
		m_vecExpandLanguage.pushBack(*j);

		if (vecLanguage.getSize() > 1)
			++j;
	}
}

//
//	FUNCTION private
//	FullText2::FullTextFile::setProjectionParameter
//		-- 取得するフィールドを設定する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::setProjectionParameter(OpenOption& cOpenOption_)
{
	// LogicalInterface::getProjectionParameter で取得可能な組わせかどうか
	// チェックしているので、ここでは特に確認しない
	// 正しいものとして処理する

	// ビットマップで取得するかどうか
	m_bGetByBitSet = cOpenOption_.isGetByBitSet();
	
	// 検索対象のフィールド
	ModVector<int> vecSearchField;
	int n = cOpenOption_.getSearchFieldCount();
	for (int i = 0; i < n; ++i)
	{
		vecSearchField.pushBack(cOpenOption_.getSearchFieldNumber(i));
	}
	
	// キーの数
	int key = m_cFileID.getKeyCount();

	// 取得するフィールドを設定する
	int count = cOpenOption_.getProjectionFieldCount();
	for (int i = 0; i < count; ++i)
	{
		// 取得するフィールド番号はビットで表現されている
		int n = cOpenOption_.getProjectionFieldNumber(i);
		// ビット位置からフィールド番号の配列に変換する
		ModVector<int> fields = _convertField(n);
		
		OpenOption::Function::Value f = cOpenOption_.getProjectionFunction(i);

		switch (f)
		{
		case OpenOption::Function::ClusterID:
			{
				// クラスタリング
				m_bCluster = true;
				// スコアが必要
				m_bScore = true;

				if (m_vecClusterScale.getSize() == 0)
				{
					// クラスターの重みのデフォルトを設定する
					ModVector<int>::Iterator j = vecSearchField.begin();
					for (; j != vecSearchField.end(); ++j)
					{
						m_vecClusterScale.pushBack(
							ModPair<int, float>(*j, 1.0));
					}
				}

				// 検索結果クラスではフィールドは気にしない
				n = -1;
			}
			break;
		case OpenOption::Function::Score:
			// スコアが必要
			m_bScore = true;
			// 検索結果クラスではフィールドは気にしない
			n = -1;
			break;
		case OpenOption::Function::Tf:
		case OpenOption::Function::Existence:
			// TF値リストが必要
			m_bTfList = true;
			// 検索結果クラスではフィールドは気にしない
			n = -1;
			break;
		case OpenOption::Function::Word:
			// 単語取得
			m_bGetWord = true;
			// 検索結果クラスではフィールドは気にしない
			n = -1;
			break;
		case OpenOption::Function::Section:
			// 位置情報が必要
			m_bLocation = true;
			m_vecLocationField.pushBack(fields[0]);
			break;
		case OpenOption::Function::RoughKwicPosition:
			{
				// 位置情報が必要
				m_bLocation = true;
				m_vecLocationField.pushBack(fields[0]);

				if (m_uiKwicMarginScaleFactor == 0)
				{
					// スケールファクターを求める
					
					if (m_cFileID.isNormalized() == false &&
						m_cFileID.getIndexingType() == IndexingType::Ngram)
					{
						m_uiKwicMarginScaleFactor
							= _KwicMarginScaleFactor.get();
					}
					else if (m_cFileID.isRoughKwic() == true)
					{
						m_uiKwicMarginScaleFactor
							= _KwicMarginScaleFactorForNormalizing.get();
					}
					else
					{
						m_uiKwicMarginScaleFactor
							= _KwicMarginScaleFactorForNoRoughKwic.get();
					}
				}

				// KWICサイズを設定する
				ModUnicodeString arg
					= cOpenOption_.getProjectionFunctionArgument(i);
				int size = ModUnicodeCharTrait::toInt(arg);

				// KWICサイズを append する
				m_vecKwicSize.pushBack(
					ModPair<int, ModSize>(fields[0], size));
			}
			break;
		}

		if (n == -1)
		{
			fields.clear();
		}
		
		// セクションをアタッチする
		// n == -1 の場合には、prepareSearch内でアタッチされる

		ModVector<int>::Iterator j = fields.begin();
		for (; j != fields.end(); ++j)
		{
			if (*j < key)
				attachSection(*j);
		}
		m_vecGetField.pushBack(ResultSet::FieldData(fields, f));
	}
}

//
//	FUNCTION private
//	FullText2::FullTextFile::setSortParameter
//		-- ソート条件を設定する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::setSortParameter(OpenOption& cOpenOption_)
{
	//
	//【注意】
	//	GetByBitSetの場合には、OpenOptionの中でソートなしに固定されている
	//
	m_eSortParameter = cOpenOption_.getSortParameter();
	
	switch (m_eSortParameter)
	{
	case OpenOption::SortParameter::ScoreDesc:
	case OpenOption::SortParameter::ScoreAsc:
		// スコアが必要
		m_bScore = true;
		break;
	}

	//
	//【注意】
	//	GetByBitSetの場合には、OpenOptionの中で両方 1 に固定されている
	//
	m_uiLimit = cOpenOption_.getLimit();
	m_uiOffset = (cOpenOption_.getOffset() - 1);	// 1ペースなので、1を引く
}

//
//	FUNCTION private
//	FullText2::FullTextFile::setUpdateParameter
//		-- 更新条件を設定する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::setUpdateParameter(OpenOption& cOpenOption_)
{
	// 更新対象のフィールド数を得る
	int n = cOpenOption_.getUpdateFieldCount();
	
	// 更新対象のフィールドを指定する
	if (n != 0)
	{
		m_bOnlyScoreField = true;	// 一旦 true にする
		m_vecUpdateField.reserve(n);
		FieldMask mask(m_cFileID);
		
		for (int i = 0; i < n; ++i)
		{
			int field = cOpenOption_.getUpdateFieldNumber(i);
			m_vecUpdateField.pushBack(field);

			if (mask.isScoreAdjustField(field) == false)
			{
				// スコア調整フィールド以外も更新する
				m_bOnlyScoreField = false;
			}
		}
	}
}

//
//	FUNCTION private
//	FullText2::FullTextFile::getSearchInformation
//		-- 検索情報クラスを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::SearchInforamtion&
//		検索情報クラス
//
//	EXCEPTIONS
//
SearchInformation&
FullTextFile::getSearchInformation()
{
	if (m_pSearchInfo == 0)
	{
		if (m_eCombineMethod == Query::CombineMethod::None)
		{
			// 単独フィールドしか条件ではないので、
			// 最初に attach されているセクションの検索情報クラスを返す

			ModVector<InvertedSection*>::Iterator i = m_vecpSection.begin();
			for (; i != m_vecpSection.end(); ++i)
			{
				if (*i)
				{
					m_pSearchInfo = (*i)->getSearchInformation();
					break;
				}
			}
		}
		else
		{
			SearchInformationArray* tmp = 0;
			
			if (m_eCombineMethod == Query::CombineMethod::Tf)
			{
				// TF値に重みをつけて、連結文字列として検索する
				
				tmp = new SearchInformationConcatinate(m_cFileID);
			}
			else
			{
				// 別々の文字列から得られたスコア値に重みをつけて、検索する

				tmp = new SearchInformationArray(m_cFileID);
			}

			m_pSearchInfo = tmp;

			ModVector<InvertedSection*>::Iterator i = m_vecpSection.begin();
			for (; i != m_vecpSection.end(); ++i)
			{
				// attachしていないセクションも null を push する

				SearchInformation* p = 0;
				
				if (*i)
				{
					p = (*i)->getSearchInformation();
					
					if (m_uiRankingDocumentCount)
						p->setDocumentCount(m_uiRankingDocumentCount);
				}

				tmp->pushElement(p);
			}
		}

		if (m_uiRankingDocumentCount)
			m_pSearchInfo->setDocumentCount(m_uiRankingDocumentCount);
	}
	
	return *m_pSearchInfo;
}

//
//	FUNCTION private
//	FullText2::FullTextFile::getNewDocumentID
//		-- 新しい文書IDを採番して返す
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::DocumentID
//		新しい文書ID
//
//	EXCEPTIONS
//
DocumentID
FullTextFile::getNewDocumentID()
{
	DocumentID uiDocID = m_pDocIDVectorFile->getMaxKey();
	return ++uiDocID;
}

//
//	FUNCTION private
//	FullText2::FullTextFile::createVector
//		-- ROWID <-> DocIDのベクターファイルを作成する
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
FullTextFile::createVector()
{
	if (m_pRowIDVectorFile->isMounted(*m_pTransaction) == false)
		m_pRowIDVectorFile->create();
	
	if (m_pDocIDVectorFile->isMounted(*m_pTransaction) == false)
		m_pDocIDVectorFile->create();
}

//
//	FUNCTION private
//	FullText2::FullTextFile::insertVector
//		-- ROWID <-> DocIDのベクターファイルに登録する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::DocumentID uiDocID_
//		文書ID
//	ModUInt32 uiRowID_
//		ROWID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::insertVector(DocumentID uiDocID_, ModUInt32 uiRowID_)
{
	m_pRowIDVectorFile->insert(uiRowID_, uiDocID_);
	m_pDocIDVectorFile->insert(uiDocID_, uiRowID_);
}

//
//	FUNCTION private
//	FullText2::FullTextFile::expungeVector
//		-- ROWID <-> DocIDのベクターファイルから削除する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::DocumentID uiDocID_
//		文書ID
//	ModUInt32 uiRowID_
//		ROWID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::expungeVector(DocumentID uiDocID_, ModUInt32 uiRowID_)
{
	m_pRowIDVectorFile->expunge(uiRowID_);
	m_pDocIDVectorFile->expunge(uiDocID_);
}

//
//	FUNCTION private
//	FullText2::FullTextFile::getRowID -- タプルからROWIDのデータを取り出す
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cTuple_
//		タプル
//
//	RETURN
//	ModUInt32
//		ROWIDのデータ
//
//	EXCEPTIONS
//
ModUInt32
FullTextFile::getRowID(const Common::DataArrayData& cTuple_)
{
	//	0: String or StringArray		文字列
	//	1: Language or LanguageArray	言語情報
	//	2: Double						スコア調整
	//	3: ROWID						ROWID

	int c = m_cFileID.getKeyCount();
	if (m_cFileID.isLanguage()) ++c;
	if (m_cFileID.isScoreField()) ++c;
	
	// ROWID
	if (cTuple_.getCount() <= c)
		_SYDNEY_THROW0(Exception::BadArgument);

	const Common::Data* pData = cTuple_.getElement(c).get();
	
	ModUInt32 uiRowID;
	convertRowID(pData, uiRowID);

	return uiRowID;
}

//
//	FUNCTION private
//	FullText2::FullTextFile::convertString
//		-- 文書データを変換する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data* pData_
//		文書データ
//	ModVector<ModUnicodeString>& vecDocument_
//		変換後の文書データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::convertString(const Common::Data* pData_,
							ModVector<ModUnicodeString>& vecDocument_)
{
	vecDocument_.erase(vecDocument_.begin(), vecDocument_.end());
	
	if (pData_->isNull())
	{
		// 空文字列を設定する

		vecDocument_.pushBack(ModUnicodeString());
	}
	else if (pData_->getType() == Common::DataType::Array)
	{
		// 配列
		const Common::DataArrayData& v
			= _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&, *pData_);
		
		vecDocument_.reserve(v.getCount());

		bool isnull = true;
		
		for (int i = 0; i < v.getCount(); ++i)
		{
			const Common::Data& data = *v.getElement(i);
			if (data.isNull())
			{
				// 要素が null ということはあり得る
				vecDocument_.pushBack(ModUnicodeString());
			}
			else
			{
				; _CHECK_TYPE(data, String);
				const Common::StringData& e
					= _SYDNEY_DYNAMIC_CAST(const Common::StringData&, data);
				vecDocument_.pushBack(e.getValue());
				isnull = false;
			}
		}

		if (isnull == true)
		{
			// すべての要素が null なのでエラー
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
	else
	{
		; _CHECK_TYPE(*pData_, String);
		
		// スカラー

		const Common::StringData& v
			= _SYDNEY_DYNAMIC_CAST(const Common::StringData&, *pData_);

		vecDocument_.pushBack(v.getValue());
	}
}

//
//	FUNCTION private
//	FullText2::FullTextFile::convertLanguage
//		-- 言語データを変換する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data* pData_
//		言語データ
//	ModVector<ModLanguageSet>& vecLangauge_
//		変換後の言語データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::convertLanguage(const Common::Data* pData_,
							  ModVector<ModLanguageSet>& vecLanguage_)
{
	vecLanguage_.erase(vecLanguage_.begin(), vecLanguage_.end());

	if (pData_->isNull())
		return;
	
	if (pData_->getType() == Common::DataType::Array)
	{
		// 配列
		const Common::DataArrayData& v
			= _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&, *pData_);
		
		vecLanguage_.reserve(v.getCount());
		
		for (int i = 0; i < v.getCount(); ++i)
		{
			const Common::Data& data = *v.getElement(i);

			if (data.isNull())
			{
				// NULLの場合は空を割り当てる
				vecLanguage_.pushBack(ModLanguageSet());
			}
			else if (data.getType() == Common::DataType::Language)
			{
				// 言語情報
				const Common::LanguageData& e
					= _SYDNEY_DYNAMIC_CAST(const Common::LanguageData&, data);
				vecLanguage_.pushBack(e.getValue());
			}
			else if (data.getType() == Common::DataType::String)
			{
				// 文字列で与えられた
				const Common::StringData& e
					= _SYDNEY_DYNAMIC_CAST(const Common::StringData&, data);
				vecLanguage_.pushBack(ModLanguageSet(e.getValue()));
			}
			else
			{
				_SYDNEY_THROW0(Exception::BadArgument);
			}
		}
	}
	else if (pData_->getType() == Common::DataType::Language)
	{
		// 言語情報
		const Common::LanguageData& v
			= _SYDNEY_DYNAMIC_CAST(const Common::LanguageData&, *pData_);
		vecLanguage_.pushBack(v.getValue());
	}
	else if (pData_->getType() == Common::DataType::String)
	{
		// 文字列で与えられた
		const Common::StringData& v
			= _SYDNEY_DYNAMIC_CAST(const Common::StringData&, *pData_);
		vecLanguage_.pushBack(ModLanguageSet(v.getValue()));
	}
	else
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}
}

//
//	FUNCTION private
//	FullText2::FullTextFile::convertScore
//		-- スコア調整値を変換する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data* pData_
//		言語データ
//	double& dblScore_
//		変換後のスコア調整値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::convertScore(const Common::Data* pData_,
						   double& dblScore_)
{
	dblScore_ = 0;

	if (pData_->isNull())
		return;

	; _CHECK_TYPE(*pData_, Double);

	const Common::DoubleData& v
		= _SYDNEY_DYNAMIC_CAST(const Common::DoubleData&, *pData_);
	dblScore_ = v.getValue();
}

//
//	FUNCTION private
//	FullText2::FullTextFile::convertRowID
//		-- ROWIDを変換する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data* pData_
//		言語データ
//	ModUInt32& uiRowID_
//		変換後のROWID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::convertRowID(const Common::Data* pData_,
						   ModUInt32& uiRowID_)
{
	uiRowID_ = 0;

	if (pData_->isNull())
		return;

	; _CHECK_TYPE(*pData_, UnsignedInteger);

	const Common::UnsignedIntegerData& v
		= _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData&, *pData_);
	uiRowID_ = v.getValue();
}

//
//	FUNCTION private
//	FullText2::FullTextFile::checkAndMerge
//		-- バッチインサート時に、必要ならマージする
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
FullTextFile::checkAndMerge()
{
	if (m_lBatchSize > _BatchSizeMax.get())
	{
		// マージが必要

		ModVector<InvertedSection*>::Iterator i = m_vecpSection.begin();
		for (; i != m_vecpSection.end(); ++i)
		{
			// マージする
			(*i)->syncMerge();
		}

		m_lBatchSize = 0;
	}
}

//
//	FUNCTION private
//	FullText2::FullTextFile::getForSearch -- 検索結果を取得する
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData& cTuple_
//		タプル
//
//	RETURN
//	bool
//		検索結果が存在する場合はtrue、存在しない場合はfalse
//
//	EXCEPTIONS
//
bool
FullTextFile::getForSearch(Common::DataArrayData& cTuple_)
{
	if (m_bSearch == false)
	{
		// 検索を実行していないので、検索する

		SortKey::Value eKey = SortKey::DocID;
		Order::Value eOrder = Order::Asc;
		
		switch (m_eSortParameter)
		{
		case OpenOption::SortParameter::ScoreDesc:
			// スコア降順
			eKey = SortKey::Score;
			eOrder = Order::Desc;
			break;
		case OpenOption::SortParameter::ScoreAsc:
			// スコア昇順
			eKey = SortKey::Score;
			eOrder = Order::Asc;
			break;
		}

		if (m_bCluster)
		{
			// クラスタリングの場合には、スコアの降順に固定される
			//
			// それ以外のソート条件が指定されたら、FileID::getSortParameterで
			// エラーとしている

			eKey = SortKey::Score;
			eOrder = Order::Desc;
		}

		// マルチスレッドで検索する
		GetResult getResult(m_pQuery->getRootNode(),
							&(getSearchInformation()),
							m_bScore, eKey, eOrder,
							m_pNarrowing);
		getResult.run();

		// 検索結果を設定する
		m_pResultSet->setResultSet(getResult.getResultSet());

		if (m_eAdjustMethod != AdjustMethod::Unknown ||
			(m_eSortParameter == OpenOption::SortParameter::ScoreAsc
			 && m_bCluster))
		{
			// スコア値を調整する場合か、
			// クラスタリングなのにソートがスコアの昇順の場合には
			// もう一度ソートする
			
			switch (m_eSortParameter)
			{
			case OpenOption::SortParameter::ScoreDesc:
				// スコア降順
				eKey = SortKey::Score;
				eOrder = Order::Desc;
				break;
			case OpenOption::SortParameter::ScoreAsc:
				// スコア昇順
				eKey = SortKey::Score;
				eOrder = Order::Asc;
				break;
			}

			m_pResultSet->adjustScore(m_eAdjustMethod, eKey, eOrder);
		}

		// 検索は終了
		m_bSearch = true;
	}

	if (m_bFirst)
	{
		// 最初なので、検索結果の位置に進める
		m_pResultSet->seek(m_uiOffset, m_uiLimit);

		m_bFirst = false;
	}

	// 結果を取得する
	return m_pResultSet->next(m_pDocIDVectorFile, cTuple_);
}

//
//	FUNCTION private
//	FullText2::FullTextFile::getForWord -- 検索キーワードを取得する
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData& cTuple_
//		タプル
//
//	RETURN
//	bool
//		検索結果が存在する場合はtrue、存在しない場合はfalse
//
//	EXCEPTIONS
//
bool
FullTextFile::getForWord(Common::DataArrayData& cTuple_)
{
	if (m_bSearch == false)
	{
		// 初めての呼び出し

		// 検索語を取り出す
		m_pQuery->getWord(m_vecWord);

		// 得られた検索語は選択値の降順になっているので、
		// ここでソートする
		
		if (m_eSortParameter
			== OpenOption::SortParameter::WordScaleDesc)
		{
			ModSort(m_vecWord.begin(), m_vecWord.end(), _ScaleGreater());
		}
		else if (m_eSortParameter
				 == OpenOption::SortParameter::WordScaleAsc)
		{
			ModSort(m_vecWord.begin(), m_vecWord.end(), _ScaleLess());
		}
		else if (m_eSortParameter
				 == OpenOption::SortParameter::WordDfDesc)
		{
			ModSort(m_vecWord.begin(), m_vecWord.end(), _DfGreater());
		}
		else if (m_eSortParameter
				 == OpenOption::SortParameter::WordDfAsc)
		{
			ModSort(m_vecWord.begin(), m_vecWord.end(), _DfLess());
		}

		// 検索終了
		m_bSearch = true;
	}

	if (m_bFirst)
	{
		// イテレータに割り当てる
		m_wordIte = m_vecWord.begin();

		// オフセットが指定されていたら、読み飛ばす
		while (m_wordIte != m_vecWord.end() && m_uiOffset)
		{
			++m_wordIte;
			--m_uiOffset;
		}

		m_bFirst = false;
	}

	if (m_wordIte == m_vecWord.end() ||
		m_uiGetCount == m_uiLimit)
		return false;

	// 引数のチェック
	if (cTuple_.getCount() != 1)
		_SYDNEY_THROW0(Exception::BadArgument);
	Common::Data& cData = *cTuple_.getElement(0);
	_CHECK_TYPE(cData, Word);

	// 代入する
	Common::WordData& c = _SYDNEY_DYNAMIC_CAST(Common::WordData&, cData);
	c = *m_wordIte;

	++m_wordIte;
	++m_uiGetCount;

	return true;
}

//
//	FUNCTION private
//	FullText2::FullTextFile::getForRead -- 定数値を取得する
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData& cTuple_
//		タプル
//
//	RETURN
//	bool
//		検索結果が存在する場合はtrue、存在しない場合はfalse
//
//	EXCEPTIONS
//
bool
FullTextFile::getForRead(Common::DataArrayData& cTuple_)
{
	bool result = false;
	
	if (m_bFirst)
	{
		// 初めての呼び出し

		// 固定値の取得なので、一度結果を返したら、終わり

		// 検索情報クラスを得る
		SearchInformation& cSearchInfo = getSearchInformation();

		int n = 0;
		ModVector<ResultSet::FieldData>::Iterator i = m_vecGetField.begin();
		for (; i != m_vecGetField.end(); ++i, ++n)
		{
			// 格納するデータクラスを得る

			Common::Data& cData = *cTuple_.getElement(n);

			switch ((*i).second)
			{
			case OpenOption::Function::AverageLength:
			case OpenOption::Function::AverageCharLength:
			case OpenOption::Function::AverageWordCount:
				{
					// v16.5から DoubleData ではなく
					// UnsignedIntegerData になった
					
					if (cData.getType() != Common::DataType::UnsignedInteger)
						_SYDNEY_THROW0(Exception::BadArgument);

					Common::UnsignedIntegerData& c
						= _SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData&,
											   cData);

					// 総文書長
					double len = 0;
					// 文書数
					double count = 0;

					if (isMounted(*m_pTransaction))
					{
						ModVector<int>::Iterator j = (*i).first.begin();
						for (; j != (*i).first.end(); ++j)
						{
							// フィールド番号に対応した検索情報クラスを得る
							SearchInformation& info
								= cSearchInfo.getElement(*j);

							// 総文書長
							len	+= static_cast<double>(
								info.getTotalDocumentLength());
							
							// 文書数
							//
							//【注意】
							//	文書数はどのフィールドでも同じ数である
							//
							count = static_cast<double>(
								info.getDocumentCount());
						}
					}

					if (count != 0)
						c.setValue(ModSize(len / count));
					else
						// 件数が0件の場合はnullにする
						c.setNull();
				}
				break;
			case OpenOption::Function::Count:
				{
					if (cData.getType() != Common::DataType::UnsignedInteger)
						_SYDNEY_THROW0(Exception::BadArgument);

					Common::UnsignedIntegerData& c
						= _SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData&,
											   cData);

					// 文書数
					ModSize count = 0;

					if (isMounted(*m_pTransaction))
					{
						// フィールド番号に対応した検索情報クラスを得る
						SearchInformation& info
							= cSearchInfo.getElement((*i).first[0]);

						count = info.getDocumentCount();
					}

					c.setValue(count);
				}
				break;
			}
		}

		m_bFirst = false;
		result = true;
	}

	return result;
}

//
//	FUNCTION private
//	FullText2::FullTextFile::getForEqual
//		-- 整合性検査のためのequal検索の結果を得る
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData& cTuple_
//		タプル
//
//	RETURN
//	bool
//		検索結果が存在する場合はtrue、存在しない場合はfalse
//
//	EXCEPTIONS
//
bool
FullTextFile::getForEqual(Common::DataArrayData& cTuple_)
{
	bool result = false;
	
	if (m_bFirst)
	{
		// 初めての呼び出し
		// 一度結果を返したら、終わり

		if (m_uiRowID != Os::Limits<ModUInt32>::getMax())
		{
			// ヒットしている
			result = true;

			// ROWIDのみの取得しかない
			Common::Data& cData = *cTuple_.getElement(0);

			if (cData.getType() != Common::DataType::UnsignedInteger)
				_SYDNEY_THROW0(Exception::BadArgument);

			Common::UnsignedIntegerData& v =
				_SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData&,
									 cData);

			v.setValue(m_uiRowID);
		}

		m_bFirst = false;
	}

	return result;
}

//
//	FUNCTION private
//	FullText2::FullTextFile::parseField
//		-- フィールド指定をパースする
//
//	NOTES
//
//	ARGUMETNS
//	ModVector<int>& vecField_
//	   	パースした結果を格納する配列
//	const ModUnicodeChar*& p_
//		パースする文字列へのポインタ
//		実行後は、次の文字へ移動する
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::parseField(ModVector<int>& vecField_,
						 const ModUnicodeChar*& p_)
{
	// フィールド番号をパースし、
	// 必要なセクションを attach する

	const ModUnicodeChar* tmp = p_;
	Query::parseIntArray(vecField_, p_);
	
	if (vecField_.getSize() == 0)
		// フィールドが指定されていない
		_TRMEISTER_THROW1(Exception::SQLSyntaxError, tmp);
	if (m_eCombineMethod == Query::CombineMethod::None &&
		vecField_.getSize() != 1)
		// 合成方法がシングルなのに、フィールドが複数
		_TRMEISTER_THROW1(Exception::SQLSyntaxError, tmp);
	if (m_eCombineMethod != Query::CombineMethod::None &&
		vecField_.getSize() == 1)
		// 合成方法がシングルではないのに、フィールドが１つ
		_TRMEISTER_THROW1(Exception::SQLSyntaxError, tmp);
		
	ModVector<int>::Iterator i = vecField_.begin();
	for (; i != vecField_.end(); ++i)
		attachSection(*i);
}

//
//	FUNCTION private
//	FullText2::FullTextFile::parseScale
//		-- スケールをパースする
//
//	NOTES
//
//	ARGUMETNS
//	const ModVector<int>& vecField_
//	   	フィールド指定
//	ModVector<Query::Scale>& vecScale_
//		パースした結果を格納する配列
//	const ModUnicodeChar*& p_
//		パースする文字列へのポインタ
//		実行後は、次の文字へ移動する
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::parseScale(const ModVector<int>& vecField_,
						 ModVector<Query::ScaleData>& vecScale_,
						 const ModUnicodeChar*& p_)
{
	// スケール指定をパースし、得られた ScaleData にフィールド番号を付与する
	// スケール指定されていない場合には、デフォルトスケールを設定する

	const ModUnicodeChar* tmp = p_;
	
	if (m_eCombineMethod != Query::CombineMethod::None)
	{
		// スコア合成時にしかスケールは書かれない
		
		Query::parseScaleArray(vecScale_, p_);
	}

	if (vecScale_.getSize())
	{
		if (vecScale_.getSize() != vecField_.getSize())
			_TRMEISTER_THROW1(Exception::SQLSyntaxError, tmp);

		// フィールド番号を指定する
		
		ModVector<int>::ConstIterator i = vecField_.begin();
		ModVector<Query::ScaleData>::Iterator j = vecScale_.begin();
		for (; i != vecField_.end(); ++i, ++j)
		{
			(*j).m_iField = (*i);
		}
	}
	else
	{
		// スケール指定されていないので、デフォルトを設定する

		ModVector<int>::ConstIterator i = vecField_.begin();
		for (; i != vecField_.end(); ++i)
		{
			vecScale_.pushBack(Query::ScaleData(*i, 1, 0));
		}
	}
}

//
//	FUNCTION private
//	FullText2::FullTextFile::setAverageLength
//		-- 平均文書長を検索情報クラスに設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<int>& vecField_
//		検索対象のフィールド
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	const ModUnicodeChar*& p_
//		パースする文字列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::setAverageLength(const ModVector<int>& vecField_,
							   SearchInformation& cSearchInfo_,
							   const ModUnicodeChar*& p_)
{
	// 平均文書長指定をパースし、検索情報クラスの各要素に値を設定する

	const ModUnicodeChar* tmp = p_;
	ModVector<double> v;
	Query::parseDoubleArray(v, p_);

	if (v.getSize())
	{
		if (m_eCombineMethod == Query::CombineMethod::None ||
			m_eCombineMethod == Query::CombineMethod::Tf)
		{
			// 要素ではなく、引数の検索情報クラスに設定する
			
			if (v.getSize() != 1)
				// 配列で指定されている
				_TRMEISTER_THROW1(Exception::SQLSyntaxError, tmp);
				
			cSearchInfo_.setAverageLength(*(v.begin()));
		}
		else
		{
			// 各要素に設定する
			
			int s = static_cast<int>(cSearchInfo_.getElementSize());
			ModVector<double>::Iterator i = v.begin();
			
			for (int n = 0; n < s; ++n)
			{
				if (cSearchInfo_.isElementNull(n) == false)
				{
					// null ではないので、設定する

					cSearchInfo_.getElement(n).setAverageLength(*i);
					++i;
				}
			}

			if (i != v.end())
				// 要素数が合っていないのでエラー
				_TRMEISTER_THROW1(Exception::SQLSyntaxError, tmp);
		}
	}
}

//
//	FUNCTION private
//	FullText2::FullTextFile::setDocumentFrequency
//		-- 総文書数を検索情報クラスに設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<int>& vecField_
//		検索対象のフィールド
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	const ModUnicodeChar*& p_
//		パースする文字列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullTextFile::setDocumentFrequency(const ModVector<int>& vecField_,
								   SearchInformation& cSearchInfo_,
								   const ModUnicodeChar*& p_)
{
	// 総文書数をパースし、検索情報クラスの各要素に値を設定する

	const ModUnicodeChar* tmp = p_;
	ModVector<int> v;
	Query::parseIntArray(v, p_);

	if (v.getSize())
	{
		if (m_eCombineMethod == Query::CombineMethod::None ||
			m_eCombineMethod == Query::CombineMethod::Tf)
		{
			// 要素ではなく、引数の検索情報クラスに設定する
			
			if (v.getSize() != 1)
				// 配列で指定されている
				_TRMEISTER_THROW1(Exception::SQLSyntaxError, tmp);
				
			cSearchInfo_.setTotalDocumentFrequency(*(v.begin()));
		}
		else
		{
			// 各要素に設定する
			
			int s = static_cast<int>(cSearchInfo_.getElementSize());
			ModVector<int>::Iterator i = v.begin();
			
			for (int n = 0; n < s; ++n)
			{
				if (cSearchInfo_.isElementNull(n) == false)
				{
					// null ではないので、設定する

					cSearchInfo_.getElement(n).setTotalDocumentFrequency(*i);
					++i;
				}
			}

			if (i != v.end())
				// 要素数が合っていないのでエラー
				_TRMEISTER_THROW1(Exception::SQLSyntaxError, tmp);
		}
	}
}

//
//	FUNCTION private
//	FullText2::FullTextFile::clearTokenizer --
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
FullTextFile::clearTokenizer()
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	
	ModVector<Tokenizer*>::Iterator i = m_vecpTokenizer.begin();
	for (; i != m_vecpTokenizer.end(); ++i)
	{
		delete *i;
	}
	m_vecpTokenizer.clear();
}

//
//	FUNCTION public
//	FullText2::FullTextFile::clearMP -- マルチスレッド関係の変数をクリアする
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
FullTextFile::clearMP()
{
	m_iNextKeyNumber = 0;
	m_vecSuccessKeyNumber.clear();
}

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2016, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
