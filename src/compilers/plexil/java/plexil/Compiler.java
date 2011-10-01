/* Copyright (c) 2006-2011, Universities Space Research Association (USRA).
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Universities Space Research Association nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package plexil;

import java.io.File;

import org.antlr.runtime.*;
import org.antlr.runtime.tree.*;

import net.n3.nanoxml.*;

public class Compiler
{
    public static void main(String[] args)
    {
        CompilerState state = new CompilerState(args);

        // open source file

        // Pass 1: Parse plan
        PlexilTreeNode plan1 = pass1(state);
        if (plan1 == null) {
            for (Diagnostic d : state.getDiagnostics()) {
                System.err.println(d.toString());
            }
            System.out.println("Syntax error(s) detected. Compilation aborted.");
            System.exit(-1);
        }
        if (state.debug) {
            System.err.println("Pass 1 output:");
            System.err.println(plan1.toStringTree());
        }
        if (state.syntaxOnly) {
            System.exit(0);
        }

        // Pass 2: Tree parse & transformations
        // *** DISABLED until something useful is done in this pass ***
        // PlexilTreeNode plan2 = pass2(plan1, state);
        // if (state.debug) {
        // 	System.err.println("Pass 2 output:");
        // 	System.err.println(plan2.toStringTree());
        // }

        // Pass 3: semantic checks
        if (!pass3(plan1, state)) {
            for (Diagnostic d : state.getDiagnostics()) {
                System.err.println(d.toString());
            }
            System.out.println("Semantic error(s) detected. Compilation aborted.");
            System.exit(-1);
        }
        if (state.debug)
            System.err.println("Semantic checks succeeded"); 
        if (state.semanticsOnly) {
            System.exit(0);
        }

        // Pass 4: generate Extended Plexil XML
        if (!pass4(plan1, state)) {
            System.out.println("Internal error: XML generation failed. Compilation aborted.");
            System.exit(-1);
        }

        // Pass 5: translate Extended Plexil to Core Plexil
        if (!state.epxOnly) {
            if (!pass5(plan1, state)) {
                System.out.println("Internal error: translation from Extended Plexil XML failed. Compilation aborted.");
                System.exit(-1);
            }
        }

        System.exit(0);
    }

    public static PlexilTreeNode pass1(CompilerState state)
    {
        PlexilLexer lexer = new PlexilLexer(state.getInputStream(), state.sharedState);
        TokenStream tokenStream = new CommonTokenStream(lexer);

        PlexilParser parser = new PlexilParser(tokenStream, state.sharedState);
        parser.setTreeAdaptor(new PlexilTreeAdaptor());

        try {
            PlexilParser.plexilPlan_return planReturn = 
                parser.plexilPlan();
            if (state.maxErrorSeverity() <= 0)
                return (PlexilTreeNode) planReturn.getTree();
            else
                return null;
        }
        catch (RecognitionException x) {
            System.out.println("First pass error: " + x);
        }
        return null;
    }

    public static PlexilTreeNode pass2(PlexilTreeNode plan1, CompilerState state)
    {
        CommonTreeNodeStream treeStream = new CommonTreeNodeStream(plan1);
        PlexilTree treeParser = new PlexilTree(treeStream, state.sharedState);
        treeParser.setTreeAdaptor(new PlexilTreeAdaptor());

        try {
            PlexilTree.plexilPlan_return planReturn = 
                treeParser.plexilPlan();
            return (PlexilTreeNode) planReturn.getTree();
        }
        catch (RecognitionException x) {
            System.err.println("Second pass error: " + x);
        }
        return null;
    }

    public static boolean pass3(PlexilTreeNode plan, CompilerState state)
    {
        GlobalContext gcontext = GlobalContext.getGlobalContext();
        plan.earlyCheck(gcontext, state);
        plan.check(gcontext, state);
        return state.maxErrorSeverity() <= 0;
    }

    public static boolean pass4(PlexilTreeNode plan, CompilerState state)
    {
        IXMLElement planXML = null;
        try {
            planXML = plan.getXML();
            XMLWriter writer = state.getEpxWriter();
            if (writer == null) {
                System.err.println("Unable to create Extended Plexil output stream");
                return false;
            }
            writer.write(planXML, state.prettyPrintEpx);
            return true;
        }
        catch (Exception e) {
            System.err.println("XML generation error: " + e);
            return false;
        }
    }

    public static boolean pass5(PlexilTreeNode plan, CompilerState state)
    {
        File epxFile = state.getEpxFile();
        if (epxFile == null) {
            System.err.println("No Extended Plexil file specified");
            return false;
        }
        File outputFile = state.getOutputFile();
        try {
            // Invoke XSLT translator
            if (state.debug)
                System.err.println("Translating to Core PLEXIL file " + outputFile);

            String[] saxonArgs = new String[4];
            saxonArgs[0] = "-o";
            saxonArgs[1] = outputFile.toString();
            saxonArgs[2] = epxFile.toString();
            saxonArgs[3] = System.getenv("PLEXIL_HOME") + "/schema/translate-plexil.xsl";
            net.sf.saxon.Transform.main(saxonArgs);

            if (!state.keepEpx) {
                try {
                    if (!epxFile.delete())
                        System.err.println("WARNING: Could not delete " + epxFile.toString());
                }
                catch (Exception e) {
                    System.err.println("Error deleting " + epxFile.toString() + ": " + e);
                }
            }
            return true;
        }
        catch (Exception e) {
            System.err.println("Extended Plexil translation error: " + e);
            return false;
        }
    }

}
