// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Object.h -- 構文要素を表わすスーパークラス
// 
// Copyright (c) 1999, 2002, 2003, 2004, 2005, 2006, 2008, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_OBJECT_H
#define __SYDNEY_STATEMENT_OBJECT_H

#include "Statement/Module.h"
#include "Statement/Type.h"

#include "Common/Externalizable.h"
#include "Common/Object.h"

#include "ModVector.h"
#include "ModUnicodeString.h"

class ModUnicodeOstrStream;

_SYDNEY_BEGIN

namespace Common
{
	class Externalizable;
}
namespace Analysis
{
	class Analyzer;
	namespace Interface
	{
		class IAnalyzer;
	}
}

_SYDNEY_STATEMENT_BEGIN

//
//	CLASS
//		Object -- 構文要素を表わすスーパークラス
//
//	NOTES
//		構文要素を表わすスーパークラス
//
class SYD_STATEMENT_FUNCTION Object
	: public	Common::Object,
	  public	Common::Externalizable
{
public:
	// デストラクタ
	virtual ~Object();

	// コピーコンストラクタ
	Object(const Object& cOther_);

	// 型を得る
	ObjectType::Type getType() const;

#ifdef OBSOLETE // 現在のところStatement::Objectをマップに登録することはない
	// ハッシュコードを得る
	ModSize hashCode() const;
	// 大小比較
	bool operator<(const Object& cObj_) const;

	// ハッシュコードを計算する
	ModSize calculateHashCode();
#endif

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;
#ifndef SYD_COVERAGE
	// 文字列で値を得る(デバッグ用)
	virtual ModUnicodeString toString() const;
	// LISP形式で出力する(デバッグ用)
	virtual void toString(ModUnicodeOstrStream& cStream_, int iIndent_ = 0) const;
#endif

	// オブジェクトを追加する。Parser専用
	void addBody(Statement::Object* pBody_);
	// オブジェクトを空にする。Parser専用
	// 下位オブジェクトはdeleteせず、自分だけDeleteするとき使う
	void clearBody();

	// 下位のオブジェクトもまとめて解放する
	virtual void destruct();

#ifndef SYD_COVERAGE
	// toStringのデフォルト
	friend ModUnicodeString toStringDefault(const Object* pObj_);
	// TreeWalker専用
	virtual ModVector<Object*>* getElementVector();
#endif

	//初期化を行う
	static void initialize();
	//後処理を行う
	static void terminate();

	// パラメータObjectと自分の比較結果を返す
	bool equals(const Statement::Object* pObj_) const;

	// Optimize, Reorganize or System の識別子
	enum Generate
	{
		Undefine,
		Optimize,
		Reorganize,
		System
	};
#ifdef OBSOLETE
	Generate getGenerate() const;
#endif

	//ログ追加フラグ
	bool isRecordLog() const;

	//UnsignedIntegerをIntegerに変換する
	virtual void convertToSignedInteger();

	//And, Or を展開する
	virtual void expandCondition(int iDepth_ = 0);

	//コピーする
	virtual Object* copy() const = 0;

	// Analyzerを得る
#ifdef USE_OLDER_VERSION
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

/////////////////////////////
// Common::Externalizable::
	virtual int getClassID() const;

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

protected:
	// コンストラクタ (1)
	Object(ObjectType::Type eType_, Generate eGen_ = Undefine, bool bLogRec_ = false);
	// コンストラクタ (2)
	Object(ObjectType::Type eType_, int iN_, Generate eGen_ = Undefine, bool bLogRec_ = false);

	// 型を設定する
	void setType(ObjectType::Type eType_);

	// メンバーを得る
	Object*
	getElement(unsigned int i, ObjectType::Type type) const;
	// メンバーを設定する
	void
	setElement(unsigned int i, Object* p);

	// 整数型メンバーを得る
	int
	getIntegerElement(unsigned int i) const;
	// 整数型メンバーを設定する
	void
	setIntegerElement(unsigned int i, int v);

#ifdef OBSOLETE // 現在のところStatement::Objectをマップに登録することはない
	// ハッシュコードを計算する
	virtual ModSize getHashCode();

	// 同じ型のオブジェクト同士でless比較する
	virtual bool compare(const Object& cObj_) const;
#endif

	// メンバ
	// タイプ番号
	ObjectType::Type		m_eType;
	// 下位メンバ
	ModVector<Object*>		m_vecpElements;
#ifdef OBSOLETE
	// Optimeze or Reorganize の識別子
	const Generate			m_eGenerate;
#endif
	//ログ追加フラグ
	bool				m_bLogRec;

private:
	// 代入オペレーターは使わない
	Object& operator=(const Object& cOther_);

	ModSize m_iHashCode;
};

//
//	FUNCTION global
//	Statement::getClassInstance -- クラスのインスタンスを確保する
//
//	NOTES
//	シリアル化可能クラスのクラスIDからそのクラスのインスタンスを確保する。
//
//	ARGUMENTS
//	int iClassID_
//		クラスID
//
//	RETURN
//	Common::Externalizable*
//		シリアル化可能クラスのインスタンス。
//		存在しないクラスIDの場合は0を返す。
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Common::Externalizable* getClassInstance(int iClassID_);

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_OBJECT_H

//
// Copyright (c) 1999, 2002, 2003, 2004, 2005, 2006, 2008, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
