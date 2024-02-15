// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PageManager.cpp -- 
// 
// Copyright (c) 2005, 2006, 2009, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Vector2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Vector2/PageManager.h"
#include "Vector2/VectorFile.h"

#include "Os/Memory.h"

#include "Common/Message.h"
#include "Common/Thread.h"

#include "Exception/BadDataPage.h"

_SYDNEY_BEGIN
_SYDNEY_VECTOR2_BEGIN

//
//	FUNCTION public
//	Vector2::PageManager::PageManager -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
PageManager::PageManager()
	: m_pVectorFile(0)
{
	//
	// 自作する
	//

	// 特になし
}

//
//	FUNCTION public
//	Vector2::PageManager::~PageManager -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
PageManager::~PageManager()
{
	//
	// 自作する
	//

	// 特になし
}

//
//	FUNCTION public
//	Vector2::PageManager::open --
//
//	NOTES
// [?] コンストラクタ、デストラクタとは別にopen,closeがあるということは
//		closeしたあとも使いまわすことを想定している？
//		それともVectorFileをコンストラクトした時点では、
//		初期設定するためのデータが足りない？
// [?] initializeで使うTransacsionとの違いは？
//
//	ARGUMENTS
//	const Trans::Transaction&	cTransaction_
//		トランザクション記述子
//  Buffer::Page::FixMode	eFixMode_
//		??ページのFixモード
//	Os::Memory::Size	uiPageDataSize_
//		データ領域のサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
PageManager::open(const Trans::Transaction& cTransaction_,
				  Buffer::Page::FixMode::Value eFixMode,
				  Os::Memory::Size uiPageDataSize_)
{
	//
	// 自作する
	//

	// Btree2/File.cppから引用
	m_pTransaction = &cTransaction_;
	m_eFixMode = eFixMode;
	m_uiPageDataSize = uiPageDataSize_;

	// [?] 他に何する？
}

//
//	FUNCTION public
//	Vector2::PageManager::close --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
PageManager::close()
{
	detachManagePage();

	m_pTransaction = 0;
	m_eFixMode = Buffer::Page::FixMode::Unknown;
}

//
//	FUNCTION public
//	Vector2::PageManager::initialize -- ヘッダーを初期化する
//
//	NOTES
//	create tableの時のみ呼ばれる
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
PageManager::initialize(const Trans::Transaction& cTransaction_)
{
	// FixMode::Allocateでfixしたページは0x00で初期化されているので、
	// ここで初期化の必要はない。
	// また、ReplacementPriority::Valueはv15では使わない。
	m_cHeaderPage
		= Version::Page::fix(cTransaction_,
							 *m_pVersionFile,
							 0,
							 Buffer::Page::FixMode::Allocate);
	m_cHeaderPage.dirty();
}

//
//	FUNCTION public
//	Vector2::PageManager::setVersionFile -- バージョンファイルをセットする
//
//	NOTES
//
//	ARGUMENTS
//	Version::File* pVersionFile_
//		??バージョンファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
PageManager::setVersionFile(Version::File* pVersionFile_)
{
	m_pVersionFile = pVersionFile_;
}

//
//	FUNCTION public
//	Vector2::PageManager::attach -- 指定されたページをfixする
//
//	NOTES
//	insertの場合、先頭からinsertされるページまでの全てのページをallocateする
//
//	ARGUMENTS
//	Version::Page::ID	uiPageID_
//		fixするページのページ識別子
//	PageManager::Operation::Value eOperation_
//		ページの使い道の指定
//
//	RETURN
//	Version::Page::Memory
//		fixしたページを返す
//		存在しないページをattachしたらisOwner()==falseなページを返す
//
//	EXCEPTIONS
//
Version::Page::Memory
PageManager::attach(Version::Page::ID uiPageID_,
					PageManager::Operation::Value eOperation_)
{
	Version::Page::Memory page;
	
	if (eOperation_ == Operation::Insert)
	{
		Header* pHeader = getHeader();

		// 今まで確保されていないページを確保する
		while (pHeader->m_uiMaxPageID < uiPageID_ - 1)
		{
			// allocatePage()はm_uiMaxPageID+1のページをallocateする
			page = allocatePage();
			page.unfix(false);
		}
		if(pHeader->m_uiMaxPageID == uiPageID_ - 1)
		{
			page = allocatePage();
		}
	}

	if (page.getPageID() != uiPageID_)
	{
		// Insert以外の場合や、
		// Insertの場合でも、uiPageID_がすでにallocateされたページの場合
		try
		{
			page = Version::Page::fix(*m_pTransaction,
									  *m_pVersionFile,
									  uiPageID_,
									  m_eFixMode);
		}
		catch (Exception::BadDataPage&)
		{
			// Exception::BadDataPageは不正なページをfixした場合に発生する
			// 不正なページとは
			//	o ファイルサイズを超えたページ
			//	o CRCが不正なページ
			// である
			// CRCが不正な場合は上位にこの例外を投げる必要があるので、
			// ここでヘッダーをチェックし、uiPageID_が最大ページID以下か
			// チェックし、以下なら例外を再送する

			Version::Page::ID uiMaxPageID = Version::Block::IllegalID;
			try
			{
				uiMaxPageID = getConstHeader()->m_uiMaxPageID;
			}
			catch (...)
			{
				// ここでの例外は無視する
				// ここで例外が発生したら、uiMaxPageIDはIllegalIDなので、
				// 必ず uiPageID_ よりも大きくなり BadDataPage が throw される
			}

			if (uiMaxPageID >= uiPageID_)
				// 最大ページIDを超えていないので例外を再送する
				_SYDNEY_RETHROW;

			Common::Thread::resetErrorCondition();

			// 最大ページIDを超えているので、isOwner() == false なページを返す
		}
	}

	return page;
}

//
//	FUNCTION public
//	Vector2::PageManager::verify -- 指定されたページをverifyでfixする
//
//	NOTES
//	存在していなかったらisOwner()==falseなページを返す
//	Vector2のverifyはファイルを訂正できないため常にReadなのでOperationは不要
//
//	ARGUMENTS
//	Version::Page::ID	uiPageID_
//		??
//	//Buffer::Page::FixMode::Value eMode_
//	//	??
//	Admin::Verification::Progress&	cProgress_
//		??
//
//	RETURN
//	Version::Page::Memory
//		??
//
//	EXCEPTIONS
//
Version::Page::Memory
PageManager::verify(Version::Page::ID uiPageID_,
					Admin::Verification::Progress& cProgress_)
//PageManager::verify(Version::Page::ID uiPageID_,
//					Operation::Value eOperation_,
//					Admin::Verification::Progress& cProgress_)
{
	Version::Page::Memory page;
	try
	{
		page = Version::Page::verify(*m_pTransaction,
									 *m_pVersionFile,
									 uiPageID_,
									 m_eFixMode,
									 cProgress_);
	}
	catch (Exception::BadDataPage&)
	{
		// Exception::BadDataPageは不正なページをfixした場合に発生する
		// 不正なページとは
		//	o ファイルサイズを超えたページ
		//	o CRCが不正なページ
		// である
		// CRCが不正な場合は上位にこの例外を投げる必要があるので、
		// ここでヘッダーをチェックし、uiPageID_が最大ページID以下か
		// チェックし、以下なら例外を再送する

		Version::Page::ID uiMaxPageID = Version::Block::IllegalID;
		try
		{
			uiMaxPageID = getConstHeader()->m_uiMaxPageID;
		}
		catch (...)
		{
			// ここでの例外は無視する
			// ここで例外が発生したら、uiMaxPageIDはIllegalIDなので、
			// 必ず uiPageID_ よりも大きくなり BadDataPage が throw される
		}

		if (uiMaxPageID >= uiPageID_)
			// 最大ページIDを超えていないので例外を再送する
			_SYDNEY_RETHROW;

		Common::Thread::resetErrorCondition();

		// 最大ページIDを超えているので、isOwner() == false なページを返す
	}

	return page;
}

//
//	FUNCTION public
//	Vector2::PageManager::getCount -- カウントを得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	ModSize
//		Vectorに入っているエントリの個数
//
//	EXCEPTIONS
//
ModSize
PageManager::getCount()
{
	return getConstHeader()->m_uiCount;
}

//
//	FUNCTION public
//	Vector2::PageManager::incrementCount -- エントリ数を増やす
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
PageManager::incrementCount()
{
	Header* pHeader = getHeader();
	m_cHeaderPage.dirty();
	pHeader->m_uiCount++;
}

//
//	FUNCTION public
//	Vector2::PageManager::decrementCount -- エントリ数を減らす
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
PageManager::decrementCount()
{
	Header* pHeader = getHeader();
	m_cHeaderPage.dirty();
	pHeader->m_uiCount--;
}

//
//	FUNCTION public
//	Vector2::PageManager::next -- 次のページを得る
//
//	NOTES
//	指定されたページより後のページの中で、
//	データが入っているページのページIDを返す。
//
//	ARGUMENTS
//	Version::Page::ID	uiCurrentPageID_
//		ページID
//
//	RETURN
//	Version::Page::ID
//		ページID
//		得られない場合は、Version::Block::IllegalIDを返す。
//
//	EXCEPTIONS
//
Version::Page::ID
PageManager::next(Version::Page::ID uiCurrentPageID_) const
{
	const Header* pHeader = getConstHeader();
	Version::Page::ID nextID = Version::Block::IllegalID;
	while (uiCurrentPageID_ <= pHeader->m_uiMaxPageID)
	{
		//現在のページIDが含まれる管理テーブルページを取得する。
		Version::Page::Memory& page = getManagePage(uiCurrentPageID_);
		
		// ビットマップから次のページを求める
		Bitmap map(page);
		// nextIDは、この時点ではBitmap内の位置を示す。
		nextID = map.next(uiCurrentPageID_ - page.getPageID());
		
		if (nextID == Version::Block::IllegalID)
		{
			// この管理ページにはもうページは存在していない
			// 次の管理ページのIDを使う
			
			// uiCurrentPageID_は、
			// 次に調べる管理テーブルページのIDを示すようになる。
			uiCurrentPageID_ = page.getPageID() + getPagePerTable();
		}
		else
		{
			// 見つかった
			// nextIDをBitmap内の位置からVectorFile内の位置に戻す。
			nextID += page.getPageID();
			break;
		}
	}
		
	return nextID;
}

//
//	FUNCTION public
//	Vector2::PageManager::prev -- 前のページを得る
//
//	NOTES
//	指定されたページより前のページの中で、
//	データが入っているページのページIDを返す。
//
//	ARGUMENTS
//	Version::Page::ID	uiCurrentPageID_
//		ページID
//
//	RETURN
//	Version::Page::ID
//		ページID
//		得られない場合は、0を返す。
//
//	EXCEPTIONS
//
Version::Page::ID
PageManager::prev(Version::Page::ID uiCurrentPageID_) const
{
	Version::Page::ID prevID = 0;
	while (true)
	{
		//現在のページIDが含まれる管理テーブルページを取得する。
		Version::Page::Memory& page = getManagePage(uiCurrentPageID_);
		
		// ビットマップから次のページを求める
		Bitmap map(page);
		// prevIDは、この時点ではBitmap内の位置を示す。
		prevID = map.prev((uiCurrentPageID_ == page.getPageID()) ?
						  Version::Block::IllegalID :
						  uiCurrentPageID_ - page.getPageID());
		
		if (prevID == 0)
		{
			
			// この管理ページにはもうページは存在していない
			// 次の管理ページのIDを使う

			if (page.getPageID() == 0)
				// ヘッダページなので次の管理ページは存在しない
				break;
			
			// uiCurrentPageID_は、
			// 次に調べる管理テーブルページのIDを示すようになる。
			uiCurrentPageID_ = page.getPageID() - getPagePerTable();
		}
		else
		{
			// 見つかった
			// prevIDをBitmap内の位置からVectorFile内の位置に戻す。
			prevID += page.getPageID();
			break;
		}
	}
	
	return prevID;
}

//
//	FUNCTION public
//	Vector2::PageManager::getMaxPageID -- 最大ページIDを得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	Version::Page::ID
//		最大のページID
//
//	EXCEPTIONS
//
Version::Page::ID
PageManager::getMaxPageID() const
{
	return getConstHeader()->m_uiMaxPageID;
}

//
//	FUNCTION public
//	Vector2::PageManager::detachManagePage -- 管理ページをunfixする
//
//	NOTES
//	キャッシュしている管理ページをunfixする
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
PageManager::detachManagePage()
{
	m_cHeaderPage.unfix(false);
	m_cTablePage.unfix(false);
}

//
//	FUNCTION public
//	Vector2::PageManager::on -- ページに該当するビットをONする
//
//	NOTES
//
//	ARGUMENTS
//	Version::Page::ID uiPageID_
//		ONするページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
PageManager::on(Version::Page::ID uiPageID_)
{
	// 管理ページを得る
	Version::Page::Memory& table = getManagePage(uiPageID_);

	// ビットマップを更新する
	Bitmap map(table);
	map.on(uiPageID_ - table.getPageID());
}

//
//	FUNCTION public
//	Vector2::PageManager::off -- ページに該当するビットをOFFにする
//
//	NOTES
//
//	ARGUMENTS
//	Version::Page::ID uiPageID_
//		OFFにするページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
PageManager::off(Version::Page::ID uiPageID_)
{
	// 管理ページを得る
	Version::Page::Memory& table = getManagePage(uiPageID_);

	// ビットマップを更新する
	Bitmap map(table);
	map.off(uiPageID_ - table.getPageID());
}

//
//	FUNCTION public
//	Vector2::PageManager::getBit -- ビットを返す
//
//	NOTES
//
//	ARGUMENTS
//	Version::Page::ID uiPageID_
//		返すビットに対応するページID
//
//	RETURN
//	bool
//		ページに該当するビットを返す
//		ONならtrue、OFFならfalseを返す
//
//	EXCEPTIONS
//
bool
PageManager::getBit(const Version::Page::ID uiPageID_)
{
	// 管理ページを得る
	Version::Page::Memory& table = getManagePage(uiPageID_);

	// ビットを取得する
	Bitmap map(table);
	return map.get(uiPageID_ - table.getPageID());
}

//
//	FUNCTION public
//	Vector2::PageManager::Bitmap::Bitmap -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Version::Page::Memory& page
//		??ビットマップが格納されるページ
//	Os::Memory::Size uiSize_
//		??利用可能なページサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
PageManager::Bitmap::Bitmap(Version::Page::Memory& page)
	: m_cPage(page)
{
}

//
//	FUNCTION public
//	Vector2::PageManager::Bitmap::~Bitmap -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
PageManager::Bitmap::~Bitmap()
{
}

//
//	FUNCTION public
//	Vector2::PageManager::Bitmap::next -- 次のビットを得る
//
//	NOTES
//	指定されたビットより後のビットの中で、立っているビットの位置を得る。
//
//	ARGUMENTS
//	ModUInt32 uiCurrentID_
//		ビットマップ内でのビットの位置
//
//	RETURN
//	ModUInt32
//		ビットマップ内でのビットの位置
//		このビットマップ内にない場合は、Version::Block::IllegalIDを返す。
//
//	EXCEPTIONS
//
ModUInt32
PageManager::Bitmap::next(ModUInt32 uiCurrentID_) const
{
	// ビットマップの先頭
	const unsigned char* s = begin();
	// ビットマップの終端
	const unsigned char* e = end();

	++uiCurrentID_;	// 次

	// メモリの位置を取得
	const unsigned char* p = s + (uiCurrentID_ / 8);
	// バイト内のビットの位置を取得
	ModUInt32 pos = uiCurrentID_ % 8;
	// ビットの位置のマスクを作成
	unsigned char n = 1;
	n <<= pos;

	for (; p < e; ++p)
	{
		if (*p)
		{
			// このバイトには1つ以上のビットが立っている
			for (; n != 0; n <<= 1, ++pos)
				if (*p & n)
					// 見つかった
					return static_cast<ModUInt32>(p - s) * 8 + pos;
		}
		n = 1;
		pos = 0;
	}

	// 見つからなかった
	return Version::Block::IllegalID;
}

//
//	FUNCTION public
//	Vector2::PageManager::Bitmap::prev -- 前のビットを得る
//
//	NOTES
//	指定されたビットより前のビットの中で、立っているビットの位置を得る。
//
//	ARGUMENTS
//	ModUInt32 uiCurrentID_
//		ビットマップ内でのビットの位置
//
//	RETURN
//	ModUInt32
//		ビットマップ内でのビットの位置
//		このビットマップ内にない場合は、0 を返す。
//
//	EXCEPTIONS
//
ModUInt32
PageManager::Bitmap::prev(ModUInt32 uiCurrentID_) const
{
	// ビットマップの先頭
	const unsigned char* s = begin();
	// ビットマップの終端
	const unsigned char* e = end();

	--uiCurrentID_;	// 前
	
	// 超えていたら最大値に変更する
	if (uiCurrentID_ >= static_cast<ModUInt32>(e - s) * 8)
		uiCurrentID_ = static_cast<ModUInt32>(e - s) * 8 - 1;

	// メモリの位置を取得
	const unsigned char* p = s + (uiCurrentID_ / 8);
	// バイト内のビットの位置を取得
	ModUInt32 pos = uiCurrentID_ % 8;
	// ビットの位置のマスクを作成
	unsigned char n = 1;
	n <<= pos;

	for (; p < e; ++p)
	{
		if (*p)
		{
			// このバイトには1つ以上のビットが立っている
			for (;; n >>= 1, --pos)
			{
				if (*p & n)
					// 見つかった
					return static_cast<ModUInt32>(p - s) * 8 + pos;
				if (pos == 0)
					break;
			}
		}
		n = 0x80;
		pos = 7;
	}

	// 見つからなかった
	return 0;
}

//
//	FUNCTION public
//	Vector2::PageManager::Bitmap::on -- ビットをONする
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32	uiPosition_
//		ONにするビットのビットマップページ内での位置
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
// ビットをONする
void
PageManager::Bitmap::on(ModUInt32 uiPosition_)
{
	unsigned char* p = begin() + (uiPosition_ / 8);
	m_cPage.dirty();
	*p |= (1 << (uiPosition_ % 8));
}

//
//	FUNCTION public
//	Vector2::PageManager::Bitmap::off -- ビットをOFFする
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32	uiPosition_
//		OFFにするビットのビットマップページ内での位置
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
PageManager::Bitmap::off(ModUInt32 uiPosition_)
{
	unsigned char* p = begin() + (uiPosition_ / 8);
	m_cPage.dirty();
	*p &= ~(1 << (uiPosition_ % 8));
}

//
//	FUNCTION public
//	Vector2::PageManager::Bitmap::get -- ビットを返す
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32	uiPosition_
//		取得するビットのビットマップページ内での位置
//
//	RETURN
//	bool
//		取得したビットがONならtrue、OFFならfalseを返す
//
//	EXCEPTIONS
//
bool
PageManager::Bitmap::get(const ModUInt32 uiPosition_) const
{
	const unsigned char* p = begin() + (uiPosition_ / 8);
	return *p & (1 << (uiPosition_ % 8));
}

//
//	FUNCTION private
//	Vector2::PageManager::Bitmap::begin -- ビットマップの先頭を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const unsigned char*
//	unsigned char*
//		ビットマップ領域の先頭
//
//	EXCEPTIONS
//
const unsigned char*
PageManager::Bitmap::begin() const
{
	// 下記だとconstなしのcastが呼ばれてしまう。
	//return syd_reinterpret_cast<const unsigned char*>(
	//	static_cast<const char*>(m_cPage) + sizeof(PageManager::Header));
	return syd_reinterpret_cast<const unsigned char*>(
		static_cast<const Version::Page::Memory&>(m_cPage).operator const char*() + sizeof(PageManager::Header));
		//m_cPage.operator const char*() + sizeof(PageManager::Header));
}
unsigned char*
PageManager::Bitmap::begin()
{
	return syd_reinterpret_cast<unsigned char*>(
		static_cast<char*>(m_cPage) + sizeof(PageManager::Header));
}

//
//	FUNCTION private
//	Vector2::PageManager::Bitmap::end -- ビットマップの終端を得る
//
//	NOTES
//	実際は、終端自身ではなく終端の次の位置を得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const unsigned char*
//	unsigned char*
//		ビットマップ領域の終端
//
//	EXCEPTIONS
//
const unsigned char*
PageManager::Bitmap::end() const
{
	// 下記だとconstなしのcastが呼ばれてしまう。
	//return syd_reinterpret_cast<const unsigned char*>(
	//	static_cast<const char*>(m_cPage) + m_cPage.getSize());
	return syd_reinterpret_cast<const unsigned char*>(
		//m_cPage.operator const char*() + m_cPage.getSize());
		static_cast<const Version::Page::Memory&>(m_cPage).operator const char*()+ m_cPage.getSize());
}
unsigned char*
PageManager::Bitmap::end()
{
	return syd_reinterpret_cast<unsigned char*>(
		static_cast<char*>(m_cPage) + m_cPage.getSize());
		
}

//
//	FUNCTION private
//	Vector2::PageManager::getHeader -- ヘッダーへのポインタを得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	Vector2::PageManager::Header*
//		ヘッダーへのポインタ
//
//	EXCEPTIONS
//
PageManager::Header*
PageManager::getHeader()
{
	if (m_cHeaderPage.isOwner() == false)
	{
		m_cHeaderPage = Version::Page::fix(*m_pTransaction,
										   *m_pVersionFile,
										   0,
										   m_eFixMode);
	}
	return syd_reinterpret_cast<Header*>(static_cast<char*>(m_cHeaderPage));
}

const PageManager::Header*
PageManager::getConstHeader() const
{
	if (m_cHeaderPage.isOwner() == false)
	{
		m_cHeaderPage = Version::Page::fix(*m_pTransaction,
										   *m_pVersionFile,
										   0,
										   m_eFixMode);
	}
	// 下記のキャスト方法だと、LogicalInterface::getCount()から呼び出すと
	// oprator char* が呼ばれてしまう
	//return syd_reinterpret_cast<const Header*>(
	//	static_cast<const char*>(m_cHeaderPage));
	return syd_reinterpret_cast<const Header*>(
	static_cast<const Version::Page::Memory&>(m_cHeaderPage).operator const char*());
	  //m_cHeaderPage.operator const char*());
}

//
//	FUNCTION private
//	Vector2::PageManager::allocatePage -- 1ページallocateする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Version::Page::Memory
//		確保したページ
//
//	EXCEPTIONS
//
Version::Page::Memory
PageManager::allocatePage()
{
	Header* pHeader = getHeader();
	// この時点でヘッダに変更を加えると、
	// fixに失敗した時、エラー処理が必要になってしまうので一時変数を使う
	Version::Page::ID maxID = pHeader->m_uiMaxPageID;
	maxID++;

	// 1ページの管理テーブルで管理できるページ数は
	// Version::Page::getSize() - sizeof(Header) バイトのビット数
	// ただし、bit 0 は使用しない
	
	if ((maxID % getPagePerTable()) == 0)
	{
		// ヘッダページは来ない。maxID >= 1 のため。
		// これが呼ばれるまでにfixされてなくとも、getHeader()でfixされる。

		// 管理テーブルページ
		// 初期化の必要はない
		m_cTablePage = Version::Page::fix(*m_pTransaction,
										  *m_pVersionFile,
										  maxID,
										  Buffer::Page::FixMode::Allocate);
		m_cHeaderPage.dirty();
		pHeader->m_uiMaxPageID = maxID;
		maxID++;
	}

	// データページ
	Version::Page::Memory page
		= Version::Page::fix(*m_pTransaction,
							 *m_pVersionFile,
							 maxID,
							 Buffer::Page::FixMode::Allocate);
	// 得られたページを初期化する
	m_pVectorFile->resetPage(page);
	m_cHeaderPage.dirty();
	pHeader->m_uiMaxPageID = maxID;

	return page;
}

//
//	FUNCTION public
//	Vector2::PageManager::getManagePage -- 管理ページを得る
//
//	NOTES
//	指定されたページを管理している管理ページを得る。
//
//	ARGUMENTS
//	Version::Page::ID uiPageID_
//		ページID
//
//	RETURN
//	Version::Page::Memory&
//		管理ページ
//		ページIDが管理ページを指していた場合は、その管理ページを返す。
//
//	EXCEPTIONS
//
Version::Page::Memory&
PageManager::getManagePage(Version::Page::ID uiPageID_) const
{
	// uiPageID_を管理している管理ページのID
	Version::Page::ID uiManageID
		= uiPageID_ / getPagePerTable()	* getPagePerTable();

	if (uiManageID == 0)
	{
		// ヘッダページが管理ページの場合
		if (m_cHeaderPage.isOwner() == false)
			m_cHeaderPage = Version::Page::fix(*m_pTransaction,
											   *m_pVersionFile,
											   uiManageID,
											   m_eFixMode);
		return m_cHeaderPage;
	}

	if (m_cTablePage.isOwner() == false ||
		m_cTablePage.getPageID() != uiManageID)
		m_cTablePage = Version::Page::fix(*m_pTransaction,
										  *m_pVersionFile,
										  uiManageID,
										  m_eFixMode);
	return m_cTablePage;

}

_SYDNEY_VECTOR2_END
_SYDNEY_END

//
//	Copyright (c) 2005, 2006, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
