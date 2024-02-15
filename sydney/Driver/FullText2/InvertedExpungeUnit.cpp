// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedExpungeUnit.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
#include "FullText2/InvertedExpungeUnit.h"

#include "FullText2/ExpungeIDVectorFile.h"
#include "FullText2/FakeError.h"

#include "Schema/File.h"

#include "Common/Message.h"

#include "Exception/Object.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	//
	//  VARIABLE
	//
	const Os::Ucs2 _pszPath[] = {'I','D','V','e','c','t','o','r',0};
}

//
//	FUNCTION public
//	FullText2::InvertedExpungeUnit::InvertedExpungeUnit -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedSection& cInvertedSection_
//		転置ファイル
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
//	なし
//
InvertedExpungeUnit::InvertedExpungeUnit(InvertedSection& cInvertedSection_,
										 const Os::Path& cPath_,
										 bool bBatch_)
	: InvertedUnit(cInvertedSection_, cPath_, bBatch_),
	  m_pIDVectorFile(0)
{
	// 削除用の小転置にはTF値も位置情報も不要なので、
	// メンバー変数を修正する
	m_bNolocation = true;
	m_bNoTF = true;

	// ファイルをattachする
	attachIDVector();
}

//
//	FUNCTION public
//	FullText2::InvertedExpungeUnit::~InvertedExpungeUnit -- デストラクタ
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
//	なし
//
InvertedExpungeUnit::~InvertedExpungeUnit()
{
	detachIDVector();
}

//
//	FUNCTION public
//	FullText2::InvertedExpungeUnit::create -- ファイルを作成する
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
InvertedExpungeUnit::create()
{
	int step = 0;
	try
	{
		// スーパークラスを呼び出す
		InvertedUnit::create();
		step++;
		// ベクターファイルを作成する
		m_pIDVectorFile->create();
		step++;

		flushAllPages();
	}
	catch (...)
	{
		try
		{
			recoverAllPages();

			switch (step)
			{
			case 2: m_pIDVectorFile->destroy();
			case 1: InvertedUnit::destroy();
			case 0:
				break;
			}
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
//	FullText2::InvertedExpungeUnit::move -- ファイルを移動する
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
InvertedExpungeUnit::move(const Trans::Transaction& cTransaction_,
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
	Os::Path cOrgPath = m_cPath;
	int step = 0;
	try
	{
		// 成功するとスパークラスがm_cPathを書き換えるので、
		// 文書ID変換ベクターから移動する
		
		Os::Path path = cNewPath_;
		path.addPart(_pszPath);
		m_pIDVectorFile->move(cTransaction_, path);
		step++;
		
		InvertedUnit::move(cTransaction_, cNewPath_, false);
		step++;

		if (accessible)
			rmdir(cOrgPath);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
#ifdef SYD_FAKE_ERROR
		FakeErrorMessage << "InvertedExpungeUnit::move (step="
						 << step << ")" << ModEndl;
#endif
		try
		{
			switch (step)
			{
			case 2:
				InvertedUnit::move(cTransaction_, cOrgPath, false);
			case 1:
				{
					Os::Path path = cOrgPath;
					path.addPart(_pszPath);
					m_pIDVectorFile->move(cTransaction_, path);
				}
			}
			m_cPath = cOrgPath;
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
}

//
//	FUNCTION public
//	FullText2::InvertedExpugeUnit::verify -- 整合性検査を行う
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
InvertedExpungeUnit::
verify(const Trans::Transaction& cTransaction_,
	   const Admin::Verification::Treatment::Value eTreatment_,
	   Admin::Verification::Progress& cProgress_)
{
	InvertedUnit::verify(cTransaction_, eTreatment_, cProgress_);
	if (cProgress_.isGood())
		m_pIDVectorFile->verify(cTransaction_, eTreatment_, cProgress_);
}

//
//	FUNCTION public
//	FullText2::InvertedExpungeUnit::clear -- ファイルをクリアする
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
InvertedExpungeUnit::clear()
{
	InvertedUnit::clear();
	try
	{
		// 文書ID変換ベクターの中身もクリアする
		// ここで例外が発生しても、リカバリできないので、
		// 使用可能性をOFFにする
			
		m_pIDVectorFile->clear();
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		SydErrorMessage << "Recovery failed." << ModEndl;
		Schema::File::setAvailability(getLockName(), false);

		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	FullText2::InvertedExpungeUnit::saveAllPages -- すべてのページを確定する
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
bool
InvertedExpungeUnit::saveAllPages()
{
	bool r = InvertedUnit::saveAllPages();
	if (r == true)
		m_pIDVectorFile->flushAllPages();
	return r;
}

//
//	FUNCTION public
//	FullText2::InvertedExpungeUnit::convertToBigDocumentID
//		-- 大転置の文書IDに変換する
//
//	NOTES
//	削除用小転置を大転置にマージするときに、大転置が本メソッドを呼び出す
//
//	ARGUMENTS
//	FullText2::DocumentID uiDocumentID_
//		小転置の文書ID
//	int& iUnitNumber_
//		対象文書が格納されているユニットの番号
//
//	RETURN
//	FullText2::DocumentID
//		大転置の文書ID
//
//	EXCEPTIONS
//
DocumentID
InvertedExpungeUnit::convertToBigDocumentID(DocumentID uiDocumentID_,
											int& iUnitNumber_)
{
	DocumentID uiFullID;
	if (m_pIDVectorFile->get(uiDocumentID_, uiFullID, iUnitNumber_) == false)
		uiFullID = UndefinedDocumentID;
	return uiFullID;
}

//
//	FUNCTION public
//	FullText2::InvertedExpungeUnit::assignDocumentID
//		-- 大転置の文書IDを登録し、小転置の文書IDを得る
//
//	NOTES
//	小転置への登録時に本メソッドを呼び出し、登録する
//
//	ARGUMENTS
//	FullText2::DocumentID uiDocumentID_
//		大転置の文書ID
//	int iUnitNumber_
//		削除対象の文書が格納されているユニットの番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
DocumentID
InvertedExpungeUnit::assignDocumentID(DocumentID uiDocumentID_,
									  int iUnitNumber_)
{
	// 現在格納されている最大のキー値を取得し、
	// それより1つ大きなキー値を登録する
	
	DocumentID uiKey = m_pIDVectorFile->getMaxKey();
	++uiKey;
	m_pIDVectorFile->insert(uiKey, uiDocumentID_, iUnitNumber_);
	
	return uiKey;
}

//
//	FUNCTION public
//	FullText2::InvertedExpungeUnit::expungeIDVector
//		-- 大転置ID <-> 小転置ID変換ベクターからエントリを削除する
//
//	NOTES
//	削除のUNDOで利用する
//
//	ARGUMENTS
//	FullText2::DocumentID uiDocumentID_
//	   	小転置ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedExpungeUnit::expungeIDVector(DocumentID uiDocumentID_)
{
	m_pIDVectorFile->expunge(uiDocumentID_);
}

//
//	FUNCTION public
//	FullText2::InvertedExpungeUnit::getAll
//		-- 削除したすべての文書の文書IDを得る
//
//	NOTES
//
//	ARGUMENTS
//	Common::LargeVector<DocumentID>& vecID_
//		文書IDの配列
//
//	RETURN
//	なし
//
void
InvertedExpungeUnit::getAll(Common::LargeVector<DocumentID>& vecID_)
{
	m_pIDVectorFile->getAll(vecID_);
}

//
//	FUNCTION protected
//	FullText2::InvertedExpungeUnit::attachIDVector
//		-- ファイルをattachする
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
InvertedExpungeUnit::attachIDVector()
{
	if (!m_pIDVectorFile)
	{
		Os::Path path = m_cPath;
		path.addPart(_pszPath);
		m_pIDVectorFile = new ExpungeIDVectorFile(m_cSection.getFileID(),
												  path, m_bBatch);
		MultiFile::pushBackSubFile(m_pIDVectorFile);
	}
}

//
//	FUNCTION protected
//	FullText2::InvertedExpungeUnit::detachIDVector
//		-- ファイルをdetachする
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
InvertedExpungeUnit::detachIDVector()
{
	if (m_pIDVectorFile) delete m_pIDVectorFile, m_pIDVectorFile = 0;
}

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
