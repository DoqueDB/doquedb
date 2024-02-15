// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Sequence.h -- シーケンス関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_SEQUENCE_H
#define	__SYDNEY_SCHEMA_SEQUENCE_H

#include "Schema/Module.h"
#include "Schema/Object.h"

#include "Admin/Verification.h"
#include "Common/Object.h"
#include "Os/CriticalSection.h"
#include "Os/Limits.h"
#include "Os/Path.h"

#include "ModTypes.h"

_SYDNEY_BEGIN

namespace PhysicalFile {
	class File;
	class Page;
}

namespace Trans {
	class Transaction;
	class TimeStamp;
}

_SYDNEY_SCHEMA_BEGIN

class Default;

//	CLASS
//	Schema::Sequence -- Declare interface for sequence classes
//
//	NOTES

class Sequence
	: public	Common::Object
{
public:
	//	STRUCT
	//	Schema::Sequence::Unsigned -- used to create a namespace for unsigned sequence
	//
	//	NOTES

	struct Unsigned
	{
		//	TYPEDEF
		//	Schema::Sequence::Unsigned::Value -- シーケンスの値を表す型
		//
		//	NOTES

		typedef	unsigned int	Value;

		//	CONST
		//	Schema::Sequence::Unsigned::Invalid -- 不正なシーケンスの値
		//
		//	NOTES

		//	CONST
		//	Schema::Sequence::Unsigned::MaxValue -- シーケンスの最大値(default)
		//
		//	NOTES

		enum
		{
			Invalid =			ModUInt32Max,
			MaxValue =			Invalid - (Schema::Object::Category::ValueNum
										   * 20 /* max num of columns */
										   * 2  /* column + field */
										   + 4 * 3 /* index fields */)
		};
	};

	//	STRUCT
	//	Schema::Sequence::Signed -- used to create a namespace for signed sequence
	//
	//	NOTES

	struct Signed
	{
		//	TYPEDEF
		//	Schema::Sequence::Signed::Value -- シーケンスの値を表す型
		//
		//	NOTES

		typedef	int	Value;

		//	CONST
		//	Schema::Sequence::Signed::Invalid -- 不正なシーケンスの値
		//
		//	NOTES

		//	CONST
		//	Schema::Sequence::Unsigned::MinValue -- シーケンスの最小値(default)
		//
		//	NOTES

		//	CONST
		//	Schema::Sequence::Unsigned::MaxValue -- シーケンスの最大値(default)
		//
		//	NOTES

		enum
		{
			Invalid =			ModInt32Min,
			MinValue =			0,
			MaxValue =			ModInt32Max
		};
	};

	//	CLASS
	//	Schema::Sequence::Value -- シーケンスの値を保持するクラス
	//
	//	NOTES

	class Value
		: public Common::Object
	{
	public:
		// コンストラクター
		Value() : m_bSigned(false), m_uValue(Unsigned::Invalid) {}
		Value(Unsigned::Value uValue_) : m_bSigned(false), m_uValue(uValue_) {}
		Value(Signed::Value iValue_) : m_bSigned(true), m_iValue(iValue_) {}
		// コピーコンストラクター
		Value(const Value& cOther_) : m_bSigned(cOther_.m_bSigned), m_uValue(cOther_.m_uValue) {}
		// デストラクター
		~Value() {};

		// 代入演算子
		Value& operator=(Unsigned::Value uUnsigned_) {m_bSigned = false; m_uValue = uUnsigned_; return *this;}
		Value& operator=(Signed::Value iSigned_) {m_bSigned = true; m_iValue = iSigned_; return *this;}
		Value& operator=(const Value& cOther_)
		{
			if (this != &cOther_) {
				if ((m_bSigned = cOther_.m_bSigned) == true) {
					m_iValue = cOther_.m_iValue;
				} else {
					m_uValue = cOther_.m_uValue;
				}
			}
			return *this;
		}

		// 比較演算子
		bool operator==(const Value& cOther_) const;
		bool operator<(const Value& cOther_) const;

		// Incrementする
		Value& operator+=(Signed::Value iIncrement_)
		{
			if (m_bSigned) {
				m_iValue += iIncrement_;
			} else {
				m_uValue += iIncrement_;
			}
			return *this;
		}

		// 未設定の状態か
		bool isInvalid() const
		{
			return (m_bSigned && (m_iValue == Signed::Invalid))
				|| (!m_bSigned && (m_uValue == Unsigned::Invalid));
		}

		// 1を加えられるか
		bool isAbleToAddOne(const Value& cMax_) const
		{
			return *this < cMax_;
		}
		// 値を加えられるか
		bool isAbleToAdd(Signed::Value iIncrement_, const Value& cMax_) const;

		// 値の取得
		Unsigned::Value getUnsigned() const;
		Signed::Value getSigned() const;

		// 物理ファイルに書き込む
		void write(PhysicalFile::Page* pPage_, Trans::Transaction& cTrans_);
		// 物理ファイルから読み込む
		void read(PhysicalFile::Page* pPage_, Trans::Transaction& cTrans_);

	private:
		bool m_bSigned;					// 符号付きか
		union {
			Unsigned::Value m_uValue;	// 符号なしの値
			Signed::Value m_iValue;		// 符号付きの値
		};
	};

	// コンストラクター
	Sequence(const Os::Path& cPath_,
			 ObjectID::Value iDatabaseID_,
			 Unsigned::Value uMax_ = Unsigned::MaxValue,
			 bool bMount_ = true,
			 Schema::Object::Scope::Value eScope_ = Schema::Object::Scope::Permanent,
			 bool bReadOnly_ = false);
	Sequence(const Os::Path& cPath_,
			 ObjectID::Value iDatabaseID_,
			 ObjectID::Value iTableID_,
			 ObjectID::Value iColumnID_,
			 Unsigned::Value uMax_,
			 bool bMount_ = true,
			 Schema::Object::Scope::Value eScope_ = Schema::Object::Scope::Permanent,
			 bool bReadOnly_ = false);
	Sequence(const Os::Path& cPath_,
			 ObjectID::Value iDatabaseID_,
			 ObjectID::Value iTableID_,
			 ObjectID::Value iColumnID_,
			 const Default& cDefault_,
			 bool bMount_ = true,
			 Schema::Object::Scope::Value eScope_ = Schema::Object::Scope::Permanent,
			 bool bReadOnly_ = false);

	// デストラクター
	virtual ~Sequence();

	// Ascending Sequenceかを得る
	bool					isAscending() const;
	// 値を明示されても整合するかを得る
	bool					isGetMax() const;

	void					create(Trans::Transaction& cTrans_,
								   const Value& cInit_ = Value(),
								   bool bAllowExistence_ = false);
												// シーケンスを生成する
	void					drop(Trans::Transaction& cTrans_, bool bForce_ = true);
												// シーケンスを破棄する
	void					mount(Trans::Transaction& cTrans_);
												// ファイルを mount する
	void					unmount(Trans::Transaction& cTrans_);
												// ファイルを unmount する
	void					flush(Trans::Transaction& cTrans_);
												// ファイルを flush する
	void					sync(Trans::Transaction& cTrans_, bool& incomplete, bool& modified);
												// 不要な版を破棄する
	void					startBackup(Trans::Transaction& cTrans_,
										bool bRestorable_ = true);
												// バックアップを開始する
	void					endBackup(Trans::Transaction& cTrans_);
												// バックアップを終了する
	void					recover(Trans::Transaction& cTrans_,
									const Trans::TimeStamp& cPoint_);
												//	障害から回復する
	void					restore(Trans::Transaction&	Transaction_,
									const Trans::TimeStamp&	Point_);
												// ある時点に開始された
												// 読取専用トランザクションが
												// 参照する版を最新版とする
	void					move(Trans::Transaction& cTrans_,
								 const Os::Path& cNewPath_,
								 bool bUndo_ = false,
								 bool bRecovery_ = false);
												// シーケンスを移動する

	Value					getLastValue() const;
												// シーケンスの値を得る
	Value					getNextValue(Trans::Transaction& cTrans_, const Value& cInit_ = Value());
												// シーケンスの値を増やして、
												// その値を得る
	Value					getNextValue(const Value& cValue_, Trans::Transaction& cTrans_,
										 const Value& cInit_ = Value(), bool bPersist_ = true);
												// シーケンスの値の整合性をとる

	bool					isAccessible(Trans::Transaction& cTrans_);
												// ファイルがあるかを得る

	void					verify(Admin::Verification::Progress& cResult_,
								   Trans::Transaction& cTrans_,
								   Admin::Verification::Treatment::Value eTreatment_,
								   const Value& cValue_);
												// 整合性検査

	void					attachFile(const Os::Path& cPath_,
									   bool bMount_ = true,
									   Schema::Object::Scope::Value eScope_ = Schema::Object::Scope::Permanent,
									   bool bReadOnly_ = false);
	void					detachFile();

	// m_cValueの値をファイルに書き込む
	void					persist(Trans::Transaction& cTrans_);

private:
	void					setValue(Trans::Transaction& cTrans_,
									 const Value& cValue_);

	void					verifyExistence(Admin::Verification::Progress& cResult_,
											Trans::Transaction& cTrans_,
											Admin::Verification::Treatment::Value eTreatment_,
											const Value& cValue_);
	void					verifyFile(Admin::Verification::Progress& cResult_,
									   Trans::Transaction& cTrans_,
									   Admin::Verification::Treatment::Value eTreatment_);
	void					verifyValue(Admin::Verification::Progress& cResult_,
										Trans::Transaction& cTrans_,
										Admin::Verification::Treatment::Value eTreatment_,
										const Value& cValue_);
												// 整合性検査の下請け

	// m_cValueの値をファイルに書き込む
	void					doPersist(Trans::Transaction& cTrans_);
	// m_cValueの値をファイルから読み込む
	void					load(Trans::Transaction& cTrans_, const Value& cInit_ = Value());

	// Defaultの内容からメンバーの値をセットする
	void					setArgument(const Default& cDefault_);

	// 次に割り当てるm_cValueの値が制限値を超えるか
	bool					isReachMax(const Value& cValue_) const;

	//////////////////////
	// ファイル作成遅延 //
	//////////////////////
	bool isVacant() const;
	void substantiate(Trans::Transaction& cTrans_, const Value& cInit_);

	PhysicalFile::File*		m_pFile;			// シーケンスを格納するファイル
	PhysicalFile::Page*		m_pPage;			// 読み書きするページ
	ObjectID::Value			m_iDatabaseID;		// このファイルが所属するデータ—ベースのＩＤ
	ObjectID::Value			m_iTableID;			// このファイルが所属する表のＩＤ
	ObjectID::Value			m_iColumnID;		// このファイルが値を生成する列のＩＤ
	Value					m_cValue;			// 現在の値
	Value					m_cMin;				// 最小値
	Value					m_cMax;				// 最大値
	Value					m_cInit;			// 初期値
	Signed::Value			m_iIncrement;		// ステップ値
	bool					m_bCycle;			// シーケンスが循環するか
	bool					m_bGetMax;			// 対応する列に値が明示されても現在の値に反映するか
	Os::Path				m_cPath;			// パス名

	bool					m_bLoaded;			// データが読み込まれているか
	bool					m_bDirty;			// データが変更されているか

	Os::CriticalSection		m_cLatch;			// 排他制御を行う
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_SEQUENCE_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
