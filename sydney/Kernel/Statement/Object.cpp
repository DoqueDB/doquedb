// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Object.cpp -- 構文要素を表わすスーパークラス
// 
// Copyright (c) 1999, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
	const char moduleName[] = "Statement";
	const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Statement/IntegerValue.h"
#include "Statement/Externalizable.h"
#include "Statement/Object.h"
#include "Statement/Type.h"
#include "Statement/Utility.h"

#include "Common/Assert.h"
#include "Common/InputArchive.h"
#include "Common/OutputArchive.h"
#include "Os/AutoCriticalSection.h"
#include "Exception/NotSupported.h"

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace
{

typedef ModVector<Object*>::ConstIterator ConstIterator;
typedef ModVector<Object*>::Iterator Iterator;

namespace _Object
{
	// 下記を保護するためのラッチ
	Os::CriticalSection		_latch;
	// 初期化した回数
	int						_initialized = 0;
}

}

//
//	FUNCTION public
//		Statement::Object::Object -- コンストラクタ (1)
//
//	NOTES
//		コンストラクタ (1)
//
//	ARGUMENTS
//		ObjectType::Type eType_
//			タイプ識別子
//		Generate eGen_
//			生成識別子
//			Default = Undefine
//		bool
//			ログ書き込みフラグ
//			Default = ture
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外を再送
//
Statement::Object::Object(ObjectType::Type eType_,
						  Statement::Object::Generate eGen_/*=Undefine*/,
						  bool bLogRec_/*=false*/)
	: Common::Object(),
	  m_eType(eType_),
	  m_vecpElements(),
#ifdef OBSOLETE
	  m_eGenerate(eGen_),
#endif
	  m_bLogRec(bLogRec_),
	  m_iHashCode(static_cast<ModSize>(-1))
{
}

//
//	FUNCTION public
//		Statement::Object::Object -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		ObjectType::Type eType_
//		int n_
//		Generate eGen_
//			生成識別子
//			Default = Undefine
//		bool
//			ログ書き込みフラグ
//			Default = ture
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外を再送
//
Statement::Object::Object(ObjectType::Type eType_, int iN_,
						  Statement::Object::Generate eGen_/*=Undefine*/,
						  bool bLogRec_/*=false*/)
	: Common::Object(),
	  m_eType(eType_),
	  m_vecpElements(iN_),
#ifdef OBSOLETE
	  m_eGenerate(eGen_),
#endif
	  m_bLogRec(bLogRec_),
	  m_iHashCode(static_cast<ModSize>(-1))
{
}

//
//	FUNCTION public
//		Statement::Object::getType -- 型を得る
//
//	NOTES
//		型を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ObjectType::Type
//
//	EXCEPTIONS
//		なし
//
ObjectType::Type
Statement::Object::getType() const
{
	return m_eType;
}

#ifdef OBSOLETE // 現在のところStatement::Objectをマップに登録することはない

// FUNCTION public
//	Statement::Statement::Object::hashCode -- ハッシュコードを得る
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	ModSize
//
// EXCEPTIONS

ModSize
Statement::Object::
hashCode() const
{
	return m_iHashCode;
}

// FUNCTION public
//	Statement::Statement::Object::calculateHashCode -- ハッシュコードを計算しておく
//
// NOTES
//	MT-safeではない
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS

ModSize
Statement::Object::
calculateHashCode()
{
	if (m_iHashCode == static_cast<ModSize>(-1))
		m_iHashCode = getHashCode();
	return m_iHashCode;
}

// FUNCTION public
//	Statement::Statement::Object::getHashCode -- ハッシュコードを計算する
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
ModSize
Statement::Object::
getHashCode()
{
	const int shift = sizeof(ModSize) * 8;

	ModSize result = m_eType;
	if (!m_vecpElements.isEmpty()) {
		ModSize g;
		ModVector<Object*>::ConstIterator iterator = m_vecpElements.begin();
		const ModVector<Object*>::ConstIterator& last = m_vecpElements.end();
		do {
			result <<= 4;
			result += (*iterator)->hashCode();
			if (g = result & ((ModSize)0xf << shift)) {
				result ^= g >> (shift - 8);
				result ^= g;
			}
		} while (++iterator != last);
	}
	return result;
}

// FUNCTION public
//	Statement::Statement::Object::operator< -- 大小比較
//
// NOTES
//
// ARGUMENTS
//	const Object& cObj_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Statement::Object::
operator<(const Object& cObj_) const
{
	return getType() < cObj_.getType()
		|| (getType() == cObj_.getType() && compare(cObj_));
}

// 同じ型のオブジェクト同士でless比較する
//virtual
bool
Statement::Object::
compare(const Object& cObj_) const
{
	ModSize n = m_vecpElements.getSize();
	ModSize m = cObj_.m_vecpElements.getSize();
	if (n < m) return true;
	if (n > m) return false;
	if (n > 0) {
		// 途中でこちらのElementsのほうに小さいものがあればtrue
		ModVector<Object*>::ConstIterator iterator0 = m_vecpElements.begin();
		ModVector<Object*>::ConstIterator iterator1 = cObj_.m_vecpElements.begin();
		do {
			if (*(*iterator0) < *(*iterator1)) return true;
			if (*(*iterator1) < *(*iterator0)) return false;
			++iterator0;
			++iterator1;
		} while (--n > 0);
	}
	return false;
}
#endif

//
//	FUNCTION public
//		Statement::Object::addBody -- Objectに要素を追加する
//
//	NOTES
//		Objectに要素を追加する
//
//	ARGUMENTS
//		Statement::Object* pBody_
//		LispParser専用。
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外を再送
//
void Statement::Object::addBody(Statement::Object* pBody_)
{
	m_vecpElements.pushBack(pBody_);
}

//
//	FUNCTION public
//		Statement::Object::clearBody -- Objectを空にする
//
//	NOTES
//		Objectを空にする。
//		LispParser専用。
//
//	ARGUMENTS
//		Statement::Object* pBody_
//			pBody_を追加する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外を再送
//
void Statement::Object::clearBody()
{
	// 要素を持っていないことにする。領域は維持する。
	m_vecpElements.erase(m_vecpElements.begin(), m_vecpElements.end());
}

#ifndef SYD_COVERAGE
//
//	FUNCTION public virtual
//		Statement::Object::getElementVector -- 要素のVectorを返す
//
//	NOTES
//		要素のVectorを返す
//		TreeWalker専用
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModVector<Statement::Object*>*
//			このオブジェクトのm_vecpElementsへのポインタ
//			下位要素がない場合は0を返す
//
//	EXCEPTIONS
//		なし
//
ModVector<Statement::Object*>* Statement::Object::getElementVector()
{
	return &m_vecpElements;
}
#endif

#ifdef OBSOLETE
//	FUNCTION public
//	Statement::Object::getGenerate -- Optimize or Reorganize の識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//	enum
//	Statement::Object::Generate
//		Undefine	定義無し
//		Optimize	Optimize を使用する
//		Reorganize	Reorganize を使用する
//
//	EXCEPTIONS
//		なし

Statement::Object::Generate
Statement::Object::getGenerate() const
{
	return m_eGenerate;
}
#endif

//
//	FUNCTION public
//	Statement::Object::isRecordLog
//		-- ログ追加構文問合せ
//
//	NOTES
//	ログ追加構文問合せ
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		bool
//			true  : ログに追加する構文
//			false : ログに追加しない構文
//
//	EXCEPTIONS
//		なし
//
bool
Statement::Object::isRecordLog() const
{
	return m_bLogRec;
}

//
//	FUNCTION public
//	Statement::Object::Object
//		-- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const Statement::Object& cOther_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Object::Object(const Object& cOther_)
: m_eType(cOther_.m_eType),
#ifdef OBSOLETE
  m_eGenerate(cOther_.m_eGenerate),
#endif
  m_bLogRec(cOther_.m_bLogRec)
{
	int n = 0;
	ConstIterator i = cOther_.m_vecpElements.begin();
	for (; i != cOther_.m_vecpElements.end(); ++i)
	{
		Object* p = 0;
		if (*i != 0)
			p = (*i)->copy();
		m_vecpElements.pushBack(p);
	}
}

//
//	FUNCTION public
//		Statement::Object::~Object -- デストラクタ
//		下位のオブジェクトもまとめて解放する
//		自身も解放する
//
//	NOTES
//		デストラクタ
//		下位オブジェクトは解放したくないとき、
//		delete前にclearBody()を実行するとよい。
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//
Object::~Object()
{
	for (ConstIterator i = m_vecpElements.begin();
		 i != m_vecpElements.end(); ++i) {
		if (*i != 0) {
			delete *i;
		}
	}
}

//
//	FUNCTION public
//		Statement::Object::destruct -- 下位のオブジェクトもまとめて解放する
//
//	NOTES
//		下位のオブジェクトもまとめて解放する
//		自身も解放する
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//
void
Object::destruct()
{
	delete this;
}

//
//	FUNCTION public
//		Statement::Object::toSQLStatement -- SQL文を得る
//
//	NOTES
//		SQL文を得る
//
//	ARGUMENTS
//		bool bForCascade_ = false
//
//	RETURN
//		ModUnicodeString
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ModUnicodeString
Object::toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream cStream;
	cStream << getTypeName(getType());

	const char cDelimiter0 = '(';
	const char cDelimiter1 = ',';
	const char* pDelimiter = &cDelimiter0;

	for (ConstIterator i = m_vecpElements.begin();
		 i != m_vecpElements.end(); ++i) {
		cStream << *pDelimiter;
		pDelimiter = &cDelimiter1;
		if (*i == 0) {
			cStream << "null";
		} else {
			cStream << (*i)->toSQLStatement(bForCascade_);
		}
	}
	cStream << ")";

	return ModUnicodeString(cStream.getString());
}

#ifndef SYD_COVERAGE
//
//	FUNCTION public
//		Statement::Object::toString -- 文字列で値を得る(デバッグ用)
//
//	NOTES
//		文字列で値を得る(デバッグ用)
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModUnicodeString
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ModUnicodeString
Object::toString() const
{
	ModUnicodeOstrStream cStream;
	cStream << "(";
	cStream << getTypeName(getType());

	for (ConstIterator i = m_vecpElements.begin();
		 i != m_vecpElements.end(); ++i) {
		cStream << " ";
		if (*i == 0) {
			cStream << "null";
		} else {
			cStream << (*i)->toString();
		}
	}
	cStream << ")";

	return ModUnicodeString(cStream.getString());
}

//
//	FUNCTION
//		Statement::Object::toString -- LISP形式で出力する
//
//	NOTES
//		LISP形式で出力する
//
//	ARGUMENTS
//		ModUnicodeOstrStream& cStream_
//		int iIndent_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
Object::toString(ModUnicodeOstrStream& cStream_, int iIndent_) const
{
	int i;
	for (i=0; i<iIndent_; ++i)
		cStream_ << ' ';

	cStream_ << '(';
	cStream_ << getTypeName(getType());

	// メンバの出力
	for (ConstIterator im = m_vecpElements.begin();
		 im != m_vecpElements.end(); ++im) {
		cStream_ << '\n';
		if (*im == 0) {
			for (i=0; i<iIndent_+1; ++i)
				cStream_ << ' ';
			cStream_ << "null";
		} else {
			(*im)->toString(cStream_, iIndent_+1);
		}
	}

	cStream_ << ')';
}

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN
//
//	FUNCTION global
//		Statement::toStringDefault -- 文字列で値を得る(デバッグ用)
//
//	NOTES
//		文字列で値を得る(デバッグ用)
//
//	ARGUMENTS
//		const Statement::Object* pObj_
//			文字列化したいオブジェクト
//
//	RETURN
//		ModString
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ModUnicodeString
toStringDefault(const Object* pObj_)
{
	ModUnicodeOstrStream cStream;
	cStream << "(";
	cStream << getTypeName(pObj_->getType());

	for (ConstIterator i = pObj_->m_vecpElements.begin();
		 i != pObj_->m_vecpElements.end(); ++i) {
		cStream << " ";
		if (*i == 0) {
			cStream << "null";
		} else {
			cStream << (*i)->toString();
		}
	}
	cStream << ")";

	return ModUnicodeString(cStream.getString());
}
_SYDNEY_STATEMENT_END
_SYDNEY_END
#endif

//	FUNCTION public
//	Statement::Object::initialize -- 初期化を行う
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

void
Object::initialize()
{
	Os::AutoCriticalSection	latch(_Object::_latch);

	if (_Object::_initialized++ == 0)
		Common::Externalizable::insertFunction(
			Common::Externalizable::StatementClasses,
			Statement::getClassInstance);
}

//	FUNCTION public
//	Statement::Object::terminate -- 終了処理を行う
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

void
Object::terminate()
{
	Os::AutoCriticalSection	latch(_Object::_latch);

	--_Object::_initialized;
}

//
//	FUNCTION public
//		Statement::Object::equals -- パラメータObjectと自分の比較結果を返す
//
//	NOTES
//		パラメータObjectと自分の比較結果を返す
//
//	ARGUMENTS
//		const Statement::Object* pObj_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外を再送
//
bool
Object::equals(const Object* pObj_) const
{
	if (pObj_ == 0) return false;
	if (m_eType != pObj_->getType())
	{
		return false;
	}
	const ModVector<Object*>& vecpWork = pObj_->m_vecpElements;
	int iCount = 0;
	if ((iCount = m_vecpElements.getSize()) != static_cast<int>(vecpWork.getSize()))
	{
		return false;
	}
	for (int iLoopCnt = 0; iLoopCnt < iCount; iLoopCnt++)
	{
		if (m_vecpElements.at(iLoopCnt) == vecpWork.at(iLoopCnt))
		{
			continue;
		}
		if (!m_vecpElements.at(iLoopCnt) ||
			!m_vecpElements.at(iLoopCnt)->equals(vecpWork.at(iLoopCnt)))
		{
			return false;
		}
	}
	return true;
}

//
//	FUNCTION public
//	Statement::Object::convertToSignedInteger
//		-- UnsignedIntegerのLiteralをIntegerLiteralに変換する
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
void
Object::convertToSignedInteger()
{
	for (Iterator i = m_vecpElements.begin(); i != m_vecpElements.end(); ++i)
	{
		if (*i)
			(*i)->convertToSignedInteger();
	}
}

//
//	FUNCTION public
//	Statement::Object::expandCondition
//		-- And, Or を展開する
//
//	NOTES
//
//	ARGUMENTS
//	int iDepth_ (default 0)
//		ORノードの数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Object::expandCondition(int iDepth_)
{
	for (Iterator i = m_vecpElements.begin(); i != m_vecpElements.end(); ++i)
	{
		if (*i)
			(*i)->expandCondition(iDepth_);
	}
}

// FUNCTION public
//	Statement::Object::setType -- 型を設定する
//
// NOTES
//
// ARGUMENTS
//	ObjectType::Type eType_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Object::
setType(ObjectType::Type eType_)
{
	m_eType = eType_;
}

//	FUNCTION protected
//	Statement::Object::getElement -- メンバーを得る
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		i
//			取得するメンバーを識別する値
//		Statement::ObjectType::Type	type
//			取得するメンバーの型
//
//	RETURN
//		得られたメンバーを格納する領域の先頭アドレス
//
//	EXCEPTIONS

Object*
Object::getElement(unsigned int i, ObjectType::Type type) const
{
	Object* p = m_vecpElements[i];
	return (p && type == p->getType()) ? p : 0;
}

//	FUNCTION protected
//	Statement::Object::setElement -- メンバーを設定する
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		i
//			設定するメンバーを識別する値
//		Statement::Object*	p
//			設定するメンバーを格納する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Object::setElement(unsigned int i, Object* p)
{
	m_vecpElements[i] = p;
}

//	FUNCTION protected
//	Statement::Object::getIntegerElement -- 整数型メンバーを得る
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		i
//			取得するメンバーを識別する値
//
//	RETURN
//		得られた整数値
//
//	EXCEPTIONS

int
Object::getIntegerElement(unsigned int i) const
{
	IntegerValue* p = _SYDNEY_DYNAMIC_CAST(
		IntegerValue*, getElement(i, ObjectType::IntegerValue));
	return (p) ? p->getValue() : 0;
}

//	FUNCTION protected
//	Statement::Object::setIntegerElement -- 整数型メンバーを設定する
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		i
//			設定するメンバーを識別する値
//		int					v
//			設定する整数値
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Object::setIntegerElement(unsigned int i, int v)
{
	IntegerValue* p = _SYDNEY_DYNAMIC_CAST(
		IntegerValue*, getElement(i, ObjectType::IntegerValue));
	if (p)
		p->setValue(v);
	else {
		p = new IntegerValue(v);
		; _SYDNEY_ASSERT(p);
		setElement(i, p);
	}
}

#ifdef USE_OLDER_VERSION
// FUNCTION public
//	Statement::Object::getAnalyzer -- Analyzerを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Analysis::Analyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Analyzer*
Object::getAnalyzer() const
{
	// デフォルトは0を返す
	return 0;
}
#endif

// FUNCTION public
//	Statement::Object::getAnalyzer2 -- Analyzerを得る
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	const Analysis::Interface::IAnalyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Interface::IAnalyzer*
Object::
getAnalyzer2() const
{
	// デフォルトはNotSupported0を返す
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Statement::Object::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Object::
getClassID() const
{
	return Statement::Externalizable::ClassID::Base + getType();
}

// FUNCTION public
//	Statement::Object::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Object::
serialize(ModArchive& archiver_)
{
	Utility::Serialize::EnumValue(archiver_, m_eType);
	Utility::Serialize::ObjectVector(archiver_, m_vecpElements);
	archiver_(m_bLogRec);
}

//
//	Copyright (c) 1999, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
