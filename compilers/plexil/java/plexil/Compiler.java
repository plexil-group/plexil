/* Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

import plexil.xml.SimpleXmlWriter;

import java.io.File;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.antlr.runtime.*;
import org.antlr.runtime.tree.*;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

public class Compiler
{
    public CommandParser m_commandParser = null;

    private static DocumentBuilder s_documentBuilder = null;

    public static void main(String[] argv)
    {
        Compiler compiler = new Compiler();
        int status = compiler.compilerMain(argv);
        System.exit(status);
    }

    private Compiler()
    {
        m_commandParser = new CommandParser();
    }

    private int compilerMain(String[] argv)
    {
        int status = m_commandParser.parse(argv);
        if (status != 0) {
            return status;
        }

        File infile = m_commandParser.nextInputFile();
        while (infile != null) {
            CompilerState state = compileOne(infile);
            int severity = state.maxErrorSeverity();
            if (severity > status) {
                status = severity;
            }
            infile = m_commandParser.nextInputFile();
        }
        if (status < 0) {
            status = 0;
        }
        return status;
    }

    //*
    // Compile one input file.
    //
    // \param infile The File to compile.
    //
    // \param cp The CommandParser instance.
    //
    // \return TBD
    //
    // \note Should only exit on truly fatal errors.
    //
    public CompilerState compileOne(File infile)
    {
        System.out.println("Compiling " + infile.toString());
        CompilerState state = new CompilerState(infile, m_commandParser);

        // Pass 1: Parse plan
        PlexilTreeNode plan1 = pass1(state);
        if (plan1 == null) {
            state.displayDiagnostics();
            System.out.println("Syntax error(s) detected. Compilation aborted.");
            return state;
        }
        if (m_commandParser.debug) {
            System.err.println("Pass 1 output:");
            System.err.println(plan1.toStringTree());
        }
        if (m_commandParser.syntaxOnly) {
            state.displayDiagnostics();
            return state;
        }

        // Pass 2: semantic checks
        if (!pass2(plan1, state)) {
            state.displayDiagnostics();
            System.out.println("Semantic error(s) detected. Compilation aborted.");
            System.exit(1);
        }
        if (m_commandParser.debug)
            System.err.println("Semantic checks succeeded"); 
        if (m_commandParser.semanticsOnly) {
            state.displayDiagnostics();
            System.exit(0);
        }

        // Pass 3: Tree parse & transformations
        PlexilTreeNode plan2 = pass3(plan1, state);
        if (m_commandParser.debug) {
        	System.err.println("Pass 3 output:");
        	System.err.println(plan2.toStringTree());
        }

        // Pass 4: generate Extended Plexil XML
        Document planXML = pass4(plan2, state);
        if (planXML == null) {
            state.displayDiagnostics();
            System.err.println("Internal error: XML generation failed. Compilation aborted.");
            return state;
        }

        // Write .epx file here if requested
        if (m_commandParser.writeEpx) {
            try {
                SimpleXmlWriter writer = new SimpleXmlWriter(state.getEpxStream());
                writer.setIndent(m_commandParser.indentOutput);
                if (m_commandParser.debug)
                    System.err.println("Writing Extended PLEXIL file " + state.getEpxFile());
                writer.write(planXML);
            }
            catch (Throwable t) {
                System.err.println("Internal error while writing Extended Plexil file:");
                t.printStackTrace(System.err);
            }
        }

        // Pass 5: translate Extended Plexil to Core Plexil
        if (!m_commandParser.epxOnly) {
            if (!pass5(plan2, state, planXML)) {
                state.displayDiagnostics();
                System.out.println("Translation from Extended Plexil failed. Compilation aborted.");
                return state;
            }
        }

        // We made it! Tell the user what went wrong.
        state.displayDiagnostics();
        return state;
    }

    // Pass 1: Parse the plan

    public PlexilTreeNode pass1(CompilerState state)
    {
        PlexilLexer lexer = new PlexilLexer(state.getInputStream(), state.sharedState);
        TokenStream tokenStream = new CommonTokenStream(lexer);

        PlexilParser parser = new PlexilParser(tokenStream, state);
        parser.setTreeAdaptor(new PlexilTreeAdaptor());

        try {
            PlexilParser.plexilPlan_return planReturn = 
                parser.plexilPlan();
            if (state.maxErrorSeverity() > 0)
                return null;
            return (PlexilTreeNode) planReturn.getTree();
        }
        catch (RecognitionException x) {
            System.out.println("Parser error: " + x);
        }
        catch (Throwable t) {
            System.err.println("Internal error in parser:");
            t.printStackTrace(System.err);
        }
        return null;
    }

    // Pass 2: Perform semantic checks on the parse tree

    public boolean pass2(PlexilTreeNode plan, CompilerState state)
    {
        try {
            GlobalContext gcontext = state.getGlobalContext();
            plan.earlyCheck(gcontext, state);
            plan.check(gcontext, state);
            return state.maxErrorSeverity() <= 0;
        }
        catch (Throwable t) {
            System.err.println("Internal error in semantic check pass:");
            t.printStackTrace(System.err);
        }
        return false;
    }

    // Pass 3: Transform the parse tree prior to output generation

    public PlexilTreeNode pass3(PlexilTreeNode plan1, CompilerState state)
    {
        TreeAdaptor adaptor = new PlexilTreeAdaptor();
        TreeNodeStream treeStream = new CommonTreeNodeStream(adaptor, plan1);
        PlexilTreeTransforms treeRewriter = new PlexilTreeTransforms(treeStream, state);
        treeRewriter.setTreeAdaptor(adaptor);
        
        try {
            Object rewriteResult = treeRewriter.downup(plan1, m_commandParser.debug);
            if (state.maxErrorSeverity() > 0)
                return null; // errors already reported
            PlexilTreeNode rewritePlan = (PlexilTreeNode) rewriteResult;
            if (rewritePlan == null) 
                System.err.println("Internal error: Plan transformation pass resulted in empty plan");
            return rewritePlan;
        }
        catch (Throwable t) {
            System.err.println("Internal error in plan transformation pass:");
            t.printStackTrace(System.err);
        }
        return null;
    }

    // Pass 4: Generate Extended Plexil output as DOM

    public Document pass4(PlexilTreeNode plan, CompilerState state)
    {
        Document planDoc = constructRootDocument();
        state.setRootDocument(planDoc);
        
        Element rootElement = null;
        try {
            rootElement = plan.getXML(planDoc);
            planDoc.appendChild(rootElement);
        }
        catch (Throwable t) {
            System.err.println("Internal error while generating XML:");
            t.printStackTrace(System.err);
            return null;
        }
        return planDoc;
    }

    // Pass 5: Transform the Extended Plexil XML into Core Plexil

    public boolean pass5(PlexilTreeNode plan, CompilerState state, Document planDoc)
    {
        File outputFile = state.getOutputFile();
        try {
            // Invoke XSLT translator
            if (m_commandParser.debug)
                System.err.println("Translating to Core PLEXIL file " + outputFile);

            plexil.xml.SaxonTransformer xformer = new plexil.xml.SaxonTransformer();
            xformer.setIndent(m_commandParser.indentOutput);

            File stylesheet =
                new File(System.getenv("PLEXIL_HOME"),
                         "schema/epx-translator/translate-plexil.xsl");
            return xformer.translateDOM(stylesheet, planDoc, outputFile);
        }
        catch (Exception e) {
            System.err.println("Extended Plexil translation error: " + e);
            return false;
        }
    }

    //
    // Utilities
    //

    private static Document constructRootDocument()
    {
        if (s_documentBuilder == null) {
            // Configuration??
            try {
                s_documentBuilder =
                    DocumentBuilderFactory.newInstance().newDocumentBuilder();
            } catch (ParserConfigurationException p) {
                System.err.println("Fatal error: unable to boostrap XML construction:\n"
                                   + p.toString());
                return null;
            };
        }
        Document root = s_documentBuilder.newDocument();
        root.setXmlVersion("1.0");
        return root;
    }

}
