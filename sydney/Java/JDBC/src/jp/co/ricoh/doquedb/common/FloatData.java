// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FloatData.java -- float型をあらわすクラス
//
// Copyright (c) 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
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
 * float型をあらわすクラス
 *
 */
public final class FloatData extends ScalarData
	implements Serializable
{
	/** 値 */
	private float _value;

	/**
	 * 新たにfloat型のデータを作成する。
	 */
	public FloatData()
	{
		super(DataType.FLOAT);
		_value = 0;
	}

	/**
	 * 新たにfloat型のデータを作成する。
	 *
	 * @param value_	格納する値
	 */
	public FloatData(float value_)
	{
		super(DataType.FLOAT);
		_value = value_;
	}

	/**
	 * 新たにfloat型のデータを作成する。
	 *
	 * @param value_	格納する値
	 */
	public FloatData(FloatData value_)
	{
		super(DataType.FLOAT);
		_value = value_.getValue();
	}

	/**
	 * 値を得る
	 *
	 * @return	格納されている値
	 */
	public float getValue()
	{
		return _value;
	}

	/**
	 * 値を設定する
	 *
	 * @param value_	格納する値
	 */
	public void setValue(float value_)
	{
		_value = value_;
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
		_value = input_.readFloat();
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
		output_.writeFloat(_value);
	}

	/**
	 * データが等しいか比較する
	 *
	 * @return	等しい場合はtrue、それ以外の場合はfalse
	 */
	public boolean equals(Object other_)
	{
		boolean result = false;
		if ((other_ instanceof FloatData) == true)
		{
			FloatData o = (FloatData)other_;
			result = (getValue() == o.getValue());
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
		return new FloatData(this);
	}

	/**
	 * 文字列に変換する
	 *
	 * @return	文字列
	 */
	public String toString()
	{
		return Float.toString(_value);
	}

	/**
	 * {@link ClassID クラスID}を得る
	 *
	 * @return {@link ClassID クラスID}
	 * @see Serializable#getClassID() getClassID
	 */
	public int getClassID()
	{
		return ClassID.FLOAT_DATA;
	}
}

//
// Copyright (c) 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
