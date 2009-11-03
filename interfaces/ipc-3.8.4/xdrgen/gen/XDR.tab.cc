/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDENTIFIER = 258,
     STRINGVAL = 259,
     LONGVAL = 260,
     PASS_THROUGH_CODE = 261,
     BOOL = 262,
     CHAR = 263,
     CONST = 264,
     DOUBLE = 265,
     FLOAT = 266,
     STRING = 267,
     STRUCT = 268,
     TYPEDEF = 269,
     IPC_TYPE = 270,
     UNSIGNED = 271,
     VOID = 272,
     OPAQUE = 273,
     LONG = 274,
     INT = 275,
     SHORT = 276,
     ENUM = 277,
     DEFAULT = 278,
     HYPER = 279,
     SWITCH = 280,
     UNION = 281,
     CASE = 282
   };
#endif
/* Tokens.  */
#define IDENTIFIER 258
#define STRINGVAL 259
#define LONGVAL 260
#define PASS_THROUGH_CODE 261
#define BOOL 262
#define CHAR 263
#define CONST 264
#define DOUBLE 265
#define FLOAT 266
#define STRING 267
#define STRUCT 268
#define TYPEDEF 269
#define IPC_TYPE 270
#define UNSIGNED 271
#define VOID 272
#define OPAQUE 273
#define LONG 274
#define INT 275
#define SHORT 276
#define ENUM 277
#define DEFAULT 278
#define HYPER 279
#define SWITCH 280
#define UNION 281
#define CASE 282




/* Copy the first part of user declarations.  */
#line 14 "XDR.y"
 /* C code section */

#define YYERROR_VERBOSE 1
#define YYDEBUG 1

#include <stdlib.h>	/* For malloc */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
/* #include <ctype.h> */
#include <iostream>

#include "XDRTree.h"
#include "XDRInternal.h"

#ifndef IRIX
extern void __yy_memcpy (char *to, char *from, int count);
#endif

 using namespace std;

/*** EXPORTED DATA STRUCTURE ***/

XDRSpecification *yyTree;
const char *yyinName;



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union
#line 46 "XDR.y"
yystacktype
                {
		  XDRLongToken		  *longVal;
		  XDRStringToken	  *stringVal;

		  XDRDeclNode             *declVal;
		  XDRArrayDimNode         *arrayDimVal;
		  XDRArrayDimsNode        *arrayDimsVal;
		  XDRTypeSpecNode         *typeVal;
		  XDRTypeSpecStringNode   *typeStringVal;
		  XDRTypeSpecStructNode   *typeStructVal;
		  XDRDeclListNode         *declListVal;
		  XDRTypeDefNode          *typeDefVal;
		  XDRIPCTypeNode          *ipcTypeVal;
		  XDRDefinitionNode       *defVal;
		  XDRDefinitionListNode   *defListVal;
		  XDRSpecification        *specVal;
		  XDRPassThroughCodeNode  *passThroughVal;
		  XDREnumListNode         *enumListVal;
		  XDREnumEntryNode        *enumEntryVal;
		}
/* Line 193 of yacc.c.  */
#line 202 "gen/XDR.tab.cc"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 215 "gen/XDR.tab.cc"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

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
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  11
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   77

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  37
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  22
/* YYNRULES -- Number of rules.  */
#define YYNRULES  50
/* YYNRULES -- Number of states.  */
#define YYNSTATES  83

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   282

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    36,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    28,
      30,    29,    31,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    32,     2,    33,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    34,     2,    35,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     7,    13,    16,    20,    24,    28,    33,
      35,    38,    40,    43,    47,    50,    54,    56,    58,    61,
      63,    65,    67,    69,    72,    74,    76,    79,    81,    83,
      86,    90,    92,    96,    98,   102,   104,   107,   112,   115,
     119,   123,   128,   133,   135,   137,   139,   140,   143,   144,
     146
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      58,     0,    -1,    15,     3,    28,    -1,    15,     3,    29,
       4,    28,    -1,    46,     3,    -1,    46,     3,    41,    -1,
      46,     3,    40,    -1,    45,     3,    40,    -1,    45,     3,
      41,    42,    -1,    42,    -1,    40,    42,    -1,    43,    -1,
      41,    43,    -1,    30,    44,    31,    -1,    30,    31,    -1,
      32,    44,    33,    -1,     5,    -1,    12,    -1,    16,    20,
      -1,    20,    -1,    11,    -1,    10,    -1,     7,    -1,    13,
      52,    -1,     3,    -1,    18,    -1,    16,     8,    -1,     8,
      -1,    47,    -1,    22,    48,    -1,    34,    49,    35,    -1,
      50,    -1,    49,    36,    50,    -1,     3,    -1,     3,    29,
      44,    -1,    53,    -1,    17,    28,    -1,    34,    51,    57,
      35,    -1,    39,    28,    -1,    53,    39,    28,    -1,    14,
      39,    28,    -1,    13,     3,    52,    28,    -1,    22,     3,
      48,    28,    -1,    54,    -1,    38,    -1,     6,    -1,    -1,
      56,    55,    -1,    -1,     6,    -1,    56,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   111,   111,   113,   119,   126,   131,   137,   147,   157,
     162,   168,   173,   179,   181,   184,   187,   194,   197,   199,
     206,   208,   210,   212,   214,   220,   222,   224,   226,   231,
     234,   237,   242,   248,   250,   256,   258,   264,   270,   275,
     301,   303,   305,   311,   313,   315,   322,   323,   330,   331,
     334
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENTIFIER", "STRINGVAL", "LONGVAL",
  "PASS_THROUGH_CODE", "BOOL", "CHAR", "CONST", "DOUBLE", "FLOAT",
  "STRING", "STRUCT", "TYPEDEF", "IPC_TYPE", "UNSIGNED", "VOID", "OPAQUE",
  "LONG", "INT", "SHORT", "ENUM", "DEFAULT", "HYPER", "SWITCH", "UNION",
  "CASE", "';'", "'='", "'<'", "'>'", "'['", "']'", "'{'", "'}'", "','",
  "$accept", "ipcType", "decl", "varArrayDims", "fixArrayDims", "varArray",
  "fixArray", "value", "stringTypeSpec", "typeSpecifier", "enumTypeSpec",
  "enumBody", "enumList", "enumEntry", "structInternal", "structBody",
  "declList", "typeDef", "definition", "definitionList", "optCode",
  "specification", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,    59,    61,
      60,    62,    91,    93,   123,   125,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    37,    38,    38,    39,    39,    39,    39,    39,    40,
      40,    41,    41,    42,    42,    43,    44,    45,    46,    46,
      46,    46,    46,    46,    46,    46,    46,    46,    46,    47,
      48,    49,    49,    50,    50,    51,    51,    52,    53,    53,
      54,    54,    54,    55,    55,    55,    56,    56,    57,    57,
      58
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     5,     2,     3,     3,     3,     4,     1,
       2,     1,     2,     3,     2,     3,     1,     1,     2,     1,
       1,     1,     1,     2,     1,     1,     2,     1,     1,     2,
       3,     1,     3,     1,     3,     1,     2,     4,     2,     3,
       3,     4,     4,     1,     1,     1,     0,     2,     0,     1,
       1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      46,    50,     0,    45,     0,     0,     0,     0,    44,    43,
      47,     1,     0,    24,    22,    27,    21,    20,    17,     0,
       0,    25,    19,     0,     0,     0,     0,    28,     0,     0,
       0,     0,    23,    26,    18,     0,    29,    40,     0,     4,
       2,     0,     0,     0,     0,    48,    35,    41,    33,     0,
      31,     0,     0,     7,     0,     9,    11,     6,     5,     0,
      42,    36,    38,    49,     0,     0,     0,    30,     0,    16,
      14,     0,     0,    10,     8,    12,     3,    37,    39,    34,
      32,    13,    15
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     8,    24,    53,    54,    55,    56,    71,    25,    26,
      27,    36,    49,    50,    45,    31,    46,     9,    10,     1,
      64,     2
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -46
static const yytype_int8 yypact[] =
{
     -46,    31,    22,   -46,    26,    20,    36,    38,   -46,   -46,
     -46,   -46,    -9,   -46,   -46,   -46,   -46,   -46,   -46,    -9,
       4,   -46,   -46,     9,    24,    51,    52,   -46,     6,     9,
      -2,    28,   -46,   -46,   -46,    54,   -46,   -46,   -28,   -28,
     -46,    55,    30,    32,    33,    56,    20,   -46,    34,    14,
     -46,    -5,    59,    35,   -28,   -46,   -46,    35,    37,    39,
     -46,   -46,   -46,   -46,    40,    42,    59,   -46,    54,   -46,
     -46,    41,    43,   -46,   -46,   -46,   -46,   -46,   -46,   -46,
     -46,   -46,   -46
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -46,   -46,   -27,    27,    29,    -6,   -41,   -45,   -46,   -46,
     -46,    44,   -46,     3,   -46,    58,   -46,   -46,   -46,   -46,
     -46,   -46
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      69,    13,    51,    44,    52,    14,    15,    72,    16,    17,
      18,    19,    33,    75,    20,    43,    21,    75,    22,    65,
      23,    79,    11,    13,    34,    30,    70,    14,    15,    12,
      16,    17,    18,    19,    40,    41,    20,     3,    21,    28,
      22,    29,    23,    35,     4,     5,     6,    73,    74,    67,
      68,    73,    37,     7,    38,    39,    47,    48,    60,    59,
      61,    62,    63,    66,    69,    51,    57,    76,    58,    52,
      78,    80,    81,    42,     0,    77,    82,    32
};

static const yytype_int8 yycheck[] =
{
       5,     3,    30,    30,    32,     7,     8,    52,    10,    11,
      12,    13,     8,    54,    16,    17,    18,    58,    20,    46,
      22,    66,     0,     3,    20,    34,    31,     7,     8,     3,
      10,    11,    12,    13,    28,    29,    16,     6,    18,     3,
      20,     3,    22,    34,    13,    14,    15,    53,    54,    35,
      36,    57,    28,    22,     3,     3,    28,     3,    28,     4,
      28,    28,     6,    29,     5,    30,    39,    28,    39,    32,
      28,    68,    31,    29,    -1,    35,    33,    19
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    56,    58,     6,    13,    14,    15,    22,    38,    54,
      55,     0,     3,     3,     7,     8,    10,    11,    12,    13,
      16,    18,    20,    22,    39,    45,    46,    47,     3,     3,
      34,    52,    52,     8,    20,    34,    48,    28,     3,     3,
      28,    29,    48,    17,    39,    51,    53,    28,     3,    49,
      50,    30,    32,    40,    41,    42,    43,    40,    41,     4,
      28,    28,    28,     6,    57,    39,    29,    35,    36,     5,
      31,    44,    44,    42,    42,    43,    28,    35,    28,    44,
      50,    31,    33
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


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
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
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

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
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
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

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

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

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
        case 2:
#line 112 "XDR.y"
    { (yyval.ipcTypeVal) = new XDRIPCTypeNode((yyvsp[(2) - (3)].stringVal)); ;}
    break;

  case 3:
#line 114 "XDR.y"
    {
		 (yyval.ipcTypeVal) = new XDRIPCTypeNode((yyvsp[(2) - (5)].stringVal));
		 (yyval.ipcTypeVal)->addFormatString((yyvsp[(4) - (5)].stringVal));
	       ;}
    break;

  case 4:
#line 120 "XDR.y"
    {
		   (yyval.declVal) = new XDRDeclNode((yyvsp[(2) - (2)].stringVal),(yyvsp[(1) - (2)].typeVal));
                 ;}
    break;

  case 5:
#line 127 "XDR.y"
    {
		   (yyvsp[(1) - (3)].typeVal)->addDims(A_FIXED_ARRAY,(yyvsp[(3) - (3)].arrayDimsVal));
		   (yyval.declVal) = new XDRDeclNode((yyvsp[(2) - (3)].stringVal),(yyvsp[(1) - (3)].typeVal));
                 ;}
    break;

  case 6:
#line 132 "XDR.y"
    {
		   (yyvsp[(1) - (3)].typeVal)->addDims(A_VAR_ARRAY,(yyvsp[(3) - (3)].arrayDimsVal));
		   (yyval.declVal) = new XDRDeclNode((yyvsp[(2) - (3)].stringVal),(yyvsp[(1) - (3)].typeVal));
                 ;}
    break;

  case 7:
#line 138 "XDR.y"
    {
                   /* take the last var array dimension off the back
		      and make it the string array dim */
		   (yyvsp[(1) - (3)].typeStringVal)->addStringArrayDim((yyvsp[(3) - (3)].arrayDimsVal)->getItems().back());
		   (yyvsp[(3) - (3)].arrayDimsVal)->pop_back();
		   if ((yyvsp[(3) - (3)].arrayDimsVal)->getItems().size() > 0) (yyvsp[(1) - (3)].typeStringVal)->addDims(A_VAR_ARRAY,(yyvsp[(3) - (3)].arrayDimsVal));
		   (yyvsp[(3) - (3)].arrayDimsVal)->checkRef();
		   (yyval.declVal) = new XDRDeclNode((yyvsp[(2) - (3)].stringVal),(yyvsp[(1) - (3)].typeStringVal));
		 ;}
    break;

  case 8:
#line 148 "XDR.y"
    {
		   (yyvsp[(1) - (4)].typeStringVal)->addDims(A_FIXED_ARRAY,(yyvsp[(3) - (4)].arrayDimsVal));
		   (yyvsp[(1) - (4)].typeStringVal)->addStringArrayDim((yyvsp[(4) - (4)].arrayDimVal));
		   (yyval.declVal) = new XDRDeclNode((yyvsp[(2) - (4)].stringVal),(yyvsp[(1) - (4)].typeStringVal));
		 ;}
    break;

  case 9:
#line 158 "XDR.y"
    {
		   (yyval.arrayDimsVal) = new XDRArrayDimsNode;
		   (yyval.arrayDimsVal)->push_back((yyvsp[(1) - (1)].arrayDimVal));
                 ;}
    break;

  case 10:
#line 163 "XDR.y"
    {
		   (yyval.arrayDimsVal) = (yyvsp[(1) - (2)].arrayDimsVal);
		   (yyval.arrayDimsVal)->push_back((yyvsp[(2) - (2)].arrayDimVal));
		 ;}
    break;

  case 11:
#line 169 "XDR.y"
    {
		   (yyval.arrayDimsVal) = new XDRArrayDimsNode;
		   (yyval.arrayDimsVal)->push_back((yyvsp[(1) - (1)].arrayDimVal));
                 ;}
    break;

  case 12:
#line 174 "XDR.y"
    {
		   (yyval.arrayDimsVal) = (yyvsp[(1) - (2)].arrayDimsVal);
		   (yyval.arrayDimsVal)->push_back((yyvsp[(2) - (2)].arrayDimVal));
		 ;}
    break;

  case 13:
#line 180 "XDR.y"
    { (yyval.arrayDimVal) = new XDRArrayDimNode((yyvsp[(2) - (3)].longVal)); ;}
    break;

  case 14:
#line 182 "XDR.y"
    { (yyval.arrayDimVal) = new XDRArrayDimNode(); ;}
    break;

  case 15:
#line 185 "XDR.y"
    { (yyval.arrayDimVal) = new XDRArrayDimNode((yyvsp[(2) - (3)].longVal)); ;}
    break;

  case 16:
#line 188 "XDR.y"
    { (yyval.longVal) = (yyvsp[(1) - (1)].longVal); ;}
    break;

  case 17:
#line 195 "XDR.y"
    { (yyval.typeStringVal) = new XDRTypeSpecStringNode; ;}
    break;

  case 18:
#line 198 "XDR.y"
    { (yyval.typeVal) = new XDRTypeSpecNode(T_UINT); ;}
    break;

  case 19:
#line 200 "XDR.y"
    { (yyval.typeVal) = new XDRTypeSpecNode(T_INT); ;}
    break;

  case 20:
#line 207 "XDR.y"
    { (yyval.typeVal) = new XDRTypeSpecNode(T_FLOAT); ;}
    break;

  case 21:
#line 209 "XDR.y"
    { (yyval.typeVal) = new XDRTypeSpecNode(T_DOUBLE); ;}
    break;

  case 22:
#line 211 "XDR.y"
    { (yyval.typeVal) = new XDRTypeSpecNode(T_BOOL); ;}
    break;

  case 23:
#line 213 "XDR.y"
    { (yyval.typeVal) = (yyvsp[(2) - (2)].typeStructVal); ;}
    break;

  case 24:
#line 215 "XDR.y"
    { (yyval.typeVal) = new XDRTypeSpecIdentNode((yyvsp[(1) - (1)].stringVal)); ;}
    break;

  case 25:
#line 221 "XDR.y"
    { (yyval.typeVal) = new XDRTypeSpecNode(T_UCHAR); ;}
    break;

  case 26:
#line 223 "XDR.y"
    { (yyval.typeVal) = new XDRTypeSpecNode(T_UCHAR); ;}
    break;

  case 27:
#line 225 "XDR.y"
    { (yyval.typeVal) = new XDRTypeSpecNode(T_CHAR); ;}
    break;

  case 29:
#line 232 "XDR.y"
    { (yyval.typeVal) = (yyvsp[(2) - (2)].typeVal); ;}
    break;

  case 30:
#line 235 "XDR.y"
    { (yyval.typeVal) = new XDRTypeSpecEnumNode((yyvsp[(2) - (3)].enumListVal)); ;}
    break;

  case 31:
#line 238 "XDR.y"
    {
		   (yyval.enumListVal) = new XDREnumListNode;
		   (yyval.enumListVal)->push_back((yyvsp[(1) - (1)].enumEntryVal));
		 ;}
    break;

  case 32:
#line 243 "XDR.y"
    {
		   (yyvsp[(1) - (3)].enumListVal)->push_back((yyvsp[(3) - (3)].enumEntryVal));
		   (yyval.enumListVal) = (yyvsp[(1) - (3)].enumListVal);
		 ;}
    break;

  case 33:
#line 249 "XDR.y"
    { (yyval.enumEntryVal) = new XDREnumEntryNode((yyvsp[(1) - (1)].stringVal)); ;}
    break;

  case 34:
#line 251 "XDR.y"
    {
		   (yyval.enumEntryVal) = new XDREnumEntryNode((yyvsp[(1) - (3)].stringVal));
		   (yyval.enumEntryVal)->addValue((yyvsp[(3) - (3)].longVal));
		 ;}
    break;

  case 35:
#line 257 "XDR.y"
    { (yyval.typeStructVal) = new XDRTypeSpecStructNode((yyvsp[(1) - (1)].declListVal)); ;}
    break;

  case 36:
#line 259 "XDR.y"
    {
		   XDRDeclListNode *dl = new XDRDeclListNode;
		   (yyval.typeStructVal) = new XDRTypeSpecStructNode(dl);
		 ;}
    break;

  case 37:
#line 265 "XDR.y"
    {
		   if (0 != (yyvsp[(3) - (4)].passThroughVal)) (yyvsp[(2) - (4)].typeStructVal)->addAfterDecls((yyvsp[(3) - (4)].passThroughVal));
		   (yyval.typeStructVal) = (yyvsp[(2) - (4)].typeStructVal);
                 ;}
    break;

  case 38:
#line 271 "XDR.y"
    {
		   (yyval.declListVal) = new XDRDeclListNode;
		   (yyval.declListVal)->push_back((yyvsp[(1) - (2)].declVal));
		 ;}
    break;

  case 39:
#line 276 "XDR.y"
    {
		   (yyvsp[(1) - (3)].declListVal)->push_back((yyvsp[(2) - (3)].declVal));
		   (yyval.declListVal) = (yyvsp[(1) - (3)].declListVal);
		 ;}
    break;

  case 40:
#line 302 "XDR.y"
    { (yyval.typeDefVal) = (yyvsp[(2) - (3)].declVal); ;}
    break;

  case 41:
#line 304 "XDR.y"
    { (yyval.typeDefVal) = new XDRTypeDefNode((yyvsp[(2) - (4)].stringVal),(yyvsp[(3) - (4)].typeStructVal)); ;}
    break;

  case 42:
#line 306 "XDR.y"
    { (yyval.typeDefVal) = new XDRTypeDefNode((yyvsp[(2) - (4)].stringVal),(yyvsp[(3) - (4)].typeVal)); ;}
    break;

  case 43:
#line 312 "XDR.y"
    { (yyval.defVal) = new XDRDefinitionTypeDefNode((yyvsp[(1) - (1)].typeDefVal)); ;}
    break;

  case 44:
#line 314 "XDR.y"
    { (yyval.defVal) = new XDRDefinitionIPCTypeNode((yyvsp[(1) - (1)].ipcTypeVal)); ;}
    break;

  case 45:
#line 316 "XDR.y"
    { (yyval.defVal) = new XDRDefinitionCodeNode((yyvsp[(1) - (1)].passThroughVal));}
    break;

  case 46:
#line 322 "XDR.y"
    { (yyval.defListVal) = new XDRDefinitionListNode; ;}
    break;

  case 47:
#line 324 "XDR.y"
    {
		   (yyvsp[(1) - (2)].defListVal)->push_back((yyvsp[(2) - (2)].defVal));
		   (yyval.defListVal) = (yyvsp[(1) - (2)].defListVal);
		 ;}
    break;

  case 48:
#line 330 "XDR.y"
    { (yyval.passThroughVal) = 0; ;}
    break;

  case 49:
#line 332 "XDR.y"
    { (yyval.passThroughVal) = (yyvsp[(1) - (1)].passThroughVal); ;}
    break;

  case 50:
#line 335 "XDR.y"
    { 
		   (yyval.specVal) = new XDRSpecification((yyvsp[(1) - (1)].defListVal),yyinName);
		   if (0 != fstat(fileno(yyin), &((yyval.specVal)->specFileStat))) {
		     cerr << "xdrgen: couldn't fstat " << yyinName << ": "
			  << strerror(errno) << endl;
		     exit(EXIT_FAILURE);
		   }
		   yyTree = (yyval.specVal);
		   yyTree->ref();
		 ;}
    break;


/* Line 1267 of yacc.c.  */
#line 1792 "gen/XDR.tab.cc"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


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


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
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

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 346 "XDR.y"

/* C code section */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: XDR.y,v $
 * Revision 1.4  2009/01/12 15:54:59  reids
 * Added BSD Open Source license info
 *
 * Revision 1.3  2004/04/06 15:06:08  trey
 * updated for more recent bison and g++
 *
 * Revision 1.2  2003/02/13 20:47:19  reids
 * Updated to compile under gcc 3.0.1
 *
 * Revision 1.1  2001/03/16 17:56:01  reids
 * Release of Trey's code to generate IPC format strings from XDR definitions.
 *
 * Revision 1.3  2001/02/08 04:14:56  trey
 * added enumerated types and fixed C language output
 *
 * Revision 1.2  2001/02/08 00:41:57  trey
 * added external IPC format feature to xdrgen; we also now tag the generated file with a version string
 *
 * Revision 1.1  2001/02/06 02:00:55  trey
 * initial check-in
 *
 * Revision 1.2  1999/11/08 15:34:59  trey
 * major overhaul
 *
 * Revision 1.1  1999/11/03 19:31:34  trey
 * initial check-in
 *
 *
 ***************************************************************************/

