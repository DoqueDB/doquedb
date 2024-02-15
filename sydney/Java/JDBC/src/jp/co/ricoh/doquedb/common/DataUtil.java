// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataUtil.java -- データ型のためのユーティリティ
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
 * データ型のためのユーティリティ
 *
 */
public final class DataUtil
{
	/** 圧縮するかどうかの閾値(byte) */
	private final static int THRESHOLD = 1024;

	/**
	 * 可能なら圧縮する。
	 * 閾値以下か、圧縮しても小さくならない場合には圧縮しない。
	 *
	 * @param buf_	圧縮されていない値
	 * @return	圧縮されたデータ
	 */
	public static byte[] compress(byte[] buf_)
	{
		byte[] compressed;
		if (buf_.length > THRESHOLD)
		{
			try
			{
				// 閾値を越えているので圧縮してみる
				java.io.ByteArrayOutputStream stream
					= new java.io.ByteArrayOutputStream(buf_.length - 1);
				java.util.zip.DeflaterOutputStream gzipStream
					= new java.util.zip.DeflaterOutputStream(
							stream,
							new java.util.zip.Deflater(),
							buf_.length - 1);
				gzipStream.write(buf_, 0, buf_.length);
				gzipStream.finish();

				// 圧縮されたので圧縮されたデータを得る
				compressed = stream.toByteArray();
			}
			catch (java.io.IOException e)
			{
				//圧縮できなかった -> 圧縮しても小さくならなかった
				compressed = buf_;
			}
		}
		else
		{
			//閾値以下なのでそのまま
			compressed = buf_;
		}

		return compressed;
	}

	/**
	 * 伸長する。
	 * 圧縮後のサイズと、圧縮前のサイズが同じ場合は伸長しない。
	 *
	 * @param compressed_	圧縮されている値
	 * @param size_			伸長後のサイズ
	 * @return	伸長したデータ
	 * @throws	java.io.IOException
	 *			伸長に失敗した
	 */
	public static byte[] uncompress(byte[] compressed_, int size_)
		throws java.io.IOException
	{
		byte[] buf = compressed_;

		if (compressed_.length < size_)
		{
			// 圧縮後のサイズの方が小さいので伸長する

			java.io.ByteArrayInputStream stream
				= new java.io.ByteArrayInputStream(compressed_);
			java.util.zip.InflaterInputStream gzipStream
				= new java.util.zip.InflaterInputStream(
						stream,
						new java.util.zip.Inflater(),
						size_);

			buf = new byte[size_];
			gzipStream.read(buf, 0, size_);
		}

		return buf;
	}
}

//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
