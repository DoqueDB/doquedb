// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Serializable.java -- DoqueDBと互換性のあるSerializableインターフェース
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
 * DoqueDBと互換性を保持しつつシリアル化を行うためのインターフェース。
 * Javaにも同じ名前のインターフェースがあるが、それはDoqueDBとの
 * 互換性がないので、専用のものを用意する。
 * odbcのデータ型クラスは、 本インターフェースをインプリメントする必要がある。
 *
 */
public interface Serializable
{
	/**
	 * ストリームから読み込む
	 *
	 * @param input_	入力用のストリーム
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			{@link ClassID クラスID}のクラスが見つからない
	 */
	public abstract void readObject(InputStream input_)
		throws java.io.IOException,
			   ClassNotFoundException;

	/**
	 * ストリームに書き出す
	 *
	 * @param output_	出力用のストリーム
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 */
	public abstract void writeObject(OutputStream output_)
		throws java.io.IOException;

	/**
	 * {@link ClassID クラスID}を得る
	 *
	 * @return {@link ClassID クラスID}
	 */
	public abstract int getClassID();
}

//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
