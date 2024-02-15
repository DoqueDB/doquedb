// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Condition.h --
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE2_CONDITION_H
#define __SYDNEY_BTREE2_CONDITION_H

#include "Btree2/Module.h"
#include "Btree2/Data.h"
#include "Btree2/AutoPointer.h"
#include "Btree2/Compare.h"
#include "Btree2/FileID.h"
#include "Common/Object.h"
#include "LogicalFile/OpenOption.h"
#include "LogicalFile/TreeNodeInterface.h"
#include "Utility/CharTrait.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_BTREE2_BEGIN

//
//	CLASS
//	Btree2::Condition -- 条件作成クラス
//
//	NOTES
//
class Condition : public Common::Object
{
public:
	//
	//	STRUCT
	//	Btree2::Condition::LimitCond -- 条件をあらわすデータ
	//
	struct LimitCond
	{
		LogicalFile::TreeNodeInterface::Type
								m_eType;		// 一致条件
		AutoPointer<ModUInt32>	m_pBuffer;		// 検索条件
		ModSize					m_uiBufferSize;	// 検索条件のサイズ(byte)
		unsigned char			m_nullBitmap;	// 検索条件のnullビットマップ
		Compare					m_cCompare;		// 比較クラス

		void copy(const LimitCond& other_);		// 中身をコピーする
		void allocate(ModSize size_);			// 検索条件バッファを確保する

		void clear()
		{
			m_eType = LogicalFile::TreeNodeInterface::Undefined;
			m_pBuffer = 0;
			m_uiBufferSize = 0;
			m_nullBitmap = 0;
		}
	};

	//
	//	STRUCT
	//	Btree2::Condition::Cond -- 条件をあらわすデータ
	//
	struct Cond
	{
		LogicalFile::TreeNodeInterface::Type
								m_eType;		// 一致条件
		AutoPointer<ModUInt32>	m_pBuffer;		// 検索条件
		ModSize					m_uiBufferSize;	// 検索条件のサイズ(byte)
		int						m_iFieldID;		// フィールド番号
		// Optional character
		// For like, it's escape character.
		// For other than like, it's padding character.
		ModUnicodeChar			m_usOptionalChar;
		
		void copy(const Cond& other_);			// 中身をコピーする
		void allocate(ModSize size_);			// 検索条件バッファを確保する
	};

	//
	//	STRUCT
	//	Btree2::Condition::Key -- オープンオプションのキー
	//
	struct Key
	{
		enum Value
		{
			ConditionCount = LogicalFile::OpenOption::DriverNumber::Btree,
			Condition,
			EqualFieldNumber,
			Reverse,
			FetchFieldNumber
		};
	};
	
	//
	//	CLASS
	//	Btree2::Condition::ParseValue
	//
	class ParseValue
	{
	public:
		ParseValue() : m_usOptionalChar(0), m_pNext(0), m_bNormalized(false) {}
		~ParseValue() { delete m_pNext; }

		static LogicalFile::TreeNodeInterface::Type
			getStream(const ModUnicodeChar*& p_,
					  ModUnicodeString& cstrValue_,
					  ModUnicodeChar& usOptionalChar_,
					  bool& bNormalized_)
		{
			using namespace LogicalFile;

			TreeNodeInterface::Type eType;
			p_++;
			if (*p_ == 'e')
			{
				eType = TreeNodeInterface::Equals;
			}
			else if (*p_ == 'n')
			{
				if (*(p_+1) == 'l')
					eType = TreeNodeInterface::EqualsToNull;
				else
					eType = TreeNodeInterface::NotEquals;
			}
			else if (*p_ == 'g')
			{
				if (*(p_+1) == 't')
					eType = TreeNodeInterface::GreaterThan;
				else
					eType = TreeNodeInterface::GreaterThanEquals;
			}
			else if (*p_ == 'l')
			{
				if (*(p_+1) == 't')
					eType = TreeNodeInterface::LessThan;
				else if (*(p_+1) == 'e')
					eType = TreeNodeInterface::LessThanEquals;
				else
					eType = TreeNodeInterface::Like;
			}
			else if (*p_ == 'u')
			{
				eType = TreeNodeInterface::Unknown;
			}
			p_ += 2;
			if (eType != TreeNodeInterface::EqualsToNull
				&& eType != TreeNodeInterface::Unknown)
				getStreamValue(p_, cstrValue_, usOptionalChar_, bNormalized_);
			return eType;
		}

		void putStream(ModUnicodeOstrStream& cStream_)
		{
			using namespace LogicalFile;

			switch (m_eType)
			{
			case TreeNodeInterface::Equals:
				cStream_ << "#eq";
				putStreamValue(cStream_);
				break;
			case TreeNodeInterface::EqualsToNull:
				cStream_ << "#nl";
				break;
			case TreeNodeInterface::GreaterThan:
				cStream_ << "#gt";
				putStreamValue(cStream_);
				break;
			case TreeNodeInterface::GreaterThanEquals:
				cStream_ << "#ge";
				putStreamValue(cStream_);
				break;
			case TreeNodeInterface::LessThan:
				cStream_ << "#lt";
				putStreamValue(cStream_);
				break;
			case TreeNodeInterface::LessThanEquals:
				cStream_ << "#le";
				putStreamValue(cStream_);
				break;
			case TreeNodeInterface::NotEquals:
				cStream_ << "#ne";
				putStreamValue(cStream_);
				break;
			case TreeNodeInterface::Like:
				cStream_ << "#lk";
				putStreamLikeValue(cStream_);
				break;
			case TreeNodeInterface::Unknown:
				cStream_ << "#uk";
				break;
			}
		}
		
		LogicalFile::TreeNodeInterface::Type	m_eType;
		ModUnicodeString						m_cValue;
		// See Condition::Cond::m_usOptionalChar
		ModUnicodeChar							m_usOptionalChar;
		
		ParseValue*								m_pNext;

		bool									m_bNormalized;
		
	private:
		void putStreamValue(ModUnicodeOstrStream& cStream_)
		{
			cStream_ << '(';
			const ModUnicodeChar* p = m_cValue;
			while (*p)
			{
				if (*p == ')' || *p == '\\' || *p == ',')
				{
					cStream_ << '\\';
				}
				cStream_ << *p++;
			}
			if (m_usOptionalChar != 0)
			{
				cStream_ << ",#oc(" << m_usOptionalChar << ')';
			}
			if (m_bNormalized == true)
			{
				// LessThan's condition may be normalized when parsing.
				// See Condition::parseLikeNode() for detail.
				cStream_ << ",#ns";
			}
			cStream_ << ')';
		}
		//
		//	NOTES
		//
		//	When the type of the field is char or varchar,
		//	an escape character of SQL-sentence is casted
		//	to the type of unsigned char.
		//	If the escape character is a wide character,
		//	the cast will be failed.
		//	So, when parsing a like condition,
		//	the m_usOptionalChars in the m_cValue have to be replaced
		//	to an ascii character.
		//
		void putStreamLikeValue(ModUnicodeOstrStream& cStream_)
		{
			cStream_ << '(';

			// Escape fullwidth wildcard characters,
			// and replace an original escape character to an internal one.
			ModUnicodeString cstrEscaped;
			Utility::CharTrait::escape(
				m_cValue,
				Utility::CharTrait::EscapeTarget::FullwidthWildcard,
				m_usOptionalChar,
				cstrEscaped);

			const ModUnicodeChar* p = cstrEscaped;
			while (*p)
			{
				if (*p == ')' || *p == '\\' || *p == ',')
				{
					// The avobe characters are used
					// when parsing a string which is created here.
					// So escape them with '\'.
					cStream_ << '\\';
				}
				cStream_ << *p++;
			}
			if (m_usOptionalChar != 0)
			{
				// Set an new escape character for SQL-sentence.
				// The character may be replaced in CharTrait::escape().
				cStream_ << ",#oc(" << m_usOptionalChar << ')';
			}
			// Like's condition is not normalized when parsing.
			cStream_ << ')';
		}
		//
		//	NOTES
		//
		//	usOptionalChar_ has TWO applications.
		//	One is an escape character for LIKE.
		//	In such case, the character is '*'. 
		//	See ParseValue::putStreamLikeValue().
		//	The other is an padding character for other than LIKE.
		//	In such case, the character is ' '.
		//	See Condition::getPaddingChar().
		//	And these characters are NOT used at the same time.
		//
		static void getStreamValue(const ModUnicodeChar*& p_,
								   ModUnicodeString& cstrValue_,
								   ModUnicodeChar& usOptionalChar_,
								   bool& bNormalized_)
		{
			cstrValue_ = "";
			p_++;
			while (*p_ != ')' && *p_ != ',')
			{
				if (*p_ == '\\')
				{
					p_++;
				}
				cstrValue_ += *p_;
				p_++;
			}
			while (*p_ == ',')
			{
				p_ += 2; // ",#"
				if (*p_ == 'o')
				{
					p_ += 3; // "oc("
					usOptionalChar_ = *p_;
					p_ += 2; // "'OPTIONAL CHARACTER')"
				}
				else if (*p_ == 'n')
				{
					p_+= 2; // "ns"
					bNormalized_ = true;
				}
			}
			//; _SYDNEY_ASSERT(*p_ == ')');
			p_++;
		}
	};

	// コンストラクタ
	Condition(const FileID& cFileID_);
	// デストラクタ
	virtual ~Condition();

	// コピーを得る
	Condition* copy();

	// TreeNodeから検索構文を作成する
	bool getSearchParameter(const LogicalFile::TreeNodeInterface* pCondition_,
							LogicalFile::OpenOption& cOpenOption_);

	// オープンオプションを設定する
	void setOpenOption(const LogicalFile::OpenOption& cOpenOption_,
					   int iNumber_ = 0);
	// fetch用のキーを設定する
	void setFetchKey(const Common::DataArrayData& cKey_);
	// 制約ロックのための検索か否か
	void setConstraintLock(bool flag_) { m_bConstraintLock = flag_; }

	// 下限条件を得る
	LimitCond& getLowerCondition() { return m_cLowerData; }
	// 上限条件を得る
	LimitCond& getUpperCondition() { return m_bLowerIsUpper ? m_cLowerData : m_cUpperData; }

	// その他条件にマッチしているかどうか
	bool isOtherConditionMatch(const ModUInt32* pBuffer_);
	bool isOtherConditionMatch(const ModUInt32* pBuffer_,
							   unsigned char nullBitmap_);

	// 有効な検索条件か
	bool isValid() const { return m_bValid; }

	// Fetchに利用するフィールド数を設定する
	void setFetchField(int n) { m_iFetchField = n; }
	// Fetchに利用するフィールド数を得る
	int getFetchField() const { return m_iFetchField; }

	// 制約ロックのための検索か
	bool isConstraintLock() const { return m_bConstraintLock; }

private:
	// TreeNodeをパースし、OpenOptionに設定する
	bool parseTreeNode(const LogicalFile::TreeNodeInterface* pCondition_,
					   int n_,
					   LogicalFile::OpenOption& cOpenOption_);

	// 1つのノードを解析する
	bool parseOneNode(const LogicalFile::TreeNodeInterface* pCondition_,
					  ModVector<ParseValue*>& vecMain_,
					  ModVector<ParseValue*>& vecOther_);
	
	// Equalsノードを解析する
	bool parseEqualsNode(const LogicalFile::TreeNodeInterface* pCondition_,
						 ModVector<ParseValue*>& vecMain_,
						 ModVector<ParseValue*>& vecOther_);
	
	// LessThanノードを解析する
	bool parseLessThanNode(const LogicalFile::TreeNodeInterface* pCondition_,
						   ModVector<ParseValue*>& vecMain_,
						   ModVector<ParseValue*>& vecOther_);
	
	// GreaterThanノードを解析する
	bool parseGreaterThanNode(const LogicalFile::TreeNodeInterface* pCondition_,
							  ModVector<ParseValue*>& vecMain_,
							  ModVector<ParseValue*>& vecOther_);
	
	// NotEqualsノードを解析する
	bool parseNotEqualsNode(const LogicalFile::TreeNodeInterface* pCondition_,
							ModVector<ParseValue*>& vecMain_,
							ModVector<ParseValue*>& vecOther_);
	
	// EqualsToNullノードを解析する
	bool parseEqualsToNullNode(
		const LogicalFile::TreeNodeInterface* pCondition_,
		ModVector<ParseValue*>& vecMain_,
		ModVector<ParseValue*>& vecOther_);
	
	// Likeノードを解析する
	bool parseLikeNode(const LogicalFile::TreeNodeInterface* pCondition_,
					   ModVector<ParseValue*>& vecMain_,
					   ModVector<ParseValue*>& vecOther_);

	// 単項演算をチェックする
	int checkOneTerm(const LogicalFile::TreeNodeInterface* pCondition_);
	// 2項演算をチェックする
	int checkTwoTerm(const LogicalFile::TreeNodeInterface* pCondition_,
					 LogicalFile::TreeNodeInterface::Type& eMatch_,
					 ModUnicodeString& cstrValue_,
					 bool& isValid_,
					 bool& isNoPad_);
	// Alternate two terms.
	void alternateTerm(const LogicalFile::TreeNodeInterface*& pFirst_,
					   const LogicalFile::TreeNodeInterface*& pSecond_,
					   LogicalFile::TreeNodeInterface::Type& eMatch_);

	// Check whether the sort order of the field is NO PAD.
	bool checkNoPadSortOrder(Data::Type::Value eType_,
							 bool bFixedField_, bool bNoPadKey_) const;

	// Set ParseValue of Equals.
	void setEqualsParseValue(ParseValue* pNew_,
							 ParseValue*& pMain_,
							 ParseValue*& pOther_,
							 bool bNoPadSortOrder_,
							 bool isNoPad_);
	// Set ParseValue of Equals.
	void setEqualsParseValueWithSortOrder(ParseValue* pNew1_,
										  ParseValue*& pMain_,
										  ParseValue*& pOther_,
										  bool bNoPadSortOrder_,
										  bool isNoPad_);

	// Set ParseValue of LessThan
	void setLessThanParseValue(ParseValue* pNew_,
							   ParseValue*& pMain_,
							   ParseValue*& pOther_,
							   bool bNoPadSortOrder_,
							   bool isNoPad_);
	// Set ParseValue of LessThan
	void setLessThanParseValueWithSortOrder(ParseValue* pNew_,
											ParseValue*& pMain_,
											ParseValue*& pOther_,
											bool bNoPadSortOrder_,
											bool isNoPad_);
	
	// Set ParseValue of GreaterThan
	void setGreaterThanParseValue(ParseValue* pNew_,
								  ParseValue*& pMain_,
								  ParseValue*& pOther_,
								  bool bNoPadSortOrder_,
								  bool isNoPad_);
	// Set ParseValue of GreaterThan
	void setGreaterThanParseValueWithSortOrder(ParseValue* pNew_,
											   ParseValue*& pMain_,
											   ParseValue*& pOther_,
											   bool bNoPadSortOrder_,
											   bool isNoPad_);

	// Set ParseValue of the prefix match.
	void setPrefixMatchParseValue(ParseValue* pNew1_,
								  ParseValue*& pMain_,
								  ParseValue*& pOther_,
								  bool bNoPadSortOrder_,
								  bool isNoPad_);
	// Set ParseValue of the prefix match.
	void setPrefixMatchParseValueWithSortOrder(ParseValue* pNew1_,
											   ParseValue*& pParseValue_,
											   bool bNoPadSortOrder_,
											   bool isNoPad_);

	// Set ParseValue of the like.
	void setLikeParseValue(ModUnicodeString& cstrValue_,
						   ModUnicodeChar escape_,
						   ParseValue*& pOther_);
	
	// Set ParseValue of Unknown
	void setUnknownParseValue(ModVector<ParseValue*>& vecMain_,
							  ModVector<ParseValue*>& vecOther_);

	// OpenOptionに設定する
	bool setToOpenOption(ModVector<ParseValue*>& vecMain_,
						 ModVector<ParseValue*>& vecOther_,
						 int n_,
						 LogicalFile::OpenOption& cOpenOption_);

	// 下限条件を設定する
	void setLowerData(const ModUnicodeChar*& p_,
					  LogicalFile::OpenOption::OpenMode::Value eMode_);
	// 上限条件を設定する
	void setUpperData(const ModUnicodeChar*& p_,
					  LogicalFile::OpenOption::OpenMode::Value eMode_);
	// その他条件を設定する
	void setOtherData(const ModUnicodeChar*& p_);

	// Common::Dataを作成する
	Common::Data::Pointer createCommonData(
		Data::Type::Value eType_,
		int iPosition_, 
		ModUnicodeString& cstrValue_,
		LogicalFile::TreeNodeInterface::Type eMatch_,
		bool bAlreadyNormalized_ = false);

	// Cond構造体を作成する
	Cond makeCond(LogicalFile::TreeNodeInterface::Type eType_,
				  const Common::Data& cData_, int n_,
				  ModUnicodeChar usOptionalChar_);

	// Get the padding char.
	ModUnicodeChar getPaddingChar(bool bNoPad_) const;

	// Get the key type.
	Data::Type::Value getKeyType(Data::Type::Value eType_,
								 ModUnicodeChar usOptionalChar_) const;

	// Get the position of the trailling SOH. [0-base]
	int getPositionOfTraillingSOH(
		const ModUnicodeString& cstrString_) const;
	
	// ファイルID
	const FileID& m_cFileID;
	// キータイプ
	const ModVector<Data::Type::Value>& m_vecKeyType;
	mutable ModVector<int> m_vecKeyPosition;

	// 異表記正規化するかどうか
	bool m_isNormalized;

	// 下限条件
	LimitCond m_cLowerData;
	// 上限条件
	LimitCond m_cUpperData;
	// 下限＝上限
	bool m_bLowerIsUpper;

	// その他条件
	ModVector<Cond> m_vecOtherCondition;

	// 比較クラス
	Compare m_cCompare;
	// データクラス
	Data m_cData;

	// 有効な検索条件かどうか
	bool m_bValid;
	// 制約ロックのための検索かどうか
	bool m_bConstraintLock;
	// エントリにヘッダーがあるかどうか
	bool m_bHeader;

	// Data class for fetch
	Data m_cFetchData;
	// Fetchフィールドの数
	int m_iFetchField;
	// The types of the field for fetch.
	ModVector<Data::Type::Value> m_vecFetchType;
	// The count of the field which need to expand the condition for fetch.
	int m_iExpandedFetchField;
	// The types of the field which need to expand the condition for fetch.
	ModVector<Data::Type::Value> m_vecExpandedFetchType;
};

_SYDNEY_BTREE2_END
_SYDNEY_END

#endif //__SYDNEY_BTREE2_CONDITION_H

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
