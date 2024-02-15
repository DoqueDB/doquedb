// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InputStream.java -- DoqueDBとの互換性のある入力ストリームクラス
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
 * DoqueDB との互換性のある入力ストリームクラス。
 *
 */
public class InputStream
	extends java.io.DataInputStream
{
	/**
	 * 入力ストリームを新しく作成する。
	 *
	 * @param inputStream_	Javaの入力ストリーム。
	 */
	public InputStream(java.io.InputStream inputStream_)
	{
		super(inputStream_);
	}

	/**
	 * DoqueDBとの互換性を維持し、Serializableのサブクラスを読み込む
	 *
	 * @return 読み込んだSerializableのサブクラス
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 */
	public Serializable readObject()
		throws java.io.IOException,
			   ClassNotFoundException
	{
		//まずクラスIDを得る
		int id = readInt();
		//クラスのインスタンスを得る
		Serializable object = Instance.get(id);
		if (object != null)
			//中身を読み取る
			object.readObject(this);

		return object;
	}

	/**
	 * DoqueDBとの互換性を維持し、Serializableのサブクラスを読み込む
	 *
	 * @param	data	データを格納するSerializableクラス
	 * @return 読み込んだSerializableのサブクラス
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 */
	public Serializable readObject(Serializable data)
		throws java.io.IOException,
			   ClassNotFoundException
	{
		//まずクラスIDを得る
		int id = readInt();
		Serializable object = null;
		if (data != null && data.getClassID() == id)
		{
			object = data;
		}
		else
		{
			object = Instance.get(id);
		}
		if (object != null)
			//中身を読み取る
			object.readObject(this);

		return object;
	}

}

//
// Copyright (c) 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
