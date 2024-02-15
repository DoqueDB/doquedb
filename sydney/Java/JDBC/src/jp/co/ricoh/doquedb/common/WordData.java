// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// WordData.java -- ワードをあらわすクラス
//
// Copyright (c) 2004, 2007, 2023 Ricoh Company, Ltd.
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
 * ワードをあらわすクラス
 *
 */
public final class WordData extends Data
	implements Serializable
{
	/** カテゴリ - 未定義 */
	public final static int CATEGORY_UNDEFINED = 0;

	/** カテゴリ - 必須 */
	public final static int CATEGORY_ESSENTIAL = 1;
	/** カテゴリ - 重要 */
	public final static int CATEGORY_IMPORTANT = 2;
	/** カテゴリ - 有用 */
	public final static int CATEGORY_HELPFUL = 3;
	/** カテゴリ - 必須関連語 */
	public final static int CATEGORY_ESSENTIAL_RELATED = 4;
	/** カテゴリ - 重要関連語 */
	public final static int CATEGORY_IMPORTANT_RELATED = 5;
	/** カテゴリ - 有用関連語 */
	public final static int CATEGORY_HELPFUL_RELATED = 6;
	/** カテゴリ - 禁止 */
	public final static int CATEGORY_PROHIBITIVE = 7;
	/** カテゴリ - 禁止関連語 */
	public final static int CATEGORY_PROHIBITIVE_RELATED = 8;

	/** カテゴリの文字列表現 */
	private final static String[] CATEGORY_STRING =
	{
		"Undefined",
		"Essential",
		"Important",
		"Helpful",
		"EssentialRelated",
		"ImportantRelated",
		"HelpfulRelated",
		"Prohibitive",
		"ProhibitiveRelated"
	};

	/** 単語 */
	private String term;
	/** 言語 */
	private LanguageSet language;
	/** カテゴリ */
	private int category;
	/** スケール */
	private double scale;
	/** 文書頻度 */
	private int df;

	/**
	 * 新たにデータを作成する。
	 */
	public WordData()
	{
		super(DataType.WORD);
		term = "";
		language = new LanguageSet();
		category = CATEGORY_UNDEFINED;
		scale = 0;
		df = 0;
	}

	/**
	 * 新たにデータを作成する。
	 *
	 * @param value_	格納する値
	 */
	public WordData(WordData value_)
		throws IllegalArgumentException
	{
		super(DataType.WORD);
		term = value_.getTerm();
		language = new LanguageSet();
		language.set(value_.getLanguage());
		category = value_.getCategory();
		scale = value_.getScale();
		df = value_.getDocumentFrequency();
	}

	/**
	 * 新たにデータを作成する。
	 *
	 * @param term_	格納する値
	 */
	public WordData(String term_)
	{
		super(DataType.WORD);
		term = term_;
		language = new LanguageSet();
		category = CATEGORY_UNDEFINED;
		scale = 0;
		df = 0;
	}

	/** 単語を得る */
	public String getTerm()
		{ return term; }
	/** 単語を設定する */
	public void setTerm(String term)
		{ this.term = term; }

	/** 言語文字列を得る */
	public String getLanguageString()
		{ return language.toString(); }
	/** 言語を文字列で設定する */
	public void setLanguageString(String lang) throws IllegalArgumentException
		{ language.set(lang); }
	/** 言語を得る */
	public LanguageSet getLanguage()
		{ return language; }
	/** 言語を設定する */
	public void setLanguage(LanguageSet lang) throws IllegalArgumentException
		{ language.set(lang); }

	/** カテゴリを得る */
	public int getCategory()
		{ return category; }
	/** カテゴリを設定する */
	public void setCategory(int category)
		{ this.category = category; }

	/** スケールを得る */
	public double getScale()
		{ return scale; }
	/** スケールを設定する */
	public void setScale(double scale)
		{ this.scale = scale; }

	/** 文書頻度を得る */
	public int getDocumentFrequency()
		{ return df; }
	/** 文書頻度を設定する */
	public void setDocumentFrequency(int df)
		{ this.df = df; }

	/**
	 * ストリームから読み込む
	 *
	 * @param input_	入力用のストリーム
	 * @throws	java.io.IOException
	 *			入出力関係の例外が発生した
	 * @see Serializable#readObject(InputStream) readObject
	 */
	public void readObject(InputStream input_)
		throws java.io.IOException
	{
		term = UnicodeString.readObject(input_);
		language.readObject(input_);
		category = input_.readInt();
		scale = input_.readDouble();
		df = input_.readInt();
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
		UnicodeString.writeObject(output_, term);
		language.writeObject(output_);
		output_.writeInt(category);
		output_.writeDouble(scale);
		output_.writeInt(df);
	}

	/**
	 * {@link ClassID クラスID}を得る
	 *
	 * @return {@link ClassID クラスID}
	 * @see Serializable#getClassID() getClassID
	 */
	public int getClassID()
	{
		return ClassID.WORD_DATA;
	}

	/**
	 * データが等しいか比較する
	 *
	 * @return	等しい場合はtrue、それ以外の場合はfalse
	 */
	public boolean equals(Object other_)
	{
		boolean result = false;
		if ((other_ instanceof WordData) == true)
		{
			WordData o = (WordData)other_;
			result = (getTerm().equals(o.getTerm())
					  && getLanguage().equals(o.getLanguage())
					  && getCategory() == o.getCategory()
					  && getScale() == o.getScale()
					  && getDocumentFrequency() == o.getDocumentFrequency());
		}
		return result;
	}

	/**
	 * ハッシュコードを得る
	 */
	public int hashCode()
	{
		return ((getLanguage().hashCode() + getCategory()) << 4 |
				getTerm().hashCode());
	}

	/**
	 * オブジェクトのコピーを作成して返す
	 *
	 * @return	コピーされたオブジェクト
	 */
	public Object clone()
	{
		return new WordData(this);
	}

	/**
	 * 文字列に変換する
	 *
	 * @return	文字列
	 */
	public String toString()
	{
		String s;
		if (category != CATEGORY_UNDEFINED)
		{
			// word(xxx) 用
			s = String.format(
				"'%s' language '%s' category '%s' scale %3.2f df %d",
				term, language.toString(), CATEGORY_STRING[category], scale,
				df);
		}
		else
		{
			// cluster(xxx).keryword 用
			s = String.format("'%s' scale %3.2f", term, scale);
		}
		return s;
	}
}

//
// Copyright (c) 2004, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
