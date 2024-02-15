// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CharTrait.cpp -- 文字列型の便利関数の関数定義
// 
// Copyright (c) 2009, 2010, 2023 Ricoh Company, Ltd.
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
const char srcFile[] = __FILE__;
const char moduleName[] = "Utility";
}

//	【注意】
//	UNA V9 でも OK なように ifdef が入っていたが、UNA V12 以降限定にした

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "SyTypes.h"

#include "Utility/CharTrait.h"

#include "Common/Assert.h"
#include "Common/StringData.h"
#include "Common/SystemParameter.h"
#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"
#include "Exception/NullNotAllowed.h"
#include "Exception/SQLSyntaxError.h"
#include "Exception/TooLongConditionalPattern.h"
#include "Exception/TooManyExpandedPattern.h"
#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"

#include "ModAutoPointer.h"
#include "ModLanguageSet.h"
#include "ModNLP.h"
#include "ModUnicodeString.h"
#include "ModMap.h"
#include "ModCharString.h"

//#include <iostream>
//#include <stdlib.h>
//using namespace std;

#ifdef USE_ICU
#include "unicode/normlzr.h"
#include "unicode/unistr.h"
#endif

_TRMEISTER_USING
_TRMEISTER_UTILITY_USING

namespace {

namespace _Parameter
{

//
//	TEMPLATE CLASS
//	_Parameter::_Boolean
//
//	NOTES
//	パラメータ値を取得する
//
class _Boolean
{
public:
	//コンストラクタ
	_Boolean(const char* pParameterName_, bool bDefaultValue_)
		: m_bRead(false), m_bGetDefault(false)
	{
		//パラメータ名をコピー
		m_cstrParameterName = pParameterName_;
		//デフォルト値をコピー
		m_bDefaultValue = bDefaultValue_;
	}

	//値を取得する
	const bool get(bool& bGetDefault_)
	{
		if (m_bRead == false)
		{
			//パラメータを取得する
			Os::AutoCriticalSection cAuto(m_cLatch);

			if (Common::SystemParameter::getValue(
					m_cstrParameterName, m_bValue) == false)
			{
				//パラメータが設定されていないので、デフォルト値
				m_bValue = m_bDefaultValue;
				m_bGetDefault = true;
			}

			m_bRead = true;
		}

		bGetDefault_ = m_bGetDefault;
		return m_bValue;
	}

	//値をクリアする
	void clear()
	{
		m_bRead = false;
		m_bGetDefault = false;
	}

	//パラメータ名を得る
	const ModCharString& getParameterName() const
	{
		return m_cstrParameterName;
	}

protected:
	//パラメータ値
	bool m_bValue;
	//デフォルト値
	bool m_bDefaultValue;

	//システムパラメータを参照したか
	bool m_bRead;

	//読み込み済みのパラメータ値はデフォルト値か
	bool m_bGetDefault;

private:
	//パラメータ名
	ModCharString m_cstrParameterName;

	//排他制御用のクリティカルセクション
	Os::CriticalSection m_cLatch;
};

class _String
{
public:
	//コンストラクタ
	_String(const char* pParameterName_,
			const ModUnicodeString& cDefaultValue_)
		: m_bRead(false), m_bGetDefault(false)
	{
		//パラメータ名をコピー
		m_cstrParameterName = pParameterName_;
		//デフォルト値をコピー
		m_cDefaultValue = cDefaultValue_;
	}

	//値を取得する
	const ModUnicodeString& get(bool& bGetDefault_)
	{
		if (m_bRead == false)
		{
			//パラメータを取得する
			Os::AutoCriticalSection cAuto(m_cLatch);

			if (Common::SystemParameter::getValue(
					m_cstrParameterName, m_cValue) == false)
			{
				//パラメータが設定されていないので、デフォルト値
				m_cValue = m_cDefaultValue;
				m_bGetDefault = true;
			}

			m_bRead = true;
		}

		bGetDefault_ = m_bGetDefault;
		return m_cValue;
	}

	//値をクリアする
	void clear()
	{
		m_bRead = false;
		m_bGetDefault = false;
	}

	//パラメータ名を得る
	const ModCharString& getParameterName() const
	{
		return m_cstrParameterName;
	}

protected:
	//パラメータ値
	ModUnicodeString m_cValue;
	//デフォルト値
	ModUnicodeString m_cDefaultValue;

	//システムパラメータを参照したか
	bool m_bRead;

	//読み込み済みのパラメータ値はデフォルト値か
	bool m_bGetDefault;

private:
	//パラメータ名
	ModCharString m_cstrParameterName;

	//排他制御用のクリティカルセクション
	Os::CriticalSection m_cLatch;
};

//
//	VARIABLE
//	Carriage -- 改行を跨る正規化を行うかどうか
//	Stremming -- ステミングを行うかどうか
//	DeleteSpace -- スペースを除去するかどうか
//	Lang -- 言語情報
//
_Boolean Carriage("Utility_NormalizeCarriage", true);
_Boolean Stemming("Utility_NormalizeStemming", false);
_Boolean DeleteSpace("Utility_NormalizeDeleteSpace", false);
_String Lang("Utility_NormalizeLanguage", "");

_Boolean CommonCarriage("Common_NormalizeCarriage", true);
_Boolean CommonStemming("Common_NormalizeStemming", false);
_Boolean CommonDeleteSpace("Common_NormalizeDeleteSpace", false);
_String CommonLang("Common_NormalizeLanguage", "");

} // namespace _Parameter

namespace _UNA
{
	// 単語境界のセパレータ
	ModUnicodeString _sep("/");
	
	// UNAに渡すパラメータを得る
	void getParameter(
		ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> >&
		param_,
		Boolean::Value bCarriage_,
		Boolean::Value bStemming_,
		Boolean::Value bDeleteSpace_)
	{
		// [NOTE] doNorm は設定しないが、デフォルトがtrue。
		//  詳細は ModNlpAnalyzerUnaJp::ModNlpAnalyzerUnaJp() や、
		//  LibUna/ModNLPLocal.cpp の ParameterWrapper::getBoolean() を参照。
		
		param_.insert("compound", "true");

		bool bGetDefault;
		bool temp;
		
		// [NOTE] もともとはCommonモジュールの一部だったので、
		//  まずUtilityをチェックし、未設定ならCommonもチェックする。

		if (bCarriage_ == Boolean::Unknown)
		{
			temp = _Parameter::Carriage.get(bGetDefault);
			if (bGetDefault == true)
			{
				bool temp2 = _Parameter::CommonCarriage.get(bGetDefault);
				if (bGetDefault == false)
				{
					temp = temp2;
				}
			}
		}
		else
			temp = (bCarriage_ == Boolean::True) ? true : false;
		param_.insert("carriage", (temp) ? "true" : "false");

		if (bStemming_ == Boolean::Unknown)
		{
			temp = _Parameter::Stemming.get(bGetDefault);
			if (bGetDefault == true)
			{
				bool temp2 = _Parameter::CommonStemming.get(bGetDefault);
				if (bGetDefault == false)
				{
					temp = temp2;
				}
			}
		}
		else
			temp = (bStemming_ == Boolean::True) ? true : false;
		param_.insert("stem", (temp) ? "true" : "false");

		if (bDeleteSpace_ == Boolean::Unknown)
		{
			temp = _Parameter::DeleteSpace.get(bGetDefault);
			if (bGetDefault == true)
			{
				bool temp2 = _Parameter::CommonDeleteSpace.get(bGetDefault);
				if (bGetDefault == false)
				{
					temp = temp2;
				}
			}
		}
		else
			temp = (bDeleteSpace_ == Boolean::True) ? true : false;
		// [NOTE] falseは削除しないではなく、ModNlpAsIs(リソース任せ)である
		param_.insert("space", (temp) ? "2" : "0");
	}

	// UNAに渡す言語指定を得る
	void getLanguage(ModLanguageSet& langSet_)
	{
		bool bGetDefault;
		
		// [NOTE] もともとはCommonモジュールの一部だったので、
		//  まずUtilityをチェックし、未設定ならCommonもチェックする。
		
		langSet_ = ModLanguageSet(_Parameter::Lang.get(bGetDefault));
		if (bGetDefault == true)
		{
			ModLanguageSet temp(_Parameter::CommonLang.get(bGetDefault));
			if (bGetDefault == false)
			{
				langSet_ = temp;
			}
		}
	}
} // namespace _UNA


namespace _StringData
{
#ifndef USE_ICU
	// 半角カタカナを全角に変換するための配列
	ModUnicodeChar _half2full[] =
	{
//FF61;HALFWIDTH IDEOGRAPHIC FULL STOP;So;0;ON;3002;;;;N;HALFWIDTH IDEOGRAPHIC PERIOD;;;;
		0x3002,
//FF62;HALFWIDTH LEFT CORNER BRACKET;Ps;0;ON;300C;;;;N;HALFWIDTH OPENING CORNER BRACKET;;;;
		0x300C,
//FF63;HALFWIDTH RIGHT CORNER BRACKET;Pe;0;ON;300D;;;;N;HALFWIDTH CLOSING CORNER BRACKET;;;;
		0x300D,
//FF64;HALFWIDTH IDEOGRAPHIC COMMA;So;0;ON;3001;;;;N;;;;;
		0x3001,
//FF65;HALFWIDTH KATAKANA MIDDLE DOT;So;0;L;30FB;;;;N;;;;;
		0x30FB,
//FF66;HALFWIDTH KATAKANA LETTER WO;Lo;0;L;30F2;;;;N;;;;;
		0x30F2,
//FF67;HALFWIDTH KATAKANA LETTER SMALL A;Lo;0;L;30A1;;;;N;;;;;
		0x30A1,
//FF68;HALFWIDTH KATAKANA LETTER SMALL I;Lo;0;L;30A3;;;;N;;;;;
		0x30A3,
//FF69;HALFWIDTH KATAKANA LETTER SMALL U;Lo;0;L;30A5;;;;N;;;;;
		0x30A5,
//FF6A;HALFWIDTH KATAKANA LETTER SMALL E;Lo;0;L;30A7;;;;N;;;;;
		0x30A7,
//FF6B;HALFWIDTH KATAKANA LETTER SMALL O;Lo;0;L;30A9;;;;N;;;;;
		0x30A9,
//FF6C;HALFWIDTH KATAKANA LETTER SMALL YA;Lo;0;L;30E3;;;;N;;;;;
		0x30E3,
//FF6D;HALFWIDTH KATAKANA LETTER SMALL YU;Lo;0;L;30E5;;;;N;;;;;
		0x30E5,
//FF6E;HALFWIDTH KATAKANA LETTER SMALL YO;Lo;0;L;30E7;;;;N;;;;;
		0x30E7,
//FF6F;HALFWIDTH KATAKANA LETTER SMALL TU;Lo;0;L;30C3;;;;N;;;;;
		0x30C3,
//FF70;HALFWIDTH KATAKANA-HIRAGANA PROLONGED SOUND MARK;So;0;L;30FC;;;;N;;;;;
		0x30FC,
//FF71;HALFWIDTH KATAKANA LETTER A;Lo;0;L;30A2;;;;N;;;;;
		0x30A2,
//FF72;HALFWIDTH KATAKANA LETTER I;Lo;0;L;30A4;;;;N;;;;;
		0x30A4,
//FF73;HALFWIDTH KATAKANA LETTER U;Lo;0;L;30A6;;;;N;;;;;
		0x30A6,
//FF74;HALFWIDTH KATAKANA LETTER E;Lo;0;L;30A8;;;;N;;;;;
		0x30A8,
//FF75;HALFWIDTH KATAKANA LETTER O;Lo;0;L;30AA;;;;N;;;;;
		0x30AA,
//FF76;HALFWIDTH KATAKANA LETTER KA;Lo;0;L;30AB;;;;N;;;;;
		0x30AB,
//FF77;HALFWIDTH KATAKANA LETTER KI;Lo;0;L;30AD;;;;N;;;;;
		0x30AD,
//FF78;HALFWIDTH KATAKANA LETTER KU;Lo;0;L;30AF;;;;N;;;;;
		0x30AF,
//FF79;HALFWIDTH KATAKANA LETTER KE;Lo;0;L;30B1;;;;N;;;;;
		0x30B1,
//FF7A;HALFWIDTH KATAKANA LETTER KO;Lo;0;L;30B3;;;;N;;;;;
		0x30B3,
//FF7B;HALFWIDTH KATAKANA LETTER SA;Lo;0;L;30B5;;;;N;;;;;
		0x30B5,
//FF7C;HALFWIDTH KATAKANA LETTER SI;Lo;0;L;30B7;;;;N;;;;;
		0x30B7,
//FF7D;HALFWIDTH KATAKANA LETTER SU;Lo;0;L;30B9;;;;N;;;;;
		0x30B9,
//FF7E;HALFWIDTH KATAKANA LETTER SE;Lo;0;L;30BB;;;;N;;;;;
		0x30BB,
//FF7F;HALFWIDTH KATAKANA LETTER SO;Lo;0;L;30BD;;;;N;;;;;
		0x30BD,
//FF80;HALFWIDTH KATAKANA LETTER TA;Lo;0;L;30BF;;;;N;;;;;
		0x30BF,
//FF81;HALFWIDTH KATAKANA LETTER TI;Lo;0;L;30C1;;;;N;;;;;
		0x30C1,
//FF82;HALFWIDTH KATAKANA LETTER TU;Lo;0;L;30C4;;;;N;;;;;
		0x30C4,
//FF83;HALFWIDTH KATAKANA LETTER TE;Lo;0;L;30C6;;;;N;;;;;
		0x30C6,
//FF84;HALFWIDTH KATAKANA LETTER TO;Lo;0;L;30C8;;;;N;;;;;
		0x30C8,
//FF85;HALFWIDTH KATAKANA LETTER NA;Lo;0;L;30CA;;;;N;;;;;
		0x30CA,
//FF86;HALFWIDTH KATAKANA LETTER NI;Lo;0;L;30CB;;;;N;;;;;
		0x30CB,
//FF87;HALFWIDTH KATAKANA LETTER NU;Lo;0;L;30CC;;;;N;;;;;
		0x30CC,
//FF88;HALFWIDTH KATAKANA LETTER NE;Lo;0;L;30CD;;;;N;;;;;
		0x30CD,
//FF89;HALFWIDTH KATAKANA LETTER NO;Lo;0;L;30CE;;;;N;;;;;
		0x30CE,
//FF8A;HALFWIDTH KATAKANA LETTER HA;Lo;0;L;30CF;;;;N;;;;;
		0x30CF,
//FF8B;HALFWIDTH KATAKANA LETTER HI;Lo;0;L;30D2;;;;N;;;;;
		0x30D2,
//FF8C;HALFWIDTH KATAKANA LETTER HU;Lo;0;L;30D5;;;;N;;;;;
		0x30D5,
//FF8D;HALFWIDTH KATAKANA LETTER HE;Lo;0;L;30D8;;;;N;;;;;
		0x30D8,
//FF8E;HALFWIDTH KATAKANA LETTER HO;Lo;0;L;30DB;;;;N;;;;;
		0x30DB,
//FF8F;HALFWIDTH KATAKANA LETTER MA;Lo;0;L;30DE;;;;N;;;;;
		0x30DE,
//FF90;HALFWIDTH KATAKANA LETTER MI;Lo;0;L;30DF;;;;N;;;;;
		0x30DF,
//FF91;HALFWIDTH KATAKANA LETTER MU;Lo;0;L;30E0;;;;N;;;;;
		0x30E0,
//FF92;HALFWIDTH KATAKANA LETTER ME;Lo;0;L;30E1;;;;N;;;;;
		0x30E1,
//FF93;HALFWIDTH KATAKANA LETTER MO;Lo;0;L;30E2;;;;N;;;;;
		0x30E2,
//FF94;HALFWIDTH KATAKANA LETTER YA;Lo;0;L;30E4;;;;N;;;;;
		0x30E4,
//FF95;HALFWIDTH KATAKANA LETTER YU;Lo;0;L;30E6;;;;N;;;;;
		0x30E6,
//FF96;HALFWIDTH KATAKANA LETTER YO;Lo;0;L;30E8;;;;N;;;;;
		0x30E8,
//FF97;HALFWIDTH KATAKANA LETTER RA;Lo;0;L;30E9;;;;N;;;;;
		0x30E9,
//FF98;HALFWIDTH KATAKANA LETTER RI;Lo;0;L;30EA;;;;N;;;;;
		0x30EA,
//FF99;HALFWIDTH KATAKANA LETTER RU;Lo;0;L;30EB;;;;N;;;;;
		0x30EB,
//FF9A;HALFWIDTH KATAKANA LETTER RE;Lo;0;L;30EC;;;;N;;;;;
		0x30EC,
//FF9B;HALFWIDTH KATAKANA LETTER RO;Lo;0;L;30ED;;;;N;;;;;
		0x30ED,
//FF9C;HALFWIDTH KATAKANA LETTER WA;Lo;0;L;30EF;;;;N;;;;;
		0x30EF,
//FF9D;HALFWIDTH KATAKANA LETTER N;Lo;0;L;30F3;;;;N;;;;;
		0x30F3,
//FF9E;HALFWIDTH KATAKANA VOICED SOUND MARK;So;0;L;309B;;;;N;;;;;
		0x309B,
//FF9F;HALFWIDTH KATAKANA SEMI-VOICED SOUND MARK;So;0;L;309C;;;;N;;;;;
		0x309C
	};

	// 濁点をつけられるひらがな・カタカナか
	ModUnicodeChar _voiced[] =
	{
//3040
		Common::UnicodeChar::usNull,
//3041;HIRAGANA LETTER SMALL A;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3042;HIRAGANA LETTER A;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3043;HIRAGANA LETTER SMALL I;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3044;HIRAGANA LETTER I;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3045;HIRAGANA LETTER SMALL U;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3046;HIRAGANA LETTER U;Lo;0;L;;;;;N;;;;;
		0x3094,
//3047;HIRAGANA LETTER SMALL E;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3048;HIRAGANA LETTER E;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3049;HIRAGANA LETTER SMALL O;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//304A;HIRAGANA LETTER O;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//304B;HIRAGANA LETTER KA;Lo;0;L;;;;;N;;;;;
		0x304C,
//304C;HIRAGANA LETTER GA;Lo;0;L;304B 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//304D;HIRAGANA LETTER KI;Lo;0;L;;;;;N;;;;;
		0x304E,
//304E;HIRAGANA LETTER GI;Lo;0;L;304D 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//304F;HIRAGANA LETTER KU;Lo;0;L;;;;;N;;;;;
		0x3050,
//3050;HIRAGANA LETTER GU;Lo;0;L;304F 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3051;HIRAGANA LETTER KE;Lo;0;L;;;;;N;;;;;
		0x3052,
//3052;HIRAGANA LETTER GE;Lo;0;L;3051 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3053;HIRAGANA LETTER KO;Lo;0;L;;;;;N;;;;;
		0x3054,
//3054;HIRAGANA LETTER GO;Lo;0;L;3053 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3055;HIRAGANA LETTER SA;Lo;0;L;;;;;N;;;;;
		0x3056,
//3056;HIRAGANA LETTER ZA;Lo;0;L;3055 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3057;HIRAGANA LETTER SI;Lo;0;L;;;;;N;;;;;
		0x3058,
//3058;HIRAGANA LETTER ZI;Lo;0;L;3057 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3059;HIRAGANA LETTER SU;Lo;0;L;;;;;N;;;;;
		0x305A,
//305A;HIRAGANA LETTER ZU;Lo;0;L;3059 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//305B;HIRAGANA LETTER SE;Lo;0;L;;;;;N;;;;;
		0x305C,
//305C;HIRAGANA LETTER ZE;Lo;0;L;305B 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//305D;HIRAGANA LETTER SO;Lo;0;L;;;;;N;;;;;
		0x305E,
//305E;HIRAGANA LETTER ZO;Lo;0;L;305D 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//305F;HIRAGANA LETTER TA;Lo;0;L;;;;;N;;;;;
		0x3060,
//3060;HIRAGANA LETTER DA;Lo;0;L;305F 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3061;HIRAGANA LETTER TI;Lo;0;L;;;;;N;;;;;
		0x3062,
//3062;HIRAGANA LETTER DI;Lo;0;L;3061 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3063;HIRAGANA LETTER SMALL TU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3064;HIRAGANA LETTER TU;Lo;0;L;;;;;N;;;;;
		0x3065,
//3065;HIRAGANA LETTER DU;Lo;0;L;3064 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3066;HIRAGANA LETTER TE;Lo;0;L;;;;;N;;;;;
		0x3067,
//3067;HIRAGANA LETTER DE;Lo;0;L;3066 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3068;HIRAGANA LETTER TO;Lo;0;L;;;;;N;;;;;
		0x3069,
//3069;HIRAGANA LETTER DO;Lo;0;L;3068 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//306A;HIRAGANA LETTER NA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//306B;HIRAGANA LETTER NI;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//306C;HIRAGANA LETTER NU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//306D;HIRAGANA LETTER NE;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//306E;HIRAGANA LETTER NO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//306F;HIRAGANA LETTER HA;Lo;0;L;;;;;N;;;;;
		0x3070,
//3070;HIRAGANA LETTER BA;Lo;0;L;306F 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3071;HIRAGANA LETTER PA;Lo;0;L;306F 309A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3072;HIRAGANA LETTER HI;Lo;0;L;;;;;N;;;;;
		0x3073,
//3073;HIRAGANA LETTER BI;Lo;0;L;3072 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3074;HIRAGANA LETTER PI;Lo;0;L;3072 309A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3075;HIRAGANA LETTER HU;Lo;0;L;;;;;N;;;;;
		0x3076,
//3076;HIRAGANA LETTER BU;Lo;0;L;3075 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3077;HIRAGANA LETTER PU;Lo;0;L;3075 309A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3078;HIRAGANA LETTER HE;Lo;0;L;;;;;N;;;;;
		0x3079,
//3079;HIRAGANA LETTER BE;Lo;0;L;3078 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//307A;HIRAGANA LETTER PE;Lo;0;L;3078 309A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//307B;HIRAGANA LETTER HO;Lo;0;L;;;;;N;;;;;
		0x307C,
//307C;HIRAGANA LETTER BO;Lo;0;L;307B 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//307D;HIRAGANA LETTER PO;Lo;0;L;307B 309A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//307E;HIRAGANA LETTER MA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//307F;HIRAGANA LETTER MI;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3080;HIRAGANA LETTER MU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3081;HIRAGANA LETTER ME;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3082;HIRAGANA LETTER MO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3083;HIRAGANA LETTER SMALL YA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3084;HIRAGANA LETTER YA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3085;HIRAGANA LETTER SMALL YU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3086;HIRAGANA LETTER YU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3087;HIRAGANA LETTER SMALL YO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3088;HIRAGANA LETTER YO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3089;HIRAGANA LETTER RA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//308A;HIRAGANA LETTER RI;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//308B;HIRAGANA LETTER RU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//308C;HIRAGANA LETTER RE;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//308D;HIRAGANA LETTER RO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//308E;HIRAGANA LETTER SMALL WA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//308F;HIRAGANA LETTER WA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3090;HIRAGANA LETTER WI;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3091;HIRAGANA LETTER WE;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3092;HIRAGANA LETTER WO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3093;HIRAGANA LETTER N;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3094;HIRAGANA LETTER VU;Lo;0;L;3046 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3095;HIRAGANA LETTER SMALL KA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3096;HIRAGANA LETTER SMALL KE;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3097
		Common::UnicodeChar::usNull,
//3098
		Common::UnicodeChar::usNull,
//3099;COMBINING KATAKANA-HIRAGANA VOICED SOUND MARK;Mn;8;L;;;;;N;NON-SPACING KATAKANA-HIRAGANA VOICED SOUND MARK;;;;
		Common::UnicodeChar::usNull,
//309A;COMBINING KATAKANA-HIRAGANA SEMI-VOICED SOUND MARK;Mn;8;L;;;;;N;NON-SPACING KATAKANA-HIRAGANA SEMI-VOICED SOUND MARK;;;;
		Common::UnicodeChar::usNull,
//309B;KATAKANA-HIRAGANA VOICED SOUND MARK;Lm;0;L;0020 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//309C;KATAKANA-HIRAGANA SEMI-VOICED SOUND MARK;Lm;0;L;0020 309A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//309D;HIRAGANA ITERATION MARK;Lm;0;L;;;;;N;;;;;
		0x309E,
//309E;HIRAGANA VOICED ITERATION MARK;Lm;0;L;309D 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//309F;HIRAGANA DIGRAPH YORI;Lo;0;L;<vertical> 3088 308A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30A0;KATAKANA-HIRAGANA DOUBLE HYPHEN;Pd;0;ON;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30A1;KATAKANA LETTER SMALL A;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30A2;KATAKANA LETTER A;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30A3;KATAKANA LETTER SMALL I;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30A4;KATAKANA LETTER I;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30A5;KATAKANA LETTER SMALL U;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30A6;KATAKANA LETTER U;Lo;0;L;;;;;N;;;;;
		0x30F4,
//30A7;KATAKANA LETTER SMALL E;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30A8;KATAKANA LETTER E;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30A9;KATAKANA LETTER SMALL O;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30AA;KATAKANA LETTER O;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30AB;KATAKANA LETTER KA;Lo;0;L;;;;;N;;;;;
		0x30AC,
//30AC;KATAKANA LETTER GA;Lo;0;L;30AB 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30AD;KATAKANA LETTER KI;Lo;0;L;;;;;N;;;;;
		0x30AE,
//30AE;KATAKANA LETTER GI;Lo;0;L;30AD 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30AF;KATAKANA LETTER KU;Lo;0;L;;;;;N;;;;;
		0x30B0,
//30B0;KATAKANA LETTER GU;Lo;0;L;30AF 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30B1;KATAKANA LETTER KE;Lo;0;L;;;;;N;;;;;
		0x30B2,
//30B2;KATAKANA LETTER GE;Lo;0;L;30B1 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30B3;KATAKANA LETTER KO;Lo;0;L;;;;;N;;;;;
		0x30B4,
//30B4;KATAKANA LETTER GO;Lo;0;L;30B3 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30B5;KATAKANA LETTER SA;Lo;0;L;;;;;N;;;;;
		0x30B6,
//30B6;KATAKANA LETTER ZA;Lo;0;L;30B5 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30B7;KATAKANA LETTER SI;Lo;0;L;;;;;N;;;;;
		0x30B8,
//30B8;KATAKANA LETTER ZI;Lo;0;L;30B7 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30B9;KATAKANA LETTER SU;Lo;0;L;;;;;N;;;;;
		0x30BA,
//30BA;KATAKANA LETTER ZU;Lo;0;L;30B9 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30BB;KATAKANA LETTER SE;Lo;0;L;;;;;N;;;;;
		0x30BC,
//30BC;KATAKANA LETTER ZE;Lo;0;L;30BB 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30BD;KATAKANA LETTER SO;Lo;0;L;;;;;N;;;;;
		0x30BE,
//30BE;KATAKANA LETTER ZO;Lo;0;L;30BD 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30BF;KATAKANA LETTER TA;Lo;0;L;;;;;N;;;;;
		0x30C0,
//30C0;KATAKANA LETTER DA;Lo;0;L;30BF 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30C1;KATAKANA LETTER TI;Lo;0;L;;;;;N;;;;;
		0x30C2,
//30C2;KATAKANA LETTER DI;Lo;0;L;30C1 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30C3;KATAKANA LETTER SMALL TU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30C4;KATAKANA LETTER TU;Lo;0;L;;;;;N;;;;;
		0x30C5,
//30C5;KATAKANA LETTER DU;Lo;0;L;30C4 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30C6;KATAKANA LETTER TE;Lo;0;L;;;;;N;;;;;
		0x30C7,
//30C7;KATAKANA LETTER DE;Lo;0;L;30C6 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30C8;KATAKANA LETTER TO;Lo;0;L;;;;;N;;;;;
		0x30C9,
//30C9;KATAKANA LETTER DO;Lo;0;L;30C8 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30CA;KATAKANA LETTER NA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30CB;KATAKANA LETTER NI;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30CC;KATAKANA LETTER NU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30CD;KATAKANA LETTER NE;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30CE;KATAKANA LETTER NO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30CF;KATAKANA LETTER HA;Lo;0;L;;;;;N;;;;;
		0x30D0,
//30D0;KATAKANA LETTER BA;Lo;0;L;30CF 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30D1;KATAKANA LETTER PA;Lo;0;L;30CF 309A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30D2;KATAKANA LETTER HI;Lo;0;L;;;;;N;;;;;
		0x30D3,
//30D3;KATAKANA LETTER BI;Lo;0;L;30D2 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30D4;KATAKANA LETTER PI;Lo;0;L;30D2 309A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30D5;KATAKANA LETTER HU;Lo;0;L;;;;;N;;;;;
		0x30D6,
//30D6;KATAKANA LETTER BU;Lo;0;L;30D5 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30D7;KATAKANA LETTER PU;Lo;0;L;30D5 309A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30D8;KATAKANA LETTER HE;Lo;0;L;;;;;N;;;;;
		0x30D9,
//30D9;KATAKANA LETTER BE;Lo;0;L;30D8 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30DA;KATAKANA LETTER PE;Lo;0;L;30D8 309A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30DB;KATAKANA LETTER HO;Lo;0;L;;;;;N;;;;;
		0x30DC,
//30DC;KATAKANA LETTER BO;Lo;0;L;30DB 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30DD;KATAKANA LETTER PO;Lo;0;L;30DB 309A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30DE;KATAKANA LETTER MA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30DF;KATAKANA LETTER MI;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30E0;KATAKANA LETTER MU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30E1;KATAKANA LETTER ME;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30E2;KATAKANA LETTER MO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30E3;KATAKANA LETTER SMALL YA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30E4;KATAKANA LETTER YA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30E5;KATAKANA LETTER SMALL YU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30E6;KATAKANA LETTER YU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30E7;KATAKANA LETTER SMALL YO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30E8;KATAKANA LETTER YO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30E9;KATAKANA LETTER RA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30EA;KATAKANA LETTER RI;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30EB;KATAKANA LETTER RU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30EC;KATAKANA LETTER RE;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30ED;KATAKANA LETTER RO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30EE;KATAKANA LETTER SMALL WA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30EF;KATAKANA LETTER WA;Lo;0;L;;;;;N;;;;;
		0x30F7,
//30F0;KATAKANA LETTER WI;Lo;0;L;;;;;N;;;;;
		0x30F8,
//30F1;KATAKANA LETTER WE;Lo;0;L;;;;;N;;;;;
		0x30F9,
//30F2;KATAKANA LETTER WO;Lo;0;L;;;;;N;;;;;
		0x30FA,
//30F3;KATAKANA LETTER N;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30F4;KATAKANA LETTER VU;Lo;0;L;30A6 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30F5;KATAKANA LETTER SMALL KA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30F6;KATAKANA LETTER SMALL KE;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30F7;KATAKANA LETTER VA;Lo;0;L;30EF 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30F8;KATAKANA LETTER VI;Lo;0;L;30F0 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30F9;KATAKANA LETTER VE;Lo;0;L;30F1 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30FA;KATAKANA LETTER VO;Lo;0;L;30F2 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30FB;KATAKANA MIDDLE DOT;Po;0;L;00B7;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30FC;KATAKANA-HIRAGANA PROLONGED SOUND MARK;Lm;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30FD;KATAKANA ITERATION MARK;Lm;0;L;;;;;N;;;;;
		0x30FE,
//30FE;KATAKANA VOICED ITERATION MARK;Lm;0;L;30FD 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30FF;KATAKANA DIGRAPH KOTO;Lo;0;L;<vertical> 30B3 30C8;;;;N;;;;;
		Common::UnicodeChar::usNull
	};
	// 半濁点をつけられるひらがな・カタカナか
	ModUnicodeChar _semiVoiced[] =
	{
//3040
		Common::UnicodeChar::usNull,
//3041;HIRAGANA LETTER SMALL A;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3042;HIRAGANA LETTER A;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3043;HIRAGANA LETTER SMALL I;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3044;HIRAGANA LETTER I;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3045;HIRAGANA LETTER SMALL U;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3046;HIRAGANA LETTER U;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3047;HIRAGANA LETTER SMALL E;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3048;HIRAGANA LETTER E;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3049;HIRAGANA LETTER SMALL O;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//304A;HIRAGANA LETTER O;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//304B;HIRAGANA LETTER KA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//304C;HIRAGANA LETTER GA;Lo;0;L;304B 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//304D;HIRAGANA LETTER KI;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//304E;HIRAGANA LETTER GI;Lo;0;L;304D 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//304F;HIRAGANA LETTER KU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3050;HIRAGANA LETTER GU;Lo;0;L;304F 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3051;HIRAGANA LETTER KE;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3052;HIRAGANA LETTER GE;Lo;0;L;3051 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3053;HIRAGANA LETTER KO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3054;HIRAGANA LETTER GO;Lo;0;L;3053 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3055;HIRAGANA LETTER SA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3056;HIRAGANA LETTER ZA;Lo;0;L;3055 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3057;HIRAGANA LETTER SI;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3058;HIRAGANA LETTER ZI;Lo;0;L;3057 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3059;HIRAGANA LETTER SU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//305A;HIRAGANA LETTER ZU;Lo;0;L;3059 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//305B;HIRAGANA LETTER SE;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//305C;HIRAGANA LETTER ZE;Lo;0;L;305B 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//305D;HIRAGANA LETTER SO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//305E;HIRAGANA LETTER ZO;Lo;0;L;305D 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//305F;HIRAGANA LETTER TA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3060;HIRAGANA LETTER DA;Lo;0;L;305F 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3061;HIRAGANA LETTER TI;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3062;HIRAGANA LETTER DI;Lo;0;L;3061 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3063;HIRAGANA LETTER SMALL TU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3064;HIRAGANA LETTER TU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3065;HIRAGANA LETTER DU;Lo;0;L;3064 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3066;HIRAGANA LETTER TE;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3067;HIRAGANA LETTER DE;Lo;0;L;3066 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3068;HIRAGANA LETTER TO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3069;HIRAGANA LETTER DO;Lo;0;L;3068 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//306A;HIRAGANA LETTER NA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//306B;HIRAGANA LETTER NI;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//306C;HIRAGANA LETTER NU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//306D;HIRAGANA LETTER NE;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//306E;HIRAGANA LETTER NO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//306F;HIRAGANA LETTER HA;Lo;0;L;;;;;N;;;;;
		0x3071,
//3070;HIRAGANA LETTER BA;Lo;0;L;306F 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3071;HIRAGANA LETTER PA;Lo;0;L;306F 309A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3072;HIRAGANA LETTER HI;Lo;0;L;;;;;N;;;;;
		0x3074,
//3073;HIRAGANA LETTER BI;Lo;0;L;3072 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3074;HIRAGANA LETTER PI;Lo;0;L;3072 309A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3075;HIRAGANA LETTER HU;Lo;0;L;;;;;N;;;;;
		0x3077,
//3076;HIRAGANA LETTER BU;Lo;0;L;3075 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3077;HIRAGANA LETTER PU;Lo;0;L;3075 309A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3078;HIRAGANA LETTER HE;Lo;0;L;;;;;N;;;;;
		0x307A,
//3079;HIRAGANA LETTER BE;Lo;0;L;3078 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//307A;HIRAGANA LETTER PE;Lo;0;L;3078 309A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//307B;HIRAGANA LETTER HO;Lo;0;L;;;;;N;;;;;
		0x307D,
//307C;HIRAGANA LETTER BO;Lo;0;L;307B 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//307D;HIRAGANA LETTER PO;Lo;0;L;307B 309A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//307E;HIRAGANA LETTER MA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//307F;HIRAGANA LETTER MI;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3080;HIRAGANA LETTER MU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3081;HIRAGANA LETTER ME;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3082;HIRAGANA LETTER MO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3083;HIRAGANA LETTER SMALL YA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3084;HIRAGANA LETTER YA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3085;HIRAGANA LETTER SMALL YU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3086;HIRAGANA LETTER YU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3087;HIRAGANA LETTER SMALL YO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3088;HIRAGANA LETTER YO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3089;HIRAGANA LETTER RA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//308A;HIRAGANA LETTER RI;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//308B;HIRAGANA LETTER RU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//308C;HIRAGANA LETTER RE;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//308D;HIRAGANA LETTER RO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//308E;HIRAGANA LETTER SMALL WA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//308F;HIRAGANA LETTER WA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3090;HIRAGANA LETTER WI;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3091;HIRAGANA LETTER WE;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3092;HIRAGANA LETTER WO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3093;HIRAGANA LETTER N;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3094;HIRAGANA LETTER VU;Lo;0;L;3046 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3095;HIRAGANA LETTER SMALL KA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3096;HIRAGANA LETTER SMALL KE;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//3097
		Common::UnicodeChar::usNull,
//3098
		Common::UnicodeChar::usNull,
//3099;COMBINING KATAKANA-HIRAGANA VOICED SOUND MARK;Mn;8;L;;;;;N;NON-SPACING KATAKANA-HIRAGANA VOICED SOUND MARK;;;;
		Common::UnicodeChar::usNull,
//309A;COMBINING KATAKANA-HIRAGANA SEMI-VOICED SOUND MARK;Mn;8;L;;;;;N;NON-SPACING KATAKANA-HIRAGANA SEMI-VOICED SOUND MARK;;;;
		Common::UnicodeChar::usNull,
//309B;KATAKANA-HIRAGANA VOICED SOUND MARK;Lm;0;L;0020 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//309C;KATAKANA-HIRAGANA SEMI-VOICED SOUND MARK;Lm;0;L;0020 309A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//309D;HIRAGANA ITERATION MARK;Lm;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//309E;HIRAGANA VOICED ITERATION MARK;Lm;0;L;309D 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//309F;HIRAGANA DIGRAPH YORI;Lo;0;L;<vertical> 3088 308A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30A0;KATAKANA-HIRAGANA DOUBLE HYPHEN;Pd;0;ON;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30A1;KATAKANA LETTER SMALL A;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30A2;KATAKANA LETTER A;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30A3;KATAKANA LETTER SMALL I;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30A4;KATAKANA LETTER I;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30A5;KATAKANA LETTER SMALL U;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30A6;KATAKANA LETTER U;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30A7;KATAKANA LETTER SMALL E;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30A8;KATAKANA LETTER E;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30A9;KATAKANA LETTER SMALL O;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30AA;KATAKANA LETTER O;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30AB;KATAKANA LETTER KA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30AC;KATAKANA LETTER GA;Lo;0;L;30AB 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30AD;KATAKANA LETTER KI;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30AE;KATAKANA LETTER GI;Lo;0;L;30AD 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30AF;KATAKANA LETTER KU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30B0;KATAKANA LETTER GU;Lo;0;L;30AF 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30B1;KATAKANA LETTER KE;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30B2;KATAKANA LETTER GE;Lo;0;L;30B1 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30B3;KATAKANA LETTER KO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30B4;KATAKANA LETTER GO;Lo;0;L;30B3 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30B5;KATAKANA LETTER SA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30B6;KATAKANA LETTER ZA;Lo;0;L;30B5 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30B7;KATAKANA LETTER SI;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30B8;KATAKANA LETTER ZI;Lo;0;L;30B7 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30B9;KATAKANA LETTER SU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30BA;KATAKANA LETTER ZU;Lo;0;L;30B9 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30BB;KATAKANA LETTER SE;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30BC;KATAKANA LETTER ZE;Lo;0;L;30BB 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30BD;KATAKANA LETTER SO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30BE;KATAKANA LETTER ZO;Lo;0;L;30BD 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30BF;KATAKANA LETTER TA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30C0;KATAKANA LETTER DA;Lo;0;L;30BF 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30C1;KATAKANA LETTER TI;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30C2;KATAKANA LETTER DI;Lo;0;L;30C1 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30C3;KATAKANA LETTER SMALL TU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30C4;KATAKANA LETTER TU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30C5;KATAKANA LETTER DU;Lo;0;L;30C4 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30C6;KATAKANA LETTER TE;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30C7;KATAKANA LETTER DE;Lo;0;L;30C6 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30C8;KATAKANA LETTER TO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30C9;KATAKANA LETTER DO;Lo;0;L;30C8 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30CA;KATAKANA LETTER NA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30CB;KATAKANA LETTER NI;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30CC;KATAKANA LETTER NU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30CD;KATAKANA LETTER NE;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30CE;KATAKANA LETTER NO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30CF;KATAKANA LETTER HA;Lo;0;L;;;;;N;;;;;
		0x30D1,
//30D0;KATAKANA LETTER BA;Lo;0;L;30CF 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30D1;KATAKANA LETTER PA;Lo;0;L;30CF 309A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30D2;KATAKANA LETTER HI;Lo;0;L;;;;;N;;;;;
		0x30D4,
//30D3;KATAKANA LETTER BI;Lo;0;L;30D2 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30D4;KATAKANA LETTER PI;Lo;0;L;30D2 309A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30D5;KATAKANA LETTER HU;Lo;0;L;;;;;N;;;;;
		0x30D7,
//30D6;KATAKANA LETTER BU;Lo;0;L;30D5 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30D7;KATAKANA LETTER PU;Lo;0;L;30D5 309A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30D8;KATAKANA LETTER HE;Lo;0;L;;;;;N;;;;;
		0x30DA,
//30D9;KATAKANA LETTER BE;Lo;0;L;30D8 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30DA;KATAKANA LETTER PE;Lo;0;L;30D8 309A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30DB;KATAKANA LETTER HO;Lo;0;L;;;;;N;;;;;
		0x30DD,
//30DC;KATAKANA LETTER BO;Lo;0;L;30DB 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30DD;KATAKANA LETTER PO;Lo;0;L;30DB 309A;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30DE;KATAKANA LETTER MA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30DF;KATAKANA LETTER MI;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30E0;KATAKANA LETTER MU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30E1;KATAKANA LETTER ME;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30E2;KATAKANA LETTER MO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30E3;KATAKANA LETTER SMALL YA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30E4;KATAKANA LETTER YA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30E5;KATAKANA LETTER SMALL YU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30E6;KATAKANA LETTER YU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30E7;KATAKANA LETTER SMALL YO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30E8;KATAKANA LETTER YO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30E9;KATAKANA LETTER RA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30EA;KATAKANA LETTER RI;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30EB;KATAKANA LETTER RU;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30EC;KATAKANA LETTER RE;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30ED;KATAKANA LETTER RO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30EE;KATAKANA LETTER SMALL WA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30EF;KATAKANA LETTER WA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30F0;KATAKANA LETTER WI;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30F1;KATAKANA LETTER WE;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30F2;KATAKANA LETTER WO;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30F3;KATAKANA LETTER N;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30F4;KATAKANA LETTER VU;Lo;0;L;30A6 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30F5;KATAKANA LETTER SMALL KA;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30F6;KATAKANA LETTER SMALL KE;Lo;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30F7;KATAKANA LETTER VA;Lo;0;L;30EF 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30F8;KATAKANA LETTER VI;Lo;0;L;30F0 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30F9;KATAKANA LETTER VE;Lo;0;L;30F1 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30FA;KATAKANA LETTER VO;Lo;0;L;30F2 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30FB;KATAKANA MIDDLE DOT;Po;0;L;00B7;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30FC;KATAKANA-HIRAGANA PROLONGED SOUND MARK;Lm;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30FD;KATAKANA ITERATION MARK;Lm;0;L;;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30FE;KATAKANA VOICED ITERATION MARK;Lm;0;L;30FD 3099;;;;N;;;;;
		Common::UnicodeChar::usNull,
//30FF;KATAKANA DIGRAPH KOTO;Lo;0;L;<vertical> 30B3 30C8;;;;N;;;;;
		Common::UnicodeChar::usNull
	};
#endif //#ifndef USE_ICU
	
	// 与えられた文字列を正規化する
	void
	normalize(const ModUnicodeString& s,
			  CharTrait::NormalizingMethod::Value method,
			  ModUnicodeString& result,
			  Una::ResourceID::Value resourceID = Una::ResourceID::Unknown,
			  Boolean::Value bCarriage_ = Boolean::Unknown,
			  Boolean::Value bStemming_ = Boolean::Unknown,
			  Boolean::Value bDeleteSpace_ = Boolean::Unknown,
			  bool bWordSeparate_ = false);

	// 正規化方法の文字列表現
	ModUnicodeString _cBuiltIn("BuiltIn");
	ModUnicodeString _cBuiltIn_HK("BuiltIn_HK");
	ModUnicodeString _cUNA("UNA:");
	ModUnicodeString _cNORM("NORM:");

	// セパレータ
	ModUnicodeString _sep("/");

	// 文字列をパースし、正規化方法を取得する
	void
	parseNormalizingMethod(const ModUnicodeString& src,
						   CharTrait::NormalizingMethod::Value& method_,
						   Una::ResourceID::Value& resourceID_,
						   Boolean::Value& bCarriage_,
						   Boolean::Value& bStemming_,
						   Boolean::Value& bDeleteSpace_,
						   bool& bWordSeparate_);

	// ヒント
	ModUnicodeString _cCarriage("carriage");
	ModUnicodeString _cStemming("stemming");
	ModUnicodeString _cDeleteSpace("deletespace");
	ModUnicodeString _cWordSeparate("wordseparate");
	ModUnicodeString _cTrue("true");
	ModUnicodeString _cFalse("false");

	// ヒントをパースする
	void
	parseHint(const ModUnicodeChar* p,
			  Una::ResourceID::Value& resourceID_,
			  Boolean::Value& bCarriage_,
			  Boolean::Value& bStemming_,
			  Boolean::Value& bDeleteSpace_,
			  Boolean::Value& bWordSeparate_);

	// Target characters of escaping
	ModUnicodeChar _usFullwidthPercentSign =	0xFF05;	// ％
	ModUnicodeChar _usFullwidthLowLine =		0xFF3F;	// ＿
	
	// Internal escape character
	// The reason for using '*' is that
	// Btree2 has already used '\' for an internal escape character,
	// and needs that the character is one of ascii characters.
	// See Btree2::Condition::ParseValue::putStreamLikeValue() for details.
	ModUnicodeChar _usInternalEscapeCharacter = Common::UnicodeChar::usAsterisc;
	ModUnicodeChar _usPreNormalizedInternalEscapeCharacter = 0xFF0A;	// ＊
	
	// Escape target characters
	void
	escape(const ModUnicodeString& cstrSrc_,
		   CharTrait::EscapeTarget::Value eEscapeTarget_,
		   ModUnicodeChar& usEscapeCharacter_,
		   ModUnicodeString& cstrDst_);

} // namespace _StringData

//	FUNCTION
//	$$$::_StringData::normalize -- 与えられた文字列を正規化する
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeString&	s
//			正規化する文字列
//		CharTrait::NormalizingMethod::Value	method
//			正規化をどのような手段で行うかを表す
//		ModUnicodeString&	result
//			正規化された文字列が設定される
//		Una::ResourceID::Value resourceID
//			UNAのリソースID (defaultはUna::ResourceID::Unknown)
//		Boolean::Value bCarriage_
//			改行をまたがる正規化を行うかどうか(defaultはBoolean::Unknown)
//		Boolean::Value bStemming_
//			ステミングを行うかどうか(defaultはBoolean::Unknown)
//		Boolean::Value bDeleteSpace_
//			スペースを除去するかどうか(defaultはBoolean::Unknown)
//		bool bWordSeparate_
//			単語単位でセパレートするか(defaultはfalse)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//
void
_StringData::normalize(const ModUnicodeString& s,
					   CharTrait::NormalizingMethod::Value method,
					   ModUnicodeString& result,
					   Una::ResourceID::Value resourceID,
					   Boolean::Value bCarriage_,
					   Boolean::Value bStemming_,
					   Boolean::Value bDeleteSpace_,
					   bool bWordSeparate_)
{
	const ModUnicodeChar* p = s;
	const ModSize n = s.getLength();

	switch (method) {
	case CharTrait::NormalizingMethod::None:

		// 異表記正規化せず、渡された文字列をそのまま結果とする

		result.clear();
		result.append(p, n);
		break;

	case CharTrait::NormalizingMethod::BuiltIn:
	case CharTrait::NormalizingMethod::BuiltIn_HK:
	{
#ifdef USE_ICU
		// ICU を使用して、
		// 与えられたユニコード文字列を正規形 KC (NFKC)に正規化する

		icu::UnicodeString	src(p, n);
		icu::UnicodeString	dst;
		::UErrorCode		code = ::U_ZERO_ERROR;

		icu::Normalizer::normalize(src, UNORM_NFKC, 0, dst, code);

		if (::U_FAILURE(code))
			_TRMEISTER_THROW0(Exception::BadArgument);

		// 小文字があれば大文字に変換する

		dst.toUpper();

		// 正規化された結果を result に設定する

		if (dst.length() && dst.getTerminatedBuffer())
			result = dst.getTerminatedBuffer();
		else
			result.clear();
#else
		// 結果として返す文字列を初期化する

		result.clear();
		result.reallocate(n);

		for (ModSize i = 0; i < n; ) {

			// 大文字から小文字へ変換する

			ModUnicodeChar c0 = ModUnicodeCharTrait::toLower(p[i++]);

			if (c0 >= 0xFF01 && c0 <= 0xFF5E)

				// ASCII の範囲を全角から半角へ変換する

				c0 -= (0xFF01 - 0x0021);

			else {
				if (c0 >= 0xFF61 && c0 <= 0xFF9F)

					// 半角カタカナを全角に変換する

					c0 = _half2full[c0 - 0xFF61];

				if (c0 >= 0x3040 && c0 <= 0x30FF) {

					// 今調べている文字は全角ひらがな・カタカナである

					// 今調べている文字の次の文字を得る

					const ModUnicodeChar c1 =
						(i == n) ? Common::UnicodeChar::usNull : p[i];

					if (_voiced[c0 - 0x3040] &&
						(c1 == 0x3099 || c1 == 0x309B || c1 == 0xFF9E))

						// 今調べている文字が濁点と合字可能で、
						// 次の文字が濁点なので、合字を得る

						c0 = _voiced[c0 - 0x3040], ++i;

					else if (_semiVoiced[c0 - 0x3040] &&
							 (c1 == 0x309A || c1 == 0x309C || c1 == 0xFF9F))

						// 今調べている文字が半濁点と合字可能で、
						// 次の文字が半濁点なので、合字を得る

						c0 = _semiVoiced[c0 - 0x3040], ++i;
				}
					
				if (method == CharTrait::NormalizingMethod::BuiltIn_HK)
				{
					// itBox向け
						
					if ((c0 >= 0x3041 && c0 <= 0x3096)
						|| c0 == 0x309D || c0 == 0x309E)

						// ひらがな・カタカナの同一視を行うので、
						// カタカナにする
					
						c0 += (0x30A0 - 0x3040);

					else if (c0 == 0x3000)

						// 全角スペースを半角スペースにする

						c0 = 0x20;

				}
			}

			// 結果を設定する

			result.append(c0);
		}
#endif
		break;
	}
	case CharTrait::NormalizingMethod::UNA:
	case CharTrait::NormalizingMethod::NORM:
	{
		// UNA を使用して正規化する

		if (resourceID == Una::ResourceID::Unknown)
			resourceID = Una::Manager::getResourceForLikeOperator();

		ModAutoPointer<UNA::ModNlpAnalyzer> analyzer(
			Utility::Una::Manager::getModNlpAnalyzer(resourceID));
		; _TRMEISTER_ASSERT(analyzer.get());

		ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> >
			param;

		// パラメータを得る
		_UNA::getParameter(param, bCarriage_, bStemming_, bDeleteSpace_);

		// ModNlpAnalyzerの動作を指定するパラメータを設定する
		analyzer->prepare(param);

		// 正規化対象の文字列を設定する
		ModLanguageSet lang;
		_UNA::getLanguage(lang);
		analyzer->set(s, lang);

		// 結果として返す文字列を初期化する

		result.clear();
		result.reallocate(n);

		// 正規化結果を得る

		if (method == CharTrait::NormalizingMethod::UNA)
		{
			// UNA利用

			if (bWordSeparate_)
			{
				// 単語ごとにセパレーターを挟む

				ModUnicodeString word;
				while (analyzer->getWord(word) == ModTrue)
				{
					result.append(_sep);
					result.append(word);
				}

				if (result.getLength())
					result.append(_sep);
			}
			else
			{
				// 全体を一括して取得する

				analyzer->getWholeText(result);
			}
		}
		else

			// NORM利用
			analyzer->getNormalizeBuf(result);
		
		break;
	}
#ifdef DEBUG
	default:
		; _TRMEISTER_ASSERT(false);
#endif
	}
}

//
//	FUNCTION private
//	$$$::_StringData::parseNormalizingMethod -- 正規化方法をパースする
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeString&	src
//			正規化方法の文字列表現
//		CharTrait::NormalizingMethod::Value&	method
//			正規化をどのような手段で行うかを表す
//		Una::ResourceID::Value& resourceID
//			UNAのリソースID
//			正規化方法がUNAではない場合は、Una::ResourceID::Unknown
//
//	RETURN
//
//	EXCEPTIONS
//
void
_StringData::parseNormalizingMethod(
	const ModUnicodeString& src,
	CharTrait::NormalizingMethod::Value& method,
	Una::ResourceID::Value& resourceID,
	Boolean::Value& carriage_,
	Boolean::Value& stemming_,
	Boolean::Value& deletespace_,
	bool& separate_)
{
	// まずはデフォルトを設定する
	resourceID = Una::ResourceID::Unknown;
	carriage_ = stemming_ = deletespace_ = Boolean::Unknown;
	separate_ = false;
	
	if (ModUnicodeCharTrait::compare(src,
									 _cBuiltIn,
									 ModFalse) == 0)
	{
		method = CharTrait::NormalizingMethod::BuiltIn;
	}
	else if (ModUnicodeCharTrait::compare(src,
										  _cBuiltIn_HK,
										  ModFalse) == 0)
	{
		method = CharTrait::NormalizingMethod::BuiltIn_HK;
	}
	else if (ModUnicodeCharTrait::compare(src,
										  _cUNA,
										  ModFalse,
										  _cUNA.getLength()) == 0)
	{
		// UNAの場合は 'UNA:2' とかが指定されている
		
		method = CharTrait::NormalizingMethod::UNA;
		
		// UNAのリソースIDを取得する

		const ModUnicodeChar* p = src;
		p += _cUNA.getLength();
		Boolean::Value s = Boolean::Unknown;
		parseHint(p, resourceID, carriage_, stemming_, deletespace_, s);
		if (s == Boolean::True)
			separate_ = true;
	}
	else if (ModUnicodeCharTrait::compare(src,
										  _cNORM,
										  ModFalse,
										  _cNORM.getLength()) == 0)
	{
		// NORMの場合は 'NORM:2' とかが指定されている
		
		method = CharTrait::NormalizingMethod::NORM;
		
		// UNAのリソースIDを取得する

		const ModUnicodeChar* p = src;
		p += _cNORM.getLength();
		Boolean::Value s = Boolean::Unknown;
		parseHint(p, resourceID, carriage_, stemming_, deletespace_, s);

		if (carriage_ != Boolean::Unknown ||
			stemming_ != Boolean::Unknown ||
			deletespace_ != Boolean::Unknown ||
			s != Boolean::Unknown)

			_TRMEISTER_THROW0(Exception::NotSupported);
			
	}
	else
	{
		_TRMEISTER_THROW0(Exception::NotSupported);
	}
}

//
//	FUNCTION local
//	$$$::_StringData::parseHint -- ヒントをパースする
//
//	NOTES
//
//	ARGUMENTS
//		const ModUnicodeChar* p_
//			パースする文字列
//		Una::ResourceID::Value& resourceID_
//			リソースID
//		Boolean::Value& carriage_
//			改行を除去するかどうか
//		Boolean::Value& stemming_
//			ステミングを行うかどうか
//		Boolean::Value& deletespace_
//			スペースを除去するかどうか
//		Boolean::Value& separate_
//			単語境界でセパレートするかどうか
//
//	RETURN
//		なし
//
//	EXCPETIONS

void
_StringData::parseHint(const ModUnicodeChar* p_,
					   Una::ResourceID::Value& resourceID_,
					   Boolean::Value& carriage_,
					   Boolean::Value& stemming_,
					   Boolean::Value& deletespace_,
					   Boolean::Value& separate_)
{
	// ここに渡ってくる文字列は、
	//	"2 wordsperate stemming=false"
	// のような感じ
	
	// まず先頭は数字のはずなので、そのまま toInt する
	resourceID_ = ModUnicodeCharTrait::toInt(p_);

	while (*p_ != 0)
	{
		if (*p_ < '0' || *p_ > '9')
			break;
		++p_;
	}

	// スペースを読み飛ばす
	while (*p_ != 0 && *p_ == ' ') ++p_;

	if (*p_ == 0) return;
	
	// パースする文字列は以下のパターン
	//
	// <identifier> [ = { true | false } ]
	// <identifier> ::= { [A-Za-z][A-Za-z0-9]+ }
	//
	// <identifier>のみの場合は true であるとみなす

	const ModUnicodeChar* i = 0;
	
	// 状態を表す変数
	//
	//	0: <identifier>のパース中
	//	1: { true | false }のパース中
	//	2: 終了
	
	int s = 0;
	Boolean::Value* now = 0;

	for (;;)
	{
		if ((*p_ >= '0' && *p_ <= '9') ||
			(*p_ >= 'A' && *p_ <= 'Z') ||
			(*p_ >= 'a' && *p_ <= 'z'))

		{
			if (i == 0) i = p_;
		}
		else if (*p_ == '=')
		{
			if (s != 0)
				// エラー
				_TRMEISTER_THROW1(Exception::SQLSyntaxError, p_);

			if (i)
			{
				// 引数の選択
				if (ModUnicodeCharTrait::compare(
						i, _cCarriage, ModFalse,
						_cCarriage.getLength()) == 0)
					now = &carriage_;
				else if (ModUnicodeCharTrait::compare(
							 i, _cStemming, ModFalse,
							 _cStemming.getLength()) == 0)
					now = &stemming_;
				else if (ModUnicodeCharTrait::compare(
							 i, _cDeleteSpace, ModFalse,
							 _cDeleteSpace.getLength()) == 0)
					now = &deletespace_;
				else if (ModUnicodeCharTrait::compare(
							 i, _cWordSeparate, ModFalse,
							 _cWordSeparate.getLength()) == 0)
					now = &separate_;
				else
					_TRMEISTER_THROW1(Exception::SQLSyntaxError, i);

				i = 0;
			}
			s = 1;
		}
		else if (*p_ == ' ' || *p_ == 0)
		{
			if (i)
			{
				if (s == 0)
				{
					// 引数の選択
					if (ModUnicodeCharTrait::compare(
							i, _cCarriage, ModFalse,
							_cCarriage.getLength()) == 0)
						now = &carriage_;
					else if (ModUnicodeCharTrait::compare(
								 i, _cStemming, ModFalse,
								 _cStemming.getLength()) == 0)
						now = &stemming_;
					else if (ModUnicodeCharTrait::compare(
								 i, _cDeleteSpace, ModFalse,
								 _cDeleteSpace.getLength()) == 0)
						now = &deletespace_;
					else if (ModUnicodeCharTrait::compare(
								 i, _cWordSeparate, ModFalse,
								 _cWordSeparate.getLength()) == 0)
						now = &separate_;
					else
						_TRMEISTER_THROW1(Exception::SQLSyntaxError, i);

					// とりあえず TRUE にする
					*now = Boolean::True;
				}
				else if (s == 1)
				{
					// true or false
					if (ModUnicodeCharTrait::compare(
							i, _cTrue, ModFalse,
							_cTrue.getLength()) == 0)
						*now = Boolean::True;
					else if (ModUnicodeCharTrait::compare(
								 i, _cFalse, ModFalse,
								 _cFalse.getLength()) == 0)
						*now = Boolean::False;
					else
						_TRMEISTER_THROW1(Exception::SQLSyntaxError, i);
				}
			
				i = 0;
			}
			s = 0;
		}
		else
		{
			// エラー
			_TRMEISTER_THROW1(Exception::SQLSyntaxError, p_);
		}

		if (*p_ == 0)
			break;

		++p_;
	}

}

//
//	FUNCTION local
//	$$$::_StringData::escape -- 指定された文字をエスケープする
//
//	NOTES
//	None以外では、エスケープ文字の置き換えも実行される。
//	正規化によりパターンの意味が異なってしまうことを回避するため。
//
//	ARGUMENTS
//	const ModUnicodeString& cstrSrc_
//	CharTrait::EscapeTarget::Value eEscapeTarget_
//	ModUnicodeChar& usEscapeCharacter_
//		None以外の時に使われる。
//		[IN] エスケープ文字
//		[OUT] 置き換え後のエスケープ文字
//			usNullの場合、エスケープ対象の文字がなかったことを示す。
//	ModUnicodeString& cstrDst_
//
//	RETURN
//
//	EXCPETIONS
//
void
_StringData::escape(const ModUnicodeString& cstrSrc_,
					CharTrait::EscapeTarget::Value eEscapeTarget_,
					ModUnicodeChar& usEscapeCharacter_,
					ModUnicodeString& cstrDst_)
{
	switch (eEscapeTarget_)
	{
	case CharTrait::EscapeTarget::None:
	{
		// エスケープせず、渡された文字列をそのまま結果とする
		cstrDst_.clear();
		cstrDst_.append(cstrSrc_);
		break;
	}
	case CharTrait::EscapeTarget::FullwidthWildcard:
	{
		cstrDst_.clear();
		// All the characters of the cstrSrc_ may be target characters.
		cstrDst_.reallocate(2 * cstrSrc_.getLength());

		bool bEscape = false;
		const ModUnicodeChar* p = cstrSrc_;
		while (*p)
		{
			if (*p == usEscapeCharacter_)
			{
				bEscape = true;
				cstrDst_.append(_usInternalEscapeCharacter);
				if (*++p == 0)
				{
					break;
				}
			}
			else if (*p == _usInternalEscapeCharacter ||
					 *p == _usPreNormalizedInternalEscapeCharacter ||
					 *p == _usFullwidthPercentSign ||
					 *p == _usFullwidthLowLine)
			{
				bEscape = true;
				cstrDst_.append(_usInternalEscapeCharacter);
			}
			cstrDst_.append(*p++);
		}
		usEscapeCharacter_ = (bEscape == true) ?
			_usInternalEscapeCharacter : Common::UnicodeChar::usNull;
		break;
	}
	default:
	{
		; _TRMEISTER_ASSERT(false);
	}
	}
}

} // namespace $$

//
//	FUNCTION pubic
//	Utility::CharTrait::normalize -- 正規化する
//
//	NOTES
//	保持している文字列を正規化する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
CharTrait::normalize(Common::StringData* pStringData_,
					 NormalizingMethod::Value eMethod_,
					 Una::ResourceID::Value eResourceID_)
{
	ModUnicodeString temp;
	_StringData::normalize(
		pStringData_->getValue(), eMethod_, temp, eResourceID_);
	pStringData_->setValue(temp);
}

//
//	FUNCTION pubic
//	Utility::CharTrait::normalize -- 正規化する
//
//	NOTES
//	保持している文字列を正規化する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
CharTrait::normalize(const ModUnicodeString& src_,
					 NormalizingMethod::Value eMethod_,
					 ModUnicodeString& dst_,
					 Una::ResourceID::Value eResourceID_)
{
	_StringData::normalize(src_, eMethod_, dst_, eResourceID_);
}

//
//	FUNCTION pubic
//	Utility::CharTrait::normalize -- 正規化する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& src_
//		正規化対象の文字列
//	const ModUnicodeString& method_
//		正規化方法
//	ModUnicodeString& dst_
//		正規化後の文字列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//static
void
CharTrait::normalize(const ModUnicodeString& src_,
					 const ModUnicodeString& cstrMethod_,
					 ModUnicodeString& dst_)
{
	NormalizingMethod::Value m;
	Una::ResourceID::Value r;
	Boolean::Value carriage;
	Boolean::Value stemming;
	Boolean::Value deletespace;
	bool separate;
	_StringData::parseNormalizingMethod(cstrMethod_, m, r,
										carriage,
										stemming,
										deletespace,
										separate);
	_StringData::normalize(src_, m, dst_, r,
						   carriage,
						   stemming,
						   deletespace,
						   separate);
}

//
//	FUNCTION pubic
//	Utility::CharTrait::expandSynonym -- 同義語展開する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& src_
//		対象の単語
//	const ModUnicodeString& method_
//		展開方法
//	ModVector<ModUnicodeString>& dst_
//		展開した語(ソースも含む)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//static
void
CharTrait::expandSynonym(const ModUnicodeString& src_,
						 const ModUnicodeString& cstrMethod_,
						 ModVector<ModUnicodeString>& dst_)
{
	NormalizingMethod::Value m;
	Una::ResourceID::Value r;
	Boolean::Value carriage;
	Boolean::Value stemming;
	Boolean::Value deletespace;
	bool separate;
	_StringData::parseNormalizingMethod(cstrMethod_, m, r,
										carriage,
										stemming,
										deletespace,
										separate);

	if (m != NormalizingMethod::UNA)
	{
		_TRMEISTER_THROW0(Exception::NotSupported);
	}

	// アナライザーを得る
	ModAutoPointer<UNA::ModNlpAnalyzer> analyzer(
		Utility::Una::Manager::getModNlpAnalyzer(r));
	; _TRMEISTER_ASSERT(analyzer.get());

	ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> >
		param;

	// パラメータを得る
	_UNA::getParameter(param, carriage, stemming, deletespace);
	
	// ModNlpAnalyzerの動作を指定するパラメータを設定する
	analyzer->prepare(param);

	// 展開対象の文字列を設定する
	ModLanguageSet lang;
	_UNA::getLanguage(lang);

	// 展開する
	expandSynonym(analyzer.get(),
				  src_,
				  lang,
				  dst_);
}

//static
void
CharTrait::expandSynonym(UNA::ModNlpAnalyzer* analyzer_,
						 const ModUnicodeString& src_,
						 const ModLanguageSet& lang_,
						 ModVector<ModUnicodeString>& dst_)
{
	// 転置から呼び出される部分を切り出した
	
	analyzer_->set(src_, lang_);

	// 展開する
	UNA::ModNlpAnalyzer::ExpandResult r;
	analyzer_->getExpandStrings(r, dst_);
	if (r == UNA::ModNlpAnalyzer::ExpandResultMaxExpTargetStrLen)
	{
		dst_.clear();
		_TRMEISTER_THROW0(Exception::TooLongConditionalPattern);
	}
	else if (r == UNA::ModNlpAnalyzer::ExpandResultMaxExpPatternNum)
	{
		dst_.clear();
		_TRMEISTER_THROW0(Exception::TooManyExpandedPattern);
	}

	// 展開成功
}

//
//	FUNCTION public
//	Utility::CharTrait::like
//		自分自身に対してパターンがマッチするか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Common::StringData*	pattern
//			自分自身に対してマッチするか調べるパターンを表す文字列データ
//		ModUnicodeChar	escape
//			Common::UnicodeChar::usNull 以外が指定されたとき
//				パターン中で文字をエスケープするために使用する文字
//			Common::UnicodeChar::usNull または指定されないとき
//				エスケープ文字を使用しない
//
//	RETURN
//		true
//			マッチした
//		false
//			マッチしなかった
//
//	EXCEPTIONS
//		Exception::BadArgument
//			パターンとしてNULL 値が与えられた
//		Exception::NullNotAllowed
//			自分自身は NULL 値である
//
bool
CharTrait::like(const Common::StringData* pStringData_,
					 const Common::StringData* pattern,
					 ModUnicodeChar escape)
{
	; _TRMEISTER_ASSERT(pStringData_);
	; _TRMEISTER_ASSERT(pattern);

	if (pStringData_->isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);
	if (pattern->isNull())
		_TRMEISTER_THROW0(Exception::BadArgument);

	return like_NotNull(pStringData_, pattern, escape);
}
bool
CharTrait::like_NotNull(const Common::StringData* pStringData_,
							 const Common::StringData* pattern,
							 ModUnicodeChar escape)
{
	; _TRMEISTER_ASSERT(pStringData_);
	; _TRMEISTER_ASSERT(pattern);
	; _TRMEISTER_ASSERT(!pStringData_->isNull());
	; _TRMEISTER_ASSERT(!pattern->isNull());

	// [NOTE] CompressedStringDataの場合、伸長してから処理する必要がある。
	//  以下のgetValue_NotNull()で実行されるので問題ない。
	
	const ModUnicodeString& v = pStringData_->getValue_NotNull();
	const ModUnicodeString& p = pattern->getValue_NotNull();

	bool result;
	if (escape == Common::UnicodeChar::usNull)
		result = like(v, v.getLength(), p, p.getLength(),
					  pStringData_->getCollation());
	else
		result = like(v, v.getLength(), p, p.getLength(), escape,
					  pStringData_->getCollation());
	return result;
}

//
//	FUNCTION public
//	Utility::CharTrait::like --
//		自分自身に対してパターンがマッチするか調べる
//
//	NOTES
//		正規化は UNICODE 標準の正規形 KC になるように行う
//
//	ARGUMENTS
//		Common::StringData*	pattern
//			自分自身に対してマッチするか調べるパターンを表す文字列データ
//		CharTrait::NormalizingMethod::Value	normalizing
//			自分自身、パターン、エスケープ文字を正規化するか、
//			正規化するならどのような手段で行うかを表す
//		ModUnicodeChar	escape
//			Common::UnicodeChar::usNull 以外が指定されたとき
//				パターン中で文字をエスケープするために使用する文字
//			Common::UnicodeChar::usNull または指定されないとき
//				エスケープ文字を使用しない
//
//	RETURN
//		true
//			マッチした
//		false
//			マッチしなかった
//
//	EXCEPTIONS
//		Exception::BadArgument
//			パターンとしてNULL 値が与えられた
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

bool
CharTrait::like(const Common::StringData* pStringData_,
					 const Common::StringData* pattern,
					 NormalizingMethod::Value normalizing,
					 ModUnicodeChar escape)
{
	; _TRMEISTER_ASSERT(pStringData_);
	; _TRMEISTER_ASSERT(pattern);

	if (pStringData_->isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);
	if (pattern->isNull())
		_TRMEISTER_THROW0(Exception::BadArgument);

	return like_NotNull(pStringData_, pattern, normalizing, escape);
}
bool
CharTrait::like_NotNull(const Common::StringData* pStringData_,
							 const Common::StringData* pattern,
							 NormalizingMethod::Value normalizing,
							 ModUnicodeChar escape)
{
	; _TRMEISTER_ASSERT(pStringData_);
	; _TRMEISTER_ASSERT(pattern);
	; _TRMEISTER_ASSERT(!pStringData_->isNull());
	; _TRMEISTER_ASSERT(!pattern->isNull());

	if (normalizing) {
		struct _Normalized {
			ModUnicodeString _data;
			ModUnicodeString _pattern;
			ModUnicodeString _escape;
		} normalized;

		// [NOTE] CompressedStringDataの場合、伸長してから処理する必要がある。
		//  以下のgetValue_NotNull()で実行されるので問題ない。
	
		normalize(pStringData_->getValue_NotNull(),
				  normalizing,
				  normalized._data);
		// Escape a pattern and replace an escape character before normalizing.
		ModUnicodeString cstrEscapedPattern;
		_StringData::escape(pattern->getValue_NotNull(),
							EscapeTarget::FullwidthWildcard,
							escape,
							cstrEscapedPattern);
		normalize(cstrEscapedPattern,
				  normalizing,
				  normalized._pattern);
		// [NOTE] The character of 'escape' may be replaced in escape().
		if (escape != Common::UnicodeChar::usNull)
			normalize(ModUnicodeString(&escape, 1),
					  normalizing,
					  normalized._escape);

		return (escape != Common::UnicodeChar::usNull &&
				normalized._escape.getLength()) ?
			like(normalized._data, normalized._data.getLength(),
				 normalized._pattern, normalized._pattern.getLength(),
				 normalized._escape[0],
				 pStringData_->getCollation())
			:
			like(normalized._data, normalized._data.getLength(),
				 normalized._pattern, normalized._pattern.getLength(),
				 pStringData_->getCollation());
	}

	return like_NotNull(pStringData_, pattern, escape);
}

//
//	FUNCTION public static
//	Utility::CharTrait::escape --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
CharTrait::escape(const ModUnicodeString& cstrSrc_,
				  CharTrait::EscapeTarget::Value eEscapeTarget_,
				  ModUnicodeChar& usEscapeCharacter_,
				  ModUnicodeString& cstrDst_)
{
	_StringData::escape(cstrSrc_, eEscapeTarget_, usEscapeCharacter_, cstrDst_);
}


//
//	Copyright (c) 2009, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

