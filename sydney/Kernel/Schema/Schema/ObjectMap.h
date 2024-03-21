// -*-Mode: C++; tab-width: 4; c-basic-offmap: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectMap.h -- スキーマオブジェクトのマップを表すクラス定義、関数宣言
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2023, 2024 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_OBJECT_MAP_H
#define	__SYDNEY_SCHEMA_OBJECT_MAP_H

#include "SyDynamicCast.h"
#include "Schema/Object.h"

#include "Common/Assert.h"
#include "Common/Object.h"

#include "Schema/AutoRWLock.h"
#include "Schema/Database.h"
#include "Schema/Utility.h"

#include "ModHashMap.h"

_SYDNEY_BEGIN

namespace Trans
{
	class Transaction;
}

_SYDNEY_SCHEMA_BEGIN

namespace Utility
{
	class InputArchive;
	class OutputArchive;
}

class Database;

//	TEMPLATE CLASS
//	Schema::ApplyFunction
//
//	NOTES
//		ObjectMapでIterationしながら適用する関数のテンプレート

template <class _Object_>
class ApplyFunction
{
public:
	virtual void apply(_Object_* pObject_) const = 0;
};

template <class _Object_>
class ApplyFunction0
	: public ApplyFunction<_Object_>
{
public:
	typedef void (_Object_::* Method)();

	ApplyFunction0(Method method)
		: m_method(method)
	{ }

	void apply(_Object_* pObject_) const
	{
		(pObject_->*m_method)();
	}
private:
	Method m_method;
};

template <class _Object_, class _Arg1_>
class ApplyFunction1
	: public ApplyFunction<_Object_>
{
public:
	typedef void (_Object_::* Method)(_Arg1_ a1);

	ApplyFunction1(Method method, _Arg1_ a1)
		: m_method(method), m_a1(a1)
	{ }

	void apply(_Object_* pObject_) const
	{
		(pObject_->*m_method)(m_a1);
	}
private:
	Method m_method;
	_Arg1_ m_a1;
};

template <class _Object_, class _Arg1_, class _Arg2_>
class ApplyFunction2
	: public ApplyFunction<_Object_>
{
public:
	typedef void (_Object_::* Method)(_Arg1_ a1, _Arg2_ a2);

	ApplyFunction2(Method method, _Arg1_ a1, _Arg2_ a2)
		: m_method(method), m_a1(a1), m_a2(a2)
	{ }

	void apply(_Object_* pObject_) const
	{
		(pObject_->*m_method)(m_a1, m_a2);
	}
private:
	Method m_method;
	_Arg1_ m_a1;
	_Arg2_ m_a2;
};

template <class _Object_, class _Arg1_, class _Arg2_, class _Arg3_>
class ApplyFunction3
	: public ApplyFunction<_Object_>
{
public:
	typedef void (_Object_::* Method)(_Arg1_ a1, _Arg2_ a2, _Arg3_ a3);

	ApplyFunction3(Method method, _Arg1_ a1, _Arg2_ a2, _Arg3_ a3)
		: m_method(method), m_a1(a1), m_a2(a2), m_a3(a3)
	{ }

	void apply(_Object_* pObject_) const
	{
		(pObject_->*m_method)(m_a1, m_a2, m_a3);
	}
private:
	Method m_method;
	_Arg1_ m_a1;
	_Arg2_ m_a2;
	_Arg3_ m_a3;
};

template <class _Object_, class _Arg1_, class _Arg2_, class _Arg3_, class _Arg4_>
class ApplyFunction4
	: public ApplyFunction<_Object_>
{
public:
	typedef void (_Object_::* Method)(_Arg1_ a1, _Arg2_ a2, _Arg3_ a3, _Arg4_ a4);

	ApplyFunction4(Method method, _Arg1_ a1, _Arg2_ a2, _Arg3_ a3, _Arg4_ a4)
		: m_method(method), m_a1(a1), m_a2(a2), m_a3(a3), m_a4(a4)
	{ }

	void apply(_Object_* pObject_) const
	{
		(pObject_->*m_method)(m_a1, m_a2, m_a3, m_a4);
	}
private:
	Method m_method;
	_Arg1_ m_a1;
	_Arg2_ m_a2;
	_Arg3_ m_a3;
	_Arg4_ m_a4;
};

//	TEMPLATE CLASS
//	Schema::BoolFunction
//
//	NOTES
//		ObjectMapでIterationするときに除外するオブジェクトを判定する関数のテンプレート

template <class _Object_>
class BoolFunction
{
public:
	virtual bool predicate(_Object_* pObject_) const = 0;
};

template <class _Object_>
class BoolFunction0
	: public BoolFunction<_Object_>
{
public:
	typedef bool (*Func)(_Object_* pObject_);

	BoolFunction0(Func func)
		: m_func(func)
	{ }

	bool predicate(_Object_* pObject_) const
	{
		return (*m_func)(pObject_);
	}
private:
	Func m_func;
};

template <class _Object_>
class BoolMemberFunction0
	: public BoolFunction<_Object_>
{
public:
	typedef bool (_Object_::*MemberFunction)() const;

	BoolMemberFunction0(MemberFunction func)
		: m_func(func)
	{ }

	bool predicate(_Object_* pObject_) const
	{
		return (pObject_->*m_func)();
	}
private:
	MemberFunction m_func;
};

template <class _Object_, class _Arg1_>
class BoolFunction1
	: public BoolFunction<_Object_>
{
public:
	typedef bool (*Func)(_Object_* pObject_, _Arg1_ a1);

	BoolFunction1(Func func, _Arg1_ a1)
		: m_func(func), m_a1(a1)
	{ }

	bool predicate(_Object_* pObject_) const
	{
		return (*m_func)(pObject_, m_a1);
	}
private:
	Func m_func;
	_Arg1_ m_a1;
};

template <class _Object_, class _Arg1_>
class BoolMemberFunction1
	: public BoolFunction<_Object_>
{
public:
	typedef bool (_Object_::*MemberFunction)(_Arg1_ a1) const;

	BoolMemberFunction1(MemberFunction func, _Arg1_ a1)
		: m_func(func), m_a1(a1)
	{ }

	bool predicate(_Object_* pObject_) const
	{
		return (pObject_->*m_func)(m_a1);
	}
private:
	MemberFunction m_func;
	_Arg1_ m_a1;
};

template <class _Object_, class _Arg1_, class _Arg2_>
class BoolFunction2
	: public BoolFunction<_Object_>
{
public:
	typedef bool (*Func)(_Object_* pObject_, _Arg1_ a1, _Arg2_ a2);

	BoolFunction2(Func func, _Arg1_ a1, _Arg2_ a2)
		: m_func(func), m_a1(a1), m_a2(a2)
	{ }

	bool predicate(_Object_* pObject_) const
	{
		return (*m_func)(pObject_, m_a1, m_a2);
	}
private:
	Func m_func;
	_Arg1_ m_a1;
	_Arg2_ m_a2;
};

template <class _Object_, class _Arg1_, class _Arg2_>
class BoolMemberFunction2
	: public BoolFunction<_Object_>
{
public:
	typedef bool (_Object_::*MemberFunction)(_Arg1_ a1, _Arg2_ a2) const;

	BoolMemberFunction2(MemberFunction func, _Arg1_ a1, _Arg2_ a2)
		: m_func(func), m_a1(a1), m_a2(a2)
	{ }

	bool predicate(_Object_* pObject_) const
	{
		return (pObject_->*m_func)(m_a1, m_a2);
	}
private:
	MemberFunction m_func;
	_Arg1_ m_a1;
	_Arg2_ m_a2;
};

template <class _Object_, class _Arg1_, class _Arg2_, class _Arg3_>
class BoolFunction3
	: public BoolFunction<_Object_>
{
public:
	typedef bool (*Func)(_Object_* pObject_, _Arg1_ a1, _Arg2_ a2, _Arg3_ a3);

	BoolFunction3(Func func, _Arg1_ a1, _Arg2_ a2, _Arg3_ a3)
		: m_func(func), m_a1(a1), m_a2(a2), m_a3(a3)
	{ }

	bool predicate(_Object_* pObject_) const
	{
		return (*m_func)(pObject_, m_a1, m_a2, m_a3);
	}
private:
	Func m_func;
	_Arg1_ m_a1;
	_Arg2_ m_a2;
	_Arg3_ m_a3;
};

template <class _Object_, class _Arg1_, class _Arg2_, class _Arg3_>
class BoolMemberFunction3
	: public BoolFunction<_Object_>
{
public:
	typedef bool (_Object_::*MemberFunction)(_Arg1_ a1, _Arg2_ a2, _Arg3_ a3) const;

	BoolMemberFunction3(MemberFunction func, _Arg1_ a1, _Arg2_ a2, _Arg3_ a3)
		: m_func(func), m_a1(a1), m_a2(a2), m_a3(a3)
	{ }

	bool predicate(_Object_* pObject_) const
	{
		return (pObject_->*m_func)(m_a1, m_a2, m_a3);
	}
private:
	MemberFunction m_func;
	_Arg1_ m_a1;
	_Arg2_ m_a2;
	_Arg3_ m_a3;
};

// デフォルトのBoolFunction
namespace _Bool
{
	// 常にFALSEを返す関数
	template <class _Object_>
	inline bool
	_False(_Object_* pObject_)
	{
		return false;
	}

	// 削除されたオブジェクトを除外するための関数
	template <class _Object_>
	inline bool
	_Deleted(_Object_* pObject_)
	{
		return pObject_->getStatus() == Object::Status::Deleted
			|| pObject_->getStatus() == Object::Status::DeletedInRecovery;
	}

	// Findによく使う関数
	template <class _Object_>
	inline bool
	_findByName(_Object_* pObject_, const Object::Name& cName_)
	{
		return pObject_->getName() == cName_;
	}
}

// Viewのソートに使う関数
namespace _Sorter
{
	class _Object
	{
	public:
		ModBoolean operator()(Object* l, Object* r)
		{
			return l->isLessThan(*r) ? ModTrue : ModFalse;
		}
	};
}

//	TEMPLATE CLASS
//	Schema::ObjectMap -- スキーマオブジェクトのマップを表すクラス
//
//	NOTES

template <class _Object_, class _Pointer_>
class ObjectMap
	: public ModHashMap<Object::ID::Value, _Pointer_, ModHasher<Object::ID::Value> >
{
public:
	typedef ModHashMap<Object::ID::Value, _Pointer_, ModHasher<Object::ID::Value> > Map;
	typedef ModVector<_Object_*> Vector;

	// コンストラクター
	ObjectMap(ModSize iSize_, ModBoolean bEnableLink_, bool bUseView_, bool bUseCache_ = true);
	// デストラクター
	~ObjectMap();

	// 中身を初期化する
	void reset(Database& cDatabase_);
	void reset();
	// マップの変更操作
	void insert(const _Pointer_& pPointer_);
	void insert(const ObjectMap<_Object_, _Pointer_>& cMap_);
	void erase(Object::ID::Value iID_);
	void erase(Database& cDatabase_, Object::ID::Value iID_);

	// 中身をserializeする
	void writeObject(Utility::OutputArchive& cArchiver_);
	void readObject(Utility::InputArchive& cArchiver_, ModSize iSize_);

	// IDを指定してオブジェクトを得る
	_Pointer_ get(Object::ID::Value iID_) const;

	// Sorterの順に並んだVectorを得る
	Vector& getView(Os::RWLock& lock_) const;
	Vector& getView(Os::RWLock& lock_, const BoolFunction<_Object_>& funcOmit_) const;

	// Viewのキャッシュ情報を開放する
	void clearView();

	//////////////////////////////////
	// マップの中身に対する種々の操作

	void apply(const ApplyFunction<_Object_>& funcApply_) const;
	void apply(const ApplyFunction<_Object_>& funcApply_, const BoolFunction<_Object_>& funcOmit_) const;
	void apply(const ApplyFunction<_Object_>& funcApply_, const ApplyFunction<_Object_>& funcRecovery_, bool bUndo_ = false) const;

	// 条件に合致するオブジェクトを探す
	_Object_* find(const BoolFunction<_Object_>& funcFind_) const;
	_Object_* find(const BoolFunction<_Object_>& funcFind_, const BoolFunction<_Object_>& funcOmit_) const;
	typename Map::ConstIterator find(Object::ID::Value iKey_) const {return Map::find(iKey_);}
	typename Map::Iterator find(Object::ID::Value iKey_) {return Map::find(iKey_);}
										// using Map::findがなぜかVC6では利かない
	void extract(Vector& vecResult_, const BoolFunction<_Object_>& funcExtract_) const;

  using Map::getSize;
  using Map::begin;
  using Map::end;
  using Map::isEmpty;
  using Map::getFront;
  using Map::popFront;
  using Map::clear;
  using Map::getValue;

protected:
	// Vectorで保持するデータの変更操作
	void resetView() const;
	void insertView(_Object_* pObject_) const;
	void eraseView(_Object_* pObject_) const;

private:
	// デストラクターの下位関数
	void destruct();

	bool m_bUseView;
	bool m_bUseCache;
	mutable Vector* m_pVector;
};

//	TEMPLATE FUNCTION
//	Schema::ObjectMap::ObjectMap -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		ModSize iSize_
//			ハッシュ表のサイズ
//		ModBoolean bEnableLink_
//			ModTrueのときIteration用のデータ構造を使う
//		bool bUseView_
//			trueのときVectorにも並べる
//		bool bUseCache_
//			trueのときDatabaseのCacheに入れる
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class _Object_, class _Pointer_>
inline
ObjectMap<_Object_, _Pointer_>::
ObjectMap(ModSize iSize_, ModBoolean bEnableLink_, bool bUseView_, bool bUseCache_ /* = true */)
	: Map(iSize_, bEnableLink_), m_pVector(0), m_bUseView(bUseView_), m_bUseCache(bUseCache_)
{
	resetView();
}

//	TEMPLATE FUNCTION
//	Schema::ObjectMap::~ObjectMap -- デストラクター
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

template <class _Object_, class _Pointer_>
inline
ObjectMap<_Object_, _Pointer_>::
~ObjectMap()
{
	destruct();
}

//	TEMPLATE FUNCTION
//	Schema::ObjectMap::reset -- 中身を初期化する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class _Object_, class _Pointer_>
void
ObjectMap<_Object_, _Pointer_>::
reset(Database& cDatabase_)
{
	while (!isEmpty()) {
		const _Pointer_& pObject = getFront();
		// 下位オブジェクトがあればそれを抹消する
		pObject->reset(cDatabase_);
		// キャッシュから消す
		if (m_bUseCache)
			cDatabase_.eraseCache(pObject->getID());
		// マップから消す
		popFront();
	}
	resetView();
}

//	TEMPLATE FUNCTION
//	Schema::ObjectMap::reset -- 中身を初期化する
//
//	NOTES
//		中身を見ないバージョン
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class _Object_, class _Pointer_>
void
ObjectMap<_Object_, _Pointer_>::
reset()
{
	Map::erase(begin(), end());
	resetView();
	; _SYDNEY_ASSERT(getSize() == 0);
	; _SYDNEY_ASSERT(!m_pVector || m_pVector->getSize() == 0);
}

//	TEMPLATE FUNCTION
//	Schema::ObjectMap::insert -- マップに挿入する
//
//	NOTES
//
//	ARGUMENTS
//		const _Pointer_& pPointer_
//			挿入するオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class _Object_, class _Pointer_>
void
ObjectMap<_Object_, _Pointer_>::
insert(const _Pointer_& pPointer_)
{
	if (Map::insert(pPointer_->getID(), pPointer_).second == ModTrue) {
		// 新規挿入だったときのみVectorの処理をする
		if (m_bUseView)
			insertView(pPointer_.get());
		else
			resetView();
	}
}

template <class _Object_, class _Pointer_>
void
ObjectMap<_Object_, _Pointer_>::
insert(const ObjectMap<_Object_, _Pointer_>& cMap_)
{
	typename Map::ConstIterator iterator = cMap_.begin();
	const typename Map::ConstIterator end = cMap_.end();
	for (; iterator != end; ++iterator)
		insert(*iterator);
}

//	TEMPLATE FUNCTION
//	Schema::ObjectMap::erase -- マップから消去する
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Object::ID::Value iID_
//			消去するオブジェクトのID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class _Object_, class _Pointer_>
void
ObjectMap<_Object_, _Pointer_>::
erase(Object::ID::Value iID_)
{
	typename Map::Iterator iterator = Map::find(iID_);

	if (iterator != end()) {
		const _Pointer_& pObject = getValue(iterator);
		eraseView(pObject.get());

		Map::erase(iterator);
	}
}

// eraseの前にresetを呼ぶ
template <class _Object_, class _Pointer_>
void
ObjectMap<_Object_, _Pointer_>::
erase(Database& cDatabase_, Object::ID::Value iID_)
{
	typename Map::Iterator iterator = Map::find(iID_);

	if (iterator != end()) {
		const _Pointer_& pObject = getValue(iterator);
		pObject->reset(cDatabase_);
		if (m_bUseCache)
			cDatabase_.eraseCache(iID_);

		eraseView(pObject.get());

		Map::erase(iterator);
	}
}

//	TEMPLATE FUNCTION
//	Schema::ObjectMap::writeObject -- マップをアーカイブする
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Utility::OutputArchive& cArchiver_
//			アーカイバー
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class _Object_, class _Pointer_>
void
ObjectMap<_Object_, _Pointer_>::
writeObject(Utility::OutputArchive& cArchiver_)
{
	if (getSize()) {
		if (m_pVector && !m_pVector->isEmpty()) {
			typename ModVector<_Object_*>::Iterator iterator = m_pVector->begin();
			const typename ModVector<_Object_*>::Iterator& end = m_pVector->end();
			for (; iterator != end; ++iterator)
				cArchiver_.writeObject(*iterator);

		} else {
			typename Map::Iterator iterator = begin();
			for (; iterator != end(); ++iterator)
				cArchiver_.writeObject(getValue(iterator).get());
		}
	}
}

//	TEMPLATE FUNCTION
//	Schema::ObjectMap::readObject -- マップをアーカイブから読み込む
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Utitily::InputArchive& cArchiver_
//			アーカイバー
//		ModSize iSize_
//			読み込むオブジェクト数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class _Object_, class _Pointer_>
void
ObjectMap<_Object_, _Pointer_>::
readObject(Utility::InputArchive& cArchiver_, ModSize iSize_)
{
	if (iSize_) {
		reset();
		for (ModSize i = 0; i < iSize_; ++i) {
			_Pointer_ pObject =
				_SYDNEY_DYNAMIC_CAST(_Object_*, cArchiver_.readObject());
			insert(pObject);

			if (m_bUseCache && cArchiver_.getDatabase())
				cArchiver_.getDatabase()->addCache(pObject);
		}
	}
}

//	TEMPLATE FUNCTION
//	Schema::ObjectMap::apply -- マップの中身に対して指定された操作を行う
//
//	NOTES
//
//	ARGUMENTS
//		Schema::ApplyFunction funcApply_
//			適用するメソッド
//		Schema::BoolFunction funcOmit_
//			指定されたときイテレーションから除外するオブジェクトを示す
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class _Object_, class _Pointer_>
void
ObjectMap<_Object_, _Pointer_>::
apply(const ApplyFunction<_Object_>& funcApply_) const
{
	if (getSize()) {
		if (m_pVector && !m_pVector->isEmpty()) {
			typename ModVector<_Object_*>::ConstIterator iterator = m_pVector->begin();
			const typename ModVector<_Object_*>::ConstIterator& end = m_pVector->end();

			// メソッドによってはdeleteされるかもしれないのでwhileで回す
			while (iterator != end) 
				funcApply_.apply(*iterator++);

		} else {
			typename Map::ConstIterator iterator = begin();

			// メソッドによってはdeleteされるかもしれないのでwhileで回す
			while (iterator != end())
				funcApply_.apply(Map::getValue(iterator++).get());
		}
	}
}

template <class _Object_, class _Pointer_>
void
ObjectMap<_Object_, _Pointer_>::
apply(const ApplyFunction<_Object_>& funcApply_, const BoolFunction<_Object_>& funcOmit_) const
{
	if (getSize()) {
		if (m_pVector && !m_pVector->isEmpty()) {
			typename ModVector<_Object_*>::ConstIterator iterator = m_pVector->begin();
			const typename ModVector<_Object_*>::ConstIterator& end = m_pVector->end();

			// メソッドによってはdeleteされるかもしれないのでwhileで回す
			while (iterator != end) {
				_Object_* pObject = *iterator++;
				if (!funcOmit_.predicate(pObject))
					funcApply_.apply(pObject);
			}

		} else {
			typename Map::ConstIterator iterator = begin();

			// メソッドによってはdeleteされるかもしれないのでwhileで回す
			while (iterator != end()) {
				_Object_* pObject = Map::getValue(iterator++).get();
				if (!funcOmit_.predicate(pObject))
					funcApply_.apply(pObject);
			}
		}
	}
}

//	TEMPLATE FUNCTION
//	Schema::ObjectMap::apply -- マップの中身に対して指定された操作を行う
//
//	NOTES
//
//	ARGUMENTS
//		Schema::ApplyFunction funcApply_
//			適用するメソッド
//		Schema::ApplyFunction funcRecovery_
//			エラー発生時に実行するメソッド
//		bool bUndo_ = false
//			trueのときUNDO中なので重ねてエラー処理しない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class _Object_, class _Pointer_>
void
ObjectMap<_Object_, _Pointer_>::
apply(const ApplyFunction<_Object_>& funcApply_, const ApplyFunction<_Object_>& funcRecovery_, bool bUndo_ /* = false */) const
{
	if (getSize()) {
		if (m_pVector && !m_pVector->isEmpty()) {
			typename ModVector<_Object_*>::ConstIterator iterator = m_pVector->begin();
			const typename ModVector<_Object_*>::ConstIterator& end = m_pVector->end();
			typename ModVector<_Object_*>::ConstIterator last = iterator;

			try {
				// メソッドによってはdeleteされるかもしれないのでwhileで回す
				while (iterator != end) {
					funcApply_.apply(*iterator++);
					last = iterator;
				}
			} catch (...) {

				// エラー処理
				if (!bUndo_) {
					iterator = m_pVector->begin();
					while (iterator != last)
						funcRecovery_.apply(*iterator++);
				}

				throw;
			}

		} else {
			typename Map::ConstIterator iterator = begin();
			typename Map::ConstIterator last = iterator;

			try {
				// メソッドによってはdeleteされるかもしれないのでwhileで回す
				while (iterator != end()) {
					funcApply_.apply(Map::getValue(iterator++).get());
					last = iterator;
				}
			} catch (...) {

				// エラー処理
				if (!bUndo_) {
					iterator = begin();
					while (iterator != last)
						funcRecovery_.apply(Map::getValue(iterator++).get());
				}

				throw;
			}
		}
	}
}

//	TEMPLATE FUNCTION
//	Schema::ObjectMap::find -- マップの中身から条件に合うオブジェクトを得る
//
//	NOTES
//		条件に合うもののうち最初にヒットしたものを得る
//
//	ARGUMENTS
//		Schema::BoolFunction funcFind_
//			取得するオブジェクトの条件を指定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class _Object_, class _Pointer_>
inline
_Object_*
ObjectMap<_Object_, _Pointer_>::
find(const BoolFunction<_Object_>& funcFind_) const
{
	typename BoolFunction0<_Object_>::Func func = _Bool::_False<_Object_>;
	return find(funcFind_, BoolFunction0<_Object_>(func));
}

template <class _Object_, class _Pointer_>
_Object_*
ObjectMap<_Object_, _Pointer_>::
find(const BoolFunction<_Object_>& funcFind_, const BoolFunction<_Object_>& funcOmit_) const
{
	if (getSize()) {
		if (m_pVector && !m_pVector->isEmpty()) {
			typename ModVector<_Object_*>::ConstIterator iterator = m_pVector->begin();
			const typename ModVector<_Object_*>::ConstIterator& end = m_pVector->end();

			for (; iterator != end; ++iterator) {
				_Object_* pObject = *iterator;
				if (!funcOmit_.predicate(pObject) && funcFind_.predicate(pObject))
					return pObject;
			}

		} else {
			typename Map::ConstIterator iterator = begin();

			for (; iterator != end(); ++iterator) {
				_Object_* pObject = Map::getValue(iterator).get();
				if (!funcOmit_.predicate(pObject) && funcFind_.predicate(pObject))
					return pObject;
			}
		}
	}
	return 0;
}

//	TEMPLATE FUNCTION
//	Schema::ObjectMap::extract -- マップの中身から条件に合うオブジェクトを得る
//
//	NOTES
//		条件に合うものをすべて得る
//
//	ARGUMENTS
//		Schema::ObjectMap::Vector& vecResult_
//			返り値を入れる
//		Schema::BoolFunction funcExtract_
//			取得するオブジェクトの条件を指定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class _Object_, class _Pointer_>
void
ObjectMap<_Object_, _Pointer_>::
extract(Vector& vecResult_, const BoolFunction<_Object_>& funcExtract_) const
{
	if (getSize()) {
		if (m_pVector && !m_pVector->isEmpty()) {
			typename ModVector<_Object_*>::ConstIterator iterator = m_pVector->begin();
			const typename ModVector<_Object_*>::ConstIterator& end = m_pVector->end();

			for (; iterator != end; ++iterator)
				if (funcExtract_.predicate(*iterator))
					vecResult_.pushBack(*iterator);

		} else {
			typename Map::ConstIterator iterator = begin();

			for (; iterator != end(); ++iterator)
				if (funcExtract_.predicate(Map::getValue(iterator).get()))
					vecResult_.pushBack(Map::getValue(iterator).get());
		}
	}
}

//	TEMPLATE FUNCTION
//	Schema::ObjectMap::get -- IDを指定してオブジェクトを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value iID_
//			取得するオブジェクトのID
//
//	RETURN
//		_Pointer_
//			IDが一致するオブジェクト
//
//	EXCEPTIONS

template <class _Object_, class _Pointer_>
_Pointer_
ObjectMap<_Object_, _Pointer_>::
get(Object::ID::Value iID_) const
{
	typename Map::ConstIterator iterator = Map::find(iID_);
	if (iterator != end())
		return Map::getValue(iterator);

	return _Pointer_();
}

//	TEMPLATE FUNCTION
//	Schema::ObjectMap::getView -- Vectorを得る
//
//	NOTES
//
//	ARGUMENTS
//		Os::RWLock& lock_
//			排他制御に使うロック
//		Schema::BoolFunction
//			Viewに含めないオブジェクトの条件
//			指定されない場合はすべてのオブジェクトを含める
//
//	RETURN
//		Schema::ObjectMap::Vector&
//			Sorterの順に並んだVector
//
//	EXCEPTIONS

template <class _Object_, class _Pointer_>
inline
typename ObjectMap<_Object_, _Pointer_>::Vector&
ObjectMap<_Object_, _Pointer_>::
getView(Os::RWLock& lock_) const
{
	// 引数に関数が与えられない場合はOmit条件が常にfalseになる
	typename BoolFunction0<_Object_>::Func func = _Bool::_False<_Object_>;
	return getView(lock_, BoolFunction0<_Object_>(func));
}

template <class _Object_, class _Pointer_>
typename ObjectMap<_Object_, _Pointer_>::Vector&
ObjectMap<_Object_, _Pointer_>::
getView(Os::RWLock& lock_, const BoolFunction<_Object_>& funcOmit_) const
{
	AutoRWLock l(lock_);

	if (!m_bUseView && (!m_pVector || m_pVector->isEmpty())) {

		// Read -> Write ロックへ変換する

		l.convert(Os::RWLock::Mode::Write);

		// ロックの変換後、別のスレッドにより先に
		// 取得されているかもしれないので、サイドチェックする

		if (!m_pVector)
			m_pVector = new Vector;

		if (m_pVector->isEmpty()) {
			m_pVector->reserve(getSize());

			typename Map::ConstIterator iterator = begin();
			for (; iterator != end(); ++iterator) {
				_Object_* pObject = Map::getValue(iterator).get();
				if (!funcOmit_.predicate(pObject)) {
					insertView(pObject);
				}
			}
		}
	}

	return *m_pVector;
}

// Viewのキャッシュ情報を開放する
template <class _Object_, class _Pointer_>
void
ObjectMap<_Object_, _Pointer_>::
clearView()
{
	if (m_pVector)
		m_pVector->clear();
}

//	TEMPLATE FUNCTION
//	Schema::ObjectMap::destruct -- メンバーをすべて破棄する
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

template <class _Object_, class _Pointer_>
inline
void
ObjectMap<_Object_, _Pointer_>::
destruct()
{
	clear();
	delete m_pVector, m_pVector = 0;
}

//	TEMPLATE FUNCTION
//	Schema::ObjectMap::resetView -- Vectorを初期化する
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

template <class _Object_, class _Pointer_>
void
ObjectMap<_Object_, _Pointer_>::
resetView() const
{
	if (!m_pVector) {
		if (m_bUseView)
			m_pVector = new Vector;
	} else
		m_pVector->clear();

	if (m_pVector)
		m_pVector->reserve(getSize());
}

//	TEMPLATE FUNCTION
//	Schema::ObjectMap::insertView -- Vectorに追加する
//
//	NOTES
//
//	ARGUMENTS
//		_Object_* pObject_
//			追加するオブジェクトのポインター
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class _Object_, class _Pointer_>
void
ObjectMap<_Object_, _Pointer_>::
insertView(_Object_* pObject_) const
{
	if (!m_pVector)
		resetView();

	_Sorter::_Object sorter;
	typename Vector::Iterator iterator =
		ModLowerBound(m_pVector->begin(), m_pVector->end(),
					  pObject_, sorter);
	m_pVector->insert(iterator, pObject_);
}

//	TEMPLATE FUNCTION
//	Schema::ObjectMap::eraseView -- Vectorから削除する
//
//	NOTES
//
//	ARGUMENTS
//		_Object_* pObject_
//			追加するオブジェクトのポインター
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class _Object_, class _Pointer_>
void
ObjectMap<_Object_, _Pointer_>::
eraseView(_Object_* pObject_) const
{
	if (m_pVector) {
		typename Vector::Iterator iterator = m_pVector->find(pObject_);
		if (iterator != m_pVector->end())
			m_pVector->erase(iterator);
	}
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif // __SYDNEY_SCHEMA_OBJECT_MAP_H

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2023, 2024 Ricoh Company, Ltd.
//	All rights reserved.
//
