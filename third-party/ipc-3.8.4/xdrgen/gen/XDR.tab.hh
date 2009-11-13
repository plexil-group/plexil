/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

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
/* Line 1529 of yacc.c.  */
#line 125 "gen/XDR.tab.hh"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

