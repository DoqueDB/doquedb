// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SQLScanner.cpp -- SQL 文トークナイザSentence Torcnaiza
// 
// Copyright (c) 1999, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

namespace {
const char moduleName[] = "Statement";
const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Statement/SQLScanner.h"
#include "Statement/Module.h"
#include "Statement/Token.h"
#include "Statement/Utility.h"

#include "Common/Assert.h"
#include "Common/UnicodeString.h"
#include "Exception/SQLSyntaxError.h"

#include "ModUnicodeOstrStream.h"
#include "ModUnicodeCharTrait.h"
#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

#include "Statement/SQLParserL.h"

namespace
{
}

//
//	FUNCTION public
//		Statement::SQLScanner::getLine -- 行番号を得るThe line - number is obtained. 
//
//	NOTES
//		行番号を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		int
//
//	EXCEPTIONS
//		なし
//
int
SQLScanner::getLine() const
{
	return m_iLine;
}

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

namespace _Keyword
{
	typedef Utility::NameTable::Map Map;
	typedef Utility::NameTable::Entry Entry;

	Map			_cMap;

	Entry _keyWords[] = {
			{ "ABS",				TOKEN__ABS				},
			{ "ADD",				TOKEN__ADD				},
			{ "ALL",				TOKEN__ALL				},
			{ "ALTER",				TOKEN__ALTER			},
			{ "ALWAYS",				TOKEN__ALWAYS			},
			{ "AND",				TOKEN__AND				},
//			{ "APPROXIMATEWORD",	TOKEN__APPROXIMATEWORD	},
			{ "AREA",				TOKEN__AREA				},
			{ "ARRAY",				TOKEN__ARRAY			},
			{ "AS",					TOKEN__AS				},
			{ "ASC",				TOKEN__ASC				},
			{ "ASYMMETRIC",			TOKEN__ASYMMETRIC		},
			{ "AVERAGE",			TOKEN__AVERAGE			},
			{ "AVG",				TOKEN__AVG				},
			{ "BACKUP",				TOKEN__BACKUP			},
			{ "BETWEEN",			TOKEN__BETWEEN			},
			{ "BIGINT",				TOKEN__BIGINT			},
			{ "BINARY",				TOKEN__BINARY			},
			{ "BITMAP",				TOKEN__BITMAP			},
			{ "BLOB",				TOKEN__BLOB				},
			{ "BY",					TOKEN__BY				},
            { "CALCULATOR",			TOKEN__CALCULATOR		},
            { "CASCADE",			TOKEN__CASCADE			},
            { "CASE",				TOKEN__CASE			},
            { "CATEGORY",			TOKEN__CATEGORY			},
			{ "CHAR",				TOKEN__CHAR				},
//			{ "CHAR_LENGTH",		TOKEN__CHAR_LENGTH		},
			{ "CHECKPOINT",			TOKEN__CHECKPOINT		},
			{ "CLUSTERED",			TOKEN__CLUSTERED		},
			{ "COLUMN",				TOKEN__COLUMN			},
			{ "COMBINER",			TOKEN__COMBINER			},
			{ "COMMIT",				TOKEN__COMMIT			},
			{ "COMMITTED",			TOKEN__COMMITTED		},
			{ "CONSTANT",			TOKEN__CONSTANT			},
			{ "CONTAINS",			TOKEN__CONTAINS			},
            { "CONTINUE",			TOKEN__CONTINUE			},
            { "CORRECT",			TOKEN__CORRECT			},
            { "CORRESPONDING",		TOKEN__CORRESPONDING	},
		    { "CONNECTION",			TOKEN__CONNECTION		},
			{ "CONST",				TOKEN__CONST			},
			{ "CONSTANT",			TOKEN__CONSTANT			},
			{ "COUNT",				TOKEN__COUNT			},
			{ "CREATE",				TOKEN__CREATE			},
			{ "CROSS",				TOKEN__CROSS			},
//			{ "CURRENT",			TOKEN__CURRENT			},
			{ "CURRENT_DATE",		TOKEN__CURRENT_DATE		},
			{ "CURRENT_PATH",		TOKEN__CURRENT_PATH		},
			{ "CURRENT_TIMESTAMP",	TOKEN__CURRENT_TIMESTAMP},
			{ "CURRENT_USER",		TOKEN__CURRENT_USER		},
//			{ "CURSOR",				TOKEN__CURSOR			},
			{ "CYCLE",				TOKEN__CYCLE			},
			{ "DATE",				TOKEN__DATE				},
			{ "DATABASE",			TOKEN__DATABASE			},
			{ "DATETIME",			TOKEN__DATETIME			},
			{ "DEC",		     	TOKEN__DEC	    		},
			{ "DECIMAL",			TOKEN__DECIMAL	    	},
			{ "DEFAULT",			TOKEN__DEFAULT			},
			{ "DECLARE",			TOKEN__DECLARE			},			
			{ "DELETE",				TOKEN__DELETE			},
			{ "DESC",				TOKEN__DESC				},
			{ "DF",					TOKEN__DF				},
            { "DISCARD",			TOKEN__DISCARD			},
		    { "DISCONNECT",			TOKEN__DISCONNECT		},
			{ "DISTINCT",			TOKEN__DISTINCT			},
			{ "DROP",				TOKEN__DROP				},
			{ "ELLIPSIS",			TOKEN__ELLIPSIS			},
			{ "ELSE",				TOKEN__ELSE				},
			{ "ENCLOSE",			TOKEN__ENCLOSE			},
			{ "END",				TOKEN__END				},
			{ "ESCAPE",				TOKEN__ESCAPE			},
			{ "EQUALS",				TOKEN__EQUALS			}, // =, ==とも同じ
			{ "EXACTWORD",			TOKEN__EXACTWORD		},
			{ "EXCEPT",				TOKEN__EXCEPT			},
			{ "EXECUTE",			TOKEN__EXECUTE			},
			{ "EXISTS",				TOKEN__EXISTS			},
			{ "EXPAND",				TOKEN__EXPAND			},
			{ "EXPAND_SYNONYM",		TOKEN__EXPAND_SYNONYM	},
			{ "EXPLAIN",			TOKEN__EXPLAIN			},
			{ "EXTRACTOR",			TOKEN__EXTRACTOR		},
			{ "FILE",				TOKEN__FILE				},
//			{ "FILESIZE",			TOKEN__FILESIZE			},
			{ "FLOAT",				TOKEN__FLOAT			},
			{ "FOR",				TOKEN__FOR				},
			{ "FOREIGN",			TOKEN__FOREIGN			},
			{ "FORGET",				TOKEN__FORGET			},
			{ "FREETEXT",			TOKEN__FREETEXT			},
			{ "FROM",				TOKEN__FROM				},
			{ "FULL",				TOKEN__FULL				},
			{ "FULLTEXT",			TOKEN__FULLTEXT			},
			{ "FUNCTION",			TOKEN__FUNCTION			},
			{ "GENERATED",			TOKEN__GENERATED		},
			{ "GET",				TOKEN__GET				},
			{ "GLOBAL",				TOKEN__GLOBAL			},
			{ "GRANT",				TOKEN__GRANT			},
			{ "GROUP",				TOKEN__GROUP			},
			{ "GO",					TOKEN__SEMICOLON		}, // SQLサーバーライク
			{ "HAVING",				TOKEN__HAVING			},
			{ "HEAD", 				TOKEN__HEAD				},
			{ "HEAP",				TOKEN__HEAP				},
			{ "HINT",				TOKEN__HINT				},
			{ "IDENTITY",			TOKEN__IDENTITY			},
			{ "IF",					TOKEN__IF				},
			{ "IMAGE",				TOKEN__IMAGE			},
			{ "IN",					TOKEN__IN				},
			{ "INCREMENT",			TOKEN__INCREMENT		},
			{ "INDEX",				TOKEN__INDEX			},
			{ "INNER",				TOKEN__INNER			},
			{ "INPUT",				TOKEN__INPUT			},
			{ "INSERT",				TOKEN__INSERT			},
			{ "INT",				TOKEN__INT				},
			{ "INTERSECT",			TOKEN__INTERSECT		},
			{ "INTO",				TOKEN__INTO				},
			{ "IS",					TOKEN__IS				},
			{ "ISOLATION",			TOKEN__ISOLATION		},
			{ "JOIN",				TOKEN__JOIN				},
			{ "KDTREE",				TOKEN__KDTREE			},
			{ "KEY",				TOKEN__KEY				},
			{ "KWIC",				TOKEN__KWIC				},
			{ "LANGUAGE",			TOKEN__LANGUAGE			},
			{ "LEFT",				TOKEN__LEFT				},
			{ "LENGTH",				TOKEN__LENGTH			},
			{ "LEVEL",				TOKEN__LEVEL			},
			{ "LIKE",				TOKEN__LIKE				},
			{ "LIMIT",				TOKEN__LIMIT			},
			{ "LOCAL",				TOKEN__LOCAL			},
			{ "LOGICALLOG",			TOKEN__LOGICALLOG		},
			{ "LOWER",				TOKEN__LOWER			},
			{ "MASTER",				TOKEN__MASTER			},
			{ "MAX",				TOKEN__MAX				},
			{ "MAXVALUE",			TOKEN__MAXVALUE			},
//			{ "METADATA",			TOKEN__METADATA			},
			{ "MIGRATE",			TOKEN__MIGRATE			},
			{ "MIN",				TOKEN__MIN				},
			{ "MINVALUE",			TOKEN__MINVALUE			},
			{ "MOD",				TOKEN__MOD				},
			{ "MODIFY",				TOKEN__MODIFY			},
			{ "MOUNT",				TOKEN__MOUNT			},
			{ "NATURAL",			TOKEN__NATURAL			},
			{ "NCHAR",				TOKEN__NCHAR			},
			{ "NCLOB",				TOKEN__NCLOB			},
			{ "NO",					TOKEN__NO				},
			{ "NONCLUSTERED",		TOKEN__NONCLUSTERED		},
			{ "NONTRUNCATE",		TOKEN__NONTRUNCATE		},
//			{ "NONE",				TOKEN__NONE				},
			{ "NORMALIZE",			TOKEN__NORMALIZE		},
			{ "NOT",				TOKEN__NOT				},
			{ "NTEXT",				TOKEN__NTEXT			},
			{ "NULL",				TOKEN__NULL				},
			{ "NUMERIC",			TOKEN__NUMERIC	    	},
			{ "NVARCHAR",			TOKEN__NVARCHAR			},
//			{ "OCTET_LENGTH",		TOKEN__OCTET_LENGTH		},
			{ "OF",					TOKEN__OF				},
			{ "OFFLINE",			TOKEN__OFFLINE			},
			{ "OFFSET",				TOKEN__OFFSET			},
			{ "ON",					TOKEN__ON				},
			{ "ONE",				TOKEN__ONE				},
			{ "ONLINE",				TOKEN__ONLINE			},
			{ "ONLY",				TOKEN__ONLY				},
			{ "OR",					TOKEN__OR				},
			{ "ORDER",				TOKEN__ORDER			},
			{ "OUTER",				TOKEN__OUTER			},
			{ "OUTPUT",				TOKEN__OUTPUT			},
			{ "OVERLAY",			TOKEN__OVERLAY			},
			{ "PARAMETER",			TOKEN__PARAMETER		},
			{ "PARTITION",			TOKEN__PARTITION		},
//			{ "PASSWORD",			TOKEN__PASSWORD			},
			{ "PATH",				TOKEN__PATH				},
			{ "PLACING",			TOKEN__PLACING			},
			{ "PREPARE",			TOKEN__PREPARE			},
			{ "PRIMARY",			TOKEN__PRIMARY			},
			{ "PHASE",				TOKEN__PHASE			},
			{ "PHYSICALLOG",		TOKEN__PHYSICALLOG		},
			{ "RANK",				TOKEN__RANK,			},
			{ "READ",				TOKEN__READ,			},
			{ "RECOVER",			TOKEN__RECOVER,			},
			{ "RECOVERY",			TOKEN__RECOVERY,		},
			{ "REFERENCES",			TOKEN__REFERENCES,		},
			{ "RENAME",				TOKEN__RENAME,			},
			{ "REPEATABLE",			TOKEN__REPEATABLE		},
			{ "RESUME",				TOKEN__RESUME			},
			{ "RETURN",				TOKEN__RETURN			},
			{ "RETURNS",			TOKEN__RETURNS			},
			{ "REVOKE",				TOKEN__REVOKE			},
			{ "RIGHT",				TOKEN__RIGHT			},
			{ "ROLLBACK",			TOKEN__ROLLBACK			},
			{ "ROW",				TOKEN__ROW				},
			{ "ROWS",				TOKEN__ROWS				},
			{ "SCALE",				TOKEN__SCALE			},
			{ "SCORE",				TOKEN__SCORE			},
			{ "SECTIONIZED",		TOKEN__SECTIONIZED		},
			{ "SELECT",				TOKEN__SELECT			},
			{ "SERIALIZABLE",		TOKEN__SERIALIZABLE		},
		    { "SESSION",			TOKEN__SESSION			},
			{ "SESSION_USER",		TOKEN__SESSION_USER		},
			{ "SET",				TOKEN__SET				},
			{ "SIMILAR",			TOKEN__SIMILAR			},
			{ "SIMPLEWORD",			TOKEN__SIMPLEWORD		},
//			{ "SOME",				TOKEN__SOME				},
            { "SLAVE",				TOKEN__SLAVE			},
            { "SNAPSHOT",			TOKEN__SNAPSHOT			},
            { "START",				TOKEN__START			},
            { "STOP",				TOKEN__STOP				},
			{ "STRING",				TOKEN__CONTAINS_STRING	},
			{ "SUBSTRING",			TOKEN__SUBSTRING		},
			{ "SUM",				TOKEN__SUM				},
			{ "SUSPEND",			TOKEN__SUSPEND			},
			{ "SYSTEM",				TOKEN__SYSTEM			},
			{ "SYMMETRIC",			TOKEN__SYMMETRIC		},
			{ "SYNC",				TOKEN__SYNC				},
			{ "SYNONYM",			TOKEN__SYNONYM			},
			{ "TABLE",				TOKEN__TABLE			},
			{ "TAIL",				TOKEN__TAIL				},
			{ "TEMPORARY",			TOKEN__TEMPORARY		},
			{ "THEN",				TOKEN__THEN				},
//			{ "TIME",				TOKEN__TIME				},
//			{ "TF",					TOKEN__TF				},
			{ "TIMESTAMP",			TOKEN__TIMESTAMP		},
			{ "TO",					TOKEN__TO				},
			{ "TRANSACTION",		TOKEN__TRANSACTION		},
			{ "TRIGGER",			TOKEN__TRIGGER			},
			{ "UNCOMMITTED",		TOKEN__UNCOMMITTED		},
			{ "UNION",				TOKEN__UNION			},
			{ "UNIQUE",				TOKEN__UNIQUE			},
			{ "UNIQUEIDENTIFIER",	TOKEN__UNIQUEIDENTIFIER	},
			{ "UPDATABLE",			TOKEN__UPDATABLE		},
			{ "UPDATE",				TOKEN__UPDATE			},
			{ "UPPER",				TOKEN__UPPER			},
			{ "USER",				TOKEN__USER				},
			{ "USING", 				TOKEN__USING			},
			{ "UNMOUNT",			TOKEN__UNMOUNT			},
			{ "UNNEST",				TOKEN__UNNEST			},
			{ "VALUE",				TOKEN__VALUE			},
			{ "VALUES",				TOKEN__VALUES			},
            { "VARCHAR",			TOKEN__VARCHAR			},
            { "VERBOSE", 			TOKEN__VERBOSE			},
            { "VERIFY", 			TOKEN__VERIFY			},
            { "WEIGHT", 			TOKEN__WEIGHT			},
            { "WHEN",				TOKEN__WHEN			},
            { "WHERE",				TOKEN__WHERE			},
            { "WITH",				TOKEN__WITH				},
            { "WITHIN",				TOKEN__WITHIN			},
			{ "WORD",				TOKEN__WORD				},
			{ "WORDHEAD",			TOKEN__WORDHEAD			},
			{ "WORDLIST",			TOKEN__WORDLIST			},
			{ "WORDTAIL",			TOKEN__WORDTAIL			},
			{ "WORK",				TOKEN__WORK				},
			{ "WRITE",				TOKEN__WRITE			},
			{ "XA",					TOKEN__XA				},
			{ 0, 0 }
	};

	// 与えられた文字列が対応するキーワードエントリーがあれば返す
	//It returns it if there is a key word entry where the given character string corresponds. 
	const Entry* getElement(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_)
	{
		return _cMap.getEntry(&_keyWords[0], pHead_, pTail_);
	}
} // namespace _Keyword

_SYDNEY_STATEMENT_END
_SYDNEY_END

//
//	FUNCTION public
//		Statement::SQLScanner::getNextToken -- 次のトークンを得る
//
//	NOTES
//		次のトークンを得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Token*
//
//	EXCEPTIONS
//		なし
//
Token*
SQLScanner::getNextToken()
{
	using namespace Common::UnicodeChar;

	// 対応するSQL文を得るために、トークンの位置を覚えておくTo obtain the corresponding SQL sentence, the position of the token is remembered. 
	m_psz2ndLastEnd = m_pszNext;

	if (!m_pszNext) {
		// 文字列がセットされていない!The character string is not set. 
		m_bIsEOF = true;
		// fall thru
	}
  Again:
	// skip leading whitespace
	while (!m_bIsEOF &&
		   *m_pszNext &&
		   isSpace(*m_pszNext)) {
		// 行頭かどうかを覚えておく(リターン識別)It is remembered whether it is the head of line. (return identification)
		if (*m_pszNext == usCtrlRet) {
			++m_iLine;
			m_bIsBOL = true;
		} else {
			m_bIsBOL = false;
		}
		++m_pszNext;
	}

	// 空白スキップ完了の位置を覚える
	m_pszLastBeg = m_pszNext;

	// check eof
	if (!m_bIsEOF && *m_pszNext == usNull) {
		m_bIsEOF = true;
		/* fall through */
	}
	if (m_bIsEOF) {
		return 0;
	}
	// now we have something

	// 行頭から空白をあけずに始まるコメントはThe comment that starts without opening the blank from the head of line
	// ヒントとしてparserに送るIt sends it to parser as a hint. 
	if (m_pszNext[0] == usHyphen && m_bIsBOL &&
		m_pszNext[1] == usHyphen)
	{
		do {
			++m_pszNext;
		} while (*m_pszNext && *m_pszNext != usCtrlRet &&
				 isSpace(*m_pszNext));
		const ModUnicodeChar* beg = m_pszNext;
		do {
			++m_pszNext;
		} while (*m_pszNext && *m_pszNext != usCtrlRet);

		//コメントは Token に追加していたが要望により何もせず読み飛ばす
		//	00/10/17
		// ModUnicodeString cstrComment(beg, m_pszNext - beg);
		// return new Token(TOKEN__COMMENT, cstrComment);
		goto Again;
	}
	m_bIsBOL = false;
	// それ以外the rest
	switch(*m_pszNext) {
	case usLparent:		// '('
		++m_pszNext;
		return new Token(TOKEN__LEFT_PARENTHESIS);
	case usRparent:		// ')'
		++m_pszNext;
		return new Token(TOKEN__RIGHT_PARENTHESIS);
	case usLbracket:	// '['
		++m_pszNext;
		return new Token(TOKEN__LEFT_BRACKET);
	case usRbracket:
		++m_pszNext;
		return new Token(TOKEN__RIGHT_BRACKET);
	case usAsterisc:	// '*'
		++m_pszNext;
		return new Token(TOKEN__ASTERISK);
	case usSemiColon:	// ';':
		++m_pszNext;
		return new Token(TOKEN__SEMICOLON);
/*
	case usColon:		// ':':
		++m_pszNext;
		return new Token(TOKEN__COLON);
*/
	case usComma:		// ',':
		++m_pszNext;
		return new Token(TOKEN__COMMA);
	case usPlus:		// '+':
		++m_pszNext;
		return new Token(TOKEN__PLUS);
	case usHyphen:		// '-':
		++m_pszNext;
		if (*m_pszNext == usHyphen) {
			// コメント
			// 行頭以外から始まるものは本当のコメントThe one that starts excluding the head of line is a true comment. 
			do {
				++m_pszNext;
			} while (*m_pszNext && *m_pszNext != usCtrlRet);
			goto Again;
		}
		return new Token(TOKEN__MINUS, m_pszNext - 1, m_pszNext);
	case usSlash:		// '/':
		++m_pszNext;
		return new Token(TOKEN__SOLIDUS);
	case usEqual:		// '=':
		++m_pszNext;
		if (*m_pszNext == usEqual)
			++m_pszNext;
		return new Token(TOKEN__EQUALS);
	case usLcompare:	// '<':
		++m_pszNext;
		if (*m_pszNext == usRcompare) {
			++m_pszNext;
			return new Token(TOKEN__NOT_EQUALS);
		} else if (*m_pszNext == usEqual) {
			++m_pszNext;
			return new Token(TOKEN__LESS_THAN_OR_EQUALS);
		} else {
			return new Token(TOKEN__LESS_THAN);
		}
	case usRcompare:	// '>':
		++m_pszNext;
		if (*m_pszNext == usEqual) {
			++m_pszNext;
			return new Token(TOKEN__GREATER_THAN_OR_EQUALS);
		} else {
			return new Token(TOKEN__GREATER_THAN);
		}
	case usExclamation:	// '!':
		if (m_pszNext[1] == usEqual) {
			m_pszNext += 2;
			return new Token(TOKEN__NOT_EQUALS);
		}
		break;
	case usPeriod:		// '.':
		// .の後に数字があればfloat定数
		if (ModUnicodeCharTrait::isDigit(m_pszNext[1]))
			goto Numeric;
		++m_pszNext;
		return new Token(TOKEN__PERIOD);
	case usQuestion:	// '?':
		// ?にはSQL文中の出現位置順に通し番号を振る。?The serial number is shaken in appearance position order in SQL sentence. 
		// 実行時にDataArrayDataで渡される値のインデックスと同じになるように、To become it as well as the index of the value passed with DataArrayData when executing it
		// 0から数える。It counts from 0. 
		++m_pszNext;
		return new Token(TOKEN__QUESTION_MARK, m_iPlaceHolder++);
	case usVerticalLine:	// '|'
		if (m_pszNext[1] == usVerticalLine) {
			m_pszNext += 2;
			return new Token(TOKEN__STRING_CONCAT);
		}
		++m_pszNext;
		return new Token(TOKEN__VERTICALLINE);
		break;

	// contains用
	case usAmpersand:	// '&'
		++m_pszNext;
		return new Token(TOKEN__AMPERSAND);

	case usQuate:		// '''
	{
		// 文字列

		const ModUnicodeChar* pHead = ++m_pszNext;
		const ModUnicodeChar* p = pHead;
		int iToken = TOKEN__STRING;
		for (; p < m_pTail; ++p) {
			if (*p == usQuate) {
				if (*(p+1) == usQuate) {
					// ''ならまだ文字列の中
					++p;
					// 識別子を変える
					iToken = TOKEN__STRING_WITH_QUOTE;
					continue;
				}
				// 終り
				break;
			}
		}
		if (p == m_pTail) {
			// 'がないまま文字列が尽きた
			_SYDNEY_THROW1(Exception::SQLSyntaxError, ModUnicodeString("Unfinished string literal"));
		}
		m_pszNext = p + 1;
		return new Token(iToken, pHead, p);
	}
	case usLargeX:		// 'X'
	{
		const ModUnicodeChar* p = m_pszNext;

		// 'x' の直後に空白文字があればすべて読み飛ばす

		while (++p < m_pTail && isSpace(*p)) ;

		if (p < m_pTail && *p == usQuate) {

			// ''' から始まるなにかがある

			const ModUnicodeChar* head = ++p;
			for (; p < m_pTail; ++p) {
				if (*p == usQuate) {

					// ''' で終わったので、中身をバイナリ列とする

					if ((p - head) % 2)

						// 奇数個の16進数を表現する文字が含まれている

						_SYDNEY_THROW1(Exception::SQLSyntaxError,
									_TRMEISTER_U_STRING(
									"Binary string literal is not in octets"));

					m_pszNext = p + 1;
					return new Token(TOKEN__BINARY_STRING, head, p);
				}
				if (!ModUnicodeCharTrait::isXdigit(*p))

					// 16進数を表現する文字以外の文字が含まれている

					break;
			}
		}

		// 'x' から始まるなにかをキーワードか識別子として扱う

		break;
	}
	case usWquate:		// '"'
	{
		// 区切られた識別子

		const ModUnicodeChar* pHead = ++m_pszNext;
		const ModUnicodeChar* p = pHead;
		int iToken = TOKEN__IDENTIFIER;
		for (; p < m_pTail; ++p) {
			if (*p == usWquate) {
				if (*(p+1) == usWquate) {
					// ""ならまだ識別子の中
					++p;
					iToken = TOKEN__IDENTIFIER_WITH_QUOTE;
					continue;
				}
				// 終り
				break;
			}
		}
		if (p == m_pTail) {
			// "がないまま識別子が尽きた
			_SYDNEY_THROW1(Exception::SQLSyntaxError, ModUnicodeString("Unfinished quoted identifier"));
		}
		if (p == pHead) {
			// empty string
			_SYDNEY_THROW1(Exception::SQLSyntaxError, ModUnicodeString("Empty quoted identifier"));
		}
		m_pszNext = p + 1;
		return new Token(iToken, pHead, p);
	}
	}

	// 数値
	if (ModUnicodeCharTrait::isDigit(*m_pszNext)) {
	  Numeric:
		const ModUnicodeChar* p = m_pszNext;
		const ModUnicodeChar* beg = p;
		// 整数部を見るThe integer part is seen. 
		while (ModUnicodeCharTrait::isDigit(*++p)) {}
		if (*p != usPeriod && *p != usSmallE && *p != usLargeE) {
			// 整数
			m_pszNext = p;
			return new Token(TOKEN__INTEGER_LITERAL, beg, p);
		}
		// 小数点をスキップThe decimal point is skipped. 
		if (*p == usPeriod) {
			// 小数部
			while (ModUnicodeCharTrait::isDigit(*++p)) {}
		}
		// 指数部 ('e', 'E')
		if (*p == usSmallE || *p == usLargeE) {
			p++;
			// 指数部符号('-', '+')
			if (*p == usHyphen || *p == usPlus) {
				p++;
			}
			// 指数部
			while (ModUnicodeCharTrait::isDigit(*p)) {
				p++;
			}
		}
		m_pszNext = p;
		{
			return new Token(TOKEN__FLOAT_LITERAL, beg, p);
		}
	}

	// 標準識別子
	//
	//【注意】	ModUnicodeCharTrait::isLetterOther は
	//			UNICODE の文字種である Lm (Letter, Modifier) を
	//			ModFalse にしてしまう
	//
	//			Lm には 'ー’(U+30FC) も含んでおり、It contains it.
	//			それだけは除外するために特別に処理するTo exclude it, only it is specially processed. 
	//
	//			本当は Lm をすべて処理すべきかもしれないが、Though all Lm might have to be processed in reality
	//			現状では MOD にその手段がないため、行わないIt doesn't do because the means is not in MOD under the present situation. 
	{
	const ModUnicodeChar* start = m_pszNext;

	for (; m_pszNext < m_pTail; ++m_pszNext)
		if (!(ModUnicodeCharTrait::isAlphabet(*m_pszNext) ||
			  ModUnicodeCharTrait::isLetterOther(*m_pszNext) ||
			  *m_pszNext == 0x30fc ||
			  ((m_pszNext == start) ? *m_pszNext == usSharp :
			   ModUnicodeCharTrait::isDigit(*m_pszNext)) ||
			  *m_pszNext == usLowLine))
			break;

	int tokenID;
	return new Token((isKeywordRegisted(start, m_pszNext, tokenID)) ?
					 tokenID : TOKEN__IDENTIFIER,
				 start, m_pszNext);
	}
}

//
//	FUNCTION public
//		Statement::SQLScanner::getLastTokenBeginning()
//
//	NOTES
//		最後に読んだトークンの先頭位置を返すThe first position of the token read at the end is returned. 
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		const ModUnicodeChar* 
//
//	EXCEPTIONS
//		なし
//
const ModUnicodeChar*
SQLScanner::getLastTokenBeginning() const
{
	return m_pszLastBeg;
}

//
//	FUNCTION public
//		Statement::SQLScanner::getNextPlaceHolder()
//
//	NOTES
//		次のPlaceHolder番号を返す
//
//		placeholder(引数を表わす?)によって文が始まることはないので、
//		getLastPlaceHolder()は必要ない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		int
//
//	EXCEPTIONS
//		なし
//
int
SQLScanner::getNextPlaceHolder() const
{
	return m_iPlaceHolder;
}

//
//	FUNCTION public
//		Statement::SQLScanner::isEOF -- すでにEOFか
//
//	NOTES
//		すでにEOFか
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		bool
//
//	EXCEPTIONS
//		なし
//
bool
SQLScanner::isEOF() const
{
	return m_bIsEOF;
}

// FUNCTION public
//	Statement::SQlScanner::isKeywordRegisted -- キーワード検索
//
// NOTES
//		キーワードが登録されているか検索します
//		この関数が初めて起動された時にキーワードマップを作成します
//The key word is registered or it retrieves it. 
//When this function is started for the first time, the key word map is made. 
//
// ARGUMENTS
//	const ModUnicodeChar* pHead_
//			検索対象文字列の先頭アドレス
//	const ModUnicodeChar* pTail_
//			検索対象文字列の終端アドレス
//	int& iID
//			登録時、Token ID を設定します
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
SQLScanner::
isKeywordRegisted(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_, int& iID)
{
	const _Keyword::Entry* pElement = _Keyword::getElement(pHead_, pTail_);

	if (!pElement) return false;

	iID = pElement->_value;
	return true;
}

//
//	FUNCTION private
//		Statement::SQLScanner::isSpace
//			-- スペース文字判定
//
//	NOTES
//		文字がスペースか判定します
//		ModCharTrait::isSpace と ModUnicodeCharTrait::isSpace の
//		互換を保つ為に存在します
//
//Whether the character is space is judged. 
//It exists to keep the interchangeability of ModCharTrait::isSpace and ModUnicodeCharTrait::isSpace. 
//	ARGUMENTS
//		const ModUnicodeChar suChar
//
//	RETURN
//		ModBoolean
//
//	EXCEPTIONS
//		なし
//
bool
SQLScanner::isSpace(const ModUnicodeChar suChar)
{
	using namespace Common;
	using namespace UnicodeChar;

	return *m_pszNext == usSpace || *m_pszNext == usCtrlTab ||
		*m_pszNext == usCtrlRet	|| *m_pszNext == usCtrlCr;
}

//
//	Copyright (c) 1999, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
