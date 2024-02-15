//-*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PageManager.cpp -- ページマネージャの実装ファイル
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Vector";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "FileCommon/AutoAttach.h"

#include "Exception/BadArgument.h"
#include "Exception/Unexpected.h"
#include "Exception/IllegalFileAccess.h"
#include "Exception/FakeError.h"
#include "Exception/MemoryExhaust.h"
#include "Exception/BadDataPage.h"

#include "PhysicalFile/File.h"
#include "PhysicalFile/Types.h"

#include "Vector/FileInformation.h"
#include "Vector/FileParameter.h"
#include "Vector/OpenParameter.h" //Vector::Object生成時に使用
#include "Vector/Object.h"
#include "Vector/PageManager.h"

#include "ModAutoPointer.h"

_SYDNEY_USING

// FileInformationの出納に必要になる情報
namespace{
	const ModUInt32 m_ulFileInformationPageID     = 0;
	const ModUInt32 m_ulFileInformationPageNumber = 1;
}

using namespace Vector;

//
//	FUNCTION
//	PageManager::PageManager -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
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
PageManager::PageManager(const Trans::Transaction&	rTransaction_,
						 FileParameter&				rFileParameter_,
						 OpenParameter*				pOpenParameter_,
						 PhysicalFile::File*			pPhysicalFile_,
						 bool						bFirst_)
	: m_rTransaction(rTransaction_),
	  m_rFileParameter(rFileParameter_),
	  m_pOpenParameter(pOpenParameter_),
	  m_pPhysicalFile(pPhysicalFile_),
	  m_bLastPageIDValid(false),
	  m_pCacheObject(0), m_pCachePage(0), m_pCacheFileInformation(0)
{
	if (bFirst_) {
		m_ulLastPageID = 0;
		m_bLastPageIDValid = true;
	}
}

//
//	FUNCTION
//	PageManager::~PageManager -- デストラクタ
//
//	NOTES
//	デストラクタ。特に何もしない。
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
PageManager::~PageManager()
{
	m_pOpenParameter = 0;
	m_pPhysicalFile = 0;

	delete m_pCacheObject;
}

//
//	FUNCTION public
//	Vector::PageManager::close -- クローズする
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
PageManager::close()
{
	// isNoVersion() == false の場合には、キャッシュしている物理ページを
	// detachする

	if (m_pCachePage)
	{
		m_pPhysicalFile->detachPage(m_pCachePage,
									PhysicalFile::Page::UnfixMode::NotDirty);
		m_pPhysicalFile->unfixVersionPage(true);

		// Not set false to m_bLastPageIDValid.
		// Because, this function is called from only File::close(),
		// and File::close() always deletes this PageManager,
	}
	if (m_pCacheFileInformation) {
		FileInformation* pFileInfo = m_pCacheFileInformation;
		m_pCacheFileInformation = 0;
		detachFileInformation(pFileInfo);
	}
}

//	FUNCTION
//	PageManager::getFile -- ファイル記述子へのポインタを取得する
//
//	NOTES
//	ファイル記述子へのポインタを取得する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const PhysicalFile::File*&
//
//	EXCEPTIONS
//	なし
//
PhysicalFile::File*&
PageManager::getFile()
{
	return m_pPhysicalFile;
}

//
//	FUNCTION
//	PageManager::attachObject -- 
//	  ベクタキーに対応するオブジェクトを生成して渡す
//
//	NOTES
//	ベクタキーに対応するオブジェクトを生成して渡す。
//	!Leakable!
//
//	ARGUMENTS
//	ModUInt32 ulVectorKey_
//
//	Buffer::Page::FixMode::Value eFixMode_
//	  デフォルトはBuffer::Page::FixMode::Write。
//
//	RETURN
//	Object* 
//	  結果であるオブジェクトへのポインタ。
//	  該当のオブジェクトが存在しないときはnull。
//
//	EXCEPTIONS
//	  なし
//
Object* 
PageManager::attachObject(ModUInt32 ulVectorKey_,
						  Buffer::Page::FixMode::Value eFixMode_)
{
	//	undefinedならばnullを返す
	if (ulVectorKey_ == FileCommon::VectorKey::Undefined) {
		return 0;
	}

	// 頻用するのでキャッシュ
	ModSize ulPageID = getPageID(ulVectorKey_);

	// 該当するページが最終ページを越える位置にあるか、
	// アロケートされていないページであればnull
	initLastPageID();

	if (ulPageID > m_ulLastPageID)
	{
		return 0;
	}
	else if (eFixMode_ != Buffer::Page::FixMode::ReadOnly)
	{
		_SYDNEY_FAKE_ERROR("Vector::PageManager::attachObject",Exception::MemoryExhaust(moduleName, srcFile, __LINE__));

		if (!m_pPhysicalFile->isUsedPage(m_rTransaction, ulPageID))
			return 0;
	}

	// ページへのポインタを得る
	const PhysicalFile::Page* pPage = 0;
	try
	{
		pPage = setCurrentPage(ulPageID, eFixMode_);
	}
	catch (Exception::BadDataPage&)
	{
		// 該当するページが存在しないらしい
		
		return 0;
	}
	
	try{
		// 使用ビット及びブロック先頭へのポインタを得る
		ModUInt32 ulBlockID = getBlockID(ulVectorKey_);
		if (isValidBlock(pPage, ulBlockID)) {
			return constructObject(pPage, ulBlockID);
		}
	} catch(...) {
		unsetCurrentPage(pPage, PhysicalFile::Page::UnfixMode::NotDirty);
		_SYDNEY_RETHROW;
	}

	// 該当のblockがinvalidである場合

	unsetCurrentPage(pPage, PhysicalFile::Page::UnfixMode::NotDirty);
	return 0;
}

//
//	FUNCTION
//	Vector::attachNewObject -- Update::insert()専用のattachObject()
//
//	NOTE
//		Update::insert()専用のattachObject()。
//
//	ARGUMENTS
//		ModUInt32 ulVectorKey_
//			
//	RETURN
//		// Update::insert
//
//	EXCEPTIONS
//		なし
//			
Object* 
PageManager::attachNewObject(ModUInt32 ulVectorKey_)
{
	//	undefinedならばnullを返す
	if (ulVectorKey_ == FileCommon::VectorKey::Undefined) {
		return 0;
	}

	// 頻用するのでキャッシュ
	ModSize ulPageID = getPageID(ulVectorKey_);

	// 該当のページがアロケートされていない場合新しく作る
	initLastPageID();

	_SYDNEY_FAKE_ERROR("Vector::PageManager::attachNewObject_1",Exception::MemoryExhaust(moduleName, srcFile, __LINE__));

	try
	{
		if (ulPageID > m_ulLastPageID ||
			(!m_pOpenParameter->isBatchMode()
			 && 
			 !m_pPhysicalFile->isUsedPage(m_rTransaction, ulPageID))) {

			_SYDNEY_FAKE_ERROR("Vector::PageManager::attachNewObject_2",Exception::MemoryExhaust(moduleName, srcFile, __LINE__));

			m_pPhysicalFile->allocatePage(m_rTransaction, ulPageID);

			if (ulPageID > m_ulLastPageID)
				m_ulLastPageID = ulPageID;
		}
	}
	catch (...)
	{
		m_pPhysicalFile->unfixVersionPage(false);
		_SYDNEY_RETHROW;
	}

	// ページ先頭へのポインタを得る
	const PhysicalFile::Page* pPage = setCurrentPage
		(ulPageID, Buffer::Page::FixMode::Write);
	try{
		// 使用ビット及びブロック先頭へのポインタを得る
		ModUInt32 ulBlockID = getBlockID(ulVectorKey_);
		if (!isValidBlock(pPage, ulBlockID)) {
			return constructObject(pPage, ulBlockID);
		}
	} catch(...) {
		unsetCurrentPage(pPage, PhysicalFile::Page::UnfixMode::NotDirty);
		_SYDNEY_RETHROW;
	}

	// 該当のblockがvalidである場合
	unsetCurrentPage(pPage, PhysicalFile::Page::UnfixMode::NotDirty);
	return 0;
}

//
//	FUNCTION
//	PageManager::detachObject -- 
//	  attachObjectで作ったオブジェクトを始末する
//
//	NOTES
//	attachObjectで作ったオブジェクトを始末する。
//
//	ARGUMENTS
//	Object*& rpObject_
//
//	PhysicalFile::Page::UnfixMode::Value eUnfixMode_
//	  デフォルトはPhysicalFile::Page::UnfixMode::Dirty。
//
//	RETURN
//	Object* 
//	  結果であるオブジェクトへのポインタ。
//
//	EXCEPTIONS
//	  なし
//
bool
PageManager::detachObjectCore(PhysicalFile::Page*& pPage,
						  Vector::Object*& rpObject_,
						  PhysicalFile::Page::UnfixMode::Value eUnfixMode_)
{
	SydAssert(rpObject_ != 0);

	pPage = rpObject_->getPage();

	ModUInt32 iCount = 1;
	if (eUnfixMode_ == PhysicalFile::Page::UnfixMode::Dirty)
	{
		// cCountに値を代入
		Common::UnsignedIntegerData	cCount;
		FileCommon::DataManager::accessToCommonData
			(pPage, 0, cCount, FileCommon::DataManager::AccessRead);
		//	ここで0はカウント領域へのoffset

		// 以下の " ページ内のオブジェクトカウントの操作 " 中において、
		// 例外が発生した場合、ブロック利用ビットマップとの整合性が
		// とれなくなってしまう可能性があります。

		iCount = cCount.getValue();
		// ページ内のオブジェクトカウントの操作
		if (!rpObject_->wasValid() && rpObject_->isValid()) {
			++iCount;
			cCount.setValue(iCount);
			FileCommon::DataManager::accessToCommonData
				(pPage, 0, cCount, FileCommon::DataManager::AccessWrite);
		} else if (rpObject_->wasValid() && !rpObject_->isValid()) {
			--iCount;
			cCount.setValue(iCount);
			FileCommon::DataManager::accessToCommonData
				(pPage, 0, cCount, FileCommon::DataManager::AccessWrite);
		}
	}
	// オブジェクトを破棄
	if (m_pCacheObject == 0)
	{
		m_pCacheObject = rpObject_;
	}
	else
	{
		delete rpObject_;
	}
	rpObject_ = 0;
	// 物理ページの参照カウンタを検査
	return (iCount == 0);
}


void
PageManager::detachObject(Vector::Object*& rpObject_,
						  PhysicalFile::Page::UnfixMode::Value eUnfixMode_)
{
	PhysicalFile::Page* pPage = 0;
	bool bFreePage = detachObjectCore(pPage, rpObject_, eUnfixMode_);

	// オブジェクトの載っている物理ページを破棄(参照カウンタが０の場合）
	unsetCurrentPage(pPage, eUnfixMode_, bFreePage);
}


void
PageManager::reattachObject(Vector::Object*& rpObject_,
						  PhysicalFile::Page::UnfixMode::Value eUnfixMode_,
						  ModUInt32 ulVectorKey_)
{
	PhysicalFile::Page* pPage = 0;
	bool bFreePage = detachObjectCore(pPage, rpObject_, eUnfixMode_);

	_SYDNEY_ASSERT(pPage->getID() == getPageID(ulVectorKey_));
	// 同一ページ上のオブジェクトへのアタッチなので、
	// 物理ページのデタッチ・アタッチは削除できる

	// 使用ビット及びブロック先頭へのポインタを得る
	ModUInt32 ulBlockID = getBlockID(ulVectorKey_);
	if (! isValidBlock(pPage, ulBlockID)) {
		// 該当のblockがinvalidである場合
		// オブジェクトの載っている物理ページを破棄(参照カウンタが０の場合）
		unsetCurrentPage(pPage, eUnfixMode_, bFreePage);
		//rpObject_ = 0; //detachObjectCore() で delete 済み
	} else {
		// 新規オブジェクトを作成
		rpObject_ = constructObject(pPage, ulBlockID);
	}
}


//	FUNCTION Vector::PageMamager::nextVectorKey
//	  -- 使用中の「次」(前あるいは後ろ)のベクタキーを返す
//
//	NOTES
//	使用中の「次」(前あるいは後ろ)のベクタキーを返す。
//	どのブロックも使っていないページはunsetCurrentPage()によって
//  未使用になるので、現在のページと次のページのbitmapのみ
//	調べればよい。
//
//	ARGUMENTS
//	const ModUInt32 ulVectorKey_
//	  「出発点」のベクタキー。
//	bool bDirection_
//	  走査方向(falseなら順方向、trueなら逆方向)。
//
//	RETURN
//	ModUInt32
//	  「次」のベクタキーがあればそれ、
//	  なければ出発点のベクタキー。
//	  (従って、この関数のクライアントは、
//	  第一引数と返り値とを比較することによって、
//	  「次」のベクタキーの有無を知ることができる)
//
//	EXCEPTIONS
//	Unexpected
//	  「次」のベクタキーがあるともないとも分からなかった
//
ModUInt32
PageManager::nextVectorKey(ModUInt32 ulVectorKey_,
						   bool bDirection_)
{

	// 頻用するのでキャッシュ
	const ModUInt32 ulBPP = m_rFileParameter.getBlocksPerPage();

	// 方向に依存する変数の設定
	ModInt32 d;
	ModInt32 ulStartBlockID;
	ModInt32 ulEndBlockID;
	if (bDirection_) { // true : backward
		d = -1;
		ulStartBlockID = ulBPP-1;
		ulEndBlockID = -1;		// ← 0ではいけない
	} else { // false : forward
		d = 1;
		ulStartBlockID = 0;
		ulEndBlockID = ulBPP;	// ← ulBPP-1ではいけない
	}

	// CurrentPageのセット
	ModUInt32 ulPageID = getPageID(ulVectorKey_);
	// ↓tryの外側で定義しておく
	ModUInt32 ulNewPageID = PhysicalFile::ConstValue::UndefinedPageID; 
	const PhysicalFile::Page* pPage = 
		setCurrentPage(ulPageID, Buffer::Page::FixMode::ReadOnly);
	try {
		// まずCurrentPageを走査
		for (ModInt32 i = getBlockID(ulVectorKey_) + d;
			 i != ulEndBlockID; i += d) {
			if (isValidBlock(pPage, i)) {
				unsetCurrentPage(pPage, PhysicalFile::Page::UnfixMode::NotDirty);
				return (ulPageID-m_ulFileInformationPageNumber) * ulBPP + i;
			}
		}

		_SYDNEY_FAKE_ERROR("Vector::PageManager::nextVectorKey_1",Exception::MemoryExhaust(moduleName, srcFile, __LINE__));

		// 次のページに進もうとする
		if (bDirection_) { // true == backward
			ulNewPageID = m_pPhysicalFile->getPrevPageID(m_rTransaction, ulPageID);
		} else { // false == forward
			ulNewPageID = m_pPhysicalFile->getNextPageID(m_rTransaction, ulPageID);
		}
	} catch(...) {
		unsetCurrentPage(pPage, PhysicalFile::Page::UnfixMode::NotDirty);
		_SYDNEY_RETHROW;
	}
	unsetCurrentPage(pPage, PhysicalFile::Page::UnfixMode::NotDirty);

	//SydDebugMessage << "new page " << ulNewPageID << ModEndl;
	// 「次のページ」がない場合、上の関数は-1を返す
	if (ulNewPageID == PhysicalFile::ConstValue::UndefinedPageID) { 
		return ulVectorKey_;
	} else {
		// 再検索
		// Vector::File::expunge() → Vector::PageManager::detachObject() →
		//		Vector::PageManager::unsetCurrentPage()内において、物理ページ内で
		// 唯一のオブジェクトを削除する場合には、detachPage → freePageを順に行うが、
		// detachPageには成功するがfreePageで何らかのエラーが発生した場合、ブロック利用ビットマップ中の
		// 削除したオブジェクトに対応するビットはOFFされた状態で永続化されるが、
		// その物理ページ自体は使用中として残り、次のページ内での isValidBlock(pPage, i) が失敗する可能性がある為、
		// 物理ページ内のオブジェクト数を検査しながら、次のページを再検索します。

		try{
			pPage = 0;
			for ( /*ulNewPageID は初期化済み*/ ; PhysicalFile::ConstValue::UndefinedPageID != ulNewPageID
				 ; ulNewPageID = bDirection_ ? m_pPhysicalFile->getPrevPageID( m_rTransaction, ulNewPageID )
											 : m_pPhysicalFile->getNextPageID( m_rTransaction, ulNewPageID )
				)
			{
				pPage = setCurrentPage(ulNewPageID, Buffer::Page::FixMode::ReadOnly);
				Common::UnsignedIntegerData	cCount( 0 );

				// cCountに値を代入
				FileCommon::DataManager::accessToCommonData(pPage, 0, cCount, FileCommon::DataManager::AccessRead);
				//	ここで0はカウント領域へのoffset
				if( 0 < cCount.getValue() ) break;// pPage は attach されたまま抜ける。

				// 次ページへ移る前に detach
				unsetCurrentPage(pPage, PhysicalFile::Page::UnfixMode::NotDirty);
				pPage = 0;//例外処理を考慮し、detach に成功したらポインタをクリア

				_SYDNEY_FAKE_ERROR("Vector::PageManager::nextVectorKey_2",Exception::MemoryExhaust(moduleName, srcFile, __LINE__));
			}
		} catch( ... ) {
			// アタッチ前のページが残っていたらデタッチ 
			if (pPage) unsetCurrentPage(pPage, PhysicalFile::Page::UnfixMode::NotDirty);
			_SYDNEY_RETHROW;
		}

		// 「次のページ」がない場合、上の関数は-1を返す
		if (ulNewPageID == PhysicalFile::ConstValue::UndefinedPageID) { 
			return ulVectorKey_;
		} else {
			// あれば次ページの走査(このページの中に必ずあるはず)
			// pPage は attach済み
			try {
				//- この実装では無駄なブロックも読む恐れがあるが
				//- とりあえず気にしない
				for(ModInt32 i = ulStartBlockID; i != ulEndBlockID; i += d) {
					if (isValidBlock(pPage, i)) {
						unsetCurrentPage(pPage, PhysicalFile::Page::UnfixMode::NotDirty);
						return (ulNewPageID-1) * ulBPP + i;
					}
				}
			} catch (...) {
				unsetCurrentPage(pPage, PhysicalFile::Page::UnfixMode::NotDirty);
				_SYDNEY_RETHROW;
			}
		}

		SydErrorMessage << "can't happen" << ModEndl;
		unsetCurrentPage(pPage, PhysicalFile::Page::UnfixMode::NotDirty);
		SydAssert(false);
	}
	return 0; // dummy to satisfy the compiler
}

//	FUNCTION
//	PageManager::createFileInformation --
//	  ファイル情報を作成する
//
//	NOTES
//	ファイル情報を作成する
//	!leakable!
//
//	ARGUMENTS
//	Buffer::Page::FixMode::Value eFixMode_
//
//	RETURN
//	FileInformation* 
//
//	EXCEPTIONS
//	Unexpected
//	  FileInformation::FileInformationから上がってくる
//
void
PageManager::createFileInformation()
{
	try
	{
		_SYDNEY_FAKE_ERROR("Vector::PageManager::createFileInformation_1",Exception::MemoryExhaust(moduleName, srcFile, __LINE__));
		m_pPhysicalFile->allocatePage(m_rTransaction, m_ulFileInformationPageID);
	
		_SYDNEY_FAKE_ERROR("Vector::PageManager::createFileInformation_2",Exception::MemoryExhaust(moduleName, srcFile, __LINE__));
		FileCommon::AutoPhysicalPage pPage( m_pPhysicalFile
											,m_rTransaction
											,m_ulFileInformationPageID
											,Buffer::Page::FixMode::Write
											,Buffer::ReplacementPriority::Middle
			);
		// 得たポインタを用いてFileInformationを初期化
		FileInformation cFileInformation( pPage, Buffer::Page::FixMode::Write, true );
		cFileInformation.initialize();
		pPage.dirty();
	}
	catch (...)
	{
		m_pPhysicalFile->unfixVersionPage(false);
		_SYDNEY_RETHROW;
	}
	m_pPhysicalFile->unfixVersionPage(true);
}

//	FUNCTION
//	PageManager::attachFileInformation --
//	  ファイル情報をアタッチする
//
//	NOTES
//	ファイル情報をアタッチする
//	!leakable!
//
//	ARGUMENTS
//	Buffer::Page::FixMode::Value eFixMode_
//
//	RETURN
//	FileInformation* 
//
//	EXCEPTIONS
//	Unexpected
//	  FileInformation::FileInformationから上がってくる
//
FileInformation* 
PageManager::attachFileInformation
	(Buffer::Page::FixMode::Value eFixMode_)
{
	// 該当ページ先頭へのポインタを得る
	_SYDNEY_FAKE_ERROR("Vector::PageManager::attachFileInformation",Exception::MemoryExhaust(moduleName, srcFile, __LINE__));

	if (m_pCacheFileInformation) {
		if (m_pCacheFileInformation->getFixMode() == eFixMode_) {
			return m_pCacheFileInformation;
		} else {
			FileInformation* pFileInfo = m_pCacheFileInformation;
			m_pCacheFileInformation = 0;
			detachFileInformation(pFileInfo);
		}
	}

	FileCommon::AutoPhysicalPage pPage( m_pPhysicalFile
	                                   ,m_rTransaction
	                                   ,m_ulFileInformationPageID
	                                   ,eFixMode_
	                                   ,Buffer::ReplacementPriority::Middle
	                                  );
	SydAssert(pPage != 0);

	// 得たポインタを用いてFileInformationを初期化し、
	// それへのポインタを返り値とする
	ModAutoPointer<FileInformation> pFileInformation( new FileInformation(pPage, eFixMode_) );
	SydAssert(pFileInformation!=0);

	// new FileInformation() が成功したら、pPage の所有権を放棄
	pPage.release();

	// cache
	if ((m_rTransaction.isNoVersion() == false)
		||
		m_pOpenParameter->isBatchMode()) {
		m_pCacheFileInformation = pFileInformation.get();
	}

	return pFileInformation.release();
}

//	FUNCTION
//	PageManager::detachFileInformation --
//	  ファイル情報をデタッチする
//
//	NOTES
//	ファイル情報をデタッチする
//
//	ARGUMENTS
//  FileInformation*& rpFileInformation_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Unexpected
//	  FileInformation::FileInformationから上がってくる
//
void 
PageManager::detachFileInformation(FileInformation*& rpFileInformation_)
{
	// LastPageIDをこの機会にセット
	_SYDNEY_FAKE_ERROR("Vector::PageManager::detachFileInformation",Exception::MemoryExhaust(moduleName, srcFile, __LINE__));

	if (m_pCacheFileInformation == rpFileInformation_) {
		// if file infomation is cached, nothing to do
		return;
	}

	PhysicalFile::Page::UnfixMode::Value eUnfixMode;
	if (rpFileInformation_->getFixMode()
		== Buffer::Page::FixMode::ReadOnly) {
		eUnfixMode = PhysicalFile::Page::UnfixMode::NotDirty;
	} else {
		eUnfixMode = PhysicalFile::Page::UnfixMode::Dirty;
	}
	PhysicalFile::Page* pPage = rpFileInformation_->getPage();
	m_pPhysicalFile->detachPage(pPage, eUnfixMode);
	pPage = 0;

	// Not set false to m_bLastPageIDValid.
	// Because, FileInformation does NOT have any cache like m_ulLastPageID.

	delete rpFileInformation_, rpFileInformation_ = 0;
}

//// Private Functions

//	FUNCTION
//	PageManager::setCurrentPage -- 指定したIDのページをセット
//
//	NOTES
//	指定したIDのページをインスタンス内に保持する。
//	FileInformationのページには使用できない。
//
//	ARGUMENTS
//	ModUInt32 ulPageID_
//	Buffer::Page::FixMode::Value eFixMode_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
const PhysicalFile::Page*
PageManager::setCurrentPage(ModUInt32 ulPageID_,
							Buffer::Page::FixMode::Value eFixMode_)
{
//	SydMessage << "@@@" << ulPageID_ << ModEndl;
	_SYDNEY_FAKE_ERROR("Vector::PageManager::setCurrentPage",Exception::MemoryExhaust(moduleName, srcFile, __LINE__));

	PhysicalFile::Page* pPage = 0;

	if (m_pCachePage)
	{
		if (m_pCachePage->getID() == ulPageID_)
		{
			pPage = m_pCachePage;
			m_pCachePage = 0;
		}
	}

	if (pPage == 0)
		pPage = m_pPhysicalFile->attachPage(m_rTransaction, ulPageID_, 
											eFixMode_,
											Buffer::ReplacementPriority::Low);

	return pPage;
}

//	FUNCTION
//	PageManager::unsetCurrentPage -- 
//	  現在保持しているページを手放す
//
//	NOTES
//	現在保持しているページを手放す。
//	FileInformationのページには使用できない。
//
//	ARGUMENTS
//	const PhysicalFile::Page* pPage_
//
//	PhysicalFile::Page::UnfixMode::Value eUnfixMode_
//
//	bool bFreePage_
//	ページを解放するかどうかを指定する。デフォルトはfalse。
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	今のところなし
//
void
PageManager::unsetCurrentPage(
			const PhysicalFile::Page* pPage_,
			PhysicalFile::Page::UnfixMode::Value eUnfixMode_,
			bool bFreePage_)
{
	// getFixMode() では例外は発生しないはず
	if (pPage_->getFixMode() == Buffer::Page::FixMode::ReadOnly) {
		eUnfixMode_ = PhysicalFile::Page::UnfixMode::NotDirty;
	}	

	PhysicalFile::PageID	iPhysicalFilePageID = pPage_->getID();	

	if ((m_rTransaction.isNoVersion() == false)
		||
		(m_pOpenParameter->isBatchMode()))
	{
		if (m_pCachePage)
		{
			// 古いキャッシュをdetachする
			m_pPhysicalFile->detachPage(m_pCachePage, eUnfixMode_);
			// Not set false to m_bLastPageIDValid.
			// Because, m_ulLastPageID is NOT changed in this transaction.
		}
		m_pCachePage = const_cast<PhysicalFile::Page*>(pPage_);
	}
	else
	{
		PhysicalFile::Page* pNonConstPage = 
			const_cast<PhysicalFile::Page*>(pPage_);
		m_pPhysicalFile->detachPage(pNonConstPage, eUnfixMode_);
		// For getting latest m_ulLastPageID in initLastPageID() next time.
		m_bLastPageIDValid = false;
	}

	if (bFreePage_) {
		try{
			_SYDNEY_FAKE_ERROR("Vector::PageManager::unsetCurrentPage",Exception::MemoryExhaust(moduleName, srcFile, __LINE__));
			m_pPhysicalFile->freePage(m_rTransaction, iPhysicalFilePageID);
		} catch( ... ) {
			// Vector::File::expunge() → Vector::PageManager::detachObject() →
			//		Vector::PageManager::unsetCurrentPage()内において、物理ページ内で
			// 唯一のオブジェクトを削除する場合には、detachPage → freePageを順に行うが、
			// detachPageには成功するがfreePageで何らかのエラーが発生した場合、ここに到達する。
			// ブロック利用ビットマップ中の削除したオブジェクトに対応するビットはOFFされた状態で
			// 永続化され、その物理ページ自体は使用中として残るが、Vectorモジュールでは問題ありません
		}
	}
	m_pPhysicalFile->unfixVersionPage(true);
}


//
//	FUNCTION
//	Vector::PageManager::constructObject -- 引数に対応したオブジェクトを生成し、ポインタを返す
//
//	NOTE
//		引数に対応したオブジェクトを生成し、ポインタを返す
//		!Leakable!
//
//	ARGUMENTS
//		char* pPageHead_
//			
//		ModUInt32 ulBlockID_
//			
//	RETURN
//		Object*
//
//	EXCEPTIONS
//		なし
//			
Vector::Object*
PageManager::constructObject(
		 const PhysicalFile::Page* pPage_, ModUInt32 ulBlockID_)
{
	ModOffset ulBitMapOffset = 
		m_rFileParameter.getBitMapAreaOffset() + (ulBlockID_/8);
	ModOffset ulBlockOffset = 
		m_rFileParameter.getBlockAreaOffset()
		+ m_rFileParameter.getBlockSize() * ulBlockID_;
	char ucsBitMask = 0x80 >> (ulBlockID_ % 8);

	Vector::Object* p = 0;
	
	if (m_pCacheObject == 0)
	{
		p = new Vector::Object(m_rFileParameter, *m_pOpenParameter, pPage_,
							   ulBlockOffset, ulBitMapOffset, ucsBitMask);
	}
	else
	{
		m_pCacheObject->reset(pPage_, ulBlockOffset,
							  ulBitMapOffset, ucsBitMask);
		p = m_pCacheObject;
		m_pCacheObject = 0;
	}

	return p;
}

//	FUNCTION Vector::PageManager::isValidBlock
//	-- *m_pCurrentPageの指定したブロックに対応するビットを調べる
//
//	NOTES
//	現在保持しているページの指定したブロックに対応するビットを調べる。
//
//	ARGUMENTS
//
//
//	RETURN
//	bool
//	  true:  ブロックは使用中である
//	  false: ブロックは使用中ではない
//
//	EXCEPTIONS
//	BadArgument
//	  CurrentPageがセットされていないにもかかわらず
//	  この関数を使おうとした
//
bool 
PageManager::isValidBlock(
    const PhysicalFile::Page* pPage_,
	ModUInt32 ulBlockID_) const
{
	// まず範囲チェック
	if (ulBlockID_ >= m_rFileParameter.getBlocksPerPage()) {
		SydDebugMessage << "out of bound." << ModEndl;
		return false;	
	}

	const char* pPageHead = *pPage_;
	const char* pBitMap = pPageHead + 
		m_rFileParameter.getBitMapAreaOffset() + (ulBlockID_/8);
	unsigned char  ucsBitMask = 0x80 >> (ulBlockID_ % 8);
	return *pBitMap & ucsBitMask;
}

//	FUNCTION
//	PageManager::getPageID const
//	  -- VectorKeyに対応するページIDを返す
//
//	NOTES
//	VectorKeyに対応するページIDを返す
//	BlocksPerPageはファイルによって変わり得るので、
//	この関数はstaticにできない。
//
//	ARGUMENTS
//	const ModUInt32 ulVectorKey_
//
//	RETURN
//	const ModUInt32
//
//	EXCEPTIONS
//	なし
//
ModUInt32
PageManager::getPageID(ModUInt32 ulVectorKey_) const
{
	if (ulVectorKey_ == FileCommon::VectorKey::Undefined)
		return 0;
	else 
		return m_ulFileInformationPageNumber + 
		(ulVectorKey_ / m_rFileParameter.getBlocksPerPage());
}

//	FUNCTION
//	PageManager::getBlockID const
//	  -- VectorKeyに対応するブロックIDを返す
//
//	NOTES
//	VectorKeyに対応するブロックIDを返す
//
//	ARGUMENTS
//	ModUInt32 ulVectorKey_
//
//	RETURN
//	ModUInt32
//
//	EXCEPTIONS
//	なし
//
ModUInt32
PageManager::getBlockID(ModUInt32 ulVectorKey_) const
{
	return ulVectorKey_ % m_rFileParameter.getBlocksPerPage();
}

//	FUNCTION
//	PageManager::initLastPageID
//		 -- m_ulLastPageID の値をページから読み込んで初期化。
//
//	NOTES
//	m_ulLastPageID の値が、正当でなければ初期化する。ページから読み込む。
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
void
PageManager::initLastPageID()
{
	if ( !m_bLastPageIDValid ) {//まだ m_bLastPageID の初期化がされていない場合にのみ実行
		// LastPageIDをセット
		_SYDNEY_FAKE_ERROR("Vector::PageManager::initLastPageID_2",Exception::MemoryExhaust(moduleName, srcFile, __LINE__));

		m_ulLastPageID = m_pPhysicalFile->getLastPageID( m_rTransaction );
		m_bLastPageIDValid = true;
	}
}

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
