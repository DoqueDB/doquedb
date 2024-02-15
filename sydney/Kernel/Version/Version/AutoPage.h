// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoPage.h -- オートバージョンページ関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VERSION_AUTOPAGE_H
#define	__SYDNEY_VERSION_AUTOPAGE_H

#include "Version/Module.h"
#include "Version/Page.h"

#include "ModAutoPointer.h"

_SYDNEY_BEGIN
_SYDNEY_VERSION_BEGIN

//	CLASS
//	Version::AutoPage -- オートバージョンページ記述子を表すクラス
//
//	NOTES

class AutoPage
	: public	ModAutoPointer<Page>
{
public:
	// コンストラクター
	AutoPage(Page* page, bool reserve);
	// デストラクター
	~AutoPage();

	// バージョンページ記述子を破棄する
	void				free(bool reserve);

//	virtual void			free();
//	は呼び出すことはないので定義しない

private:
	// また参照されるときのために破棄せずにとっておくか
	const bool				_reserve;
};

//	FUNCTION public
//	Version::AutoPage::AutoPage --
//		オートバージョンページ記述子を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Version::Page*		page
//			オートバージョンページ記述子が保持する
//			バージョンページ記述子を格納する領域の先頭アドレス
//		bool				reserve
//			true
//				このクラスの破棄時に、
//				保持するバージョンページ記述子がどこからも参照されなくても、
//				また参照されるときのためにとっておく
//			false
//				このクラスの破棄時に、保持するバージョンページ記述子が
//				どこからも参照されなければ、破棄する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
AutoPage::AutoPage(Page* page, bool reserve)
	: ModAutoPointer<Page>(page),
	  _reserve(reserve)
{}

//	FUNCTION public
//	Version::AutoPage::~AutoPage --
//		オートバージョンページ記述子を表すクラスのデストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
AutoPage::~AutoPage()
{
	free(_reserve);
}

//	FUNCTION public
//	Version::AutoPage::free -- 保持するバージョンページ記述子を破棄する
//
//	NOTES
//		オートバージョンページ記述子の破棄時などに呼び出される
//
//	ARGUMENTS
//		bool			reserve
//			true
//				どこからも参照されなくなったバージョンページ記述子でも、
//				また参照されるときのために破棄せずにとっておく
//			false
//				どこからも参照されなくなったバージョンページ記述子は破棄する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
AutoPage::free(bool reserve)
{
	if (isOwner())
		if (Page* page = release())
			Page::detach(page, reserve);
}

_SYDNEY_VERSION_END
_SYDNEY_END

#endif	// __SYDNEY_VERSION_AUTOPAGE_H

//
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
