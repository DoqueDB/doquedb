// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DateData.java -- date型をあらわすクラス
//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
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
 * date型をあらわすクラス
 *
 */
public final class DateData extends ScalarData
	implements Serializable
{
	/** 値 */
	private java.util.GregorianCalendar _value;

	/**
	 * 新たにdate型のデータを作成する。
	 */
	public DateData()
	{
		super(DataType.DATE);
		_value = new java.util.GregorianCalendar();
	}

	/**
	 * 新たにdate型のデータを作成する。
	 *
	 * @param value_	格納する値
	 */
	public DateData(java.sql.Date value_)
	{
		super(DataType.DATE);
		_value = new java.util.GregorianCalendar();
		_value.setTime(value_);
	}

	/**
	 * 新たにdate型のデータを作成する。
	 *
	 * @param value_	格納する値
	 */
	public DateData(DateData value_)
	{
		super(DataType.DATE);
		_value = new java.util.GregorianCalendar();
		_value.setTime(value_._value.getTime());
	}

	/**
	 * 値を得る
	 *
	 * @return	格納されている値
	 */
	public java.sql.Date getValue()
	{
		return new java.sql.Date(_value.getTime().getTime());
	}

	/**
	 * 値を設定する
	 *
	 * @param value_	格納する値
	 */
	public void setValue(java.sql.Date value_)
	{
		_value.setTime(value_);
	}

	/**
	 * ストリームから読み込む
	 *
	 * @param input_	入力用のストリーム
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @see Serializable#readObject(InputStream) readObject
	 */
	public void readObject(InputStream input_)
		throws java.io.IOException
	{
		//
		// ***
		// DoqueDB は、“月”は当然 1 〜 12 の間で書かれている。
		// が、しかし、 java.util.Calendar では 0 〜 11 で設定する。
		//

		int year = input_.readInt();
		// *** 上記コメントを参照。
		int month = input_.readInt() - 1;
		int day = input_.readInt();
		_value.set(year, month, day);
	}

	/**
	 * ストリームに書き出す
	 *
	 * @param output_	出力用のストリーム
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @see	Serializable#writeObject(OutputStream) writeObject
	 */
	public void writeObject(OutputStream output_)
		throws java.io.IOException
	{
		output_.writeInt(_value.get(java.util.Calendar.YEAR));
		// *** readObject() のコメントを参照。
		output_.writeInt(_value.get(java.util.Calendar.MONTH) + 1);
		output_.writeInt(_value.get(java.util.Calendar.DATE));
	}

	/**
	 * データが等しいか比較する
	 *
	 * @return	等しい場合はtrue、それ以外の場合はfalse
	 */
	public boolean equals(Object other_)
	{
		boolean result = false;
		if ((other_ instanceof DateData) == true)
		{
			DateData o = (DateData)other_;
			result = _value.equals(o._value);
		}
		return result;
	}

	/**
	 * オブジェクトのコピーを作成して返す
	 *
	 * @return	コピーされたオブジェクト
	 */
	public Object clone()
	{
		return new DateData(this);
	}

	/**
	 * 文字列に変換する
	 *
	 * @return	文字列
	 */
	public String toString()
	{
		java.text.SimpleDateFormat f
			= new java.text.SimpleDateFormat("yyyy-MM-dd");
		java.util.Date d = new java.util.Date(_value.getTime().getTime());
		return f.format(d);
	}

	/**
	 * {@link ClassID クラスID}を得る
	 *
	 * @return {@link ClassID クラスID}
	 * @see Serializable#getClassID() getClassID
	 */
	public int getClassID()
	{
		return ClassID.DATE_DATA;
	}
}

//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
