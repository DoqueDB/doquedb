// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BinaryData.java -- バイナリ型をあらわすクラス
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
 * バイナリ型をあらわすクラス
 *
 */
public class BinaryData extends Data
	implements Serializable
{
	/** 値 */
	private byte[] _value;

	/**
	 * データ型をあらわすクラスを新たに作成する
	 */
	public BinaryData()
	{
		super(DataType.BINARY);
		_value = new byte[0];
	}

	/**
	 * データ型をあらわすクラスを新たに作成する
	 *
	 * @param value_	格納する値
	 */
	public BinaryData(byte[] value_)
	{
		super(DataType.BINARY);
		_value = (byte[])value_.clone();

	}

	/**
	 * データ型をあらわすクラスを新たに作成する
	 *
	 * @param value_	格納する値
	 */
	public BinaryData(BinaryData value_)
	{
		super(DataType.BINARY);
		_value = (byte[])value_.getValue().clone();
	}

	/**
	 * 値を得る
	 *
	 * @return	格納されている値
	 */
	public byte[] getValue()
	{
		return _value;
	}

	/**
	 * 値を設定する
	 *
	 * @param value_	格納する値
	 */
	public void setValue(byte[] value_)
	{
		_value = (byte[])value_.clone();
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
		int len = input_.readInt();	// DoqueDBはunsigned intで書いているが、
									// Javaにはないのでintで読む
		_value = new byte[len];
		int off = 0;
		while (len > 0)
		{
			int n = input_.read(_value, off, len);
			len -= n;
			off += n;
		}
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
		output_.writeInt(_value.length);	// DoqueDBはunsigned intで読んで
											// いるが、Javaにはないのでintで
											// 書く
		if (_value.length > 0)
			output_.write(_value);
	}

	/**
	 * データが等しいか比較する
	 *
	 * @return	等しい場合はtrue、それ以外の場合はfalse
	 */
	public boolean equals(Object other_)
	{
		boolean result = false;
		if ((other_ instanceof BinaryData) == true)
		{
			BinaryData o = (BinaryData)other_;
			result = getValue().equals(o.getValue());
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
		return new BinaryData(this);
	}

	/**
	 * 文字列に変換する
	 *
	 * @return	文字列
	 */
	public String toString()
	{
		StringBuilder s = new StringBuilder("size=");
		s.append(Integer.toString(_value.length));
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
		return ClassID.BINARY_DATA;
	}
}

//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
