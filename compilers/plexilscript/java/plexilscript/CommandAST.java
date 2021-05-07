// Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

//
// CommandAST
//

package plexilscript;

import java.util.List;

public class CommandAST implements ElementAST {
    String             tag;
    String             resultTag;
    String             name;
    List<ParameterAST> parameters;
    String             type;
    List<String>       results;

    public CommandAST
	(String             tag0,
	 String             resultTag0,
	 String             name0,
	 List<ParameterAST> parameters0,
	 String             type0,
	 List<String>       results0)
    {
	tag        = tag0;
	resultTag  = resultTag0;
	name       = name0;
	parameters = parameters0;
	type       = type0;
	results    = results0;
    }

    public void print () {
	System.out.printf ("    <%s name=\"%s\" type=\"%s\">\n", tag, name, type);
	for (ParameterAST p : parameters)
	    p.print ();
	for (String r : results)
	    System.out.printf ("      <%s>%s</%s>\n", resultTag, r, resultTag);
	System.out.printf ("    </%s>\n", tag);
    }
}

