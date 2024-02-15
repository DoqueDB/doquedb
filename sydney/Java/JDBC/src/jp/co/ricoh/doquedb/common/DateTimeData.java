// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DateTimeData.java -- datetime型をあらわすクラス
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
 * datetime型をあらわすクラス
 *
 */
public final class DateTimeData extends ScalarData
	implements Serializable
{
	/** 値 */
	private java.util.GregorianCalendar _value;
	/** ミリ秒 */
	private int _millisecond;

	/**
	 * 新たにdatetime型のデータを作成する。
	 */
	public DateTimeData()
	{
		super(DataType.DATE_TIME);
		_value = new java.util.GregorianCalendar();
		_millisecond = 0;
	}

	/**
	 * 新たにdatetime型のデータを作成する。
	 *
	 * @param value_	格納する値
	 */
	public DateTimeData(java.sql.Timestamp value_)
	{
		super(DataType.DATE_TIME);
		_value = new java.util.GregorianCalendar();
		setValue(value_);
	}

	/**
	 * 新たにdatetime型のデータを作成する。
	 *
	 * @param value_	格納する値
	 */
	public DateTimeData(DateTimeData value_)
	{
		super(DataType.DATE_TIME);
		_value = new java.util.GregorianCalendar();
		_value.setTime(value_._value.getTime());
		_millisecond = value_._millisecond;
	}

	/**
	 * 値を得る
	 *
	 * @return	格納されている値
	 */
	public java.sql.Timestamp getValue()
	{
		long	offset = _value.getTime().getTime();
//		offset += _millisecond;
//		return new java.sql.Timestamp(offset);
		java.sql.Timestamp	timestamp = new java.sql.Timestamp(offset);
		timestamp.setNanos(_millisecond * 1000000); // millis -> nanos
		return timestamp;
	}

	/**
	 * 値を設定する
	 *
	 * @param value_	格納する値
	 */
	public void setValue(java.sql.Timestamp value_)
	{
		_value.setTime(value_);
		_millisecond = (value_.getNanos() / 1000000);
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
		int hour = input_.readInt();
		int minute = input_.readInt();
		int second = input_.readInt();
		_millisecond = input_.readInt();
		int precision = input_.readInt();
		_value.set(year, month, day, hour, minute, second);
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
		output_.writeInt(_value.get(java.util.Calendar.HOUR_OF_DAY));
		output_.writeInt(_value.get(java.util.Calendar.MINUTE));
		output_.writeInt(_value.get(java.util.Calendar.SECOND));
		output_.writeInt(_millisecond);
		output_.writeInt(3);	// precision はつねに3
	}

	/**
	 * データが等しいか比較する
	 *
	 * @return	等しい場合はtrue、それ以外の場合はfalse
	 */
	public boolean equals(Object other_)
	{
		boolean result = false;
		if ((other_ instanceof DateTimeData) == true)
		{
			DateTimeData o = (DateTimeData)other_;
			result = _value.equals(o._value);
			if (result == true)
				result = (_millisecond == o._millisecond);
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
		return new DateTimeData(this);
	}

	/**
	 * 文字列に変換する
	 *
	 * @return	文字列
	 */
	public String toString()
	{
		java.text.SimpleDateFormat f
			= new java.text.SimpleDateFormat("yyyy-MM-dd HH:mm:ss.");
		java.util.Date d = _value.getTime();
		StringBuilder s = new StringBuilder(f.format(d));
		if (_millisecond < 10)
			s.append("0");
		if (_millisecond < 100)
			s.append("0");
		s.append(_millisecond);

		return s.toString();
	}

	/**
	 * {@link ClassID クラスID}を得る
	 *
	 * @return {@link ClassID クラスID}
	 * @see Serializable#getClassID() getClassID
	 */
	public int getClassID()
	{
		return ClassID.DATE_TIME_DATA;
	}
}

//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
