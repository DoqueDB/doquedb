// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoVerification.h --	オートベリフィケーションクラス関連の
//							クラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VERSION_AUTOVERIFICATION_H
#define	__SYDNEY_VERSION_AUTOVERIFICATION_H

#include "Version/Module.h"
#include "Version/Verification.h"

#include "ModAutoPointer.h"

_SYDNEY_BEGIN
_SYDNEY_VERSION_BEGIN

//	CLASS
//	Version::AutoVerification -- オートベリフィケーションクラスを表すクラス
//
//	NOTES

class AutoVerification
	: public	ModAutoPointer<Verification>
{
public:
	// コンストラクター
	AutoVerification(Verification* verification, bool reserve);
	// デストラクター
	~AutoVerification();

	// 整合性検査に関する情報を表すクラスを破棄する
	void					free(bool reserve);

//	virtual void			free();
//	は呼び出すことはないので定義しない

private:
	// また参照されるときのために破棄せずにとっておくか
	const bool				_reserve;
};

//	FUNCTION public
//	Version::AutoVerification::AutoVerification --
//		オートベリフィケーションクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification*	verification
//			オートベリフィケーションクラスが保持する
//			整合性検査に関する情報を表すクラスを格納する領域の先頭アドレス
//		bool				reserve
//			true
//				このクラスの破棄時に、保持する整合性検査に関する
//				情報を表すクラスがどこからも参照されなくても、
//				また参照されるときのためにとっておく
//			false
//				このクラスの破棄時に、保持する整合性検査に関する
//				情報を表すクラスがどこからも参照されなければ、破棄する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
AutoVerification::AutoVerification(Verification* verification, bool reserve)
	: ModAutoPointer<Verification>(verification),
	  _reserve(reserve)
{}

//	FUNCTION public
//	Version::AutoVerification::~AutoVerification --
//		オートベリフィケーションクラスのデストラクター
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
AutoVerification::~AutoVerification()
{
	free(_reserve);
}

//	FUNCTION public
//	Version::AutoVerification::free --
//		保持する整合性検査に関する情報を表すクラスを破棄する
//
//	NOTES
//		オートベリフィケーションクラスの破棄時などに呼び出される
//
//	ARGUMENTS
//		bool			reserve
//			true
//				どこからも参照されなくなった整合性検査に関する情報を
//				表すクラスでも、また参照されるときのために破棄せずにとっておく
//			false
//				どこからも参照されなくなった整合性検査に関する情報を
//				表すクラスは破棄する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
AutoVerification::free(bool reserve)
{
	if (isOwner())
		if (Verification* verification = release())
			Verification::detach(verification, reserve);
}

_SYDNEY_VERSION_END
_SYDNEY_END

#endif	// __SYDNEY_VERSION_AUTOVERIFICATION_H

//
// Copyright (c) 2001, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

