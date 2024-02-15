// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedCountUnit.cpp --
// 
// Copyright (c) 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
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
#include "FullText2/InvertedCountUnit.h"

#include "FullText2/FakeError.h"
#include "FullText2/Parameter.h"

#include "Schema/File.h"

#include "Common/Message.h"

#include "Exception/Object.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	//
	//  削除数を格納するB木のパス
	//
	const Os::Ucs2 _pszPath[] = {'C','o','u','n','t',0};

	//
	//	削除数がいくつになったらバキュームを行うかの閾値
	//	32ビット版ではバキューム機能はデフォルトではOFFにする
	//
#ifdef SYD_ARCH64
	ParameterInteger _cVacuumThreshold("FullText2_VacuumThreshold", 10000);
#else
	ParameterInteger _cVacuumThreshold("FullText2_VacuumThreshold", 0x7fffffff);
#endif
}

//
//	FUNCTION public
//	FullText2::InvertedCountUnit::InvertedCountUnit -- コンストラクタ
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
//	int iUnitNumber
//		ユニット番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
InvertedCountUnit::InvertedCountUnit(InvertedSection& cInvertedSection_,
									 const Os::Path& cPath_,
									 bool bBatch_,
									 int iUnitNumber_)
	: InvertedUnit(cInvertedSection_, cPath_, bBatch_, iUnitNumber_),
	  m_pCountFile(0)
{
	// ファイルをattachする
	attachCountFile();
}

//
//	FUNCTION public
//	FullText2::InvertedCountUnit::~InvertedCountUnit -- デストラクタ
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
InvertedCountUnit::~InvertedCountUnit()
{
	detachCountFile();
}

//
//	FUNCTION public
//	FullText2::InvertedCountUnit::create -- ファイルを作成する
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
InvertedCountUnit::create()
{
	int step = 0;
	try
	{
		// スーパークラスを呼び出す
		InvertedUnit::create();
		step++;
		// B木ファイルを作成する
		m_pCountFile->create();
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
			case 2: m_pCountFile->destroy();
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
//	FullText2::InvertedCountUnit::move -- ファイルを移動する
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
InvertedCountUnit::move(const Trans::Transaction& cTransaction_,
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
		// B木から移動する
		
		Os::Path path = cNewPath_;
		path.addPart(_pszPath);
		m_pCountFile->move(cTransaction_, path);
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
		FakeErrorMessage << "InvertedCountUnit::move (step="
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
					m_pCountFile->move(cTransaction_, path);
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
InvertedCountUnit::
verify(const Trans::Transaction& cTransaction_,
	   const Admin::Verification::Treatment::Value eTreatment_,
	   Admin::Verification::Progress& cProgress_)
{
	InvertedUnit::verify(cTransaction_, eTreatment_, cProgress_);
	if (cProgress_.isGood())
	{
		m_pCountFile->startVerification(cTransaction_,
										eTreatment_,
										cProgress_);
		try
		{
			m_pCountFile->verify();
		}
		catch (...)
		{
			flushAllPages();
			m_pCountFile->endVerification();
			_SYDNEY_RETHROW;
		}
		
		flushAllPages();
		m_pCountFile->endVerification();
	}
}

//
//	FUNCTION public
//	FullText2::InvertedCountUnit::clear -- ファイルをクリアする
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
InvertedCountUnit::clear()
{
	InvertedUnit::clear();
	try
	{
		// 削除数を保持するB木ファイルの中身もクリアする
		// ここで例外が発生しても、リカバリできないので、
		// 使用可能性をOFFにする
			
		m_pCountFile->clear(false);

		flushAllPages();
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
InvertedCountUnit::saveAllPages()
{
	bool r = InvertedUnit::saveAllPages();
	if (r == true)
		m_pCountFile->saveAllPages();
	return r;
}

//
//	FUNCTION public
//	FullText2::InvertedCountUnit::isNeedVacuum
//		-- バキュームが必要かどうか
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		キー
//	int iNewExpungeCount_
//		新たに削除した数
//
//	RETURN
//	bool
//		バキュームが必要な場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InvertedCountUnit::isNeedVacuum(const ModUnicodeString& cstrKey_,
								int iNewExpungeCount_)
{
	// 既存のエントリを探す
	
	ModUInt32 uiValue = 0;
	if (m_pCountFile->find(cstrKey_, uiValue) == false)
	{
		if (iNewExpungeCount_ > 0)
		{
			// エントリがないので挿入する
			uiValue = static_cast<ModUInt32>(iNewExpungeCount_);
			m_pCountFile->insert(cstrKey_, uiValue);
		}
	}
	else
	{
		// 見つかった
		if (iNewExpungeCount_ > 0)
		{
			// 更新する
			ModUInt32 uiOldValue = uiValue;
			uiValue += static_cast<ModUInt32>(iNewExpungeCount_);
			m_pCountFile->update(cstrKey_, uiOldValue, cstrKey_, uiValue);
		}
	}

#ifndef SYD_ARCH64
	if (cstrKey_.getLength() == 0)
		
		// 単語境界の転置リストは大きいので、32ビット版では vacuum がONでも
		// vacuum しない
		
		return false;
#endif

	return (static_cast<ModUInt32>(_cVacuumThreshold.get()) < uiValue) ?
		true : false;
}

//
//	FUNCTION public
//	FullText2::InvertedCountUnit::clearExpungeCount
//		-- バキュームが必要かどうかのカウントをクリアする
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		カウントをクリアするキー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedCountUnit::clearExpungeCount(const ModUnicodeString& cstrKey_)
{
	ModUInt32 uiValue = 0;
	if (m_pCountFile->find(cstrKey_, uiValue) == true)
	{
		// ヒットしたので、更新する
		
		ModUInt32 uiNewValue = 0;
		m_pCountFile->update(cstrKey_, uiValue, cstrKey_, uiNewValue);
	}
}

//
//	FUNCTION protected
//	FullText2::InvertedCountUnit::attachCountFile
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
InvertedCountUnit::attachCountFile()
{
	if (!m_pCountFile)
	{
		Os::Path path = m_cPath;
		path.addPart(_pszPath);
		m_pCountFile = new BtreeFile(m_cSection.getFileID(),
									 path, m_bBatch);
		MultiFile::pushBackSubFile(m_pCountFile);
	}
}

//
//	FUNCTION protected
//	FullText2::InvertedCountUnit::detachCountFile
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
InvertedCountUnit::detachCountFile()
{
	if (m_pCountFile) delete m_pCountFile, m_pCountFile = 0;
}

//
//	Copyright (c) 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
