// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedFile.cpp --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Inverted/InvertedFile.h"
#include "Inverted/InvertedUnit.h"
#include "Inverted/FeatureSet.h"
#include "Inverted/DocumentIDVectorFile.h"
#include "Inverted/RowIDVectorFile.h"
#include "Inverted/RowIDVectorFile2.h"
#include "Inverted/Types.h"
#include "Inverted/LeafPage.h"
#include "Inverted/OpenOption.h"
#include "Inverted/ListManager.h"
#include "Inverted/MultiListManager.h"
#include "Inverted/BatchListMap.h"
#include "Inverted/Parameter.h"
#include "Inverted/LocationListMap.h"
#include "Inverted/UnaAnalyzerManager.h"
#include "Inverted/NormalizerManager.h"
#include "Inverted/TermResourceManager.h"
#include "Inverted/MessageAll_Class.h"
#include "Inverted/BatchList.h"
#include "Inverted/BatchNolocationList.h"
#include "Inverted/BatchNolocationNoTFList.h"

#include "LogicalFile/Estimate.h"

#include "Common/Message.h"
#include "Common/Assert.h"
#include "Common/LanguageData.h"
#include "Common/UnsignedIntegerData.h"

#include "Schema/File.h"

#include "Exception/Unexpected.h"
#include "Exception/BadArgument.h"
#include "Exception/VerifyAborted.h"
#include "Exception/Cancel.h"

#include "PhysicalFile/Types.h"

#include "Os/File.h"

#include "ModInvertedCoder.h"
#include "ModInvertedTokenizer.h"

#include "ModTerm.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

namespace
{
	//
	//  CLASS
	//  _$$::_AutoAttachFile
	//
	class _AutoAttachFile
	{
	public:
		_AutoAttachFile(InvertedFile& cFile_)
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
		InvertedFile& m_cFile;
		bool m_bOwner;
	};

	//
	//  CLASS
	//  _$$::_AutoDetachPage
	//
	class _AutoDetachPage
	{
	public:
		_AutoDetachPage(InvertedFile& cFile_) : m_cFile(cFile_)
			{
			}
		~_AutoDetachPage()
			{
				m_cFile.detachAllPages();
			}

	private:
		InvertedFile& m_cFile;
	};

	//
	//  VARIABLE
	//  _$$::_cMaximumBatchSize
	//	  -- バッチ挿入時にためる転置リストの最大総計サイズ
	//
	ParameterInteger _cMaximumBatchSize("Inverted_BatchSizeMax", 60 << 20);

	//
	//  VARIABLE local
	//  _$$::_cUnaAnalyzerManager -- UNAルールを管理するクラス
	//
	//  NOTES
	//
	UnaAnalyzerManager _cUnaAnalyzerManager;

	//
	//  VARIABLE local
	//  _$$::_cNormalizerManager -- 異表記正規化器ルールを管理するクラス
	//
	//  NOTES
	//
	NormalizerManager _cNormalizerManager;

#ifdef SYD_USE_UNA_V10
	//
	//	VARIABLE local
	//	_$$::_cMaxWordLength -- 最大単語長
	//
	//	NOTES
	//
	ParameterInteger _cMaxWordLength("Inverted_MaxWordLength", 32);
#endif

	//
	//  VARIABLE local
	//
	const char* _TermResourceID = "@TERMRSCID";

	//
	//	VARIABLE local
	//
	const ModUnicodeString _cUnaRscID = _TRMEISTER_U_STRING("UnaRscID");
	const ModUnicodeString _cDefaultLanguageSet = _TRMEISTER_U_STRING(
		"DefaultLanguageSet");
	const ModUnicodeString _cCarriage = _TRMEISTER_U_STRING("carriage");
	const ModUnicodeString _cCompound = _TRMEISTER_U_STRING("compound");
	const ModUnicodeString _cDoNorm = _TRMEISTER_U_STRING("donorm");
	const ModUnicodeString _cMaxWordLen = _TRMEISTER_U_STRING("maxwordlen");
	const ModUnicodeString _cSpace = _TRMEISTER_U_STRING("space");
	const ModUnicodeString _cStem = _TRMEISTER_U_STRING("stem");
	
	const ModUnicodeString _cTrue = _TRMEISTER_U_STRING("true");
	const ModUnicodeString _cFalse = _TRMEISTER_U_STRING("false");
}

//
//  FUNCTION public static
//  Inverted::InvertedFile::initialize -- 初期化
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
InvertedFile::initialize()
{
}

//
//  FUNCTION public static
//  Inverted::InvertedFile::terminate -- 終了処理
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
InvertedFile::terminate()
{
	// 質問処理のリソースを開放する
	TermResourceManager::terminate();
}

//
//  FUNCTION public
//  Inverted::InvertedFile::InvertedFile -- コンストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  const LogicalFile::FileID& cFileID_
//	  ファイルID
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
InvertedFile::InvertedFile()
	: m_pDocumentIDVectorFile(0),
	  m_pRowIDVectorFile(0), m_pRowIDVectorFile2(0),
	  m_pInvertedUnit(0), m_iMaxUnitCount(0), m_bOpenUnit(false),
	  m_pIdCoder(0), m_pFrequencyCoder(0),
	  m_pLengthCoder(0), m_pLocationCoder(0),
	  m_pWordIdCoder(0), m_pWordFrequencyCoder(0),
	  m_pWordLengthCoder(0), m_pWordLocationCoder(0),
	  m_pListManager(0), m_pBatchListMap(0), m_pTransaction(0)
{
	tokenizer = 0;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::~InvertedFile -- デストラクタ
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
//  なし
//
InvertedFile::~InvertedFile()
{
	unsetCoder();
	delete m_pBatchListMap, m_pBatchListMap = 0;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::getFileID -- ファイルIDを得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  const LocgicalFile::FileID&
//	  ファイルID
//
//  EXCEPTIONS
//
const LogicalFile::FileID&
InvertedFile::getFileID() const
{
	return m_cFileID;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::getSize -- ファイルサイズを得る
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//
//  RETURN
//  ModUInt64
//	  ファイルサイズ
//
//  EXCEPTIONS
//
ModUInt64
InvertedFile::getSize(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);

	ModUInt64 size = 0;
	if (isMounted(cTransaction_))
	{
		size += m_pDocumentIDVectorFile->getSize();
		size += getRowIDVector()->getSize();
		for (int i = 0; i < m_iMaxUnitCount; ++i)
			size += m_pInvertedUnit[i].getSize();
	}
	return size;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::getUsedSize -- 使用ファイルサイズを得る(マージ用)
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  ModUInt64
//	  使用ファイルサイズ
//
//  EXCEPTIONS
//
ModUInt64
InvertedFile::getUsedSize(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);

	ModUInt64 size = 0;
	if (isMounted(cTransaction_))
	{
		for (int i = 0; i < m_iMaxUnitCount; ++i)
			size += m_pInvertedUnit[i].getUsedSize(cTransaction_);
	}

	return size;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::getCount -- ファイルに挿入されている件数を得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  ModInt64
//	  件数
//
//  EXCEPTIONS
//
ModInt64
InvertedFile::getCount() const
{
	_AutoDetachPage cAuto(*const_cast<InvertedFile*>(this));
	return static_cast<ModInt64>(getDocumentFrequency());
}

//
//  FUNCTION public
//  Inverted::InvertedFile::getOverhead -- オーバヘッドコストを得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  double
//	  オーバーヘッドコスト
//
//  EXCEPTIONS
//
double
InvertedFile::getOverhead() const
{
	if (isMounted(*m_pTransaction))
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
		double usedList = m_iTermCount;

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
//  FUNCTION public
//  Inverted::InvertedFile::getProcessCost
//	  -- ひとつのタプルを取得する際のプロセスコストを得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  double
//	  コスト
//
//  EXCEPTIONS
//
double
InvertedFile::getProcessCost() const
{
	// すべての検索結果はメモリー上にあるので、それの転送速度
	return static_cast<double>(sizeof(ModUInt32))
		/ LogicalFile::Estimate::getTransferSpeed(
			LogicalFile::Estimate::Memory);
}

//
//  FUNCTION public
//  Inverted::InvertedFile::getSearchParameter
//	  -- 検索オープンパラメータを設定する
//
//  NOTES
//
//  ARGUMENTS
//  const LogicalFile::TreeNodeInterface* pCondition_
//	  検索ノード
//  LogicalFile::OpenOption& cOpenOption_
//	  オープンオプション
//
//  RETURN
//  bool
//	  検索が行える場合はtrue、それ以外の場合はfalse
//
//  EXCEPTIONS
//
/* static */
bool
InvertedFile::getSearchParameter(
	const LogicalFile::TreeNodeInterface* pCondition_,
	bool isLanguage_,
	bool isScoreField_,
	const LogicalFile::FileID& cFileID_,
	LogicalFile::OpenOption& cOpenOption_)
{
	OpenOption cOpenOption(cOpenOption_);
	return cOpenOption.parse(cFileID_, isLanguage_, isScoreField_, pCondition_);
}

//
//  FUNCTION public
//  Inverted::InvertedFile::create -- 転置ファイルを作成する
//
//  NOTES
//  遅延作成対応のため、実際にはファイルを作成しない
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//
//  RETURN
//  const LogicalFile::FileID&
//	  ファイルID
//
//  EXCEPTIONS
//
const LogicalFile::FileID&
InvertedFile::create(const Trans::Transaction& cTransaction_,
					 bool bBigInverted_)
{
	m_cFileID.create(bBigInverted_);
	return m_cFileID;
}

//  FUNCTION public
//  Inverted::InvertedFile::destroy -- 転置ファイルを削除する
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//
//  RETURN
//  なし
//
//  EXCEPTIONS

void
InvertedFile::destroy(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);

	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく削除する
	//
	//【注意】  そうしないと下位層で管理している
	//		  情報がメンテナンスされない

	m_pDocumentIDVectorFile->destroy(cTransaction_);
	getRowIDVector()->destroy(cTransaction_);
	for (int i = 0; i < m_iMaxUnitCount; ++i)
		m_pInvertedUnit[i].destroy(cTransaction_);
}

//
//  FUNCTION public
//  Inverted::InvertedFile::mount -- マウントする
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//
//  RETURN
//  const LogicalFile::FileID&
//	  ファイルID
//
//  EXCEPTIONS
//
const LogicalFile::FileID&
InvertedFile::mount(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);

	if (!isMounted(cTransaction_))
	{
		// マウントされていなければ、マウントしてみる

		int step = 0;
		try
		{
			m_pDocumentIDVectorFile->mount(cTransaction_);
			step++;
			getRowIDVector()->mount(cTransaction_);
			step++;
			for (int i = 0; i < m_iMaxUnitCount; ++i)
			{
				m_pInvertedUnit[i].mount(cTransaction_);
				step++;
			}
		}
		catch (...)
		{
			try
			{
				while (step > 2)
				{
					m_pInvertedUnit[step - 3].unmount(cTransaction_);
					--step;
				}
				switch (step)
				{
				case 2: getRowIDVector()->unmount(cTransaction_);
				case 1: m_pDocumentIDVectorFile->unmount(cTransaction_);
				}
			}
			catch (...)
			{
				SydErrorMessage << "Recovery failed." << ModEndl;
				Schema::File::setAvailability(m_cFileID.getLockName(), false);
			}

			_SYDNEY_RETHROW;
		}

		// マウントされたことをファイル識別子に記録する

		m_cFileID.setMounted(true);
	}

	return m_cFileID;
}

//  FUNCTION public
//  Inverted::InvertedFile::unmount -- アンマウントする
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//
//  RETURN
//  const LogicalFile::FileID&
//	  ファイルID
//
//  EXCEPTIONS

const LogicalFile::FileID&
InvertedFile::unmount(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);

	// マウントの有無や実体の存在の有無を確認せずに
	// とにかくアンマウントする
	//
	//【注意】  そうしないと下位層で管理している
	//		  情報がメンテナンスされない

	int step = 0;
	try
	{
		m_pDocumentIDVectorFile->unmount(cTransaction_);
		step++;
		getRowIDVector()->unmount(cTransaction_);
		step++;
		for (int i = 0; i < m_iMaxUnitCount; ++i)
		{
			m_pInvertedUnit[i].unmount(cTransaction_);
			step++;
		}
	}
	catch (...)
	{
		// ここにくるのはmountされているときのみ
		try
		{
			while (step > 2)
			{
				m_pInvertedUnit[step - 3].mount(cTransaction_);
				--step;
			}
			switch (step)
			{
			case 2: getRowIDVector()->mount(cTransaction_);
			case 1: m_pDocumentIDVectorFile->mount(cTransaction_);
			}
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(m_cFileID.getLockName(), false);
		}

		_SYDNEY_RETHROW;
	}

	// アンマウントされたことをファイル識別子に記録する

	m_cFileID.setMounted(false);

	return m_cFileID;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::flush -- フラッシュする
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::flush(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);

	if (isMounted(cTransaction_))
	{
		m_pDocumentIDVectorFile->flush(cTransaction_);
		getRowIDVector()->flush(cTransaction_);
		for (int i = 0; i < m_iMaxUnitCount; ++i)
			m_pInvertedUnit[i].flush(cTransaction_);
	}
}

//
//  FUNCTION public
//  Inverted::InvertedFile::startBackup -- バックアップ開始する
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//  const bool bRestorable_
//	  リストアフラグ
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::startBackup(const Trans::Transaction& cTransaction_,
						  const bool bRestorable_)
{
	_AutoAttachFile cAuto(*this);

	if (isMounted(cTransaction_))
	{
		int step = 0;
		try
		{
			m_pDocumentIDVectorFile->startBackup(cTransaction_, bRestorable_);
			step++;
			getRowIDVector()->startBackup(cTransaction_, bRestorable_);
			step++;
			for (int i = 0; i < m_iMaxUnitCount; ++i)
			{
				m_pInvertedUnit[i].startBackup(cTransaction_, bRestorable_);
				step++;
			}
		}
		catch (...)
		{
			try
			{
				while (step > 2)
				{
					m_pInvertedUnit[step - 3].endBackup(cTransaction_);
					--step;
				}
				switch (step)
				{
				case 2: getRowIDVector()->endBackup(cTransaction_);
				case 1: m_pDocumentIDVectorFile->endBackup(cTransaction_);
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
}

//
//  FUNCTION public
//  Inverted::InvertedFile::endBackup -- バックアップを終了する
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::endBackup(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);

	if (isMounted(cTransaction_))
	{
		try
		{
			m_pDocumentIDVectorFile->endBackup(cTransaction_);
			getRowIDVector()->endBackup(cTransaction_);
			for (int i = 0; i < m_iMaxUnitCount; ++i)
				m_pInvertedUnit[i].endBackup(cTransaction_);
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(m_cFileID.getLockName(), false);
		}
	}
}

//
//  FUNCTION public
//  Inverted::InvertedFile::recover -- 障害から回復する(物理ログ)
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//  const Trans::TimeStamp& cPoint_
//	  チェックポイント
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::recover(const Trans::Transaction& cTransaction_,
					  const Trans::TimeStamp& cPoint_)
{
	_AutoAttachFile cAuto(*this);

	if (isMounted(cTransaction_))
	{
		m_pDocumentIDVectorFile->recover(cTransaction_, cPoint_);
		getRowIDVector()->recover(cTransaction_, cPoint_);
		for (int i = 0; i < m_iMaxUnitCount; ++i)
			m_pInvertedUnit[i].recover(cTransaction_, cPoint_);

		if (!isAccessible())
		{
			// リカバリの結果
			// 実体である OS ファイルが存在しなくなったので、
			// サブディレクトリを削除する

			m_pDocumentIDVectorFile->rmdir();
			getRowIDVector()->rmdir();
		}
	}
}

//
//  FUNCTION public
//  Inverted::InvertedFile::restore
//	  -- ある時点に開始された読取専用トランザクションが
//		  参照する版を最新版とする
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//  const Trans::TimeStamp& cPoint_
//	  チェックポイント
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::restore(const Trans::Transaction& cTransaction_,
					  const Trans::TimeStamp& cPoint_)
{
	_AutoAttachFile cAuto(*this);

	if (isMounted(cTransaction_))
	{
		m_pDocumentIDVectorFile->restore(cTransaction_, cPoint_);
		getRowIDVector()->restore(cTransaction_, cPoint_);
		for (int i = 0; i < m_iMaxUnitCount; ++i)
			m_pInvertedUnit[i].restore(cTransaction_, cPoint_);
	}
}

//
//  FUNCTION public
//  Inverted::InvertedFile::verify -- 整合性検査を行う
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//  Admin::Verification::Treatment::Value uiTreatment_
//	  動作
//  Admin::Verification::Progress& cProgress_
//	  経過
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::verify(const Trans::Transaction& cTransaction_,
					 Admin::Verification::Treatment::Value uiTreatment_,
					 Admin::Verification::Progress& cProgress_)
{
	_AutoAttachFile cAuto(*this);

	if (isMounted(cTransaction_))
	{
		// 整合性検査開始
		startVerification(cTransaction_, uiTreatment_, cProgress_);

		try
		{
			// ベクターのチェック
			verifyVectorFile(uiTreatment_, cProgress_, m_cFileID.getPath());

			for (int i = 0; i < m_iMaxUnitCount; ++i)
			{
				InvertedUnit* u = m_pInvertedUnit + i;
				if (u->isMounted(cTransaction_) == false)
					continue;

				// まずはB木のチェック
				u->verifyBtree();

				// 論理的な内容のチェック
				ListManager cListManager(u);
				cListManager.verify(uiTreatment_, cProgress_,
									m_cFileID.getPath());

				if (uiTreatment_ & Admin::Verification::Treatment::Correct)
				{
					// 削除できるIDブロックがあれば削除する
					InvertedUnit::Map::Iterator k
						= u->m_mapDeleteIdBlock.begin();
					for (; k != u->m_mapDeleteIdBlock.end(); ++k)
					{
						// 転置リストを割り当てる
						if (cListManager.reset(
								(*k).first,ModInvertedListSearchMode)
							== ModTrue)
						{
							// 割り当てたリストからIDブロックを削除する
							cListManager.expungeIdBlock((*k).second);

							InvertedUnit::Vector::Iterator n
								= (*k).second.begin();
							for (; n != (*k).second.end(); ++n)
							{
								_SYDNEY_VERIFY_CORRECTED(
									cProgress_,
									m_cFileID.getPath(),
									Message::CorrectDisusedIdBlock(*n));
							}
						}

						// 全ファイルの変更を確定する
						saveAllPages();
					}
				}

				u->m_mapDeleteIdBlock.erase(
					u->m_mapDeleteIdBlock.begin(),
					u->m_mapDeleteIdBlock.end());
			}
		}
		catch (Exception::VerifyAborted&)
		{
			// なにもしない
			;
		}
		catch (...)
		{
			flushAllPages();
			// 整合性検査終了
			endVerification();
			_SYDNEY_RETHROW;
		}

		flushAllPages();
		// 整合性検査終了
		endVerification();

	}
}

//
//  FUNCTION public
//  Inverted::InvertedFile::open -- ファイルをオープンする
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//  const LogicalFile::OpenOption& cOpenOption_
//	  チェックポイント
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::open(const Trans::Transaction& cTransaction_,
				   const LogicalFile::OpenOption& cOpenOption_)
{
	try
	{
		// create fulltext indexを行う際にfeatureを指定しないとエラーになる
		// トランザクションを設定
		m_pTransaction = &cTransaction_;

		// FIXモードを設定
		int iValue = cOpenOption_.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key));
		if (iValue == LogicalFile::OpenOption::OpenMode::Read
			|| iValue == LogicalFile::OpenOption::OpenMode::Search)
			m_eFixMode = Buffer::Page::FixMode::ReadOnly;
		else if (iValue == LogicalFile::OpenOption::OpenMode::Update
				 || iValue == LogicalFile::OpenOption::OpenMode::Initialize
				 || iValue == LogicalFile::OpenOption::OpenMode::Batch)
			m_eFixMode = Buffer::Page::FixMode::Write;
		else {
			; _SYDNEY_THROW0(Exception::BadArgument);
		}

		if (m_eFixMode == Buffer::Page::FixMode::Write &&
			isBatchInsert() == false)
		{
			// バッチインサート以外の場合はdiscardableにする
			m_eFixMode |= Buffer::Page::FixMode::Discardable;
		}

		m_iTermCount = 0;
		if (m_eFixMode == Buffer::Page::FixMode::ReadOnly)
		{
			// 検索語数を設定する
			OpenOption cOpenOption(cOpenOption_);
			m_iTermCount = cOpenOption.getTermCount();
			if (m_iTermCount == 0)
				m_iTermCount = 3; // 0の場合は3にする(てきとう、てきとう)
		}

		// DocumentIDVectorファイルをアタッチする
		m_pDocumentIDVectorFile = new DocumentIDVectorFile(m_cFileID,
														   isBatchInsert());

		// 索引タイプを設定する(Mod)
		indexingType = m_cFileID.getIndexingType();

		// ファイルをオープンする
		open(cTransaction_, m_eFixMode);
	}
	catch (...)
	{
		flushAllPages();
		detachFile();
		m_pTransaction = 0;

		_SYDNEY_RETHROW;
	}
}

//
//  FUNCTION public
//  Inverted::InvertedFile::close -- ファイルをクローズする
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
InvertedFile::close()
{
	flushAllPages();

	; _SYDNEY_ASSERT(m_pTransaction);

	for (int i = 0; i < m_iMaxUnitCount; ++i)
		m_pInvertedUnit[i].close();
	m_bOpenUnit = false;
	if (m_pRowIDVectorFile) m_pRowIDVectorFile->close();
	if (m_pRowIDVectorFile2) m_pRowIDVectorFile2->close();
	if (m_pDocumentIDVectorFile) m_pDocumentIDVectorFile->close();

	detachFile();
	tokenizer = 0;
	m_pTransaction = 0;
}

//  FUNCTION public
//  Inverted::InvertedFile::sync -- 転置ファイルの同期をとる
//
//  NOTES
//
//  ARGUMENTS
//	  Trans::Transaction& cTransaction_
//		  転置ファイルの同期を取る
//		  トランザクションのトランザクション記述子
//	  bool&			incomplete
//		  true
//			  今回の同期処理で転置ファイルを持つ
//			  オブジェクトの一部に処理し残しがある
//		  false
//			  今回の同期処理で転置ファイルを持つ
//			  オブジェクトを完全に処理してきている
//
//			  同期処理の結果、転置ファイルを処理し残したかを設定する
//	  bool&			modified
//		  true
//			  今回の同期処理で転置ファイルを持つ
//			  オブジェクトの一部が既に更新されている
//		  false
//			  今回の同期処理で転置ファイルを持つ
//			  オブジェクトはまだ更新されていない
//
//			  同期処理の結果、転置ファイルが更新されたかを設定する
//
//  RETURN
//	  なし
//
//  EXCEPTIONS

void
InvertedFile::sync(const Trans::Transaction& cTransaction_,
				   bool& incomplete, bool& modified)
{
	_AutoAttachFile cAuto(*this);

	if (isMounted(cTransaction_))
	{
		m_pDocumentIDVectorFile->sync(cTransaction_, incomplete, modified);
		getRowIDVector()->sync(cTransaction_, incomplete, modified);
		for (int i = 0; i < m_iMaxUnitCount; ++i)
			m_pInvertedUnit[i].sync(cTransaction_, incomplete, modified);
	}
}

//
//  FUNCTION public
//  Inverted::InvertedFile::move -- ファイルを移動する
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//  const Os::Path& cFilePath_
//	  移動先のパス
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::move(const Trans::Transaction& cTransaction_,
				   const Os::Path& cFilePath_)
{
	_AutoAttachFile cAuto(*this);

	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく移動する
	//
	//【注意】  そうしないと下位層で管理している
	//		  情報がメンテナンスされない

	int step = 0;
	try
	{
		m_pDocumentIDVectorFile->move(cTransaction_, cFilePath_);
		step++;
		getRowIDVector()->move(cTransaction_, cFilePath_);
		step++;
		for (int i = 0; i < m_iMaxUnitCount; ++i)
		{
			m_pInvertedUnit[i].move(cTransaction_, cFilePath_);
			step++;
		}
	}
	catch (...)
	{
		try
		{
			while (step > 2)
			{
				m_pInvertedUnit[step - 3].move(cTransaction_,
											   m_cFileID.getPath());
				--step;
			}
			switch (step)
			{
			case 2: getRowIDVector()->move(cTransaction_,
										   m_cFileID.getPath());
			case 1: m_pDocumentIDVectorFile->move(cTransaction_,
												  m_cFileID.getPath());
			}
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(m_cFileID.getLockName(), false);
		}

		_SYDNEY_RETHROW;
	}

	m_cFileID.setPath(cFilePath_);
}

//
//  FUNCTION public
//  Inverted::InvertedFile::isAccessible --
//	  実体である OS ファイルが存在するか調べる
//
//  NOTES
//
//  ARGUMENTS
//  bool bForce_ (default false)
//	  強制的に行うか
//
//  RETURN
//  bool
//	   ファイルが作成されている場合はtrue、それ以外の場合はfalse
//
//  EXCEPTIONS
//
bool
InvertedFile::isAccessible(bool bForce_) const
{
	_AutoAttachFile cAuto(*const_cast<InvertedFile*>(this));

	return m_pDocumentIDVectorFile->isAccessible(bForce_);
}

//  FUNCTION public
//  Inverted::InvertedFile::isMounted -- マウントされているか調べる
//
//  NOTES
//
//  ARGUMENTS
//	  Trans::Transaction& trans
//		  マウントされているか調べる
//		  トランザクションのトランザクション記述子
//
//  RETURN
//	  true
//		  マウントされている
//	  false
//		  マウントされていない
//
//  EXCEPTIONS

bool
InvertedFile::isMounted(const Trans::Transaction& trans) const
{
	_AutoAttachFile cAuto(*const_cast<InvertedFile*>(this));

	return m_pDocumentIDVectorFile->isMounted(trans);
}

//
//  FUNCTION public
//  Inverted::InvertedFile::clear -- ファイルをクリアする
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//  bool bForce_ (default false)
//	  強制的に行うか
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::clear(const Trans::Transaction& cTransaction_, bool bForce_)
{
	_AutoAttachFile cAuto(*this);

	if (isMounted(cTransaction_))
	{
		_AutoDetachPage cAutoDetach(*this);
		m_pDocumentIDVectorFile->clear(cTransaction_, bForce_);
		getRowIDVector()->clear(cTransaction_, bForce_);
		for (int i = 0; i < m_iMaxUnitCount; ++i)
			m_pInvertedUnit[i].clear(cTransaction_, bForce_);
	}
}

//
//  FUNCTION public
//  Inverted::InvertedFile::insert -- 文書データを挿入する
//
//  NOTES
//
//  ARGUMENTS
//  const ModUnicodeString& vecDocument_
//	  各セクションが連結された文書データ
//  const ModVector<ModLanguage>& vecLanguage_
//	  言語データ
//  ModUInt32 uiTupleID_
//	  ROWID
//  ModVector<ModSize>& vecSectionOffset_
//	  セクション区切り(文字数)
//	ModInvertedFeatureList& vecFeature_
//	  特徴語
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::insert(
	const ModUnicodeString& cstrDocument_,
	const ModVector<ModLanguageSet>& vecLanguage_,
	ModUInt32 uiTupleID_,
	ModVector<ModSize>& vecSectionOffset_,
	ModInvertedFeatureList& vecFeature_)
{
	; _SYDNEY_ASSERT(m_pTransaction);
	if (!isMounted(*m_pTransaction))
	{
		// 作成遅延でまだファイルが作成されていない
		substantiate();
	}

	// トークナイズモード
	ModInvertedTokenizer::TokenizeMode eTokenizeMode;
	switch (m_cFileID.getIndexingType())
	{
	case ModInvertedFileNgramIndexing:
		eTokenizeMode = ModInvertedTokenizer::ngramIndexingOnly;
		break;
	case ModInvertedFileWordIndexing:
		eTokenizeMode = ModInvertedTokenizer::wordIndexingOnly;
		break;
	case ModInvertedFileDualIndexing:
		eTokenizeMode = ModInvertedTokenizer::document;
		break;
	}

	// セクション区切り位置(文字->バイトに変換)
	ModVector<ModSize>::Iterator i = vecSectionOffset_.begin();
	for (; i != vecSectionOffset_.end(); ++i)
	{
		(*i) *= sizeof(ModUnicodeChar);
	}

	// 言語指定をチェックする
	ModVector<ModLanguageSet> vecLanguage
		= checkLanguageSet(vecSectionOffset_.getSize(), vecLanguage_);

	//
	//  トークナイズを行う
	//

	ModSize uiDocumentLength;			// 正規化した文書長
	ModVector<ModSize> vecNewSectionOffset; // 正規化したセクション区切り位置
	LocationListMap cLocationListMap;	// トークナイズ結果

	// 特徴語抽出
	// feature は、特徴語と重要度(log(TF+1) x 生起コスト)のペア
	ModInvertedFeatureList* pf = 0;
	const ModTermResource* pTermResource = 0;
	if (m_cFileID.isClustering())
	{
		pf = &vecFeature_;
		ModUnicodeString cstrExtractor = m_cFileID.getExtractor();
		ModSize uiTermResourceID = ModInvertedTokenizer::getResourceID(
			cstrExtractor.getString(), _TermResourceID);
		pTermResource = TermResourceManager::get(uiTermResourceID);
	}

	tokenizer->tokenize(cstrDocument_,	  // [IN]連結された文書データ
						eTokenizeMode,	  // [IN]トークナイズモード
						cLocationListMap, // [OUT]トークナイズ結果
						uiDocumentLength, // [OUT]正規化した文書長
						&vecSectionOffset_, // [IN]セクション区切り位置(Byte)
						&vecNewSectionOffset,
						// [OUT]正規化したセクション区切り位置(文字)
						&vecLanguage,		// [IN]セクションの言語情報
						pf,			 		// [OUT]特徴語
						pTermResource);		// [IN]TermResource

	if (isBatchInsert() == true)
	{
		// バッチインサート
		insertBatch(cLocationListMap, uiDocumentLength, uiTupleID_);
	}
	else
	{
		// ファイルへのインサート
		insertFile(cLocationListMap, uiDocumentLength, uiTupleID_);
	}

	// セクション区切り位置(1オリジン->0オリジンに変換)
	vecSectionOffset_ = vecNewSectionOffset;
	ModVector<ModSize>::Iterator j = vecSectionOffset_.begin();
	for (; j != vecSectionOffset_.end(); ++j)
	{
		if ((*j)) (*j) -= 1;
	}
}

//
//  FUNCTION public
//  Inverted::InvertedFile::expunge -- 文書データを削除する
//
//  NOTES
//
//  ARGUMENTS
//  const ModUnicodeString& vecDocument_
//	  各セクションが連結された文書データ
//  const ModVector<ModLanguage>& vecLanguage_
//	  言語データ
//  ModUInt32 uiTupleID_
//	  ROWID
//  ModVector<ModSize>& vecSectionOffset_
//	  セクション区切り(文字数)
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::expunge(const ModUnicodeString& cstrDocument_,
					  const ModVector<ModLanguageSet>& vecLanguage_,
					  ModUInt32 uiTupleID_,
					  const ModVector<ModSize>& vecSectionOffset_)
{
	; _SYDNEY_ASSERT(m_pTransaction);
	; _SYDNEY_ASSERT(isMounted(*m_pTransaction));

	// トークナイズモード
	ModInvertedTokenizer::TokenizeMode eTokenizeMode;
	switch (m_cFileID.getIndexingType())
	{
	case ModInvertedFileNgramIndexing:
		eTokenizeMode = ModInvertedTokenizer::ngramIndexingOnly;
		break;
	case ModInvertedFileWordIndexing:
		eTokenizeMode = ModInvertedTokenizer::wordIndexingOnly;
		break;
	case ModInvertedFileDualIndexing:
		eTokenizeMode = ModInvertedTokenizer::document;
		break;
	}

	// セクション区切り位置(文字->バイトに変換)
	ModVector<ModSize> vecSectionOffset = vecSectionOffset_;	// コピー
	ModVector<ModSize>::Iterator i = vecSectionOffset.begin();
	for (; i != vecSectionOffset.end(); ++i)
	{
		(*i) *= sizeof(ModUnicodeChar);
	}

	// 言語指定をチェックする
	ModVector<ModLanguageSet> vecLanguage
		= checkLanguageSet(vecSectionOffset_.getSize(), vecLanguage_);

	//
	//  トークナイズを行う
	//

	ModSize uiDocumentLength;			// 正規化した文書長
	ModVector<ModSize> vecNewSectionOffset; // 正規化したセクション区切り位置
	ModInvertedLocationListMap cLocationListMap; // トークナイズ結果

	tokenizer->tokenize(cstrDocument_,		  // [IN]連結された文書データ
						eTokenizeMode,		  // [IN]トークナイズモード
						cLocationListMap,	// [OUT]トークナイズ結果
						uiDocumentLength,	// [OUT]正規化した文書長
						&vecSectionOffset,	  // [IN]セクション区切り位置
						&vecNewSectionOffset,
						// [OUT]正規化したセクション区切り位置
						&vecLanguage,		// [IN]セクションの言語情報
						0,					// [OUT]特徴語(不要)
						0);					// [IN]TermResource(不要)

	// 文書IDを求める
	ModInt32 iUnit = 0;
	ModUInt32 uiDocumentID = convertToDocumentID(uiTupleID_, iUnit);

	// 削除対象のユニット
	InvertedUnit* u = m_pInvertedUnit + iUnit;
	// オープンする
	u->open(*m_pTransaction, m_eFixMode);

	//
	//  トークナイズ結果を挿入する
	//
	ModInvertedLocationListMap::Iterator j = cLocationListMap.begin();
	int step = 0;
	try
	{
		{
			ListManager cListManager(u);

			for (; j != cLocationListMap.end(); ++j)
			{
				// 転置リストを割り当てる
				if (cListManager.reset(
						(*j).first, ModInvertedListSearchMode) == ModTrue)
				{
					// 割り当てたリストからデータを削除する
					cListManager.expunge(uiDocumentID);
				}

				// 全ファイルの変更を確定する
				saveAllPages();
			}

			step++;

			// ROWID<->文書IDのベクタから削除する
			m_pDocumentIDVectorFile->expunge(uiDocumentID, iUnit);
			if (m_pRowIDVectorFile)
				m_pRowIDVectorFile->expunge(uiTupleID_);
			else
				m_pRowIDVectorFile2->expunge(uiTupleID_);
		}

		flushAllPages();

		// Undoログを削除する
		u->m_mapExpungeFirstDocumentID.erase(
			u->m_mapExpungeFirstDocumentID.begin(),
			u->m_mapExpungeFirstDocumentID.end());
	}
	catch (...)
	{
		u->m_mapDeleteIdBlock.erase(
			u->m_mapDeleteIdBlock.begin(), u->m_mapDeleteIdBlock.end());

		try
		{
			// 全ファイルの変更を破棄する
			recoverAllPages();

			if (j != cLocationListMap.begin())
			{
				ListManager cListManager(u);

				// 削除した部分を取り消す
				ModInvertedLocationListMap::Iterator k
					= cLocationListMap.begin();
				for (; k != j; ++k)
				{
					// 転置リストを割り当てる
					if (cListManager.reset(
							(*k).first, ModInvertedListSearchMode) == ModTrue)
					{
						// 割り当てたリストの削除を取り消す
						cListManager.undoExpunge(uiDocumentID, (*k).second);
					}

					// 全ファイルの変更を確定する
					saveAllPages();
				}
			}

			flushAllPages();

			// Undoログを削除する
			u->m_mapExpungeFirstDocumentID.erase(
				u->m_mapExpungeFirstDocumentID.begin(),
				u->m_mapExpungeFirstDocumentID.end());
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			// もう回復できない...
			Schema::File::setAvailability(m_cFileID.getLockName(), false);
		}

		_SYDNEY_RETHROW;
	}

	try
	{
		{
			ListManager cListManager(u);

			// 削除できるIDブロックがあれば削除する
			InvertedUnit::Map::Iterator k = u->m_mapDeleteIdBlock.begin();
			for (; k != u->m_mapDeleteIdBlock.end(); ++k)
			{
				// 転置リストを割り当てる
				ModBoolean result
					= cListManager.reset((*k).first, ModInvertedListSearchMode);
				; _SYDNEY_ASSERT(result == ModTrue);

				// 割り当てたリストからIDブロックを削除する
				cListManager.expungeIdBlock((*k).second);

				// 全ファイルの変更を確定する
				saveAllPages();
			}
		}

		flushAllPages();

		u->m_mapDeleteIdBlock.erase(u->m_mapDeleteIdBlock.begin(),
									u->m_mapDeleteIdBlock.end());
	}
	catch (...)
	{
		// ここでエラーが発生すると、回復できない。回復できなくても問題ない
		recoverAllPages();
		_SYDNEY_RETHROW;
	}
}

//
//  FUNCTION public
//  Inverted::InvertedFile::merge -- 1つの転置リストをマージする
//
//  NOTES
//  索引単位が同じ挿入用小転置の転置リストと削除用小転置の転置リストをマージする
//
//  ARGUMENTS
//  Inverted::InvertedList* pInsertList_
//	  挿入用小転置の転置リスト
//  Inverted::InvertedList* pExpungeList_
//	  削除用小転置のリスト
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::merge(InvertedList* pInsertList_, InvertedList* pExpungeList_)
{
	if (!isMounted(*m_pTransaction))
	{
		// 作成遅延でまだファイルが作成されていない
		substantiate();
	}

	ModUnicodeString cstrKey;
	if (pInsertList_)
	{
		cstrKey = pInsertList_->getKey();
	}
	else if (pExpungeList_)
	{
		cstrKey = pExpungeList_->getKey();
	}

	try
	{
		// まず削除の処理を行う

		if (pExpungeList_ != 0)
		{
			for (int i = 0; i < m_iMaxUnitCount; ++i)
			{
				InvertedUnit* u = m_pInvertedUnit + i;

				// 存在をチェックし、存在していなかったら飛ばす
				if (m_pDocumentIDVectorFile->isInserted(i) == false)
					continue;
				
				u->open(*m_pTransaction, m_eFixMode); // オープンする

				// 存在をチェックし、存在していなかったら飛ばす
				if (u->isMounted(*m_pTransaction) == false)
					continue;

				ListManager cListManager(u);

				if (cListManager.reset(cstrKey, ModInvertedListSearchMode)
					== ModTrue)
				{
					// 削除する
					cListManager.expunge(*pExpungeList_);

					// 削除できるIDブロックがあれば削除する
					InvertedUnit::Map::Iterator k
						= u->m_mapDeleteIdBlock.begin();
					for (; k != u->m_mapDeleteIdBlock.end(); ++k)
					{
						;_SYDNEY_ASSERT(cstrKey == (*k).first);

						// 割り当てたリストからIDブロックを削除する
						cListManager.expungeIdBlock((*k).second);

						// ここでショートになっているかもしれないので、
						// もう一度割り当てなおす
						cListManager.reset(cstrKey, ModInvertedListSearchMode);
					}

					u->m_mapDeleteIdBlock.erase(
						u->m_mapDeleteIdBlock.begin(),
						u->m_mapDeleteIdBlock.end());
				}
			}
		}

		// 次に挿入の処理を行う
		if (pInsertList_)
		{
			// 挿入ユニットを得る
			int unit = m_pDocumentIDVectorFile->getInsertUnit();
			InvertedUnit* u = m_pInvertedUnit + unit;

			u->open(*m_pTransaction, m_eFixMode);	// オープンする

			// 存在をチェックし、存在していなかったら作成する
			if (u->isMounted(*m_pTransaction) == false)
			{
				u->create();
			}

			ListManager cListManager(u);

			if (cListManager.reset(cstrKey, ModInvertedListCreateMode)
				== ModTrue)
			{
				// 挿入する
				cListManager.insert(*pInsertList_);
			}
		}

		if (isBatchInsert() == false)
			// すべてのページを確定する
			flushAllPages();
	}
	catch (...)
	{
		// すべてのページを破棄する
		recoverAllPages();
		_SYDNEY_RETHROW;
	}

	for (int i = 0; i < m_iMaxUnitCount; ++i)
	{
		InvertedUnit* u = m_pInvertedUnit + i;
		// Undoログを削除する
		u->m_mapExpungeFirstDocumentID.erase(
			u->m_mapExpungeFirstDocumentID.begin(),
			u->m_mapExpungeFirstDocumentID.end());
	}
}

//
//  FUNCTION public
//  Inverted::InvertedFile::beginBatchInsert -- バッチインサートを開始する
//
//  NOTES
//
//  ARGUMENTS
//  ModSize uiSize_ (default 0)
//	  メモリー上にためるリストの総計。
//	  0が与えられた場合はパラメータから取得する
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::beginBatchInsert(ModSize uiSize_)
{
	if (uiSize_ == 0)
	{
		// パラメータから取得する
		uiSize_ = _cMaximumBatchSize.get();
	}
	m_uiMaximumBatchSize = uiSize_;

	// バッチ用のリストをためるマップを確保する
	m_pBatchListMap = new BatchListMap;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::endBatchInsert -- バッチインサートを終了する
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
InvertedFile::endBatchInsert()
{
	try
	{
		// バッチマップの内容を反映する
		mergeBatch();
	}
	catch (...)
	{
		delete m_pBatchListMap, m_pBatchListMap = 0;
		_SYDNEY_RETHROW;
	}
	delete m_pBatchListMap, m_pBatchListMap = 0;
}

//
//  FUCNTION public
//  Inverted::InvertedFile::detachAllPages -- すべてのページをデタッチする
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
InvertedFile::detachAllPages()
{
	flushAllPages();
}

//
//  FUNCTION public
//  Inverted::InvertedFile::lowerBound -- 転置リストを取り出す
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  Inverted::InvertedList*
//	  転置リスト。存在しない場合は0が返る
//
//  EXCEPTIONS
//
InvertedList*
InvertedFile::lowerBound(const ModUnicodeString& cstrKey_)
{
	InvertedList* pInvertedList = 0;
	; _SYDNEY_ASSERT(m_pTransaction);
	if (isMounted(*m_pTransaction))
	{
		; _SYDNEY_ASSERT(m_iMaxUnitCount == 1);

		if (m_pListManager == 0)
		{
			//
			//  【注意】
			//  このメソッドは小転置専用である。
			//
			openUnit();
			m_pListManager = new ListManager(m_pInvertedUnit);
		}

		if (m_pListManager->reset(cstrKey_,
								  ModInvertedListLowerBoundMode) == ModTrue)
		{
			// 転置リストを取り出す
			pInvertedList = m_pListManager->getInvertedList();
		}
	}

	return pInvertedList;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::next -- 次の転置リストを取り出す
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  Inverted::InvertedList*
//	  転置リスト。終了した場合は0が返る
//
//  EXCEPTIONS
//
InvertedList*
InvertedFile::next()
{
	InvertedList* pInvertedList = 0;
	; _SYDNEY_ASSERT(m_pTransaction);
	if (isMounted(*m_pTransaction))
	{
		if (m_pListManager->next() == ModTrue)
		{
			// 転置リストを取り出す
			pInvertedList = m_pListManager->getInvertedList();
		}
	}

	return pInvertedList;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::mergeVectorFile -- ベクターファイルをマージする
//
//  NOTES
//
//  ARGUMENTS
//  Inverted::InvertedFile* pInsertFile_
//	  挿入用小転置
//  Inverted::InvertedFile* pExpungeFile_
//	  削除用小転置
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::mergeVectorFile(InvertedFile* pInsertFile_,
							  InvertedFile* pExpungeFile_)
{
	try
	{
		ModUInt32 uiLastDocumentID = getLastDocumentID();

		// ベクターファイルの内容をマージする

		; _SYDNEY_ASSERT(m_pTransaction);

		// 削除から
		if (pExpungeFile_ && pExpungeFile_->isMounted(*m_pTransaction))
		{
			// 小転置はファイル分散はしていないので、
			RowIDVectorFile* pRowVector = pExpungeFile_->m_pRowIDVectorFile;

			RowIDVectorFile::Iterator i = pRowVector->begin();
			for (; i != pRowVector->end(); ++i)
			{
				
				ModUInt32 uiTupleID = (*i).key;
				ModInt32 unit = 0;
				ModUInt32 uiDocumentID = convertToDocumentID(uiTupleID, unit);

				m_pDocumentIDVectorFile->expunge(uiDocumentID, unit);
				if (m_pRowIDVectorFile)
					m_pRowIDVectorFile->expunge(uiTupleID);
				else
					m_pRowIDVectorFile2->expunge(uiTupleID);
			}
		}

		int u = m_pDocumentIDVectorFile->getInsertUnit();

		// 次に挿入
		if (pInsertFile_ && pInsertFile_->isMounted(*m_pTransaction))
		{
			// 最大ROWIDを取得し、RowIDVectorFileを拡張する
			ModUInt32 uiMax = pInsertFile_->m_pRowIDVectorFile->getMaximumKey();
			if (m_pRowIDVectorFile)
				m_pRowIDVectorFile->expand(uiMax);
			else
				m_pRowIDVectorFile2->expand(uiMax);
			
			DocumentIDVectorFile* pIDVector
				= pInsertFile_->m_pDocumentIDVectorFile;

			DocumentIDVectorFile::Iterator i = pIDVector->begin();
			for (; i != pIDVector->end(); ++i)
			{
				ModUInt32 uiTupleID = (*i).value.first;
				ModUInt32 uiDocumentID = (*i).key;
				ModSize uiLength = (*i).value.second;

				m_pDocumentIDVectorFile->insert(
					uiDocumentID + uiLastDocumentID, u, uiTupleID, uiLength);
				if (m_pRowIDVectorFile)
					m_pRowIDVectorFile->insert(
						uiTupleID, uiDocumentID + uiLastDocumentID);
				else
					m_pRowIDVectorFile2->insert(
						uiTupleID, uiDocumentID + uiLastDocumentID, u);
			}

			// 挿入ユニットをチェックする
			m_pDocumentIDVectorFile->checkInsertUnit();
		}

		// 内容を確定する
		flushAllPages();
	}
	catch (...)
	{
		// 内容を破棄する
		recoverAllPages();
		_SYDNEY_RETHROW;
	}
}

//
//  FUNCTION public
//  Inverted::InvertedFile::contains -- 指定したROWIDのデータが含まれているか
//
//  NOTES
//
//  ARGUMENTS
//  ModUInt32 uiRowID_
//	  ROWID
//
//  RETURN
//  bool
//	  含まれている場合はtrue、それ以外の場合はfalse
//
//  EXCEPTIONS
//
bool
InvertedFile::contains(ModUInt32 uiRowID_)
{
	bool bResult = false;
	; _SYDNEY_ASSERT(m_pTransaction);
	if (isMounted(*m_pTransaction))
	{
		_AutoDetachPage cAuto(*this);
		bResult = (convertToDocumentID(uiRowID_) == UndefinedDocumentID)
			? false : true;
	}
	return bResult;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::getUnaParameter --
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
void
InvertedFile::getUnaParameter(Common::DataArrayData& cUnaParameterKey_,
							  Common::DataArrayData& cUnaParameterValue_)
{
	; _TRMEISTER_ASSERT(cUnaParameterKey_.getCount() == 0
						&& cUnaParameterValue_.getCount() == 0);
	
	ModOstrStream stream;

	// Resource ID
	cUnaParameterKey_.pushBack(new Common::StringData(_cUnaRscID));
	cUnaParameterValue_.pushBack(
		new Common::UnsignedIntegerData(
			ModInvertedTokenizer::getResourceID(
				m_cFileID.getTokenizeParameter(),
				ModInvertedTokenizer::analyzerResourceID)));

	// Default language set
	cUnaParameterKey_.pushBack(
		new Common::StringData(_cDefaultLanguageSet));
	cUnaParameterValue_.pushBack(
		new Common::LanguageData(m_cFileID.getDefaultLanguageSet()));

	// Carriage
	cUnaParameterKey_.pushBack(new Common::StringData(_cCarriage));
	cUnaParameterValue_.pushBack(
		new Common::StringData(m_cFileID.isCarriage() ? _cTrue: _cFalse));
	
	// Compound
	cUnaParameterKey_.pushBack(new Common::StringData(_cCompound));
	// See ModInvertedDualTokenizer() for details.
	cUnaParameterValue_.pushBack(new Common::StringData(_cTrue));
	
	// DoNorm(Normalizing)
	cUnaParameterKey_.pushBack(new Common::StringData(_cDoNorm));
	cUnaParameterValue_.pushBack(
		new Common::StringData(m_cFileID.isNormalized() ? _cTrue: _cFalse));
	
	// Max word length
	cUnaParameterKey_.pushBack(new Common::StringData(_cMaxWordLen));
	stream.clear();
	stream << _cMaxWordLength.get();
	cUnaParameterValue_.pushBack(new Common::StringData(stream.getString()));
	
	// Space
	cUnaParameterKey_.pushBack(new Common::StringData(_cSpace));
	stream.clear();
	stream << m_cFileID.getSpaceMode();
	cUnaParameterValue_.pushBack(new Common::StringData(stream.getString()));
	
	// Stemming
	cUnaParameterKey_.pushBack(new Common::StringData(_cStem));
	cUnaParameterValue_.pushBack(
		new Common::StringData(m_cFileID.isStemming() ? _cTrue: _cFalse));
}

//
//  FUNCTION public
//  Inverted::InvertedFile::getIdCoder
//  Inverted::InvertedFile::getFrequencyCoder
//  Inverted::InvertedFile::getLengthCoder
//  Inverted::InvertedFile::getLocationCoder
//	  -- 圧縮器を得る
//
//  NOTES
//
//  ARGUMENTS
//  const ModUnicodeString& cstrKey_
//	  索引単位
//
//  RETURN
//  ModInvertedCoder*
//	  圧縮器
//
//  EXCEPIOTNS
//
ModInvertedCoder*
InvertedFile::getIdCoder(const ModUnicodeString& cstrKey_) const
{
	return (cstrKey_.getLength() == 0)
		? m_pWordIdCoder : m_pIdCoder;
}
ModInvertedCoder*
InvertedFile::getFrequencyCoder(const ModUnicodeString& cstrKey_) const
{
	return (cstrKey_.getLength() == 0)
		? m_pWordFrequencyCoder : m_pFrequencyCoder;
}
ModInvertedCoder*
InvertedFile::getLengthCoder(const ModUnicodeString& cstrKey_) const
{
	return (cstrKey_.getLength() == 0)
		? m_pWordLengthCoder : m_pLengthCoder;
}
ModInvertedCoder*
InvertedFile::getLocationCoder(const ModUnicodeString& cstrKey_) const
{
	return (cstrKey_.getLength() == 0)
		? m_pWordLocationCoder : m_pLocationCoder;
}

//
//  【注意】
//  B木関係のこれらのメソッドはカバレージ測定のためexportしている
//

void
InvertedFile::insertBtree_debug(const ModUnicodeString& cstrKey_,
								PhysicalFile::PageID uiPageID_)
{
	m_pInvertedUnit->insertBtree(cstrKey_, uiPageID_);
}

void
InvertedFile::expungeBtree_debug(const ModUnicodeString& cstrKey_,
								 PhysicalFile::PageID uiPageID_)
{
	m_pInvertedUnit->expungeBtree(cstrKey_, uiPageID_);
}

void
InvertedFile::updateBtree_debug(const ModUnicodeString& cstrKey1_,
								PhysicalFile::PageID uiPageID1_,
								const ModUnicodeString& cstrKey2_,
								PhysicalFile::PageID uiPageID2_)
{
	m_pInvertedUnit->updateBtree(cstrKey1_, uiPageID1_, cstrKey2_, uiPageID2_);
}

bool
InvertedFile::searchBtree_debug(const ModUnicodeString& cstrKey_,
								PhysicalFile::PageID& uiPageID_)
{
	return m_pInvertedUnit->searchBtree(cstrKey_, uiPageID_);
}

// ここまで

#ifndef SYD_COVERAGE
//
//  FUNCTION public
//  Inverted::InvertedFile::reportFile -- ファイル状況を報告する
//
//  NOTES
//
//  ARGUMENTS
//  ModOstream& stream_
//	  出力ストリーム
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::reportFile(ModOstream& stream_)
{
	// openとsetTokenizerされているのが前提
	; _SYDNEY_ASSERT(m_pTransaction);

	if (!isMounted(*m_pTransaction))
		return;

	stream_ << "TotalDocumentCount="
			<< m_pDocumentIDVectorFile->getDocumentCount() << ModEndl;
	stream_ << "AverageDocumentLength="
			<< m_pDocumentIDVectorFile->getAverageDocumentLength() << ModEndl;
	stream_ << "TotalListCount="
			<< m_pDocumentIDVectorFile->getListCount() << ModEndl;

	for (int i = 0; i < m_iMaxUnitCount; ++i)
		m_pInvertedUnit[i].reportFile(*m_pTransaction, m_eFixMode, stream_);

	flushAllPages();
}
#endif

//
//  FUNCTION public
//  Inverted::InvertedFile::createTokenizer -- トークナイザーを作成する
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  ModInvertedTokenizer*
//	  トークナイザー
//
//  EXCEPTIONS
//
ModInvertedTokenizer*
InvertedFile::createTokenizer()
{
	// 圧縮器を設定する
	setCoder();

	return ModInvertedTokenizer::create(
		m_cFileID.getTokenizeParameter(),
		this,
		m_cFileID.isNormalized() ? ModTrue : ModFalse,
		m_cFileID.isStemming() ? ModTrue : ModFalse,
		m_cFileID.getSpaceMode(),
		m_cFileID.isCarriage() ? ModTrue : ModFalse
#ifdef SYD_USE_UNA_V10
		, _cMaxWordLength.get()
#endif
		, m_cFileID.getFeatureSize());
}

//
//  FUNCTION public static
//  Inverted::InvertedFile::deleteTokenizer -- トークナイザーを破棄する
//
//  NOTES
//
//  ARGUMENTS
//  ModInvertedTokenizer* tokenizer_
//	  トークナイザー
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::deleteTokenizer(ModInvertedTokenizer* tokenizer_)
{
	if (tokenizer_) delete tokenizer_;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::setTokenizer -- トークナイザーを設定する
//
//  NOTES
//  トークナイザーを設定するということは、このファイルが使用されるということ。
//
//  ARGUMENTS
//  ModInvertedTokenizer* tokenizer_
//	  トークナイザーを設定する
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::setTokenizer(ModInvertedTokenizer* tokenizer_)
{
	// すでに設定済みなら実行しない
	if (tokenizer) return;

	// 圧縮器を設定する
	setCoder();

	// トークナイザーを設定する
	tokenizer = tokenizer_;

	// 必要なファイルをアタッチする
	attachFile(m_eFixMode != Buffer::Page::FixMode::ReadOnly);

	// ファイルをオープンする
	open(*m_pTransaction, m_eFixMode);
}

//
//  FUNCTION public
//  Inverted::InvertedFile::getLastDocumentID -- 最終文書IDを得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  ModUInt32
//	  最終文書ID
//
//  EXCEPTIONS
//
ModUInt32
InvertedFile::getLastDocumentID()
{
	ModUInt32 uiLastDocumentID = 0;
	; _SYDNEY_ASSERT(m_pTransaction);
	if (isMounted(*m_pTransaction))
		uiLastDocumentID = m_pDocumentIDVectorFile->getLastDocumentID();
	return uiLastDocumentID;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::convertToRowID -- 文書ID->ROWIDの変換を行う
//
//  NOTES
//
//  ARGUMENTS
//  ModUInt32 uiDocumentID_
//	  文書ID。存在しない場合は
//
//  RETURN
//  ModUInt32
//	  ROWID。存在しない場合は UndefinedRowID
//
//  EXCEPTIONS
//
ModUInt32
InvertedFile::convertToRowID(ModUInt32 uiDocumentID_)
{
	ModUInt32 uiRowID = UndefinedRowID;
	ModUInt32 uiDocumentLength;

	// ベクターを検索する
	m_pDocumentIDVectorFile->find(uiDocumentID_, uiRowID, uiDocumentLength);

	return uiRowID;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::convertToDocumentID -- ROWID->文書IDの変換を行う
//
//  NOTES
//
//  ARGUMENTS
//  ModUInt32 uiRowID_
//	  RowID
//
//  RETURN
//  ModUInt32
//	  文書ID。存在しない場合は UndefinedDocumentID
//
//  EXCEPTIONS
//
ModUInt32
InvertedFile::convertToDocumentID(ModUInt32 uiRowID_)
{
	ModInt32 dummy;
	return convertToDocumentID(uiRowID_, dummy);
}

//
//  FUNCTION public
//  Inverted::InvertedFile::convertToDocumentID
//	  -- ROWIDから文書IDとユニット番号を得る
//
//  NOTES
//
//  ARGUMENTS
//  ModUInt32 uiRowID_
//	  ROWID
//  ModInt32& iUnit_
//	  ユニット番号
//
//  RETURN
//  ModUInt32
//	  文書ID
//
//  EXCEPTIONS
//
ModUInt32
InvertedFile::convertToDocumentID(ModUInt32 uiRowID_, ModInt32& iUnit_)
{
	ModUInt32 uiDocumentID = UndefinedDocumentID;
	iUnit_ = 0;

	if (m_pRowIDVectorFile)
		m_pRowIDVectorFile->find(uiRowID_, uiDocumentID);
	else
		m_pRowIDVectorFile2->find(uiRowID_, uiDocumentID, iUnit_);

	return uiDocumentID;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::incrementListCount -- 転置リスト数を1つ増やす
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
InvertedFile::incrementListCount(int element_)
{
	m_pDocumentIDVectorFile->incrementListCount(element_);
}

//
//  FUNCTION public
//  Inverted::invertedFile::attachFile -- 各種ファイルにアタッチする
//
//  NOTES
//
//  ARGUMENTS
//  bool bUpdate_
//	  更新のためかどうか (default true)
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::attachFile(bool bUpdate_)
{
	if (!m_pDocumentIDVectorFile)
		m_pDocumentIDVectorFile = new DocumentIDVectorFile(m_cFileID,
														   isBatchInsert());

	if (bUpdate_ || m_pTransaction == 0 || isMounted(*m_pTransaction))
	{
		// 更新系または、実態がある場合のみ
		if (!m_pRowIDVectorFile )
		{
			if (m_cFileID.isDistribution())
				m_pRowIDVectorFile2 = new RowIDVectorFile2(m_cFileID,
														   isBatchInsert());
			else
				m_pRowIDVectorFile = new RowIDVectorFile(m_cFileID,
														 isBatchInsert());
		}
		if (m_iMaxUnitCount == 0)
		{
			m_iMaxUnitCount = m_cFileID.getDistribute();
			if (m_iMaxUnitCount == 0) m_iMaxUnitCount = 1;
			m_pInvertedUnit = new InvertedUnit[m_iMaxUnitCount];
			for (int i = 0; i < m_iMaxUnitCount; ++i)
				m_pInvertedUnit[i].initialize(this,
											  &m_cFileID, i, isBatchInsert());
		}
	}
}

//
//  FUNCTION public
//  Inverted::InvertedFile::detachFile -- すべてのファイルをデタッチする
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
InvertedFile::detachFile()
{
	delete m_pDocumentIDVectorFile, m_pDocumentIDVectorFile = 0;
	delete m_pRowIDVectorFile, m_pRowIDVectorFile = 0;
	delete m_pRowIDVectorFile2, m_pRowIDVectorFile2 = 0;
	delete [] m_pInvertedUnit, m_pInvertedUnit = 0;
	m_iMaxUnitCount = 0;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::check -- 整合性検査のための存在チェック
//
//  NOTES
//
//  ARGUMENTS
//  const ModUnicodeString& cstrDocument_
//	  文書データ
//  ModUInt32 uiTupleID_
//	  タプルID
//  const ModVector<ModLanguage>& vecLanguage_
//	  言語情報
//  ModVector<ModSize>& vecSectionByteOffset_
//	  セクションの区切り位置
//
//  RETURN
//  bool
//	  存在する場合はtrue、それ以外の場合はfalse
//
//  EXCEPTIONS
//
bool
InvertedFile::check(const ModUnicodeString& cstrDocument_,
					ModUInt32 uiTupleID_,
					const ModVector<ModLanguageSet>& vecLanguage_,
					ModVector<ModSize>& vecSectionByteOffset_)
{
	; _SYDNEY_ASSERT(m_pTransaction);
	if (!isMounted(*m_pTransaction))

		// 作成遅延でまだマウントされていない

		return false;

	ModInt32 iUnit = 0;
	ModUInt32 uiDocumentID = convertToDocumentID(uiTupleID_, iUnit);
	if (uiDocumentID == UndefinedDocumentID)
	{
		// 文書が登録されていない

		return false;
	}

	//
	//  登録されている内容と同じかどうかチェックする
	//

	// トークナイズモード
	ModInvertedTokenizer::TokenizeMode eTokenizeMode;
	switch (m_cFileID.getIndexingType())
	{
	case ModInvertedFileNgramIndexing:
		eTokenizeMode = ModInvertedTokenizer::ngramIndexingOnly;
		break;
	case ModInvertedFileWordIndexing:
		eTokenizeMode = ModInvertedTokenizer::wordIndexingOnly;
		break;
	case ModInvertedFileDualIndexing:
		eTokenizeMode = ModInvertedTokenizer::document;
		break;
	}

	// 言語指定をチェックする
	ModVector<ModLanguageSet> vecLanguage
		= checkLanguageSet(vecSectionByteOffset_.getSize(), vecLanguage_);

	// トークナイズを行う
	ModSize uiDocumentLength;			// 正規化した文書長
	ModVector<ModSize> vecNewSectionOffset; // 正規化したセクション区切り位置
	LocationListMap cLocationListMap;	// トークナイズ結果

	tokenizer->tokenize(cstrDocument_,	  // [IN]連結された文書データ
						eTokenizeMode,	  // [IN]トークナイズモード
						cLocationListMap, // [OUT]トークナイズ結果
						uiDocumentLength, // [OUT]正規化した文書長
						&vecSectionByteOffset_, // [IN]セクション区切り位置
						&vecNewSectionOffset,
						// [OUT]正規化したセクション区切り位置
						&vecLanguage,		// [IN]セクションの言語情報
						0,					// [OUT]特徴語(不要)
						0);					// [IN]TermResource(不要)

	// 位置情報をチェックする
	m_pInvertedUnit[iUnit].open(*m_pTransaction, m_eFixMode);
	ListManager cListManager(m_pInvertedUnit + iUnit);
	LocationListMap::Iterator j = cLocationListMap.begin();
	for (; j != cLocationListMap.end(); ++j)
	{
		// トークナイズして得られた各文字列と一致する
		// 索引語の転置リストを、順番に確認する。
		
		// 転置リストを割り当てる
		if (cListManager.reset((*j).first, ModInvertedListSearchMode)
			== ModFalse)
			// 転置リストがなかった
			return false;

		if (isNoTF() == false)
		{
			// 少なくともTFは格納されている (位置リストは格納されていないかも)
			
			// 割り当てたリストと同じかどうかチェックする
			if (cListManager.check(uiDocumentID, (*j).second) == false)
				return false;
		}
	}
	
	// 全文側でセクション情報との整合性を見るのでここで代入する
	vecSectionByteOffset_ = vecNewSectionOffset;

	return true;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::getTotalDocumentLength -- 総文書長を得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  ModUInt64
//	  総文書長
//
//  EXCEPTIONS
//
ModUInt64
InvertedFile::getTotalDocumentLength()
{
	ModUInt64 ulLength = 0;
	; _SYDNEY_ASSERT(m_pTransaction);
	if (isMounted(*m_pTransaction))
		ulLength = m_pDocumentIDVectorFile->getTotalDocumentLength();
	return ulLength;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::getDefaultLanguageSet -- デフォルトの言語指定を得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  const ModLanguageSet&
//	  デフォルトの言語指定
//
//  EXCEPTIONS
//
const ModLanguageSet&
InvertedFile::getDefaultLanguageSet()
{
	return m_cFileID.getDefaultLanguageSet();
}

//
//  FUNCTION public
//  Inverted::InvertedFile::isAttached -- ファイルがアタッチされているか
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  bool
//	  アタッチされている場合はtrue、それ以外の場合はfalse
//
//  EXCEPTIONS
//
bool
InvertedFile::isAttached() const
{
	return (m_pDocumentIDVectorFile != 0) ? true : false;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::isBatchInsert -- バッチインサート中かどうか
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  bool
//	  バッチインサート中の場合はtrue、それ以外の場合はfalse
//
//  EXCEPTIONS
//
bool
InvertedFile::isBatchInsert() const
{
	return (m_pBatchListMap != 0) ? true : false;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::isCancel -- 中断要求がきているかどうか
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  bool
//	  中断要求がきている場合はtrue、それ以外の場合はfalse
//
//  EXCEPTIONS
//
bool
InvertedFile::isCancel() const
{
	return const_cast<Trans::Transaction*>(m_pTransaction)
		->isCanceledStatement();
}

//
//  FUNCTION public
//  Inverted::InvertedFile::getLibTerm
//	  -- この転置ファイルに適切なModTermを得る
//
//  NOTES
//
//  ARGUMENTS
//  ModSize resourceID_
//	  リソースID
//  ModSize unaResourceID_
//	  UNAリソースID(指定しない場合はUndefinedResourceID)
//  ModSize collectionSize_
//	  登録文書数
//  ModSize averageLength_
//	  平均文書長
//
//  RETURN
//  ModNlpAnalyzer*
//
//  EXCEPTIONS
//
ModTerm*
InvertedFile::getLibTerm(ModSize resourceID_,
						 ModSize unaResourceID_,
						 ModSize collectionSize_,
						 ModSize averageLength_)
{
#ifdef SYD_USE_UNA_V10
	UNA::ModNlpAnalyzer* analyzer = 0;
#else
 	ModNlpAnalyzer* analyzer = 0;
#endif
	ModBoolean owner = ModFalse;
	if (unaResourceID_ == UndefinedResourceID)
	{
		analyzer = tokenizer->getAnalyzer(ModCharString());
		if (analyzer == 0)
		{
			// Dualじゃない
			ModUInt32 id = ModInvertedTokenizer::getResourceID(
				m_cFileID.getTokenizeParameter(),
				ModInvertedTokenizer::normalizerResourceID);
			analyzer = UnaAnalyzerManager::get(id);
			owner = ModTrue;
		}
	}
	else
	{
		// libTerm用のUNAリソースが指定されている
		analyzer = UnaAnalyzerManager::get(unaResourceID_);
		owner = ModTrue;
	}

	const ModTermResource* termResource = TermResourceManager::get(resourceID_);

	return new ModTerm(termResource,
					   analyzer,
					   owner,
					   collectionSize_,
					   averageLength_,
					   _cMaxWordLength.get());
}

//
//  FUNCTION public
//  Inverted::InvertedFile::getInvertedList -- 新しい転置リストを得る(Mod)
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  ModInvertedList*
//	  転置リストクラス
//
//  EXCEPTIONS
//
ModInvertedList*
InvertedFile::getInvertedList() const
{
	openUnit();

	if (m_iMaxUnitCount <= 1)
		return new ListManager(const_cast<InvertedUnit*>(m_pInvertedUnit));
	else
		return new MultiListManager(m_pDocumentIDVectorFile,
									m_pRowIDVectorFile2,
									m_pInvertedUnit,
									m_iMaxUnitCount);
}

//
//  FUNCTION public
//  Inverted::InvertedFile::getInvertedList -- 転置リストをリセットする(Mod)
//
//  NOTES
//
//  ARGUMENTS
//  const ModUnicodeString& cstrKey_
//	  索引単位
//  ModInvertedList& cInvertedList_
//	  転置リスト
//  const ModInvertedListAccessMode eAccessMode_
//	  アクセスモード
//
//  RETURN
//  ModBoolean
//	  索引単位の転置リストが存在する場合はModTrue、それ以外の場合はModFalse
//
//  EXCEPTIONS
//
ModBoolean
InvertedFile::getInvertedList(
	const ModUnicodeString& cstrKey_,
	ModInvertedList& cInvertedList_,
	const ModInvertedListAccessMode eAccessMode_) const
{
	return cInvertedList_.reset(cstrKey_, eAccessMode_);
}

//
//  FUNCTION public
//  Inverted::InvertedFile::getDocumentLengthFile -- 文書情報ファイルを得る(Mod)
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  ModInvertedDocumentLengthFile*
//	  文書情報ファイル
//
//  EXCEPTIONS
//
ModInvertedDocumentLengthFile*
InvertedFile::getDocumentLengthFile() const
{
	return m_pDocumentIDVectorFile;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::getDocumentFrequency -- 文書頻度を得る(Mod)
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  ModSize
//	  文書頻度
//
//  EXCEPTIONS
//
ModSize
InvertedFile::getDocumentFrequency() const
{
	ModSize count = 0;
	; _SYDNEY_ASSERT(m_pTransaction);
	if (isMounted(*m_pTransaction))
	{
		count = const_cast<DocumentIDVectorFile*>(m_pDocumentIDVectorFile)
			->getDocumentCount();
	}
	return count;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::getTokenizer -- トークナイザーを得る(Mod)
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  ModInvertedTokenizer*
//	  トークナイザー
//
//  EXCEPTIONS
//
ModInvertedTokenizer*
InvertedFile::getTokenizer() const
{
	return tokenizer;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::getMaxDocumentID -- 最大文書IDを得る(Mod)
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  ModInvertedDocumentID
//	  最大文書ID
//
//  EXCEPTIONS
//
ModInvertedDocumentID
InvertedFile::getMaxDocumentID() const
{
	ModInvertedDocumentID id = 0;
	if (isMounted(*m_pTransaction))
	{
		id = m_pDocumentIDVectorFile->getMaximumDocumentID();
	}
	return id;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::getMinDocumentID -- 最小文書IDを得る(Mod)
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  ModInvertedDocumentID
//	  最小文書ID
//
//  EXCEPTIONS
//
ModInvertedDocumentID
InvertedFile::getMinDocumentID() const
{
	ModInvertedDocumentID id = 0;
	if (isMounted(*m_pTransaction))
	{
		id = m_pDocumentIDVectorFile->getMinimumDocumentID();
	}
	return id;
}

//
//  FUNCTION private
//  Inverted::InvertedFile::setCoder -- 圧縮器を設定する
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
InvertedFile::setCoder()
{
	if (m_pIdCoder == 0)
	{
		m_pIdCoder
			= ModInvertedCoder::create(m_cFileID.getIdCoder());
	}
	if (m_pFrequencyCoder == 0)
	{
		m_pFrequencyCoder
			= ModInvertedCoder::create(m_cFileID.getFrequencyCoder());
	}
	if (m_pLengthCoder == 0)
	{
		m_pLengthCoder
			= ModInvertedCoder::create(m_cFileID.getLengthCoder());
	}
	if (m_pLocationCoder == 0)
	{
		m_pLocationCoder
			= ModInvertedCoder::create(m_cFileID.getLocationCoder());
	}
	if (m_pWordIdCoder == 0)
	{
		m_pWordIdCoder
			= ModInvertedCoder::create(m_cFileID.getWordIdCoder());
	}
	if (m_pWordFrequencyCoder == 0)
	{
		m_pWordFrequencyCoder
			= ModInvertedCoder::create(m_cFileID.getWordFrequencyCoder());
	}
	if (m_pWordLengthCoder == 0)
	{
		m_pWordLengthCoder
			= ModInvertedCoder::create(m_cFileID.getWordLengthCoder());
	}
	if (m_pWordLocationCoder == 0)
	{
		m_pWordLocationCoder
			= ModInvertedCoder::create(m_cFileID.getWordLocationCoder());
	}
}

//
//  FUNCTION private
//  Inverted::InvertedFile::unsetCoder -- 圧縮器を開放する
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
InvertedFile::unsetCoder()
{
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
//  FUNCTION private
//  Inverted::InvertedFile::substantiate -- ファイルを本当に作成する
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
InvertedFile::substantiate()
{
	int step = 0;
	try
	{
		m_pDocumentIDVectorFile->create();
		step++;
		getRowIDVector()->create();
		step++;

		flushAllPages();
	}
	catch (...)
	{
		recoverAllPages();

		switch (step)
		{
		case 1: m_pDocumentIDVectorFile->destroy();
		case 0:
			break;
		}

		Os::Directory::remove(m_cFileID.getPath());

		_SYDNEY_RETHROW;
	}
}

//
//  FUNCTION private
//  Inverted::InvertedFile::open -- ファイルをオープンする
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//  Buffer::Page::FixMode::Value eFixMode_
//	  FIXモード
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::open(const Trans::Transaction& cTransaction_,
					 Buffer::Page::FixMode::Value eFixMode_)
{
	m_bOpenUnit = false;
	int step = 0;
	try
	{
		if (m_pDocumentIDVectorFile)
			m_pDocumentIDVectorFile->open(cTransaction_, eFixMode_);
		step++;
		if (getRowIDVector())
			getRowIDVector()->open(cTransaction_, eFixMode_);
		step++;
	}
	catch (...)
	{
		switch (step)
		{
		case 1: if (m_pDocumentIDVectorFile) m_pDocumentIDVectorFile->close();
		}
		_SYDNEY_RETHROW;
	}
}

//
//  FUNCTION public
//  Inverted::InvertedFile::flushAllPages
//	  -- すべてのページを確定し、デタッチする
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
InvertedFile::flushAllPages()
{
	if (m_pListManager) delete m_pListManager, m_pListManager = 0;

	if (m_pDocumentIDVectorFile) m_pDocumentIDVectorFile->flushAllPages();
	if (getRowIDVector()) getRowIDVector()->flushAllPages();
	for (int i = 0; i < m_iMaxUnitCount; ++i)
		m_pInvertedUnit[i].flushAllPages();
}

//
//  FUNCTION public
//  Inverted::InvertedFile::recoverAllPages
//	  -- すべてのページを破棄し、デタッチする
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
InvertedFile::recoverAllPages()
{
	if (m_pListManager) delete m_pListManager, m_pListManager = 0;

	if (m_pDocumentIDVectorFile) m_pDocumentIDVectorFile->recoverAllPages();
	if (getRowIDVector()) getRowIDVector()->recoverAllPages();
	for (int i = 0; i < m_iMaxUnitCount; ++i)
		m_pInvertedUnit[i].recoverAllPages();
}

//
//  FUNCTION public
//  Inverted::InvertedFile::saveAllPages -- すべてのページを確定する
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
InvertedFile::saveAllPages()
{
	if (m_pDocumentIDVectorFile) m_pDocumentIDVectorFile->saveAllPages();
	if (getRowIDVector()) getRowIDVector()->saveAllPages();
	for (int i = 0; i < m_iMaxUnitCount; ++i)
		m_pInvertedUnit[i].saveAllPages();
}

//
//  FUNCTION private
//  Inverted::InvertedFile::insertFile -- ファイルに文書を挿入する
//
//  NOTES
//
//  ARGUMENTS
//  LocationListMap& cLocationListMap_
//	  トークナイズ結果
//  ModUInt32 uiDocumentLength_
//	  文書長
//  ModUInt32 uiTupleID_
//	  RowID
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::insertFile(LocationListMap& cLocationListMap_,
						 ModUInt32 uiDocumentLength_, ModUInt32 uiTupleID_)
{
	// 文書IDを求める
	ModUInt32 uiDocumentID = getLastDocumentID();
	uiDocumentID += 1;  // 新規に挿入する文書の文書IDはひとつ大きなものになる

	// 挿入するユニット番号を得る
	int unit = m_pDocumentIDVectorFile->getInsertUnit();
	InvertedUnit* u = m_pInvertedUnit + unit;

	// 挿入するユニットをオープンする
	u->open(*m_pTransaction, m_eFixMode);
	if (u->isMounted(*m_pTransaction) == false)
	{
		// まだ作成されていないので、作成する
		u->create();
	}
	else if (m_pDocumentIDVectorFile->isInserted(unit) == false)
	{
		// 全件削除されている
		u->clear(*m_pTransaction);
		m_pDocumentIDVectorFile->clearUnit(unit);

		// 変更を確定する
		u->flushAllPages();
		m_pDocumentIDVectorFile->flushAllPages();
	}

	//
	//  トークナイズ結果を挿入する
	//
	LocationListMap::Iterator j = cLocationListMap_.begin();
	try
	{
		{
			ListManager cListManager(u);

			for (; j != cLocationListMap_.end(); ++j)
			{
				// 転置リストを割り当てる
				cListManager.reset((*j).first, ModInvertedListCreateMode);
				// 割り当てたリストにデータを挿入する
				cListManager.insert(uiDocumentID, (*j).second);

				// 全ファイルの変更を確定する
				saveAllPages();
			}

			// ROWID<->文書IDのベクタに登録する
			m_pDocumentIDVectorFile->insert(uiDocumentID, unit,
											uiTupleID_, uiDocumentLength_);
			if (m_pRowIDVectorFile)
			{
				m_pRowIDVectorFile->expand(uiTupleID_);
				m_pRowIDVectorFile->insert(uiTupleID_, uiDocumentID);
			}
			else
			{
				m_pRowIDVectorFile2->expand(uiTupleID_);
				m_pRowIDVectorFile2->insert(uiTupleID_, uiDocumentID, unit);
			}
		}

		// 挿入するユニットをチェックする
		m_pDocumentIDVectorFile->checkInsertUnit();

		flushAllPages();
	}
	catch (...)
	{
		try
		{
			// 全ファイルの変更を破棄する
			recoverAllPages();

			if (j != cLocationListMap_.begin())
			{
				ListManager cListManager(u);

				// 挿入した部分を削除する
				LocationListMap::Iterator k = cLocationListMap_.begin();
				for (; k != j; ++k)
				{
					// 転置リストを割り当てる
					if (cListManager.reset(
						(*k).first, ModInvertedListSearchMode) == ModTrue)
					{
						// 割り当てたリストからデータを削除する
						cListManager.expunge(uiDocumentID);
					}

					// 全ファイルの変更を確定する
					saveAllPages();
				}
			}

			flushAllPages();
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			// もう回復できない...
			Schema::File::setAvailability(m_cFileID.getLockName(), false);
		}

		_SYDNEY_RETHROW;
	}
}

//
//  FUNCTION private
//  Inverted::InvertedFile::insertBatch -- バッチマップに文書を挿入する
//
//  NOTES
//
//  ARGUMENTS
//  LocationListMap& cLocationListMap_
//	  トークナイズ結果
//  ModUInt32 uiDocumentLength_
//	  文書長
//  ModUInt32 uiTupleID_
//	  RowID
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::insertBatch(LocationListMap& cLocationListMap_,
							ModUInt32 uiDocumentLength_, ModUInt32 uiTupleID_)
{
	//
	// m_pBatchListMap にトークナイズ結果を挿入し、
	// その結果、更新する転置リスト数が一定数を超えたら、
	// 実際に挿入(merge)する。
	//
	
	try
	{
		// 文書IDを求める
		ModUInt32 uiDocumentID = m_pBatchListMap->getLastDocumentID();

		// 挿入するユニット番号を得る
		int unit = m_pDocumentIDVectorFile->getInsertUnit();
		InvertedUnit* u = m_pInvertedUnit + unit;

		//
		//  トークナイズ結果を挿入する
		//
		LocationListMap::Iterator j = cLocationListMap_.begin();
		for (; j != cLocationListMap_.end(); ++j)
		{
			if ((*j).second.getSize() == 0)
				// 位置リストが0件なので次のトークンへ
				continue;

			//
			// このトークンを持つ転置リスト(のリスト)を得る
			//
			BatchListMap::Iterator i = m_pBatchListMap->find((*j).first);
			if (i == m_pBatchListMap->end())
			{
				// 見つからない
				
				// 新規に(BatchListMapにBatchListのリストを)挿入する
				ModList<BatchBaseList*> list;
				list.pushBack(makeBatchList(u, (*j).first));
				// 挿入する
				ModPair<BatchListMap::Iterator, ModBoolean> r
					= m_pBatchListMap->insertEntry((*j).first, list);
				
				// マップのイテレータを、
				// 新規に挿入した転置リストのリストに置き換える
				i = r.first;
			}

			// 転置リストを得る
			ModList<BatchBaseList*>::Iterator k = (*i).second.end();
			--k;

			//
			// 転置リストにデータ(位置リスト)を挿入する
			//
			while ((*k)->insert(uiDocumentID, (*j).second) == false)
			{
				// 挿入に失敗した

				// 転置リストのリストに新規の転置リストを追加する
				(*i).second.pushBack(makeBatchList(u, (*j).first));

				// 新規の転置リストを、次の挿入先に設定する
				k = (*i).second.end();
				--k;
			}
		}

		// ROWID<->文書IDのベクタに登録する
		m_pBatchListMap->insertVector(uiDocumentID,
										uiTupleID_, uiDocumentLength_);

		//
		//  大きさをチェックし、超えていたらマージする
		//
		if (m_pBatchListMap->getListSize() > m_uiMaximumBatchSize)
		{
			// 超えているので、マージ
			mergeBatch();

			delete m_pBatchListMap;
			m_pBatchListMap = new BatchListMap;
		}
	}
	catch (...)
	{
		delete m_pBatchListMap, m_pBatchListMap = 0;
		_SYDNEY_RETHROW;
	}
}

//
//  FUNCTION private
//  Inverted::InvertedFile::mergeBatch -- バッチの内容を反映する
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
InvertedFile::mergeBatch()
{
	if (m_pBatchListMap == 0 ||
		(m_pRowIDVectorFile == 0 && m_pRowIDVectorFile2 == 0))
		// バッチインサート時に何か起こった
		return;

#if 0
	SydMessage << "Begin Inverted Merge" << ModEndl;
#endif

	// 転置リストをマージする
	BatchListMap::Iterator i = m_pBatchListMap->begin();
	for (; i != m_pBatchListMap->end(); ++i)
	{
		// 転置リストのリストを得る
		
		ModList<BatchBaseList*>::Iterator j = (*i).second.begin();
		for (; j != (*i).second.end(); ++j)
		{
			// 転置リストを一つマージする
			merge(*j, 0);
		}
		
		// マージが終わったリストから削除する
		for (j = (*i).second.begin(); j != (*i).second.end(); ++j)
			// 転置リストを一つ削除する
			delete (*j);
		// 転置リストのリストを削除する
		(*i).second.clear();

		// 1つの索引単位の処理が終わったので、変更をセーブする
		saveAllPages();
	}

#if 0
	SydMessage << "Begin Vector Merge" << ModEndl;
#endif

	try
	{
		ModUInt32 uiLastDocumentID = getLastDocumentID();
		int unit = m_pDocumentIDVectorFile->getInsertUnit();

		// 最大ROWID分のページを確保する
		if (m_pRowIDVectorFile)
			m_pRowIDVectorFile->expand(m_pBatchListMap->getMaxRowID());
		else
			m_pRowIDVectorFile2->expand(m_pBatchListMap->getMaxRowID());

		// ベクタファイルの内容をマージする
		BatchListMap::Vector& cVector = m_pBatchListMap->getVector();
		BatchListMap::Vector::Iterator j = cVector.begin();
		for (; j != cVector.end(); ++j)
		{
			m_pDocumentIDVectorFile->insert(
				(*j).m_uiDocumentID + uiLastDocumentID, unit,
				(*j).m_uiRowID, (*j).m_uiLength);
			if (m_pRowIDVectorFile)
				m_pRowIDVectorFile->insert(
					(*j).m_uiRowID, (*j).m_uiDocumentID + uiLastDocumentID);
			else
				m_pRowIDVectorFile2->insert(
					(*j).m_uiRowID, (*j).m_uiDocumentID + uiLastDocumentID,
					unit);
		}

		// 挿入するユニットをチェックする
		m_pDocumentIDVectorFile->checkInsertUnit();

		flushAllPages();
	}
	catch (...)
	{
		// 内容を破棄する
		recoverAllPages();
		_SYDNEY_RETHROW;
	}

#if 0
	SydMessage << "End Merge" << ModEndl;
#endif
}

//
//  FUNCTION public
//  Inverted::InvertedFile::startVerification -- 整合性検査を開始する
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//  Admin::Verification::Treatment::Value uiTreatment_
//	  動作
//  Admin::Verification::Progress& cProgress_
//	  経過
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::startVerification(
	const Trans::Transaction& cTransaction_,
	Admin::Verification::Treatment::Value uiTreatment_,
	Admin::Verification::Progress& cProgress_)
{
	int step = 0;
	try
	{
		m_pDocumentIDVectorFile->startVerification(
			cTransaction_, uiTreatment_, cProgress_);

		step++;

		getRowIDVector()->startVerification(
			cTransaction_, uiTreatment_, cProgress_);

		step++;
		for (int i = 0; i < m_iMaxUnitCount; ++i)
		{
			m_pInvertedUnit[i].startVerification(
				cTransaction_, uiTreatment_, cProgress_);
			step++;
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		try
		{
			for (int i = 0; i < m_iMaxUnitCount && step >= 3; ++i)
			{
				m_pInvertedUnit[i].endVerification();
				step--;
			}
			if (step >= 2)
			{
				getRowIDVector()->endVerification();
				step--;
			}
			if (step)
			{
				m_pDocumentIDVectorFile->endVerification();
			}
		}
		catch(...)
		{
			Schema::File::setAvailability(m_cFileID.getLockName(), false);
		}
		_SYDNEY_RETHROW;
	}

	m_pTransaction = &cTransaction_;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::endVerification -- 整合性検査を終了する
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
InvertedFile::endVerification()
{
	m_pDocumentIDVectorFile->endVerification();
	getRowIDVector()->endVerification();
	for (int i = 0; i < m_iMaxUnitCount; ++i)
		m_pInvertedUnit[i].endVerification();

	m_pTransaction = 0;
}

//
//  FUNCTION public
//  Inverted::InvertedFile::verifyVectorFile -- ベクターファイルの整合性を検査する
//
//  NOTES
//
//  ARGUMENTS
//  Admin::Verification::Treatment::Value uiTreatment_
//	  動作
//  Admin::Verification::Progress& cProgress_
//	  経過
//  const Os::Path& cRootPath_
//	  ルートパス
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
InvertedFile::verifyVectorFile(
	Admin::Verification::Treatment::Value uiTreatment_,
	Admin::Verification::Progress& cProgress_,
	const Os::Path& cRootPath_)
{
	{
		ModUInt64 ulDocumentLength = 0;
		DocumentIDVectorFile::Iterator i = m_pDocumentIDVectorFile->begin();
		for (; i != m_pDocumentIDVectorFile->end(); ++i)
		{
			ulDocumentLength += (*i).value.second;
			// ROWIDが格納されているかチェックする
			ModUInt32 uiDocumentID;
			ModInt32 dummy;
			if (m_pRowIDVectorFile && m_pRowIDVectorFile->find(
					(*i).value.first, uiDocumentID) == false)
			{
				_SYDNEY_VERIFY_INCONSISTENT(
					cProgress_, cRootPath_,
					Message::CantConvertToTupleID((*i).value.first));
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}
			else if (m_pRowIDVectorFile2 && m_pRowIDVectorFile2->find(
						 (*i).value.first, uiDocumentID, dummy) == false)
			{
				_SYDNEY_VERIFY_INCONSISTENT(
					cProgress_, cRootPath_,
					Message::CantConvertToTupleID((*i).value.first));
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}
			if (uiDocumentID != (*i).key)
			{
				_SYDNEY_VERIFY_INCONSISTENT(
					cProgress_, cRootPath_,
					Message::InconsistentDocIDandTupleID((*i).key,
														 (*i).value.first));
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}
		}
		if (ulDocumentLength
			!= m_pDocumentIDVectorFile->getTotalDocumentLength())
		{
			_SYDNEY_VERIFY_INCONSISTENT(
				cProgress_, cRootPath_, Message::IllegalTotalDocumentLength());
			_SYDNEY_THROW0(Exception::VerifyAborted);
		}
	}
	if (isCancel() == true)
	{
		_SYDNEY_THROW0(Exception::Cancel);
	}
	if (m_pRowIDVectorFile)
	{
		RowIDVectorFile::Iterator i = m_pRowIDVectorFile->begin();
		for (; i != m_pRowIDVectorFile->end(); ++i)
		{
			// 文書IDが格納されているかチェックする
			ModUInt32 uiRowID;
			ModUInt32 uiDocumentLength;
			if (m_pDocumentIDVectorFile->find(
				(*i).value, uiRowID, uiDocumentLength) == false)
			{
				_SYDNEY_VERIFY_INCONSISTENT(
					cProgress_, cRootPath_,
					Message::CantConvertToDocID((*i).value));
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}
			if (uiRowID != (*i).key)
			{
				_SYDNEY_VERIFY_INCONSISTENT(
					cProgress_,
					cRootPath_,
					Message::InconsistentDocIDandTupleID((*i).value, (*i).key));
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}
		}
	}
	else
	{
		RowIDVectorFile2::Iterator i = m_pRowIDVectorFile2->begin();
		for (; i != m_pRowIDVectorFile2->end(); ++i)
		{
			// 文書IDが格納されているかチェックする
			ModUInt32 uiRowID;
			ModUInt32 uiDocumentLength;
			if (m_pDocumentIDVectorFile->find(
				(*i).value.first, uiRowID, uiDocumentLength) == false)
			{
				_SYDNEY_VERIFY_INCONSISTENT(
					cProgress_, cRootPath_,
					Message::CantConvertToDocID((*i).value.first));
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}
			if (uiRowID != (*i).key)
			{
				_SYDNEY_VERIFY_INCONSISTENT(
					cProgress_,
					cRootPath_,
					Message::InconsistentDocIDandTupleID(
						(*i).value.first, (*i).key));
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}
		}
	}
}

//
//  FUNCTION public
//  Inverted::InvertedFile::getUnitCount -- ユニットの登録文書数を得る
//
//  NOTES
//
//  ARGUMENTS
//  int unit_
//	  ユニット番号
//
//  RETURN
//  ModSize
//	  登録文書数
//
//  EXCEPTIONS
//
ModSize
InvertedFile::getUnitCount(int unit_)
{
	return m_pDocumentIDVectorFile->getUnitDocumentCount(unit_);
}

//
//  FUNCTION private
//  Inverted::InvertedFile::checkLanguageSet -- 言語指定をチェックする
//
//  NOTES
//
//  ARGUMENTS
//  ModSize iSize_
//	  セクション数
//  cosnt ModVector<ModLanguageSet>& vecLanguage_
//	  言語指定
//
//  RETURN
//  ModVector<ModLanguageSet>
//	  正しい言語指定
//
//  EXCEPTIONS
//
ModVector<ModLanguageSet>
InvertedFile::checkLanguageSet(ModSize iSize_,
								 const ModVector<ModLanguageSet>& vecLanguage_)
{
	ModVector<ModLanguageSet> vecLanguage = vecLanguage_;
	ModSize i = vecLanguage.getSize();
	for (; i < iSize_; ++i)
	{
		vecLanguage.pushBack(getDefaultLanguageSet());
	}
	return vecLanguage;
}

//
//  FUNCTION private
//  Inverted::InvertedFile::getRowIDVector -- ROWIDベクターファイルを得る
//
//  NOTES
//
Inverted::File*
InvertedFile::getRowIDVector()
{
	if (m_pRowIDVectorFile)
		return m_pRowIDVectorFile;
	return m_pRowIDVectorFile2;
}
const Inverted::File*
InvertedFile::getRowIDVector() const
{
	if (m_pRowIDVectorFile)
		return m_pRowIDVectorFile;
	return m_pRowIDVectorFile2;
}

//
//  FUNCTION private
//  Inverted::InvertedFile::openUnit -- 検索に必要なユニットのみオープンする
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
InvertedFile::openUnit() const
{
	if (m_bOpenUnit == false)
	{
		for (int i = 0; i < m_iMaxUnitCount; ++i)
		{
			if (m_pDocumentIDVectorFile->isInserted(i) == true)
			{
				// データが挿入されているので、オープンする
				m_pInvertedUnit[i].open(*m_pTransaction, m_eFixMode);
			}
		}
		m_bOpenUnit = true;
	}
}

//
//  FUNCTION private
//  Inverted::InvertedFile::makeBatchList -- BatchListを作成する
//
//  NOTES
//
//  ARGUMENTS
//  InvertedUnit* pInvertedUnit_
//	const ModUnicodeString& cstrKey_
//
//  RETURN
//  BatchBaseList*
//
//  EXCEPTIONS
//
BatchBaseList*
InvertedFile::makeBatchList(InvertedUnit* pInvertedUnit_,
							const ModUnicodeString& cstrKey_)
{
	//
	// 該当するキーがm_pBatchListMapに存在しない時に使われるので、
	// 引数にキーが必要。
	//
	
	BatchBaseList* pList = 0;

	if (isNolocation() == true)
	{
		if (isNoTF() == true)
		{
			pList = new BatchNolocationNoTFList(
				*pInvertedUnit_, *m_pBatchListMap, cstrKey_);
		}
		else
		{
			pList = new BatchNolocationList(
				*pInvertedUnit_, *m_pBatchListMap, cstrKey_);
		}
	}
	else
	{
		pList = new BatchList(*pInvertedUnit_, *m_pBatchListMap, cstrKey_);
	}

	return pList;
}

//
//  Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
