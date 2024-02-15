// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ContainsExpression.java
// 
// Copyright (c) 2007, 2010, 2011, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.hibernate.criterion;

import java.util.Collection;

import org.hibernate.Criteria;
import org.hibernate.HibernateException;
import org.hibernate.criterion.CriteriaQuery;
import org.hibernate.criterion.Criterion;
import org.hibernate.criterion.DetachedCriteria;
import org.hibernate.engine.spi.TypedValue;

/**
 * Doquedbの全文検索用演算子である"contains"を表現するためのクラスです。
 */
public class ContainsExpression implements Criterion
{
	private static final TypedValue[] NO_VALUES = new TypedValue[0];

	/** スコア計算器 */
	public static final String TFIDF = "TfIdf";
	public static final String NORMALIZED_TFIDF = "NormalizedTfIdf";
	public static final String OKAPI_TFIDF = "OkapiTfIdf";
	public static final String NORMALIZED_OKAPI_TFIDF = "NormalizedOkapiTfIdf";
	public static final String OKAPI_TF = "OkapiTf";
	public static final String NORMALIZED_OKAPI_TF = "NormalizedOkapiTf";

	/** プロパティ名 */
	private final String propertyName;
	/** 条件 */
	private final ContainsValue value;
	/** スコア計算器 */
	private String scoreCalculator;
	/** 平均文書長 */
	private Double averageLength;
	/** 文書頻度 */
	private Integer totalCount;
	/** 拡張指定 */
	private ExpandSubqueryExpression expand;
	/** 抽出指定 */
	private String extractor;

	/** コンストラクタです */
	protected ContainsExpression(String propertyName, String value)
	{
		this(propertyName, new SimpleContainsValue(value));
	}

	/** コンストラクタです */
	protected ContainsExpression(String propertyName, ContainsValue value)
	{
		this.propertyName = propertyName;
		this.value = value;
		this.scoreCalculator = null;
		this.averageLength = null;
		this.totalCount = null;
		this.expand = null;
		this.extractor = null;
	}

	/** クエリ拡張のためのサブクエリを設定します */
	public ContainsExpression expand(DetachedCriteria expand)
	{
		this.expand = new ExpandSubqueryExpression(expand);
		return this;
	}

	/** スコア計算器を設定します */
	public ContainsExpression scoreCalculator(String scoreCalculator)
	{
		this.scoreCalculator = scoreCalculator;
		return this;
	}

	/** 平均文書長を設定します */
	public ContainsExpression averageLength(double averageLength)
	{
		this.averageLength = new Double(averageLength);
		return this;
	}

	/** 総文書数を設定します */
	public ContainsExpression totalCount(int totalCount)
	{
		this.totalCount = new Integer(totalCount);
		return this;
	}

	/** 抽出指定を設定します */
	public ContainsExpression extractor(String extractor)
	{
		this.extractor = extractor;
		return this;
	}

	/**
	 * SQL文を生成します。
	 */
	public String toSqlString(Criteria criteria, CriteriaQuery criteriaQuery)
		throws HibernateException
	{
		// プロパティーに対応するカラムを取り出す
		String[] columns
			= criteriaQuery.getColumnsUsingProjection(criteria, propertyName);
		if (columns.length != 1)
			throw new HibernateException("not supported");

		// SQL文を格納するバッファ
		StringBuilder buf = new StringBuilder();

		// カラム名 + contains
		buf.append(columns[0]).append(" contains ");
		// 条件
		buf.append(value.toSqlString(criteria, criteriaQuery));
		// スコア計算器
		if (scoreCalculator != null)
			buf.append(" calculator ")
				.append(StringHelper.toSqlString(scoreCalculator));
		// 平均文書長
		if (averageLength != null)
			buf.append(" average length ").append(averageLength.toString());
		// 文書頻度
		if (totalCount != null)
			buf.append(" df ").append(totalCount.toString());
		// 拡張指定
		if (expand != null)
			buf.append(" expand")
				.append(expand.toSqlString(criteria, criteriaQuery));
		// 抽出指定
		if (extractor != null)
			buf.append(" extractor ")
				.append(StringHelper.toSqlString(extractor));

		return buf.toString();
	}

	/**
	 * プレースホルダー '?' に対応するオブジェクトを返します。
	 */
	public TypedValue[] getTypedValues(Criteria criteria,
									   CriteriaQuery criteriaQuery)
		throws HibernateException
	{
		if (expand != null)
			return expand.getTypedValues(criteria, criteriaQuery);
		else
			return NO_VALUES;
	}

	/**
	 * 文字列表現を取り出します
	 */
	public String toString()
	{
		return propertyName + " contains " + value.toString()
			+ " calculator "
			+ ((scoreCalculator == null) ? "null" : scoreCalculator)
			+ " average length "
			+ ((averageLength == null) ? "null" : averageLength.toString())
			+ " df "
			+ ((totalCount == null) ? "null" : totalCount.toString())
			+ " expand "
			+ ((expand == null) ? "null" : expand.toString())
			+ " extractor "
			+ ((extractor == null) ? "null" : extractor);
	}

	/** 単純なキーワード検索用の条件を作成します */
	public static SimpleContainsValue term(String term)
	{
		return new SimpleContainsValue(term);
	}
	public static SimpleContainsValue term(String term, String language)
	{
		return term(term).language(language);
	}

	/** フリーテキスト検索用の条件を作成します */
	public static FreeText freeText(String freeText)
	{
		return new FreeText(freeText);
	}
	public static FreeText freeText(String freeText, String language)
	{
		return freeText(freeText).language(language);
	}

	/** ワードリスト検索用の条件を作成します */
	public static WordList wordList()
	{
		return new WordList();
	}
	public static WordList wordList(WordTerm[] words)
	{
		return wordList().add(words);
	}
	public static WordList wordList(Collection words)
	{
		return wordList().add(words);
	}

	/** 論理積の条件を作成します */
	public static ContainsValueJunction and()
	{
		return new ContainsValueJunction("&");
	}
	public static ContainsValueJunction and(String term1, String term2)
	{
		return and().add(term(term1)).add(term(term2));
	}
	public static ContainsValueJunction and(String term1, String lang1,
											String term2, String lang2)
	{
		return and().add(term(term1, lang1)).add(term(term2, lang2));
	}
	public static ContainsValueJunction and(ContainsValue term1,
											ContainsValue term2)
	{
		return and().add(term1).add(term2);
	}
	public static ContainsValueJunction and(Collection list)
	{
		return and().add(list);
	}

	/** 論理和の条件を作成します */
	public static ContainsValueJunction or()
	{
		return new ContainsValueJunction("|");
	}
	public static ContainsValueJunction or(String term1, String term2)
	{
		return or().add(term(term1)).add(term(term2));
	}
	public static ContainsValueJunction or(String term1, String lang1,
										   String term2, String lang2)
	{
		return or().add(term(term1, lang1)).add(term(term2, lang2));
	}
	public static ContainsValueJunction or(ContainsValue term1,
										   ContainsValue term2)
	{
		return or().add(term1).add(term2);
	}
	public static ContainsValueJunction or(Collection list)
	{
		return or().add(list);
	}

	/** 論理差の条件を作成します */
	public static ContainsValueBinary andnot()
	{
		return new ContainsValueBinary("-");
	}
	public static ContainsValueBinary andnot(String term1, String term2)
	{
		return andnot().left(term(term1)).right(term(term2));
	}
	public static ContainsValueBinary andnot(String term1, String lang1,
											 String term2, String lang2)
	{
		return andnot().left(term(term1, lang1)).right(term(term2, lang2));
	}
	public static ContainsValueBinary andnot(ContainsValue term1,
											 ContainsValue term2)
	{
		return andnot().left(term1).right(term2);
	}

	/** 前方一致 */
	public static PartMatch head(String term)
	{
		return new PartMatch(term(term), PartMatch.HEAD);
	}
	public static PartMatch head(String term, String lang)
	{
		return new PartMatch(term(term, lang), PartMatch.HEAD);
	}
	public static PartMatch head(ContainsValue condition)
	{
		return new PartMatch(condition, PartMatch.HEAD);
	}

	/** 後方一致 */
	public static PartMatch tail(String term)
	{
		return new PartMatch(term(term), PartMatch.TAIL);
	}
	public static PartMatch tail(String term, String lang)
	{
		return new PartMatch(term(term, lang), PartMatch.TAIL);
	}
	public static PartMatch tail(ContainsValue condition)
	{
		return new PartMatch(condition, PartMatch.TAIL);
	}

	/** 重み指定 */
	public static Weight weight(String term, double scale)
	{
		return new Weight(term(term), scale);
	}
	public static Weight weight(String term, String lang, double scale)
	{
		return new Weight(term(term, lang), scale);
	}
	public static Weight weight(ContainsValue condition, double scale)
	{
		return new Weight(condition, scale);
	}

	/** 完全単語一致 */
	public static ContainsValueOption exactWord(String term)
	{
		return new ContainsValueOption(term(term),
									   ContainsValueOption.EXACTWORD);
	}
	public static ContainsValueOption exactWord(String term, String lang)
	{
		return new ContainsValueOption(term(term, lang),
									   ContainsValueOption.EXACTWORD);
	}
	public static ContainsValueOption exactWord(SimpleContainsValue term)
	{
		return new ContainsValueOption(term, ContainsValueOption.EXACTWORD);
	}

	/** 単純単語一致 */
	public static ContainsValueOption simpleWord(String term)
	{
		return new ContainsValueOption(term(term),
									   ContainsValueOption.SIMPLEWORD);
	}
	public static ContainsValueOption simpleWord(String term, String language)
	{
		return new ContainsValueOption(term(term, language),
									   ContainsValueOption.SIMPLEWORD);
	}
	public static ContainsValueOption simpleWord(SimpleContainsValue term)
	{
		return new ContainsValueOption(term, ContainsValueOption.SIMPLEWORD);
	}

	/** 文字列一致 */
	public static ContainsValueOption string(String term)
	{
		return new ContainsValueOption(term(term),
									   ContainsValueOption.STRING);
	}
	public static ContainsValueOption string(String term, String language)
	{
		return new ContainsValueOption(term(term, language),
									   ContainsValueOption.STRING);
	}
	public static ContainsValueOption string(SimpleContainsValue term)
	{
		return new ContainsValueOption(term, ContainsValueOption.STRING);
	}

	/** 先頭が単語境界と一致 */
	public static ContainsValueOption wordHead(String term)
	{
		return new ContainsValueOption(term(term),
									   ContainsValueOption.WORDHEAD);
	}
	public static ContainsValueOption wordHead(String term, String language)
	{
		return new ContainsValueOption(term(term, language),
									   ContainsValueOption.WORDHEAD);
	}
	public static ContainsValueOption wordHead(SimpleContainsValue term)
	{
		return new ContainsValueOption(term, ContainsValueOption.WORDHEAD);
	}

	/** 末尾が単語境界と一致 */
	public static ContainsValueOption wordTail(String term)
	{
		return new ContainsValueOption(term(term),
									   ContainsValueOption.WORDTAIL);
	}
	public static ContainsValueOption wordTail(String term, String language)
	{
		return new ContainsValueOption(term(term, language),
									   ContainsValueOption.WORDTAIL);
	}
	public static ContainsValueOption wordTail(SimpleContainsValue term)
	{
		return new ContainsValueOption(term, ContainsValueOption.WORDTAIL);
	}

	/** 近傍検索 */
	public static Within withinSymmetric(ContainsValue[] values,
										 int upper)
	{
		return new Within(values, upper).order(Within.SYMMETRIC);
	}
	public static Within withinSymmetric(Collection list, int upper)
	{
		return withinSymmetric(
			(ContainsValue[])list.toArray(new ContainsValue[0]), upper);
	}
	public static Within withinSymmetric(ContainsValue[] values,
								  int upper, int lower)
	{
		return withinSymmetric(values, upper).lower(lower);
	}
	public static Within withinSymmetric(Collection list, int upper, int lower)
	{
		return withinSymmetric(list, upper).lower(lower);
	}

	public static Within withinAsymmetric(ContainsValue[] values,
										  int upper)
	{
		return new Within(values, upper).order(Within.ASYMMETRIC);
	}
	public static Within withinAsymmetric(Collection list, int upper)
	{
		return withinAsymmetric(
			(ContainsValue[])list.toArray(new ContainsValue[0]), upper);
	}
	public static Within withinAsymmetric(ContainsValue[] values,
								  int upper, int lower)
	{
		return withinAsymmetric(values, upper).lower(lower);
	}
	public static Within withinAsymmetric(Collection list, int upper, int lower)
	{
		return withinAsymmetric(list, upper).lower(lower);
	}

	/** ワードターム */
	public static WordTerm wordTerm(String term)
	{
		return new WordTerm(term);
	}

}

//
// Copyright (c) 2007, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
