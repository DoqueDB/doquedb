// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LanguageTag.java --
//	言語種別と国・地域種別のペア（以下、言語タグ）を扱うクラス
//
// Copyright (c) 2003, 2004, 2007, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.common;

/**
 * 言語種別と国・地域種別のペア（言語タグ）を表現するクラス。
 *
 * @see		jp.co.ricoh.doquedb.common.Language
 * @see		jp.co.ricoh.doquedb.common.Country
 */
public final class LanguageTag
{
	/** 言語種別コード */
	public int	_languageCode;

	/** 国・地域コード */
	public int	_countryCode;

	/**
	 * 新たに LanguageTag 型のデータを作成する。
	 */
	public LanguageTag()
	{
		_languageCode = Language.UNDEFINED;
		_countryCode = Country.UNDEFINED;
	}

	/**
	 * 新たに LanguageTag 型のデータを作成する。
	 *
	 * @param	languageCode_
	 *			言語種別コード。
	 * @param	countryCode_
	 *			国・地域コード。
	 */
	public LanguageTag(int	languageCode_,
					   int	countryCode_)
	{
		_languageCode =
			 Language.isValid(languageCode_) ?
				languageCode_ : Language.UNDEFINED;
		_countryCode =
			Country.isValid(countryCode_) ?
				countryCode_ : Country.UNDEFINED;
	}

	/**
	 * 言語タグが等しいか比較します。
	 *
	 * @return	等しい場合には <code>true</code> 、
	 *			異なる場合には <code>false</code> 。
	 */
	public boolean equals(Object	other_)
	{
		if (other_ instanceof LanguageTag) {

			LanguageTag	other = (LanguageTag)other_;
			return
				_languageCode == other._languageCode &&
				_countryCode == other._countryCode;
		}
		return false;
	}

	/**
	 * コピーする
	 */
	public Object clone()
	{
		return new LanguageTag(_languageCode, _countryCode);
	}

	/**
	 * ハッシュコードを得る
	 */
	public int hashCode()
	{
		return (_countryCode << 4 | _languageCode);
	}

}

//
// Copyright (c) 2003, 2004, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
