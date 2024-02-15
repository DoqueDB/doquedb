// #T1#
// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
//	RuleParser.tab.cpp -- Implement file of RuleParser.tab
// 
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
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

// #T2#
/* A Bison parser, made by GNU Bison 1.875b.  */

// #T3#
/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 2004-2005, 2023 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

// #T4#
/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

// #T5#
/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

// #T6#
/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

// #T7#
/* Identify Bison output.  */
#define YYBISON 1

// #T8#
/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

// #T9#
/* Pure parsers.  */
#define YYPURE 1

// #T10#
/* Using locations.  */
#define YYLSP_NEEDED 0



// #T11#
/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
// #T12#
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     NL = 300,
     RULE_STRING = 310
   };
#endif
#define NL 300
#define RULE_STRING 310




// #T13#
/* Copy the first part of user declarations.  */
#line 16 "RuleParser.y"

// #T14#
/*---------------------------------------------------------------------------*/
/*  declaration part of C                                                    */
/*---------------------------------------------------------------------------*/

#include "LibUna/RuleElementSet.h"
#include "LibUna/RuleScanner.h"
#include "LibUna/Rule.h"
#include "LibUna/RuleMaker.h"
#include "LibUna/RuleHolder.h"
#include "ModUnicodeString.h"
#include "UnaReinterpretCast.h"

_UNA_BEGIN

// #T15#
// FUNCTION pubic
//	::yylex
//		-- lexcal analyzer for Bison
//
// NOTES
//
// ARGUMENTS
//	RuleElementSet** lvalp_
//		lvalp_ was defined in yyparse() which was not used anywhere.
//	void* param_
//		param_ is LexScanner object which was defined RuleMaker.
//
// RETURN
//	int
// 		token value
//
//	EXCEPTIONS
int
yylex(RuleElementSet** lvalp_, void* param_)
{
	RuleElementSet* set = una_reinterpret_cast<RuleElementSet*>(param_);
	return set->_object._scanner->lex(&set->_data);
}

// #T16#
// managing case command
#pragma warning(disable:  4065)

#define		YYERROR_VERBOSE
#define		YYPARSE_PARAM				_param_
#define		YYLEX_PARAM				_param_
#define		YYSTYPE					RuleElementSet*
// #T17#
//#define	YYSTYPE					ModUnicodeString*

#define		RULE_SCANNER(_elem_)		(_elem_->_object->_scanner)
#define		TMP_STRING(_elem_)			(_elem_->_data._tmpstr)
#define		RULE(_elem_)				(_elem_->_data._rule)
#define		BITSET_ARY(_elem_)			(_elem_->_data._bitsets)
#define		TGT_BITSET(_elem_)			(_elem_->_object._targetresult)
#define		PRETYPE(_elem_)				(_elem_->_data._prestep)
#define		STRARY(_elem_)				(_elem_->_data._strary)

namespace {

// #T18#
	// access number
	void
	SetElement(ModVector<ModUnicodeString>& ary_,
			   const ModUnicodeString& elem_,
			   ModSize n_)
	{
		ModSize max = ary_.getSize();
		if ( n_ >= max ) {
			ary_.insert(ary_.end(), n_ - max, ModUnicodeString());
			ary_.pushBack(elem_);
		} else {
			ary_[n_] = elem_;
		}
	}

// #T19#
} // end of namespace

void yyerror(const char* s_) 
{
	ModErrorMessage << s_ << ModEndl;
}



// #T20#
/* Enabling traces */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

// #T21#
/* Enabling verbose error messages */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
// #T22#
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



// #T23#
/* Copy the second part of user declarations.  */


// #T24#
/* Line 214 of yacc.c.  */
#line 159 "RuleParser.tab.cpp"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

// #T25#
/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
// #T26#
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
// #T27#
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
// #T28#
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYSTYPE_IS_TRIVIAL)))

// #T29#
/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

// #T30#
/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

// #T31#
/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

// #T32#
/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

// #T33#
/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

// #T34#
/* YYFINAL -- State number of the termination state. */
#define YYFINAL  3
// #T35#
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   32

// #T36#
/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  13
// #T37#
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  13
// #T38#
/* YYNRULES -- Number of rules. */
#define YYNRULES  23
// #T39#
/* YYNRULES -- Number of states. */
#define YYNSTATES  40

// #T40#
/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   311

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

// #T41#
/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     6,     2,
      11,    12,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     5,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     8,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     9,     7,    10,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       3,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       4,     2
};

#if YYDEBUG
// #T42#
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     5,     6,     9,    11,    13,    15,    19,
      24,    29,    34,    36,    40,    45,    49,    54,    58,    63,
      65,    67,    69,    71
};

// #T43#
/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      14,     0,    -1,    15,    -1,    -1,    15,    16,    -1,     3,
      -1,    17,    -1,    18,    -1,    21,     5,    18,    -1,    21,
       6,     5,    18,    -1,    21,     7,     5,    18,    -1,    21,
       8,     5,    18,    -1,    19,    -1,    19,     9,    10,    -1,
      19,     9,    25,    10,    -1,    20,     9,    10,    -1,    20,
       9,    24,    10,    -1,    22,    11,    12,    -1,    22,    11,
      23,    12,    -1,     4,    -1,     4,    -1,     4,    -1,     4,
      -1,     4,    -1
};

// #T44#
/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   103,   103,   105,   106,   110,   111,   124,   134,   146,
     157,   168,   182,   186,   190,   197,   201,   207,   211,   220,
     229,   239,   249,   259
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
// #T45#
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NL", "RULE_STRING", "'='", "'&'", "'|'", 
  "'^'", "'{'", "'}'", "'('", "')'", "$accept", "top", "input", "line", 
  "command", "command_right", "command_pos", "command_head", "value_name", 
  "rule_name", "rule_option", "pos_rule", "marker_rule", 0
};
#endif

# ifdef YYPRINT
// #T46#
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   311,   300,   310,    61,    38,   124,    94,   123,
     125,    40,    41
};
# endif

// #T47#
/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    13,    14,    15,    15,    16,    16,    17,    17,    17,
      17,    17,    18,    18,    18,    19,    19,    20,    20,    21,
      22,    23,    24,    25
};

// #T48#
/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     0,     2,     1,     1,     1,     3,     4,
       4,     4,     1,     3,     4,     3,     4,     3,     4,     1,
       1,     1,     1,     1
};

// #T49#
/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       3,     0,     2,     1,     5,    19,     4,     6,     7,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      23,    13,     0,    22,    15,     0,    20,     8,     0,     0,
       0,    21,    17,     0,    14,    16,     9,    10,    11,    18
};

// #T50#
/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,     1,     2,     6,     7,     8,     9,    10,    11,    12,
      33,    25,    22
};

// #T51#
/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -16
static const yysigned_char yypact[] =
{
     -16,     3,     2,   -16,   -16,    -4,   -16,   -16,   -16,    12,
      13,    11,     1,    -2,     0,     7,    15,    18,    19,    -3,
     -16,   -16,    16,   -16,   -16,    17,   -16,   -16,     7,     7,
       7,   -16,   -16,    20,   -16,   -16,   -16,   -16,   -16,   -16
};

// #T52#
/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -16,   -16,   -16,   -16,   -16,   -15,   -16,   -16,   -16,   -16,
     -16,   -16,   -16
};

// #T53#
/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -21
static const yysigned_char yytable[] =
{
      27,    31,    20,     3,    23,     4,     5,   -20,    21,    32,
      24,    26,    19,    36,    37,    38,    15,    16,    17,    18,
      28,    13,    14,    29,    30,     0,    34,    35,     0,     0,
       0,     0,    39
};

static const yysigned_char yycheck[] =
{
      15,     4,     4,     0,     4,     3,     4,    11,    10,    12,
      10,     4,    11,    28,    29,    30,     5,     6,     7,     8,
       5,     9,     9,     5,     5,    -1,    10,    10,    -1,    -1,
      -1,    -1,    12
};

// #T54#
/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    14,    15,     0,     3,     4,    16,    17,    18,    19,
      20,    21,    22,     9,     9,     5,     6,     7,     8,    11,
       4,    10,    25,     4,    10,    24,     4,    18,     5,     5,
       5,     4,    12,    23,    10,    10,    18,    18,    18,    12
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
// #T55#
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1


// #T56#
/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

// #T57#
/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

// #T58#
/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
#endif

// #T59#
/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
// #T60#
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

// #T61#
/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
// #T62#
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


// #T63#
/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
// #T64#
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

// #T65#
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
// #T66#
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
// #T67#
#endif /* !YYDEBUG */


// #T68#
/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

// #T69#
/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
// #T70#
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
// #T71#
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

// #T72#
#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
// #T73#
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
// #T74#
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

// #T75#
#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
// #T76#
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


// #T77#
/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
// #T78#
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
// #T79#
#endif /* ! YYPARSE_PARAM */






// #T80#
/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
// #T81#
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
// #T82#
  /* The lookahead symbol.  */
int yychar;

// #T83#
/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

// #T84#
/* Number of syntax errors so far.  */
int yynerrs;

  register int yystate;
  register int yyn;
  int yyresult;
// #T85#
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
// #T86#
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

// #T87#
  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

// #T88#
  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

// #T89#
  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

// #T90#
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


// #T91#
  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
// #T92#
  yychar = YYEMPTY;		/* Cause a token to be read.  */

// #T93#
  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

// #T94#
/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
// #T95#
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
// #T96#
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
// #T97#
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


// #T98#
	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
// #T99#
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
// #T100#
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
// #T101#
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

// #T102#
/*-----------.
| yybackup.  |
`-----------*/
yybackup:

// #T103#
/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

// #T104#
  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

// #T105#
  /* Not known => get a lookahead token if don't already have one.  */

// #T106#
  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

// #T107#
  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

// #T108#
  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

// #T109#
  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


// #T110#
  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


// #T111#
/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


// #T112#
/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
// #T113#
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

// #T114#
  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 6:
#line 112 "RuleParser.y"
    {
    RuleElementSet* element = una_reinterpret_cast<RuleElementSet*>(_param_);

// #T115#
	// execute of rule maker
	RuleMaker* maker = element->_object._maker;

	maker->execute(&element->_data);

;}
    break;

  case 7:
#line 125 "RuleParser.y"
    {
// #T116#
	// setting present execution status.
	RuleElementSet* elem = una_reinterpret_cast<RuleElementSet*>(_param_);

// #T117#
	// setting string of operator
	TMP_STRING(elem) = ""; // non

    yyval = yyvsp[0];
;}
    break;

  case 8:
#line 135 "RuleParser.y"
    {
// #T118#
	// setting present execution status.
	RuleElementSet* elem = una_reinterpret_cast<RuleElementSet*>(_param_);

// #T119#
	// setting string of operator
	using namespace Type;
	SetElement(STRARY(elem), ModUnicodeString("="), Rule::Step::Operate);


    yyval = yyvsp[-2];
;}
    break;

  case 9:
#line 147 "RuleParser.y"
    {
// #T120#
	// setting present execution status.
	RuleElementSet* elem = una_reinterpret_cast<RuleElementSet*>(_param_);

// #T121#
	// setting string of operator
	using namespace Type;
	SetElement(STRARY(elem), ModUnicodeString("&="), Rule::Step::Operate);

    yyval = yyvsp[-3];
;}
    break;

  case 10:
#line 158 "RuleParser.y"
    {
// #T122#
	// setting present execution status.
	RuleElementSet* elem = una_reinterpret_cast<RuleElementSet*>(_param_);

// #T123#
	// setting string of operator
	using namespace Type;
	SetElement(STRARY(elem), ModUnicodeString("|="), Rule::Step::Operate);

    yyval = yyvsp[-3];
;}
    break;

  case 11:
#line 169 "RuleParser.y"
    {
// #T124#
	// setting present execution status.
	RuleElementSet* elem = una_reinterpret_cast<RuleElementSet*>(_param_);

// #T125#
	// setting string of operator
	using namespace Type;
	SetElement(STRARY(elem), ModUnicodeString("^="), Rule::Step::Operate);

    yyval = yyvsp[-3];
;}
    break;

  case 12:
#line 183 "RuleParser.y"
    {
    yyval = yyvsp[0];
;}
    break;

  case 13:
#line 187 "RuleParser.y"
    {
	yyval = yyvsp[-2];
;}
    break;

  case 14:
#line 191 "RuleParser.y"
    {
    yyval = yyvsp[-3];
;}
    break;

  case 15:
#line 198 "RuleParser.y"
    {
    yyval = yyvsp[-2];
;}
    break;

  case 16:
#line 202 "RuleParser.y"
    {
    yyval = yyvsp[-3];
;}
    break;

  case 17:
#line 208 "RuleParser.y"
    {
	yyval = yyvsp[-2];
;}
    break;

  case 18:
#line 212 "RuleParser.y"
    {
    yyval = yyvsp[-3];
;}
    break;

  case 19:
#line 221 "RuleParser.y"
    {
// #T126#
	// setting present execution status.
	RuleElementSet* elem = una_reinterpret_cast<RuleElementSet*>(_param_);
	SetElement(STRARY(elem), TMP_STRING(elem), Rule::Step::ValueName);

    yyval = yyvsp[0];
;}
    break;

  case 20:
#line 230 "RuleParser.y"
    {
// #T127#
	// setting present execution status.
	RuleElementSet* elem = una_reinterpret_cast<RuleElementSet*>(_param_);
	SetElement(STRARY(elem), TMP_STRING(elem), Rule::Step::RuleType);

    yyval = yyvsp[0];
;}
    break;

  case 21:
#line 240 "RuleParser.y"
    {
// #T128#
	// setting present execution status.
	RuleElementSet* elem = una_reinterpret_cast<RuleElementSet*>(_param_);
	SetElement(STRARY(elem), TMP_STRING(elem), Rule::Step::Option);

    yyval = yyvsp[0];
;}
    break;

  case 22:
#line 250 "RuleParser.y"
    {
// #T129#
	// setting present execution status.
	RuleElementSet* elem = una_reinterpret_cast<RuleElementSet*>(_param_);
	SetElement(STRARY(elem), TMP_STRING(elem), Rule::Step::Arg1);

    yyval = yyvsp[0];
;}
    break;

  case 23:
#line 260 "RuleParser.y"
    {
// #T130#
	// setting present execution status.
	RuleElementSet* elem = una_reinterpret_cast<RuleElementSet*>(_param_);
	SetElement(STRARY(elem), TMP_STRING(elem), Rule::Step::Arg2);

    yyval = yyvsp[0];
;}
    break;


    }

// #T131#
/* Line 999 of yacc.c.  */
#line 1248 "RuleParser.tab.cpp"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


// #T132#
  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


// #T133#
/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
// #T134#
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

// #T135#
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

// #T136#
	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
// #T137#
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
// #T138#
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

// #T139#
      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
// #T140#
	  /* Pop the error token.  */
          YYPOPSTACK;
// #T141#
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

// #T142#
  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


// #T143#
/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:
// #T144#
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

// #T145#
      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


// #T146#
/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

// #T147#
/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
// #T148#
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
// #T149#
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 269 "RuleParser.y"

_UNA_END

// #T150#
/*---------------------------------------------------------------------------*/
/* C program part of addition                                                */
/*---------------------------------------------------------------------------*/


// #T151#
//
//	Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

