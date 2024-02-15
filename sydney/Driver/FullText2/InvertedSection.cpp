// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedSection.cpp --
// 
// Copyright (c) 2010, 2011, 2014, 2017, 2023 Ricoh Company, Ltd.
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
#include "FullText2/InvertedSection.h"

#include "FullText2/DelayListManager.h"
#include "FullText2/DummyListManager.h"
#include "FullText2/FakeError.h"
#include "FullText2/FileID.h"
#include "FullText2/IDVectorFile.h"
#include "FullText2/InvertedBatch.h"
#include "FullText2/InvertedExpungeBatch.h"
#include "FullText2/InvertedExpungeUnit.h"
#include "FullText2/InvertedList.h"
#include "FullText2/InvertedUnit.h"
#include "FullText2/InvertedMultiUnit.h"
#include "FullText2/ListManager.h"
#include "FullText2/OtherInformationFile.h"
#include "FullText2/SearchInformationInSection.h"
#include "FullText2/Parameter.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "Schema/File.h"

#include "Exception/BadArgument.h"
#include "Exception/Cancel.h"
#include "Exception/VerifyAborted.h"
#include "Exception/Unexpected.h"

#include "ModAutoPointer.h"
#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	namespace _Path
	{
	
		//
		//	VARIABLE local
		//	_$$::_Path::_FullInvert -- 大転置のパス
		//
		ModUnicodeString _FullInvert("FullInvert");

		//
		//	VARIABLE local
		//	_$$::_Path::_Insert0 -- 小転置のパス
		//	_$$::_Path::_Insert1
		//	_$$::_Path::_Expunge0
		//	_$$::_Path::_Expunge1
		//
		ModUnicodeString _Insert0("InsertInvert");
		ModUnicodeString _Insert1("InsertInvert1");
		ModUnicodeString _Expunge0("DeleteInvert");
		ModUnicodeString _Expunge1("DeleteInvert1");

		//
		//	VARIABLE local
		//	_$$::_Path::_ExpungeFlag -- 削除フラグのパス
		//
		ModUnicodeString _ExpungeFlag("DeleteFlag");
	};
	
	//
	//	VARIABLE local
	//	_$$::_InsertMergeFileSize -- マージを開始する小転置のファイルサイズ
	//	_$$::_ExpungeMergeFileSize -- マージを開始する小転置のファイルサイズ
	//
	//	NOTES
	//
	ParameterInteger _InsertMergeFileSize(
		"FullText_InsertMergeFileSize", 128 << 20);
	ParameterInteger _ExpungeMergeFileSize(
		"FullText_ExpungeMergeFileSize", 128 << 20);

	//
	//	VARIABLE local
	//	_$$::_IsAsyncMerge -- 非同期マージを行うかどうか
	//
	ParameterBoolean _IsAsyncMerge("FullText_IsAsyncMerge", true, false);

	//
	//	CLASS
	//	_$$::_AutoDetachPage
	//
	class _AutoDetachPage
	{
	public:
		_AutoDetachPage(InvertedSection& cFile_) : m_cFile(cFile_)
		{
		}
		~_AutoDetachPage()
		{
			recover();
		}
		void flush()
		{
			m_cFile.flushAllPages();
		}
		void recover()
		{
			m_cFile.recoverAllPages();
		}

	private:
		InvertedSection& m_cFile;
	};

}

//
//	FUNCTION public
//	FullText2::InvertedSection::InvertedSection -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::FullTextFile& cFile_
//		全文索引ファイル
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
InvertedSection::InvertedSection(FullTextFile& cFile_,
								 const Os::Path& cPath_,
								 bool bBatch_)
	: InvertedFile(cFile_.getFileID(), cPath_), m_cFile(cFile_),
	  m_pTransaction(0), m_eFixMode(Buffer::Page::FixMode::Unknown),
	  m_pFull(0), m_pInsert0(0), m_pInsert1(0),
	  m_pExpunge0(0), m_pExpunge1(0), m_pOtherFile(0), m_pExpungeFlag(0),
	  m_pExpungeIDs(0),
	  m_pBatchInsert(0), m_pBatchExpunge(0),
	  m_bBatch(bBatch_), m_pMergeData(0),
	  m_uiExpungeSmallID(UndefinedDocumentID), m_pExpungeFile(0)
{
	attach(bBatch_);
}

//
//	FUNCTION public
//	FullText2::InvertedSection::~InvertedSection -- デストラクタ
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
InvertedSection::~InvertedSection()
{
	detach();
}

//
//	FUNCTION public
//	FullText2::InvertedSection::getCount -- 登録件数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		登録件数
//
//	EXCEPTIONS
//
ModUInt32
InvertedSection::getCount()
{
	ModUInt32 count = 0;
	if (isMounted(*m_pTransaction))
		count = m_pOtherFile->getCount();
	return count;
}

//
//	FUNCTION public
//	FullText2::InvertedSection::open -- オープンする
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
InvertedSection::open(const Trans::Transaction& cTransaction_,
					  Buffer::Page::FixMode::Value eFixMode_)
{
	m_pTransaction = &cTransaction_;
	m_eFixMode = eFixMode_;
	
	InvertedFile::open(cTransaction_, eFixMode_);

	if (m_bBatch && isMounted(cTransaction_))
	{
		_AutoDetachPage cAuto(*this);
		
		// バッチインサートの場合、ここで小転置をマージする
		syncMerge();

		cAuto.flush();
	}
}

//
//	FUNCTION public
//	FullText2::InvertedSection::close -- クローズする
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
InvertedSection::close()
{
	InvertedFile::close();

	if (m_pExpungeIDs) delete m_pExpungeIDs, m_pExpungeIDs = 0;
	m_pTransaction = 0;
	m_eFixMode = Buffer::Page::FixMode::Unknown;
}

//
//	FUNCTION public
//	FullText2::InvertedSection::move -- ファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Os::Path& cNewPath_
//		パス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedSection::move(const Trans::Transaction& cTransaction_,
					  const Os::Path& cNewPath_)
{
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく移動する
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	bool accessible = (isAccessible() &&
					   Os::Path::compare(cNewPath_, m_cPath)
					   == Os::Path::CompareResult::Unrelated);
	int step = 0;
	Os::Path path;
	try
	{
		{
			// 大転置
			path = cNewPath_;
			path.addPart(_Path::_FullInvert);
			m_pFull->move(cTransaction_, path);
		}
		step++;
		if (m_pInsert0)
		{
			// 挿入用小転置
			path = cNewPath_;
			path.addPart(_Path::_Insert0);
			m_pInsert0->move(cTransaction_, path);
		}
		step++;
		if (m_pInsert1)
		{
			// 挿入用小転置
			path = cNewPath_;
			path.addPart(_Path::_Insert1);
			m_pInsert1->move(cTransaction_, path);
		}
		step++;
		if (m_pExpunge0)
		{
			// 削除用小転置
			path = cNewPath_;
			path.addPart(_Path::_Expunge0);
			m_pExpunge0->move(cTransaction_, path);
		}
		step++;
		if (m_pExpunge1)
		{
			// 削除用小転置
			path = cNewPath_;
			path.addPart(_Path::_Expunge1);
			m_pExpunge1->move(cTransaction_, path);
		}
		step++;
		if (m_pExpungeFlag)
		{
			// 削除フラグ
			path = cNewPath_;
			path.addPart(_Path::_ExpungeFlag);
			m_pExpungeFlag->move(cTransaction_, path);
		}
		step++;
		{
			// その他情報ファイル
			m_pOtherFile->move(cTransaction_, cNewPath_);
		}
		step++;

		if (accessible)
			rmdir(m_cPath);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
#ifdef SYD_FAKE_ERROR
		FakeErrorMessage << "InvertedSection::move (step="
						 << step << ")" << ModEndl;
#endif
		try
		{
			switch (step)
			{
			case 7:
				{
					// その他情報ファイル
					m_pOtherFile->move(cTransaction_, m_cPath);
				}
			case 6:
				if (m_pExpungeFlag)
				{
					// 削除フラグ
					path = m_cPath;
					path.addPart(_Path::_ExpungeFlag);
					m_pExpungeFlag->move(cTransaction_, path);
				}
			case 5:
				if (m_pExpunge1)
				{
					// 削除用小転置
					path = m_cPath;
					path.addPart(_Path::_Expunge1);
					m_pExpunge1->move(cTransaction_, path);
				}
			case 4:
				if (m_pExpunge0)
				{
					// 削除用小転置
					path = m_cPath;
					path.addPart(_Path::_Expunge0);
					m_pExpunge0->move(cTransaction_, path);
				}
			case 3: 
				if (m_pInsert1)
				{
					// 挿入用小転置
					path = m_cPath;
					path.addPart(_Path::_Insert1);
					m_pInsert1->move(cTransaction_, path);
				}
			case 2:
				if (m_pInsert0)
				{
					// 挿入用小転置
					path = m_cPath;
					path.addPart(_Path::_Insert0);
					m_pInsert0->move(cTransaction_, path);
				}
			case 1:
				{
					// 大転置
					path = m_cPath;
					path.addPart(_Path::_FullInvert);
					m_pFull->move(cTransaction_, path);
				}
			}
			
			if (accessible)
				rmdir(cNewPath_);
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(getLockName(), false);
		}

		_SYDNEY_RETHROW;
	}

	m_cPath = cNewPath_;
}

//
//	FUNCTION public
//	FullText2::InvertedSection::verify -- 整合性検査を行う
//
//	NOTES
//
//	ARGUMENS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Admin::Verification::Treatment::Value eTreatment_
//		処理方法
//	Admin::Verification::Progress& cProgress_
//		経過報告
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedSection::verify(const Trans::Transaction& cTransaction_,
						const Admin::Verification::Treatment::Value eTreatment_,
						Admin::Verification::Progress& cProgress_)
{
	if (isMounted(cTransaction_))
	{
		// 下位ファイルのverifyを実行する
		InvertedFile::verify(cTransaction_, eTreatment_, cProgress_);
	}
}

//
//	FUNCTION public
//	FullText2::InvertedSection::insert -- 文書を挿入する
//
//	NOTES
//
//	ARGUEMNTS
//	const ModVector<ModUnicodeString>& vecDocument_
//		文書データ
//	const ModVector<ModLanaugageSet>& vecLanguage_
//		言語データ
//	FullText2::DocumentID uiDocumentID_
//		文書ID
//	double dblScoreData_
//		スコア調整カラム
//
//	RETURN
//	bool
//		マージデーモンへのリクエストが必要な場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InvertedSection::insert(const ModVector<ModUnicodeString>& vecDocument_,
						const ModVector<ModLanguageSet>& vecLanguage_,
						DocumentID uiDocumentID_,
						double dblScoreData_)
{
	_AutoDetachPage cAuto(*this);
	
	// 引数の確認
	if (vecDocument_.getSize() != vecLanguage_.getSize() &&
		vecLanguage_.getSize() != 1)
	{
		_TRMEISTER_THROW0(Exception::BadArgument);
	}
	
	// マウントされているか確認し、されていなかったら
	// 必要最小限のファイルを作成する
	if (isMounted(*m_pTransaction) == false)
	{
		substantiate();
	}
	
	// トークナイザーを得る
	Tokenizer::AutoPointer pTokenizer = m_cFile.getTokenizer();

	// 特徴語を抽出する必要があるか
	if (m_cFileID.isClustering())
	{
		// 必要なパラメータを設定する
		pTokenizer->setFeatureParameter(m_cFileID.getFeatureSize(),
										m_cFile.getTermResource());
	}

	// トークナイズする
	SmartLocationListMap cResult;		// トークナイズ結果
	ModVector<ModSize> vecSize;			// 正規化後の文書長
	ModVector<ModSize> vecOriginalSize;	// 正規化前の文書長
	pTokenizer->tokenize(vecDocument_, vecLanguage_,
						 cResult, vecSize, vecOriginalSize);

	// 挿入用の転置を得る
	// 大転置小転置方式の場合にはエグゼキュータ側の小転置になり、
	// バッチモードの場合はにはバッチ用転置になる
	// このメモリは解放する必要なし

	int iUnitNumber = -1;
	InvertedUpdateFile* pInvertedFile = getInsertFile(iUnitNumber);

	if (pInvertedFile->isMounted(*m_pTransaction) == false)
	{
		// まだファイルが作成されていないので、作成する
		pInvertedFile->create();
	}

	// トークナイズ結果を転置ファイルユニットに挿入する
	insertLocationList(pInvertedFile, uiDocumentID_, cResult);

	try
	{
		// 特徴語を取得する
		FeatureList vecFeatureList;
		pTokenizer->getFeatureList(vecFeatureList);

		// その他情報ファイルにその他の情報を登録する
		m_pOtherFile->insert(uiDocumentID_,
							 vecSize,
							 vecOriginalSize,
							 iUnitNumber,
							 dblScoreData_,
							 vecFeatureList);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
#ifdef SYD_FAKE_ERROR
		FakeErrorMessage << "InvertedSection::insert1" << ModEndl;
#endif
		cAuto.recover();
		
		try
		{
			// 登録した内容を削除する
			expungeLocationList(pInvertedFile, uiDocumentID_, cResult);

			// IDブロックの削除をUDOするためのログは不要なので、削除する
			pInvertedFile->clearDeleteIdBlockUndoLog();
			// 次に削除できるIDブロックがあれば削除する
			pInvertedFile->expungeIdBlock();
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			// もう回復できない...
			Schema::File::setAvailability(m_cFileID.getLockName(), false);

			try {
				// 例外を無視する
				
				pInvertedFile->recoverAllPages();
				
			} catch (...) {}
		}

		cAuto.flush();
		_SYDNEY_RETHROW;
	}

	cAuto.flush();
	bool needMerge = false;

	try
	{
		// マージが必要か
		//
		// ここで判断しているのは小転置の場合のみ
		// バッチインサート時はFullTextFileで判断する
		//
		needMerge = isNeedMerge();
		if (needMerge && isSyncMerge())
		{
			// マージする
			syncMerge();
			
			needMerge = false;
		}
		
		if (m_cFileID.isDelayed() == false && m_bBatch == false)
		{
			// 必要ならユニットを変更する
			changeUnit();
		}

	}
	catch (Exception::Object& e)
	{
#ifdef SYD_FAKE_ERROR
		FakeErrorMessage << "InvertedSection::insert2" << ModEndl;
#endif
		// ここでエラーが発生してもエラー処理の必要はない
		SydErrorMessage << e << ModEndl;
		SydErrorMessage << m_cFileID.getPath() << ModEndl;
		needMerge = false;
	}
#ifndef NO_CATCH_ALL
	catch (...)
	{
#ifdef SYD_FAKE_ERROR
		FakeErrorMessage << "InvertedSection::insert2" << ModEndl;
#endif
		// ここでエラーが発生してもエラー処理の必要はない
		SydErrorMessage << "Unexpected Excpetion." << ModEndl;
		SydErrorMessage << m_cFileID.getPath() << ModEndl;
		needMerge = false;
	}
#endif

	cAuto.flush();
	
	return needMerge; 
}

//
//	FUNCTION public
//	FullText2::InvertedSection::expunge -- 文書を削除する
//
//	NOTES
//
//	ARGUEMNTS
//	const ModVector<ModUnicodeString>& vecDocument_
//		文書データ
//	const ModVector<ModLanaugageSet>& vecLanguage_
//		言語データ
//	FullText2::DocumentID uiDocumentID_
//		文書ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedSection::expunge(const ModVector<ModUnicodeString>& vecDocument_,
						 const ModVector<ModLanguageSet>& vecLanguage_,
						 DocumentID uiDocumentID_)
{
	_AutoDetachPage cAuto(*this);
	
	// 引数の確認
	if (vecDocument_.getSize() != vecLanguage_.getSize() &&
		vecLanguage_.getSize() != 1)
	{
		_TRMEISTER_THROW0(Exception::BadArgument);
	}
	
	// マウントされているか確認し、されていなかったら何もしない
	if (isMounted(*m_pTransaction) == false)
	{
		return;
	}

	// トークナイズ結果
	// 削除の rollback でも利用するので、ここで宣言
	SmartLocationListMap cResult;
	
	if (m_pExpungeFlag)
	{
		// 削除フラグを利用するモードであるので、
		// トークナイズなどせず削除フラグを立てる

		if (m_pExpungeFlag->isMounted(*m_pTransaction) == false)
		{
			// まだファイルが作成されていないので、作成する
			m_pExpungeFlag->create();
		}

		ModUInt32 key = m_pExpungeFlag->getMaxKey();
		m_pExpungeFlag->insert(key + 1, uiDocumentID_);
	}
	else
	{
		// トークナイザーを得る
		Tokenizer::AutoPointer pTokenizer = m_cFile.getTokenizer();
	
		if (m_cFileID.isClustering())
		{
			// 削除には特徴語は不要なので、パラメータをクリアする
			pTokenizer->setFeatureParameter(0, 0);
		}

		// トークナイズする
		ModVector<ModSize> vecSize;			// 正規化後の文書長
		ModVector<ModSize> vecOriginalSize;	// 正規化前の文書長
		pTokenizer->tokenize(vecDocument_, vecLanguage_,
							 cResult, vecSize, vecOriginalSize);

		// 遅延更新の場合、エグゼキュータ側の挿入用小転置に挿入されている場合は
		// エグゼキュータ側の挿入用小転置から削除し、
		// それ以外の場合は、エグゼキュータ側の削除用小転置に挿入する

		// 削除用の小転置を得る
		m_pExpungeFile = getExpungeFile(uiDocumentID_);
		
		if (m_pExpungeFile)
		{
			// 削除用小転置に挿入する

			if (m_pExpungeFile->isMounted(*m_pTransaction) == false)
			{
				// まだファイルが作成されていないので、作成する
				m_pExpungeFile->create();
			}

			// 削除する文書のユニット番号を得る
			int iUnitNumber = -1;
			m_pOtherFile->getUnitNumber(uiDocumentID_, iUnitNumber);
		
			// 大転置の文書IDを削除用小転置の文書IDに変換する
			m_uiExpungeSmallID
				= m_pExpungeFile->assignDocumentID(uiDocumentID_, iUnitNumber);

			// トークナイズ結果を転置ファイルユニットに挿入する
			insertLocationList(m_pExpungeFile, m_uiExpungeSmallID, cResult);
		}
		else
		{
			// 登録されている転置ファイルユニットから削除する

			// 登録されている転置ファイルを得る
			m_pExpungeFile = getInsertedFile(uiDocumentID_);

			// トークナイズ結果を利用して転置リストからデータを削除する
			expungeLocationList(m_pExpungeFile, uiDocumentID_, cResult);
		}

		cAuto.flush();
	}

	try
	{
		// 削除する前に、Undoのためのデータを取得する
			
		m_Undo_dblScoreData = 0.0;
		m_Undo_iUnitNumber = -1;
		if (m_pOtherFile->isScoreData())
		{
			m_pOtherFile->getScoreData(uiDocumentID_, m_Undo_dblScoreData);
		}
		m_pOtherFile->getUnitNumber(uiDocumentID_, m_Undo_iUnitNumber);

		// その他情報ファイルから削除する
			
		m_pOtherFile->expunge(uiDocumentID_);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
#ifdef SYD_FAKE_ERROR
		FakeErrorMessage << "InvertedSection::expunge" << ModEndl;
#endif
		// 削除フラグの場合は、この recover で回復
		
		cAuto.recover();

		if (!m_pExpungeFlag)
		{
			try
			{
				// 普通の削除は削除の取り消しが必要

				expungeRollBack(cResult, uiDocumentID_);
			}
			catch (...)
			{
				SydErrorMessage << "Recovery failed." << ModEndl;
				Schema::File::setAvailability(getLockName(), false);
			}

			cAuto.flush();
		}
		_SYDNEY_RETHROW;
	}

	cAuto.flush();
}

//
//	FUNCTION public
//	FullText2::InvertedSection::expungeCommit -- 文書削除を確定する
//
//	NOTES
//
//	ARGUEMNTS
//	なし
//
//	RETURN
//	bool
//		マージデーモンへのリクエストが必要な場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InvertedSection::expungeCommit()
{
	_AutoDetachPage cAuto(*this);
	
	// マウントされているか確認し、されていなかったら何もしない
	if (isMounted(*m_pTransaction) == false)
	{
		return false;
	}

	if (!m_pExpungeFlag)
	{
		if (m_uiExpungeSmallID == UndefinedDocumentID)
		{
			try
			{
				// 転置ファイルから削除されている

				// IDブロックの削除をUNDOするためのログは不要なので
				// 削除する
			
				m_pExpungeFile->clearDeleteIdBlockUndoLog();

				// 次に削除できるIDブロックがあれば削除する

				m_pExpungeFile->expungeIdBlock();

			}
#ifdef NO_CATCH_ALL
			catch (Exception::Object&)
#else
			catch (...)
#endif
			{

				try
				{
					// 削除対象のIDブロック情報をクリアする

					m_pExpungeFile->clearDeleteIdBlockLog();
				}
				catch (...)
				{
					SydErrorMessage << "Recovery failed." << ModEndl;
					Schema::File::setAvailability(m_cFileID.getLockName(),
												  false);
				}

				_SYDNEY_RETHROW;
			}
		}
	}

	cAuto.flush();

	bool needMerge = false;

	try
	{
		// マージが必要か
		//
		// ここで判断しているのは小転置の場合のみ
		// バッチインサート時はFullTextFileで判断する
		//
		needMerge = isNeedMerge();
		if (needMerge && isSyncMerge())
		{
			// マージする
			syncMerge();
			
			needMerge = false;
		}
	}
	catch (Exception::Object& e)
	{
		// ここでエラーが発生してもエラー処理の必要はない
		SydErrorMessage << e << ModEndl;
		needMerge = false;
	}
#ifndef NO_CATCH_ALL
	catch (...)
	{
		// ここでエラーが発生してもエラー処理の必要はない
		SydErrorMessage << "Unexpected Excpetion." << ModEndl;
		needMerge = false;
	}
#endif

	cAuto.flush();

	m_uiExpungeSmallID = UndefinedDocumentID;
	m_pExpungeFile = 0;

	return needMerge; 
}

//
//	FUNCTION public
//	FullText2::InvertedSection::expungeRollBack -- 文書削除をキャンセルする
//
//	NOTES
//
//	ARGUEMNTS
//	const ModVector<ModUnicodeString>& vecDocument_
//		文書データ
//	const ModVector<ModLanaugageSet>& vecLanguage_
//		言語データ
//	FullText2::DocumentID uiDocumentID_
//		文書ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedSection::
expungeRollBack(const ModVector<ModUnicodeString>& vecDocument_,
				const ModVector<ModLanguageSet>& vecLanguage_,
				DocumentID uiDocumentID_)
{
	_AutoDetachPage cAuto(*this);
	
	// マウントされているか確認し、されていなかったら何もしない
	if (isMounted(*m_pTransaction) == false)
	{
		return;
	}

	// トークナイザーを得る
	Tokenizer::AutoPointer pTokenizer = m_cFile.getTokenizer();

	// 特徴語を抽出する必要があるか
	if (m_cFileID.isClustering())
	{
		// 必要なパラメータを設定する
		pTokenizer->setFeatureParameter(m_cFileID.getFeatureSize(),
										m_cFile.getTermResource());
	}
	
	// トークナイズする
	SmartLocationListMap cResult;		// トークナイズ結果
	ModVector<ModSize> vecSize;			// 正規化後の文書長
	ModVector<ModSize> vecOriginalSize;	// 正規化前の文書長
	pTokenizer->tokenize(vecDocument_, vecLanguage_,
						 cResult, vecSize, vecOriginalSize);

	if (m_pExpungeFlag)
	{
		// 削除フラグへの登録をロールバックする

		ModUInt32 key = m_pExpungeFlag->getMaxKey();
		m_pExpungeFlag->expunge(key);
	}
	else
	{
		// 転置リストの削除を取り消す
			
		expungeRollBack(cResult, uiDocumentID_);
	}

	// 特徴語を取得する
	FeatureList vecFeatureList;
	pTokenizer->getFeatureList(vecFeatureList);

	// その他情報ファイルに消してしまったその他の情報を登録する
	m_pOtherFile->insert(uiDocumentID_,
						 vecSize,
						 vecOriginalSize,
						 m_Undo_iUnitNumber,
						 m_Undo_dblScoreData,
						 vecFeatureList);

	cAuto.flush();
}

//
//	FUNCTION public
//	FullText2::InvertedSection::updateScoreData
//		-- スコア調整カラムのみを更新する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::DocumentID uiDocumentID_
//		文書ID
//	double dblScoreData_
//		スコア
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedSection::updateScoreData(DocumentID uiDocumentID_,
								 double dblScoreData_)
{
	if (isMounted(*m_pTransaction) == false)
	{
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	// 格納されているか確認する

	ModUInt32 uiLength;
	if (m_pOtherFile->getDocumentLength(uiDocumentID_, uiLength) == false)
	{
		// 対象の文書は格納されていない -> そんなことは通常ありえない
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	// 更新する
	m_pOtherFile->updateScoreData(uiDocumentID_, dblScoreData_);
}

//
//	FUNCTION public
//	FullText2::InvertedSection::updateFeatureList
//		-- 特徴語リストのみを更新する
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<ModUnicodeString>& vecDocument_
//		文書データ
//	const ModVector<ModLanguageSet>& vecLanguage_
//		言語データ
//	FullText2::DocumentID uiDocumentID_
//		文書ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedSection::
updateFeatureList(const ModVector<ModUnicodeString>& vecDocument_,
				  const ModVector<ModLanguageSet>& vecLanguage_,
				  DocumentID uiDocumentID_)
{
	// 引数の確認
	if (m_cFileID.isClustering() == false)
	{
		_TRMEISTER_THROW0(Exception::BadArgument);
	}
	
	if (vecDocument_.getSize() != vecLanguage_.getSize() &&
		vecLanguage_.getSize() != 1)
	{
		_TRMEISTER_THROW0(Exception::BadArgument);
	}
	
	if (isMounted(*m_pTransaction) == false)
	{
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	// 格納されているか確認する

	ModUInt32 uiLength;
	if (m_pOtherFile->getDocumentLength(uiDocumentID_, uiLength) == false)
	{
		// 対象の文書は格納されていない -> そんなことは通常ありえない
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	// トークナイザーを得る
	Tokenizer::AutoPointer pTokenizer = m_cFile.getTokenizer();
	
	// 必要なパラメータを設定する
	pTokenizer->setFeatureParameter(m_cFileID.getFeatureSize(),
									m_cFile.getTermResource());

	// トークナイズする
	SmartLocationListMap cResult;		// トークナイズ結果
	ModVector<ModSize> vecSize;			// 正規化後の文書長
	ModVector<ModSize> vecOriginalSize;	// 正規化前の文書長
	pTokenizer->tokenize(vecDocument_, vecLanguage_,
						 cResult, vecSize, vecOriginalSize);

	// 特徴語を取得する
	FeatureList vecFeatureList;
	pTokenizer->getFeatureList(vecFeatureList);

	// 更新する
	m_pOtherFile->updateFeatureList(uiDocumentID_,
									vecFeatureList);
	
}

//
//	FUNCTION public
//	FullText2::InvertedSection::check
//		-- 与えられたデータが格納されているか確認する
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<ModUnicodeString>& vecDocument_
//		文書データ
//	const ModVector<ModLanguageSet>& vecLanguage_
//		言語データ
//	FullText2::DocumentID uiDocumentID_
//		文書ID
//
//	RETURN
//	bool
//		格納されている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InvertedSection::check(const ModVector<ModUnicodeString>& vecDocument_,
					   const ModVector<ModLanguageSet>& vecLanguage_,
					   DocumentID uiDocumentID_)
{
	bool result = false;

	if (isMounted(*m_pTransaction) == false)
	{
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	// トークナイザーを得る
	Tokenizer::AutoPointer pTokenizer = m_cFile.getTokenizer();
	
	// 特徴語を抽出する必要があるか
	if (m_cFileID.isClustering())
	{
		// 必要なパラメータを設定する
		pTokenizer->setFeatureParameter(m_cFileID.getFeatureSize(),
										m_cFile.getTermResource());
	}

	// トークナイズする
	SmartLocationListMap cResult;		// トークナイズ結果
	ModVector<ModSize> vecSize;			// 正規化後の文書長
	ModVector<ModSize> vecOriginalSize;	// 正規化前の文書長
	pTokenizer->tokenize(vecDocument_, vecLanguage_,
						 cResult, vecSize, vecOriginalSize);

	// 指定の文書IDを持つ文書が挿入されているファイルを得る
	InvertedUpdateFile* pFile = getInsertedFile(uiDocumentID_);

	// ListManagerを得る
	ModAutoPointer<UpdateListManager> pListManager
		= pFile->getUpdateListManager();

	// トークナイズ結果と一致しているか確認する

	SmartLocationListMap::Iterator i = cResult.begin();
	for (; i != cResult.end(); ++i)
	{
		// 索引単位ごとに確認していく

		if (pListManager->reset((*i).first,
								ListManager::AccessMode::Search) == false)
			// 該当する転置リストがない
			break;

		// 指定の文書IDが格納されているか、
		// さらに、位置情報があっているかを確認する

		if (pListManager->check(uiDocumentID_, (*i).second) == false)
			// あっていない
			break;
	}

	if (i == cResult.end())
		// 格納されていた
		result = true;

	return result;
}

//
//	FUNCTION public
//	FullText2::InvertedSection::syncMerge -- 同期マージを行う
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
InvertedSection::syncMerge(bool bNoException_)
{
	if (isMounted(*m_pTransaction) == false)
	{
		// マウントされていないということはマージするものは何もないということ
		return;
	}
	
	if (m_pOtherFile->isProceeding())
	{
		// マージデーモンによるマージ中なので、
		// まずは、マージデーモン側の小転置をマージする

		InvertedUpdateFile* pInsertFile = getMergeInsertFile();
		InvertedUpdateFile* pExpungeFile = getMergeExpungeFile();

		// マージする
		mergeFile(pInsertFile, pExpungeFile);

		// 小転置の中身をクリアする
		pInsertFile->clear();
		if (pExpungeFile) pExpungeFile->clear();

		// マージ終了
		m_pOtherFile->mergeDone();
	}

	if (m_cFileID.isDelayed())
	{
		// 小転置を入れ替え、マージ中にする

		m_pOtherFile->flip();
	
		// マージデーモン側の小転置をマージする

		InvertedUpdateFile* pInsertFile = getMergeInsertFile();
		InvertedUpdateFile* pExpungeFile = getMergeExpungeFile();

		// マージする
		mergeFile(pInsertFile, pExpungeFile);

		// 小転置の中身をクリアする
		pInsertFile->clear();
		if (pExpungeFile) pExpungeFile->clear();

		// マージ終了
		m_pOtherFile->mergeDone();
	}
		
	if (m_pBatchInsert)
	{
		// マージ中にする

		m_pOtherFile->flip();

		try
		{
			// バッチモードなので、それをマージする

			mergeFile(m_pBatchInsert, m_pBatchExpunge);
		}
		catch (...)
		{
			// 小転置の中身をクリアする
			m_pBatchInsert->clear();
			m_pBatchExpunge->clear();

			// マージ終了
			m_pOtherFile->mergeDone();

			if (bNoException_) return;

			_SYDNEY_RETHROW;
		}
		
		// 小転置の中身をクリアする
		m_pBatchInsert->clear();
		m_pBatchExpunge->clear();

		// マージ終了
		m_pOtherFile->mergeDone();
	}
	
	// 必要ならユニットを変更する
	changeUnit();
}

//
//	FUNCTION public
//	FullText2::InvertedSection::openForMerge -- マージのためにオープンする
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
InvertedSection::openForMerge(const Trans::Transaction& cTransaction_)
{
	_AutoDetachPage cAuto(*this);
	
	open(cTransaction_,
		 Buffer::Page::FixMode::Write | Buffer::Page::FixMode::Discardable);

	// マージ中に必要なデータを確保する
	m_pMergeData = new MergeData(m_pFull->getUnitCount());

	// 小転置を入れ替え、マージ中にする
	m_pOtherFile->flip();

	// すべての変更を確定する
	cAuto.flush();
}

//
//	FUNCTION public
//	FullText2::InvertedSection::closeForMerge -- マージのためのクローズ処理
//
//	NOTES
//
//	ARGUMENTS
//	bool success_
//		マージが成功したかどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedSection::closeForMerge(bool success_)
{
	_AutoDetachPage cAuto(*this);

	if (success_ && m_pOtherFile->isProceeding())
	{
		// マージ中に、エグゼキュータ側で同期マージが実行されると、
		// isProcessding() が false となるので、それ以外の場合に終了処理を行う

		// 小転置をクリアする
		getMergeInsertFile()->clear();
		if (getMergeExpungeFile()) getMergeExpungeFile()->clear();

		// マージを終了する
		m_pOtherFile->mergeDone();
	
		// ユニットに変更がないか確認する
		changeUnit();
	}
	
	// マージ用のデータを解放する
	delete m_pMergeData, m_pMergeData = 0;

	// すべての変更を確定する
	cAuto.flush();
	
	close();
}

//
//	FUNCTION public
//	FullText2::InvertedSection::mergeList -- １つの転置リストをマージする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		続きがある場合にはtrue、それ以外の場合にはfalse
//
//	EXCEPTIONS
//
bool
InvertedSection::mergeList()
{
	//【注意】高速化のため、マージ処理を変更した
	//
	// 多くのページを更新する削除を、ユニット単位で実施し、
	// その後、索引単位ごとに挿入する
	
	if (m_pMergeData->m_bInsDone && m_pMergeData->m_bExpDone)
		// 終了している
		return false;

	_AutoDetachPage cAuto(*this);

	if (m_pMergeData->m_iUnitCount > 1 &&
		m_pMergeData->m_bExpDone == false)
	{
		if (mergeExpungeList() == true)
		{
			// すべての変更を確定する
			cAuto.flush();

			return true;
		}
	}
	
	for (;;)
	{
		// マージ対象の転置リスト
		InvertedList* pInsList = 0;
		InvertedList* pExpList = 0;

		// リストマネージャー
		ModAutoPointer<UpdateListManager> pInsManager;
		ModAutoPointer<UpdateListManager> pExpManager;

		if (m_pMergeData->m_bInsDone == false)
		{
			// マージデーモン側の挿入用の小転置を得る
			InvertedUpdateFile* file = getMergeInsertFile();
			// リストマネージャーを得る
			pInsManager = file->getUpdateListManager();
			// 転置リストを得る
			if (pInsManager->reset(m_pMergeData->m_cKey,
								   ListManager::AccessMode::LowerBound)
				== false)
				m_pMergeData->m_bInsDone = true;
		}

		if (m_pMergeData->m_bExpDone == false)
		{
			// マージデーモン側の削除用の小転置を得る
			InvertedUpdateFile* file = getMergeExpungeFile();
			if (file == 0)
			{
				// 削除用の小転置は存在しない
				m_pMergeData->m_bExpDone = true;
			}
			else
			{
				// リストマネージャーを得る
				pExpManager = file->getUpdateListManager();
				// 転置リストを得る
				if (pExpManager->reset(m_pMergeData->m_cKey,
									   ListManager::AccessMode::LowerBound)
					== false)
					m_pMergeData->m_bExpDone = true;
			}
		}

		if (m_pMergeData->m_bInsDone && m_pMergeData->m_bExpDone)
		{
			// マージ中に、エグゼキュータ側で同期マージが実行されると、
			// ここに来る可能性がある
			
			break;
		}

		if (m_pMergeData->m_bInsDone == false)
		{
			// 挿入用小転置の処理が終わっていない
			
			if (m_pMergeData->m_bExpDone ||
				pInsManager->getKey() <= pExpManager->getKey())
			{
				pInsList = pInsManager->getInvertedList();
			}
		}

		if (m_pMergeData->m_bExpDone == false)
		{
			// 削除用小転置の処理が終わっていない

			if (m_pMergeData->m_bInsDone ||
				pExpManager->getKey() <= pInsManager->getKey())
			{
				pExpList = pExpManager->getInvertedList();
			}
		}

		// マージする
		merge(pInsList, pExpList);

		// 次の索引単位を確認する
		if (pInsList)
		{
			if (pInsManager->next() == false)
			{
				// 終了
				m_pMergeData->m_bInsDone = true;
			}
		}
		if (pExpList)
		{
			if (pExpManager->next() == false)
			{
				// 終了
				m_pMergeData->m_bExpDone = true;
			}
		}
		
		if (m_pMergeData->m_bInsDone == false ||
			m_pMergeData->m_bExpDone == false)
		{
			// まだ残っているので、次の索引単位を記憶しておく
			
			if (m_pMergeData->m_bExpDone ||
				(m_pMergeData->m_bInsDone == false &&
				 pInsManager->getKey() <= pExpManager->getKey()))
			{
				m_pMergeData->m_cKey = pInsManager->getKey();
			}
			else
			{
				m_pMergeData->m_cKey = pExpManager->getKey();
			}
		}

		break;
	}

	// すべての変更を確定する
	cAuto.flush();

	return !(m_pMergeData->m_bInsDone && m_pMergeData->m_bExpDone);
}

//
//	FUNCTION public
//	FullText2::InvertedSection::isMounted -- マウントされているかどうか
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
InvertedSection::isMounted(const Trans::Transaction& cTransaction_) const
{
	return m_pOtherFile->isMounted(cTransaction_);
}

//
//	FUNCTION public
//	FullText2::InvertedSection::isAccessible
//		-- 実体である OS ファイルが存在するかどうか
//
//	NOTES
//
//	ARGUMENTS
//	bool force_ (default false)
//		強制モードかどうか
//
//	RETURN
//	bool
//		OS ファイルが存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InvertedSection::isAccessible(bool force_) const
{
	return m_pOtherFile->isAccessible(force_);
}

//
//	FUNCTION public
//	FullText2::InvertedSection::getExpungeList
//		-- 削除文書のリストを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Common::LargeVector<DocumentID>&
//		削除文書リスト
//
//	EXCEPTIONS
//
const Common::LargeVector<DocumentID>&
InvertedSection::getExpungeList()
{
	if (m_pExpungeIDs == 0)
	{
		// インスタンスを確保する
		m_pExpungeIDs = new Common::LargeVector<DocumentID>();
		// 最大値を登録する
		m_pExpungeIDs->pushBack(UndefinedDocumentID);
		
		if (m_pBatchExpunge)
		{
			// バッチモード

			m_pBatchExpunge->getAll(*m_pExpungeIDs);
		}
		else if (m_pExpungeFlag)
		{
			// 削除フラグ

			m_pExpungeFlag->getAll(*m_pExpungeIDs);
		}
		else if (m_pExpunge0)
		{
			// 遅延更新モード

			m_pExpunge0->getAll(*m_pExpungeIDs);
			m_pExpunge1->getAll(*m_pExpungeIDs);
		}

		// 文書IDの昇順にソートする
		ModSort(m_pExpungeIDs->begin(),
				m_pExpungeIDs->end(),
				ModLess<DocumentID>());
	}
	return *m_pExpungeIDs;
}

//
//
//	FUNCTION public
//	FullText2::InvertedSection::getListManager -- 検索用のListManagerを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::ListManager*
//		検索用の ListManager
//
//	EXCEPTIONS
//
ListManager*
InvertedSection::getListManager()
{
	ListManager* ret = 0;

	if (isMounted(*m_pTransaction) == false)
	{
		// ファイルが存在しないので、ダミーのListManagerを得る
		ret = new DummyListManager(m_cFile);
	}
	else if (m_pBatchInsert)
	{
		// バッチモードの場合、小転置は１つしかない

		DelayListManager* delay = new DelayListManager(m_cFile, *this);
		
		DocumentID max = m_pOtherFile->getFullMaxID();
		
		if (max != 0)
		{
			delay->pushBack(m_pFull->getListManager(), max);
		}
		
		delay->pushBack(m_pBatchInsert->getListManager());

		ret = delay;
	}
	else if (m_pInsert0 || m_pExpungeFlag)
	{
		DocumentID max = m_pOtherFile->getFullMaxID();
		DocumentID max0 = m_pOtherFile->getIns0MaxID();
		DocumentID max1 = m_pOtherFile->getIns1MaxID();

		DelayListManager* delay = new DelayListManager(m_cFile, *this);
		
		if (m_pInsert0 == 0 || (max0 == 0 && max1 == 0))
		{
			// 遅延更新ではないか、大転置にしかデータが入っていない
			
			delay->pushBack(m_pFull->getListManager());
		}
		else
		{
			if (max != 0)
			{
				delay->pushBack(m_pFull->getListManager(), max);
			}

			// 遅延更新の場合、マージ中なら小転置は２つ
		
			if (m_pOtherFile->isProceeding())
			{
				// 小転置は２つ
				if (m_pOtherFile->getIndex() == 0)
				{
					// エグゼキュータ側が0なので、1 -> 0 の順
					if (max1 != 0 && max1 != max)
						delay->pushBack(m_pInsert1->getListManager(), max1);
					delay->pushBack(m_pInsert0->getListManager());
				}
				else
				{
					// エグゼキュータ側が1なので、0 -> 1 の順
					if (max0 != 0 && max0 != max)
						delay->pushBack(m_pInsert0->getListManager(), max0);
					delay->pushBack(m_pInsert1->getListManager());
				}
			}
			else
			{
				// 小転置は１つ
				delay->pushBack(getInsertFile()->getListManager());
			}
		}

		ret = delay;
	}
	else
	{
		// 大転置のみ
		ret = m_pFull->getListManager();
	}

	return ret;
}

//
//	FUNCTION public
//	FullText2::InvertedSection::getSearchInformation
//		-- 検索情報クラスを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::SearchInformation*
//		検索情報クラス
//
//	EXCEPTIONS
//	なし
//
SearchInformation*
InvertedSection::getSearchInformation()
{
	// 削除リストには必ず UndefinedDocumentID が最後に含まれているので、
	// -1 する
	
	return new SearchInformationInSection(m_cFileID, m_pOtherFile,
										  getExpungeList().getSize() - 1);
}

//
//	FUNCTION public
//	FullText2::InvertedSection::expungeRollBack
//		-- 転置リスト部分の削除をロールバックする
//
//	NOTES
//
//	ARGUEMNTS
//	FullText2::SmartLocationListMap& cResult_
//		トークナイズ結果
//	FullText2::DocumentID uiDocumentID_
//		文書ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedSection::
expungeRollBack(SmartLocationListMap& cResult_,
				DocumentID uiDocumentID_)
{
	if (m_uiExpungeSmallID != UndefinedDocumentID)
	{
		// 小転置に挿入したので、小転置から削除する

		m_pExpungeFile->expungeIDVector(m_uiExpungeSmallID);
		expungeLocationList(m_pExpungeFile, m_uiExpungeSmallID, cResult_);

		// 削除を確定させる

		m_pExpungeFile->clearDeleteIdBlockUndoLog();
		m_pExpungeFile->expungeIdBlock();
	}
	else
	{
		// 転置ファイルから削除したので、削除を取り消す

		// 削除対象のIDブロック情報をクリアする
		m_pExpungeFile->clearDeleteIdBlockLog();

		{
			// flushAllPagesする前に LisManager を破棄する必要あり
			ModAutoPointer<UpdateListManager> pListManager
				= m_pExpungeFile->getUpdateListManager();

			SmartLocationListMap::Iterator i = cResult_.begin();

			for (; i != cResult_.end(); ++i)
			{
				// 索引単位ごとに、削除を取り消していく

				if (pListManager->reset(
						(*i).first,
						ListManager::AccessMode::Search) == true)
				{
					// 削除を取り消す
					pListManager->undoExpunge(uiDocumentID_,
											  (*i).second);

					// 必要なら索引単位ごとに確定する
					m_pExpungeFile->saveAllPages();
				}
			}
		}

		// すべての変更を確定させる
		m_pExpungeFile->flushAllPages();

		// Undoログを削除する
		m_pExpungeFile->clearDeleteIdBlockUndoLog();
	}
}

//
//	FUNCTION private
//	FullText2::InvertedSection::attach -- ファイルをアタッチする
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
InvertedSection::attach(bool bBatch_)
{
	Os::Path path;
	
	//
	//	大転置を attach する
	//

	path = m_cPath;
	path.addPart(_Path::_FullInvert);
	m_pFull = new InvertedMultiUnit(*this, path, bBatch_);
	InvertedFile::pushBackSubFile(m_pFull);
		

	if (m_cFileID.isDelayed())
	{
		//
		//	遅延更新なので、挿入用小転置と削除用小転置を２つづつ attach する
		//

		path = m_cPath;
		path.addPart(_Path::_Insert0);
		m_pInsert0 = new InvertedUnit(*this, path, bBatch_);
		InvertedFile::pushBackSubFile(m_pInsert0);

		path = m_cPath;
		path.addPart(_Path::_Insert1);
		m_pInsert1 = new InvertedUnit(*this, path, bBatch_);
		InvertedFile::pushBackSubFile(m_pInsert1);

		if (m_cFileID.isExpungeFlag() == false)
		{
			path = m_cPath;
			path.addPart(_Path::_Expunge0);
			m_pExpunge0 = new InvertedExpungeUnit(*this, path, bBatch_);
			InvertedFile::pushBackSubFile(m_pExpunge0);
			
			path = m_cPath;
			path.addPart(_Path::_Expunge1);
			m_pExpunge1 = new InvertedExpungeUnit(*this, path, bBatch_);
			InvertedFile::pushBackSubFile(m_pExpunge1);
		}
	}

	if (m_cFileID.isExpungeFlag())
	{
		//
		//	削除フラグなので、削除フラグ用のファイルを attach する
		//

		path = m_cPath;
		path.addPart(_Path::_ExpungeFlag);
		m_pExpungeFlag = new IDVectorFile(m_cFileID, path, bBatch_);
		InvertedFile::pushBackSubFile(m_pExpungeFlag);
	}

	//
	//	その他の情報ファイルを attach する
	//

	m_pOtherFile = new OtherInformationFile(m_cFileID, m_cPath, bBatch_);
	InvertedFile::pushBackSubFile(m_pOtherFile);

}

//
//	FUNCTION private
//	FullText2::InvertedSection::detach -- detach する
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
InvertedSection::detach()
{
	if (m_pFull) delete m_pFull, m_pFull = 0;
	if (m_pInsert0) delete m_pInsert0, m_pInsert0 = 0;
	if (m_pInsert1) delete m_pInsert1, m_pInsert1 = 0;
	if (m_pExpunge0) delete m_pExpunge0, m_pExpunge0 = 0;
	if (m_pExpunge1) delete m_pExpunge1, m_pExpunge1 = 0;
	if (m_pOtherFile) delete m_pOtherFile, m_pOtherFile = 0;
	if (m_pExpungeFlag) delete m_pExpungeFlag, m_pExpungeFlag = 0;
	if (m_pBatchInsert) delete m_pBatchInsert, m_pBatchInsert = 0;
	if (m_pBatchExpunge) delete m_pBatchExpunge, m_pBatchExpunge = 0;
}

//
//	FUNCTION private
//	FullText2::InvertedSection::substantiate -- ファイルを作成する
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
InvertedSection::substantiate()
{
	// 作成するのは、その他情報ファイルのみ
	m_pOtherFile->create();
}

//
//	FUNCTION private
//	FullText2::InvertedSection::isNeedMerge -- マージが必要かどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		マージが必要な場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InvertedSection::isNeedMerge()
{
	if (m_cFileID.isDelayed() && !m_bBatch)
	{
		// マージが中断されているか確認する
		if (m_pOtherFile->isCanceled())

			return true;
		
		// 遅延更新なので、サイズを確認する
		if (getInsertFile()->getUsedSize(*m_pTransaction) >=
			_InsertMergeFileSize.get() ||
			m_cFileID.isExpungeFlag() == false &&
			getExpungeFile()->getUsedSize(*m_pTransaction) >=
			_ExpungeMergeFileSize.get())

			return true;
	}

	return false;
}

//
//	FUNCTION private
//	FullText2::InvertedSection::changeUnit -- 必要ならユニットを変更する
//
//	NOTES
//
//	ARGUEMNTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedSection::changeUnit()
{
	if (m_cFileID.isDistribute() == false)
		return;

	// 現在のユニット番号を得る
	int iUnitNumber = m_pOtherFile->getInsertUnit();
	int iNextNumber = iUnitNumber;
	
	// ファイルサイズを確認する
	for (;;)
	{
		ModUInt64 size
			= m_pFull->getUnit(iNextNumber)->getUsedSize(*m_pTransaction);
		if (size < m_pOtherFile->getMaxFileSize())
			// 超えていないので終わり
			break;
		
		// 超えているので、次のユニットへ
		++iNextNumber;

		if (static_cast<ModSize>(iNextNumber) >= m_pFull->getUnitCount())
		{
			// すべてのユニットを利用したので、容量を倍にして初めから

			m_pOtherFile->updateMaxFileSize();
			iNextNumber = 0;
		}
	}

	if (iUnitNumber != iNextNumber)
	{
		// 変更されたので保存する

		m_pOtherFile->setInsertUnit(iNextNumber);
	}
}

//
//	FUNCTION private
//	FullText2::InvertedSection::insertLocationList
//		-- トークナイズ結果を転置ファイルユニットに挿入する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedUpdateFile* pInvertedFile_
//		転置ファイルユニット
//	FullText2::SmartLocationListMap& cResult_
//		トークナイズ結果
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedSection::insertLocationList(InvertedUpdateFile* pInvertedFile_,
									DocumentID uiDocumentID_,
									SmartLocationListMap& cResult_)
{
	// エラー処理で挿入したものを削除する必要があるので、
	// ここでイテレータを宣言する
	SmartLocationListMap::Iterator i = cResult_.begin();
	SmartLocationListMap::Iterator s = i;
	
	try
	{
		{
			// 挿入するために、UpdateListManagerを得る
			// flushAllPagesする前に LisManager を破棄する必要あり
			
			ModAutoPointer<UpdateListManager> pListManager
				= pInvertedFile_->getUpdateListManager();

			// 挿入する

			for (; i != cResult_.end(); ++i)
			{
				// 索引単位ごとに挿入していく

				// 挿入する索引単位の転置リストをセットする
				pListManager->reset((*i).first,
									ListManager::AccessMode::Create);

				// セットした転置リストに位置情報を挿入する
				pListManager->insert(uiDocumentID_, (*i).second);

				// 全索引単位の位置情報を挿入してから確定すると、
				// ダーティなページが多くなりすぎ、
				// メモリが不足する恐れがあるので、
				// 必要なら索引単位ごとに確定する

				if (pInvertedFile_->saveAllPages() == true)
				{
					// ここまでは確定した
					s = i;
				}
			}
		}

		// すべての変更を確定する
		pInvertedFile_->flushAllPages();
		s = i;
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
#ifdef SYD_FAKE_ERROR
		FakeErrorMessage << "InvertedSection::insertLocationList" << ModEndl;
#endif
		
		try
		{
			// すべての変更をリカバーする
			pInvertedFile_->recoverAllPages();

			if (s != cResult_.begin())
			{
				// 挿入した部分をロールバックする

				{
					// 削除するために、UpdateListManagerを得る
					ModAutoPointer<UpdateListManager> pListManager
						= pInvertedFile_->getUpdateListManager();
			
					do
					{
						--s;

						// 削除する索引単位の転置リストをセットする
						if (pListManager->reset(
								(*s).first,
								ListManager::AccessMode::Search) == true)
						{
							// 削除する
							pListManager->expunge(uiDocumentID_);
							
							// 必要なら索引単位ごとに確定する
							pInvertedFile_->saveAllPages();
						}
					}
					while (s != cResult_.begin());
				}

				// すべての変更を確定させる
				pInvertedFile_->flushAllPages();
			}
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			// もう回復できない...
			Schema::File::setAvailability(m_cFileID.getLockName(), false);

			try {
				// 例外を無視する
				
				pInvertedFile_->recoverAllPages();
				
			} catch (...) {}
		}

		if (m_pBatchInsert)
		{
			// 小転置の中身をクリアする
			m_pBatchInsert->clear();
			m_pBatchExpunge->clear();
		}

		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION private
//	FullText2::InvertedSection::expungeLocationList
//		-- トークナイズ結果を転置ファイルユニットから削除する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedUpdateFile* pInvertedFile_
//		転置ファイルユニット
//	FullText2::DocumentID uiDocumentID_
//		文書ID
//	FullText2::SmartLocationListMap& cResult_
//		トークナイズ結果
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedSection::expungeLocationList(InvertedUpdateFile* pInvertedFile_,
									 DocumentID uiDocumentID_,
									 SmartLocationListMap& cResult_)
{
	// 削除処理は、まず転置リストを走査して該当する文書のデータを１つづつ
	// 消していき、次に、その結果として空になったIDブロックを削除する
	// という２ステップで実行される
	
	// エラー処理で挿入したものを削除する必要があるので、
	// ここでイテレータを宣言する
	SmartLocationListMap::Iterator i = cResult_.begin();
	SmartLocationListMap::Iterator s = i;
	
	try
	{
		{
			// 削除するために、UpdateListManagerを得る
			// flushAllPagesする前に LisManager を破棄する必要あり
			
			ModAutoPointer<UpdateListManager> pListManager
				= pInvertedFile_->getUpdateListManager();

			// 削除する

			for (; i != cResult_.end(); ++i)
			{
				// 索引単位ごとに削除していく

				// 削除する索引単位の転置リストをセットする
				if (pListManager->reset(
						(*i).first,
						ListManager::AccessMode::Search) == true)
				{
					// 削除する
					pListManager->expunge(uiDocumentID_);

					// 必要なら索引単位ごとに確定する
					if (pInvertedFile_->saveAllPages() == true)
					{
						// ここまでは確定した
						s = i;
					}
				}
			}
		}

		// すべての変更を確定する
		pInvertedFile_->flushAllPages();
		s = i;
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
#ifdef SYD_FAKE_ERROR
		FakeErrorMessage << "InvertedSection::expungeLocationList" << ModEndl;
#endif
		// 削除対象のIDブロック情報をクリアする
		pInvertedFile_->clearDeleteIdBlockLog();

		try
		{
			// ファイルの変更を破棄する
			pInvertedFile_->recoverAllPages();

			if (s != cResult_.begin())
			{
				// 削除した部分をロールバックする

				{
					// 削除を取り消すために、UpdateListManagerを得る
					ModAutoPointer<UpdateListManager> pListManager
						= pInvertedFile_->getUpdateListManager();
			
					do
					{
						--s;

						// 削除を取り消す索引単位の転置リストをセットする
						if (pListManager->reset(
								(*s).first,
								ListManager::AccessMode::Search) == true)
						{
							// 削除を取り消す
							pListManager->undoExpunge(uiDocumentID_,
													  (*s).second);

							// 必要なら索引単位ごとに確定する
							pInvertedFile_->saveAllPages();
						}
					}
					while (s != cResult_.begin());
				}

				// すべての変更を確定させる
				pInvertedFile_->flushAllPages();

				// Undoログを削除する
				pInvertedFile_->clearDeleteIdBlockUndoLog();
			}
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			// もう回復できない...
			Schema::File::setAvailability(m_cFileID.getLockName(), false);

			try {
				// 例外を無視する
				
				pInvertedFile_->recoverAllPages();
				
			} catch (...) {}
		}

		if (m_pBatchInsert)
		{
			// 小転置の中身をクリアする
			m_pBatchInsert->clear();
			m_pBatchExpunge->clear();
		}

		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION private
//	FullText2::InvertedSection::merge -- 1つの転置リストをマージする
//
//	NOTES
//	同じ索引単位のリストである必要がある
//
//	ARGUMENTS
//	FullText2::InvertedList* pInsertList_
//		挿入する転置リスト
//	FullText2::InvertedList* pExpungeList_
//		削除する転置リスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedSection::merge(InvertedList* pInsertList_, InvertedList* pExpungeList_)
{
	//【注意】
	//	このメソッド内では、ページを unfix することはしない
	//	呼び出し側はこのメソッド実行後、
	//	大転置の saveAllPages か flushAllPages を呼ぶ必要あり
	
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
		if (pExpungeList_ != 0)
		{
			// まず削除の処理を行う

			int unitCount = static_cast<int>(m_pFull->getUnitCount());
			for (int i = 0; i < unitCount; ++i)
			{
				InvertedUnit* u = m_pFull->getUnit(i);

				if (u->isMounted(*m_pTransaction) == false)
					// 何も挿入されていないので、次へ
					continue;

				ModAutoPointer<UpdateListManager> listManager
					= u->getUpdateListManager();

				if (listManager->reset(cstrKey, ListManager::AccessMode::Search)
					== true)
				{
					// 索引単位があったので、削除する
					int n = listManager->expunge(*pExpungeList_);

					// 削除できるIDブロックがあれば削除する
					u->expungeIdBlock();

					// 削除IDブロックのUndoログをクリアする
					// マージではUndoする必要はない
					u->clearDeleteIdBlockUndoLog();

					//
					// 削除数が一定以上に達していた場合にはバキュームを実施する
					//
					
					if (listManager->isNeedVacuum(n) == true)
					{
						// バキュームを実施する
						listManager->vacuum();
						
						// 削除数をクリアする
						u->clearExpungeCount(cstrKey);
					}
				}
			}
		}

		if (pInsertList_ != 0)
		{
			// 次に挿入する

			InvertedUnit* u = m_pFull->getUnit(m_pOtherFile->getInsertUnit());

			if (u->isMounted(*m_pTransaction) == false)
			{
				// まだファイル作成されていないので、作成する
				u->create();
			}

			ModAutoPointer<UpdateListManager> listManager
				= u->getUpdateListManager();

			if (listManager->reset(cstrKey, ListManager::AccessMode::Create)
				== true)
			{
				// 挿入する
				listManager->insert(*pInsertList_);
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
		FakeErrorMessage << "InvertedSection::merge" << ModEndl;
#endif

		// 特にエラー処理は不要
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION private
//	FullText2::InvertedSection::mergeFile -- １つの転置ファイルをマージする
//
//	NOTES
//
//	ARGUMETNS
//	FullText2::InvertedUpdateFile* pInsertFile_
//		挿入用の小転置
//	FullText2::InvertedUpdateFile* pExpungeFile_
//		削除用の小転置
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedSection::mergeFile(InvertedUpdateFile* pInsertFile_,
						   InvertedUpdateFile* pExpungeFile_)
{
	//【注意】高速化のため、マージ処理を変更した
	//
	// 多くのページを更新する削除を、ユニット単位で実施し、
	// その後、索引単位ごとに挿入する
	
	try
	{
		int unitCount = static_cast<int>(m_pFull->getUnitCount());
		
		if (unitCount > 1 && pExpungeFile_)
		{
			//
			//	まずは削除のマージ
			//

			// 更新するページ数を少なく保つため、ユニットごとに処理する
			for (int i = 0; i < unitCount; ++i)
			{
				mergeExpunge(pExpungeFile_, i);
			}
		}
		
		//
		//	次に挿入のマージ
		//
		
		// リストマネージャーを得る
		ModAutoPointer<UpdateListManager> pInsertListManager
			= pInsertFile_->getUpdateListManager();
		ModAutoPointer<UpdateListManager> pExpungeListManager;
		if (unitCount <= 1 && pExpungeFile_)
			pExpungeListManager = pExpungeFile_->getUpdateListManager();

		// 転置リストを得る
		ModUnicodeString cKey;	// 最小の索引単位は空文字列
		InvertedList* pInsertList =
			(pInsertListManager->reset(cKey,
									   ListManager::AccessMode::LowerBound)) ?
			pInsertListManager->getInvertedList() : 0;
		InvertedList* pExpungeList =
			(pExpungeListManager.get() &&
			 pExpungeListManager->reset(cKey,
										ListManager::AccessMode::LowerBound)) ?
			pExpungeListManager->getInvertedList() : 0;

		while (pInsertList != 0 || pExpungeList != 0)
		{
			InvertedList* pIns = 0;
			InvertedList* pExp = 0;

			if (pInsertList == 0)
			{
				pExp = pExpungeList;
			}
			else if (pExpungeList == 0)
			{
				pIns = pInsertList;
			}
			else
			{
				// 索引単位を比較して小さい方のみを採用
				pIns = (pInsertList->getKey() <= pExpungeList->getKey()) ?
					pInsertList : 0;
				pExp = (pExpungeList->getKey() <= pInsertList->getKey()) ?
					pExpungeList : 0;
			}

			// マージする
			merge(pIns, pExp);

			// 必要なら変更内容を確定する
			m_pFull->saveAllPages();

			// マージ対象のリストの方だけ、次に進める
			if (pIns)
			{
				pInsertList = (pInsertListManager->next()) ?
					pInsertListManager->getInvertedList() : 0;
			}
			if (pExp)
			{
				pExpungeList = (pExpungeListManager->next()) ?
					pExpungeListManager->getInvertedList() : 0;
			}

			// 中断を確認する
			if (m_pTransaction->isCanceledStatement() == true)
			{
				_SYDNEY_THROW0(Exception::Cancel);
			}

			; _FULLTEXT2_FAKE_ERROR(InvertedSection::mergeFile);
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// エラーが発生したので変更を破棄する
		// 索引単位ごとに saveAllPages しているので、
		// 一部のみしか破棄されないが、それで整合性が保てるようになっている
		
		m_pFull->recoverAllPages();
		pInsertFile_->recoverAllPages();
		if (pExpungeFile_) pExpungeFile_->recoverAllPages();

		_SYDNEY_RETHROW;
	}
	
	// すべてのページを detach する
	m_pFull->flushAllPages();
	pInsertFile_->flushAllPages();
	if (pExpungeFile_) pExpungeFile_->flushAllPages();
}

//
//	FUNCTION private
//	FullText2::InvertedSection::mergeExpunge
//		-- 削除用小転置の1つの転置リストをマージする
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedList* pExpungeList_
//		削除する転置リスト
//	InvertedUnit* pUnitFile_
//		削除対象のユニット
//
//	RETURN
//	bool
//		削除した場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InvertedSection::mergeExpunge(InvertedList* pExpungeList_,
							  InvertedUnit* pUnitFile_,
							  int iUnitNumber_)
{
	//【注意】
	//	このメソッド内では、ページを unfix することはしない
	//	呼び出し側はこのメソッド実行後、
	//	大転置の saveAllPages か flushAllPages を呼ぶ必要あり

	ModUnicodeString cKey = pExpungeList_->getKey();
	bool result = false;

	try
	{
		ModAutoPointer<UpdateListManager> listManager
			= pUnitFile_->getUpdateListManager();

		if (listManager->reset(cKey, ListManager::AccessMode::Search)
			== true)
		{
			// 索引単位があったので、削除する
			int n = listManager->expunge(*pExpungeList_);

			// 削除できるIDブロックがあれば削除する
			pUnitFile_->expungeIdBlock();

			// 削除IDブロックのUndoログをクリアする
			// マージではUndoする必要はない
			pUnitFile_->clearDeleteIdBlockUndoLog();

			// 削除した
			result = true;

			//
			// 削除数が一定以上に達していた場合にはバキュームを実施する
			//
					
			if (listManager->isNeedVacuum(iUnitNumber_) == true)
			{
				// バキュームを実施する
				listManager->vacuum();
						
				// 削除数をクリアする
				pUnitFile_->clearExpungeCount(cKey);
			}
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// 特にエラー処理は不要
		_SYDNEY_RETHROW;
	}

	return result; 
}

//
//	FUNCTION private
//	FullText2::InvertedSection::mergeExpunge
//		-- 削除用の小転置の内容をユニット単位にマージする
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedUpdateFile* pExpungeFile_
//		削除用小転置
//	int iUnit_
//		マージするユニット番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedSection::mergeExpunge(InvertedUpdateFile* pExpungeFile_,
							  int iUnit_)
{
	InvertedUnit* u = m_pFull->getUnit(iUnit_);
	if (u->isMounted(*m_pTransaction) == false)
		// 何も挿入されていないので、終了
		return;

	// リストマネージャーを得る
	ModAutoPointer<UpdateListManager> pExpungeListManager
		= pExpungeFile_->getUpdateListManager();

	// 削除用小転置の先頭の索引単位を得る
	ModUnicodeString cKey;
	InvertedList* pExpungeList = 
		(pExpungeListManager->reset(cKey,
									ListManager::AccessMode::LowerBound) ?
		 pExpungeListManager->getInvertedList() : 0);

	while (pExpungeList)
	{
		// 索引単位ごとにマージする

		if (mergeExpunge(pExpungeList, u, iUnit_) == true)
		{
			// 削除したので、確定する
			u->saveAllPages();
		}

		// 中断を確認する
		if (m_pTransaction->isCanceledStatement() == true)
		{
			_SYDNEY_THROW0(Exception::Cancel);
		}
	
		// 次へ
		pExpungeList = (pExpungeListManager->next()) ?
			pExpungeListManager->getInvertedList() : 0;
	}

	// 確定する
	u->flushAllPages();
}

//
//	FUNCTION private
//	FullText2::InvertedSection::mergeExpungeList
//		-- 1つの削除転置リストを1つのユニットにマージする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		削除を実行した場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InvertedSection::mergeExpungeList()
{
	bool r = false;
	
	if (m_pMergeData->m_bExpDone == true)
		// もう終わっている
		return r;
	
	// マージデーモン側の削除用小転置を得る
	InvertedUpdateFile* file = getMergeExpungeFile();
	if (file == 0)
	{
		// 削除用の小転置は存在しない
		m_pMergeData->m_bExpDone = true;

		return r;
	}

	for (;;)
	{
		// ユニットを得る
		InvertedUnit* u = m_pFull->getUnit(m_pMergeData->m_iUnit);
	
		if (u->isMounted(*m_pTransaction) == false)
		{
			// このユニットには何も登録されていないので、次へ
			m_pMergeData->m_iUnit++;
			m_pMergeData->m_cKey = ModUnicodeString();

			if (m_pMergeData->m_iUnit == m_pMergeData->m_iUnitCount)
			{
				// すべて終了
				m_pMergeData->m_bExpDone = true;
				return r;
			}

			continue;
		}

		// リストマネージャ−を得る
		ModAutoPointer<UpdateListManager> pExpManager
			= file->getUpdateListManager();
		// 転置リストを得る
		if (pExpManager->reset(m_pMergeData->m_cKey,
							   ListManager::AccessMode::LowerBound)
			== false)
		{
			// 終了したので、次のユニットへ
			m_pMergeData->m_iUnit++;
			m_pMergeData->m_cKey = ModUnicodeString();

			if (m_pMergeData->m_iUnit == m_pMergeData->m_iUnitCount)
			{
				// すべて終了
				m_pMergeData->m_bExpDone = true;
				return r;
			}

			continue;
		}

		// 現在のユニットから削除する

		// 削除小転置の転置リストを得る
		InvertedList* pExpList = pExpManager->getInvertedList();

		// 削除する
		r = mergeExpunge(pExpList, u, m_pMergeData->m_iUnit);

		// 次の索引単位に進める
		if (pExpManager->next() == true)
		{
			m_pMergeData->m_cKey = pExpManager->getKey();
		}
		else
		{
			// もう索引単位がないので、次のユニットにする
			m_pMergeData->m_iUnit++;
			m_pMergeData->m_cKey = ModUnicodeString();

			if (m_pMergeData->m_iUnit == m_pMergeData->m_iUnitCount)
			{
				// すべて終了
				m_pMergeData->m_bExpDone = true;
				return r;
			}
		}

		if (r == true)
			// 削除したので終了
			break;
	}

	return r;
}

//
//	FUNCTION private
//	FullText2::InvertedSection::getInsertFile
//		-- 挿入用の転置ファイルユニットを得る
//
//	NOTES
//
//	ARGUMENTS
//	int& iUnitNumber_
//		挿入するユニットのユニット番号
//		小転置やファイル分散していない場合には -1 になる
//
//	RETURN
//	FullText2::InvertedUpdateFile*
//		該当する転置ファイルユニット
//
//	EXCEPTIONS
//
InvertedUpdateFile*
InvertedSection::getInsertFile(int& iUnitNumber_)
{
	InvertedUpdateFile* pFile = 0;
	iUnitNumber_ = -1;

	if (m_bBatch)
	{
		// バッチモードなので、バッチ用の転置ファイルを返す

		if (m_pBatchInsert == 0)
		{
			m_pBatchInsert = new InvertedBatch(*this);
			m_pBatchExpunge = new InvertedExpungeBatch(*this);
		}
		
		pFile = m_pBatchInsert;
	}
	else if (m_cFileID.isDelayed())
	{
		// 遅延更新なので、エグゼキュータ側の小転置を返す

		pFile = getInsertFile();
	}
	else if (m_cFileID.isDistribute())
	{
		// 大転置内の挿入ユニットを得る

		iUnitNumber_ = m_pOtherFile->getInsertUnit();
		pFile = m_pFull->getUnit(iUnitNumber_);
	}
	else
	{
		// 大転置内のユニットを得る
		
		pFile = m_pFull->getUnit(0);
	}

	return pFile;
}

//
//	FUNCTION private
//	FullText2::InvertedSection::getExpungeFile
//		-- 削除用の転置ファイルユニットを得る
//
//	NOTES
//
//	ARGUEMNTS
//	FullText2::DocumentID uiDocumentID_
//		削除する文書の文書ID
//
//	RETURN
//	FullText2::InvertedUpdateFile*
//		削除用の転置ファイルユニット
//
//	EXCEPTIONS
//
InvertedUpdateFile*
InvertedSection::getExpungeFile(DocumentID uiDocumentID_)
{
	InvertedUpdateFile* pFile = 0;

	if (m_bBatch)
	{
		// バッチモードなので、バッチ用の転置ファイルを返す

		if (m_pOtherFile->getFirstDocumentID() > uiDocumentID_)
		{
			if (m_pBatchInsert == 0)
			{
				m_pBatchInsert = new InvertedBatch(*this);
				m_pBatchExpunge = new InvertedExpungeBatch(*this);
			}
		
			pFile = m_pBatchExpunge;
		}
	}
	else if (m_cFileID.isDelayed())
	{
		// 遅延更新なので、エグゼキュータ側の小転置を返す

		if (m_pOtherFile->getFirstDocumentID() > uiDocumentID_)
		{
			pFile = getExpungeFile();
		}
	}

	return pFile;
}

//
//	FUNCTION private
//	FullText2::InvertedSection::getInsertFile
//		-- エグゼキュータ側の挿入用の転置ファイルユニットを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::InvertedUpdateFile*
//		転置ファイルユニット
//
//	EXCEPTIONS
//
InvertedUpdateFile*
InvertedSection::getInsertFile()
{
	InvertedUpdateFile* pFile = 0;
	
	if (m_cFileID.isDelayed())
	{
		// 遅延更新なので、エグゼキュータ側の小転置を返す

		pFile = (m_pOtherFile->getIndex() == 0) ? m_pInsert0 : m_pInsert1;
	}

	return pFile;
}

//
//	FUNCTION private
//	FullText2::InvertedSection::getExpungeFile
//		-- エグゼキュータ側の削除用の転置ファイルユニットを得る
//
//	NOTES
//
//	ARGUEMNTS
//	なし
//
//	RETURN
//	FullText2::InvertedUpdateFile*
//		削除用の転置ファイルユニット
//
//	EXCEPTIONS
//
InvertedUpdateFile*
InvertedSection::getExpungeFile()
{
	InvertedUpdateFile* pFile = 0;

	if (m_cFileID.isDelayed())
	{
		// 遅延更新なので、エグゼキュータ側の小転置を返す

		pFile = (m_pOtherFile->getIndex() == 0) ? m_pExpunge0 : m_pExpunge1;
	}

	return pFile;
}

//
//	FUNCTION private
//	FullText2::InvertedSection::getMergeInsertFile
//		-- マージデーモン側の挿入用の転置ファイルユニットを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::InvertedUpdateFile*
//		転置ファイルユニット
//
//	EXCEPTIONS
//
InvertedUpdateFile*
InvertedSection::getMergeInsertFile()
{
	InvertedUpdateFile* pFile = 0;
	
	if (m_cFileID.isDelayed())
	{
		// 遅延更新なので、マージデーモン側の小転置を返す

		pFile = (m_pOtherFile->getIndex() == 0) ? m_pInsert1 : m_pInsert0;
	}

	return pFile;
}

//
//	FUNCTION private
//	FullText2::InvertedSection::getMergeExpungeFile
//		-- マージデーモン側の削除用の転置ファイルユニットを得る
//
//	NOTES
//
//	ARGUEMNTS
//	なし
//
//	RETURN
//	FullText2::InvertedUpdateFile*
//		削除用の転置ファイルユニット
//
//	EXCEPTIONS
//
InvertedUpdateFile*
InvertedSection::getMergeExpungeFile()
{
	InvertedUpdateFile* pFile = 0;

	if (m_cFileID.isDelayed())
	{
		// 遅延更新なので、マージデーモン側の小転置を返す

		pFile = (m_pOtherFile->getIndex() == 0) ? m_pExpunge1 : m_pExpunge0;
	}

	return pFile;
}

//
//	FUNCTION private
//	FullText2::InvertedSection::getInsertedFile
//		-- 引数の文書IDの文書が挿入されているユニットを得る
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::DocumentID uiDocumentID_
//		文書ID
//
//	RETURN
//	FullText2::InvertedUpdateFile*
//		挿入されている転置ファイルユニット
//
//	EXCEPTIONS
//
InvertedUpdateFile*
InvertedSection::getInsertedFile(DocumentID uiDocumentID_)
{
	InvertedUpdateFile* pFile = 0;

	if (m_bBatch)
	{
		// バッチモードなので、バッチ用の転置ファイルを返す

		if (m_pOtherFile->getFirstDocumentID() <= uiDocumentID_)
		{
			pFile = m_pBatchInsert;
		}
	}
	else if (m_cFileID.isDelayed())
	{
		// 遅延更新なので、小転置を返す

		if (m_pOtherFile->getFirstDocumentID() <= uiDocumentID_)
		{
			pFile = getInsertFile();
		}
		else if (m_pOtherFile->isProceeding() &&
				 m_pOtherFile->getMergeFirstDocumentID() <= uiDocumentID_)
		{
			pFile = getMergeInsertFile();
		}
	}
	
	if (pFile == 0)
	{
		// 大転置内にある

		if (m_cFileID.isDistribute())
		{
			int unit;
			if (m_pOtherFile->getUnitNumber(uiDocumentID_, unit) == true)
			{
				pFile = m_pFull->getUnit(unit);
			}
		}
		else
		{
			pFile = m_pFull->getUnit(0);
		}
	}

	return pFile;
}

//
//	FUNCTION private
//	FullText2::InvertedSection::isSyncMerge -- 同期マージか否か
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		同期マージの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InvertedSection::isSyncMerge()
{
	if (m_pTransaction->isNoLock() ||
		m_cFileID.isSyncMerge() ||
		_IsAsyncMerge.get() == false)
	{
		// ロックしないトランザクションで更新されたか、
		// ヒントに同期マージと書かれているか、
		// パラメータで非同期マージが FALSE になっていたら、
		// 同期マージする
		
		return true;
	}
	
	return false;
}

//
//	Copyright (c) 2010, 2011, 2014, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
