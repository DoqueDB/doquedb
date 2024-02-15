// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CompressedString.java -- 圧縮されたString型をあらわすクラス
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
 * 圧縮されたString型をあらわすクラス
 *
 */
public final class CompressedStringData extends StringData
{
	/** 圧縮データ */
	private BinaryData _compressedValue;
	/** 圧縮前のデータサイズ */
	private int _size;
	/** 圧縮後のデータサイズ */
	private int _compressedSize;

	/**
	 * 新たに圧縮されたString型のデータを作成する。
	 */
	public CompressedStringData()
	{
		super();
		_compressedValue = new BinaryData();
		_size = 0;
		_compressedSize = 0;
	}

	/**
	 * 新たに圧縮されたString型のデータを作成する。
	 *
	 * @param value_	圧縮されていない文字列
	 */
	public CompressedStringData(String value_)
	{
		super();
		_compressedValue = new BinaryData();
		setValue(value_);
	}

	/**
	 * 新たに圧縮されたString型のデータを作成する。
	 *
	 * @param value_	圧縮されている文字列
	 */
	public CompressedStringData(CompressedStringData value_)
	{
		super();
		_compressedValue = new BinaryData(value_._compressedValue);
	}

	/**
	 * 値を得る
	 *
	 * @return	圧縮されていない文字列
	 */
	public String getValue()
	{
		String s = super.getValue();
		if ((s == null || s.length() == 0) && _size != 0)
		{
			try
			{
				//圧縮されていないデータが存在しない -> uncompressする
				byte[] buf = DataUtil.uncompress(_compressedValue.getValue(),
												 _size);
				//UTF-8 -> UCS2の変換
				s = new String(buf, "UTF-8");

				//親クラスに設定する
				super.setValue(s);
			}
			catch (java.lang.Exception e)
			{
				e.printStackTrace();
			}
		}
		return s;
	}

	/**
	 * 値を設定する
	 *
	 * @param value_	圧縮されていない文字列
	 */
	public void setValue(String value_)
	{
		//親クラスのメンバーをクリアする
		super.setValue("");

		try
		{
			//UTF-8で取り出す
			byte[] buf = value_.getBytes("UTF-8");
			//圧縮前の長さを書く
			_size = buf.length;

			//圧縮可能なら圧縮する
			byte[] compressed = DataUtil.compress(buf);
			//圧縮後の長さを書く
			_compressedSize = compressed.length;

			//圧縮データを格納する
			_compressedValue.setValue(compressed);
		}
		catch (java.io.UnsupportedEncodingException e)
		{
			e.printStackTrace();
		}
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
		// DoqueDBはunsigned int (ModSize)で書き込むが、
		// Javaにはないので int で読む
		_size = input_.readInt();			//圧縮前のサイズ
		_compressedSize = input_.readInt();	//圧縮後のサイズ

		if (_compressedSize != 0)
			_compressedValue.readObject(input_);
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
		// DoqueDBはunsigned int (ModSize)で読み出すが、
		// Javaにはないので int で書く
		output_.writeInt(_size);			//圧縮前のサイズ
		output_.writeInt(_compressedSize);	//圧縮後のサイズ

		if (_compressedSize != 0)
			_compressedValue.writeObject(output_);
	}

	/**
	 * オブジェクトのコピーを作成して返す
	 *
	 * @return	コピーされたオブジェクト
	 */
	public Object clone()
	{
		return new CompressedStringData(this);
	}

	/**
	 * {@link ClassID クラスID}を得る
	 *
	 * @return {@link ClassID クラスID}
	 * @see Serializable#getClassID() getClassID
	 */
	public int getClassID()
	{
		return ClassID.COMPRESSED_STRING_DATA;
	}
}

//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
