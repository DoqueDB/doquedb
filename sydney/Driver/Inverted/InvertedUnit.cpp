// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedUnit.cpp --
// 
// Copyright (c) 2005, 2006, 2008, 2009, 2023 Ricoh Company, Ltd.
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
#include "Inverted/InvertedUnit.h"
#include "Inverted/BtreeFile.h"
#include "Inverted/LeafFile.h"
#include "Inverted/OverflowFile.h"

#include "Schema/File.h"

#include "Common/Message.h"
#include "Common/Assert.h"

#include "Exception/Object.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

namespace
{
	//
	//	CLASS
	//	_$$::_AutoAttachFile
	//
	class _AutoAttachFile
	{
	public:
		_AutoAttachFile(InvertedUnit& cUnit_)
			: m_cUnit(cUnit_), m_bOwner(false)
		{
			if (m_cUnit.isAttached() == false)
			{
				m_cUnit.attach();
				m_bOwner = true;
			}
		}
		~_AutoAttachFile()
		{
			if (m_bOwner) m_cUnit.detach();
		}

	private:
		InvertedUnit& m_cUnit;
		bool m_bOwner;
	};

	const ModUnicodeChar _pszPart[] = {'I','n','v',0};
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::InvertedUnit -- コンストラクタ
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
InvertedUnit::InvertedUnit()
	: m_pBtreeFile(0), m_pLeafFile(0), m_pOverflowFile(0),
	  m_pInvertedFile(0), m_pFileID(0), m_iElement(-1),
	  m_bBatch(false), m_bDistribute(false),
	  m_bNolocation(false), m_bNoTF(false)
{
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::~InvertedUnit -- デストラクタ
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
InvertedUnit::~InvertedUnit()
{
	detach();
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::initialize -- 初期化
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::InvertedFile* pInvertedFile_
//		転置ファイル
//	const Inverted::FileID* pFileID_
//		ファイルID
//	int element_
//		要素番号(default -1)。-1の場合は分散なしと解釈する
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedUnit::initialize(InvertedFile* pInvertedFile_,
						 const FileID* pFileID_,
						 int element_,
						 bool batch_)
{
	if (!m_pInvertedFile)
	{
		m_pInvertedFile = pInvertedFile_;
		m_pFileID = pFileID_;
		m_iElement = element_;
		m_bBatch = batch_;
		m_bDistribute = pFileID_->isDistribution();
		m_bNolocation = pFileID_->isNolocation();
		m_bNoTF = pFileID_->isNoTF();
	}
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::getSize -- ファイルサイズを得る
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
InvertedUnit::getSize() const
{
	_AutoAttachFile cAuto(*const_cast<InvertedUnit*>(this));
	
	ModUInt64 size = 0;
	size += m_pBtreeFile->getSize();
	size += m_pLeafFile->getSize();
	size += m_pOverflowFile->getSize();
	return size;
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::getUsedSize -- 使用ファイルサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		使用ファイルサイズ
//
//	EXCEPTIONS
//
ModUInt64
InvertedUnit::getUsedSize(const Trans::Transaction& cTransaction_) const
{
	_AutoAttachFile cAuto(*const_cast<InvertedUnit*>(this));
	
	ModUInt64 size = 0;
	size += m_pBtreeFile->getUsedSize(cTransaction_);
	size += m_pLeafFile->getUsedSize(cTransaction_);
	size += m_pOverflowFile->getUsedSize(cTransaction_);
	return size;
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::open -- オープンする
//
//	NOTES
//
//	ARGUMETNS
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
InvertedUnit::open(const Trans::Transaction& cTransaction_,
				   Buffer::Page::FixMode::Value eFixMode_)
{
	if (isAttached() == false)
	{
		attach();
	
		m_pBtreeFile->open(cTransaction_, eFixMode_);
		m_pLeafFile->open(cTransaction_, eFixMode_);
		m_pOverflowFile->open(cTransaction_, eFixMode_);
	}
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::close -- クローズする
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
InvertedUnit::close()
{
	if (isAttached())
	{
		m_pBtreeFile->close();
		m_pLeafFile->close();
		m_pOverflowFile->close();

		detach();
	}
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::create -- ファイルを作成する
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
InvertedUnit::create()
{
	int step = 0;
	try
	{
		m_pBtreeFile->create();
		step++;
		// これより前にB木と文書IDベクタを作成する必要あり
		m_pLeafFile->create(*this);
		step++;
		m_pOverflowFile->create();
		step++;

		flushAllPages();
	}
	catch (...)
	{
		recoverAllPages();

		switch (step)
		{
		case 2: m_pLeafFile->destroy();
		case 1: m_pBtreeFile->destroy();
		case 0:
			break;
		}

		if (m_bDistribute)
			Os::Directory::remove(m_cPath);

		_SYDNEY_RETHROW;
	}

#if 0
	SydMessage << "Create Inverted Unit" << ModEndl;
#endif
}

//	FUNCTION public
//	Inverted::InvertedUnit::destroy -- 転置ファイルを削除する
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
InvertedUnit::destroy(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);
	
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく削除する
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	m_pBtreeFile->destroy(cTransaction_);
	m_pLeafFile->destroy(cTransaction_);
	m_pOverflowFile->destroy(cTransaction_);

	try
	{
		if (m_bDistribute)
			Os::Directory::remove(m_cPath);
	}
	catch (...)
	{}
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::mount -- マウントする
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
InvertedUnit::mount(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);

	if (isAccessible(true) == false)
		return;
	
	int step = 0;
	try
	{
		m_pBtreeFile->mount(cTransaction_);
		step++;
		m_pLeafFile->mount(cTransaction_);
		step++;
		m_pOverflowFile->mount(cTransaction_);
		step++;
	}
	catch (...)
	{
		try
		{
			switch (step)
			{
			case 2: m_pLeafFile->unmount(cTransaction_);
			case 1: m_pBtreeFile->unmount(cTransaction_);
			}
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(m_pInvertedFile->getLockName(),
										  false);
		}
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::unmount -- アンマウントする
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
InvertedUnit::unmount(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);
	
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかくアンマウントする
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	int step = 0;
	try
	{
		m_pBtreeFile->unmount(cTransaction_);
		step++;
		m_pLeafFile->unmount(cTransaction_);
		step++;
		m_pOverflowFile->unmount(cTransaction_);
		step++;
	}
	catch (...)
	{
		// ここにくるのはmountされているときのみ
		try
		{
			switch (step)
			{
			case 2: m_pLeafFile->mount(cTransaction_);
			case 1: m_pBtreeFile->mount(cTransaction_);
			}
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(m_pInvertedFile->getLockName(),
										  false);
		}

		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::flush -- フラッシュする
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
InvertedUnit::flush(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);

	if (isMounted(cTransaction_))
	{
		m_pBtreeFile->flush(cTransaction_);
		m_pLeafFile->flush(cTransaction_);
		m_pOverflowFile->flush(cTransaction_);
	}
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::startBackup -- バックアップ開始する
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
InvertedUnit::startBackup(const Trans::Transaction& cTransaction_,
						  const bool bRestorable_)
{
	_AutoAttachFile cAuto(*this);

	if (isMounted(cTransaction_))
	{
		int step = 0;
		try
		{
			m_pBtreeFile->startBackup(cTransaction_, bRestorable_);
			step++;
			m_pLeafFile->startBackup(cTransaction_, bRestorable_);
			step++;
			m_pOverflowFile->startBackup(cTransaction_, bRestorable_);
			step++;
		}
		catch (...)
		{
			try
			{
				switch (step)
				{
				case 2: m_pLeafFile->endBackup(cTransaction_);
				case 1: m_pBtreeFile->endBackup(cTransaction_);
				}
			}
			catch (...)
			{
				SydErrorMessage << "Recovery failed." << ModEndl;
				Schema::File::setAvailability(m_pInvertedFile->getLockName(),
											  false);
			}

			_SYDNEY_RETHROW;
		}
	}
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::endBackup -- バックアップを終了する
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
InvertedUnit::endBackup(const Trans::Transaction& cTransaction_)
{
	_AutoAttachFile cAuto(*this);

	if (isMounted(cTransaction_))
	{
		try
		{
			m_pBtreeFile->endBackup(cTransaction_);
			m_pLeafFile->endBackup(cTransaction_);
			m_pOverflowFile->endBackup(cTransaction_);
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(m_pFileID->getLockName(), false);
		}
	}
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::recover -- 障害から回復する(物理ログ)
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
InvertedUnit::recover(const Trans::Transaction& cTransaction_,
					  const Trans::TimeStamp& cPoint_)
{
	_AutoAttachFile cAuto(*this);

	if (isMounted(cTransaction_))
	{
		m_pBtreeFile->recover(cTransaction_, cPoint_);
		m_pLeafFile->recover(cTransaction_, cPoint_);
		m_pOverflowFile->recover(cTransaction_, cPoint_);

		if (!isAccessible())
		{
			// リカバリの結果
			// 実体である OS ファイルが存在しなくなったので、
			// サブディレクトリを削除する

			rmdir();
		}
	}
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::restore
//		-- ある時点に開始された読取専用トランザクションが
//			参照する版を最新版とする
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
InvertedUnit::restore(const Trans::Transaction& cTransaction_,
					  const Trans::TimeStamp& cPoint_)
{
	_AutoAttachFile cAuto(*this);

	if (isMounted(cTransaction_))
	{
		m_pBtreeFile->restore(cTransaction_, cPoint_);
		m_pLeafFile->restore(cTransaction_, cPoint_);
		m_pOverflowFile->restore(cTransaction_, cPoint_);
	}
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::rmdir -- ディレクトリを削除する
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
InvertedUnit::rmdir()
{
	_AutoAttachFile cAuto(*this);
	
	m_pBtreeFile->rmdir();
	m_pLeafFile->rmdir();
	m_pOverflowFile->rmdir();
	if (m_bDistribute)
		Os::Directory::remove(m_cPath);
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::sync -- 転置ファイルの同期をとる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	cTransaction_
//			転置ファイルの同期を取る
//			トランザクションのトランザクション記述子
//		bool&				incomplete
//			true
//				今回の同期処理で転置ファイルを持つ
//				オブジェクトの一部に処理し残しがある
//			false
//				今回の同期処理で転置ファイルを持つ
//				オブジェクトを完全に処理してきている
//
//				同期処理の結果、転置ファイルを処理し残したかを設定する
//		bool&				modified
//			true
//				今回の同期処理で転置ファイルを持つ
//				オブジェクトの一部が既に更新されている
//			false
//				今回の同期処理で転置ファイルを持つ
//				オブジェクトはまだ更新されていない
//
//				同期処理の結果、転置ファイルが更新されたかを設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//
void
InvertedUnit::sync(const Trans::Transaction& cTransaction_,
				   bool& incomplete, bool& modified)
{
	_AutoAttachFile cAuto(*this);

	if (isMounted(cTransaction_))
	{
		m_pBtreeFile->sync(cTransaction_, incomplete, modified);
		m_pLeafFile->sync(cTransaction_, incomplete, modified);
		m_pOverflowFile->sync(cTransaction_, incomplete, modified);
	}
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::move -- ファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Os::Path& cFilePath_
//		移動先のパス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedUnit::move(const Trans::Transaction& cTransaction_,
				   const Os::Path& cFilePath_)
{
	_AutoAttachFile cAuto(*this);

	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく移動する
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	Os::Path cFilePath = cFilePath_;
	if (m_bDistribute)
	{
		ModUnicodeOstrStream s;
		s << _pszPart << m_iElement;
		cFilePath.addPart(s.getString());
	}

	bool accessible = (m_bDistribute && isAccessible() &&
					   Os::Path::compare(cFilePath_, m_cPath)
					   == Os::Path::CompareResult::Unrelated);
	int step = 0;
	try
	{
		m_pBtreeFile->move(cTransaction_, cFilePath);
		step++;
		m_pLeafFile->move(cTransaction_, cFilePath);
		step++;
		m_pOverflowFile->move(cTransaction_, cFilePath);
		step++;
		if (accessible)
			Os::Directory::remove(m_cPath);
	}
	catch (...)
	{
		try
		{
			switch (step)
			{
			case 2:	m_pLeafFile->move(cTransaction_, m_cPath);
			case 1: m_pBtreeFile->move(cTransaction_, m_cPath);
			}
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(m_pInvertedFile->getLockName(),
										  false);
		}

		_SYDNEY_RETHROW;
	}

	m_cPath = cFilePath;
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::clear -- ファイルをクリアする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	bool bForce_ (default false)
//		強制的に行うか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedUnit::clear(const Trans::Transaction& cTransaction_, bool bForce_)
{
	attach();

	if (isMounted(cTransaction_))
	{
		m_pBtreeFile->clear(cTransaction_, bForce_);
		m_pOverflowFile->clear(cTransaction_, bForce_);
		// これより前にB木と文書IDベクタをクリアする必要あり
		m_pLeafFile->clear(*this, cTransaction_, bForce_);
	}
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::startVerification -- 整合性検査を開始する
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
InvertedUnit::startVerification(
	const Trans::Transaction& cTransaction_,
	Admin::Verification::Treatment::Value uiTreatment_,
	Admin::Verification::Progress& cProgress_)
{
	attach();

	if (isMounted(cTransaction_))
	{
		int step = 0;
		try
		{
			m_pBtreeFile
				->startVerification(cTransaction_, uiTreatment_, cProgress_);
			step++;
			m_pLeafFile
				->startVerification(cTransaction_, uiTreatment_, cProgress_);
			step++;
			m_pOverflowFile
				->startVerification(cTransaction_, uiTreatment_, cProgress_);
			step++;
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			try
			{
				switch (step)
				{
				case 2: m_pLeafFile->endVerification();
				case 1: m_pBtreeFile->endVerification();
				}
			}
			catch(...)
			{
				Schema::File::setAvailability(
					m_pInvertedFile->getLockName(), false);
			}
			_SYDNEY_RETHROW;
		}
	}
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::endVerification -- 整合性検査を終了する
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
InvertedUnit::endVerification()
{
	if (m_pBtreeFile->isMounted())
	{
		m_mapDeleteIdBlock.erase(m_mapDeleteIdBlock.begin(),
								 m_mapDeleteIdBlock.end());

		m_pBtreeFile->endVerification();
		m_pLeafFile->endVerification();
		m_pOverflowFile->endVerification();
	}

	detach();
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::verifyBtree -- B木の整合性検査を実施する
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
InvertedUnit::verifyBtree()
{
	if (m_pBtreeFile->isMounted())
	{
		m_pBtreeFile->verify();
	}
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::flushAllPages
//		-- すべてのページを確定し、デタッチする
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
InvertedUnit::flushAllPages()
{
	if (isAttached())
	{
		m_pBtreeFile->flushAllPages();
		m_pLeafFile->flushAllPages();
		m_pOverflowFile->flushAllPages();
	}
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::recoverAllPages
//		-- すべてのページを破棄し、デタッチする
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
InvertedUnit::recoverAllPages()
{
	if (isAttached())
	{
		m_pBtreeFile->recoverAllPages();
		m_pLeafFile->recoverAllPages();
		m_pOverflowFile->recoverAllPages();
	}
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::saveAllPages -- すべてのページを確定する
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
InvertedUnit::saveAllPages()
{
	if (isAttached())
	{
		m_pBtreeFile->saveAllPages();
		m_pLeafFile->saveAllPages();
		m_pOverflowFile->saveAllPages();
	}
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::attachLeafPage -- リーフページをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		アタッチするページID
//
//	RETURN
//	Inverted::LeafPage::PagePointer
//		リーフページ
//
//	EXCEPTIONS
//
LeafPage::PagePointer
InvertedUnit::attachLeafPage(PhysicalFile::PageID uiPageID_)
{
	return m_pLeafFile->attachPage(uiPageID_);
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::enterDeleteIdBlock -- 削除するIDブロックを登録する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		索引単位
//	ModUInt32 uiFirstDocumentID_
//		削除するIDブロックの先頭文書ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedUnit::enterDeleteIdBlock(const ModUnicodeString& cstrKey_,
								 ModUInt32 uiFirstDocumentID_)
{
	Map::Iterator i = m_mapDeleteIdBlock.find(cstrKey_);
	if (i != m_mapDeleteIdBlock.end())
	{
		Vector::Iterator j = (*i).second.begin();
		for (; j != (*i).second.end(); ++j)
			if ((*j) == uiFirstDocumentID_)
				return;
		(*i).second.pushBack(uiFirstDocumentID_);
	}
	else
	{
		m_mapDeleteIdBlock[cstrKey_].pushBack(uiFirstDocumentID_);
	}
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::enterExpungeFirstDocumentID
//		-- 先頭文書IDを削除したIDブロックのログを登録する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		索引単位
//	ModUInt32 uiOldDocumentID_
//		削除する前の先頭文書ID
//	ModUInt32 uiNewDocumentID_
//		削除した後に設定した先頭文書ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedUnit::enterExpungeFirstDocumentID(const ModUnicodeString& cstrKey_,
										  ModUInt32 uiOldDocumentID_,
										  ModUInt32 uiNewDocumentID_)
{
	m_mapExpungeFirstDocumentID.insert(
		ModPair<ModUnicodeString, ModUInt32>(cstrKey_, uiOldDocumentID_),
		uiNewDocumentID_);
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::getExpungeFirstDocumentID
//		-- 先頭文書IDを削除したIDブロックのログを検索する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		索引単位
//	ModUInt32 uiOldDocumentID_
//		削除する前の先頭文書ID
//
//	RETURN
//	ModUInt32
//		エントリが存在した場合は削除後のID、それ以外の場合はUndefinedDocumentID
//
//	EXCEPTIONS
//
ModUInt32
InvertedUnit::getExpungeFirstDocumentID(const ModUnicodeString& cstrKey_,
										ModUInt32 uiOldDocumentID_)
{
	ModUInt32 uiNewID = UndefinedDocumentID;
	IDMap::Iterator i = m_mapExpungeFirstDocumentID.find(
		ModPair<ModUnicodeString, ModUInt32>(cstrKey_, uiOldDocumentID_));
	if (i != m_mapExpungeFirstDocumentID.end())
	{
		uiNewID = (*i).second;
	}
	return uiNewID;
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::insertBtree -- B木に挿入する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		索引単位
//	PhysicalFile::PageID uiPageID_
//		リーフファイルのページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedUnit::insertBtree(const ModUnicodeString& cstrKey_,
						  PhysicalFile::PageID uiPageID_)
{
	m_pBtreeFile->insert(cstrKey_, uiPageID_);
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::expungeBtree -- B木から削除する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		 索引単位
//	PhysicalFile::PageID uiPageID_
//		リーフファイルのページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedUnit::expungeBtree(const ModUnicodeString& cstrKey_,
						   PhysicalFile::PageID uiPageID_)
{
	m_pBtreeFile->expunge(cstrKey_);
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::updateBtree -- B木を更新する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey1_
//		更新前の索引単位
//	PhysicalFile::PageID uiPageID1_
//		更新前のページID
//	const ModUnicodeString& cstrKey2_
//		更新後の索引単位
//	PhysicalFile::PageID uiPageID2_
//		更新後のページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedUnit::updateBtree(const ModUnicodeString& cstrKey1_,
						  PhysicalFile::PageID uiPageID1_,
						  const ModUnicodeString& cstrKey2_,
						  PhysicalFile::PageID uiPageID2_)
{
	m_pBtreeFile->update(cstrKey1_, uiPageID1_, cstrKey2_, uiPageID2_);
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::searchBtree -- B木を検索する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		索引単位
//	PhysicalFile::PageID uiPageID_
//		リーフファイルのページID(検索結果)
//
//	RETURN
//	bool
//		検索にヒットした場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InvertedUnit::searchBtree(const ModUnicodeString& cstrKey_,
						  PhysicalFile::PageID& uiPageID_)
{
	if (m_pBtreeFile && m_pBtreeFile->isMounted())
	{
		uiPageID_ = m_pBtreeFile->search(cstrKey_);
		if (uiPageID_ != PhysicalFile::ConstValue::UndefinedPageID)
			return true;
	}
	return false;
}

#ifndef SYD_COVERAGE
//
//	FUNCTION public
//	Inverted::InvertedUnit::reportFile -- ファイル状況を報告する
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
InvertedUnit::reportFile(const Trans::Transaction& cTransaction_,
						 Buffer::Page::FixMode::Value eFixMode_,
						 ModOstream& stream_)
{
	open(cTransaction_, eFixMode_);

	if (m_bDistribute)
		stream_ << "UNIT:" << m_iElement << ModEndl;
	
	if (isMounted(cTransaction_))
	{
		m_pBtreeFile->reportFile(cTransaction_, stream_);
		m_pLeafFile->reportFile(cTransaction_, stream_);
		m_pOverflowFile->reportFile(cTransaction_, stream_);
	}
}
#endif

//
//	FUNCTION public
//	Inverted::InvertedUnit::isAttached -- attachされているか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		attachされている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InvertedUnit::isAttached() const
{
	return (m_pBtreeFile != 0) ? true : false;
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::attach -- ファイルをattachする
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
InvertedUnit::attach()
{
	if (!m_pBtreeFile)
	{
		m_cPath = m_pFileID->getPath();
		
		if (m_bDistribute)
		{
			// ファイル分散を利用する
			ModUnicodeOstrStream s;
			s << _pszPart << m_iElement;
			m_cPath.addPart(s.getString());
		}
		
		m_pBtreeFile = new BtreeFile(*m_pFileID, m_cPath, m_bBatch);
		m_pLeafFile = new LeafFile(*m_pFileID, m_cPath, m_bBatch);
		m_pOverflowFile = new OverflowFile(*m_pFileID, m_cPath, m_bBatch);
	}
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::detach -- ファイルをdetachする
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
InvertedUnit::detach()
{
	if (m_pInvertedFile)
	{
		delete m_pBtreeFile, m_pBtreeFile = 0;
		delete m_pLeafFile, m_pLeafFile = 0;
		delete m_pOverflowFile, m_pOverflowFile = 0;
	}
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::isMounted -- ファイルがマウントされているか
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
InvertedUnit::isMounted(const Trans::Transaction& cTransaction_) const
{
	return m_pBtreeFile->isMounted(cTransaction_);
}

//
//	FUNCTION public
//	Inverted::InvertedUnit::isAccessible -- ファイルが存在するか
//
//	NOTES
//
//	ARGUMENTS
//	bool force_
//		強制モードかどうか(default false)
//
//	RETURN
//	bool
//		ファイルが存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InvertedUnit::isAccessible(bool force_) const 
{
	return m_pBtreeFile->isAccessible(force_);
}

//
//	Copyright (c) 2005, 2006, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
