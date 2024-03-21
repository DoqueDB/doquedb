/*
 * -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
 * vi:set ts=4 sw=4:
 *
 * ModConfig.h -- 実行環境に応じたオブジェクトを生成するための定義
 * 
 * Copyright (c) 1999, 2012, 2023 Ricoh Company, Ltd.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef	__ModConfig_H__
#define	__ModConfig_H__

/***************************************************/
/*	 バージョンの下位互換性定義                    */
/***************************************************/
#ifdef CC_GCC
#ifndef CC_GCC4_8
#define CC_GCC4_8
#endif
#endif

#ifdef CC_ICC9_0
#ifndef CC_ICC8_0
#define CC_ICC8_0
#endif
#endif

/***************************************************/
/*	 演算装置間の差異を表すマクロ                  */
/***************************************************/

/*
 *	MACRO
 *	MOD_CONF_BYTEORDER -- システムのバイトオーダーを表すマクロ
 *
 *	NOTES
 *		0		最上位バイトから順に格納される(ビッグエンディアン)
 *		1		最下位バイトから順に格納される(リトルエンディアン)
 */

#ifdef	MCH_INTEL
#define	MOD_CONF_BYTEORDER		1		/* LH */
#endif

/***************************************************/
/* 	オペレーティングシステム間の差異を表すマクロ   */
/***************************************************/

/*
 *	MACRO
 *	MOD_CONF_DLL -- システムが DLL を提供するかを表すマクロ
 *
 *	NOTES
 *		0		なし
 *		1		あり
 */

/*
 *	MACRO
 *	MOD_CONF_SYSTEM_LOG -- システムが提供するログの種類を表すマクロ
 *
 *	NOTES
 *		0		なし
 *		1		システムコンソールログがある
 *		2		イベントログがある
 */

/*
 *	MACRO
 *	MOD_CONF_REGISTRY -- システムが提供するレジストリーの種類を表すマクロ
 *
 *	NOTES
 *		0		なし
 *		1		レジストリーがある
 */

/*
 *	MACRO
 *	MOD_CONF_ACCESS_CONTROL --
 *		システムがさまざまなオブジェクトに対するアクセス制御の機能を
 *		提供しているかを表すマクロ
 *
 *	NOTES
 *		0		なし
 *		1		あり
 */

/*
 *	MACRO
 *	MOD_CONF_LITERAL_CODE -- 文字列リテラルがどの漢字コードかを表すマクロ
 *
 *	NOTES
 *		0		EUC
 *		1		シフト JIS (SJIS) コード
 *		2		UTF-8
 */

/* 
 *	MACRO
 *	MOD_CONF_MEMORY_MANAGEMENT -- MOD を介する自由記憶領域の管理方法
 *
 *	NOTES
 *		0		標準 C ライブラリーを使用する
 *		1		ModMemoryBlockSize 以下の領域のみ MOD が管理する
 *		2		すべての領域を MOD が管理する
 *		3		HeapAllocを使う(Windows用, 未使用)
 */

/*
 *	MACRO
 *	MOD_CONF_LIB_POSIX
 *		POSIX C 言語のシステムコール API を使用するかを表すマクロ
 *
 *	NOTES
 *		POSIX C 言語のシステムコール API は、
 *	ISO/IEC 9945-1 (IEEE における IEEE Std. 1003.1) で規定されているが、
 *		OS が参照した時期によって、インタフェースや動作が異なっている
 *
 *		0		使用しない
 *		1		Solaris 独自の IEEE 1003.1c ドラフト準拠 (未使用)
 *		2		IEEE 1003.1c 準拠
 */

/*
 *	MACRO
 *	MOD_CONF_LIB_PTHREAD
 *		POSIX スレッドライブラリーを使用するかを表すマクロ
 *
 *	NOTES
 *		0		使用しない
 *		1		使用する
 *		2		使用する、ただし、ミューテックスをプロセス間で共有できない
 */

/* 
 *	MACRO
 *	MOD_CONF_FUNC_GETTIMEOFDAY -- 現在時刻を得るための手段はなにか
 *
 *	NOTES
 *		0		なし
 *		1		gettimeofday()
 *		2		_ftime()			(Windows C Runtime Library, 未使用)
 */

/*
 *	MACRO
 *	MOD_CONF_LIB_SLOG -- SLogを使用する／使用しない
 *
 *	NOTES
 *		0		使用しない
 *		1		使用する
 */

/*
 *	MACRO
 *	MOD_CONF_UNICODE_PARAMETER -- パラメーターソースがUnicode化されているか
 *
 *	NOTES
 *		0		Unicode化されていない (literalCodeが使われる)
 *		1		Unicode化されている (utf8が想定される)
 */
#ifdef	OS_RHLINUX6_0
#define	MOD_CONF_DLL						0
#define	MOD_CONF_SYSTEM_LOG					1
#define	MOD_CONF_REGISTRY					0
#define	MOD_CONF_ACCESS_CONTROL				0
#define	MOD_CONF_LITERAL_CODE				2
#define	MOD_CONF_MEMORY_MANAGEMENT			0
#define	MOD_CONF_LIB_POSIX					2
#define	MOD_CONF_LIB_PTHREAD				2
#define	MOD_CONF_FUNC_GETTIMEOFDAY			1
#define MOD_CONF_LIB_SLOG					0
#define MOD_CONF_UNICODE_PARAMETER			0
#endif

/***************************************************/
/*	コンパイラー間の差異を表すマクロ               */
/***************************************************/

/*
 *	MACRO
 *	MOD_CONF_BOOL_TYPE -- 基本型として bool があるか
 *
 *	NOTES
 *		0		ない
 *		1		ある
 */

/*
 *	MACRO
 *	MOD_CONF_NO_INDIRECTIONAL_FOR_BASIC --
 *		基本型のテンプレートクラスに返り値を持つ間接参照演算子を定義できないか
 *
 *	NOTES
 *		返り値を返す間接参照演算子 -> を持つテンプレートクラスを定義したとき、
 *		char など間接参照演算子 -> を適用できない基本型を
 *		テンプレート引数として与えられるか
 *
 *		0		与えられる
 *		1		SUN C++ 4.2 Patch 104631-07 では、構文エラーになるため、
 *				与えられない (未使用)
 */

/*
 *	MACRO
 *	MOD_CONF_FULL_SPECIALIZATION_STYLE -- 特別バージョンの定義方法を表す
 *
 *	NOTES
 *		0		特別バージョンを定義できない
 *		1		template <> なしでないと、構文エラーになるため定義できない
 *		2		template <> を最初に付加して定義することもできる
 */

/*
 *	MACRO
 *	MOD_CONF_PARTIAL_SPECIALIZATION_STYLE -- 部分特別バージョンの定義方法を表す
 *
 *	NOTES
 *		0		部分特別バージョンを定義できない
 *		1		特別バージョンパターン <T*> により定義することができる
 */

/*
 *	MACRO
 *	MOD_CONF_TYPENAME -- typename が必要かどうか
 *
 *	NOTES
 *		0		不要
 *		1		必要
 */

/*
 *	MACRO
 *	MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
 *		-- IteratorでValueTypeを定義するかどうか
 *
 *	NOTES
 *	Iteratorを引数にとる関数中でValueTypeが必要な時に、ModValueTypeという
 *	関数を利用していたが、このマクロが定義されているときには、
 *	Iterator::ValueTypeを利用するようにして、ModValueTypeは廃止する
 */

#ifdef CC_GCC
#define	MOD_CONF_BOOL_TYPE						1
#define	MOD_CONF_NO_INDIRECTIONAL_FOR_BASIC		0
#define	MOD_CONF_FULL_SPECIALIZATION_STYLE		2
#define	MOD_CONF_PARTIAL_SPECIALIZATION_STYLE	0
#define MOD_CONF_TYPENAME                       1
#define MOD_SELF_MEMORY_MANAGEMENT_OFF
#define MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
#define MOD_CONF_DEFINE_FRIENDFUNCTION
#endif
#ifdef CC_ICC8_0
#define	MOD_CONF_BOOL_TYPE						1
#define	MOD_CONF_NO_INDIRECTIONAL_FOR_BASIC		0
#define	MOD_CONF_FULL_SPECIALIZATION_STYLE		2
#define	MOD_CONF_PARTIAL_SPECIALIZATION_STYLE	0
#define	MOD_CONF_TYPENAME						1
#define MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
#define MOD_SELF_MEMORY_MANAGEMENT_OFF
#endif

/***************************************************/
/*	C++ 構文を補完するマクロ                       */
/***************************************************/

/*
 *	MACRO
 *	ModDLLExport -- DLL へ関数やオブジェクトをエクスポートする
 *
 *	NOTES
 */

/*
 *	MACRO
 *	ModDLLImport -- DLL から関数やオブジェクトをインポートする
 *
 *	NOTES
 */

#if MOD_CONF_DLL == 1
#else
#define	ModDLLExport
#define	ModDLLImport
#endif

/*
 *	MACRO
 *	ModTemplateNull -- テンプレートの明示的な特殊化の開始
 *
 *	NOTES
 */

#if MOD_CONF_FULL_SPECIALIZATION_STYLE == 2
#define	ModTemplateNull	template <>
#else
#define ModTemplateNull
#endif

/*
 *	MACRO
 *	ModTypename -- typename 
 *
 *	NOTES
 */

#if MOD_CONF_TYPENAME == 1
#define	ModTypename	typename
#else
#define	ModTypename
#endif

#ifdef PURIFY
#ifndef MOD_SELF_MEMORY_MANAGEMENT_OFF
#define MOD_SELF_MEMORY_MANAGEMENT_OFF
#endif
#endif

#endif	/* __ModConfig_H__ */

/*
 * Copyright (c) 1999, 2012, 2023 Ricoh Company, Ltd.
 * All rights reserved.
 */
