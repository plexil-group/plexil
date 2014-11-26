// Copyright (c) 2006-2014, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//    * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Tree-rewriting rules for Plexil AST
// See https://theantlrguy.atlassian.net/wiki/display/ANTLR3/Tree+pattern+matching

tree grammar PlexilTreeTransforms;

options
{
    language = Java;
    tokenVocab = Plexil;
    ASTLabelType = PlexilTreeNode;
    output = AST;
    filter = true;
	rewrite = true;
    backtrack = true;
}

@header
{
package plexil;

import plexil.*;
}

@members
{
    GlobalContext m_globalContext = GlobalContext.getGlobalContext();
    NodeContext m_context = m_globalContext;
    Stack<String> m_paraphrases = new Stack<String>();

	// Overrides to enhance error reporting
	public String getErrorMessage(RecognitionException e,
		   		  				  String[] tokenNames)
	{
	  String msg = super.getErrorMessage(e, tokenNames);
	  if (m_paraphrases.size() > 0) {
		 msg = msg + " " + m_paraphrases.peek();
	  }
      return msg;
	}

	public void displayRecognitionError(String[] tokenNames,
										RecognitionException e)
	{
	  CompilerState.getCompilerState().addDiagnostic((PlexilTreeNode) e.node,
													 getErrorHeader(e) + " " + getErrorMessage(e, tokenNames),
													 Severity.ERROR);
	}
}

////////////////////////////////////////
// *** BEGIN TREE TRANSFORMATIONS *** //
////////////////////////////////////////

topdown:
        enterContext ;

bottomup:
        exitContext
    |   associativeReduction ;

//
// Top-down transforms
//

enterContext:
        bindingContextNode
        {
            m_context = $bindingContextNode.start.getContext();
            // System.out.println("Enter context " + m_context.getNodeName()); // DEBUG
        } ;

//
// Bottom-up transforms
//

exitContext:
        bindingContextNode
        {
            // System.out.println("Exit context " + m_context.getNodeName()); // DEBUG
            m_context = m_context.getParentContext();
            // System.out.println("Restore context " + m_context.getNodeName()); // DEBUG
        } ;

// Combine binary ops where appropriate
associativeReduction:
        ^(AND_KYWD ^(AND_KYWD (args+=.)+) argn=.) -> ^(AND_KYWD ($args)+ $argn)
    |   ^(OR_KYWD ^(OR_KYWD (args+=.)+) argn=.)   -> ^(OR_KYWD ($args)+ $argn)
    |   ^(PLUS ^(PLUS (args+=.)+) argn=.)         -> ^(PLUS ($args)+ $argn)
    |   ^(MINUS ^(MINUS (args+=.)+) argn=.)       -> ^(MINUS ($args)+ $argn)
    |   ^(ASTERISK ^(ASTERISK (args+=.)+) argn=.) -> ^(ASTERISK ($args)+ $argn)
    |   ^(MAX_KYWD ^(MAX_KYWD (args+=.)+) argn=.) -> ^(MAX_KYWD ($args)+ $argn)
    |   ^(MIN_KYWD ^(MIN_KYWD (args+=.)+) argn=.) -> ^(MIN_KYWD ($args)+ $argn)
    ;

//        ^(op1=associativeOp ^(op2=associativeOp (rest+=.)+) arg3=. {$op1.start.getType() == $op2.start.getType()}? )
//    -> ^($op2 $rest $arg3)
//    ;

//
// Recognizer rules
//

bindingContextNode:
        BLOCK
    | CONCURRENCE_KYWD
    | SEQUENCE_KYWD
    | UNCHECKED_SEQUENCE_KYWD
    | TRY_KYWD
    | FOR_KYWD
    ;

// Binary operators that can be used as N-ary
associativeOp:
        OR_KYWD
    |   AND_KYWD
    |   PLUS
    |   MINUS
    |   ASTERISK
    |   MAX_KYWD
    |   MIN_KYWD
    ;
