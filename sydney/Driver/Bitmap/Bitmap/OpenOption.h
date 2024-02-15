// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenOption.h --
// 
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BITMAP_OPENOPTION_H
#define __SYDNEY_BITMAP_OPENOPTION_H

#include "Bitmap/Module.h"
#include "Bitmap/Data.h"
#include "Bitmap/FileID.h"

#include "LogicalFile/OpenOption.h"
#include "LogicalFile/TreeNodeInterface.h"

#include "ModUnicodeOstrStream.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

//
//	CLASS
//	Bitmap::OpenOption -- ビットマップファイルドライバーのOpenOption
//
//	NOTES
//
class OpenOption
{
public:
	//
	//	STRUCT
	//	Bitmap::OpenOption::Key -- オープンオプションのキー
	//
	struct Key
	{
		enum Value
		{
			// 検索文(String)
			Condition = LogicalFile::OpenOption::DriverNumber::Bitmap,
			// verifyかどうか(Boolean)
			Verify,
			// verifyのときのRowID(String)
			RowID,

			// 以下は v16.5 から
			
			// group by のときのソートが降順(1)or昇順(0)(Integer)
			SortOrder
		};
	};
	
	//
	//	CLASS
	//	Bitmap::OpenOption::ParseValue
	//
	class ParseValue
	{
	public:
		ParseValue() : m_pNext(0), m_usOptionalChar(0) {}
		~ParseValue() { delete m_pNext; }

		// 等号検索か
		bool isEquals() const { return m_eType & Data::MatchType::IsEquals; }

		static Data::MatchType::Value
			getStream(const ModUnicodeChar*& p_,
					  ModUnicodeString& cstrValue_,
					  ModUnicodeChar& usOptionalChar_)
		{
			Data::MatchType::Value eType;
			p_++;
			if (*p_ == 'e')
			{
				eType = Data::MatchType::Equals;
			}
			else if (*p_ == 'n')
			{
				if (*(p_+1) == 'l')
					eType = Data::MatchType::EqualsToNull;
				else if (*(p_+1) == 'e')
					eType = Data::MatchType::NotEquals;
				else
					eType = Data::MatchType::EqualsToNull_All;
			}
			else if (*p_ == 'g')
			{
				if (*(p_+1) == 't')
					eType = Data::MatchType::GreaterThan;
				else
					eType = Data::MatchType::GreaterThanEquals;
			}
			else if (*p_ == 'l')
			{
				if (*(p_+1) == 't')
					eType = Data::MatchType::LessThan;
				else if (*(p_+1) == 'e')
					eType = Data::MatchType::LessThanEquals;
				else
					eType = Data::MatchType::Like;
			}
			else if (*p_ == 'u')
			{
				eType = Data::MatchType::Unknown;
			}
			p_ += 2;
			if (eType != Data::MatchType::EqualsToNull
				&& eType != Data::MatchType::EqualsToNull_All
				&& eType != Data::MatchType::Unknown)
				getStreamValue(p_, cstrValue_, usOptionalChar_);
			return eType;
		}

		void putStream(ModUnicodeOstrStream& cStream_)
		{
			switch (m_eType)
			{
			case Data::MatchType::Equals:
				cStream_ << "#eq";
				putStreamValue(cStream_);
				break;
			case Data::MatchType::EqualsToNull:
				cStream_ << "#nl";
				break;
			case Data::MatchType::GreaterThan:
				cStream_ << "#gt";
				putStreamValue(cStream_);
				break;
			case Data::MatchType::GreaterThanEquals:
				cStream_ << "#ge";
				putStreamValue(cStream_);
				break;
			case Data::MatchType::LessThan:
				cStream_ << "#lt";
				putStreamValue(cStream_);
				break;
			case Data::MatchType::LessThanEquals:
				cStream_ << "#le";
				putStreamValue(cStream_);
				break;
			case Data::MatchType::NotEquals:
				cStream_ << "#ne";
				putStreamValue(cStream_);
				break;
			case Data::MatchType::Like:
				cStream_ << "#lk";
				putStreamLikeValue(cStream_);
				break;
			case Data::MatchType::Unknown:
				cStream_ << "#uk";
				break;
			case Data::MatchType::EqualsToNull_All:
				cStream_ << "#na";
				break;
			}
		}
		
		Data::MatchType::Value					m_eType;
		ModUnicodeString						m_cValue;
		// See Condition::Cond::m_usOptionalChar
		ModUnicodeChar							m_usOptionalChar;
		
		ParseValue*								m_pNext;

		static void putStreamValue(ModUnicodeOstrStream& cStream_,
								   const ModUnicodeString& cValue_,
								   ModUnicodeChar usOptionalChar_ = 0)
		{
			cStream_ << '(';
			const ModUnicodeChar* p = cValue_;
			while (*p)
			{
				if (*p == ')' || *p == '\\' || *p == ',')
				{
					cStream_ << '\\';
				}
				cStream_ << *p++;
			}
			if (usOptionalChar_ != 0)
			{
				cStream_ << ',' << usOptionalChar_;
			}
			cStream_ << ')';
		}
		
	private:
		void putStreamValue(ModUnicodeOstrStream& cStream_)
		{
			putStreamValue(cStream_, m_cValue, m_usOptionalChar);
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
		//	And it's preferrable that the ascii character is NOT '\'
		//	for avoiding a confusion.
		//	'\' has been used as an escape character for this module.
		//	So, '*' is used as an escape character for SQL-sentence.
		//
		//	This function can NOT be unified with putStreamValue(),
		//	because of indistinguishable from
		//	a white space for a padding character and for a escape character.
		//	(After v16.2, a padding character has been used.)
		//
		void putStreamLikeValue(ModUnicodeOstrStream& cStream_)
		{
			cStream_ << '(';
			const ModUnicodeChar* p = m_cValue;
			while (*p)
			{
				if (m_usOptionalChar != 0)
				{
					if (*p == m_usOptionalChar)
					{
						// Replace an escape character for SQL-sentence.
						// When m_usOptionalChar is '*',
						// NOT need to escape '*' with '*'.
						cStream_ << '*';
						if (*++p == 0)
							break;
					}
					else if (*p == '*')
					{
						// Escape '*' with '*'.
						// Because '*' is NOT an original escape character
						// and it is treated as an escape character after here.
						cStream_ << '*';
					}
				}
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
				cStream_ << ',' << '*';
			}
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
								   ModUnicodeChar& usOptionalChar_)
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
			if (*p_ == ',')
			{
				p_++;
				usOptionalChar_ = *p_;
				p_++;
			}
			//; _SYDNEY_ASSERT(*p_ == ')');
			p_++;
		}
	};

	// コンストラクタ
	OpenOption(const FileID& cFileID_,
			   LogicalFile::OpenOption& cLogicalOpenOption_);
	// デストラクタ
	virtual ~OpenOption();

	// 検索ノードを解析し、必要な情報をOpenOptionに設定する
	bool getSearchParameter(const LogicalFile::TreeNodeInterface* pCondition_);
	
	// プロジェクションノードを解析し、必要な情報をOpenOptionに設定する
	bool getProjectionParameter(const LogicalFile::TreeNodeInterface* pNode_);

	// ソートノードを解析し、必要な情報をOpenOptionに設定する
	bool getSortParameter(const LogicalFile::TreeNodeInterface* pNode_);
	
private:
	// TreeNodeのORをパースし文字列にする
	bool parseOrNode(const LogicalFile::TreeNodeInterface* pCondition_,
					 ModUnicodeOstrStream& cStream_);

	// TreeNodeのANDをパースし文字列にする
	bool parseAndNode(const LogicalFile::TreeNodeInterface* pCondition_,
					  ModUnicodeOstrStream& cStream_);
	
	// TreeNodeをパースし文字列にする
	bool parseTreeNode(const LogicalFile::TreeNodeInterface* pCondition_,
					   ModUnicodeOstrStream& cStream_);

	// 1つのノードを解析する
	bool parseOneNode(const LogicalFile::TreeNodeInterface* pCondition_,
					  ParseValue*& pMain_,
					  ParseValue*& pOther_);
	
	// Equalsノードを解析する
	bool parseEqualsNode(const LogicalFile::TreeNodeInterface* pCondition_,
						 ParseValue*& pMain_,
						 ParseValue*& pOther_);
	
	// LessThanノードを解析する
	bool parseLessThanNode(const LogicalFile::TreeNodeInterface* pCondition_,
						   ParseValue*& pMain_,
						   ParseValue*& pOther_);
	
	// GreaterThanノードを解析する
	bool parseGreaterThanNode(const LogicalFile::TreeNodeInterface* pCondition_,
							  ParseValue*& pMain_,
							  ParseValue*& pOther_);
	
	// NotEqualsノードを解析する
	bool parseNotEqualsNode(const LogicalFile::TreeNodeInterface* pCondition_,
							ParseValue*& pMain_,
							ParseValue*& pOther_);
	
	// EqualsToNullノードを解析する
	bool parseEqualsToNullNode(
		const LogicalFile::TreeNodeInterface* pCondition_,
		ParseValue*& pMain_,
		ParseValue*& pOther_);
	
	// Likeノードを解析する
	bool parseLikeNode(const LogicalFile::TreeNodeInterface* pCondition_,
					   ParseValue*& pMain_,
					   ParseValue*& pOther_);

	// Verifyのときの検索ノードを解析する
	bool parseVerifyNode(const LogicalFile::TreeNodeInterface* pCondition_,
						 ModUnicodeOstrStream& cStream_,
						 ModUnicodeString& cRowID_);

	// 単項演算をチェックする
	bool checkOneTerm(const LogicalFile::TreeNodeInterface* pCondition_,
					  Data::MatchType::Value& eMatch_);
	// 2項演算をチェックする
	bool checkTwoTerm(const LogicalFile::TreeNodeInterface* pCondition_,
					  Data::MatchType::Value& eMatch_,
					  ModUnicodeString& cstrValue_,
					  bool& isValid_,
					  bool& bNoPadKey_);
	// Alternate two terms.
	void alternateTerm(const LogicalFile::TreeNodeInterface*& pFirst_,
					   const LogicalFile::TreeNodeInterface*& pSecond_,
					   Data::MatchType::Value& eMatch_);

	// Check whether the sort order of the field is NO PAD
	bool checkNoPadSortOrder(Data::Type::Value eType_,
							 bool bFixedField_,
							 bool bNoPadKey_) const;

	// Set ParseValue of Equals.
	void setEqualsParseValue(ParseValue* pNew_,
							 ParseValue*& pMain_,
							 ParseValue*& pOther_,
							 bool bNoPadSortOrder_,
							 bool isNoPad_);
	void setEqualsParseValueWithSortOrder(ParseValue* pNew1_,
										  ParseValue*& pMain_,
										  ParseValue*& pOther_,
										  bool bNoPadSortOrder_,
										  bool isNoPad_);

	// Set ParseValue of LessThan
	void setLessThanParseValue(ParseValue* pNew_,
							   ParseValue*& pMain_,
							   ParseValue*& pOther_,
							   bool bNoPadField_,
							   bool bNoPadKey_);
	void setLessThanParseValueWithSortOrder(ParseValue* pNew1_,
											ParseValue*& pMain_,
											ParseValue*& pOther_,
											bool bNoPadField_,
											bool bNoPadKey_);

	// Set ParseValue of GreaterThan
	void setGreaterThanParseValue(ParseValue* pNew_,
								  ParseValue*& pMain_,
								  ParseValue*& pOther_,
								  bool bNoPadField_,
								  bool bNoPadKey_);
	void setGreaterThanParseValueWithSortOrder(ParseValue* pNew1_,
											   ParseValue*& pMain_,
											   ParseValue*& pOther_,
											   bool bNoPadField_,
											   bool bNoPadKey_);
	
	// Set ParseValue of PrefixMatch
	void setPrefixMatchParseValue(ParseValue* pNew1_,
								  ParseValue*& pMain_,
								  ParseValue*& pOther_,
								  bool bNoPadField_,
								  bool bNoPadKey_);
	void setPrefixMatchParseValueWithSortOrder(ParseValue* pNew1_,
											   ParseValue*& pParseValue_,
											   bool bNoPadField_,
											   bool bNoPadKey_);

	// Set ParseValue of Like
	void setLikeParseValue(ModUnicodeString& cstrValue_,
						   ModUnicodeChar usEscape_,
						   ParseValue*& pOther_);
	
	// Set ParseValue of Unknown
	void setUnknownParseValue(ParseValue*& pMain_,
							  ParseValue*& pOther_);

	// ストリームに設定する
	bool setToStream(ParseValue* pMain_,
					 ParseValue* pOther_,
					 ModUnicodeOstrStream& cStream_);

	// Get the padding char.
	ModUnicodeChar getPaddingChar(bool bNoPad_) const;

	// Get the position of the trailling SOH. [0-base]
	int getPositionOfTraillingSOH(
		const ModUnicodeString& cstrString_) const;
	
	// ファイルID
	const FileID& m_cFileID;
	// オープンオプション
	LogicalFile::OpenOption& m_cOpenOption;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif //__SYDNEY_BITMAP_OPENOPTION_H

//
//	Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
