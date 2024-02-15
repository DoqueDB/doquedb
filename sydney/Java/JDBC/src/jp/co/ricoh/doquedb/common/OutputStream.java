// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OutputStream.java -- DoqueDBとの互換性のある出力ストリームクラス
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
 * DoqueDBとの互換性のある出力ストリームクラス。
 *
 */
public final class OutputStream
	extends java.io.DataOutputStream
{
	/**
	 * 出力ストリームを新しく作成する。
	 *
	 * @param outputStream_ Javaの出力ストリーム。
	 */
	public OutputStream(java.io.OutputStream outputStream_)
	{
		super(outputStream_);
	}

	/**
	 * DoqueDBとの互換性を維持し、Serializableのサブクラスを書き込む
	 *
	 * @param object_	書き込むSerializableのサブクラス
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 */
	public void writeObject(Serializable object_)
			throws java.io.IOException
	{
		if (object_ == null)
		{
			//nullなので
			writeInt(ClassID.NONE);
		}
		else
		{
			//まずはクラスIDを書く
			writeInt(object_.getClassID());
			//中身を書く
			object_.writeObject(this);
		}
	}
}

//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
