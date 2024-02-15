// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PageManager.h -- ページを管理する
// 
// Copyright (c) 2005, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VECTOR2_PAGEMANAGER_H
#define __SYDNEY_VECTOR2_PAGEMANAGER_H

#include "Vector2/Module.h"

#include "Buffer/Page.h"
#include "Os/Memory.h"
#include "Version/Page.h"
#include "ModTypes.h"

_SYDNEY_BEGIN

namespace Trans
{
	class Transaction;
}
namespace Version
{
	class File;
}

_SYDNEY_VECTOR2_BEGIN

class VectorFile;

//
//	CLASS
//	Vector2::PageManager -- ページを管理する
//
//	NOTES
//	【管理対象】
//	PageManagerで保持するのは管理ページのみ。
//	データページに対しては、次ページの取得やアタッチなどは実行するが、
//	保持はしないし、データの書き込みなどもVectorFileが実行する。
//
//	【ページの種類】
//	Vector2には以下の３種類のページがある。
//	(+管理ページ)
//		+ヘッダページ
//		+管理テーブルページ
//	+データページ
//
//	各ページの詳細は以下のとおり。
//	+ヘッダページ
//		以下二つのデータからなる。
//		+ビットマップ
//			class Bitmapを参照
//		+ヘッダ
//			struct Headerを参照
//	+管理テーブルページ
//		ヘッダページのビットマップに収まらなかったページを管理するページ。
//		以下二つのデータからなる。
//		+ビットマップ
//		+未使用領域
//			実装簡単化のためヘッダページのヘッダ部分と重なる領域を使わない
//	+データページ
//		データが詰まっている。
//		-> VectorFile.hのPageHeaderを参照
//
//	各ページのページID(Version::Page::ID)は以下のとおり。
//
//	  0   1   2  ... k-1  k  k+1 k+2 ... (ID)
//	-----------------------------------------
//	| H | D | D |...| D | T | D | D |...
//	-----------------------------------------
//	H: ヘッダページ
//	T: 管理テーブルページ
//	D: データページ
//
//	k: 管理ページが管理するページ数 = getPagePerTable()
//
//	注意：
//		管理ページが管理するページ数には、管理ページ自身も含まれる。
//		ページIDからビットマップ内の位置への変換や、閾値処理の簡略化のため
//
//	【ページの初期化】
//	管理ページは、ページ全部が0x00で初期状態。
//	-> PageManager::Bitmapを参照
//	ちなみに、FixMode::AllocateでVersion::Page::fixすると0x00で初期化される。
//	
//	-> データページは、VectorFile::PageHeaderを参照
//
//	【ファイルとは】
//	ファイルとはページから構成されているので、
//	ページ単位でメモリに領域を確保する必要がる。
//	これにはVersion::Page::fixが使える。
//	このfixは、Buffer::Page::FixModeによって動作が異なる。
//	例えば、Allocateは、新たに領域を確保したことを意味するので、
//	0x00で初期化された領域が得られる。
//	この領域への処理が終了したら、unfixで領域をファイルに書き込む。
//
//	また、あるページにデータが一件も入っていなくても、
//	そのページの領域はファイルに確保されている。
//	わざわざ空のページを削除してファイルを圧縮するようなことはしない。
//
//	そういう仕様もあって、insertする時はデータを入れるページより前のページも、
//	全てallocateされている必要がある。
//
//	【ページのattachとは】
//	実際していることはfix。上記のような処理を上位層から隠している。
//
//	【オブジェクトの寿命】
//	PageManagerはVectorFile（のみ？）から使われ、
//	VectorFileのコンストラクタでコンストラクタされる。
//	VectorFile::open()時に、各値が設定される。
//	[?] なんでコンストラクタで設定しない？
//	[?] VectorFileのデストラクタで一緒にデストラクされる？
//
//	【ヘッダページと管理テーブルページ】
//	当初は、ヘッダを持つページをヘッダページ、
//	現在のページIDを管理するページを管理テーブルページ、
//	ヘッダページと管理テーブルページが同じページの場合もある、
//	と思ったが、同じページを指さない実装になった。
//	[?] どちらでもいい？
//
//	【データ領域とは】
//	-> FileID.hのページデータサイズを参照
//
class PageManager
{
public:
	struct Operation
	{
		enum Value
		{
			Read,
			Insert,
			Expunge,
			Update
		};
	};
	
	//
	//	STRUCT
	//	Vector2::PageManager::Header -- Vectorファイルの内容
	//
	//	NOTES
	//	ヘッダページの先頭に記録する。
	//	バイト境界を考えて固定長のデータは可変長のデータより前に記録する。
	//	-> 構造はPageManager::Bitmapを参照
	//
	struct Header
	{
		ModUInt32			m_uiCount;			// エントリ数
		ModUInt32			m_uiMaxPageID;		// 最大ページ番号
	};

	// コンストラクタ
	PageManager();
	// デストラクタ
	~PageManager();

	// ベクターファイルを設定する
	void setVectorFile(VectorFile* pVectorFile)
		{ m_pVectorFile = pVectorFile; }

	void open(const Trans::Transaction& cTransaction_,
			  Buffer::Page::FixMode::Value eFixMode_,
			  Os::Memory::Size uiPageDataSize_);
	void close();

	// ヘッダーを初期化する
	void initialize(const Trans::Transaction& cTransaction_);

	// バージョンファイルをセットする
	void setVersionFile(Version::File* pVersionFile_);

	// 指定されたページをfixする
	// 存在していなかったらisOwner()==falseなページを返す
	Version::Page::Memory attach(Version::Page::ID uiPageID_,
								 Operation::Value eOperation_);

	// 指定されたページをverifyでfixする
	// 存在していなかったらisOwner()==falseなページを返す
	Version::Page::Memory verify(
		Version::Page::ID uiPageID_,
		Admin::Verification::Progress& cProgress_);
	//Version::Page::Memory verify(
	//	Version::Page::ID uiPageID_,
	//	Operation::Value eOperation_,
	//	Admin::Verification::Progress& cProgress_);

	// カウントを得る
	ModSize getCount();

	// エントリ数を増やす
	void incrementCount();
	// エントリ数を減らす
//	void decrementCount(Version::Page::ID uiPageID_ = Version::Page::IllegalID);
	// uiPageID_を指定して何をするのかわからん->とりあえず引数削除
	void decrementCount();

	// 次のページを得る
	Version::Page::ID next(Version::Page::ID uiCurrentPageID_) const;
	// 前のページを得る
	Version::Page::ID prev(Version::Page::ID uiCurrentPageID_) const;

	// 最大ページIDを得る
	Version::Page::ID getMaxPageID() const;

	// 管理ページをunfixする
	void detachManagePage();

	// データ領域のサイズを得る
	Os::Memory::Size getPageDataSize() const { return m_uiPageDataSize; }
	// 管理テーブルページ1ページあたりの管理ページ数を得る -> NOTESを参照
	ModSize getPagePerTable() const
		{ return (m_uiPageDataSize - sizeof(Header)) * 8; }

	// ビットをONする
	void on(Version::Page::ID uiPageID_);
	// ビットをOFFにする
	void off(Version::Page::ID uiPageID_);

	// ビットを返す
	bool getBit(const Version::Page::ID uiPageID_);
	
private:
	//
	//	CLASS
	//	Vector2::PageManager::Bitmap -- 使用ページを管理するビットマップ
	//
	//	NOTES
	// 	ページに対応するビットがたっていれば使用中である。
	//
	//	Vectorは、キー(ROWID)の順番でバリュー(RecordのObjectID)を順々に
	//	取得できるが、その際データページにデータがないことがわかれば、
	//	無駄なアタッチをしなくてもよくなる。
	//
	//	【ページの初期化】
	//	エントリ数、最大ページ番号、ビットマップ、ともに全て0。
	//
	//	【ページの構造】
	//  0		   4			   8			m_uiPageDataSize(byte)
	//	------------------------------------------
	//	| m_uiCount | m_uiMaxPageID | Bitmap ... |
	//	------------------------------------------
	//							  /				  \
	//							/					\
	//						  /						  \
	//						9				  10		\ 
	//					   -------------------------------
	//					   | 7 6 5 4 3 2 1 0 | 15 14 ... |
	//					   -------------------------------
	//										(管理ページからの相対ページID)
	// 
	//	(相対ページIDが表すページ) = (管理ページのページID)
	//								 + (相対ページID)
	//
	//	実装の都合で、相対ページID 0 は管理ページを指すと考えておく。
	//	なので、自身をこのビットで管理するわけではない。
	//
	//	ヘッダ(m_uiCount, m_uiMaxPageID)は、ヘッダページのみ記録され、
	//	管理テーブルページでは未使用領域である。
	//
	class Bitmap
	{
	public:
		// コンストラクタ
		Bitmap(Version::Page::Memory& page);
		// デストラクタ
		~Bitmap();

		// 次のビットの位置を得る
		ModUInt32 next(ModUInt32 uiCurrentID_) const;
		// 前のビットの位置を得る
		ModUInt32 prev(ModUInt32 uiCurrentID_) const;

		// ビットをONする
		void on(ModUInt32 uiPosition_);
		// ビットをOFFする
		void off(ModUInt32 uiPosition_);

		// ビットを返す
		bool get(const ModUInt32 uiPosition_) const;
		
	private:
		// ビットマップの先頭を得る
		const unsigned char* begin() const;
		unsigned char* begin();

		// ビットマップの終端を得る
		const unsigned char* end() const;
		unsigned char* end();
		
		// ページ
		Version::Page::Memory& m_cPage;
	};
	

	// ヘッダーへのポインタを得る
	Header* getHeader();
	const Header* getConstHeader() const;
	// ページをallocateする
	Version::Page::Memory allocatePage();
	// 管理ページを得る
	Version::Page::Memory& getManagePage(Version::Page::ID uiPageID_) const;

	// ベクターファイル
	VectorFile* m_pVectorFile;
	
	// バージョンファイル
	mutable Version::File* m_pVersionFile;

	// ヘッダーページ
	mutable Version::Page::Memory m_cHeaderPage;
	// 管理テーブルページ
	mutable Version::Page::Memory m_cTablePage;

	// トランザクション
	const Trans::Transaction* m_pTransaction;
	// FIXモード
	Buffer::Page::FixMode::Value m_eFixMode;
	// データ領域のサイズ
	Os::Memory::Size m_uiPageDataSize;
};

_SYDNEY_VECTOR2_END
_SYDNEY_END

#endif // __SYDNEY_VECTOR2_PAGEMANAGER_H

//
//	Copyright (c) 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
