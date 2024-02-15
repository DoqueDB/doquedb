// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LanguageSet.java -- 複数の言語タグ(以下言語セット)を扱うクラス
//
// Copyright (c) 2003, 2007, 2023 Ricoh Company, Ltd.
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
 * 複数の言語タグ（言語セット）を表現するクラス。
 *
 * @see		jp.co.ricoh.doquedb.common.LanguageTag
 */
public final class LanguageSet
{
	/** 言語タグの区切り文字 */
	public final static char	TAG_SEPARATOR = '+';

	/** 言語種別コードと国・地域コードの区切り文字 */
	public final static char	LANGUAGE_COUNTRY_SEPARATOR = '-';

	/** 言語タグを積むベクター */
	private java.util.Vector	_v;

	/**
	 * 新たに LanguageSet 型のデータを作成します。
	 */
	public LanguageSet()
	{
		_v = new java.util.Vector();
	}

	/**
	 * 新たに LanguageSet 型のデータを作成します。
	 *
	 * @param	langSetSymbol_
	 *			言語セットを示す文字列。
	 * @see		#set(String)
	 */
	public LanguageSet(String	langSetSymbol_)
	{
		this();
		set(langSetSymbol_);
	}

	/**
	 * 新たに LanguageSet 型のデータを作成します。
	 *
	 * @param	languageSet_
	 *			言語セット。
	 * @see		#set(LanguageSet)
	 */
	public LanguageSet(LanguageSet	languageSet_)
	{
		this();
		set(languageSet_);
	}

	/**
	 * 言語セットを設定します。
	 *
	 * @param	langSetSymbol_
	 *			言語セットを示す文字列。
	 * @throws	java.lang.IllegalArgumentException
	 *			言語セットを示す文字列中に不正な言語名や国・地域名が存在した。
	 */
	public void set(String	langSetSymbol_)
		throws java.lang.IllegalArgumentException
	{
		int	symbolLen = langSetSymbol_.length();

		if (symbolLen == 0) {
			clear();
			return;
		}

		LanguageSet	tmpLanguageSet = new LanguageSet();
		int			languageCode = Language.UNDEFINED;
		int			countryCode = Country.UNDEFINED;
		boolean		setCountryCode = false;
		int			st = 0;
		for (int i = 0; i <= symbolLen; i++) {

			if (i == symbolLen || langSetSymbol_.charAt(i) == TAG_SEPARATOR) {

				if (setCountryCode) {

					String	countrySymbol = langSetSymbol_.substring(st, i);
					countryCode = Country.toCode(countrySymbol);
					if (Country.isValid(countryCode) == false) {
						String	errMsg =
							"Illegal country symbol '" + countrySymbol + "'.";
						throw new java.lang.IllegalArgumentException(errMsg);
					}

				} else {

					String	languageSymbol = langSetSymbol_.substring(st, i);
					languageCode = Language.toCode(languageSymbol);
					if (Language.isValid(languageCode) == false) {
						String	errMsg = "Illegal language symbol '" +
										 languageSymbol + "'.";
						throw new java.lang.IllegalArgumentException(errMsg);
					}
				}

				LanguageTag	languageTag =
					new LanguageTag(languageCode, countryCode);
				tmpLanguageSet.add(languageTag);

				languageCode = Language.UNDEFINED;
				countryCode = Country.UNDEFINED;
				setCountryCode = false;

				st = i + 1;

			} else if (
				langSetSymbol_.charAt(i) == LANGUAGE_COUNTRY_SEPARATOR) {

				setCountryCode = true;

				String	languageSymbol = langSetSymbol_.substring(st, i);
				languageCode = Language.toCode(languageSymbol);
				if (Language.isValid(languageCode) == false) {
					String	errMsg =
						"Illegal language symbol '" + languageSymbol + "'.";

					throw new java.lang.IllegalArgumentException(errMsg);
				}

				st = i + 1;
			}
		}

		set(tmpLanguageSet);
 	}

	/**
	 * 言語セットを設定します。
	 *
	 * @param	languageSet_
	 *			言語セット。
	 */
	public void set(LanguageSet	languageSet_)
	{
		_v = (java.util.Vector)languageSet_._v.clone();
	}

	/**
	 * 言語セットに指定言語を追加します。
	 *
	 * @param	languageCode_
	 *			追加する言語の種別コード。
	 * @throws	java.lang.IllegalArgumentException
	 *			不正な言語コードである。
	 */
	public void add(int	languageCode_)
		throws java.lang.IllegalArgumentException
	{
		LanguageTag	languageTag =
			new LanguageTag(languageCode_, Country.UNDEFINED);
		add(languageTag);
	}

	/**
	 * 言語セットに指定言語タグを追加します。
	 *
	 * @param	languageTag_
	 *			追加する言語タグ。
	 * @throws	java.lang.IllegalArgumentException
	 *			不正な言語コードである。
	 */
	public void add(LanguageTag	languageTag_)
		throws java.lang.IllegalArgumentException
	{
		if (Language.isValid(languageTag_._languageCode) == false) {
			String	errMsg =
				"Illegal language code '" +
				String.valueOf(languageTag_._languageCode) +
				"'.";
			throw new java.lang.IllegalArgumentException(errMsg);
		}

		int	numberOfLanguageTags = _v.size();
		for (int i = 0; i < numberOfLanguageTags; i++) {

			LanguageTag	languageTag = (LanguageTag)_v.elementAt(i);

			if (languageTag_._languageCode == languageTag._languageCode) {

				for (; i < numberOfLanguageTags; i++) {

					languageTag = (LanguageTag)_v.elementAt(i);

					if (languageTag_._languageCode !=
						languageTag._languageCode) {
						break;
					}

					// 言語および国・地域コードが等しい言語タグが
					// 存在するのであれば、追加する必要はない。
					if (languageTag_._countryCode ==
						languageTag._countryCode) {
						return;
					}

					if (languageTag_._countryCode < languageTag._countryCode) {
						break;
					}
				}

				// 途中（または最後）に追加する。
				_v.add(i, languageTag_);
				return;

			} else if (
				languageTag_._languageCode < languageTag._languageCode) {

				// 途中に追加する。
				_v.add(i, languageTag_);
				return;
			}

		} // end for i

		// 最後に追加する。
		_v.add(languageTag_);
	}

	/** 言語セットをクリアする。 */
	public void clear()
	{
		_v.clear();
	}

	/**
	 * データが等しいか比較します。
	 *
	 * @param	other_
	 *			比較先の言語セット。
	 * @return	等しい場合は <code>true</code> 、
	 *			それ以外の場合は <code>false</code> 。
	 */
	public boolean equals(Object	other_)
	{
		if (other_ instanceof LanguageSet) {
			LanguageSet	other = (LanguageSet)other_;
			return _v.equals(other._v);
		}
		return false;
	}

	/**
	 * ハッシュコードを取得します。
	 */
	public int hashCode()
	{
		int hashCode = 0;
		for (int i = 0; i < _v.size(); ++i)
		{
			hashCode <<= 4;
			hashCode |= _v.get(i).hashCode();
		}
		return hashCode;
	}

	/**
	 * 指定言語が含まれているかどうかを調べます。
	 *
	 * @param	languageCode_
	 *			含まれているかどうかを調べる対象言語の種別コード。
	 * @return	含まれている場合には <code>true</code> 、
	 *			含まれていない場合には <code>false</code> 。
	 */
	public boolean isContained(int	languageCode_)
		throws java.lang.ArrayIndexOutOfBoundsException
	{
		if (Language.isValid(languageCode_) == false) {
			String	errMsg = "Illegal language code '" +
							 String.valueOf(languageCode_) + "'.";
			throw new java.lang.ArrayIndexOutOfBoundsException(errMsg);
		}

		int	numberOfLanguageTags = _v.size();
		for (int i = 0; i < numberOfLanguageTags; i++) {

			LanguageTag	languageTag = (LanguageTag)_v.elementAt(i);

			if (languageTag._languageCode == languageCode_) return true;
		}

		return false;
	}

	/**
	 * 指定言語タグが含まれているかどうかを調べます。
	 *
	 * @param	languageTag_
	 *			含まれているかどうかを調べる対象言語タグ。
	 * @return	含まれている場合には <code>true</code> 、
	 *			含まれていない場合には <code>false</code> 。
	 */
	public boolean isContained(LanguageTag	languageTag_)
		throws java.lang.ArrayIndexOutOfBoundsException
	{
		if (Language.isValid(languageTag_._languageCode) == false) {
			String	errMsg = "Illegal language code '" +
							 String.valueOf(languageTag_._languageCode) + "'.";
			throw new java.lang.ArrayIndexOutOfBoundsException(errMsg);
		}

		int	numberOfLanguageTags = _v.size();
		for (int i = 0; i < numberOfLanguageTags; i++) {

			LanguageTag	languageTag = (LanguageTag)_v.elementAt(i);

			if (languageTag._languageCode == languageTag_._languageCode &&
				languageTag._countryCode == languageTag_._countryCode) {
				return true;
			}
		}

		return false;
	}

	/**
	 * 言語タグから国・地域コードを除いた言語セットを取得します。
	 *
	 * @return	このオブジェクトの言語タグから国・地域コードを除いた
	 *			言語セット。
	 */
	public LanguageSet round()
	{
		LanguageSet	noCountrySet = new LanguageSet();

		int	numberOfLanguageTags = _v.size();
		for (int i = 0; i < numberOfLanguageTags; i++) {

			LanguageTag	languageTag = (LanguageTag)_v.elementAt(i);

			LanguageTag	noCountryTag =
				new LanguageTag(languageTag._languageCode, Country.UNDEFINED);
			noCountrySet.add(noCountryTag);
		}

		return noCountrySet;
	}

	/**
	 * 言語タグ数を取得します。
	 *
	 * @return	ベクターに積まれている言語タグ数。
	 */
	public int getSize()
	{
		return _v.size();
	}

	/**
	 * 言語セットを文字列に変換します。
	 *
	 * @return	言語セット文字列。
	 */
	public String toString()
	{
		String	langSetSymbol = "";

		int	numberOfLanguageTags = _v.size();
		for (int i = 0; i < numberOfLanguageTags; i++) {

			LanguageTag	languageTag = (LanguageTag)_v.elementAt(i);

			langSetSymbol =
				langSetSymbol.concat(
					Language.toSymbol(languageTag._languageCode));

			if (languageTag._countryCode != Country.UNDEFINED) {
				langSetSymbol =
					langSetSymbol.concat(
						String.valueOf(LANGUAGE_COUNTRY_SEPARATOR));
				langSetSymbol =
					langSetSymbol.concat(
						Country.toSymbol(languageTag._countryCode));
			}

			if (i < numberOfLanguageTags - 1) {
				langSetSymbol =
					langSetSymbol.concat(String.valueOf(TAG_SEPARATOR));
			}
		}

		return langSetSymbol;
	}

	/**
	 * 言語セットをストリームから読み込みます。
	 *
	 * @param	input_
	 *			入力用のストリーム。
	 * @throws	java.io.IOException
	 *			入出力関係の例外が発生した。
	 * @throws	java.io.StreamCorruptedException
	 *			未サポートの言語セットが書き込まれている。
	 * @see Serializable#readObject(InputStream) readObject
	 */
	public void readObject(InputStream	input_)
		throws java.io.IOException, java.io.StreamCorruptedException
	{
		clear();

		// 言語タグ数
		int	numberOfLanguageTags = input_.readInt();

		for (int i = 0; i < numberOfLanguageTags; i++) {

			// 言語コード
			int	languageCode = (int)input_.readShort();

			// 国・地域コード
			int	countryCode = (int)input_.readShort();

			if (Language.isValid(languageCode) == false) {
				String	errMsg = "unknown language code '" +
								 String.valueOf(languageCode) + "'.";
				throw new java.io.StreamCorruptedException(errMsg);
			}
			if (countryCode != Country.UNDEFINED &&
				Country.isValid(countryCode) == false) {

				String	errMsg = "unknown country code '" +
								 String.valueOf(countryCode) + "'.";
				throw new java.io.StreamCorruptedException(errMsg);
			}

			LanguageTag	languageTag = new LanguageTag(languageCode,
													  countryCode);
			add(languageTag);
		}
	}

	/**
	 * 言語セットをストリームに書き出します。
	 *
	 * @param	output_
	 *			出力用のストリーム。
	 * @throws	java.io.IOException
	 *			入出力関係の例外が発生した。
	 * @throws	java.lang.ArrayIndexOutOfBoundsException
	 *			言語セット内に不正な値が設定されている。
	 * @see	Serializable#writeObject(OutputStream) writeObject
	 */
	public void writeObject(OutputStream	output_)
		throws
			java.io.IOException,
			java.lang.ArrayIndexOutOfBoundsException
	{
		int	numberOfLanguageTags = _v.size();
		output_.writeInt(numberOfLanguageTags);

		for (int i = 0; i < numberOfLanguageTags; i++) {

			LanguageTag	languageTag = (LanguageTag)_v.elementAt(i);

			short	languageCode = (short)languageTag._languageCode;
			output_.writeShort(languageCode);

			short	countryCode = (short)languageTag._countryCode;
			output_.writeShort(countryCode);
		}
	}
}

//
// Copyright (c) 2003, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
