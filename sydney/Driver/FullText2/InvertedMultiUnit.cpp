// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedMultiUnit.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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
#include "FullText2/InvertedMultiUnit.h"

#include "FullText2/FileID.h"
#include "FullText2/InvertedCountUnit.h"
#include "FullText2/InvertedUnit.h"
#include "FullText2/MultiListManager.h"

#include "Common/Message.h"

#include "Schema/File.h"

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	//
	//  VARIABLE
	//
	const Os::Ucs2 _pszPath[] = {'I','n','v',0};
}

//
//	FUNCTION public
//	FullText2::InvertedMultiUnit::InvertedMultiUnit -- コンストラクタ
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
InvertedMultiUnit::InvertedMultiUnit(InvertedSection& cInvertedSection_,
									 const Os::Path& cPath_,
									 bool bBatch_)
	: InvertedFile(cInvertedSection_.getFileID(), cPath_),
	  m_cSection(cInvertedSection_)
{
	// ファイルをattachする
	attach(bBatch_);
}

//
//	FUNCTION public
//	FullText2::InvertedMultiUnit::~InvertedMultiUnit -- デストラクタ
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
InvertedMultiUnit::~InvertedMultiUnit()
{
	detach();
}

//
//	FUNCTION public
//	FullText2::InvertedMultiUnit::destroy
//		-- ファイルを破棄する(createのエラー処理用)
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
InvertedMultiUnit::destroy()
{
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく破棄する
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	try
	{
		bool distribute = (m_vecpUnit.getSize() > 1);
		
		ModVector<File*>::Iterator i = m_vecpSubFile.begin();
		for (; i != m_vecpSubFile.end(); ++i)
			// サブファイルを破棄する
			(*i)->destroy();

		// distributeではない場合には削除しないので、
		// MultiFile::destroy を上書きする
		
		if (distribute) rmdir(m_cPath);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		SydErrorMessage << "Recovery failed." << ModEndl;
		Schema::File::setAvailability(getLockName(), false);
		
		_TRMEISTER_RETHROW;
	}
}

//
//	FUNCTION public
//	FullText2::InvertedMultiUnit::destroy -- ファイルを破棄する
//
//	NOTES
//
//	ARGUMENTS
// 	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedMultiUnit::destroy(const Trans::Transaction& cTransaction_)
{
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく破棄する
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	try
	{
		bool distribute = (m_vecpUnit.getSize() > 1);
		
		ModVector<File*>::Iterator i = m_vecpSubFile.begin();
		for (; i != m_vecpSubFile.end(); ++i)
			// サブファイルを破棄する
			(*i)->destroy(cTransaction_);

		// distributeではない場合には削除しないので、
		// MultiFile::destroy を上書きする
		
		if (distribute) rmdir(m_cPath);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		SydErrorMessage << "Recovery failed." << ModEndl;
		Schema::File::setAvailability(getLockName(), false);
		
		_TRMEISTER_RETHROW;
	}
}

//
//	FUNCTION public
//	FullText2::InvertedMultiUnit::recover -- 障害から回復する(物理ログ)
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
InvertedMultiUnit::recover(const Trans::Transaction& cTransaction_,
						   const Trans::TimeStamp& cPoint_)
{
	if (isMounted(cTransaction_))
	{
		bool distribute = (m_vecpUnit.getSize() > 1);
		
		ModVector<File*>::Iterator i = m_vecpSubFile.begin();
		for (; i != m_vecpSubFile.end(); ++i)
			// サブファイルを障害から回復する
			(*i)->recover(cTransaction_, cPoint_);
		
		if (distribute && !isAccessible())
		{
			// distributeではない場合には削除しないので、
			// MultiFile::recover を上書きする

			rmdir(m_cPath);
		}
	}
}

//
//	FUNCTION public
//	FullText2::InvertedMultiUnit::move -- ファイルを移動する
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
InvertedMultiUnit::move(const Trans::Transaction& cTransaction_,
						const Os::Path& cNewPath_)
{
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく移動する
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	bool distribute = (m_vecpUnit.getSize() > 1);
	bool accessible = (distribute &&
					   Os::Path::compare(cNewPath_, m_cPath)
					   == Os::Path::CompareResult::Unrelated);
	ModVector<InvertedUnit*>::Iterator i = m_vecpUnit.begin();
	int n = 0;
	try
	{
		for (; i != m_vecpUnit.end(); ++i, ++n)
		{
			Os::Path path = cNewPath_;
			
			if (distribute)
			{
				// 分散されているので、下位のディレクトリを設定する
				ModUnicodeOstrStream s;
				s << _pszPath << n;
				path.addPart(s.getString());
			}

			// 移動する
			(*i)->move(cTransaction_, path);
		}
			
		if (accessible)
			rmdir(m_cPath);
	}
	catch (...)
	{
		try
		{
			if (i != m_vecpUnit.begin())
			{
				do
				{
					--i;
					--n;
					
					Os::Path path = m_cPath;
			
					if (distribute)
					{
						ModUnicodeOstrStream s;
						s << _pszPath << n;
						path.addPart(s.getString());
					}

					// 元に戻す
					(*i)->move(cTransaction_, path);
					
				} while (i != m_vecpUnit.begin());

				if (accessible)
					rmdir(cNewPath_);
			}
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
//	FullText2::InvertedMultiUnit::isMounted -- マウントされているか調べる
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& trans
//		トランザクション
//
//	RETURN
//	bool
//		マウントされている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InvertedMultiUnit::isMounted(const Trans::Transaction& trans) const
{
	// 先頭だけの確認で十分
	return m_vecpUnit[0]->isMounted(trans);
}

//
//	FUNCTION public
//	FullText2::InvertedMultiUnit::isAccessible
//		-- 実体である OS ファイルが存在するか調べる
//
//	NOTES
//
//	ARGUMENTS
//	bool bForce_
//		強制モードかどうか (default false)
//
//	RETURN
//	bool
//		実体である OS ファイルが存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InvertedMultiUnit::isAccessible(bool bForce_) const
{
	// 先頭だけの確認で十分
	return m_vecpUnit[0]->isAccessible(bForce_);
}

//
//	FUNCTION public
//	FullText2::InvertedMultiUnit::getListManager
//		-- 検索時に利用するListManagerを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::ListManager*
//		検索用のListManager
//
//	EXCEPTIONS
//
ListManager*
InvertedMultiUnit::getListManager()
{
	ListManager* ret = 0;
	ModVector<InvertedUnit*>::Iterator i = m_vecpUnit.begin();
	
	if (m_vecpUnit.getSize() > 1)
	{
		// 分散用

		// インスタンスを確保
		MultiListManager* p = new MultiListManager(m_cSection.getFile(),
												   m_cSection);
		// 配列の領域を確保
		p->reserve(m_vecpUnit.getSize());
		// 子供のListManagerをpush
		for (; i != m_vecpUnit.end(); ++i)
			p->pushBack((*i)->getListManager());

		ret = p;
	}
	else
	{
		ret = (*i)->getListManager();
	}
		
	return ret;
}

//
//	FUNCTION public
//	FullText2::InvertedMultiUnit::saveAllPages -- ダーティなページをsaveする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
// 	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedMultiUnit::saveAllPages()
{
	try
	{
		ModVector<InvertedUnit*>::Iterator i = m_vecpUnit.begin();
		for (; i != m_vecpUnit.end(); ++i)
			// ユニットの更新を確定する
			(*i)->saveAllPages();
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		SydErrorMessage << "Recovery failed." << ModEndl;
		Schema::File::setAvailability(getLockName(), false);

		_TRMEISTER_RETHROW;
	}
}

#ifndef SYD_COVERAGE
//
//	FUNCTION public
//	FullText2::InvertedMultiUnit::reportFile -- ファイル状況を報告する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	ModOstream& stream_
//		出力ストリーム
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedMultiUnit::reportFile(const Trans::Transaction& cTransaction_,
							  Buffer::Page::FixMode::Value eFixMode_,
							  ModOstream& stream_)
{
	open(cTransaction_, eFixMode_);
	
	int n = 0;
	ModVector<InvertedUnit*>::Iterator i = m_vecpUnit.begin();
	for (; i != m_vecpUnit.end(); ++i, ++n)
	{
		if (m_vecpUnit.getSize() > 1)
		{
			stream_ << "UNIT: " << n << ModEndl;
		}

		(*i)->reportFile(cTransaction_, eFixMode_, stream_);
	}
}
#endif

//
//	FUNCTION private
//	FullText2::InvertedMultiUnit::attach -- ファイルをattachする
//
//	NOTES
//
//	ARGUMENTS
//	bool bBatch_
//		バッチモードかどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedMultiUnit::attach(bool bBatch_)
{
	int count = 1;
	if (m_cSection.isDistribute())
	{
		// 分散しているので、その数を得る
		count = m_cSection.getDistributeCount();
	}

	// 領域を確保する
	m_vecpUnit.reserve(count);
	InvertedFile::reserveSubFile(count);
	
	for (int i = 0; i < count; ++i)
	{
		// 転置ファイルユニットをattachする

		Os::Path path = m_cPath;
		if (count > 1)
		{
			// 分散されているので、下位のディレクトリを設定する
			ModUnicodeOstrStream s;
			s << _pszPath << i;
			path.addPart(s.getString());
		}

		InvertedUnit* unit = 0;
		
		if (m_cFileID.isVacuum() == true)
		{
			unit = new InvertedCountUnit(m_cSection,
										 path,
										 bBatch_,
										 i);
		}
		else
		{
			unit = new InvertedUnit(m_cSection,
									path,
									bBatch_,
									i);
		}

		m_vecpUnit.pushBack(unit);
		InvertedFile::pushBackSubFile(unit);
	}
}

//
//	FUNCTION private
//	FullText2::InvertedMultiUnit::detach -- ファイルをdetachする
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
InvertedMultiUnit::detach()
{
	ModVector<InvertedUnit*>::Iterator i = m_vecpUnit.begin();
	for (; i != m_vecpUnit.end(); ++i)
		delete (*i);
	m_vecpUnit.clear();
}

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
