// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LanguageData.java -- Language 型をあらわすクラス
//
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
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
 * Language 型をあらわすクラス
 *
 * @see		LanguageSet
 */
public final class LanguageData extends ScalarData
	implements Serializable
{
	/**
	 * 言語セット
	 *
	 * @see	LanguageSet
	 */
	private LanguageSet	_value;

	/**
	 * 新たに Language 型のデータを作成する。
	 */
	public LanguageData()
	{
		super(DataType.LANGUAGE);
		_value = new LanguageSet();
	}

	/**
	 * 新たに Language 型のデータを作成する。
	 *
	 * @param	symbols_
	 *		言語セットを示す文字列。
	 */
	public LanguageData(String	symbols_)
	{
		super(DataType.LANGUAGE);
		_value = new LanguageSet(symbols_);
	}

	/**
	 * 新たに Language 型のデータを作成する。
	 *
	 * @param	languageData_
	 *				Language 型のデータ。
	 */
	public LanguageData(LanguageData	languageData_)
	{
		super(DataType.LANGUAGE);
		_value = new LanguageSet(languageData_._value);
	}

	/**
	 * 言語セットを返す。
	 *
	 * @return	言語セット。
	 * @see		LanguageSet
	 */
	public LanguageSet getValue()
	{
		return _value;
	}

	/**
	 * {@link ClassID クラスID}を得る
	 *
	 * @return {@link ClassID クラスID}
	 * @see Serializable#getClassID() getClassID
	 */
	public int getClassID()
	{
		return ClassID.LANGUAGE_DATA;
	}

	/**
	 * データが等しいか比較する
	 *
	 * @return	等しい場合はtrue、それ以外の場合はfalse
	 */
	public boolean equals(Object	other_)
	{
		if (other_ instanceof LanguageData) {

			LanguageData	other = (LanguageData)other_;
			return _value.equals(other._value);
		}
		return false;
	}

	/**
	 * オブジェクトのコピーを作成して返す
	 *
	 * @return	コピーされたオブジェクト
	 */
	public Object clone()
	{
		return new LanguageData(this);
	}

	/**
	 * 言語セットを文字列に変換する
	 *
	 * @return	言語セット文字列
	 */
	public String toString()
	{
		return _value.toString();
	}

	/**
	 * 言語セットをストリームから読み込む
	 *
	 * @param	input_
	 *				入力用のストリーム
	 * @throws	java.io.IOException
	 *				入出力関係の例外が発生した
	 * @throws	java.io.StreamCorruptedException
	 *				未サポートの言語セットが書き込まれている。
	 * @see Serializable#readObject(InputStream) readObject
	 */
	public void readObject(InputStream	input_)
		throws
			java.io.IOException,
			java.io.StreamCorruptedException
	{
		_value.readObject(input_);
	}

	/**
	 * 言語セットをストリームに書き出す
	 *
	 * @param	output_
	 *				出力用のストリーム
	 * @throws	java.io.IOException
	 *				入出力関係の例外が発生した
	 * @throws	java.lang.ArrayIndexOutOfBoundsException
	 *				言語セット内に不正な値が設定されている。
	 * @see	Serializable#writeObject(OutputStream) writeObject
	 */
	public void writeObject(OutputStream	output_)
		throws
			java.io.IOException,
			java.lang.ArrayIndexOutOfBoundsException
	{
		_value.writeObject(output_);
	}
}

//
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
