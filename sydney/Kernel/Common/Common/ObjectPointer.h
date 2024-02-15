// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectPointer.h -- エグゼキュータで使用するオブジェクトポインタ
// 
// Copyright (c) 1999, 2001, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_OBJECTPOINTER_H
#define __TRMEISTER_COMMON_OBJECTPOINTER_H

#include "Common/Module.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//
//	TEMPLATE CLASS
//	Common::ObjectPointer -- エグゼキュータで使用するPointer
//
//	TEMPLATE ARGUMENTS
//	class X
//		Common::ExecutableObjectの派生クラスを想定している。
//		ダウンキャストの発生を押えるために、テンプレートにしている。
//
//	NOTES
//	エグゼキュータで扱うオブジェクトのポインタはすべてこのクラスごしに
//	扱われる。オブジェクトの参照を管理する。
//
template <class X>
class ObjectPointer
{
public:
	// TYPEDEF
	//	Common::ObjectPointer<X>::ContentType -- 保持されるクラスの型
	//
	// NOTES

	typedef X ContentType;

	//
	//	TEMPLATE FUNCTION public
	//	Common::ObjectPointer<X>::ObjectPointer -- コンストラクタ(1)
	//
	//	TEMPLATE ARGUMENTS
	//	class X
	//		Common::ExecutableObjectの派生クラス
	//
	//	NOTES
	//	コンストラクタ
	//
	//	ARGUMENTS
	//	X* pObject_
	//		オブジェクトへのポインタ
	//
	//	RETURN
	//	なし
	//
	//	EXCEPTIONS
	//	なし
	//
	ObjectPointer(X* pObject_ = 0)
		: m_pObject(pObject_), m_fConstant(false)
	{
		addReference();
	}

	//
	//	TEMPLATE FUNCTION public
	//	Common::ObjectPointer<X>::ObjectPointer -- コンストラクタ(2)
	//
	//	TEMPLATE ARGUMENTS
	//	class X
	//		Common::ExecutableObjectの派生クラス
	//
	//	NOTES
	//	コンストラクタ
	//
	//	ARGUMENTS
	//	const X* pObject_
	//		オブジェクトへのポインタ
	//
	//	RETURN
	//	なし
	//
	//	EXCEPTIONS
	//	なし
	//
	ObjectPointer(const X* pObject_)
		: m_pObject(const_cast<X*>(pObject_)), m_fConstant(true)
	{
		addReference();
	}

	//
	//	TEMPLATE FUNCTION public
	//	Common::ObjectPointer<X>::~ObjectPointer -- デストラクタ
	//
	//	TEMPLATE ARGUMENTS
	//	class X
	//		Common::ExecutableObjectの派生クラス
	//
	//	NOTES
	//	デストラクタ
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
	~ObjectPointer()
	{
		release();
	}

	//
	//	TEMPLATE FUNCTION public
	//	Common::ObjectPointer<X>::ObjectPointer -- コピーコンストラクタ
	//
	//	TEMPLATE ARGUMENTS
	//	class X
	//		Common::ExecutableObjectの派生クラス
	//
	//	NOTES
	//	コピーコンストラクタ。
	//	オブジェクトの参照カウンタをインクリメントする。
	//
	//	ARGUMENTS
	//	const Common::ObjectPointer<X>& r_
	//		コピー元のオブジェクトポインタ
	//
	//	RETURN
	//	なし
	//
	//	EXCEPTIONS
	//	なし
	//
	ObjectPointer(const Common::ObjectPointer<X>& r_)
	{
		m_pObject = r_.get();
		m_fConstant = r_.m_fConstant;
		addReference();
	}

	//
	//	TEMPLATE FUNCTION public
	//	Common::ObjectPointer<X>::operator= -- 代入オペレータ
	//
	//	TEMPLATE ARGUMENTS
	//	class X
	//		Common::ExecutableObjectの派生クラス
	//
	//	NOTES
	//	代入オペレータ。
	//	オブジェクトの参照カウンタをインクリメントする。
	//
	//	ARGUMENTS
	//	const Common::ObjectPointer<X>& r_
	//		代入元のオブジェクトポインタ
	//
	//	RETURN
	//	Common::ObjectPointer<X>&
	//		自分自身への参照
	//
	//	EXCEPTIONS
	//	なし
	//

	ObjectPointer& operator =(const Common::ObjectPointer<X>& r_)
	{
		if (get() != r_.get())
		{
			//自身のオブジェクトの参照を解放する
			release();
			//代入元のオブジェクトをメンバへ代入する
			m_pObject = r_.get();
			m_fConstant = r_.m_fConstant;
			addReference();
		}
		return *this;
	}

	ObjectPointer&	operator =(X* r)
	{
		if (get() != r) {
			release();

			m_pObject = r;
			m_fConstant = false;
			addReference();
		}

		return *this;
	}

	ObjectPointer&	operator =(const X* r)
	{
		if (get() != r) {
			release();

			m_pObject = const_cast<X*>(r);
			m_fConstant = true;
			addReference();
		}

		return *this;
	}

	//
	//	TEMPLATE FUNCTION public
	//	Common::ObjectPointer<X>::operator* () -- * 前置演算子
	//
	//	TEMPLATE ARGUMENTS
	//	class X
	//		Common::ExecutableObjectの派生クラス
	//
	//	NOTES
	//	* 前置演算子
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	X&
	//		オブジェクトの実体への参照
	//
	X& operator* () const
	{
		return *get();
	}

	//
	//	TEMPLATE FUNCTION public
	//	Common::ObjectPointer<X>::operator-> () -- -> 演算子
	//
	//	TEMPLATE ARGUMENTS
	//	class X
	//		Common::ExecutableObjectの派生クラス
	//
	//	NOTES
	//	-> 演算子
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	X*
	//		オブジェクトへのポインタ
	//
	//	EXCEPTIONS
	//	なし
	//
	X* operator-> () const
	{
		return get();
	}

	//
	//	TEMPLATE FUNCTION public
	//	Common::ObjectPointer<X>::get -- オブジェクトへのポインタを得る
	//
	//	TEMPLATE ARGUMENTS
	//	class X
	//		Common::ExecutableObjectの派生クラス
	//
	//	NOTES
	//	オブジェクトへのポインタを得る。
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	X*
	//		オブジェクトへのポインタ
	//
	//	EXCEPTIONS
	//	なし
	//
	X* get() const
	{
		return const_cast<X*>(m_pObject);
	}

	//
	//	TEMPLATE FUNCTION public
	//	Common::ObjectPointer<X>::release -- オブジェクトの参照を解放する
	//
	//	TEMPLATE ARGUMENTS
	//	class X
	//		Common::ExecutableObjectの派生クラス
	//
	//	NOTES
	//	オブジェクトの参照を解放する。
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
	void release()
	{
		if (m_pObject && m_fConstant == false)
		{
			if (m_pObject->decrementReferenceCounter() == 0)
				delete m_pObject, m_pObject=0;
//			static_cast<Common::ExecutableObject*>(m_pObject)->release();
		}
	}

	//
	//	TEMPLATE FUNCTION public
	//	Common::ObjectPointer<X>::addReference -- オブジェクトの参照を加える
	//
	//	TEMPLATE ARGUMENTS
	//	class X
	//		Common::ExecutableObjectの派生クラス
	//
	//	NOTES
	//	オブジェクトの参照を加える。
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
	void addReference()
	{
		if (m_pObject && m_fConstant == false)
		{
			m_pObject->incrementReferenceCounter();
//			static_cast<Common::ExecutableObject*>(m_pObject)->addReference();
		}
	}

	//
	//	TEMPLATE FUNCTION public
	//	Common::ObjectPointer<X>::getConstantFlag -- コンスタントフラグを得る
	//
	//	TEMPLATE ARGUMENTS
	//	class X
	//		Common::ExecutableObjectの派生クラス
	//
	//	NOTES
	//	コンスタントフラグを得る。
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	bool
	//		コンスタンスフラグ
	//
	//	EXCEPTIONS
	//	なし
	//
	bool getConstantFlag() const
	{
		return m_fConstant;
	}

	//
	//	TEMPLATE FUNCTION public
	//	Common::ObjectPointer<X>::setPointer -- ポインタを設定する
	//
	//	TEMPLATE ARGUMENTS
	//	class X
	//		Common::ExecutableObjectの派生クラス
	//
	//	NOTES
	//	ポインタを設定する。
	//
	//	ARGUMENTS
	//	X* pPointer_
	//		設定するポインタ
	//	bool fConstant_
	//		コンスタントかどうか(default false)
	//
	//	RETURN
	//	なし
	//
	//	EXCEPTIONS
	//	なし
	//
	void setPointer(X* pPointer_, bool fConstant_ = false)
	{
		//自身のオブジェクトの参照を解放する
		release();
		m_pObject = pPointer_;
		m_fConstant = fConstant_;
		addReference();
	}

private:
	//オブジェクトへのポインタ
	X* m_pObject;
	//与えられたポインタがコンストかどうか
	bool m_fConstant;
};

//
//	TEMPLATE FUNCTION
//	Common::ConvertObjectPointer -- ポインタの型の違うObjectPointerに変換する
//
//	TEMPLATE ARGUMENTS
//	class X
//		変換先のポインタの型
//	class Y
//		変換元のポインタの型
//
//	NOTES
//	ポインタの型の違うObjectPointerに変換する。
//	本来ならクラステンプレートメソッドで、operator=ができればいいが、
//	Solrais だとできないので、しょうがないのでグローバル関数にする。
//
//	ARGUMENTS
//	Common::ObjectPointer<X>& cDist
//		変換先のObjectPointer
//	const Common::ObjectPointer<Y>& cSrc
//		変換元のObjectPointer
//
//	RETURN
//	Common::ObjectPointer<X>&
//		変換したObjectPointer
//
//	EXCEPTIONS
//	なし
//

template<class X, class Y>
ObjectPointer<X>&
ConvertObjectPointer(ObjectPointer<X>& cDist, const ObjectPointer<Y>& cSrc)
{
	// 変換できないと、0 を指すポインタになる

	cDist.setPointer(dynamic_cast<X*>(cSrc.get()), cSrc.getConstantFlag());
	return cDist;
}

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_OBJECTPOINTER_H

//
//	Copyright (c) 1999, 2001, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

