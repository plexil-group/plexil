// $ANTLR 2.7.6 (20060405): "plexil.g" -> "PlexilParser.java"$
package plexil;

import java.io.FileNotFoundException;
import java.io.File;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.FileWriter;
import java.text.NumberFormat;
import java.util.List;
import java.util.LinkedList;
import java.util.ArrayList;
import java.util.Set;
import java.util.HashSet;
import java.util.Vector;


import antlr.TokenBuffer;
import antlr.TokenStreamException;
import antlr.TokenStreamIOException;
import antlr.ANTLRException;
import antlr.LLkParser;
import antlr.Token;
import antlr.TokenStream;
import antlr.RecognitionException;
import antlr.NoViableAltException;
import antlr.MismatchedTokenException;
import antlr.SemanticException;
import antlr.ParserSharedInputState;
import antlr.collections.impl.BitSet;
import antlr.collections.AST;
import java.util.Hashtable;
import antlr.ASTFactory;
import antlr.ASTPair;
import antlr.collections.impl.ASTArray;

public class PlexilParser extends antlr.LLkParser implements PlexilTokenTypes {

    static final int PLEXIL_MAJOR_RELEASE = 1;
    static final int PLEXIL_MINOR_RELEASE = 1;
    static final int PLEXIL_PARSER_RELEASE = 0;
    PlexilParserState state = new PlexilParserState();
    PlexilGlobalContext globalContext = PlexilGlobalContext.getGlobalContext();
    PlexilNodeContext context = globalContext;
    NumberFormat numberFormat = NumberFormat.getInstance();

    // For the convenience of the plan editor ONLY!!
    public void setContext(PlexilNodeContext ctxt) {
        context = ctxt;
    }

    public void mergeStates(PlexilParserState toAdd) {
        state.putAll(toAdd);
    }

    public PlexilParserState getState() {
        return state;
    }

    public String toString() {
        return "PlexilParser " +
                PLEXIL_MAJOR_RELEASE + "." +
                PLEXIL_MINOR_RELEASE + "." +
                PLEXIL_PARSER_RELEASE;
    }

    //
    // Main fns
    //
    public static PlexilParser parse(File file)
            throws RecognitionException, TokenStreamException, FileNotFoundException {
        return parse(null, file, null);
    }

    public static PlexilParser parse(File file, FileWriter debugWriter)
            throws RecognitionException, TokenStreamException, FileNotFoundException {
        return parse(null, file, debugWriter);
    }

    public static PlexilParser parse(PlexilParserState init, File file, FileWriter debugWriter)
            throws RecognitionException, TokenStreamException, FileNotFoundException {
        BufferedReader reader = null;
        reader = new BufferedReader(new FileReader(file));
        PlexilLexer lexer = new PlexilLexer(reader);
        PlexilParser parser = new PlexilParser(lexer);
        parser.setASTFactory(new PlexilASTFactory(file));
        if (init != null) {
            parser.mergeStates(init);
        }

        parser.getState().pushFile(file);
        parser.plexilPlan();
        parser.getState().popFile();
        return parser;
    }

    private SemanticException createSemanticException(String message, antlr.collections.AST location) {
        PlexilASTNode plexLoc = null;
        if (location != null) {
            plexLoc = (PlexilASTNode) location;
        }
        if (plexLoc == null) {
            int line = 0;
            int col = 0;
            try {
                line = LT(0).getLine();
                col = LT(0).getColumn();
            } catch (TokenStreamException e) {
            }
            return new SemanticException(message,
                    state.getFile().getPath(),
                    line,
                    col);
        } else {
            return new SemanticException(message,
                    plexLoc.getFilename(),
                    plexLoc.getLine(),
                    plexLoc.getColumn());
        }
    }

    private SemanticException createSemanticException(String message)
            throws TokenStreamException {
        return createSemanticException(message, null);
    }

    private int parseInteger(AST the_int)
            throws SemanticException {
        int result = 0;
        try {
            result = numberFormat.parse(the_int.getText()).intValue();
        } catch (java.text.ParseException e) {
            throw createSemanticException("Internal error: \"" + the_int.getText() + "\" is not parsable as an integer",
                    the_int);
        }
        return result;
    }

    /*
    Methods involving state
     */
    public void reportError(RecognitionException ex) {
        state.error(ex);
    }

    public void reportWarning(RecognitionException ex) {
        state.warn(ex);
    }

    protected PlexilParser(TokenBuffer tokenBuf, int k) {
        super(tokenBuf, k);
        tokenNames = _tokenNames;
        buildTokenTypeASTClassMap();
        astFactory = new ASTFactory(getTokenTypeToASTClassMap());
    }

    public PlexilParser(TokenBuffer tokenBuf) {
        this(tokenBuf, 3);
    }

    protected PlexilParser(TokenStream lexer, int k) {
        super(lexer, k);
        tokenNames = _tokenNames;
        buildTokenTypeASTClassMap();
        astFactory = new ASTFactory(getTokenTypeToASTClassMap());
    }

    public PlexilParser(TokenStream lexer) {
        this(lexer, 3);
    }

    public PlexilParser(ParserSharedInputState state) {
        super(state, 3);
        tokenNames = _tokenNames;
        buildTokenTypeASTClassMap();
        astFactory = new ASTFactory(getTokenTypeToASTClassMap());
    }

    public final void plexilPlan() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST plexilPlan_AST = null;

        try {      // for error handling
            {
                switch (LA(1)) {
                    case COMMAND_KYWD:
                    case FUNCTION_KYWD:
                    case LOOKUP_KYWD:
                    case LIBRARY_NODE_KYWD:
                    case BOOLEAN_KYWD:
                    case INTEGER_KYWD:
                    case REAL_KYWD:
                    case STRING_KYWD:
                    case BLOB_KYWD:
                    case TIME_KYWD: {
                        declarations();
                        astFactory.addASTChild(currentAST, returnAST);
                        break;
                    }
                    case LBRACE:
                    case IDENT: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            {
                node();
                astFactory.addASTChild(currentAST, returnAST);
            }
            match(Token.EOF_TYPE);
            if (inputState.guessing == 0) {
                plexilPlan_AST = (AST) currentAST.root;
                plexilPlan_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(PLEXIL, "PLEXIL")).add(plexilPlan_AST));
                ((PlexilASTNode) plexilPlan_AST).setFilename(state.getFile().toString());

                currentAST.root = plexilPlan_AST;
                currentAST.child = plexilPlan_AST != null && plexilPlan_AST.getFirstChild() != null ? plexilPlan_AST.getFirstChild() : plexilPlan_AST;
                currentAST.advanceChildToEnd();
            }
            plexilPlan_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_0);
            } else {
                throw ex;
            }
        }
        returnAST = plexilPlan_AST;
    }

    public final void declarations() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST declarations_AST = null;

        try {      // for error handling
            {
                int _cnt6 = 0;
                _loop6:
                do {
                    if ((_tokenSet_1.member(LA(1)))) {
                        declaration();
                        astFactory.addASTChild(currentAST, returnAST);
                    } else {
                        if (_cnt6 >= 1) {
                            break _loop6;
                        } else {
                            throw new NoViableAltException(LT(1), getFilename());
                        }
                    }

                    _cnt6++;
                } while (true);
            }
            if (inputState.guessing == 0) {
                declarations_AST = (AST) currentAST.root;
                declarations_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(GLOBAL_DECLARATIONS, "GlobalDeclarations")).add(declarations_AST));
                currentAST.root = declarations_AST;
                currentAST.child = declarations_AST != null && declarations_AST.getFirstChild() != null ? declarations_AST.getFirstChild() : declarations_AST;
                currentAST.advanceChildToEnd();
            }
            declarations_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_2);
            } else {
                throw ex;
            }
        }
        returnAST = declarations_AST;
    }

    public final void node() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST node_AST = null;
        AST myId_AST = null;
        Token lbrace = null;
        AST lbrace_AST = null;

        String nodeName = null;


        try {      // for error handling
            {
                switch (LA(1)) {
                    case IDENT: {
                        nodeId();
                        myId_AST = (AST) returnAST;
                        astFactory.addASTChild(currentAST, returnAST);
                        match(COLON);
                        if (inputState.guessing == 0) {
                            nodeName = myId_AST.getFirstChild().getText();
                        }
                        break;
                    }
                    case LBRACE: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            lbrace = LT(1);
            lbrace_AST = astFactory.create(lbrace);
            match(LBRACE);
            if (inputState.guessing == 0) {
                node_AST = (AST) currentAST.root;

                node_AST = (AST) astFactory.make((new ASTArray(2)).add((plexil.NodeASTNode) astFactory.create(NODE, "NODE", "plexil.NodeASTNode")).add(node_AST));
                ((PlexilASTNode) node_AST).setLine(lbrace_AST.getLine());
                ((PlexilASTNode) node_AST).setColumn(lbrace_AST.getColumn());
                ((PlexilASTNode) node_AST).setFilename(state.getFile().toString());

                if (myId_AST == null) {
                    // create default node name & AST
                    nodeName = context.generateChildNodeName();
                    Token nn = new antlr.CommonToken(NodeName, nodeName);
                    nn.setLine(lbrace_AST.getLine());
                    nn.setColumn(lbrace_AST.getColumn());
                    AST nnAST = astFactory.create(nn);
                    myId_AST = (AST) astFactory.make((new ASTArray(1)).add(astFactory.create(NODE_ID, "NodeId")));
                    myId_AST.addChild(nnAST);
                    node_AST.addChild(myId_AST);
                } else {
                    nodeName = myId_AST.getFirstChild().getText();
                }

                context = new PlexilNodeContext(context, nodeName);
                ((NodeASTNode) node_AST).setContext(context);
                ((NodeASTNode) node_AST).setNodeName(nodeName);
                state.addNode(nodeName, context);

                currentAST.root = node_AST;
                currentAST.child = node_AST != null && node_AST.getFirstChild() != null ? node_AST.getFirstChild() : node_AST;
                currentAST.advanceChildToEnd();
            }
            {
                switch (LA(1)) {
                    case COMMENT_KYWD: {
                        comment();
                        astFactory.addASTChild(currentAST, returnAST);
                        break;
                    }
                    case COMMAND_KYWD:
                    case RESOURCE_KYWD:
                    case RESOURCE_PRIORITY_KYWD:
                    case PRIORITY_KYWD:
                    case PERMISSIONS_KYWD:
                    case START_CONDITION_KYWD:
                    case REPEAT_CONDITION_KYWD:
                    case SKIP_CONDITION_KYWD:
                    case PRE_CONDITION_KYWD:
                    case POST_CONDITION_KYWD:
                    case INVARIANT_CONDITION_KYWD:
                    case END_CONDITION_KYWD:
                    case START_COND_KYWD:
                    case REPEAT_KYWD:
                    case SKIP_KYWD:
                    case PRE_KYWD:
                    case POST_KYWD:
                    case INVARIANT_KYWD:
                    case END_COND_KYWD:
                    case IN_KYWD:
                    case IN_OUT_KYWD:
                    case BOOLEAN_KYWD:
                    case INTEGER_KYWD:
                    case REAL_KYWD:
                    case STRING_KYWD:
                    case BLOB_KYWD:
                    case TIME_KYWD:
                    case NODE_LIST_KYWD:
                    case FUNCTION_CALL_KYWD:
                    case ASSIGNMENT_KYWD:
                    case UPDATE_KYWD:
                    case REQUEST_KYWD:
                    case LIBRARY_CALL_KYWD:
                    case RBRACE: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            {
                _loop45:
                do {
                    if (((LA(1) >= IN_KYWD && LA(1) <= TIME_KYWD))) {
                        nodeDeclaration();
                        astFactory.addASTChild(currentAST, returnAST);
                    } else {
                        break _loop45;
                    }

                } while (true);
            }
            {
                _loop47:
                do {
                    if ((_tokenSet_3.member(LA(1)))) {
                        nodeAttribute();
                        astFactory.addASTChild(currentAST, returnAST);
                    } else {
                        break _loop47;
                    }

                } while (true);
            }
            {
                switch (LA(1)) {
                    case COMMAND_KYWD:
                    case NODE_LIST_KYWD:
                    case FUNCTION_CALL_KYWD:
                    case ASSIGNMENT_KYWD:
                    case UPDATE_KYWD:
                    case REQUEST_KYWD:
                    case LIBRARY_CALL_KYWD: {
                        nodeBody();
                        astFactory.addASTChild(currentAST, returnAST);
                        break;
                    }
                    case RBRACE: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(RBRACE);
            if (inputState.guessing == 0) {

                context = context.getParentContext(); // back out to previous

            }
            node_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_4);
            } else {
                throw ex;
            }
        }
        returnAST = node_AST;
    }

    public final void declaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST declaration_AST = null;

        try {      // for error handling
            if ((_tokenSet_5.member(LA(1))) && (LA(2) == COMMAND_KYWD || LA(2) == IDENT) && (_tokenSet_6.member(LA(3)))) {
                commandDeclaration();
                astFactory.addASTChild(currentAST, returnAST);
                declaration_AST = (AST) currentAST.root;
            } else if ((_tokenSet_7.member(LA(1))) && (LA(2) == FUNCTION_KYWD || LA(2) == IDENT) && (_tokenSet_6.member(LA(3)))) {
                functionDeclaration();
                astFactory.addASTChild(currentAST, returnAST);
                declaration_AST = (AST) currentAST.root;
            } else if ((_tokenSet_8.member(LA(1))) && (LA(2) == LOOKUP_KYWD || LA(2) == IDENT) && (_tokenSet_9.member(LA(3)))) {
                lookupDeclaration();
                astFactory.addASTChild(currentAST, returnAST);
                declaration_AST = (AST) currentAST.root;
            } else if ((LA(1) == LIBRARY_NODE_KYWD)) {
                libraryNodeDeclaration();
                astFactory.addASTChild(currentAST, returnAST);
                declaration_AST = (AST) currentAST.root;
            } else {
                throw new NoViableAltException(LT(1), getFilename());
            }

        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_10);
            } else {
                throw ex;
            }
        }
        returnAST = declaration_AST;
    }

    public final void commandDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST commandDeclaration_AST = null;
        AST tn_AST = null;
        AST cn_AST = null;
        AST p_AST = null;

        try {      // for error handling
            {
                {
                    switch (LA(1)) {
                        case BOOLEAN_KYWD:
                        case INTEGER_KYWD:
                        case REAL_KYWD:
                        case STRING_KYWD:
                        case BLOB_KYWD:
                        case TIME_KYWD: {
                            typeName();
                            tn_AST = (AST) returnAST;
                            break;
                        }
                        case COMMAND_KYWD: {
                            break;
                        }
                        default: {
                            throw new NoViableAltException(LT(1), getFilename());
                        }
                    }
                }
                AST tmp4_AST = null;
                tmp4_AST = astFactory.create(LT(1));
                astFactory.makeASTRoot(currentAST, tmp4_AST);
                match(COMMAND_KYWD);
                commandName();
                cn_AST = (AST) returnAST;
                astFactory.addASTChild(currentAST, returnAST);
                {
                    switch (LA(1)) {
                        case LPAREN: {
                            paramsSpec();
                            p_AST = (AST) returnAST;
                            break;
                        }
                        case SEMICOLON: {
                            break;
                        }
                        default: {
                            throw new NoViableAltException(LT(1), getFilename());
                        }
                    }
                }
                match(SEMICOLON);
            }
            if (inputState.guessing == 0) {
                commandDeclaration_AST = (AST) currentAST.root;

                // add return spec (if needed)
                AST return_spec = null;
                if (tn_AST != null) {
                    return_spec = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(RETURNS_KYWD, "returns")).add((AST) astFactory.make((new ASTArray(2)).add(astFactory.create(RETURN_VALUE, "RETURN_VALUE")).add(tn_AST))));
                    commandDeclaration_AST.addChild(return_spec);
                }

                // process params
                // *** add error checking ***
                if (p_AST != null) {
                    commandDeclaration_AST.addChild(p_AST);
                }

                globalContext.addCommandName(cn_AST.getText(), p_AST, return_spec);

            }
            commandDeclaration_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_10);
            } else {
                throw ex;
            }
        }
        returnAST = commandDeclaration_AST;
    }

    public final void functionDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST functionDeclaration_AST = null;
        AST tn_AST = null;
        AST fn_AST = null;
        AST p_AST = null;

        try {      // for error handling
            {
                {
                    switch (LA(1)) {
                        case BOOLEAN_KYWD:
                        case INTEGER_KYWD:
                        case REAL_KYWD:
                        case STRING_KYWD:
                        case BLOB_KYWD:
                        case TIME_KYWD: {
                            typeName();
                            tn_AST = (AST) returnAST;
                            break;
                        }
                        case FUNCTION_KYWD: {
                            break;
                        }
                        default: {
                            throw new NoViableAltException(LT(1), getFilename());
                        }
                    }
                }
                AST tmp6_AST = null;
                tmp6_AST = astFactory.create(LT(1));
                astFactory.makeASTRoot(currentAST, tmp6_AST);
                match(FUNCTION_KYWD);
                functionName();
                fn_AST = (AST) returnAST;
                astFactory.addASTChild(currentAST, returnAST);
                {
                    switch (LA(1)) {
                        case LPAREN: {
                            paramsSpec();
                            p_AST = (AST) returnAST;
                            break;
                        }
                        case SEMICOLON: {
                            break;
                        }
                        default: {
                            throw new NoViableAltException(LT(1), getFilename());
                        }
                    }
                }
                match(SEMICOLON);
            }
            if (inputState.guessing == 0) {
                functionDeclaration_AST = (AST) currentAST.root;

                // add return spec (if supplied)
                AST return_spec = null;
                if (tn_AST != null) {
                    return_spec = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(RETURNS_KYWD, "returns")).add((AST) astFactory.make((new ASTArray(2)).add(astFactory.create(RETURN_VALUE, "RETURN_VALUE")).add(tn_AST))));
                    functionDeclaration_AST.addChild(return_spec);
                }

                // error check params
                // *** NYI ***

                // add param spec
                if (p_AST != null) {
                    functionDeclaration_AST.addChild(p_AST);
                }

                // declare function
                globalContext.addFunctionName(fn_AST.getText(), p_AST, return_spec);

            }
            functionDeclaration_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_10);
            } else {
                throw ex;
            }
        }
        returnAST = functionDeclaration_AST;
    }

    public final void lookupDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST lookupDeclaration_AST = null;
        AST tn_AST = null;
        AST s_AST = null;
        AST p_AST = null;
        AST sn_AST = null;
        AST ps_AST = null;
        AST rs_AST = null;
        AST state_name = null;
        AST parm_spec = null;
        AST return_spec = null;


        try {      // for error handling
            {
                switch (LA(1)) {
                    case BOOLEAN_KYWD:
                    case INTEGER_KYWD:
                    case REAL_KYWD:
                    case STRING_KYWD:
                    case BLOB_KYWD:
                    case TIME_KYWD: {
                        typeName();
                        tn_AST = (AST) returnAST;
                        match(LOOKUP_KYWD);
                        stateName();
                        s_AST = (AST) returnAST;
                        {
                            switch (LA(1)) {
                                case LPAREN: {
                                    paramsSpec();
                                    p_AST = (AST) returnAST;
                                    break;
                                }
                                case SEMICOLON: {
                                    break;
                                }
                                default: {
                                    throw new NoViableAltException(LT(1), getFilename());
                                }
                            }
                        }
                        match(SEMICOLON);
                        if (inputState.guessing == 0) {

                            state_name = s_AST;
                            parm_spec = p_AST;
                            return_spec = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(RETURNS_KYWD, "returns")).add((AST) astFactory.make((new ASTArray(2)).add(astFactory.create(RETURN_VALUE, "RETURN_VALUE")).add(tn_AST))));

                        }
                        break;
                    }
                    case LOOKUP_KYWD: {
                        match(LOOKUP_KYWD);
                        stateName();
                        sn_AST = (AST) returnAST;
                        {
                            switch (LA(1)) {
                                case LPAREN: {
                                    paramsSpec();
                                    ps_AST = (AST) returnAST;
                                    break;
                                }
                                case RETURNS_KYWD: {
                                    break;
                                }
                                default: {
                                    throw new NoViableAltException(LT(1), getFilename());
                                }
                            }
                        }
                        match(RETURNS_KYWD);
                        returnsSpec();
                        rs_AST = (AST) returnAST;
                        match(SEMICOLON);
                        if (inputState.guessing == 0) {

                            state_name = sn_AST;
                            parm_spec = ps_AST;
                            return_spec = rs_AST;

                        }
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            if (inputState.guessing == 0) {
                lookupDeclaration_AST = (AST) currentAST.root;

                // Actions for either syntax
                lookupDeclaration_AST = (AST) astFactory.make((new ASTArray(1)).add(astFactory.create(LOOKUP_KYWD, "Lookup")));
                lookupDeclaration_AST.addChild(state_name);
                lookupDeclaration_AST.addChild(return_spec);
                if (parm_spec != null) {
                    lookupDeclaration_AST.addChild(parm_spec);
                }

                // *** to do:
                //  - massage param spec data
                //  - massage return spec data
                // *** may throw exception if lookup already defined!
                globalContext.addLookupName(s_AST.getText(), parm_spec, return_spec);

                currentAST.root = lookupDeclaration_AST;
                currentAST.child = lookupDeclaration_AST != null && lookupDeclaration_AST.getFirstChild() != null ? lookupDeclaration_AST.getFirstChild() : lookupDeclaration_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_10);
            } else {
                throw ex;
            }
        }
        returnAST = lookupDeclaration_AST;
    }

    public final void libraryNodeDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST libraryNodeDeclaration_AST = null;
        AST n_AST = null;
        AST li_AST = null;

        try {      // for error handling
            AST tmp13_AST = null;
            tmp13_AST = astFactory.create(LT(1));
            astFactory.makeASTRoot(currentAST, tmp13_AST);
            match(LIBRARY_NODE_KYWD);
            nodeName();
            n_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            {
                switch (LA(1)) {
                    case LPAREN: {
                        libraryInterfaceSpec();
                        li_AST = (AST) returnAST;
                        astFactory.addASTChild(currentAST, returnAST);
                        break;
                    }
                    case SEMICOLON: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(SEMICOLON);
            if (inputState.guessing == 0) {

                globalContext.addLibraryNode(n_AST.getText(), li_AST);

            }
            libraryNodeDeclaration_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_10);
            } else {
                throw ex;
            }
        }
        returnAST = libraryNodeDeclaration_AST;
    }

    public final void typeName() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST typeName_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case BLOB_KYWD: {
                    AST tmp15_AST = null;
                    tmp15_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp15_AST);
                    match(BLOB_KYWD);
                    typeName_AST = (AST) currentAST.root;
                    break;
                }
                case BOOLEAN_KYWD: {
                    AST tmp16_AST = null;
                    tmp16_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp16_AST);
                    match(BOOLEAN_KYWD);
                    typeName_AST = (AST) currentAST.root;
                    break;
                }
                case INTEGER_KYWD: {
                    AST tmp17_AST = null;
                    tmp17_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp17_AST);
                    match(INTEGER_KYWD);
                    typeName_AST = (AST) currentAST.root;
                    break;
                }
                case REAL_KYWD: {
                    AST tmp18_AST = null;
                    tmp18_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp18_AST);
                    match(REAL_KYWD);
                    typeName_AST = (AST) currentAST.root;
                    break;
                }
                case STRING_KYWD: {
                    AST tmp19_AST = null;
                    tmp19_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp19_AST);
                    match(STRING_KYWD);
                    typeName_AST = (AST) currentAST.root;
                    break;
                }
                case TIME_KYWD: {
                    AST tmp20_AST = null;
                    tmp20_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp20_AST);
                    match(TIME_KYWD);
                    typeName_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_11);
            } else {
                throw ex;
            }
        }
        returnAST = typeName_AST;
    }

    public final void commandName() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST commandName_AST = null;

        try {      // for error handling
            ncName();
            astFactory.addASTChild(currentAST, returnAST);
            commandName_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_12);
            } else {
                throw ex;
            }
        }
        returnAST = commandName_AST;
    }

    public final void paramsSpec() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST paramsSpec_AST = null;

        try {      // for error handling
            match(LPAREN);
            {
                switch (LA(1)) {
                    case BOOLEAN_KYWD:
                    case INTEGER_KYWD:
                    case REAL_KYWD:
                    case STRING_KYWD:
                    case BLOB_KYWD:
                    case TIME_KYWD: {
                        paramSpec();
                        astFactory.addASTChild(currentAST, returnAST);
                        {
                            _loop23:
                            do {
                                if ((LA(1) == COMMA)) {
                                    match(COMMA);
                                    paramSpec();
                                    astFactory.addASTChild(currentAST, returnAST);
                                } else {
                                    break _loop23;
                                }

                            } while (true);
                        }
                        break;
                    }
                    case RPAREN: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(RPAREN);
            if (inputState.guessing == 0) {
                paramsSpec_AST = (AST) currentAST.root;
                paramsSpec_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(PARAMETER_DECLARATIONS, "PARAMETER_DECLARATIONS")).add(paramsSpec_AST));
                currentAST.root = paramsSpec_AST;
                currentAST.child = paramsSpec_AST != null && paramsSpec_AST.getFirstChild() != null ? paramsSpec_AST.getFirstChild() : paramsSpec_AST;
                currentAST.advanceChildToEnd();
            }
            paramsSpec_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_13);
            } else {
                throw ex;
            }
        }
        returnAST = paramsSpec_AST;
    }

    public final void stateName() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST stateName_AST = null;

        try {      // for error handling
            ncName();
            astFactory.addASTChild(currentAST, returnAST);
            stateName_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_14);
            } else {
                throw ex;
            }
        }
        returnAST = stateName_AST;
    }

    public final void returnsSpec() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST returnsSpec_AST = null;

        try {      // for error handling
            returnSpec();
            astFactory.addASTChild(currentAST, returnAST);
            {
                _loop28:
                do {
                    if ((LA(1) == COMMA)) {
                        match(COMMA);
                        returnSpec();
                        astFactory.addASTChild(currentAST, returnAST);
                    } else {
                        break _loop28;
                    }

                } while (true);
            }
            if (inputState.guessing == 0) {
                returnsSpec_AST = (AST) currentAST.root;
                returnsSpec_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(RETURNS_KYWD, "returns")).add(returnsSpec_AST));
                currentAST.root = returnsSpec_AST;
                currentAST.child = returnsSpec_AST != null && returnsSpec_AST.getFirstChild() != null ? returnsSpec_AST.getFirstChild() : returnsSpec_AST;
                currentAST.advanceChildToEnd();
            }
            returnsSpec_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_15);
            } else {
                throw ex;
            }
        }
        returnAST = returnsSpec_AST;
    }

    public final void functionName() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST functionName_AST = null;

        try {      // for error handling
            ncName();
            astFactory.addASTChild(currentAST, returnAST);
            functionName_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_12);
            } else {
                throw ex;
            }
        }
        returnAST = functionName_AST;
    }

    public final void paramSpec() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST paramSpec_AST = null;

        try {      // for error handling
            typeName();
            astFactory.addASTChild(currentAST, returnAST);
            {
                switch (LA(1)) {
                    case IDENT: {
                        paramName();
                        astFactory.addASTChild(currentAST, returnAST);
                        break;
                    }
                    case COMMA:
                    case RPAREN: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            if (inputState.guessing == 0) {
                paramSpec_AST = (AST) currentAST.root;
                paramSpec_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(PARAMETER, "PARAMETER")).add(paramSpec_AST));
                currentAST.root = paramSpec_AST;
                currentAST.child = paramSpec_AST != null && paramSpec_AST.getFirstChild() != null ? paramSpec_AST.getFirstChild() : paramSpec_AST;
                currentAST.advanceChildToEnd();
            }
            paramSpec_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_16);
            } else {
                throw ex;
            }
        }
        returnAST = paramSpec_AST;
    }

    public final void paramName() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST paramName_AST = null;

        try {      // for error handling
            ncName();
            astFactory.addASTChild(currentAST, returnAST);
            paramName_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_17);
            } else {
                throw ex;
            }
        }
        returnAST = paramName_AST;
    }

    public final void returnSpec() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST returnSpec_AST = null;

        try {      // for error handling
            typeName();
            {
                switch (LA(1)) {
                    case IDENT: {
                        paramName();
                        break;
                    }
                    case SEMICOLON:
                    case COMMA: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            if (inputState.guessing == 0) {
                returnSpec_AST = (AST) currentAST.root;
                returnSpec_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(RETURN_VALUE, "RETURN_VALUE")).add(returnSpec_AST));
                currentAST.root = returnSpec_AST;
                currentAST.child = returnSpec_AST != null && returnSpec_AST.getFirstChild() != null ? returnSpec_AST.getFirstChild() : returnSpec_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = returnSpec_AST;
    }

    public final void ncName() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST ncName_AST = null;

        try {      // for error handling
            {
                AST tmp25_AST = null;
                tmp25_AST = astFactory.create(LT(1));
                astFactory.addASTChild(currentAST, tmp25_AST);
                match(IDENT);
                {
                    _loop341:
                    do {
                        if (LA(1) == PERIOD && (_tokenSet_19.member(LA(2))) && (_tokenSet_20.member(LA(3)))) {
                            {
                                switch (LA(1)) {
                                    case PERIOD: {
                                        AST tmp26_AST = null;
                                        tmp26_AST = astFactory.create(LT(1));
                                        astFactory.addASTChild(currentAST, tmp26_AST);
                                        match(PERIOD);
                                        break;
                                    }
                                    case MINUS: {
                                        AST tmp27_AST = null;
                                        tmp27_AST = astFactory.create(LT(1));
                                        astFactory.addASTChild(currentAST, tmp27_AST);
                                        match(MINUS);
                                        break;
                                    }
                                    default: {
                                        throw new NoViableAltException(LT(1), getFilename());
                                    }
                                }
                            }
                            {
                                switch (LA(1)) {
                                    case IDENT: {
                                        AST tmp28_AST = null;
                                        tmp28_AST = astFactory.create(LT(1));
                                        astFactory.addASTChild(currentAST, tmp28_AST);
                                        match(IDENT);
                                        break;
                                    }
                                    case EOF:
                                    case RETURNS_KYWD:
                                    case OR_KYWD:
                                    case AND_KYWD:
                                    case SEMICOLON:
                                    case LPAREN:
                                    case COMMA:
                                    case RPAREN:
                                    case EQUALS:
                                    case LBRACKET:
                                    case RBRACKET:
                                    case OR_ALT_KYWD:
                                    case AND_ALT_KYWD:
                                    case DEQUALS:
                                    case NEQUALS:
                                    case GREATER:
                                    case GEQ:
                                    case LESS:
                                    case LEQ:
                                    case PERIOD:
                                    case PLUS:
                                    case MINUS:
                                    case ASTERISK:
                                    case SLASH:
                                    case BAR: {
                                        break;
                                    }
                                    default: {
                                        throw new NoViableAltException(LT(1), getFilename());
                                    }
                                }
                            }
                        } else {
                            break _loop341;
                        }

                    } while (true);
                }
            }
            if (inputState.guessing == 0) {
                ncName_AST = (AST) currentAST.root;

                String myName = ncName_AST.getText();
                for (AST part = ncName_AST.getNextSibling(); part != null; part = part.getNextSibling()) {
                    myName = myName + part.getText();
                }
                Token nm = new antlr.CommonToken(NCName, myName);
                nm.setLine(ncName_AST.getLine());
                nm.setColumn(ncName_AST.getColumn());
                ncName_AST = astFactory.create(nm);

                currentAST.root = ncName_AST;
                currentAST.child = ncName_AST != null && ncName_AST.getFirstChild() != null ? ncName_AST.getFirstChild() : ncName_AST;
                currentAST.advanceChildToEnd();
            }
            ncName_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_21);
            } else {
                throw ex;
            }
        }
        returnAST = ncName_AST;
    }

    public final void nodeName() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST nodeName_AST = null;

        try {      // for error handling
            {
                AST tmp29_AST = null;
                tmp29_AST = astFactory.create(LT(1));
                astFactory.addASTChild(currentAST, tmp29_AST);
                match(IDENT);
                {
                    _loop350:
                    do {
                        if (LA(1) == PERIOD && (_tokenSet_22.member(LA(2))) && (_tokenSet_23.member(LA(3)))) {
                            {
                                switch (LA(1)) {
                                    case PERIOD: {
                                        AST tmp30_AST = null;
                                        tmp30_AST = astFactory.create(LT(1));
                                        astFactory.addASTChild(currentAST, tmp30_AST);
                                        match(PERIOD);
                                        break;
                                    }
                                    case MINUS: {
                                        AST tmp31_AST = null;
                                        tmp31_AST = astFactory.create(LT(1));
                                        astFactory.addASTChild(currentAST, tmp31_AST);
                                        match(MINUS);
                                        break;
                                    }
                                    default: {
                                        throw new NoViableAltException(LT(1), getFilename());
                                    }
                                }
                            }
                            {
                                if ((LA(1) == IDENT) && (_tokenSet_22.member(LA(2))) && (_tokenSet_23.member(LA(3)))) {
                                    AST tmp32_AST = null;
                                    tmp32_AST = astFactory.create(LT(1));
                                    astFactory.addASTChild(currentAST, tmp32_AST);
                                    match(IDENT);
                                } else if ((_tokenSet_22.member(LA(1))) && (_tokenSet_23.member(LA(2))) && (_tokenSet_24.member(LA(3)))) {
                                } else {
                                    throw new NoViableAltException(LT(1), getFilename());
                                }

                            }
                        } else {
                            break _loop350;
                        }

                    } while (true);
                }
            }
            if (inputState.guessing == 0) {
                nodeName_AST = (AST) currentAST.root;

                String myName = nodeName_AST.getText();
                for (AST part = nodeName_AST.getNextSibling(); part != null; part = part.getNextSibling()) {
                    myName = myName + part.getText();
                }
                Token nm = new antlr.CommonToken(NodeName, myName);
                nm.setLine(nodeName_AST.getLine());
                nm.setColumn(nodeName_AST.getColumn());
                nodeName_AST = astFactory.create(nm);

                currentAST.root = nodeName_AST;
                currentAST.child = nodeName_AST != null && nodeName_AST.getFirstChild() != null ? nodeName_AST.getFirstChild() : nodeName_AST;
                currentAST.advanceChildToEnd();
            }
            nodeName_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_25);
            } else {
                throw ex;
            }
        }
        returnAST = nodeName_AST;
    }

    public final void libraryInterfaceSpec() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST libraryInterfaceSpec_AST = null;

        try {      // for error handling
            match(LPAREN);
            {
                switch (LA(1)) {
                    case IN_KYWD:
                    case IN_OUT_KYWD: {
                        libraryParamSpec();
                        astFactory.addASTChild(currentAST, returnAST);
                        {
                            _loop38:
                            do {
                                if ((LA(1) == COMMA)) {
                                    match(COMMA);
                                    libraryParamSpec();
                                    astFactory.addASTChild(currentAST, returnAST);
                                } else {
                                    break _loop38;
                                }

                            } while (true);
                        }
                        break;
                    }
                    case RPAREN: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(RPAREN);
            if (inputState.guessing == 0) {
                libraryInterfaceSpec_AST = (AST) currentAST.root;
                libraryInterfaceSpec_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(PARAMETER_DECLARATIONS, "PARAMETER_DECLARATIONS")).add(libraryInterfaceSpec_AST));
                currentAST.root = libraryInterfaceSpec_AST;
                currentAST.child = libraryInterfaceSpec_AST != null && libraryInterfaceSpec_AST.getFirstChild() != null ? libraryInterfaceSpec_AST.getFirstChild() : libraryInterfaceSpec_AST;
                currentAST.advanceChildToEnd();
            }
            libraryInterfaceSpec_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_15);
            } else {
                throw ex;
            }
        }
        returnAST = libraryInterfaceSpec_AST;
    }

    public final void libraryParamSpec() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST libraryParamSpec_AST = null;

        try {      // for error handling
            {
                switch (LA(1)) {
                    case IN_KYWD: {
                        AST tmp36_AST = null;
                        tmp36_AST = astFactory.create(LT(1));
                        astFactory.makeASTRoot(currentAST, tmp36_AST);
                        match(IN_KYWD);
                        break;
                    }
                    case IN_OUT_KYWD: {
                        AST tmp37_AST = null;
                        tmp37_AST = astFactory.create(LT(1));
                        astFactory.makeASTRoot(currentAST, tmp37_AST);
                        match(IN_OUT_KYWD);
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            typeName();
            astFactory.addASTChild(currentAST, returnAST);
            paramName();
            astFactory.addASTChild(currentAST, returnAST);
            if (inputState.guessing == 0) {
            }
            libraryParamSpec_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_16);
            } else {
                throw ex;
            }
        }
        returnAST = libraryParamSpec_AST;
    }

    public final void nodeId() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST nodeId_AST = null;

        try {      // for error handling
            nodeName();
            astFactory.addASTChild(currentAST, returnAST);
            if (inputState.guessing == 0) {
                nodeId_AST = (AST) currentAST.root;
                nodeId_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(NODE_ID, "NodeId")).add(nodeId_AST));
                currentAST.root = nodeId_AST;
                currentAST.child = nodeId_AST != null && nodeId_AST.getFirstChild() != null ? nodeId_AST.getFirstChild() : nodeId_AST;
                currentAST.advanceChildToEnd();
            }
            nodeId_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_26);
            } else {
                throw ex;
            }
        }
        returnAST = nodeId_AST;
    }

    public final void comment() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST comment_AST = null;

        try {      // for error handling
            AST tmp38_AST = null;
            tmp38_AST = astFactory.create(LT(1));
            astFactory.makeASTRoot(currentAST, tmp38_AST);
            match(COMMENT_KYWD);
            match(COLON);
            plexil.StringLiteralASTNode tmp40_AST = null;
            tmp40_AST = (plexil.StringLiteralASTNode) astFactory.create(LT(1), "plexil.StringLiteralASTNode");
            astFactory.addASTChild(currentAST, tmp40_AST);
            match(STRING);
            match(SEMICOLON);
            comment_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_27);
            } else {
                throw ex;
            }
        }
        returnAST = comment_AST;
    }

    public final void nodeDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST nodeDeclaration_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case IN_KYWD:
                case IN_OUT_KYWD: {
                    interfaceDeclaration();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeDeclaration_AST = (AST) currentAST.root;
                    break;
                }
                case BOOLEAN_KYWD:
                case INTEGER_KYWD:
                case REAL_KYWD:
                case STRING_KYWD:
                case BLOB_KYWD:
                case TIME_KYWD: {
                    variableDeclaration();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeDeclaration_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_27);
            } else {
                throw ex;
            }
        }
        returnAST = nodeDeclaration_AST;
    }

    public final void nodeAttribute() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST nodeAttribute_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case START_CONDITION_KYWD:
                case START_COND_KYWD: {
                    startCondition();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeAttribute_AST = (AST) currentAST.root;
                    break;
                }
                case REPEAT_CONDITION_KYWD:
                case REPEAT_KYWD: {
                    repeatCondition();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeAttribute_AST = (AST) currentAST.root;
                    break;
                }
                case SKIP_CONDITION_KYWD:
                case SKIP_KYWD: {
                    skipCondition();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeAttribute_AST = (AST) currentAST.root;
                    break;
                }
                case PRE_CONDITION_KYWD:
                case PRE_KYWD: {
                    preCondition();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeAttribute_AST = (AST) currentAST.root;
                    break;
                }
                case POST_CONDITION_KYWD:
                case POST_KYWD: {
                    postCondition();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeAttribute_AST = (AST) currentAST.root;
                    break;
                }
                case INVARIANT_CONDITION_KYWD:
                case INVARIANT_KYWD: {
                    invariantCondition();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeAttribute_AST = (AST) currentAST.root;
                    break;
                }
                case END_CONDITION_KYWD:
                case END_COND_KYWD: {
                    endCondition();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeAttribute_AST = (AST) currentAST.root;
                    break;
                }
                case PRIORITY_KYWD: {
                    priority();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeAttribute_AST = (AST) currentAST.root;
                    break;
                }
                case RESOURCE_KYWD: {
                    resource();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeAttribute_AST = (AST) currentAST.root;
                    break;
                }
                case RESOURCE_PRIORITY_KYWD: {
                    resourcePriority();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeAttribute_AST = (AST) currentAST.root;
                    break;
                }
                case PERMISSIONS_KYWD: {
                    permissions();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeAttribute_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_28);
            } else {
                throw ex;
            }
        }
        returnAST = nodeAttribute_AST;
    }

    public final void nodeBody() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST nodeBody_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case NODE_LIST_KYWD: {
                    nodeList();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeBody_AST = (AST) currentAST.root;
                    break;
                }
                case ASSIGNMENT_KYWD: {
                    assignment();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeBody_AST = (AST) currentAST.root;
                    break;
                }
                case COMMAND_KYWD: {
                    command();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeBody_AST = (AST) currentAST.root;
                    break;
                }
                case FUNCTION_CALL_KYWD: {
                    functionCall();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeBody_AST = (AST) currentAST.root;
                    break;
                }
                case UPDATE_KYWD: {
                    update();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeBody_AST = (AST) currentAST.root;
                    break;
                }
                case REQUEST_KYWD: {
                    request();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeBody_AST = (AST) currentAST.root;
                    break;
                }
                case LIBRARY_CALL_KYWD: {
                    libraryCall();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeBody_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_29);
            } else {
                throw ex;
            }
        }
        returnAST = nodeBody_AST;
    }

    public final void nodeList() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST nodeList_AST = null;

        try {      // for error handling
            AST tmp42_AST = null;
            tmp42_AST = astFactory.create(LT(1));
            astFactory.makeASTRoot(currentAST, tmp42_AST);
            match(NODE_LIST_KYWD);
            match(COLON);
            {
                _loop54:
                do {
                    if ((LA(1) == LBRACE || LA(1) == IDENT)) {
                        node();
                        astFactory.addASTChild(currentAST, returnAST);
                    } else {
                        break _loop54;
                    }

                } while (true);
            }
            nodeList_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_29);
            } else {
                throw ex;
            }
        }
        returnAST = nodeList_AST;
    }

    public final void assignment() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST assignment_AST = null;
        AST ar_AST = null;
        AST var_AST = null;
        AST ae_AST = null;
        AST be_AST = null;
        AST ie_AST = null;
        AST re_AST = null;
        AST se_AST = null;
        AST te_AST = null;

        AST lhs = null;
        AST variable = null;
        PlexilDataType assignType = null;


        try {      // for error handling
            match(ASSIGNMENT_KYWD);
            match(COLON);
            {
                boolean synPredMatched223 = false;
                if (((LA(1) == IDENT) && (LA(2) == LBRACKET || LA(2) == PERIOD) && (_tokenSet_30.member(LA(3))))) {
                    int _m223 = mark();
                    synPredMatched223 = true;
                    inputState.guessing++;
                    try {
                        {
                            ncName();
                            match(LBRACKET);
                        }
                    } catch (RecognitionException pe) {
                        synPredMatched223 = false;
                    }
                    rewind(_m223);
                    inputState.guessing--;
                }
                if (synPredMatched223) {
                    arrayReference();
                    ar_AST = (AST) returnAST;
                    if (inputState.guessing == 0) {

                        lhs = ar_AST;
                        variable = lhs.getFirstChild();
                        assignType = context.findVariable(variable.getText()).getArrayElementType();

                    }
                } 
                else if ((LA(1) == IDENT) && (LA(2) == EQUALS || LA(2) == PERIOD) && (_tokenSet_31.member(LA(3)))) {
                    variable();
                    var_AST = (AST) returnAST;
                    if (inputState.guessing == 0) {

                        lhs = var_AST;
                        variable = lhs;
                        assignType = context.findVariable(variable.getText()).getVariableType();

                    }
                } else {
                    throw new NoViableAltException(LT(1), getFilename());
                }

            }
            match(EQUALS);
            {
                if ((((LA(1) >= LOOKUP_WITH_FREQ_KYWD && LA(1) <= LOOKUP_NOW_KYWD)) && (LA(2) == LPAREN || LA(2) == LBRACKET) && (LA(3) == INT || LA(3) == DOUBLE || LA(3) == IDENT)) && (assignType.isArray())) {
                    arrayExpression();
                    ae_AST = (AST) returnAST;
                    if (inputState.guessing == 0) {
                        assignment_AST = (AST) currentAST.root;
                        assignment_AST = (AST) astFactory.make((new ASTArray(3)).add(astFactory.create(ARRAY_ASSIGNMENT, "ARRAY_ASSIGNMENT")).add(lhs).add(ae_AST));
                        currentAST.root = assignment_AST;
                        currentAST.child = assignment_AST != null && assignment_AST.getFirstChild() != null ? assignment_AST.getFirstChild() : assignment_AST;
                        currentAST.advanceChildToEnd();
                    }
                } else if (((_tokenSet_32.member(LA(1))) && (_tokenSet_33.member(LA(2))) && (_tokenSet_34.member(LA(3)))) && (assignType == PlexilDataType.BOOLEAN_TYPE)) {
                    booleanExpression();
                    be_AST = (AST) returnAST;
                    if (inputState.guessing == 0) {
                        assignment_AST = (AST) currentAST.root;
                        assignment_AST = (AST) astFactory.make((new ASTArray(3)).add(astFactory.create(BOOLEAN_ASSIGNMENT, "BOOLEAN_ASSIGNMENT")).add(lhs).add(be_AST));
                        currentAST.root = assignment_AST;
                        currentAST.child = assignment_AST != null && assignment_AST.getFirstChild() != null ? assignment_AST.getFirstChild() : assignment_AST;
                        currentAST.advanceChildToEnd();
                    }
                } else if (((_tokenSet_35.member(LA(1))) && (_tokenSet_36.member(LA(2))) && (_tokenSet_37.member(LA(3)))) && (assignType == PlexilDataType.INTEGER_TYPE)) {
                    numericExpression();
                    ie_AST = (AST) returnAST;
                    if (inputState.guessing == 0) {
                        assignment_AST = (AST) currentAST.root;
                        assignment_AST = (AST) astFactory.make((new ASTArray(3)).add(astFactory.create(INTEGER_ASSIGNMENT, "INTEGER_ASSIGNMENT")).add(lhs).add(ie_AST));
                        currentAST.root = assignment_AST;
                        currentAST.child = assignment_AST != null && assignment_AST.getFirstChild() != null ? assignment_AST.getFirstChild() : assignment_AST;
                        currentAST.advanceChildToEnd();
                    }
                } else if (((_tokenSet_35.member(LA(1))) && (_tokenSet_36.member(LA(2))) && (_tokenSet_37.member(LA(3)))) && (assignType == PlexilDataType.REAL_TYPE)) {
                    numericExpression();
                    re_AST = (AST) returnAST;
                    if (inputState.guessing == 0) {
                        assignment_AST = (AST) currentAST.root;
                        assignment_AST = (AST) astFactory.make((new ASTArray(3)).add(astFactory.create(REAL_ASSIGNMENT, "REAL_ASSIGNMENT")).add(lhs).add(re_AST));
                        currentAST.root = assignment_AST;
                        currentAST.child = assignment_AST != null && assignment_AST.getFirstChild() != null ? assignment_AST.getFirstChild() : assignment_AST;
                        currentAST.advanceChildToEnd();
                    }
                } else if (((_tokenSet_38.member(LA(1))) && (_tokenSet_39.member(LA(2))) && (_tokenSet_40.member(LA(3)))) && (assignType == PlexilDataType.STRING_TYPE)) {
                    stringExpression();
                    se_AST = (AST) returnAST;
                    if (inputState.guessing == 0) {
                        assignment_AST = (AST) currentAST.root;
                        assignment_AST = (AST) astFactory.make((new ASTArray(3)).add(astFactory.create(STRING_ASSIGNMENT, "STRING_ASSIGNMENT")).add(lhs).add(se_AST));
                        currentAST.root = assignment_AST;
                        currentAST.child = assignment_AST != null && assignment_AST.getFirstChild() != null ? assignment_AST.getFirstChild() : assignment_AST;
                        currentAST.advanceChildToEnd();
                    }
                } else if (((_tokenSet_41.member(LA(1))) && (_tokenSet_42.member(LA(2))) && (_tokenSet_43.member(LA(3)))) && (assignType == PlexilDataType.TIME_TYPE)) {
                    timeExpression();
                    te_AST = (AST) returnAST;
                    if (inputState.guessing == 0) {
                        assignment_AST = (AST) currentAST.root;
                        assignment_AST = (AST) astFactory.make((new ASTArray(3)).add(astFactory.create(TIME_ASSIGNMENT, "TIME_ASSIGNMENT")).add(lhs).add(te_AST));
                        currentAST.root = assignment_AST;
                        currentAST.child = assignment_AST != null && assignment_AST.getFirstChild() != null ? assignment_AST.getFirstChild() : assignment_AST;
                        currentAST.advanceChildToEnd();
                    }
                } else {
                    throw new NoViableAltException(LT(1), getFilename());
                }

            }
            match(SEMICOLON);
            if (inputState.guessing == 0) {

                if (!context.isAssignableVariableName(variable.getText())) {
                    throw new antlr.SemanticException("Variable '" + variable.getText() + "' is declared an In interface variable and may not be assigned");
                }

            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_29);
            } else {
                throw ex;
            }
        }
        returnAST = assignment_AST;
    }

    public final void command() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST command_AST = null;
        AST ar_AST = null;
        AST v_AST = null;
        AST c_AST = null;

        AST lhs = null;
        AST variable = null;


        try {      // for error handling
            AST tmp48_AST = null;
            tmp48_AST = astFactory.create(LT(1));
            astFactory.makeASTRoot(currentAST, tmp48_AST);
            match(COMMAND_KYWD);
            match(COLON);
            {
                boolean synPredMatched202 = false;
                if (((LA(1) == IDENT) && (LA(2) == LBRACKET || LA(2) == PERIOD) && (_tokenSet_30.member(LA(3))))) {
                    int _m202 = mark();
                    synPredMatched202 = true;
                    inputState.guessing++;
                    try {
                        {
                            ncName();
                            match(LBRACKET);
                        }
                    } catch (RecognitionException pe) {
                        synPredMatched202 = false;
                    }
                    rewind(_m202);
                    inputState.guessing--;
                }
                if (synPredMatched202) {
                    arrayReference();
                    ar_AST = (AST) returnAST;
                    astFactory.addASTChild(currentAST, returnAST);
                    AST tmp50_AST = null;
                    tmp50_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp50_AST);
                    match(EQUALS);
                    commandName();
                    astFactory.addASTChild(currentAST, returnAST);
                    if (inputState.guessing == 0) {

                        lhs = ar_AST;
                        variable = lhs.getFirstChild();

                    }
                } else {
                    boolean synPredMatched204 = false;
                    if (((LA(1) == IDENT) && (LA(2) == EQUALS || LA(2) == PERIOD) && (_tokenSet_44.member(LA(3))))) {
                        int _m204 = mark();
                        synPredMatched204 = true;
                        inputState.guessing++;
                        try {
                            {
                                ncName();
                                match(EQUALS);
                            }
                        } catch (RecognitionException pe) {
                            synPredMatched204 = false;
                        }
                        rewind(_m204);
                        inputState.guessing--;
                    }
                    if (synPredMatched204) {
                        variable();
                        v_AST = (AST) returnAST;
                        astFactory.addASTChild(currentAST, returnAST);
                        AST tmp51_AST = null;
                        tmp51_AST = astFactory.create(LT(1));
                        astFactory.addASTChild(currentAST, tmp51_AST);
                        match(EQUALS);
                        commandName();
                        astFactory.addASTChild(currentAST, returnAST);
                        if (inputState.guessing == 0) {

                            lhs = v_AST;
                            variable = lhs;

                        }
                    } else if ((LA(1) == IDENT) && (_tokenSet_45.member(LA(2))) && (_tokenSet_46.member(LA(3)))) {
                        commandName();
                        c_AST = (AST) returnAST;
                        astFactory.addASTChild(currentAST, returnAST);
                        if (!(!context.isVariableName(c_AST.getText()))) {
                            throw new SemanticException(" !context.isVariableName(c_AST.getText()) ");
                        }
                    } else {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            {
                switch (LA(1)) {
                    case LPAREN: {
                        match(LPAREN);
                        {
                            switch (LA(1)) {
                                case TRUE_KYWD:
                                case FALSE_KYWD:
                                case INT:
                                case DOUBLE:
                                case STRING:
                                case IDENT: {
                                    argumentList();
                                    astFactory.addASTChild(currentAST, returnAST);
                                    break;
                                }
                                case RPAREN: {
                                    break;
                                }
                                default: {
                                    throw new NoViableAltException(LT(1), getFilename());
                                }
                            }
                        }
                        match(RPAREN);
                        break;
                    }
                    case SEMICOLON: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(SEMICOLON);
            if (inputState.guessing == 0) {

                if (variable != null && !context.isAssignableVariableName(variable.getText())) {
                    throw createSemanticException("Variable '" + variable.getText() + "' is declared an In interface variable and may not be assigned",
                            v_AST);
                }

                Vector nodeResources = context.getResources();
                if (!nodeResources.isEmpty() && (context.getResourcePriorityAST() == null)) {
                    // Check that a priority is supplied for each resource
                    for (int i = 0; i < nodeResources.size(); i++) {
                        AST resource = (AST) nodeResources.get(i);
                        AST child = resource.getFirstChild();
                        while (child != null) {
                            if (child.getType() == PRIORITY_KYWD) {
                                break;
                            }
                            child = child.getNextSibling();
                        }
                        if (child == null) {
                            throw createSemanticException("No priority supplied for resource", resource);
                        }
                    }
                }

            }
            command_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_29);
            } else {
                throw ex;
            }
        }
        returnAST = command_AST;
    }

    public final void functionCall() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST functionCall_AST = null;
        AST ar_AST = null;
        AST v_AST = null;
        AST f_AST = null;

        AST lhs = null;
        AST variable = null;


        try {      // for error handling
            AST tmp55_AST = null;
            tmp55_AST = astFactory.create(LT(1));
            astFactory.makeASTRoot(currentAST, tmp55_AST);
            match(FUNCTION_CALL_KYWD);
            match(COLON);
            {
                boolean synPredMatched215 = false;
                if (((LA(1) == IDENT) && (LA(2) == LBRACKET || LA(2) == PERIOD) && (_tokenSet_30.member(LA(3))))) {
                    int _m215 = mark();
                    synPredMatched215 = true;
                    inputState.guessing++;
                    try {
                        {
                            ncName();
                            match(LBRACKET);
                        }
                    } catch (RecognitionException pe) {
                        synPredMatched215 = false;
                    }
                    rewind(_m215);
                    inputState.guessing--;
                }
                if (synPredMatched215) {
                    arrayReference();
                    ar_AST = (AST) returnAST;
                    astFactory.addASTChild(currentAST, returnAST);
                    AST tmp57_AST = null;
                    tmp57_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp57_AST);
                    match(EQUALS);
                    functionName();
                    astFactory.addASTChild(currentAST, returnAST);
                    if (inputState.guessing == 0) {

                        lhs = ar_AST;
                        variable = lhs.getFirstChild();

                    }
                } else {
                    boolean synPredMatched217 = false;
                    if (((LA(1) == IDENT) && (LA(2) == EQUALS || LA(2) == PERIOD) && (_tokenSet_44.member(LA(3))))) {
                        int _m217 = mark();
                        synPredMatched217 = true;
                        inputState.guessing++;
                        try {
                            {
                                ncName();
                                match(EQUALS);
                            }
                        } catch (RecognitionException pe) {
                            synPredMatched217 = false;
                        }
                        rewind(_m217);
                        inputState.guessing--;
                    }
                    if (synPredMatched217) {
                        variable();
                        v_AST = (AST) returnAST;
                        astFactory.addASTChild(currentAST, returnAST);
                        AST tmp58_AST = null;
                        tmp58_AST = astFactory.create(LT(1));
                        astFactory.addASTChild(currentAST, tmp58_AST);
                        match(EQUALS);
                        functionName();
                        astFactory.addASTChild(currentAST, returnAST);
                        if (inputState.guessing == 0) {

                            lhs = v_AST;
                            variable = lhs;

                        }
                    } 
                    else if ((LA(1) == IDENT) && (LA(2) == LPAREN || LA(2) == PERIOD) && (_tokenSet_47.member(LA(3)))) {
                        functionName();
                        f_AST = (AST) returnAST;
                        astFactory.addASTChild(currentAST, returnAST);
                        if (!(!context.isVariableName(f_AST.getText()))) {
                            throw new SemanticException(" !context.isVariableName(f_AST.getText()) ");
                        }
                    } else {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(LPAREN);
            {
                switch (LA(1)) {
                    case TRUE_KYWD:
                    case FALSE_KYWD:
                    case INT:
                    case DOUBLE:
                    case STRING:
                    case IDENT: {
                        argumentList();
                        astFactory.addASTChild(currentAST, returnAST);
                        break;
                    }
                    case RPAREN: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(RPAREN);
            match(SEMICOLON);
            if (inputState.guessing == 0) {

                if (variable != null && !context.isAssignableVariableName(variable.getText())) {
                    throw new antlr.SemanticException("Variable '" + variable.getText() + "' is declared an In interface variable and may not be assigned");
                }

            }
            functionCall_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_29);
            } else {
                throw ex;
            }
        }
        returnAST = functionCall_AST;
    }

    public final void update() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST update_AST = null;

        try {      // for error handling
            AST tmp62_AST = null;
            tmp62_AST = astFactory.create(LT(1));
            astFactory.makeASTRoot(currentAST, tmp62_AST);
            match(UPDATE_KYWD);
            match(COLON);
            {
                switch (LA(1)) {
                    case IDENT: {
                        pair();
                        astFactory.addASTChild(currentAST, returnAST);
                        {
                            _loop228:
                            do {
                                if ((LA(1) == COMMA)) {
                                    match(COMMA);
                                    pair();
                                    astFactory.addASTChild(currentAST, returnAST);
                                } else {
                                    break _loop228;
                                }

                            } while (true);
                        }
                        break;
                    }
                    case SEMICOLON: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(SEMICOLON);
            update_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_29);
            } else {
                throw ex;
            }
        }
        returnAST = update_AST;
    }

    public final void request() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST request_AST = null;

        try {      // for error handling
            AST tmp66_AST = null;
            tmp66_AST = astFactory.create(LT(1));
            astFactory.makeASTRoot(currentAST, tmp66_AST);
            match(REQUEST_KYWD);
            match(COLON);
            nodeName();
            astFactory.addASTChild(currentAST, returnAST);
            {
                switch (LA(1)) {
                    case IDENT: {
                        pair();
                        astFactory.addASTChild(currentAST, returnAST);
                        {
                            _loop232:
                            do {
                                if ((LA(1) == COMMA)) {
                                    match(COMMA);
                                    pair();
                                    astFactory.addASTChild(currentAST, returnAST);
                                } else {
                                    break _loop232;
                                }

                            } while (true);
                        }
                        break;
                    }
                    case SEMICOLON: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(SEMICOLON);
            request_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_29);
            } else {
                throw ex;
            }
        }
        returnAST = request_AST;
    }

    public final void libraryCall() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST libraryCall_AST = null;
        AST ln_AST = null;
        PlexilGlobalDeclaration lndec = null;

        try {      // for error handling
            AST tmp70_AST = null;
            tmp70_AST = astFactory.create(LT(1));
            astFactory.makeASTRoot(currentAST, tmp70_AST);
            match(LIBRARY_CALL_KYWD);
            match(COLON);
            libraryNodeIdRef();
            ln_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (inputState.guessing == 0) {

                if (!globalContext.isLibraryNodeName(ln_AST.getText())) {
                    throw createSemanticException("Node name \"" + ln_AST.getText() + "\" is not a declared library node",
                            ln_AST);
                }
                lndec = globalContext.getLibraryNode(ln_AST.getText());

            }
            {
                switch (LA(1)) {
                    case LPAREN: {
                        aliasSpecs(lndec);
                        astFactory.addASTChild(currentAST, returnAST);
                        break;
                    }
                    case SEMICOLON: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(SEMICOLON);
            libraryCall_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_29);
            } else {
                throw ex;
            }
        }
        returnAST = libraryCall_AST;
    }

    public final void interfaceDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST interfaceDeclaration_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case IN_KYWD: {
                    in();
                    astFactory.addASTChild(currentAST, returnAST);
                    interfaceDeclaration_AST = (AST) currentAST.root;
                    break;
                }
                case IN_OUT_KYWD: {
                    inOut();
                    astFactory.addASTChild(currentAST, returnAST);
                    interfaceDeclaration_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_27);
            } else {
                throw ex;
            }
        }
        returnAST = interfaceDeclaration_AST;
    }

    public final void variableDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST variableDeclaration_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case BOOLEAN_KYWD: {
                    booleanVariableDeclarations();
                    astFactory.addASTChild(currentAST, returnAST);
                    variableDeclaration_AST = (AST) currentAST.root;
                    break;
                }
                case INTEGER_KYWD: {
                    integerVariableDeclarations();
                    astFactory.addASTChild(currentAST, returnAST);
                    variableDeclaration_AST = (AST) currentAST.root;
                    break;
                }
                case REAL_KYWD: {
                    realVariableDeclarations();
                    astFactory.addASTChild(currentAST, returnAST);
                    variableDeclaration_AST = (AST) currentAST.root;
                    break;
                }
                case STRING_KYWD: {
                    stringVariableDeclarations();
                    astFactory.addASTChild(currentAST, returnAST);
                    variableDeclaration_AST = (AST) currentAST.root;
                    break;
                }
                case BLOB_KYWD: {
                    blobVariableDeclarations();
                    astFactory.addASTChild(currentAST, returnAST);
                    variableDeclaration_AST = (AST) currentAST.root;
                    break;
                }
                case TIME_KYWD: {
                    timeVariableDeclarations();
                    astFactory.addASTChild(currentAST, returnAST);
                    variableDeclaration_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_27);
            } else {
                throw ex;
            }
        }
        returnAST = variableDeclaration_AST;
    }

    public final void startCondition() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST startCondition_AST = null;

        try {      // for error handling
            {
                switch (LA(1)) {
                    case START_CONDITION_KYWD: {
                        match(START_CONDITION_KYWD);
                        break;
                    }
                    case START_COND_KYWD: {
                        match(START_COND_KYWD);
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(COLON);
            booleanExpression();
            astFactory.addASTChild(currentAST, returnAST);
            match(SEMICOLON);
            if (inputState.guessing == 0) {
                startCondition_AST = (AST) currentAST.root;
                startCondition_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(START_CONDITION_KYWD, "StartCondition")).add(startCondition_AST));
                currentAST.root = startCondition_AST;
                currentAST.child = startCondition_AST != null && startCondition_AST.getFirstChild() != null ? startCondition_AST.getFirstChild() : startCondition_AST;
                currentAST.advanceChildToEnd();
            }
            startCondition_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_28);
            } else {
                throw ex;
            }
        }
        returnAST = startCondition_AST;
    }

    public final void repeatCondition() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST repeatCondition_AST = null;

        try {      // for error handling
            {
                switch (LA(1)) {
                    case REPEAT_CONDITION_KYWD: {
                        match(REPEAT_CONDITION_KYWD);
                        break;
                    }
                    case REPEAT_KYWD: {
                        match(REPEAT_KYWD);
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(COLON);
            booleanExpression();
            astFactory.addASTChild(currentAST, returnAST);
            match(SEMICOLON);
            if (inputState.guessing == 0) {
                repeatCondition_AST = (AST) currentAST.root;
                repeatCondition_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(REPEAT_CONDITION_KYWD, "RepeatCondition")).add(repeatCondition_AST));
                currentAST.root = repeatCondition_AST;
                currentAST.child = repeatCondition_AST != null && repeatCondition_AST.getFirstChild() != null ? repeatCondition_AST.getFirstChild() : repeatCondition_AST;
                currentAST.advanceChildToEnd();
            }
            repeatCondition_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_28);
            } else {
                throw ex;
            }
        }
        returnAST = repeatCondition_AST;
    }

    public final void skipCondition() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST skipCondition_AST = null;

        try {      // for error handling
            {
                switch (LA(1)) {
                    case SKIP_CONDITION_KYWD: {
                        match(SKIP_CONDITION_KYWD);
                        break;
                    }
                    case SKIP_KYWD: {
                        match(SKIP_KYWD);
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(COLON);
            booleanExpression();
            astFactory.addASTChild(currentAST, returnAST);
            match(SEMICOLON);
            if (inputState.guessing == 0) {
                skipCondition_AST = (AST) currentAST.root;
                skipCondition_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(SKIP_CONDITION_KYWD, "SkipCondition")).add(skipCondition_AST));
                currentAST.root = skipCondition_AST;
                currentAST.child = skipCondition_AST != null && skipCondition_AST.getFirstChild() != null ? skipCondition_AST.getFirstChild() : skipCondition_AST;
                currentAST.advanceChildToEnd();
            }
            skipCondition_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_28);
            } else {
                throw ex;
            }
        }
        returnAST = skipCondition_AST;
    }

    public final void preCondition() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST preCondition_AST = null;

        try {      // for error handling
            {
                switch (LA(1)) {
                    case PRE_CONDITION_KYWD: {
                        match(PRE_CONDITION_KYWD);
                        break;
                    }
                    case PRE_KYWD: {
                        match(PRE_KYWD);
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(COLON);
            booleanExpression();
            astFactory.addASTChild(currentAST, returnAST);
            match(SEMICOLON);
            if (inputState.guessing == 0) {
                preCondition_AST = (AST) currentAST.root;
                preCondition_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(PRE_CONDITION_KYWD, "PreCondition")).add(preCondition_AST));
                currentAST.root = preCondition_AST;
                currentAST.child = preCondition_AST != null && preCondition_AST.getFirstChild() != null ? preCondition_AST.getFirstChild() : preCondition_AST;
                currentAST.advanceChildToEnd();
            }
            preCondition_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_28);
            } else {
                throw ex;
            }
        }
        returnAST = preCondition_AST;
    }

    public final void postCondition() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST postCondition_AST = null;

        try {      // for error handling
            {
                switch (LA(1)) {
                    case POST_CONDITION_KYWD: {
                        match(POST_CONDITION_KYWD);
                        break;
                    }
                    case POST_KYWD: {
                        match(POST_KYWD);
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(COLON);
            booleanExpression();
            astFactory.addASTChild(currentAST, returnAST);
            match(SEMICOLON);
            if (inputState.guessing == 0) {
                postCondition_AST = (AST) currentAST.root;
                postCondition_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(POST_CONDITION_KYWD, "PostCondition")).add(postCondition_AST));
                currentAST.root = postCondition_AST;
                currentAST.child = postCondition_AST != null && postCondition_AST.getFirstChild() != null ? postCondition_AST.getFirstChild() : postCondition_AST;
                currentAST.advanceChildToEnd();
            }
            postCondition_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_28);
            } else {
                throw ex;
            }
        }
        returnAST = postCondition_AST;
    }

    public final void invariantCondition() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST invariantCondition_AST = null;

        try {      // for error handling
            {
                switch (LA(1)) {
                    case INVARIANT_CONDITION_KYWD: {
                        match(INVARIANT_CONDITION_KYWD);
                        break;
                    }
                    case INVARIANT_KYWD: {
                        match(INVARIANT_KYWD);
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(COLON);
            booleanExpression();
            astFactory.addASTChild(currentAST, returnAST);
            match(SEMICOLON);
            if (inputState.guessing == 0) {
                invariantCondition_AST = (AST) currentAST.root;
                invariantCondition_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(INVARIANT_CONDITION_KYWD, "InvariantCondition")).add(invariantCondition_AST));
                currentAST.root = invariantCondition_AST;
                currentAST.child = invariantCondition_AST != null && invariantCondition_AST.getFirstChild() != null ? invariantCondition_AST.getFirstChild() : invariantCondition_AST;
                currentAST.advanceChildToEnd();
            }
            invariantCondition_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_28);
            } else {
                throw ex;
            }
        }
        returnAST = invariantCondition_AST;
    }

    public final void endCondition() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST endCondition_AST = null;

        try {      // for error handling
            {
                switch (LA(1)) {
                    case END_CONDITION_KYWD: {
                        match(END_CONDITION_KYWD);
                        break;
                    }
                    case END_COND_KYWD: {
                        match(END_COND_KYWD);
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(COLON);
            booleanExpression();
            astFactory.addASTChild(currentAST, returnAST);
            match(SEMICOLON);
            if (inputState.guessing == 0) {
                endCondition_AST = (AST) currentAST.root;
                endCondition_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(END_CONDITION_KYWD, "EndCondition")).add(endCondition_AST));
                currentAST.root = endCondition_AST;
                currentAST.child = endCondition_AST != null && endCondition_AST.getFirstChild() != null ? endCondition_AST.getFirstChild() : endCondition_AST;
                currentAST.advanceChildToEnd();
            }
            endCondition_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_28);
            } else {
                throw ex;
            }
        }
        returnAST = endCondition_AST;
    }

    public final void priority() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST priority_AST = null;

        try {      // for error handling
            AST tmp101_AST = null;
            tmp101_AST = astFactory.create(LT(1));
            astFactory.makeASTRoot(currentAST, tmp101_AST);
            match(PRIORITY_KYWD);
            match(COLON);
            nonNegativeInteger();
            astFactory.addASTChild(currentAST, returnAST);
            match(SEMICOLON);
            priority_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_28);
            } else {
                throw ex;
            }
        }
        returnAST = priority_AST;
    }

    public final void resource() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST resource_AST = null;
        AST lbe_AST = null;
        AST ube_AST = null;
        AST re_AST = null;
        AST pe_AST = null;

        AST lbExpr = null;
        AST ubExpr = null;
        AST releaseExpr = null;
        AST prioExpr = null;


        try {      // for error handling
            AST tmp104_AST = null;
            tmp104_AST = astFactory.create(LT(1));
            astFactory.makeASTRoot(currentAST, tmp104_AST);
            match(RESOURCE_KYWD);
            match(COLON);
            match(NAME_KYWD);
            match(EQUALS);
            stringExpression();
            astFactory.addASTChild(currentAST, returnAST);
            {
                _loop78:
                do {
                    if ((LA(1) == COMMA)) {
                        match(COMMA);
                        {
                            switch (LA(1)) {
                                case LOWER_BOUND_KYWD: {
                                    {
                                        AST tmp109_AST = null;
                                        tmp109_AST = astFactory.create(LT(1));
                                        astFactory.addASTChild(currentAST, tmp109_AST);
                                        match(LOWER_BOUND_KYWD);
                                        match(EQUALS);
                                        numericExpression();
                                        lbe_AST = (AST) returnAST;
                                        astFactory.addASTChild(currentAST, returnAST);
                                        if (inputState.guessing == 0) {
                                            if (lbExpr == null) {
                                                lbExpr = lbe_AST;
                                            } else {
                                                throw createSemanticException("Multiple LowerBound specifications in resource",
                                                        lbe_AST);
                                            }

                                        }
                                    }
                                    break;
                                }
                                case UPPER_BOUND_KYWD: {
                                    {
                                        AST tmp111_AST = null;
                                        tmp111_AST = astFactory.create(LT(1));
                                        astFactory.addASTChild(currentAST, tmp111_AST);
                                        match(UPPER_BOUND_KYWD);
                                        match(EQUALS);
                                        numericExpression();
                                        ube_AST = (AST) returnAST;
                                        astFactory.addASTChild(currentAST, returnAST);
                                        if (inputState.guessing == 0) {
                                            if (ubExpr == null) {
                                                ubExpr = ube_AST;
                                            } else {
                                                throw createSemanticException("Multiple UpperBound specifications in resource",
                                                        ube_AST);
                                            }

                                        }
                                    }
                                    break;
                                }
                                case RELEASE_AT_TERM_KYWD: {
                                    {
                                        AST tmp113_AST = null;
                                        tmp113_AST = astFactory.create(LT(1));
                                        astFactory.addASTChild(currentAST, tmp113_AST);
                                        match(RELEASE_AT_TERM_KYWD);
                                        match(EQUALS);
                                        booleanExpression();
                                        re_AST = (AST) returnAST;
                                        astFactory.addASTChild(currentAST, returnAST);
                                        if (inputState.guessing == 0) {
                                            if (releaseExpr == null) {
                                                releaseExpr = re_AST;
                                            } else {
                                                throw createSemanticException("Multiple ReleaseAtTermination specifications in resource",
                                                        re_AST);
                                            }

                                        }
                                    }
                                    break;
                                }
                                case PRIORITY_KYWD: {
                                    {
                                        AST tmp115_AST = null;
                                        tmp115_AST = astFactory.create(LT(1));
                                        astFactory.addASTChild(currentAST, tmp115_AST);
                                        match(PRIORITY_KYWD);
                                        match(EQUALS);
                                        numericExpression();
                                        pe_AST = (AST) returnAST;
                                        astFactory.addASTChild(currentAST, returnAST);
                                        if (inputState.guessing == 0) {
                                            if (prioExpr == null) {
                                                prioExpr = pe_AST;
                                            } else {
                                                throw createSemanticException("Multiple Priority specifications in resource",
                                                        pe_AST);
                                            }

                                        }
                                    }
                                    break;
                                }
                                default: {
                                    throw new NoViableAltException(LT(1), getFilename());
                                }
                            }
                        }
                    } else {
                        break _loop78;
                    }

                } while (true);
            }
            match(SEMICOLON);
            if (inputState.guessing == 0) {
                resource_AST = (AST) currentAST.root;

                context.addResource(resource_AST);

            }
            resource_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_28);
            } else {
                throw ex;
            }
        }
        returnAST = resource_AST;
    }

    public final void resourcePriority() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST resourcePriority_AST = null;

        try {      // for error handling
            AST tmp118_AST = null;
            tmp118_AST = astFactory.create(LT(1));
            astFactory.makeASTRoot(currentAST, tmp118_AST);
            match(RESOURCE_PRIORITY_KYWD);
            match(COLON);
            numericExpression();
            astFactory.addASTChild(currentAST, returnAST);
            match(SEMICOLON);
            if (inputState.guessing == 0) {
                resourcePriority_AST = (AST) currentAST.root;

                context.setResourcePriorityAST(resourcePriority_AST);

            }
            resourcePriority_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_28);
            } else {
                throw ex;
            }
        }
        returnAST = resourcePriority_AST;
    }

    public final void permissions() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST permissions_AST = null;

        try {      // for error handling
            AST tmp121_AST = null;
            tmp121_AST = astFactory.create(LT(1));
            astFactory.makeASTRoot(currentAST, tmp121_AST);
            match(PERMISSIONS_KYWD);
            match(COLON);
            plexil.StringLiteralASTNode tmp123_AST = null;
            tmp123_AST = (plexil.StringLiteralASTNode) astFactory.create(LT(1), "plexil.StringLiteralASTNode");
            astFactory.addASTChild(currentAST, tmp123_AST);
            match(STRING);
            match(SEMICOLON);
            permissions_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_28);
            } else {
                throw ex;
            }
        }
        returnAST = permissions_AST;
    }

    public final void booleanExpression() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST booleanExpression_AST = null;
        AST t1_AST = null;
        AST tn_AST = null;

        try {      // for error handling
            booleanTerm();
            t1_AST = (AST) returnAST;
            if (inputState.guessing == 0) {
                booleanExpression_AST = (AST) currentAST.root;
                booleanExpression_AST = t1_AST;
                currentAST.root = booleanExpression_AST;
                currentAST.child = booleanExpression_AST != null && booleanExpression_AST.getFirstChild() != null ? booleanExpression_AST.getFirstChild() : booleanExpression_AST;
                currentAST.advanceChildToEnd();
            }
            {
                _loop253:
                do {
                    if ((LA(1) == OR_KYWD || LA(1) == OR_ALT_KYWD)) {
                        {
                            switch (LA(1)) {
                                case OR_KYWD: {
                                    AST tmp125_AST = null;
                                    tmp125_AST = astFactory.create(LT(1));
                                    match(OR_KYWD);
                                    break;
                                }
                                case OR_ALT_KYWD: {
                                    AST tmp126_AST = null;
                                    tmp126_AST = astFactory.create(LT(1));
                                    match(OR_ALT_KYWD);
                                    break;
                                }
                                default: {
                                    throw new NoViableAltException(LT(1), getFilename());
                                }
                            }
                        }
                        booleanTerm();
                        tn_AST = (AST) returnAST;
                        if (inputState.guessing == 0) {
                            booleanExpression_AST = (AST) currentAST.root;
                            booleanExpression_AST = (AST) astFactory.make((new ASTArray(3)).add(astFactory.create(OR_KYWD, "OR")).add(booleanExpression_AST).add(tn_AST));
                            currentAST.root = booleanExpression_AST;
                            currentAST.child = booleanExpression_AST != null && booleanExpression_AST.getFirstChild() != null ? booleanExpression_AST.getFirstChild() : booleanExpression_AST;
                            currentAST.advanceChildToEnd();
                        }
                    } else {
                        break _loop253;
                    }

                } while (true);
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_17);
            } else {
                throw ex;
            }
        }
        returnAST = booleanExpression_AST;
    }

    public final void stringExpression() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST stringExpression_AST = null;
        AST q1_AST = null;
        AST qn_AST = null;

        try {      // for error handling
            stringQuantity();
            q1_AST = (AST) returnAST;
            if (inputState.guessing == 0) {
                stringExpression_AST = (AST) currentAST.root;
                stringExpression_AST = q1_AST;
                currentAST.root = stringExpression_AST;
                currentAST.child = stringExpression_AST != null && stringExpression_AST.getFirstChild() != null ? stringExpression_AST.getFirstChild() : stringExpression_AST;
                currentAST.advanceChildToEnd();
            }
            {
                _loop316:
                do {
                    if ((LA(1) == PLUS)) {
                        match(PLUS);
                        stringQuantity();
                        qn_AST = (AST) returnAST;
                        if (inputState.guessing == 0) {
                            stringExpression_AST = (AST) currentAST.root;
                            stringExpression_AST = (AST) astFactory.make((new ASTArray(3)).add(astFactory.create(CONCAT, "CONCAT")).add(stringExpression_AST).add(qn_AST));
                            currentAST.root = stringExpression_AST;
                            currentAST.child = stringExpression_AST != null && stringExpression_AST.getFirstChild() != null ? stringExpression_AST.getFirstChild() : stringExpression_AST;
                            currentAST.advanceChildToEnd();
                        }
                    } else {
                        break _loop316;
                    }

                } while (true);
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_48);
            } else {
                throw ex;
            }
        }
        returnAST = stringExpression_AST;
    }

    public final void numericExpression() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST numericExpression_AST = null;
        AST t1_AST = null;
        AST on_AST = null;
        AST tn_AST = null;

        try {      // for error handling
            numericTerm();
            t1_AST = (AST) returnAST;
            if (inputState.guessing == 0) {
                numericExpression_AST = (AST) currentAST.root;
                numericExpression_AST = t1_AST;
                currentAST.root = numericExpression_AST;
                currentAST.child = numericExpression_AST != null && numericExpression_AST.getFirstChild() != null ? numericExpression_AST.getFirstChild() : numericExpression_AST;
                currentAST.advanceChildToEnd();
            }
            {
                _loop304:
                do {
                    if ((LA(1) == PLUS || LA(1) == MINUS)) {
                        additiveOperator();
                        on_AST = (AST) returnAST;
                        numericTerm();
                        tn_AST = (AST) returnAST;
                        if (inputState.guessing == 0) {
                            numericExpression_AST = (AST) currentAST.root;
                            numericExpression_AST = (AST) astFactory.make((new ASTArray(3)).add(on_AST).add(numericExpression_AST).add(tn_AST));
                            currentAST.root = numericExpression_AST;
                            currentAST.child = numericExpression_AST != null && numericExpression_AST.getFirstChild() != null ? numericExpression_AST.getFirstChild() : numericExpression_AST;
                            currentAST.advanceChildToEnd();
                        }
                    } else {
                        break _loop304;
                    }

                } while (true);
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_49);
            } else {
                throw ex;
            }
        }
        returnAST = numericExpression_AST;
    }

    public final void nonNegativeInteger() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST nonNegativeInteger_AST = null;

        try {      // for error handling
            plexil.LiteralASTNode tmp128_AST = null;
            tmp128_AST = (plexil.LiteralASTNode) astFactory.create(LT(1), "plexil.LiteralASTNode");
            astFactory.addASTChild(currentAST, tmp128_AST);
            match(INT);
            nonNegativeInteger_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_50);
            } else {
                throw ex;
            }
        }
        returnAST = nonNegativeInteger_AST;
    }

    public final void in() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST in_AST = null;
        AST tn_AST = null;

        try {      // for error handling
            AST tmp129_AST = null;
            tmp129_AST = astFactory.create(LT(1));
            astFactory.makeASTRoot(currentAST, tmp129_AST);
            match(IN_KYWD);
            {
                switch (LA(1)) {
                    case BOOLEAN_KYWD:
                    case INTEGER_KYWD:
                    case REAL_KYWD:
                    case STRING_KYWD:
                    case BLOB_KYWD:
                    case TIME_KYWD: {
                        typeName();
                        tn_AST = (AST) returnAST;
                        astFactory.addASTChild(currentAST, returnAST);
                        break;
                    }
                    case IDENT: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            ncName();
            astFactory.addASTChild(currentAST, returnAST);
            {
                _loop87:
                do {
                    if ((LA(1) == COMMA)) {
                        match(COMMA);
                        ncName();
                        astFactory.addASTChild(currentAST, returnAST);
                    } else {
                        break _loop87;
                    }

                } while (true);
            }
            match(SEMICOLON);
            if (inputState.guessing == 0) {
                in_AST = (AST) currentAST.root;

                AST var = in_AST.getFirstChild();
                PlexilDataType typ = null;
                if (tn_AST == null) {
                    if (context.isLibraryNode()) {
                        throw createSemanticException("In statement in library node requires a variable type",
                                in_AST);
                    }
                } else {
                    typ = PlexilDataType.findByName(tn_AST.getText());
                    if (typ == null) {
                        throw createSemanticException("\"" + tn_AST.getText() + "\" is an unknown data type in an In statement",
                                tn_AST);
                    }
                    var = var.getNextSibling(); // skip over type
                }
                while (var != null) {
                    context.declareInterfaceVariable(var.getText(), false, typ);
                    var = var.getNextSibling();
                }

            }
            in_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_27);
            } else {
                throw ex;
            }
        }
        returnAST = in_AST;
    }

    public final void inOut() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST inOut_AST = null;
        AST tn_AST = null;

        try {      // for error handling
            AST tmp132_AST = null;
            tmp132_AST = astFactory.create(LT(1));
            astFactory.makeASTRoot(currentAST, tmp132_AST);
            match(IN_OUT_KYWD);
            {
                switch (LA(1)) {
                    case BOOLEAN_KYWD:
                    case INTEGER_KYWD:
                    case REAL_KYWD:
                    case STRING_KYWD:
                    case BLOB_KYWD:
                    case TIME_KYWD: {
                        typeName();
                        tn_AST = (AST) returnAST;
                        astFactory.addASTChild(currentAST, returnAST);
                        break;
                    }
                    case IDENT: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            ncName();
            astFactory.addASTChild(currentAST, returnAST);
            {
                _loop91:
                do {
                    if ((LA(1) == COMMA)) {
                        match(COMMA);
                        ncName();
                        astFactory.addASTChild(currentAST, returnAST);
                    } else {
                        break _loop91;
                    }

                } while (true);
            }
            match(SEMICOLON);
            if (inputState.guessing == 0) {
                inOut_AST = (AST) currentAST.root;

                AST var = inOut_AST.getFirstChild();
                PlexilDataType typ = null;
                if (tn_AST == null) {
                    if (context.isLibraryNode()) {
                        throw createSemanticException("InOut statement in library node missing variable type",
                                inOut_AST);
                    }
                } else {
                    typ = PlexilDataType.findByName(tn_AST.getText());
                    if (typ == null) {
                        throw createSemanticException("\"" + tn_AST.getText() + "\" is an unknown data type in an InOut statement",
                                tn_AST);
                    }
                    var = var.getNextSibling(); // skip over type
                }
                while (var != null) {
                    context.declareInterfaceVariable(var.getText(), true, typ);
                    var = var.getNextSibling();
                }

            }
            inOut_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_27);
            } else {
                throw ex;
            }
        }
        returnAST = inOut_AST;
    }

    public final void booleanVariable() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST booleanVariable_AST = null;
        AST v_AST = null;

        try {      // for error handling
            variable();
            v_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (!(context.isBooleanVariableName(v_AST.getText()))) {
                throw new SemanticException(" context.isBooleanVariableName(v_AST.getText()) ");
            }
            booleanVariable_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_48);
            } else {
                throw ex;
            }
        }
        returnAST = booleanVariable_AST;
    }

    public final void variable() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST variable_AST = null;
        AST n_AST = null;

        try {      // for error handling
            ncName();
            n_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (!(context.isVariableName(n_AST.getText()))) {
                throw new SemanticException(" context.isVariableName(n_AST.getText()) ");
            }
            if (inputState.guessing == 0) {
                variable_AST = (AST) currentAST.root;

                variable_AST = (plexil.VariableASTNode) astFactory.create(VARIABLE, n_AST.getText(), "plexil.VariableASTNode");
                ((VariableASTNode) variable_AST).setVariable(context.findVariable(n_AST.getText()));

                currentAST.root = variable_AST;
                currentAST.child = variable_AST != null && variable_AST.getFirstChild() != null ? variable_AST.getFirstChild() : variable_AST;
                currentAST.advanceChildToEnd();
            }
            variable_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_51);
            } else {
                throw ex;
            }
        }
        returnAST = variable_AST;
    }

    public final void integerVariable() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST integerVariable_AST = null;
        AST v_AST = null;

        try {      // for error handling
            variable();
            v_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (!(context.isIntegerVariableName(v_AST.getText()))) {
                throw new SemanticException(" context.isIntegerVariableName(v_AST.getText()) ");
            }
            integerVariable_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_52);
            } else {
                throw ex;
            }
        }
        returnAST = integerVariable_AST;
    }

    public final void blobVariable() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST blobVariable_AST = null;
        AST v_AST = null;

        try {      // for error handling
            variable();
            v_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (!(context.isBlobVariableName(v_AST.getText()))) {
                throw new SemanticException(" context.isBlobVariableName(v_AST.getText()) ");
            }
            blobVariable_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_0);
            } else {
                throw ex;
            }
        }
        returnAST = blobVariable_AST;
    }

    public final void realVariable() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST realVariable_AST = null;
        AST v_AST = null;

        try {      // for error handling
            variable();
            v_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (!(context.isRealVariableName(v_AST.getText()))) {
                throw new SemanticException(" context.isRealVariableName(v_AST.getText()) ");
            }
            realVariable_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_52);
            } else {
                throw ex;
            }
        }
        returnAST = realVariable_AST;
    }

    public final void stringVariable() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST stringVariable_AST = null;
        AST v_AST = null;

        try {      // for error handling
            variable();
            v_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (!(context.isStringVariableName(v_AST.getText()))) {
                throw new SemanticException(" context.isStringVariableName(v_AST.getText()) ");
            }
            stringVariable_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_53);
            } else {
                throw ex;
            }
        }
        returnAST = stringVariable_AST;
    }

    public final void timeVariable() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST timeVariable_AST = null;
        AST v_AST = null;

        try {      // for error handling
            variable();
            v_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (!(context.isTimeVariableName(v_AST.getText()))) {
                throw new SemanticException(" context.isTimeVariableName(v_AST.getText()) ");
            }
            timeVariable_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_48);
            } else {
                throw ex;
            }
        }
        returnAST = timeVariable_AST;
    }

    public final void arrayVariable() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST arrayVariable_AST = null;
        AST v_AST = null;

        try {      // for error handling
            variable();
            v_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (!(context.isArrayVariableName(v_AST.getText()))) {
                throw new SemanticException(" context.isArrayVariableName(v_AST.getText()) ");
            }
            arrayVariable_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_0);
            } else {
                throw ex;
            }
        }
        returnAST = arrayVariable_AST;
    }

    public final void booleanArrayVariable() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST booleanArrayVariable_AST = null;
        AST v_AST = null;

        try {      // for error handling
            variable();
            v_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (!(context.isBooleanArrayVariableName(v_AST.getText()))) {
                throw new SemanticException(" context.isBooleanArrayVariableName(v_AST.getText()) ");
            }
            booleanArrayVariable_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_0);
            } else {
                throw ex;
            }
        }
        returnAST = booleanArrayVariable_AST;
    }

    public final void integerArrayVariable() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST integerArrayVariable_AST = null;
        AST v_AST = null;

        try {      // for error handling
            variable();
            v_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (!(context.isIntegerArrayVariableName(v_AST.getText()))) {
                throw new SemanticException(" context.isIntegerArrayVariableName(v_AST.getText()) ");
            }
            integerArrayVariable_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_0);
            } else {
                throw ex;
            }
        }
        returnAST = integerArrayVariable_AST;
    }

    public final void blobArrayVariable() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST blobArrayVariable_AST = null;
        AST v_AST = null;

        try {      // for error handling
            variable();
            v_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (!(context.isBlobArrayVariableName(v_AST.getText()))) {
                throw new SemanticException(" context.isBlobArrayVariableName(v_AST.getText()) ");
            }
            blobArrayVariable_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_0);
            } else {
                throw ex;
            }
        }
        returnAST = blobArrayVariable_AST;
    }

    public final void realArrayVariable() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST realArrayVariable_AST = null;
        AST v_AST = null;

        try {      // for error handling
            variable();
            v_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (!(context.isRealArrayVariableName(v_AST.getText()))) {
                throw new SemanticException(" context.isRealArrayVariableName(v_AST.getText()) ");
            }
            realArrayVariable_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_0);
            } else {
                throw ex;
            }
        }
        returnAST = realArrayVariable_AST;
    }

    public final void stringArrayVariable() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST stringArrayVariable_AST = null;
        AST v_AST = null;

        try {      // for error handling
            variable();
            v_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (!(context.isStringArrayVariableName(v_AST.getText()))) {
                throw new SemanticException(" context.isStringArrayVariableName(v_AST.getText()) ");
            }
            stringArrayVariable_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_0);
            } else {
                throw ex;
            }
        }
        returnAST = stringArrayVariable_AST;
    }

    public final void timeArrayVariable() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST timeArrayVariable_AST = null;
        AST v_AST = null;

        try {      // for error handling
            variable();
            v_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (!(context.isTimeArrayVariableName(v_AST.getText()))) {
                throw new SemanticException(" context.isTimeArrayVariableName(v_AST.getText()) ");
            }
            timeArrayVariable_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_0);
            } else {
                throw ex;
            }
        }
        returnAST = timeArrayVariable_AST;
    }

    public final void booleanVariableDeclarations() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST booleanVariableDeclarations_AST = null;
        AST bvd1_AST = null;
        AST bvdn_AST = null;

        try {      // for error handling
            if (inputState.guessing == 0) {
                booleanVariableDeclarations_AST = (AST) currentAST.root;
                booleanVariableDeclarations_AST = (AST) astFactory.make((new ASTArray(1)).add(astFactory.create(VARIABLE_DECLARATION, "VARIABLE_DECLARATION")));
                currentAST.root = booleanVariableDeclarations_AST;
                currentAST.child = booleanVariableDeclarations_AST != null && booleanVariableDeclarations_AST.getFirstChild() != null ? booleanVariableDeclarations_AST.getFirstChild() : booleanVariableDeclarations_AST;
                currentAST.advanceChildToEnd();
            }
            match(BOOLEAN_KYWD);
            {
                switch (LA(1)) {
                    case IDENT: {
                        booleanVariableDeclaration();
                        bvd1_AST = (AST) returnAST;
                        if (inputState.guessing == 0) {
                            booleanVariableDeclarations_AST = (AST) currentAST.root;
                            booleanVariableDeclarations_AST.addChild(bvd1_AST);
                        }
                        {
                            _loop110:
                            do {
                                if ((LA(1) == COMMA)) {
                                    match(COMMA);
                                    booleanVariableDeclaration();
                                    bvdn_AST = (AST) returnAST;
                                    if (inputState.guessing == 0) {
                                        booleanVariableDeclarations_AST = (AST) currentAST.root;
                                        booleanVariableDeclarations_AST.addChild(bvdn_AST);
                                    }
                                } else {
                                    break _loop110;
                                }

                            } while (true);
                        }
                        break;
                    }
                    case SEMICOLON: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(SEMICOLON);
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_27);
            } else {
                throw ex;
            }
        }
        returnAST = booleanVariableDeclarations_AST;
    }

    public final void integerVariableDeclarations() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST integerVariableDeclarations_AST = null;
        AST ivd1_AST = null;
        AST ivdn_AST = null;

        try {      // for error handling
            if (inputState.guessing == 0) {
                integerVariableDeclarations_AST = (AST) currentAST.root;
                integerVariableDeclarations_AST = (AST) astFactory.make((new ASTArray(1)).add(astFactory.create(VARIABLE_DECLARATION, "VARIABLE_DECLARATION")));
                currentAST.root = integerVariableDeclarations_AST;
                currentAST.child = integerVariableDeclarations_AST != null && integerVariableDeclarations_AST.getFirstChild() != null ? integerVariableDeclarations_AST.getFirstChild() : integerVariableDeclarations_AST;
                currentAST.advanceChildToEnd();
            }
            match(INTEGER_KYWD);
            {
                switch (LA(1)) {
                    case IDENT: {
                        integerVariableDeclaration();
                        ivd1_AST = (AST) returnAST;
                        if (inputState.guessing == 0) {
                            integerVariableDeclarations_AST = (AST) currentAST.root;
                            integerVariableDeclarations_AST.addChild(ivd1_AST);
                        }
                        {
                            _loop126:
                            do {
                                if ((LA(1) == COMMA)) {
                                    match(COMMA);
                                    integerVariableDeclaration();
                                    ivdn_AST = (AST) returnAST;
                                    if (inputState.guessing == 0) {
                                        integerVariableDeclarations_AST = (AST) currentAST.root;
                                        integerVariableDeclarations_AST.addChild(ivdn_AST);
                                    }
                                } else {
                                    break _loop126;
                                }

                            } while (true);
                        }
                        break;
                    }
                    case SEMICOLON: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(SEMICOLON);
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_27);
            } else {
                throw ex;
            }
        }
        returnAST = integerVariableDeclarations_AST;
    }

    public final void realVariableDeclarations() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST realVariableDeclarations_AST = null;
        AST rvd1_AST = null;
        AST rvdn_AST = null;

        try {      // for error handling
            if (inputState.guessing == 0) {
                realVariableDeclarations_AST = (AST) currentAST.root;
                realVariableDeclarations_AST = (AST) astFactory.make((new ASTArray(1)).add(astFactory.create(VARIABLE_DECLARATION, "VARIABLE_DECLARATION")));
                currentAST.root = realVariableDeclarations_AST;
                currentAST.child = realVariableDeclarations_AST != null && realVariableDeclarations_AST.getFirstChild() != null ? realVariableDeclarations_AST.getFirstChild() : realVariableDeclarations_AST;
                currentAST.advanceChildToEnd();
            }
            match(REAL_KYWD);
            {
                switch (LA(1)) {
                    case IDENT: {
                        realVariableDeclaration();
                        rvd1_AST = (AST) returnAST;
                        if (inputState.guessing == 0) {
                            realVariableDeclarations_AST = (AST) currentAST.root;
                            realVariableDeclarations_AST.addChild(rvd1_AST);
                        }
                        {
                            _loop140:
                            do {
                                if ((LA(1) == COMMA)) {
                                    match(COMMA);
                                    realVariableDeclaration();
                                    rvdn_AST = (AST) returnAST;
                                    if (inputState.guessing == 0) {
                                        realVariableDeclarations_AST = (AST) currentAST.root;
                                        realVariableDeclarations_AST.addChild(rvdn_AST);
                                    }
                                } else {
                                    break _loop140;
                                }

                            } while (true);
                        }
                        break;
                    }
                    case SEMICOLON: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(SEMICOLON);
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_27);
            } else {
                throw ex;
            }
        }
        returnAST = realVariableDeclarations_AST;
    }

    public final void stringVariableDeclarations() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST stringVariableDeclarations_AST = null;
        AST svd1_AST = null;
        AST svdn_AST = null;

        try {      // for error handling
            if (inputState.guessing == 0) {
                stringVariableDeclarations_AST = (AST) currentAST.root;
                stringVariableDeclarations_AST = (AST) astFactory.make((new ASTArray(1)).add(astFactory.create(VARIABLE_DECLARATION, "VARIABLE_DECLARATION")));
                currentAST.root = stringVariableDeclarations_AST;
                currentAST.child = stringVariableDeclarations_AST != null && stringVariableDeclarations_AST.getFirstChild() != null ? stringVariableDeclarations_AST.getFirstChild() : stringVariableDeclarations_AST;
                currentAST.advanceChildToEnd();
            }
            match(STRING_KYWD);
            {
                switch (LA(1)) {
                    case IDENT: {
                        stringVariableDeclaration();
                        svd1_AST = (AST) returnAST;
                        if (inputState.guessing == 0) {
                            stringVariableDeclarations_AST = (AST) currentAST.root;
                            stringVariableDeclarations_AST.addChild(svd1_AST);
                        }
                        {
                            _loop154:
                            do {
                                if ((LA(1) == COMMA)) {
                                    match(COMMA);
                                    stringVariableDeclaration();
                                    svdn_AST = (AST) returnAST;
                                    if (inputState.guessing == 0) {
                                        stringVariableDeclarations_AST = (AST) currentAST.root;
                                        stringVariableDeclarations_AST.addChild(svdn_AST);
                                    }
                                } else {
                                    break _loop154;
                                }

                            } while (true);
                        }
                        break;
                    }
                    case SEMICOLON: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(SEMICOLON);
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_27);
            } else {
                throw ex;
            }
        }
        returnAST = stringVariableDeclarations_AST;
    }

    public final void blobVariableDeclarations() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST blobVariableDeclarations_AST = null;
        AST pvd1_AST = null;
        AST pvdn_AST = null;

        try {      // for error handling
            if (inputState.guessing == 0) {
                blobVariableDeclarations_AST = (AST) currentAST.root;
                blobVariableDeclarations_AST = (AST) astFactory.make((new ASTArray(1)).add(astFactory.create(VARIABLE_DECLARATION, "VARIABLE_DECLARATION")));
                currentAST.root = blobVariableDeclarations_AST;
                currentAST.child = blobVariableDeclarations_AST != null && blobVariableDeclarations_AST.getFirstChild() != null ? blobVariableDeclarations_AST.getFirstChild() : blobVariableDeclarations_AST;
                currentAST.advanceChildToEnd();
            }
            match(BLOB_KYWD);
            {
                switch (LA(1)) {
                    case IDENT: {
                        blobVariableDeclaration();
                        pvd1_AST = (AST) returnAST;
                        if (inputState.guessing == 0) {
                            blobVariableDeclarations_AST = (AST) currentAST.root;
                            blobVariableDeclarations_AST.addChild(pvd1_AST);
                        }
                        {
                            _loop168:
                            do {
                                if ((LA(1) == COMMA)) {
                                    match(COMMA);
                                    blobVariableDeclaration();
                                    pvdn_AST = (AST) returnAST;
                                    if (inputState.guessing == 0) {
                                        blobVariableDeclarations_AST = (AST) currentAST.root;
                                        blobVariableDeclarations_AST.addChild(pvdn_AST);
                                    }
                                } else {
                                    break _loop168;
                                }

                            } while (true);
                        }
                        break;
                    }
                    case SEMICOLON: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(SEMICOLON);
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_27);
            } else {
                throw ex;
            }
        }
        returnAST = blobVariableDeclarations_AST;
    }

    public final void timeVariableDeclarations() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST timeVariableDeclarations_AST = null;
        AST tvd1_AST = null;
        AST tvdn_AST = null;

        try {      // for error handling
            if (inputState.guessing == 0) {
                timeVariableDeclarations_AST = (AST) currentAST.root;
                timeVariableDeclarations_AST = (AST) astFactory.make((new ASTArray(1)).add(astFactory.create(VARIABLE_DECLARATION, "VARIABLE_DECLARATION")));
                currentAST.root = timeVariableDeclarations_AST;
                currentAST.child = timeVariableDeclarations_AST != null && timeVariableDeclarations_AST.getFirstChild() != null ? timeVariableDeclarations_AST.getFirstChild() : timeVariableDeclarations_AST;
                currentAST.advanceChildToEnd();
            }
            match(TIME_KYWD);
            {
                switch (LA(1)) {
                    case IDENT: {
                        timeVariableDeclaration();
                        tvd1_AST = (AST) returnAST;
                        if (inputState.guessing == 0) {
                            timeVariableDeclarations_AST = (AST) currentAST.root;
                            timeVariableDeclarations_AST.addChild(tvd1_AST);
                        }
                        {
                            _loop180:
                            do {
                                if ((LA(1) == COMMA)) {
                                    match(COMMA);
                                    timeVariableDeclaration();
                                    tvdn_AST = (AST) returnAST;
                                    if (inputState.guessing == 0) {
                                        timeVariableDeclarations_AST = (AST) currentAST.root;
                                        timeVariableDeclarations_AST.addChild(tvdn_AST);
                                    }
                                } else {
                                    break _loop180;
                                }

                            } while (true);
                        }
                        break;
                    }
                    case SEMICOLON: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(SEMICOLON);
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_27);
            } else {
                throw ex;
            }
        }
        returnAST = timeVariableDeclarations_AST;
    }

    public final void booleanVariableDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST booleanVariableDeclaration_AST = null;

        try {      // for error handling
            if ((LA(1) == IDENT) && (_tokenSet_54.member(LA(2))) && (_tokenSet_55.member(LA(3)))) {
                booleanScalarDeclaration();
                astFactory.addASTChild(currentAST, returnAST);
                booleanVariableDeclaration_AST = (AST) currentAST.root;
            } 
            else if ((LA(1) == IDENT) && (LA(2) == LBRACKET || LA(2) == PERIOD) && (_tokenSet_56.member(LA(3)))) {
                booleanArrayDeclaration();
                astFactory.addASTChild(currentAST, returnAST);
                booleanVariableDeclaration_AST = (AST) currentAST.root;
            } else {
                throw new NoViableAltException(LT(1), getFilename());
            }

        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = booleanVariableDeclaration_AST;
    }

    public final void booleanScalarDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST booleanScalarDeclaration_AST = null;
        AST vn_AST = null;
        AST iv_AST = null;

        try {      // for error handling
            variableName();
            vn_AST = (AST) returnAST;
            {
                switch (LA(1)) {
                    case EQUALS: {
                        match(EQUALS);
                        booleanValue();
                        iv_AST = (AST) returnAST;
                        break;
                    }
                    case SEMICOLON:
                    case COMMA: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            if (inputState.guessing == 0) {
                booleanScalarDeclaration_AST = (AST) currentAST.root;

                context.addVariableName(vn_AST.getText(), PlexilDataType.BOOLEAN_TYPE);
                booleanScalarDeclaration_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(BOOLEAN_KYWD, "Boolean")).add(vn_AST));
                if (iv_AST != null) {
                    booleanScalarDeclaration_AST.addChild(iv_AST);
                }

                currentAST.root = booleanScalarDeclaration_AST;
                currentAST.child = booleanScalarDeclaration_AST != null && booleanScalarDeclaration_AST.getFirstChild() != null ? booleanScalarDeclaration_AST.getFirstChild() : booleanScalarDeclaration_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = booleanScalarDeclaration_AST;
    }

    public final void booleanArrayDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST booleanArrayDeclaration_AST = null;
        AST vn_AST = null;
        AST dim_AST = null;
        AST iv_AST = null;

        try {      // for error handling
            variableName();
            vn_AST = (AST) returnAST;
            match(LBRACKET);
            nonNegativeInteger();
            dim_AST = (AST) returnAST;
            match(RBRACKET);
            {
                switch (LA(1)) {
                    case EQUALS: {
                        match(EQUALS);
                        booleanArrayInitialValue();
                        iv_AST = (AST) returnAST;
                        break;
                    }
                    case SEMICOLON:
                    case COMMA: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            if (inputState.guessing == 0) {
                booleanArrayDeclaration_AST = (AST) currentAST.root;

                context.addArrayVariableName(vn_AST.getText(),
                        PlexilDataType.BOOLEAN_TYPE,
                        dim_AST.getText());
                booleanArrayDeclaration_AST = (AST) astFactory.make((new ASTArray(3)).add(astFactory.create(BOOLEAN_ARRAY, "BOOLEAN_ARRAY")).add(vn_AST).add(dim_AST));
                if (iv_AST != null) {
                    booleanArrayDeclaration_AST.addChild(iv_AST);
                }

                currentAST.root = booleanArrayDeclaration_AST;
                currentAST.child = booleanArrayDeclaration_AST != null && booleanArrayDeclaration_AST.getFirstChild() != null ? booleanArrayDeclaration_AST.getFirstChild() : booleanArrayDeclaration_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = booleanArrayDeclaration_AST;
    }

    public final void variableName() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST variableName_AST = null;

        try {      // for error handling
            ncName();
            astFactory.addASTChild(currentAST, returnAST);
            variableName_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_57);
            } else {
                throw ex;
            }
        }
        returnAST = variableName_AST;
    }

    public final void booleanValue() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST booleanValue_AST = null;

        try {      // for error handling
            {
                switch (LA(1)) {
                    case TRUE_KYWD: {
                        plexil.LiteralASTNode tmp157_AST = null;
                        tmp157_AST = (plexil.LiteralASTNode) astFactory.create(LT(1), "plexil.LiteralASTNode");
                        astFactory.addASTChild(currentAST, tmp157_AST);
                        match(TRUE_KYWD);
                        break;
                    }
                    case FALSE_KYWD: {
                        plexil.LiteralASTNode tmp158_AST = null;
                        tmp158_AST = (plexil.LiteralASTNode) astFactory.create(LT(1), "plexil.LiteralASTNode");
                        astFactory.addASTChild(currentAST, tmp158_AST);
                        match(FALSE_KYWD);
                        break;
                    }
                    default:
                        if (((LA(1) == INT)) && (LT(1).getText().equals("0") || LT(1).getText().equals("1"))) {
                            plexil.LiteralASTNode tmp159_AST = null;
                            tmp159_AST = (plexil.LiteralASTNode) astFactory.create(LT(1), "plexil.LiteralASTNode");
                            astFactory.addASTChild(currentAST, tmp159_AST);
                            match(INT);
                        } else {
                            throw new NoViableAltException(LT(1), getFilename());
                        }
                }
            }
            if (inputState.guessing == 0) {
                booleanValue_AST = (AST) currentAST.root;
                ((PlexilASTNode) booleanValue_AST).setDataType(PlexilDataType.BOOLEAN_TYPE);
            }
            booleanValue_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_58);
            } else {
                throw ex;
            }
        }
        returnAST = booleanValue_AST;
    }

    public final void booleanArrayInitialValue() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST booleanArrayInitialValue_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case TRUE_KYWD:
                case FALSE_KYWD:
                case INT: {
                    booleanValue();
                    astFactory.addASTChild(currentAST, returnAST);
                    booleanArrayInitialValue_AST = (AST) currentAST.root;
                    break;
                }
                case HASHPAREN: {
                    booleanArrayValue();
                    astFactory.addASTChild(currentAST, returnAST);
                    booleanArrayInitialValue_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = booleanArrayInitialValue_AST;
    }

    public final void booleanArrayValue() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST booleanArrayValue_AST = null;

        try {      // for error handling
            match(HASHPAREN);
            {
                _loop120:
                do {
                    if ((LA(1) == TRUE_KYWD || LA(1) == FALSE_KYWD || LA(1) == INT)) {
                        booleanValue();
                        astFactory.addASTChild(currentAST, returnAST);
                    } else {
                        break _loop120;
                    }

                } while (true);
            }
            match(RPAREN);
            if (inputState.guessing == 0) {
                booleanArrayValue_AST = (AST) currentAST.root;

                booleanArrayValue_AST =
                        (AST) astFactory.make((new ASTArray(2)).add((plexil.LiteralASTNode) astFactory.create(BOOLEAN_ARRAY_LITERAL, "BOOLEAN_ARRAY_LITERAL", "plexil.LiteralASTNode")).add(booleanArrayValue_AST));
                ((PlexilASTNode) booleanArrayValue_AST).setDataType(PlexilDataType.BOOLEAN_ARRAY_TYPE);

                currentAST.root = booleanArrayValue_AST;
                currentAST.child = booleanArrayValue_AST != null && booleanArrayValue_AST.getFirstChild() != null ? booleanArrayValue_AST.getFirstChild() : booleanArrayValue_AST;
                currentAST.advanceChildToEnd();
            }
            booleanArrayValue_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = booleanArrayValue_AST;
    }

    public final void integerVariableDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST integerVariableDeclaration_AST = null;

        try {      // for error handling
            if ((LA(1) == IDENT) && (_tokenSet_54.member(LA(2))) && (_tokenSet_59.member(LA(3)))) {
                integerScalarDeclaration();
                astFactory.addASTChild(currentAST, returnAST);
                integerVariableDeclaration_AST = (AST) currentAST.root;
            } 
            else if ((LA(1) == IDENT) && (LA(2) == LBRACKET || LA(2) == PERIOD) && (_tokenSet_56.member(LA(3)))) {
                integerArrayDeclaration();
                astFactory.addASTChild(currentAST, returnAST);
                integerVariableDeclaration_AST = (AST) currentAST.root;
            } else {
                throw new NoViableAltException(LT(1), getFilename());
            }

        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = integerVariableDeclaration_AST;
    }

    public final void integerScalarDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST integerScalarDeclaration_AST = null;
        AST vn_AST = null;
        AST iv_AST = null;

        try {      // for error handling
            variableName();
            vn_AST = (AST) returnAST;
            {
                switch (LA(1)) {
                    case EQUALS: {
                        match(EQUALS);
                        integerValue();
                        iv_AST = (AST) returnAST;
                        break;
                    }
                    case SEMICOLON:
                    case COMMA: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            if (inputState.guessing == 0) {
                integerScalarDeclaration_AST = (AST) currentAST.root;

                context.addVariableName(vn_AST.getText(), PlexilDataType.INTEGER_TYPE);
                integerScalarDeclaration_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(INTEGER_KYWD, "Integer")).add(vn_AST));
                if (iv_AST != null) {
                    integerScalarDeclaration_AST.addChild(iv_AST);
                }

                currentAST.root = integerScalarDeclaration_AST;
                currentAST.child = integerScalarDeclaration_AST != null && integerScalarDeclaration_AST.getFirstChild() != null ? integerScalarDeclaration_AST.getFirstChild() : integerScalarDeclaration_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = integerScalarDeclaration_AST;
    }

    public final void integerArrayDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST integerArrayDeclaration_AST = null;
        AST vn_AST = null;
        AST dim_AST = null;
        AST iv_AST = null;

        try {      // for error handling
            variableName();
            vn_AST = (AST) returnAST;
            match(LBRACKET);
            nonNegativeInteger();
            dim_AST = (AST) returnAST;
            match(RBRACKET);
            {
                switch (LA(1)) {
                    case EQUALS: {
                        match(EQUALS);
                        integerArrayInitialValue();
                        iv_AST = (AST) returnAST;
                        break;
                    }
                    case SEMICOLON:
                    case COMMA: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            if (inputState.guessing == 0) {
                integerArrayDeclaration_AST = (AST) currentAST.root;

                context.addArrayVariableName(vn_AST.getText(),
                        PlexilDataType.INTEGER_TYPE,
                        dim_AST.getText());
                integerArrayDeclaration_AST = (AST) astFactory.make((new ASTArray(3)).add(astFactory.create(INTEGER_ARRAY, "INTEGER_ARRAY")).add(vn_AST).add(dim_AST));
                if (iv_AST != null) {
                    integerArrayDeclaration_AST.addChild(iv_AST);
                }

                currentAST.root = integerArrayDeclaration_AST;
                currentAST.child = integerArrayDeclaration_AST != null && integerArrayDeclaration_AST.getFirstChild() != null ? integerArrayDeclaration_AST.getFirstChild() : integerArrayDeclaration_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = integerArrayDeclaration_AST;
    }

    public final void integerValue() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST integerValue_AST = null;

        try {      // for error handling
            plexil.LiteralASTNode tmp166_AST = null;
            tmp166_AST = (plexil.LiteralASTNode) astFactory.create(LT(1), "plexil.LiteralASTNode");
            astFactory.addASTChild(currentAST, tmp166_AST);
            match(INT);
            if (inputState.guessing == 0) {
                integerValue_AST = (AST) currentAST.root;

                ((PlexilASTNode) integerValue_AST).setDataType(PlexilDataType.INTEGER_TYPE);

            }
            integerValue_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_60);
            } else {
                throw ex;
            }
        }
        returnAST = integerValue_AST;
    }

    public final void integerArrayInitialValue() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST integerArrayInitialValue_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case INT: {
                    integerValue();
                    astFactory.addASTChild(currentAST, returnAST);
                    integerArrayInitialValue_AST = (AST) currentAST.root;
                    break;
                }
                case HASHPAREN: {
                    integerArrayValue();
                    astFactory.addASTChild(currentAST, returnAST);
                    integerArrayInitialValue_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = integerArrayInitialValue_AST;
    }

    public final void integerArrayValue() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST integerArrayValue_AST = null;

        try {      // for error handling
            match(HASHPAREN);
            {
                _loop135:
                do {
                    if ((LA(1) == INT)) {
                        integerValue();
                        astFactory.addASTChild(currentAST, returnAST);
                    } else {
                        break _loop135;
                    }

                } while (true);
            }
            match(RPAREN);
            if (inputState.guessing == 0) {
                integerArrayValue_AST = (AST) currentAST.root;

                integerArrayValue_AST =
                        (AST) astFactory.make((new ASTArray(2)).add((plexil.LiteralASTNode) astFactory.create(INTEGER_ARRAY_LITERAL, "INTEGER_ARRAY_LITERAL", "plexil.LiteralASTNode")).add(integerArrayValue_AST));
                ((PlexilASTNode) integerArrayValue_AST).setDataType(PlexilDataType.INTEGER_ARRAY_TYPE);

                currentAST.root = integerArrayValue_AST;
                currentAST.child = integerArrayValue_AST != null && integerArrayValue_AST.getFirstChild() != null ? integerArrayValue_AST.getFirstChild() : integerArrayValue_AST;
                currentAST.advanceChildToEnd();
            }
            integerArrayValue_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = integerArrayValue_AST;
    }

    public final void realVariableDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST realVariableDeclaration_AST = null;

        try {      // for error handling
            if ((LA(1) == IDENT) && (_tokenSet_54.member(LA(2))) && (_tokenSet_61.member(LA(3)))) {
                realScalarDeclaration();
                astFactory.addASTChild(currentAST, returnAST);
                realVariableDeclaration_AST = (AST) currentAST.root;
            } 
            else if ((LA(1) == IDENT) && (LA(2) == LBRACKET || LA(2) == PERIOD) && (_tokenSet_56.member(LA(3)))) {
                realArrayDeclaration();
                astFactory.addASTChild(currentAST, returnAST);
                realVariableDeclaration_AST = (AST) currentAST.root;
            } else {
                throw new NoViableAltException(LT(1), getFilename());
            }

        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = realVariableDeclaration_AST;
    }

    public final void realScalarDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST realScalarDeclaration_AST = null;
        AST vn_AST = null;
        AST iv_AST = null;

        try {      // for error handling
            variableName();
            vn_AST = (AST) returnAST;
            {
                switch (LA(1)) {
                    case EQUALS: {
                        match(EQUALS);
                        realValue();
                        iv_AST = (AST) returnAST;
                        break;
                    }
                    case SEMICOLON:
                    case COMMA: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            if (inputState.guessing == 0) {
                realScalarDeclaration_AST = (AST) currentAST.root;

                context.addVariableName(vn_AST.getText(), PlexilDataType.REAL_TYPE);
                realScalarDeclaration_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(REAL_KYWD, "Real")).add(vn_AST));
                if (iv_AST != null) {
                    realScalarDeclaration_AST.addChild(iv_AST);
                }

                currentAST.root = realScalarDeclaration_AST;
                currentAST.child = realScalarDeclaration_AST != null && realScalarDeclaration_AST.getFirstChild() != null ? realScalarDeclaration_AST.getFirstChild() : realScalarDeclaration_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = realScalarDeclaration_AST;
    }

    public final void realArrayDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST realArrayDeclaration_AST = null;
        AST vn_AST = null;
        AST dim_AST = null;
        AST iv_AST = null;

        try {      // for error handling
            variableName();
            vn_AST = (AST) returnAST;
            match(LBRACKET);
            nonNegativeInteger();
            dim_AST = (AST) returnAST;
            match(RBRACKET);
            {
                switch (LA(1)) {
                    case EQUALS: {
                        match(EQUALS);
                        realArrayInitialValue();
                        iv_AST = (AST) returnAST;
                        break;
                    }
                    case SEMICOLON:
                    case COMMA: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            if (inputState.guessing == 0) {
                realArrayDeclaration_AST = (AST) currentAST.root;

                context.addArrayVariableName(vn_AST.getText(),
                        PlexilDataType.REAL_TYPE,
                        dim_AST.getText());
                realArrayDeclaration_AST = (AST) astFactory.make((new ASTArray(3)).add(astFactory.create(REAL_ARRAY, "REAL_ARRAY")).add(vn_AST).add(dim_AST));
                if (iv_AST != null) {
                    realArrayDeclaration_AST.addChild(iv_AST);
                }

                currentAST.root = realArrayDeclaration_AST;
                currentAST.child = realArrayDeclaration_AST != null && realArrayDeclaration_AST.getFirstChild() != null ? realArrayDeclaration_AST.getFirstChild() : realArrayDeclaration_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = realArrayDeclaration_AST;
    }

    public final void realValue() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST realValue_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case DOUBLE: {
                    plexil.LiteralASTNode tmp173_AST = null;
                    tmp173_AST = (plexil.LiteralASTNode) astFactory.create(LT(1), "plexil.LiteralASTNode");
                    astFactory.addASTChild(currentAST, tmp173_AST);
                    match(DOUBLE);
                    realValue_AST = (AST) currentAST.root;
                    break;
                }
                case INT: {
                    plexil.LiteralASTNode tmp174_AST = null;
                    tmp174_AST = (plexil.LiteralASTNode) astFactory.create(LT(1), "plexil.LiteralASTNode");
                    astFactory.addASTChild(currentAST, tmp174_AST);
                    match(INT);
                    if (inputState.guessing == 0) {
                        realValue_AST = (AST) currentAST.root;

                        ((PlexilASTNode) realValue_AST).setDataType(PlexilDataType.REAL_TYPE);

                    }
                    realValue_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_62);
            } else {
                throw ex;
            }
        }
        returnAST = realValue_AST;
    }

    public final void realArrayInitialValue() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST realArrayInitialValue_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case INT:
                case DOUBLE: {
                    realValue();
                    astFactory.addASTChild(currentAST, returnAST);
                    realArrayInitialValue_AST = (AST) currentAST.root;
                    break;
                }
                case HASHPAREN: {
                    realArrayValue();
                    astFactory.addASTChild(currentAST, returnAST);
                    realArrayInitialValue_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = realArrayInitialValue_AST;
    }

    public final void realArrayValue() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST realArrayValue_AST = null;

        try {      // for error handling
            match(HASHPAREN);
            {
                _loop149:
                do {
                    if ((LA(1) == INT || LA(1) == DOUBLE)) {
                        realValue();
                        astFactory.addASTChild(currentAST, returnAST);
                    } else {
                        break _loop149;
                    }

                } while (true);
            }
            match(RPAREN);
            if (inputState.guessing == 0) {
                realArrayValue_AST = (AST) currentAST.root;

                realArrayValue_AST =
                        (AST) astFactory.make((new ASTArray(2)).add((plexil.LiteralASTNode) astFactory.create(REAL_ARRAY_LITERAL, "REAL_ARRAY_LITERAL", "plexil.LiteralASTNode")).add(realArrayValue_AST));
                ((PlexilASTNode) realArrayValue_AST).setDataType(PlexilDataType.REAL_ARRAY_TYPE);

                currentAST.root = realArrayValue_AST;
                currentAST.child = realArrayValue_AST != null && realArrayValue_AST.getFirstChild() != null ? realArrayValue_AST.getFirstChild() : realArrayValue_AST;
                currentAST.advanceChildToEnd();
            }
            realArrayValue_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = realArrayValue_AST;
    }

    public final void stringVariableDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST stringVariableDeclaration_AST = null;

        try {      // for error handling
            if ((LA(1) == IDENT) && (_tokenSet_54.member(LA(2))) && (_tokenSet_63.member(LA(3)))) {
                stringScalarDeclaration();
                astFactory.addASTChild(currentAST, returnAST);
                stringVariableDeclaration_AST = (AST) currentAST.root;
            } 
            else if ((LA(1) == IDENT) && (LA(2) == LBRACKET || LA(2) == PERIOD) && (_tokenSet_56.member(LA(3)))) {
                stringArrayDeclaration();
                astFactory.addASTChild(currentAST, returnAST);
                stringVariableDeclaration_AST = (AST) currentAST.root;
            } else {
                throw new NoViableAltException(LT(1), getFilename());
            }

        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = stringVariableDeclaration_AST;
    }

    public final void stringScalarDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST stringScalarDeclaration_AST = null;
        AST vn_AST = null;
        AST iv_AST = null;

        try {      // for error handling
            variableName();
            vn_AST = (AST) returnAST;
            {
                switch (LA(1)) {
                    case EQUALS: {
                        match(EQUALS);
                        stringValue();
                        iv_AST = (AST) returnAST;
                        break;
                    }
                    case SEMICOLON:
                    case COMMA: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            if (inputState.guessing == 0) {
                stringScalarDeclaration_AST = (AST) currentAST.root;

                context.addVariableName(vn_AST.getText(), PlexilDataType.STRING_TYPE);
                stringScalarDeclaration_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(STRING_KYWD, "String")).add(vn_AST));
                if (iv_AST != null) {
                    stringScalarDeclaration_AST.addChild(iv_AST);
                }

                currentAST.root = stringScalarDeclaration_AST;
                currentAST.child = stringScalarDeclaration_AST != null && stringScalarDeclaration_AST.getFirstChild() != null ? stringScalarDeclaration_AST.getFirstChild() : stringScalarDeclaration_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = stringScalarDeclaration_AST;
    }

    public final void stringArrayDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST stringArrayDeclaration_AST = null;
        AST vn_AST = null;
        AST dim_AST = null;
        AST iv_AST = null;

        try {      // for error handling
            variableName();
            vn_AST = (AST) returnAST;
            match(LBRACKET);
            nonNegativeInteger();
            dim_AST = (AST) returnAST;
            match(RBRACKET);
            {
                switch (LA(1)) {
                    case EQUALS: {
                        match(EQUALS);
                        stringArrayInitialValue();
                        iv_AST = (AST) returnAST;
                        break;
                    }
                    case SEMICOLON:
                    case COMMA: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            if (inputState.guessing == 0) {
                stringArrayDeclaration_AST = (AST) currentAST.root;

                context.addArrayVariableName(vn_AST.getText(),
                        PlexilDataType.STRING_TYPE,
                        dim_AST.getText());
                stringArrayDeclaration_AST = (AST) astFactory.make((new ASTArray(3)).add(astFactory.create(STRING_ARRAY, "STRING_ARRAY")).add(vn_AST).add(dim_AST));
                if (iv_AST != null) {
                    stringArrayDeclaration_AST.addChild(iv_AST);
                }

                currentAST.root = stringArrayDeclaration_AST;
                currentAST.child = stringArrayDeclaration_AST != null && stringArrayDeclaration_AST.getFirstChild() != null ? stringArrayDeclaration_AST.getFirstChild() : stringArrayDeclaration_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = stringArrayDeclaration_AST;
    }

    public final void stringValue() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST stringValue_AST = null;

        try {      // for error handling
            plexil.StringLiteralASTNode tmp181_AST = null;
            tmp181_AST = (plexil.StringLiteralASTNode) astFactory.create(LT(1), "plexil.StringLiteralASTNode");
            astFactory.addASTChild(currentAST, tmp181_AST);
            match(STRING);
            if (inputState.guessing == 0) {
                stringValue_AST = (AST) currentAST.root;

                ((PlexilASTNode) stringValue_AST).setDataType(PlexilDataType.STRING_TYPE);

            }
            stringValue_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_64);
            } else {
                throw ex;
            }
        }
        returnAST = stringValue_AST;
    }

    public final void stringArrayInitialValue() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST stringArrayInitialValue_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case STRING: {
                    stringValue();
                    astFactory.addASTChild(currentAST, returnAST);
                    stringArrayInitialValue_AST = (AST) currentAST.root;
                    break;
                }
                case HASHPAREN: {
                    stringArrayValue();
                    astFactory.addASTChild(currentAST, returnAST);
                    stringArrayInitialValue_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = stringArrayInitialValue_AST;
    }

    public final void stringArrayValue() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST stringArrayValue_AST = null;

        try {      // for error handling
            match(HASHPAREN);
            {
                _loop163:
                do {
                    if ((LA(1) == STRING)) {
                        stringValue();
                        astFactory.addASTChild(currentAST, returnAST);
                    } else {
                        break _loop163;
                    }

                } while (true);
            }
            match(RPAREN);
            if (inputState.guessing == 0) {
                stringArrayValue_AST = (AST) currentAST.root;

                stringArrayValue_AST =
                        (AST) astFactory.make((new ASTArray(2)).add((plexil.LiteralASTNode) astFactory.create(STRING_ARRAY_LITERAL, "STRING_ARRAY_LITERAL", "plexil.LiteralASTNode")).add(stringArrayValue_AST));
                ((PlexilASTNode) stringArrayValue_AST).setDataType(PlexilDataType.STRING_ARRAY_TYPE);

                currentAST.root = stringArrayValue_AST;
                currentAST.child = stringArrayValue_AST != null && stringArrayValue_AST.getFirstChild() != null ? stringArrayValue_AST.getFirstChild() : stringArrayValue_AST;
                currentAST.advanceChildToEnd();
            }
            stringArrayValue_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = stringArrayValue_AST;
    }

    public final void blobVariableDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST blobVariableDeclaration_AST = null;

        try {      // for error handling
            if ((LA(1) == IDENT) && (_tokenSet_54.member(LA(2))) && (_tokenSet_65.member(LA(3)))) {
                blobScalarDeclaration();
                astFactory.addASTChild(currentAST, returnAST);
                blobVariableDeclaration_AST = (AST) currentAST.root;
            } 
            else if ((LA(1) == IDENT) && (LA(2) == LBRACKET || LA(2) == PERIOD) && (_tokenSet_56.member(LA(3)))) {
                blobArrayDeclaration();
                astFactory.addASTChild(currentAST, returnAST);
                blobVariableDeclaration_AST = (AST) currentAST.root;
            } else {
                throw new NoViableAltException(LT(1), getFilename());
            }

        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = blobVariableDeclaration_AST;
    }

    public final void blobScalarDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST blobScalarDeclaration_AST = null;
        AST vn_AST = null;
        AST iv_AST = null;

        try {      // for error handling
            variableName();
            vn_AST = (AST) returnAST;
            {
                switch (LA(1)) {
                    case EQUALS: {
                        match(EQUALS);
                        pointsTo();
                        iv_AST = (AST) returnAST;
                        break;
                    }
                    case SEMICOLON:
                    case COMMA: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            if (inputState.guessing == 0) {
                blobScalarDeclaration_AST = (AST) currentAST.root;

                context.addVariableName(vn_AST.getText(), PlexilDataType.BLOB_TYPE);
                blobScalarDeclaration_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(BLOB_KYWD, "Blob")).add(vn_AST));
                if (iv_AST != null) {
                    blobScalarDeclaration_AST.addChild(iv_AST);
                }

                currentAST.root = blobScalarDeclaration_AST;
                currentAST.child = blobScalarDeclaration_AST != null && blobScalarDeclaration_AST.getFirstChild() != null ? blobScalarDeclaration_AST.getFirstChild() : blobScalarDeclaration_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = blobScalarDeclaration_AST;
    }

    public final void blobArrayDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST blobArrayDeclaration_AST = null;
        AST vn_AST = null;
        AST dim_AST = null;
        AST iv_AST = null;

        try {      // for error handling
            variableName();
            vn_AST = (AST) returnAST;
            match(LBRACKET);
            nonNegativeInteger();
            dim_AST = (AST) returnAST;
            match(RBRACKET);
            {
                switch (LA(1)) {
                    case EQUALS: {
                        match(EQUALS);
                        pointsTo();
                        iv_AST = (AST) returnAST;
                        break;
                    }
                    case SEMICOLON:
                    case COMMA: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            if (inputState.guessing == 0) {
                blobArrayDeclaration_AST = (AST) currentAST.root;

                context.addArrayVariableName(vn_AST.getText(),
                        PlexilDataType.BLOB_TYPE,
                        dim_AST.getText());
                blobArrayDeclaration_AST = (AST) astFactory.make((new ASTArray(3)).add(astFactory.create(BLOB_ARRAY, "BLOB_ARRAY")).add(vn_AST).add(dim_AST));
                if (iv_AST != null) {
                    blobArrayDeclaration_AST.addChild(iv_AST);
                }

                currentAST.root = blobArrayDeclaration_AST;
                currentAST.child = blobArrayDeclaration_AST != null && blobArrayDeclaration_AST.getFirstChild() != null ? blobArrayDeclaration_AST.getFirstChild() : blobArrayDeclaration_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = blobArrayDeclaration_AST;
    }

    public final void pointsTo() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST pointsTo_AST = null;
        AST esn_AST = null;
        AST iv_AST = null;

        try {      // for error handling
            ncName();
            esn_AST = (AST) returnAST;
            if (inputState.guessing == 0) {
                pointsTo_AST = (AST) currentAST.root;
                pointsTo_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(POINTS_TO, "POINTS_TO")).add(esn_AST));
                currentAST.root = pointsTo_AST;
                currentAST.child = pointsTo_AST != null && pointsTo_AST.getFirstChild() != null ? pointsTo_AST.getFirstChild() : pointsTo_AST;
                currentAST.advanceChildToEnd();
            }
            {
                switch (LA(1)) {
                    case LPAREN: {
                        match(LPAREN);
                        initialValue();
                        iv_AST = (AST) returnAST;
                        match(RPAREN);
                        if (inputState.guessing == 0) {
                            pointsTo_AST = (AST) currentAST.root;
                            pointsTo_AST.addChild(iv_AST);
                        }
                        break;
                    }
                    case SEMICOLON:
                    case COMMA: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = pointsTo_AST;
    }

    public final void initialValue() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST initialValue_AST = null;

        try {      // for error handling
            plexil.LiteralASTNode tmp190_AST = null;
            tmp190_AST = (plexil.LiteralASTNode) astFactory.create(LT(1), "plexil.LiteralASTNode");
            astFactory.addASTChild(currentAST, tmp190_AST);
            match(INT);
            initialValue_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_66);
            } else {
                throw ex;
            }
        }
        returnAST = initialValue_AST;
    }

    public final void timeVariableDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST timeVariableDeclaration_AST = null;

        try {      // for error handling
            if ((LA(1) == IDENT) && (_tokenSet_54.member(LA(2))) && (_tokenSet_67.member(LA(3)))) {
                timeScalarDeclaration();
                astFactory.addASTChild(currentAST, returnAST);
                timeVariableDeclaration_AST = (AST) currentAST.root;
            } 
            else if ((LA(1) == IDENT) && (LA(2) == LBRACKET || LA(2) == PERIOD) && (_tokenSet_56.member(LA(3)))) {
                timeArrayDeclaration();
                astFactory.addASTChild(currentAST, returnAST);
                timeVariableDeclaration_AST = (AST) currentAST.root;
            } else {
                throw new NoViableAltException(LT(1), getFilename());
            }

        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = timeVariableDeclaration_AST;
    }

    public final void timeScalarDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST timeScalarDeclaration_AST = null;
        AST vn_AST = null;
        AST iv_AST = null;

        try {      // for error handling
            variableName();
            vn_AST = (AST) returnAST;
            {
                switch (LA(1)) {
                    case EQUALS: {
                        match(EQUALS);
                        timeValue();
                        iv_AST = (AST) returnAST;
                        break;
                    }
                    case SEMICOLON:
                    case COMMA: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            if (inputState.guessing == 0) {
                timeScalarDeclaration_AST = (AST) currentAST.root;

                context.addVariableName(vn_AST.getText(), PlexilDataType.TIME_TYPE);
                timeScalarDeclaration_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(TIME_KYWD, "Time")).add(vn_AST));
                if (iv_AST != null) {
                    timeScalarDeclaration_AST.addChild(iv_AST);
                }

                currentAST.root = timeScalarDeclaration_AST;
                currentAST.child = timeScalarDeclaration_AST != null && timeScalarDeclaration_AST.getFirstChild() != null ? timeScalarDeclaration_AST.getFirstChild() : timeScalarDeclaration_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = timeScalarDeclaration_AST;
    }

    public final void timeArrayDeclaration() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST timeArrayDeclaration_AST = null;
        AST vn_AST = null;
        AST dim_AST = null;
        AST iv_AST = null;

        try {      // for error handling
            variableName();
            vn_AST = (AST) returnAST;
            match(LBRACKET);
            nonNegativeInteger();
            dim_AST = (AST) returnAST;
            match(RBRACKET);
            {
                switch (LA(1)) {
                    case EQUALS: {
                        match(EQUALS);
                        timeArrayInitialValue();
                        iv_AST = (AST) returnAST;
                        break;
                    }
                    case SEMICOLON:
                    case COMMA: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            if (inputState.guessing == 0) {
                timeArrayDeclaration_AST = (AST) currentAST.root;

                context.addArrayVariableName(vn_AST.getText(),
                        PlexilDataType.TIME_TYPE,
                        dim_AST.getText());
                timeArrayDeclaration_AST = (AST) astFactory.make((new ASTArray(3)).add(astFactory.create(TIME_ARRAY, "TIME_ARRAY")).add(vn_AST).add(dim_AST));
                if (iv_AST != null) {
                    timeArrayDeclaration_AST.addChild(iv_AST);
                }

                currentAST.root = timeArrayDeclaration_AST;
                currentAST.child = timeArrayDeclaration_AST != null && timeArrayDeclaration_AST.getFirstChild() != null ? timeArrayDeclaration_AST.getFirstChild() : timeArrayDeclaration_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = timeArrayDeclaration_AST;
    }

    public final void timeValue() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST timeValue_AST = null;
        Token i = null;
        plexil.LiteralASTNode i_AST = null;
        AST units_AST = null;
        AST subunits_AST = null;

        try {      // for error handling
            {
                switch (LA(1)) {
                    case INT: {
                        i = LT(1);
                        i_AST = (plexil.LiteralASTNode) astFactory.create(i, "plexil.LiteralASTNode");
                        match(INT);
                        if (inputState.guessing == 0) {
                            timeValue_AST = (AST) currentAST.root;
                            timeValue_AST = (AST) astFactory.make((new ASTArray(2)).add((plexil.TimeLiteralASTNode) astFactory.create(TIME_VALUE, "TimeValue", "plexil.TimeLiteralASTNode")).add(i_AST));
                            currentAST.root = timeValue_AST;
                            currentAST.child = timeValue_AST != null && timeValue_AST.getFirstChild() != null ? timeValue_AST.getFirstChild() : timeValue_AST;
                            currentAST.advanceChildToEnd();
                        }
                        break;
                    }
                    case LBRACKET: {
                        match(LBRACKET);
                        integerValue();
                        units_AST = (AST) returnAST;
                        match(COMMA);
                        integerValue();
                        subunits_AST = (AST) returnAST;
                        match(RBRACKET);
                        if (inputState.guessing == 0) {
                            timeValue_AST = (AST) currentAST.root;
                            timeValue_AST = (AST) astFactory.make((new ASTArray(3)).add((plexil.TimeLiteralASTNode) astFactory.create(TIME_VALUE, "TimeValue", "plexil.TimeLiteralASTNode")).add(units_AST).add(subunits_AST));
                            currentAST.root = timeValue_AST;
                            currentAST.child = timeValue_AST != null && timeValue_AST.getFirstChild() != null ? timeValue_AST.getFirstChild() : timeValue_AST;
                            currentAST.advanceChildToEnd();
                        }
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            if (inputState.guessing == 0) {
                timeValue_AST = (AST) currentAST.root;

                ((PlexilASTNode) timeValue_AST).setDataType(PlexilDataType.TIME_TYPE);

            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_68);
            } else {
                throw ex;
            }
        }
        returnAST = timeValue_AST;
    }

    public final void timeArrayInitialValue() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST timeArrayInitialValue_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case INT:
                case LBRACKET: {
                    timeValue();
                    astFactory.addASTChild(currentAST, returnAST);
                    timeArrayInitialValue_AST = (AST) currentAST.root;
                    break;
                }
                case HASHPAREN: {
                    timeArrayValue();
                    astFactory.addASTChild(currentAST, returnAST);
                    timeArrayInitialValue_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = timeArrayInitialValue_AST;
    }

    public final void timeArrayValue() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST timeArrayValue_AST = null;

        try {      // for error handling
            match(HASHPAREN);
            {
                _loop189:
                do {
                    if ((LA(1) == INT || LA(1) == LBRACKET)) {
                        timeValue();
                        astFactory.addASTChild(currentAST, returnAST);
                    } else {
                        break _loop189;
                    }

                } while (true);
            }
            match(RPAREN);
            if (inputState.guessing == 0) {
                timeArrayValue_AST = (AST) currentAST.root;

                timeArrayValue_AST =
                        (AST) astFactory.make((new ASTArray(2)).add((plexil.LiteralASTNode) astFactory.create(TIME_ARRAY_LITERAL, "TIME_ARRAY_LITERAL", "plexil.LiteralASTNode")).add(timeArrayValue_AST));
                ((PlexilASTNode) timeArrayValue_AST).setDataType(PlexilDataType.TIME_ARRAY_TYPE);

                currentAST.root = timeArrayValue_AST;
                currentAST.child = timeArrayValue_AST != null && timeArrayValue_AST.getFirstChild() != null ? timeArrayValue_AST.getFirstChild() : timeArrayValue_AST;
                currentAST.advanceChildToEnd();
            }
            timeArrayValue_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = timeArrayValue_AST;
    }

    public final void booleanArrayReference() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST booleanArrayReference_AST = null;
        AST ar_AST = null;

        try {      // for error handling
            if (!(context.isBooleanArrayVariableName(LT(1).getText()))) {
                throw new SemanticException(" context.isBooleanArrayVariableName(LT(1).getText()) ");
            }
            arrayReference();
            ar_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (inputState.guessing == 0) {

                if (((ArrayRefASTNode) ar_AST).getDataType() != PlexilDataType.BOOLEAN_TYPE) {
                    throw createSemanticException("Array variable \"" + ((ArrayRefASTNode) ar_AST).getVariable().getName() + "\" is not a Boolean array",
                            ar_AST);
                }

            }
            booleanArrayReference_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_48);
            } else {
                throw ex;
            }
        }
        returnAST = booleanArrayReference_AST;
    }

    public final void arrayReference() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST arrayReference_AST = null;
        AST var_AST = null;
        AST idx_AST = null;

        try {      // for error handling
            variable();
            var_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (!(context.isArrayVariableName(var_AST.getText()))) {
                throw new SemanticException(" context.isArrayVariableName(var_AST.getText()) ");
            }
            match(LBRACKET);
            numericExpression();
            idx_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            match(RBRACKET);
            if (inputState.guessing == 0) {
                arrayReference_AST = (AST) currentAST.root;

                arrayReference_AST = (AST) astFactory.make((new ASTArray(2)).add((plexil.ArrayRefASTNode) astFactory.create(ARRAY_REF, "ARRAY_REF", "plexil.ArrayRefASTNode")).add(arrayReference_AST));

                currentAST.root = arrayReference_AST;
                currentAST.child = arrayReference_AST != null && arrayReference_AST.getFirstChild() != null ? arrayReference_AST.getFirstChild() : arrayReference_AST;
                currentAST.advanceChildToEnd();
            }
            arrayReference_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_69);
            } else {
                throw ex;
            }
        }
        returnAST = arrayReference_AST;
    }

    public final void integerArrayReference() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST integerArrayReference_AST = null;
        AST ar_AST = null;

        try {      // for error handling
            if (!(context.isIntegerArrayVariableName(LT(1).getText()))) {
                throw new SemanticException(" context.isIntegerArrayVariableName(LT(1).getText()) ");
            }
            arrayReference();
            ar_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (inputState.guessing == 0) {

                if (((ArrayRefASTNode) ar_AST).getDataType() != PlexilDataType.INTEGER_TYPE) {
                    throw createSemanticException("Array variable \"" + ((ArrayRefASTNode) ar_AST).getVariable().getName() + "\" is not an Integer array",
                            ar_AST);
                }

            }
            integerArrayReference_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_52);
            } else {
                throw ex;
            }
        }
        returnAST = integerArrayReference_AST;
    }

    public final void blobArrayReference() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST blobArrayReference_AST = null;
        AST ar_AST = null;

        try {      // for error handling
            if (!(context.isBlobArrayVariableName(LT(1).getText()))) {
                throw new SemanticException(" context.isBlobArrayVariableName(LT(1).getText()) ");
            }
            arrayReference();
            ar_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (inputState.guessing == 0) {

                if (((ArrayRefASTNode) ar_AST).getDataType() != PlexilDataType.BLOB_TYPE) {
                    throw createSemanticException("Array variable \"" + ((ArrayRefASTNode) ar_AST).getVariable().getName() + "\" is not a BLOB array",
                            ar_AST);
                }

            }
            blobArrayReference_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_0);
            } else {
                throw ex;
            }
        }
        returnAST = blobArrayReference_AST;
    }

    public final void realArrayReference() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST realArrayReference_AST = null;
        AST ar_AST = null;

        try {      // for error handling
            if (!(context.isRealArrayVariableName(LT(1).getText()))) {
                throw new SemanticException(" context.isRealArrayVariableName(LT(1).getText()) ");
            }
            arrayReference();
            ar_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (inputState.guessing == 0) {

                if (((ArrayRefASTNode) ar_AST).getDataType() != PlexilDataType.REAL_TYPE) {
                    throw createSemanticException("Array variable \"" + ((ArrayRefASTNode) ar_AST).getVariable().getName() + "\" is not a Real array",
                            ar_AST);
                }

            }
            realArrayReference_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_52);
            } else {
                throw ex;
            }
        }
        returnAST = realArrayReference_AST;
    }

    public final void stringArrayReference() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST stringArrayReference_AST = null;
        AST ar_AST = null;

        try {      // for error handling
            if (!(context.isStringArrayVariableName(LT(1).getText()))) {
                throw new SemanticException(" context.isStringArrayVariableName(LT(1).getText()) ");
            }
            arrayReference();
            ar_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (inputState.guessing == 0) {

                if (((ArrayRefASTNode) ar_AST).getDataType() != PlexilDataType.STRING_TYPE) {
                    throw createSemanticException("Array variable \"" + ((ArrayRefASTNode) ar_AST).getVariable().getName() + "\" is not a String array",
                            ar_AST);
                }

            }
            stringArrayReference_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_53);
            } else {
                throw ex;
            }
        }
        returnAST = stringArrayReference_AST;
    }

    public final void timeArrayReference() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST timeArrayReference_AST = null;
        AST ar_AST = null;

        try {      // for error handling
            if (!(context.isTimeArrayVariableName(LT(1).getText()))) {
                throw new SemanticException(" context.isTimeArrayVariableName(LT(1).getText()) ");
            }
            arrayReference();
            ar_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (inputState.guessing == 0) {

                if (((ArrayRefASTNode) ar_AST).getDataType() != PlexilDataType.TIME_TYPE) {
                    throw createSemanticException("Array variable \"" + ((ArrayRefASTNode) ar_AST).getVariable().getName() + "\" is not a Time array",
                            ar_AST);
                }

            }
            timeArrayReference_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_48);
            } else {
                throw ex;
            }
        }
        returnAST = timeArrayReference_AST;
    }

    public final void argumentList() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST argumentList_AST = null;

        try {      // for error handling
            argument();
            astFactory.addASTChild(currentAST, returnAST);
            {
                _loop210:
                do {
                    if ((LA(1) == COMMA)) {
                        match(COMMA);
                        argument();
                        astFactory.addASTChild(currentAST, returnAST);
                    } else {
                        break _loop210;
                    }

                } while (true);
            }
            if (inputState.guessing == 0) {
                argumentList_AST = (AST) currentAST.root;

                argumentList_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(ARGUMENT_LIST, "ARGUMENT_LIST")).add(argumentList_AST));

                currentAST.root = argumentList_AST;
                currentAST.child = argumentList_AST != null && argumentList_AST.getFirstChild() != null ? argumentList_AST.getFirstChild() : argumentList_AST;
                currentAST.advanceChildToEnd();
            }
            argumentList_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_66);
            } else {
                throw ex;
            }
        }
        returnAST = argumentList_AST;
    }

    public final void argument() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST argument_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case DOUBLE: {
                    plexil.LiteralASTNode tmp203_AST = null;
                    tmp203_AST = (plexil.LiteralASTNode) astFactory.create(LT(1), "plexil.LiteralASTNode");
                    astFactory.addASTChild(currentAST, tmp203_AST);
                    match(DOUBLE);
                    argument_AST = (AST) currentAST.root;
                    break;
                }
                case STRING: {
                    plexil.StringLiteralASTNode tmp204_AST = null;
                    tmp204_AST = (plexil.StringLiteralASTNode) astFactory.create(LT(1), "plexil.StringLiteralASTNode");
                    astFactory.addASTChild(currentAST, tmp204_AST);
                    match(STRING);
                    argument_AST = (AST) currentAST.root;
                    break;
                }
                default:
                    if ((LA(1) == INT) && (LA(2) == COMMA || LA(2) == RPAREN) && (_tokenSet_70.member(LA(3)))) {
                        plexil.LiteralASTNode tmp205_AST = null;
                        tmp205_AST = (plexil.LiteralASTNode) astFactory.create(LT(1), "plexil.LiteralASTNode");
                        astFactory.addASTChild(currentAST, tmp205_AST);
                        match(INT);
                        argument_AST = (AST) currentAST.root;
                    } else if ((LA(1) == TRUE_KYWD || LA(1) == FALSE_KYWD || LA(1) == INT) && (LA(2) == COMMA || LA(2) == RPAREN) && (_tokenSet_70.member(LA(3)))) {
                        booleanValue();
                        astFactory.addASTChild(currentAST, returnAST);
                        argument_AST = (AST) currentAST.root;
                    } else if ((LA(1) == IDENT) && (_tokenSet_71.member(LA(2))) && (_tokenSet_72.member(LA(3)))) {
                        variable();
                        astFactory.addASTChild(currentAST, returnAST);
                        argument_AST = (AST) currentAST.root;
                    } 
                    else if ((LA(1) == IDENT) && (LA(2) == LBRACKET || LA(2) == PERIOD) && (_tokenSet_30.member(LA(3)))) {
                        arrayReference();
                        astFactory.addASTChild(currentAST, returnAST);
                        argument_AST = (AST) currentAST.root;
                    } else {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_16);
            } else {
                throw ex;
            }
        }
        returnAST = argument_AST;
    }

    public final void arrayExpression() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST arrayExpression_AST = null;

        try {      // for error handling
            lookup();
            astFactory.addASTChild(currentAST, returnAST);
            arrayExpression_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_15);
            } else {
                throw ex;
            }
        }
        returnAST = arrayExpression_AST;
    }

    public final void timeExpression() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST timeExpression_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case INT:
                case LBRACKET: {
                    timeValue();
                    astFactory.addASTChild(currentAST, returnAST);
                    timeExpression_AST = (AST) currentAST.root;
                    break;
                }
                case LOOKUP_WITH_FREQ_KYWD:
                case LOOKUP_ON_CHANGE_KYWD:
                case LOOKUP_NOW_KYWD: {
                    lookup();
                    astFactory.addASTChild(currentAST, returnAST);
                    timeExpression_AST = (AST) currentAST.root;
                    break;
                }
                default:
                    if ((LA(1) == IDENT) && (LA(2) == LBRACKET || LA(2) == PERIOD) && (_tokenSet_30.member(LA(3)))) {
                        timeArrayReference();
                        astFactory.addASTChild(currentAST, returnAST);
                        timeExpression_AST = (AST) currentAST.root;
                    } else if ((LA(1) == IDENT) && (_tokenSet_73.member(LA(2))) && (_tokenSet_74.member(LA(3)))) {
                        timeVariable();
                        astFactory.addASTChild(currentAST, returnAST);
                        timeExpression_AST = (AST) currentAST.root;
                    } else {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_48);
            } else {
                throw ex;
            }
        }
        returnAST = timeExpression_AST;
    }

    public final void pair() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST pair_AST = null;

        try {      // for error handling
            ncName();
            astFactory.addASTChild(currentAST, returnAST);
            match(EQUALS);
            {
                switch (LA(1)) {
                    case TRUE_KYWD:
                    case FALSE_KYWD:
                    case INT:
                    case DOUBLE:
                    case STRING:
                    case LBRACKET: {
                        value();
                        astFactory.addASTChild(currentAST, returnAST);
                        break;
                    }
                    case LOOKUP_NOW_KYWD: {
                        lookupNow();
                        astFactory.addASTChild(currentAST, returnAST);
                        break;
                    }
                    default:
                        if ((LA(1) == IDENT) && (_tokenSet_75.member(LA(2))) && (_tokenSet_76.member(LA(3)))) {
                            variable();
                            astFactory.addASTChild(currentAST, returnAST);
                        } 
                        else if ((LA(1) == IDENT) && (LA(2) == LBRACKET || LA(2) == PERIOD) && (_tokenSet_30.member(LA(3)))) {
                            arrayReference();
                            astFactory.addASTChild(currentAST, returnAST);
                        } else {
                            throw new NoViableAltException(LT(1), getFilename());
                        }
                }
            }
            pair_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_18);
            } else {
                throw ex;
            }
        }
        returnAST = pair_AST;
    }

    public final void value() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST value_AST = null;

        try {      // for error handling
            if ((LA(1) == INT) && (LA(2) == SEMICOLON || LA(2) == COMMA || LA(2) == RPAREN) && (LA(3) == SEMICOLON || LA(3) == RBRACE || LA(3) == IDENT)) {
                integerValue();
                astFactory.addASTChild(currentAST, returnAST);
                value_AST = (AST) currentAST.root;
            } else if ((LA(1) == INT || LA(1) == DOUBLE) && (LA(2) == SEMICOLON || LA(2) == COMMA || LA(2) == RPAREN) && (LA(3) == SEMICOLON || LA(3) == RBRACE || LA(3) == IDENT)) {
                realValue();
                astFactory.addASTChild(currentAST, returnAST);
                value_AST = (AST) currentAST.root;
            } else if ((LA(1) == TRUE_KYWD || LA(1) == FALSE_KYWD || LA(1) == INT) && (LA(2) == SEMICOLON || LA(2) == COMMA || LA(2) == RPAREN) && (LA(3) == SEMICOLON || LA(3) == RBRACE || LA(3) == IDENT)) {
                booleanValue();
                astFactory.addASTChild(currentAST, returnAST);
                value_AST = (AST) currentAST.root;
            } else if ((LA(1) == STRING)) {
                stringValue();
                astFactory.addASTChild(currentAST, returnAST);
                value_AST = (AST) currentAST.root;
            } else if ((LA(1) == INT || LA(1) == LBRACKET) && (_tokenSet_77.member(LA(2))) && (_tokenSet_78.member(LA(3)))) {
                timeValue();
                astFactory.addASTChild(currentAST, returnAST);
                value_AST = (AST) currentAST.root;
            } else {
                throw new NoViableAltException(LT(1), getFilename());
            }

        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_17);
            } else {
                throw ex;
            }
        }
        returnAST = value_AST;
    }

    public final void lookupNow() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST lookupNow_AST = null;

        try {      // for error handling
            AST tmp207_AST = null;
            tmp207_AST = astFactory.create(LT(1));
            astFactory.makeASTRoot(currentAST, tmp207_AST);
            match(LOOKUP_NOW_KYWD);
            match(LPAREN);
            stateName();
            astFactory.addASTChild(currentAST, returnAST);
            {
                switch (LA(1)) {
                    case LPAREN: {
                        match(LPAREN);
                        {
                            switch (LA(1)) {
                                case TRUE_KYWD:
                                case FALSE_KYWD:
                                case INT:
                                case DOUBLE:
                                case STRING:
                                case IDENT: {
                                    argumentList();
                                    astFactory.addASTChild(currentAST, returnAST);
                                    break;
                                }
                                case RPAREN: {
                                    break;
                                }
                                default: {
                                    throw new NoViableAltException(LT(1), getFilename());
                                }
                            }
                        }
                        match(RPAREN);
                        break;
                    }
                    case RPAREN: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(RPAREN);
            lookupNow_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_52);
            } else {
                throw ex;
            }
        }
        returnAST = lookupNow_AST;
    }

    public final void libraryNodeIdRef() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST libraryNodeIdRef_AST = null;
        AST n_AST = null;

        try {      // for error handling
            nodeName();
            n_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            libraryNodeIdRef_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_12);
            } else {
                throw ex;
            }
        }
        returnAST = libraryNodeIdRef_AST;
    }

    public final void aliasSpecs(
            PlexilGlobalDeclaration decl) throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST aliasSpecs_AST = null;

        try {      // for error handling
            match(LPAREN);
            {
                switch (LA(1)) {
                    case IDENT: {
                        aliasSpec(decl);
                        astFactory.addASTChild(currentAST, returnAST);
                        {
                            _loop242:
                            do {
                                if ((LA(1) == COMMA)) {
                                    match(COMMA);
                                    aliasSpec(decl);
                                    astFactory.addASTChild(currentAST, returnAST);
                                } else {
                                    break _loop242;
                                }

                            } while (true);
                        }
                        break;
                    }
                    case RPAREN: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(RPAREN);
            if (inputState.guessing == 0) {
                aliasSpecs_AST = (AST) currentAST.root;
                aliasSpecs_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(ALIASES, "ALIASES")).add(aliasSpecs_AST));
                currentAST.root = aliasSpecs_AST;
                currentAST.child = aliasSpecs_AST != null && aliasSpecs_AST.getFirstChild() != null ? aliasSpecs_AST.getFirstChild() : aliasSpecs_AST;
                currentAST.advanceChildToEnd();
            }
            aliasSpecs_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_15);
            } else {
                throw ex;
            }
        }
        returnAST = aliasSpecs_AST;
    }

    public final void aliasSpec(
            PlexilGlobalDeclaration decl) throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST aliasSpec_AST = null;

        try {      // for error handling
            boolean synPredMatched246 = false;
            if (((LA(1) == IDENT) && (LA(2) == EQUALS || LA(2) == PERIOD) && (_tokenSet_44.member(LA(3))))) {
                int _m246 = mark();
                synPredMatched246 = true;
                inputState.guessing++;
                try {
                    {
                        {
                            ncName();
                            match(EQUALS);
                            ncName();
                        }
                    }
                } catch (RecognitionException pe) {
                    synPredMatched246 = false;
                }
                rewind(_m246);
                inputState.guessing--;
            }
            if (synPredMatched246) {
                varAlias(decl);
                astFactory.addASTChild(currentAST, returnAST);
                aliasSpec_AST = (AST) currentAST.root;
            } 
            else if ((LA(1) == IDENT) && (LA(2) == EQUALS || LA(2) == PERIOD) && (_tokenSet_79.member(LA(3)))) {
                constAlias(decl);
                astFactory.addASTChild(currentAST, returnAST);
                aliasSpec_AST = (AST) currentAST.root;
            } else {
                throw new NoViableAltException(LT(1), getFilename());
            }

        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_16);
            } else {
                throw ex;
            }
        }
        returnAST = aliasSpec_AST;
    }

    public final void varAlias(
            PlexilGlobalDeclaration decl) throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST varAlias_AST = null;

        try {      // for error handling
            nodeParameterName(decl);
            astFactory.addASTChild(currentAST, returnAST);
            match(EQUALS);
            variable();
            astFactory.addASTChild(currentAST, returnAST);
            if (inputState.guessing == 0) {
                varAlias_AST = (AST) currentAST.root;
                varAlias_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(VARIABLE_ALIAS, "VARIABLE_ALIAS")).add(varAlias_AST));
                currentAST.root = varAlias_AST;
                currentAST.child = varAlias_AST != null && varAlias_AST.getFirstChild() != null ? varAlias_AST.getFirstChild() : varAlias_AST;
                currentAST.advanceChildToEnd();
            }
            varAlias_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_16);
            } else {
                throw ex;
            }
        }
        returnAST = varAlias_AST;
    }

    public final void constAlias(
            PlexilGlobalDeclaration decl) throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST constAlias_AST = null;

        try {      // for error handling
            nodeParameterName(decl);
            astFactory.addASTChild(currentAST, returnAST);
            match(EQUALS);
            value();
            astFactory.addASTChild(currentAST, returnAST);
            if (inputState.guessing == 0) {
                constAlias_AST = (AST) currentAST.root;
                constAlias_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(CONST_ALIAS, "CONST_ALIAS")).add(constAlias_AST));
                currentAST.root = constAlias_AST;
                currentAST.child = constAlias_AST != null && constAlias_AST.getFirstChild() != null ? constAlias_AST.getFirstChild() : constAlias_AST;
                currentAST.advanceChildToEnd();
            }
            constAlias_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_16);
            } else {
                throw ex;
            }
        }
        returnAST = constAlias_AST;
    }

    public final void nodeParameterName(
            PlexilGlobalDeclaration decl) throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST nodeParameterName_AST = null;
        AST v_AST = null;

        try {      // for error handling
            variableName();
            v_AST = (AST) returnAST;
            astFactory.addASTChild(currentAST, returnAST);
            if (inputState.guessing == 0) {

                if (!decl.hasParameterNamed(v_AST.getText())) {
                    throw createSemanticException("\"" + v_AST.getText() + "\" is not a valid node parameter name for library node " + decl.getName(),
                            v_AST);
                }

            }
            nodeParameterName_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_80);
            } else {
                throw ex;
            }
        }
        returnAST = nodeParameterName_AST;
    }

    public final void booleanTerm() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST booleanTerm_AST = null;
        AST f1_AST = null;
        AST fn_AST = null;

        try {      // for error handling
            booleanFactor();
            f1_AST = (AST) returnAST;
            if (inputState.guessing == 0) {
                booleanTerm_AST = (AST) currentAST.root;
                booleanTerm_AST = f1_AST;
                currentAST.root = booleanTerm_AST;
                currentAST.child = booleanTerm_AST != null && booleanTerm_AST.getFirstChild() != null ? booleanTerm_AST.getFirstChild() : booleanTerm_AST;
                currentAST.advanceChildToEnd();
            }
            {
                _loop257:
                do {
                    if ((LA(1) == AND_KYWD || LA(1) == AND_ALT_KYWD)) {
                        {
                            switch (LA(1)) {
                                case AND_KYWD: {
                                    AST tmp217_AST = null;
                                    tmp217_AST = astFactory.create(LT(1));
                                    match(AND_KYWD);
                                    break;
                                }
                                case AND_ALT_KYWD: {
                                    AST tmp218_AST = null;
                                    tmp218_AST = astFactory.create(LT(1));
                                    match(AND_ALT_KYWD);
                                    break;
                                }
                                default: {
                                    throw new NoViableAltException(LT(1), getFilename());
                                }
                            }
                        }
                        booleanFactor();
                        fn_AST = (AST) returnAST;
                        if (inputState.guessing == 0) {
                            booleanTerm_AST = (AST) currentAST.root;
                            booleanTerm_AST = (AST) astFactory.make((new ASTArray(3)).add(astFactory.create(AND_KYWD, "AND")).add(booleanTerm_AST).add(fn_AST));
                            currentAST.root = booleanTerm_AST;
                            currentAST.child = booleanTerm_AST != null && booleanTerm_AST.getFirstChild() != null ? booleanTerm_AST.getFirstChild() : booleanTerm_AST;
                            currentAST.advanceChildToEnd();
                        }
                    } else {
                        break _loop257;
                    }

                } while (true);
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_81);
            } else {
                throw ex;
            }
        }
        returnAST = booleanTerm_AST;
    }

    public final void booleanFactor() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST booleanFactor_AST = null;
        AST b1_AST = null;
        AST bop_AST = null;
        AST b2_AST = null;
        AST n1_AST = null;
        AST nop_AST = null;
        AST n2_AST = null;
        AST s1_AST = null;
        AST sop_AST = null;
        AST s2_AST = null;
        AST t1_AST = null;
        AST top_AST = null;
        AST t2_AST = null;
        AST ns1_AST = null;
        AST nsop_AST = null;
        AST ns2_AST = null;
        AST no1_AST = null;
        AST noop_AST = null;
        AST no2_AST = null;
        AST nch1_AST = null;
        AST nchop_AST = null;
        AST nch2_AST = null;
        AST nf1_AST = null;
        AST nfop_AST = null;
        AST nf2_AST = null;

        try {      // for error handling
            boolean synPredMatched261 = false;
            if (((_tokenSet_82.member(LA(1))) && (_tokenSet_83.member(LA(2))) && (_tokenSet_84.member(LA(3))))) {
                int _m261 = mark();
                synPredMatched261 = true;
                inputState.guessing++;
                try {
                    {
                        booleanKernel();
                        {
                            if ((LA(1) == DEQUALS || LA(1) == NEQUALS)) {
                                equalityOperator();
                                booleanKernel();
                            } else {
                            }

                        }
                    }
                } catch (RecognitionException pe) {
                    synPredMatched261 = false;
                }
                rewind(_m261);
                inputState.guessing--;
            }
            if (synPredMatched261) {
                booleanKernel();
                b1_AST = (AST) returnAST;
                if (inputState.guessing == 0) {
                    booleanFactor_AST = (AST) currentAST.root;
                    booleanFactor_AST = b1_AST;
                    currentAST.root = booleanFactor_AST;
                    currentAST.child = booleanFactor_AST != null && booleanFactor_AST.getFirstChild() != null ? booleanFactor_AST.getFirstChild() : booleanFactor_AST;
                    currentAST.advanceChildToEnd();
                }
                {
                    switch (LA(1)) {
                        case DEQUALS:
                        case NEQUALS: {
                            equalityOperator();
                            bop_AST = (AST) returnAST;
                            booleanKernel();
                            b2_AST = (AST) returnAST;
                            if (inputState.guessing == 0) {
                                booleanFactor_AST = (AST) currentAST.root;
                                booleanFactor_AST = (AST) astFactory.make((new ASTArray(4)).add(astFactory.create(BOOLEAN_COMPARISON, "BOOLEAN_COMPARISON")).add(bop_AST).add(booleanFactor_AST).add(b2_AST));
                                currentAST.root = booleanFactor_AST;
                                currentAST.child = booleanFactor_AST != null && booleanFactor_AST.getFirstChild() != null ? booleanFactor_AST.getFirstChild() : booleanFactor_AST;
                                currentAST.advanceChildToEnd();
                            }
                            break;
                        }
                        case OR_KYWD:
                        case AND_KYWD:
                        case SEMICOLON:
                        case COMMA:
                        case RPAREN:
                        case OR_ALT_KYWD:
                        case AND_ALT_KYWD: {
                            break;
                        }
                        default: {
                            throw new NoViableAltException(LT(1), getFilename());
                        }
                    }
                }
            } else {
                boolean synPredMatched264 = false;
                if (((_tokenSet_35.member(LA(1))) && (_tokenSet_85.member(LA(2))) && (_tokenSet_86.member(LA(3))))) {
                    int _m264 = mark();
                    synPredMatched264 = true;
                    inputState.guessing++;
                    try {
                        {
                            numericExpression();
                            equalityOperator();
                            numericExpression();
                        }
                    } catch (RecognitionException pe) {
                        synPredMatched264 = false;
                    }
                    rewind(_m264);
                    inputState.guessing--;
                }
                if (synPredMatched264) {
                    {
                        numericExpression();
                        n1_AST = (AST) returnAST;
                        equalityOperator();
                        nop_AST = (AST) returnAST;
                        numericExpression();
                        n2_AST = (AST) returnAST;
                        if (inputState.guessing == 0) {
                            booleanFactor_AST = (AST) currentAST.root;
                            booleanFactor_AST = (AST) astFactory.make((new ASTArray(4)).add(astFactory.create(NUMERIC_COMPARISON, "NUMERIC_COMPARISON")).add(nop_AST).add(n1_AST).add(n2_AST));
                            currentAST.root = booleanFactor_AST;
                            currentAST.child = booleanFactor_AST != null && booleanFactor_AST.getFirstChild() != null ? booleanFactor_AST.getFirstChild() : booleanFactor_AST;
                            currentAST.advanceChildToEnd();
                        }
                    }
                } else {
                    boolean synPredMatched267 = false;
                    if (((_tokenSet_38.member(LA(1))) && (_tokenSet_87.member(LA(2))) && (_tokenSet_88.member(LA(3))))) {
                        int _m267 = mark();
                        synPredMatched267 = true;
                        inputState.guessing++;
                        try {
                            {
                                stringExpression();
                                equalityOperator();
                                stringExpression();
                            }
                        } catch (RecognitionException pe) {
                            synPredMatched267 = false;
                        }
                        rewind(_m267);
                        inputState.guessing--;
                    }
                    if (synPredMatched267) {
                        {
                            stringExpression();
                            s1_AST = (AST) returnAST;
                            equalityOperator();
                            sop_AST = (AST) returnAST;
                            stringExpression();
                            s2_AST = (AST) returnAST;
                            if (inputState.guessing == 0) {
                                booleanFactor_AST = (AST) currentAST.root;
                                booleanFactor_AST = (AST) astFactory.make((new ASTArray(4)).add(astFactory.create(STRING_COMPARISON, "STRING_COMPARISON")).add(sop_AST).add(s1_AST).add(s2_AST));
                                currentAST.root = booleanFactor_AST;
                                currentAST.child = booleanFactor_AST != null && booleanFactor_AST.getFirstChild() != null ? booleanFactor_AST.getFirstChild() : booleanFactor_AST;
                                currentAST.advanceChildToEnd();
                            }
                        }
                    } else {
                        boolean synPredMatched270 = false;
                        if (((_tokenSet_41.member(LA(1))) && (_tokenSet_89.member(LA(2))) && (_tokenSet_90.member(LA(3))))) {
                            int _m270 = mark();
                            synPredMatched270 = true;
                            inputState.guessing++;
                            try {
                                {
                                    timeExpression();
                                    equalityOperator();
                                    timeExpression();
                                }
                            } catch (RecognitionException pe) {
                                synPredMatched270 = false;
                            }
                            rewind(_m270);
                            inputState.guessing--;
                        }
                        if (synPredMatched270) {
                            {
                                timeExpression();
                                t1_AST = (AST) returnAST;
                                equalityOperator();
                                top_AST = (AST) returnAST;
                                timeExpression();
                                t2_AST = (AST) returnAST;
                                if (inputState.guessing == 0) {
                                    booleanFactor_AST = (AST) currentAST.root;
                                    booleanFactor_AST = (AST) astFactory.make((new ASTArray(4)).add(astFactory.create(TIME_COMPARISON, "TIME_COMPARISON")).add(top_AST).add(t1_AST).add(t2_AST));
                                    currentAST.root = booleanFactor_AST;
                                    currentAST.child = booleanFactor_AST != null && booleanFactor_AST.getFirstChild() != null ? booleanFactor_AST.getFirstChild() : booleanFactor_AST;
                                    currentAST.advanceChildToEnd();
                                }
                            }
                        } else if ((_tokenSet_91.member(LA(1))) && (_tokenSet_92.member(LA(2))) && (_tokenSet_93.member(LA(3)))) {
                            {
                                nodeState();
                                ns1_AST = (AST) returnAST;
                                equalityOperator();
                                nsop_AST = (AST) returnAST;
                                nodeState();
                                ns2_AST = (AST) returnAST;
                                if (inputState.guessing == 0) {
                                    booleanFactor_AST = (AST) currentAST.root;
                                    booleanFactor_AST = (AST) astFactory.make((new ASTArray(4)).add(astFactory.create(NODE_STATE_COMPARISON, "NODE_STATE_COMPARISON")).add(nsop_AST).add(ns1_AST).add(ns2_AST));
                                    currentAST.root = booleanFactor_AST;
                                    currentAST.child = booleanFactor_AST != null && booleanFactor_AST.getFirstChild() != null ? booleanFactor_AST.getFirstChild() : booleanFactor_AST;
                                    currentAST.advanceChildToEnd();
                                }
                            }
                        } else if ((_tokenSet_94.member(LA(1))) && (_tokenSet_92.member(LA(2))) && (_tokenSet_95.member(LA(3)))) {
                            {
                                nodeOutcome();
                                no1_AST = (AST) returnAST;
                                equalityOperator();
                                noop_AST = (AST) returnAST;
                                nodeOutcome();
                                no2_AST = (AST) returnAST;
                                if (inputState.guessing == 0) {
                                    booleanFactor_AST = (AST) currentAST.root;
                                    booleanFactor_AST = (AST) astFactory.make((new ASTArray(4)).add(astFactory.create(NODE_OUTCOME_COMPARISON, "NODE_OUTCOME_COMPARISON")).add(noop_AST).add(no1_AST).add(no2_AST));
                                    currentAST.root = booleanFactor_AST;
                                    currentAST.child = booleanFactor_AST != null && booleanFactor_AST.getFirstChild() != null ? booleanFactor_AST.getFirstChild() : booleanFactor_AST;
                                    currentAST.advanceChildToEnd();
                                }
                            }
                        } else if ((_tokenSet_96.member(LA(1))) && (_tokenSet_92.member(LA(2))) && (_tokenSet_97.member(LA(3)))) {
                            {
                                nodeCommandHandle();
                                nch1_AST = (AST) returnAST;
                                equalityOperator();
                                nchop_AST = (AST) returnAST;
                                nodeCommandHandle();
                                nch2_AST = (AST) returnAST;
                                if (inputState.guessing == 0) {
                                    booleanFactor_AST = (AST) currentAST.root;
                                    booleanFactor_AST = (AST) astFactory.make((new ASTArray(4)).add(astFactory.create(NODE_COMMAND_HANDLE_COMPARISON, "NODE_COMMAND_HANDLE_COMPARISON")).add(nchop_AST).add(nch1_AST).add(nch2_AST));
                                    currentAST.root = booleanFactor_AST;
                                    currentAST.child = booleanFactor_AST != null && booleanFactor_AST.getFirstChild() != null ? booleanFactor_AST.getFirstChild() : booleanFactor_AST;
                                    currentAST.advanceChildToEnd();
                                }
                            }
                        } else if ((_tokenSet_98.member(LA(1))) && (_tokenSet_92.member(LA(2))) && (_tokenSet_99.member(LA(3)))) {
                            {
                                nodeFailure();
                                nf1_AST = (AST) returnAST;
                                equalityOperator();
                                nfop_AST = (AST) returnAST;
                                nodeFailure();
                                nf2_AST = (AST) returnAST;
                                if (inputState.guessing == 0) {
                                    booleanFactor_AST = (AST) currentAST.root;
                                    booleanFactor_AST = (AST) astFactory.make((new ASTArray(4)).add(astFactory.create(NODE_FAILURE_COMPARISON, "NODE_FAILURE_COMPARISON")).add(nfop_AST).add(nf1_AST).add(nf2_AST));
                                    currentAST.root = booleanFactor_AST;
                                    currentAST.child = booleanFactor_AST != null && booleanFactor_AST.getFirstChild() != null ? booleanFactor_AST.getFirstChild() : booleanFactor_AST;
                                    currentAST.advanceChildToEnd();
                                }
                            }
                        } else {
                            throw new NoViableAltException(LT(1), getFilename());
                        }
                    }
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_100);
            } else {
                throw ex;
            }
        }
        returnAST = booleanFactor_AST;
    }

    public final void booleanKernel() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST booleanKernel_AST = null;
        AST e1_AST = null;
        AST op_AST = null;
        AST e2_AST = null;

        try {      // for error handling
            boolean synPredMatched279 = false;
            if (((_tokenSet_35.member(LA(1))) && (_tokenSet_101.member(LA(2))) && (_tokenSet_102.member(LA(3))))) {
                int _m279 = mark();
                synPredMatched279 = true;
                inputState.guessing++;
                try {
                    {
                        numericExpression();
                        comparisonOperator();
                        numericExpression();
                    }
                } catch (RecognitionException pe) {
                    synPredMatched279 = false;
                }
                rewind(_m279);
                inputState.guessing--;
            }
            if (synPredMatched279) {
                numericExpression();
                e1_AST = (AST) returnAST;
                comparisonOperator();
                op_AST = (AST) returnAST;
                numericExpression();
                e2_AST = (AST) returnAST;
                if (inputState.guessing == 0) {
                    booleanKernel_AST = (AST) currentAST.root;
                    booleanKernel_AST = (AST) astFactory.make((new ASTArray(3)).add(op_AST).add(e1_AST).add(e2_AST));
                    currentAST.root = booleanKernel_AST;
                    currentAST.child = booleanKernel_AST != null && booleanKernel_AST.getFirstChild() != null ? booleanKernel_AST.getFirstChild() : booleanKernel_AST;
                    currentAST.advanceChildToEnd();
                }
            } else if ((LA(1) == BANG)) {
                not();
                astFactory.addASTChild(currentAST, returnAST);
                booleanKernel_AST = (AST) currentAST.root;
            } else if ((_tokenSet_103.member(LA(1))) && (_tokenSet_104.member(LA(2))) && (_tokenSet_84.member(LA(3)))) {
                simpleBoolean();
                astFactory.addASTChild(currentAST, returnAST);
                booleanKernel_AST = (AST) currentAST.root;
            } else {
                throw new NoViableAltException(LT(1), getFilename());
            }

        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_48);
            } else {
                throw ex;
            }
        }
        returnAST = booleanKernel_AST;
    }

    public final void equalityOperator() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST equalityOperator_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case DEQUALS: {
                    AST tmp219_AST = null;
                    tmp219_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp219_AST);
                    match(DEQUALS);
                    equalityOperator_AST = (AST) currentAST.root;
                    break;
                }
                case NEQUALS: {
                    AST tmp220_AST = null;
                    tmp220_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp220_AST);
                    match(NEQUALS);
                    equalityOperator_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_32);
            } else {
                throw ex;
            }
        }
        returnAST = equalityOperator_AST;
    }

    public final void nodeState() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST nodeState_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case IDENT: {
                    nodeStateVariable();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeState_AST = (AST) currentAST.root;
                    break;
                }
                case WAITING_STATE_KYWD:
                case EXECUTING_STATE_KYWD:
                case FINISHING_STATE_KYWD:
                case FAILING_STATE_KYWD:
                case FINISHED_STATE_KYWD:
                case ITERATION_ENDED_STATE_KYWD:
                case INACTIVE_STATE_KYWD: {
                    nodeStateValue();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeState_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_48);
            } else {
                throw ex;
            }
        }
        returnAST = nodeState_AST;
    }

    public final void nodeOutcome() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST nodeOutcome_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case IDENT: {
                    nodeOutcomeVariable();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeOutcome_AST = (AST) currentAST.root;
                    break;
                }
                case SUCCESS_OUTCOME_KYWD:
                case FAILURE_OUTCOME_KYWD:
                case SKIPPED_OUTCOME_KYWD:
                case INFINITE_LOOP_KYWD: {
                    nodeOutcomeValue();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeOutcome_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_48);
            } else {
                throw ex;
            }
        }
        returnAST = nodeOutcome_AST;
    }

    public final void nodeCommandHandle() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST nodeCommandHandle_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case IDENT: {
                    nodeCommandHandleVariable();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeCommandHandle_AST = (AST) currentAST.root;
                    break;
                }
                case COMMAND_SENT_KYWD:
                case COMMAND_ACCEPTED_KYWD:
                case COMMAND_DENIED_KYWD:
                case COMMAND_RCVD_KYWD:
                case COMMAND_FAILED_KYWD:
                case COMMAND_SUCCESS_KYWD: {
                    nodeCommandHandleValue();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeCommandHandle_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_48);
            } else {
                throw ex;
            }
        }
        returnAST = nodeCommandHandle_AST;
    }

    public final void nodeFailure() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST nodeFailure_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case IDENT: {
                    nodeFailureVariable();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeFailure_AST = (AST) currentAST.root;
                    break;
                }
                case INFINITE_LOOP_KYWD:
                case COMMAND_FAILED_KYWD:
                case PRE_CONDITION_FAILED_KYWD:
                case POST_CONDITION_FAILED_KYWD:
                case INVARIANT_CONDITION_FAILED_KYWD:
                case PARENT_FAILED_KYWD: {
                    nodeFailureValue();
                    astFactory.addASTChild(currentAST, returnAST);
                    nodeFailure_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_48);
            } else {
                throw ex;
            }
        }
        returnAST = nodeFailure_AST;
    }

    public final void comparisonOperator() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST comparisonOperator_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case GREATER: {
                    AST tmp221_AST = null;
                    tmp221_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp221_AST);
                    match(GREATER);
                    comparisonOperator_AST = (AST) currentAST.root;
                    break;
                }
                case GEQ: {
                    AST tmp222_AST = null;
                    tmp222_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp222_AST);
                    match(GEQ);
                    comparisonOperator_AST = (AST) currentAST.root;
                    break;
                }
                case LESS: {
                    AST tmp223_AST = null;
                    tmp223_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp223_AST);
                    match(LESS);
                    comparisonOperator_AST = (AST) currentAST.root;
                    break;
                }
                case LEQ: {
                    AST tmp224_AST = null;
                    tmp224_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp224_AST);
                    match(LEQ);
                    comparisonOperator_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_35);
            } else {
                throw ex;
            }
        }
        returnAST = comparisonOperator_AST;
    }

    public final void not() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST not_AST = null;

        try {      // for error handling
            AST tmp225_AST = null;
            tmp225_AST = astFactory.create(LT(1));
            astFactory.makeASTRoot(currentAST, tmp225_AST);
            match(BANG);
            simpleBoolean();
            astFactory.addASTChild(currentAST, returnAST);
            not_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_48);
            } else {
                throw ex;
            }
        }
        returnAST = not_AST;
    }

    public final void simpleBoolean() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST simpleBoolean_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case LPAREN: {
                    match(LPAREN);
                    booleanExpression();
                    astFactory.addASTChild(currentAST, returnAST);
                    match(RPAREN);
                    simpleBoolean_AST = (AST) currentAST.root;
                    break;
                }
                case TRUE_KYWD:
                case FALSE_KYWD:
                case INT: {
                    booleanValue();
                    astFactory.addASTChild(currentAST, returnAST);
                    simpleBoolean_AST = (AST) currentAST.root;
                    break;
                }
                case IS_KNOWN_KYWD: {
                    isKnownExp();
                    astFactory.addASTChild(currentAST, returnAST);
                    simpleBoolean_AST = (AST) currentAST.root;
                    break;
                }
                case LOOKUP_WITH_FREQ_KYWD:
                case LOOKUP_ON_CHANGE_KYWD:
                case LOOKUP_NOW_KYWD: {
                    lookup();
                    astFactory.addASTChild(currentAST, returnAST);
                    simpleBoolean_AST = (AST) currentAST.root;
                    break;
                }
                default:
                    if ((LA(1) == IDENT) && (LA(2) == LBRACKET || LA(2) == PERIOD) && (_tokenSet_30.member(LA(3)))) {
                        booleanArrayReference();
                        astFactory.addASTChild(currentAST, returnAST);
                        simpleBoolean_AST = (AST) currentAST.root;
                    } else if ((LA(1) == IDENT) && (_tokenSet_73.member(LA(2))) && (_tokenSet_74.member(LA(3)))) {
                        booleanVariable();
                        astFactory.addASTChild(currentAST, returnAST);
                        simpleBoolean_AST = (AST) currentAST.root;
                    } else {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_48);
            } else {
                throw ex;
            }
        }
        returnAST = simpleBoolean_AST;
    }

    public final void isKnownExp() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST isKnownExp_AST = null;

        try {      // for error handling
            AST tmp228_AST = null;
            tmp228_AST = astFactory.create(LT(1));
            astFactory.makeASTRoot(currentAST, tmp228_AST);
            match(IS_KNOWN_KYWD);
            match(LPAREN);
            {
                boolean synPredMatched285 = false;
                if (((LA(1) == IDENT) && (LA(2) == LBRACKET || LA(2) == PERIOD) && (_tokenSet_30.member(LA(3))))) {
                    int _m285 = mark();
                    synPredMatched285 = true;
                    inputState.guessing++;
                    try {
                        {
                            ncName();
                            match(LBRACKET);
                        }
                    } catch (RecognitionException pe) {
                        synPredMatched285 = false;
                    }
                    rewind(_m285);
                    inputState.guessing--;
                }
                if (synPredMatched285) {
                    arrayReference();
                    astFactory.addASTChild(currentAST, returnAST);
                } 
                else if ((LA(1) == IDENT) && (LA(2) == RPAREN || LA(2) == PERIOD) && (_tokenSet_105.member(LA(3)))) {
                    variable();
                    astFactory.addASTChild(currentAST, returnAST);
                } 
                else if ((LA(1) == IDENT) && (LA(2) == PERIOD) && (_tokenSet_106.member(LA(3)))) {
                    nodeStateVariable();
                    astFactory.addASTChild(currentAST, returnAST);
                } 
                else if ((LA(1) == IDENT) && (LA(2) == PERIOD) && (_tokenSet_107.member(LA(3)))) {
                    nodeOutcomeVariable();
                    astFactory.addASTChild(currentAST, returnAST);
                } 
                else if ((LA(1) == IDENT) && (LA(2) == PERIOD) && (_tokenSet_108.member(LA(3)))) {
                    nodeTimepointValue();
                    astFactory.addASTChild(currentAST, returnAST);
                } else {
                    throw new NoViableAltException(LT(1), getFilename());
                }

            }
            match(RPAREN);
            isKnownExp_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_48);
            } else {
                throw ex;
            }
        }
        returnAST = isKnownExp_AST;
    }

    public final void lookup() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST lookup_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case LOOKUP_WITH_FREQ_KYWD: {
                    lookupWithFrequency();
                    astFactory.addASTChild(currentAST, returnAST);
                    lookup_AST = (AST) currentAST.root;
                    break;
                }
                case LOOKUP_ON_CHANGE_KYWD: {
                    lookupOnChange();
                    astFactory.addASTChild(currentAST, returnAST);
                    lookup_AST = (AST) currentAST.root;
                    break;
                }
                case LOOKUP_NOW_KYWD: {
                    lookupNow();
                    astFactory.addASTChild(currentAST, returnAST);
                    lookup_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_52);
            } else {
                throw ex;
            }
        }
        returnAST = lookup_AST;
    }

    public final void nodeStateVariable() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST nodeStateVariable_AST = null;
        AST nid_AST = null;

        try {      // for error handling
            nodeIdRef();
            nid_AST = (AST) returnAST;
            match(PERIOD);
            AST tmp232_AST = null;
            tmp232_AST = astFactory.create(LT(1));
            match(STATE_KYWD);
            if (inputState.guessing == 0) {
                nodeStateVariable_AST = (AST) currentAST.root;
                nodeStateVariable_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(NODE_STATE_VARIABLE, "NODE_STATE_VARIABLE")).add(nid_AST));
                currentAST.root = nodeStateVariable_AST;
                currentAST.child = nodeStateVariable_AST != null && nodeStateVariable_AST.getFirstChild() != null ? nodeStateVariable_AST.getFirstChild() : nodeStateVariable_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_48);
            } else {
                throw ex;
            }
        }
        returnAST = nodeStateVariable_AST;
    }

    public final void nodeOutcomeVariable() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST nodeOutcomeVariable_AST = null;
        AST nid_AST = null;

        try {      // for error handling
            nodeIdRef();
            nid_AST = (AST) returnAST;
            match(PERIOD);
            match(OUTCOME_KYWD);
            if (inputState.guessing == 0) {
                nodeOutcomeVariable_AST = (AST) currentAST.root;
                nodeOutcomeVariable_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(NODE_OUTCOME_VARIABLE, "NODE_OUTCOME_VARIABLE")).add(nid_AST));
                currentAST.root = nodeOutcomeVariable_AST;
                currentAST.child = nodeOutcomeVariable_AST != null && nodeOutcomeVariable_AST.getFirstChild() != null ? nodeOutcomeVariable_AST.getFirstChild() : nodeOutcomeVariable_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_48);
            } else {
                throw ex;
            }
        }
        returnAST = nodeOutcomeVariable_AST;
    }

    public final void nodeTimepointValue() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST nodeTimepointValue_AST = null;

        try {      // for error handling
            nodeIdRef();
            astFactory.addASTChild(currentAST, returnAST);
            match(PERIOD);
            nodeStateValue();
            astFactory.addASTChild(currentAST, returnAST);
            match(PERIOD);
            timepoint();
            astFactory.addASTChild(currentAST, returnAST);
            if (inputState.guessing == 0) {
                nodeTimepointValue_AST = (AST) currentAST.root;
                nodeTimepointValue_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(NODE_TIMEPOINT_VALUE, "NODE_TIMEPOINT_VALUE")).add(nodeTimepointValue_AST));
                currentAST.root = nodeTimepointValue_AST;
                currentAST.child = nodeTimepointValue_AST != null && nodeTimepointValue_AST.getFirstChild() != null ? nodeTimepointValue_AST.getFirstChild() : nodeTimepointValue_AST;
                currentAST.advanceChildToEnd();
            }
            nodeTimepointValue_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_52);
            } else {
                throw ex;
            }
        }
        returnAST = nodeTimepointValue_AST;
    }

    public final void nodeStateValue() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST nodeStateValue_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case WAITING_STATE_KYWD: {
                    AST tmp237_AST = null;
                    tmp237_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp237_AST);
                    match(WAITING_STATE_KYWD);
                    nodeStateValue_AST = (AST) currentAST.root;
                    break;
                }
                case EXECUTING_STATE_KYWD: {
                    AST tmp238_AST = null;
                    tmp238_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp238_AST);
                    match(EXECUTING_STATE_KYWD);
                    nodeStateValue_AST = (AST) currentAST.root;
                    break;
                }
                case FINISHING_STATE_KYWD: {
                    AST tmp239_AST = null;
                    tmp239_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp239_AST);
                    match(FINISHING_STATE_KYWD);
                    nodeStateValue_AST = (AST) currentAST.root;
                    break;
                }
                case FAILING_STATE_KYWD: {
                    AST tmp240_AST = null;
                    tmp240_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp240_AST);
                    match(FAILING_STATE_KYWD);
                    nodeStateValue_AST = (AST) currentAST.root;
                    break;
                }
                case FINISHED_STATE_KYWD: {
                    AST tmp241_AST = null;
                    tmp241_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp241_AST);
                    match(FINISHED_STATE_KYWD);
                    nodeStateValue_AST = (AST) currentAST.root;
                    break;
                }
                case ITERATION_ENDED_STATE_KYWD: {
                    AST tmp242_AST = null;
                    tmp242_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp242_AST);
                    match(ITERATION_ENDED_STATE_KYWD);
                    nodeStateValue_AST = (AST) currentAST.root;
                    break;
                }
                case INACTIVE_STATE_KYWD: {
                    AST tmp243_AST = null;
                    tmp243_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp243_AST);
                    match(INACTIVE_STATE_KYWD);
                    nodeStateValue_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_109);
            } else {
                throw ex;
            }
        }
        returnAST = nodeStateValue_AST;
    }

    public final void nodeIdRef() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST nodeIdRef_AST = null;

        try {      // for error handling
            nodeName();
            astFactory.addASTChild(currentAST, returnAST);
            nodeIdRef_AST = (AST) currentAST.root;
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_110);
            } else {
                throw ex;
            }
        }
        returnAST = nodeIdRef_AST;
    }

    public final void nodeOutcomeValue() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST nodeOutcomeValue_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case SUCCESS_OUTCOME_KYWD: {
                    AST tmp244_AST = null;
                    tmp244_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp244_AST);
                    match(SUCCESS_OUTCOME_KYWD);
                    nodeOutcomeValue_AST = (AST) currentAST.root;
                    break;
                }
                case FAILURE_OUTCOME_KYWD: {
                    AST tmp245_AST = null;
                    tmp245_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp245_AST);
                    match(FAILURE_OUTCOME_KYWD);
                    nodeOutcomeValue_AST = (AST) currentAST.root;
                    break;
                }
                case SKIPPED_OUTCOME_KYWD: {
                    AST tmp246_AST = null;
                    tmp246_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp246_AST);
                    match(SKIPPED_OUTCOME_KYWD);
                    nodeOutcomeValue_AST = (AST) currentAST.root;
                    break;
                }
                case INFINITE_LOOP_KYWD: {
                    AST tmp247_AST = null;
                    tmp247_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp247_AST);
                    match(INFINITE_LOOP_KYWD);
                    nodeOutcomeValue_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_48);
            } else {
                throw ex;
            }
        }
        returnAST = nodeOutcomeValue_AST;
    }

    public final void nodeCommandHandleVariable() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST nodeCommandHandleVariable_AST = null;
        AST nid_AST = null;

        try {      // for error handling
            nodeIdRef();
            nid_AST = (AST) returnAST;
            match(PERIOD);
            match(COMMAND_HANDLE_KYWD);
            if (inputState.guessing == 0) {
                nodeCommandHandleVariable_AST = (AST) currentAST.root;
                nodeCommandHandleVariable_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(NODE_COMMAND_HANDLE_VARIABLE, "NODE_COMMAND_HANDLE_VARIABLE")).add(nid_AST));
                currentAST.root = nodeCommandHandleVariable_AST;
                currentAST.child = nodeCommandHandleVariable_AST != null && nodeCommandHandleVariable_AST.getFirstChild() != null ? nodeCommandHandleVariable_AST.getFirstChild() : nodeCommandHandleVariable_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_48);
            } else {
                throw ex;
            }
        }
        returnAST = nodeCommandHandleVariable_AST;
    }

    public final void nodeCommandHandleValue() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST nodeCommandHandleValue_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case COMMAND_SENT_KYWD: {
                    AST tmp250_AST = null;
                    tmp250_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp250_AST);
                    match(COMMAND_SENT_KYWD);
                    nodeCommandHandleValue_AST = (AST) currentAST.root;
                    break;
                }
                case COMMAND_ACCEPTED_KYWD: {
                    AST tmp251_AST = null;
                    tmp251_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp251_AST);
                    match(COMMAND_ACCEPTED_KYWD);
                    nodeCommandHandleValue_AST = (AST) currentAST.root;
                    break;
                }
                case COMMAND_DENIED_KYWD: {
                    AST tmp252_AST = null;
                    tmp252_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp252_AST);
                    match(COMMAND_DENIED_KYWD);
                    nodeCommandHandleValue_AST = (AST) currentAST.root;
                    break;
                }
                case COMMAND_RCVD_KYWD: {
                    AST tmp253_AST = null;
                    tmp253_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp253_AST);
                    match(COMMAND_RCVD_KYWD);
                    nodeCommandHandleValue_AST = (AST) currentAST.root;
                    break;
                }
                case COMMAND_FAILED_KYWD: {
                    AST tmp254_AST = null;
                    tmp254_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp254_AST);
                    match(COMMAND_FAILED_KYWD);
                    nodeCommandHandleValue_AST = (AST) currentAST.root;
                    break;
                }
                case COMMAND_SUCCESS_KYWD: {
                    AST tmp255_AST = null;
                    tmp255_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp255_AST);
                    match(COMMAND_SUCCESS_KYWD);
                    nodeCommandHandleValue_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_48);
            } else {
                throw ex;
            }
        }
        returnAST = nodeCommandHandleValue_AST;
    }

    public final void nodeFailureVariable() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST nodeFailureVariable_AST = null;
        AST nid_AST = null;

        try {      // for error handling
            nodeIdRef();
            nid_AST = (AST) returnAST;
            match(PERIOD);
            match(FAILURE_KYWD);
            if (inputState.guessing == 0) {
                nodeFailureVariable_AST = (AST) currentAST.root;
                nodeFailureVariable_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(NODE_FAILURE_VARIABLE, "NODE_FAILURE_VARIABLE")).add(nid_AST));
                currentAST.root = nodeFailureVariable_AST;
                currentAST.child = nodeFailureVariable_AST != null && nodeFailureVariable_AST.getFirstChild() != null ? nodeFailureVariable_AST.getFirstChild() : nodeFailureVariable_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_48);
            } else {
                throw ex;
            }
        }
        returnAST = nodeFailureVariable_AST;
    }

    public final void nodeFailureValue() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST nodeFailureValue_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case PRE_CONDITION_FAILED_KYWD: {
                    AST tmp258_AST = null;
                    tmp258_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp258_AST);
                    match(PRE_CONDITION_FAILED_KYWD);
                    nodeFailureValue_AST = (AST) currentAST.root;
                    break;
                }
                case POST_CONDITION_FAILED_KYWD: {
                    AST tmp259_AST = null;
                    tmp259_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp259_AST);
                    match(POST_CONDITION_FAILED_KYWD);
                    nodeFailureValue_AST = (AST) currentAST.root;
                    break;
                }
                case INVARIANT_CONDITION_FAILED_KYWD: {
                    AST tmp260_AST = null;
                    tmp260_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp260_AST);
                    match(INVARIANT_CONDITION_FAILED_KYWD);
                    nodeFailureValue_AST = (AST) currentAST.root;
                    break;
                }
                case PARENT_FAILED_KYWD: {
                    AST tmp261_AST = null;
                    tmp261_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp261_AST);
                    match(PARENT_FAILED_KYWD);
                    nodeFailureValue_AST = (AST) currentAST.root;
                    break;
                }
                case COMMAND_FAILED_KYWD: {
                    AST tmp262_AST = null;
                    tmp262_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp262_AST);
                    match(COMMAND_FAILED_KYWD);
                    nodeFailureValue_AST = (AST) currentAST.root;
                    break;
                }
                case INFINITE_LOOP_KYWD: {
                    AST tmp263_AST = null;
                    tmp263_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp263_AST);
                    match(INFINITE_LOOP_KYWD);
                    nodeFailureValue_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_48);
            } else {
                throw ex;
            }
        }
        returnAST = nodeFailureValue_AST;
    }

    public final void timepoint() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST timepoint_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case START_KYWD: {
                    AST tmp264_AST = null;
                    tmp264_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp264_AST);
                    match(START_KYWD);
                    timepoint_AST = (AST) currentAST.root;
                    break;
                }
                case END_KYWD: {
                    AST tmp265_AST = null;
                    tmp265_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp265_AST);
                    match(END_KYWD);
                    timepoint_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_52);
            } else {
                throw ex;
            }
        }
        returnAST = timepoint_AST;
    }

    public final void numericTerm() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST numericTerm_AST = null;
        AST f1_AST = null;
        AST on_AST = null;
        AST fn_AST = null;

        try {      // for error handling
            numericFactor();
            f1_AST = (AST) returnAST;
            if (inputState.guessing == 0) {
                numericTerm_AST = (AST) currentAST.root;
                numericTerm_AST = f1_AST;
                currentAST.root = numericTerm_AST;
                currentAST.child = numericTerm_AST != null && numericTerm_AST.getFirstChild() != null ? numericTerm_AST.getFirstChild() : numericTerm_AST;
                currentAST.advanceChildToEnd();
            }
            {
                _loop307:
                do {
                    if ((LA(1) == ASTERISK || LA(1) == SLASH)) {
                        multiplicativeOperator();
                        on_AST = (AST) returnAST;
                        numericFactor();
                        fn_AST = (AST) returnAST;
                        if (inputState.guessing == 0) {
                            numericTerm_AST = (AST) currentAST.root;
                            numericTerm_AST = (AST) astFactory.make((new ASTArray(3)).add(on_AST).add(numericTerm_AST).add(fn_AST));
                            currentAST.root = numericTerm_AST;
                            currentAST.child = numericTerm_AST != null && numericTerm_AST.getFirstChild() != null ? numericTerm_AST.getFirstChild() : numericTerm_AST;
                            currentAST.advanceChildToEnd();
                        }
                    } else {
                        break _loop307;
                    }

                } while (true);
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_111);
            } else {
                throw ex;
            }
        }
        returnAST = numericTerm_AST;
    }

    public final void additiveOperator() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST additiveOperator_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case PLUS: {
                    AST tmp266_AST = null;
                    tmp266_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp266_AST);
                    match(PLUS);
                    additiveOperator_AST = (AST) currentAST.root;
                    break;
                }
                case MINUS: {
                    AST tmp267_AST = null;
                    tmp267_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp267_AST);
                    match(MINUS);
                    additiveOperator_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_35);
            } else {
                throw ex;
            }
        }
        returnAST = additiveOperator_AST;
    }

    public final void numericFactor() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST numericFactor_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case LPAREN: {
                    match(LPAREN);
                    numericExpression();
                    astFactory.addASTChild(currentAST, returnAST);
                    match(RPAREN);
                    numericFactor_AST = (AST) currentAST.root;
                    break;
                }
                case BAR: {
                    absValue();
                    astFactory.addASTChild(currentAST, returnAST);
                    numericFactor_AST = (AST) currentAST.root;
                    break;
                }
                case SQRT_KYWD:
                case ABS_KYWD: {
                    numericUnaryOperation();
                    astFactory.addASTChild(currentAST, returnAST);
                    numericFactor_AST = (AST) currentAST.root;
                    break;
                }
                case INT: {
                    plexil.LiteralASTNode tmp270_AST = null;
                    tmp270_AST = (plexil.LiteralASTNode) astFactory.create(LT(1), "plexil.LiteralASTNode");
                    astFactory.addASTChild(currentAST, tmp270_AST);
                    match(INT);
                    numericFactor_AST = (AST) currentAST.root;
                    break;
                }
                case DOUBLE: {
                    plexil.LiteralASTNode tmp271_AST = null;
                    tmp271_AST = (plexil.LiteralASTNode) astFactory.create(LT(1), "plexil.LiteralASTNode");
                    astFactory.addASTChild(currentAST, tmp271_AST);
                    match(DOUBLE);
                    numericFactor_AST = (AST) currentAST.root;
                    break;
                }
                case LOOKUP_WITH_FREQ_KYWD:
                case LOOKUP_ON_CHANGE_KYWD:
                case LOOKUP_NOW_KYWD: {
                    lookup();
                    astFactory.addASTChild(currentAST, returnAST);
                    numericFactor_AST = (AST) currentAST.root;
                    break;
                }
                default:
                    if (((LA(1) == IDENT) && (LA(2) == LBRACKET || LA(2) == PERIOD) && (_tokenSet_30.member(LA(3)))) && (context.isIntegerArrayVariableName(LT(1).getText()))) {
                        integerArrayReference();
                        astFactory.addASTChild(currentAST, returnAST);
                        numericFactor_AST = (AST) currentAST.root;
                    } else if (((LA(1) == IDENT) && (LA(2) == LBRACKET || LA(2) == PERIOD) && (_tokenSet_30.member(LA(3)))) && (context.isRealArrayVariableName(LT(1).getText()))) {
                        realArrayReference();
                        astFactory.addASTChild(currentAST, returnAST);
                        numericFactor_AST = (AST) currentAST.root;
                    } else if (((LA(1) == IDENT) && (_tokenSet_112.member(LA(2))) && (_tokenSet_113.member(LA(3)))) && (context.isIntegerVariableName(LT(1).getText()))) {
                        integerVariable();
                        astFactory.addASTChild(currentAST, returnAST);
                        numericFactor_AST = (AST) currentAST.root;
                    } else if (((LA(1) == IDENT) && (_tokenSet_112.member(LA(2))) && (_tokenSet_113.member(LA(3)))) && (context.isRealVariableName(LT(1).getText()))) {
                        realVariable();
                        astFactory.addASTChild(currentAST, returnAST);
                        numericFactor_AST = (AST) currentAST.root;
                    } else if (((LA(1) == IDENT) && (LA(2) == PERIOD) && (_tokenSet_108.member(LA(3)))) && (!context.isVariableName(LT(1).getText()))) {
                        nodeTimepointValue();
                        astFactory.addASTChild(currentAST, returnAST);
                        numericFactor_AST = (AST) currentAST.root;
                    } else {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_52);
            } else {
                throw ex;
            }
        }
        returnAST = numericFactor_AST;
    }

    public final void multiplicativeOperator() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST multiplicativeOperator_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case ASTERISK: {
                    AST tmp272_AST = null;
                    tmp272_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp272_AST);
                    match(ASTERISK);
                    multiplicativeOperator_AST = (AST) currentAST.root;
                    break;
                }
                case SLASH: {
                    AST tmp273_AST = null;
                    tmp273_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp273_AST);
                    match(SLASH);
                    multiplicativeOperator_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_35);
            } else {
                throw ex;
            }
        }
        returnAST = multiplicativeOperator_AST;
    }

    public final void absValue() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST absValue_AST = null;
        AST e_AST = null;

        try {      // for error handling
            match(BAR);
            numericExpression();
            e_AST = (AST) returnAST;
            match(BAR);
            if (inputState.guessing == 0) {
                absValue_AST = (AST) currentAST.root;
                absValue_AST = (AST) astFactory.make((new ASTArray(2)).add(astFactory.create(ABS_KYWD, "abs")).add(e_AST));
                currentAST.root = absValue_AST;
                currentAST.child = absValue_AST != null && absValue_AST.getFirstChild() != null ? absValue_AST.getFirstChild() : absValue_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_52);
            } else {
                throw ex;
            }
        }
        returnAST = absValue_AST;
    }

    public final void numericUnaryOperation() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST numericUnaryOperation_AST = null;
        AST op_AST = null;
        AST exp_AST = null;

        try {      // for error handling
            numericUnaryOperator();
            op_AST = (AST) returnAST;
            match(LPAREN);
            numericExpression();
            exp_AST = (AST) returnAST;
            match(RPAREN);
            if (inputState.guessing == 0) {
                numericUnaryOperation_AST = (AST) currentAST.root;
                numericUnaryOperation_AST = (AST) astFactory.make((new ASTArray(2)).add(op_AST).add(exp_AST));
                currentAST.root = numericUnaryOperation_AST;
                currentAST.child = numericUnaryOperation_AST != null && numericUnaryOperation_AST.getFirstChild() != null ? numericUnaryOperation_AST.getFirstChild() : numericUnaryOperation_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_52);
            } else {
                throw ex;
            }
        }
        returnAST = numericUnaryOperation_AST;
    }

    public final void numericUnaryOperator() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST numericUnaryOperator_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case SQRT_KYWD: {
                    AST tmp278_AST = null;
                    tmp278_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp278_AST);
                    match(SQRT_KYWD);
                    numericUnaryOperator_AST = (AST) currentAST.root;
                    break;
                }
                case ABS_KYWD: {
                    AST tmp279_AST = null;
                    tmp279_AST = astFactory.create(LT(1));
                    astFactory.addASTChild(currentAST, tmp279_AST);
                    match(ABS_KYWD);
                    numericUnaryOperator_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_114);
            } else {
                throw ex;
            }
        }
        returnAST = numericUnaryOperator_AST;
    }

    public final void stringQuantity() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST stringQuantity_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case STRING: {
                    stringValue();
                    astFactory.addASTChild(currentAST, returnAST);
                    stringQuantity_AST = (AST) currentAST.root;
                    break;
                }
                case LOOKUP_WITH_FREQ_KYWD:
                case LOOKUP_ON_CHANGE_KYWD:
                case LOOKUP_NOW_KYWD: {
                    lookup();
                    astFactory.addASTChild(currentAST, returnAST);
                    stringQuantity_AST = (AST) currentAST.root;
                    break;
                }
                default:
                    if ((LA(1) == IDENT) && (LA(2) == LBRACKET || LA(2) == PERIOD) && (_tokenSet_30.member(LA(3)))) {
                        stringArrayReference();
                        astFactory.addASTChild(currentAST, returnAST);
                        stringQuantity_AST = (AST) currentAST.root;
                    } else if ((LA(1) == IDENT) && (_tokenSet_115.member(LA(2))) && (_tokenSet_116.member(LA(3)))) {
                        stringVariable();
                        astFactory.addASTChild(currentAST, returnAST);
                        stringQuantity_AST = (AST) currentAST.root;
                    } else {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_53);
            } else {
                throw ex;
            }
        }
        returnAST = stringQuantity_AST;
    }

    public final void lookupWithFrequency() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST lookupWithFrequency_AST = null;
        Token lwf = null;
        AST lwf_AST = null;
        AST fr_AST = null;
        AST sn_AST = null;
        AST args_AST = null;

        try {      // for error handling
            lwf = LT(1);
            lwf_AST = astFactory.create(lwf);
            match(LOOKUP_WITH_FREQ_KYWD);
            frequency();
            fr_AST = (AST) returnAST;
            match(LPAREN);
            stateName();
            sn_AST = (AST) returnAST;
            {
                switch (LA(1)) {
                    case LPAREN: {
                        match(LPAREN);
                        {
                            switch (LA(1)) {
                                case TRUE_KYWD:
                                case FALSE_KYWD:
                                case INT:
                                case DOUBLE:
                                case STRING:
                                case IDENT: {
                                    argumentList();
                                    args_AST = (AST) returnAST;
                                    break;
                                }
                                case RPAREN: {
                                    break;
                                }
                                default: {
                                    throw new NoViableAltException(LT(1), getFilename());
                                }
                            }
                        }
                        match(RPAREN);
                        break;
                    }
                    case RPAREN: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(RPAREN);
            if (inputState.guessing == 0) {
                lookupWithFrequency_AST = (AST) currentAST.root;

                lookupWithFrequency_AST = (AST) astFactory.make((new ASTArray(1)).add(lwf_AST));
                lookupWithFrequency_AST.addChild(sn_AST);
                lookupWithFrequency_AST.addChild(fr_AST);
                if (args_AST != null) {
                    lookupWithFrequency_AST.addChild(args_AST);
                }

                currentAST.root = lookupWithFrequency_AST;
                currentAST.child = lookupWithFrequency_AST != null && lookupWithFrequency_AST.getFirstChild() != null ? lookupWithFrequency_AST.getFirstChild() : lookupWithFrequency_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_52);
            } else {
                throw ex;
            }
        }
        returnAST = lookupWithFrequency_AST;
    }

    public final void lookupOnChange() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST lookupOnChange_AST = null;
        Token loc = null;
        AST loc_AST = null;
        AST sn_AST = null;
        AST args_AST = null;
        AST tol_AST = null;

        try {      // for error handling
            loc = LT(1);
            loc_AST = astFactory.create(loc);
            match(LOOKUP_ON_CHANGE_KYWD);
            match(LPAREN);
            stateName();
            sn_AST = (AST) returnAST;
            {
                switch (LA(1)) {
                    case LPAREN: {
                        match(LPAREN);
                        {
                            switch (LA(1)) {
                                case TRUE_KYWD:
                                case FALSE_KYWD:
                                case INT:
                                case DOUBLE:
                                case STRING:
                                case IDENT: {
                                    argumentList();
                                    args_AST = (AST) returnAST;
                                    break;
                                }
                                case RPAREN: {
                                    break;
                                }
                                default: {
                                    throw new NoViableAltException(LT(1), getFilename());
                                }
                            }
                        }
                        match(RPAREN);
                        break;
                    }
                    case COMMA:
                    case RPAREN: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            {
                switch (LA(1)) {
                    case COMMA: {
                        match(COMMA);
                        tolerance();
                        tol_AST = (AST) returnAST;
                        break;
                    }
                    case RPAREN: {
                        break;
                    }
                    default: {
                        throw new NoViableAltException(LT(1), getFilename());
                    }
                }
            }
            match(RPAREN);
            if (inputState.guessing == 0) {
                lookupOnChange_AST = (AST) currentAST.root;

                lookupOnChange_AST = (AST) astFactory.make((new ASTArray(1)).add(loc_AST));
                lookupOnChange_AST.addChild(sn_AST);
                if (tol_AST != null) {
                    lookupOnChange_AST.addChild(tol_AST);
                }
                if (args_AST != null) {
                    lookupOnChange_AST.addChild(args_AST);
                }

                currentAST.root = lookupOnChange_AST;
                currentAST.child = lookupOnChange_AST != null && lookupOnChange_AST.getFirstChild() != null ? lookupOnChange_AST.getFirstChild() : lookupOnChange_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_52);
            } else {
                throw ex;
            }
        }
        returnAST = lookupOnChange_AST;
    }

    public final void frequency() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST frequency_AST = null;
        AST lf_AST = null;
        AST hf_AST = null;

        try {      // for error handling
            match(LBRACKET);
            lowFreq();
            lf_AST = (AST) returnAST;
            match(COMMA);
            highFreq();
            hf_AST = (AST) returnAST;
            match(RBRACKET);
            if (inputState.guessing == 0) {
                frequency_AST = (AST) currentAST.root;
                frequency_AST = (AST) astFactory.make((new ASTArray(3)).add(astFactory.create(LOOKUP_FREQUENCY, "LOOKUP_FREQUENCY")).add(lf_AST).add(hf_AST));
                currentAST.root = frequency_AST;
                currentAST.child = frequency_AST != null && frequency_AST.getFirstChild() != null ? frequency_AST.getFirstChild() : frequency_AST;
                currentAST.advanceChildToEnd();
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_114);
            } else {
                throw ex;
            }
        }
        returnAST = frequency_AST;
    }

    public final void lowFreq() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST lowFreq_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case INT:
                case DOUBLE: {
                    realValue();
                    astFactory.addASTChild(currentAST, returnAST);
                    lowFreq_AST = (AST) currentAST.root;
                    break;
                }
                case IDENT: {
                    realVariable();
                    astFactory.addASTChild(currentAST, returnAST);
                    lowFreq_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_117);
            } else {
                throw ex;
            }
        }
        returnAST = lowFreq_AST;
    }

    public final void highFreq() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST highFreq_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case INT:
                case DOUBLE: {
                    realValue();
                    astFactory.addASTChild(currentAST, returnAST);
                    highFreq_AST = (AST) currentAST.root;
                    break;
                }
                case IDENT: {
                    realVariable();
                    astFactory.addASTChild(currentAST, returnAST);
                    highFreq_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_118);
            } else {
                throw ex;
            }
        }
        returnAST = highFreq_AST;
    }

    public final void tolerance() throws RecognitionException, TokenStreamException {

        returnAST = null;
        ASTPair currentAST = new ASTPair();
        AST tolerance_AST = null;

        try {      // for error handling
            switch (LA(1)) {
                case INT:
                case DOUBLE: {
                    realValue();
                    astFactory.addASTChild(currentAST, returnAST);
                    tolerance_AST = (AST) currentAST.root;
                    break;
                }
                case IDENT: {
                    variable();
                    astFactory.addASTChild(currentAST, returnAST);
                    tolerance_AST = (AST) currentAST.root;
                    break;
                }
                default: {
                    throw new NoViableAltException(LT(1), getFilename());
                }
            }
        } catch (RecognitionException ex) {
            if (inputState.guessing == 0) {
                reportError(ex);
                recover(ex, _tokenSet_66);
            } else {
                throw ex;
            }
        }
        returnAST = tolerance_AST;
    }
    public static final String[] _tokenNames = {
        "<0>",
        "EOF",
        "<2>",
        "NULL_TREE_LOOKAHEAD",
        "\"Comment\"",
        "\"Command\"",
        "\"Function\"",
        "\"Lookup\"",
        "\"LibraryNode\"",
        "\"returns\"",
        "\"Resource\"",
        "\"ResourcePriority\"",
        "\"Name\"",
        "\"UpperBound\"",
        "\"LowerBound\"",
        "\"ReleaseAtTermination\"",
        "\"Priority\"",
        "\"Permissions\"",
        "\"StartCondition\"",
        "\"RepeatCondition\"",
        "\"SkipCondition\"",
        "\"PreCondition\"",
        "\"PostCondition\"",
        "\"InvariantCondition\"",
        "\"EndCondition\"",
        "\"Start\"",
        "\"Repeat\"",
        "\"Skip\"",
        "\"Pre\"",
        "\"Post\"",
        "\"Invariant\"",
        "\"End\"",
        "\"In\"",
        "\"InOut\"",
        "\"Boolean\"",
        "\"Integer\"",
        "\"Real\"",
        "\"String\"",
        "\"BLOB\"",
        "\"Time\"",
        "\"true\"",
        "\"false\"",
        "\"NodeList\"",
        "\"FunctionCall\"",
        "\"Assignment\"",
        "\"Update\"",
        "\"Request\"",
        "\"LibraryCall\"",
        "\"OR\"",
        "\"AND\"",
        "\"NOT\"",
        "\"state\"",
        "\"outcome\"",
        "\"command_handle\"",
        "\"failure\"",
        "\"WAITING\"",
        "\"EXECUTING\"",
        "\"FINISHING\"",
        "\"FAILING\"",
        "\"FINISHED\"",
        "\"ITERATION_ENDED\"",
        "\"INACTIVE\"",
        "\"SUCCESS\"",
        "\"FAILURE\"",
        "\"SKIPPED\"",
        "\"INFINITE_LOOP\"",
        "\"COMMAND_SENT_TO_SYSTEM\"",
        "\"COMMAND_ACCEPTED\"",
        "\"COMMAND_DENIED\"",
        "\"COMMAND_RCVD_BY_SYSTEM\"",
        "\"COMMAND_FAILED\"",
        "\"COMMAND_SUCCESS\"",
        "\"PRE_CONDITION_FAILED\"",
        "\"POST_CONDITION_FAILED\"",
        "\"INVARIANT_CONDITION_FAILED\"",
        "\"PARENT_FAILED\"",
        "\"START\"",
        "\"END\"",
        "\"LookupWithFrequency\"",
        "\"LookupOnChange\"",
        "\"LookupNow\"",
        "\"sqrt\"",
        "\"abs\"",
        "\"isKnown\"",
        "INT",
        "DOUBLE",
        "STRING",
        "NCName",
        "NodeName",
        "PLEXIL",
        "NODE",
        "NODE_ID",
        "GLOBAL_DECLARATIONS",
        "PARAMETER_DECLARATIONS",
        "PARAMETER",
        "RETURN_VALUE",
        "VARIABLE",
        "VARIABLE_DECLARATION",
        "ALIASES",
        "CONST_ALIAS",
        "VARIABLE_ALIAS",
        "ARRAY_ASSIGNMENT",
        "BOOLEAN_ASSIGNMENT",
        "INTEGER_ASSIGNMENT",
        "REAL_ASSIGNMENT",
        "STRING_ASSIGNMENT",
        "TIME_ASSIGNMENT",
        "BLOB_ARRAY",
        "BOOLEAN_ARRAY",
        "INTEGER_ARRAY",
        "REAL_ARRAY",
        "STRING_ARRAY",
        "TIME_ARRAY",
        "ARRAY_REF",
        "BOOLEAN_ARRAY_LITERAL",
        "INTEGER_ARRAY_LITERAL",
        "REAL_ARRAY_LITERAL",
        "STRING_ARRAY_LITERAL",
        "TIME_ARRAY_LITERAL",
        "BOOLEAN_COMPARISON",
        "NODE_STATE_COMPARISON",
        "NODE_OUTCOME_COMPARISON",
        "NODE_COMMAND_HANDLE_COMPARISON",
        "NODE_FAILURE_COMPARISON",
        "NUMERIC_COMPARISON",
        "STRING_COMPARISON",
        "TIME_COMPARISON",
        "LOOKUP_FREQUENCY",
        "LOOKUP_STATE",
        "POINTS_TO",
        "TIME_VALUE",
        "NODE_STATE_VARIABLE",
        "NODE_OUTCOME_VARIABLE",
        "NODE_COMMAND_HANDLE_VARIABLE",
        "NODE_FAILURE_VARIABLE",
        "NODE_TIMEPOINT_VALUE",
        "ARGUMENT_LIST",
        "CONCAT",
        "SEMICOLON",
        "LPAREN",
        "COMMA",
        "RPAREN",
        "COLON",
        "LBRACE",
        "RBRACE",
        "EQUALS",
        "LBRACKET",
        "RBRACKET",
        "HASHPAREN",
        "OR_ALT_KYWD",
        "AND_ALT_KYWD",
        "DEQUALS",
        "NEQUALS",
        "BANG",
        "GREATER",
        "GEQ",
        "LESS",
        "LEQ",
        "PERIOD",
        "PLUS",
        "MINUS",
        "ASTERISK",
        "SLASH",
        "BAR",
        "IDENT",
        "DCOLON",
        "ESC",
        "UNICODE_ESC",
        "NUMERIC_ESC",
        "QUAD_DIGIT",
        "OCTAL_DIGIT",
        "DIGIT",
        "HEX_DIGIT",
        "EXPONENT",
        "FLOAT_SUFFIX",
        "whitespace, such as a space, tab, or newline",
        "a single line comment, delimited by //",
        "a multiple line comment, such as /* <comment> */"
    };

    protected void buildTokenTypeASTClassMap() {
        tokenTypeToASTClassMap = new Hashtable();
        tokenTypeToASTClassMap.put(new Integer(40), plexil.LiteralASTNode.class);
        tokenTypeToASTClassMap.put(new Integer(41), plexil.LiteralASTNode.class);
        tokenTypeToASTClassMap.put(new Integer(84), plexil.LiteralASTNode.class);
        tokenTypeToASTClassMap.put(new Integer(85), plexil.LiteralASTNode.class);
        tokenTypeToASTClassMap.put(new Integer(86), plexil.StringLiteralASTNode.class);
        tokenTypeToASTClassMap.put(new Integer(90), plexil.NodeASTNode.class);
        tokenTypeToASTClassMap.put(new Integer(96), plexil.VariableASTNode.class);
        tokenTypeToASTClassMap.put(new Integer(113), plexil.ArrayRefASTNode.class);
        tokenTypeToASTClassMap.put(new Integer(114), plexil.LiteralASTNode.class);
        tokenTypeToASTClassMap.put(new Integer(115), plexil.LiteralASTNode.class);
        tokenTypeToASTClassMap.put(new Integer(116), plexil.LiteralASTNode.class);
        tokenTypeToASTClassMap.put(new Integer(117), plexil.LiteralASTNode.class);
        tokenTypeToASTClassMap.put(new Integer(118), plexil.LiteralASTNode.class);
        tokenTypeToASTClassMap.put(new Integer(130), plexil.TimeLiteralASTNode.class);
    }
    ;

    private static final long[] mk_tokenSet_0() {
        long[] data = {2L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_0 = new BitSet(mk_tokenSet_0());

    private static final long[] mk_tokenSet_1() {
        long[] data = {1082331759072L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_1 = new BitSet(mk_tokenSet_1());

    private static final long[] mk_tokenSet_2() {
        long[] data = {0L, 0L, 68719509504L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_2 = new BitSet(mk_tokenSet_2());

    private static final long[] mk_tokenSet_3() {
        long[] data = {4294904832L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_3 = new BitSet(mk_tokenSet_3());

    private static final long[] mk_tokenSet_4() {
        long[] data = {2L, 0L, 68719575040L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_4 = new BitSet(mk_tokenSet_4());

    private static final long[] mk_tokenSet_5() {
        long[] data = {1082331758624L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_5 = new BitSet(mk_tokenSet_5());

    private static final long[] mk_tokenSet_6() {
        long[] data = {0L, 0L, 74088188928L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_6 = new BitSet(mk_tokenSet_6());

    private static final long[] mk_tokenSet_7() {
        long[] data = {1082331758656L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_7 = new BitSet(mk_tokenSet_7());

    private static final long[] mk_tokenSet_8() {
        long[] data = {1082331758720L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_8 = new BitSet(mk_tokenSet_8());

    private static final long[] mk_tokenSet_9() {
        long[] data = {512L, 0L, 74088187904L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_9 = new BitSet(mk_tokenSet_9());

    private static final long[] mk_tokenSet_10() {
        long[] data = {1082331759072L, 0L, 68719509504L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_10 = new BitSet(mk_tokenSet_10());

    private static final long[] mk_tokenSet_11() {
        long[] data = {224L, 0L, 68719490048L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_11 = new BitSet(mk_tokenSet_11());

    private static final long[] mk_tokenSet_12() {
        long[] data = {0L, 0L, 3072L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_12 = new BitSet(mk_tokenSet_12());

    private static final long[] mk_tokenSet_13() {
        long[] data = {512L, 0L, 1024L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_13 = new BitSet(mk_tokenSet_13());

    private static final long[] mk_tokenSet_14() {
        long[] data = {512L, 0L, 15360L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_14 = new BitSet(mk_tokenSet_14());

    private static final long[] mk_tokenSet_15() {
        long[] data = {0L, 0L, 1024L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_15 = new BitSet(mk_tokenSet_15());

    private static final long[] mk_tokenSet_16() {
        long[] data = {0L, 0L, 12288L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_16 = new BitSet(mk_tokenSet_16());

    private static final long[] mk_tokenSet_17() {
        long[] data = {0L, 0L, 13312L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_17 = new BitSet(mk_tokenSet_17());

    private static final long[] mk_tokenSet_18() {
        long[] data = {0L, 0L, 5120L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_18 = new BitSet(mk_tokenSet_18());

    private static final long[] mk_tokenSet_19() {
        long[] data = {844424930132482L, 0L, 137404234752L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_19 = new BitSet(mk_tokenSet_19());

    private static final long[] mk_tokenSet_20() {
        long[] data = {-34902897112125470L, 8376319L, 137437887488L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_20 = new BitSet(mk_tokenSet_20());

    private static final long[] mk_tokenSet_21() {
        long[] data = {844424930132482L, 0L, 67611016192L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_21 = new BitSet(mk_tokenSet_21());

    private static final long[] mk_tokenSet_22() {
        long[] data = {0L, 0L, 74088205312L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_22 = new BitSet(mk_tokenSet_22());

    private static final long[] mk_tokenSet_23() {
        long[] data = {4609435313830363616L, 0L, 74088442880L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_23 = new BitSet(mk_tokenSet_23());

    private static final long[] mk_tokenSet_24() {
        long[] data = {4610560118520483314L, 7405568L, 74120166400L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_24 = new BitSet(mk_tokenSet_24());

    private static final long[] mk_tokenSet_25() {
        long[] data = {0L, 0L, 69793238016L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_25 = new BitSet(mk_tokenSet_25());

    private static final long[] mk_tokenSet_26() {
        long[] data = {0L, 0L, 16384L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_26 = new BitSet(mk_tokenSet_26());

    private static final long[] mk_tokenSet_27() {
        long[] data = {278176441764896L, 0L, 65536L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_27 = new BitSet(mk_tokenSet_27());

    private static final long[] mk_tokenSet_28() {
        long[] data = {277081225104416L, 0L, 65536L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_28 = new BitSet(mk_tokenSet_28());

    private static final long[] mk_tokenSet_29() {
        long[] data = {0L, 0L, 65536L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_29 = new BitSet(mk_tokenSet_29());

    private static final long[] mk_tokenSet_30() {
        long[] data = {0L, 3653632L, 108448188416L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_30 = new BitSet(mk_tokenSet_30());

    private static final long[] mk_tokenSet_31() {
        long[] data = {-36025498484080640L, 8376319L, 108481873920L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_31 = new BitSet(mk_tokenSet_31());

    private static final long[] mk_tokenSet_32() {
        long[] data = {-36025498484080640L, 8376319L, 103113033728L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_32 = new BitSet(mk_tokenSet_32());

    private static final long[] mk_tokenSet_33() {
        long[] data = {-35181073553948672L, 8376319L, 137437121536L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_33 = new BitSet(mk_tokenSet_33());

    private static final long[] mk_tokenSet_34() {
        long[] data = {-1404076348669952L, 8376319L, 137437199360L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_34 = new BitSet(mk_tokenSet_34());

    private static final long[] mk_tokenSet_35() {
        long[] data = {0L, 3653632L, 103079217152L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_35 = new BitSet(mk_tokenSet_35());

    private static final long[] mk_tokenSet_36() {
        long[] data = {0L, 3653632L, 136365476864L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_36 = new BitSet(mk_tokenSet_36());

    private static final long[] mk_tokenSet_37() {
        long[] data = {4575657221408423936L, 3653632L, 136365550592L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_37 = new BitSet(mk_tokenSet_37());

    private static final long[] mk_tokenSet_38() {
        long[] data = {0L, 4308992L, 68719476736L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_38 = new BitSet(mk_tokenSet_38());

    private static final long[] mk_tokenSet_39() {
        long[] data = {0L, 0L, 7516457984L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_39 = new BitSet(mk_tokenSet_39());

    private static final long[] mk_tokenSet_40() {
        long[] data = {0L, 7847936L, 110595738624L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_40 = new BitSet(mk_tokenSet_40());

    private static final long[] mk_tokenSet_41() {
        long[] data = {0L, 1163264L, 68719738880L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_41 = new BitSet(mk_tokenSet_41());

    private static final long[] mk_tokenSet_42() {
        long[] data = {0L, 1048576L, 5368974336L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_42 = new BitSet(mk_tokenSet_42());

    private static final long[] mk_tokenSet_43() {
        long[] data = {0L, 3653632L, 108448259072L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_43 = new BitSet(mk_tokenSet_43());

    private static final long[] mk_tokenSet_44() {
        long[] data = {0L, 0L, 74088316928L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_44 = new BitSet(mk_tokenSet_44());

    private static final long[] mk_tokenSet_45() {
        long[] data = {0L, 0L, 5368712192L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_45 = new BitSet(mk_tokenSet_45());

    private static final long[] mk_tokenSet_46() {
        long[] data = {3298534883328L, 7340032L, 74088262656L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_46 = new BitSet(mk_tokenSet_46());

    private static final long[] mk_tokenSet_47() {
        long[] data = {3298534883328L, 7340032L, 74088196096L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_47 = new BitSet(mk_tokenSet_47());

    private static final long[] mk_tokenSet_48() {
        long[] data = {844424930131968L, 0L, 31470592L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_48 = new BitSet(mk_tokenSet_48());

    private static final long[] mk_tokenSet_49() {
        long[] data = {844424930131968L, 0L, 35398366208L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_49 = new BitSet(mk_tokenSet_49());

    private static final long[] mk_tokenSet_50() {
        long[] data = {0L, 0L, 525312L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_50 = new BitSet(mk_tokenSet_50());

    private static final long[] mk_tokenSet_51() {
        long[] data = {844424930131970L, 0L, 67611014144L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_51 = new BitSet(mk_tokenSet_51());

    private static final long[] mk_tokenSet_52() {
        long[] data = {844424930131968L, 0L, 67610620928L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_52 = new BitSet(mk_tokenSet_52());

    private static final long[] mk_tokenSet_53() {
        long[] data = {844424930131968L, 0L, 2178954240L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_53 = new BitSet(mk_tokenSet_53());

    private static final long[] mk_tokenSet_54() {
        long[] data = {0L, 0L, 5368845312L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_54 = new BitSet(mk_tokenSet_54());

    private static final long[] mk_tokenSet_55() {
        long[] data = {281474976648224L, 1048576L, 74088387584L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_55 = new BitSet(mk_tokenSet_55());

    private static final long[] mk_tokenSet_56() {
        long[] data = {0L, 1048576L, 74088448000L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_56 = new BitSet(mk_tokenSet_56());

    private static final long[] mk_tokenSet_57() {
        long[] data = {0L, 0L, 398336L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_57 = new BitSet(mk_tokenSet_57());

    private static final long[] mk_tokenSet_58() {
        long[] data = {847723465015296L, 1048576L, 31470592L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_58 = new BitSet(mk_tokenSet_58());

    private static final long[] mk_tokenSet_59() {
        long[] data = {278176441764896L, 1048576L, 74088387584L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_59 = new BitSet(mk_tokenSet_59());

    private static final long[] mk_tokenSet_60() {
        long[] data = {0L, 1048576L, 537600L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_60 = new BitSet(mk_tokenSet_60());

    private static final long[] mk_tokenSet_61() {
        long[] data = {278176441764896L, 3145728L, 74088387584L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_61 = new BitSet(mk_tokenSet_61());

    private static final long[] mk_tokenSet_62() {
        long[] data = {0L, 3145728L, 537600L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_62 = new BitSet(mk_tokenSet_62());

    private static final long[] mk_tokenSet_63() {
        long[] data = {278176441764896L, 4194304L, 74088387584L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_63 = new BitSet(mk_tokenSet_63());

    private static final long[] mk_tokenSet_64() {
        long[] data = {844424930131968L, 4194304L, 2178954240L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_64 = new BitSet(mk_tokenSet_64());

    private static final long[] mk_tokenSet_65() {
        long[] data = {278176441764896L, 0L, 74088387584L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_65 = new BitSet(mk_tokenSet_65());

    private static final long[] mk_tokenSet_66() {
        long[] data = {0L, 0L, 8192L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_66 = new BitSet(mk_tokenSet_66());

    private static final long[] mk_tokenSet_67() {
        long[] data = {278176441764896L, 1048576L, 74088649728L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_67 = new BitSet(mk_tokenSet_67());

    private static final long[] mk_tokenSet_68() {
        long[] data = {844424930131968L, 1048576L, 31732736L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_68 = new BitSet(mk_tokenSet_68());

    private static final long[] mk_tokenSet_69() {
        long[] data = {844424930131970L, 0L, 67610752000L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_69 = new BitSet(mk_tokenSet_69());

    private static final long[] mk_tokenSet_70() {
        long[] data = {3298534883328L, 7340032L, 68719490048L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_70 = new BitSet(mk_tokenSet_70());

    private static final long[] mk_tokenSet_71() {
        long[] data = {0L, 0L, 5368721408L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_71 = new BitSet(mk_tokenSet_71());

    private static final long[] mk_tokenSet_72() {
        long[] data = {3298534883328L, 7340032L, 74088199168L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_72 = new BitSet(mk_tokenSet_72());

    private static final long[] mk_tokenSet_73() {
        long[] data = {844424930131968L, 0L, 5400179712L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_73 = new BitSet(mk_tokenSet_73());

    private static final long[] mk_tokenSet_74() {
        long[] data = {-34903992328786912L, 8376319L, 108513278976L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_74 = new BitSet(mk_tokenSet_74());

    private static final long[] mk_tokenSet_75() {
        long[] data = {0L, 0L, 5368714240L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_75 = new BitSet(mk_tokenSet_75());

    private static final long[] mk_tokenSet_76() {
        long[] data = {0L, 0L, 74088256512L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_76 = new BitSet(mk_tokenSet_76());

    private static final long[] mk_tokenSet_77() {
        long[] data = {0L, 1048576L, 13312L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_77 = new BitSet(mk_tokenSet_77());

    private static final long[] mk_tokenSet_78() {
        long[] data = {0L, 0L, 68719547392L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_78 = new BitSet(mk_tokenSet_78());

    private static final long[] mk_tokenSet_79() {
        long[] data = {3298534883328L, 7340032L, 74088579072L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_79 = new BitSet(mk_tokenSet_79());

    private static final long[] mk_tokenSet_80() {
        long[] data = {0L, 0L, 131072L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_80 = new BitSet(mk_tokenSet_80());

    private static final long[] mk_tokenSet_81() {
        long[] data = {281474976710656L, 0L, 2110464L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_81 = new BitSet(mk_tokenSet_81());

    private static final long[] mk_tokenSet_82() {
        long[] data = {3298534883328L, 4177920L, 103112771584L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_82 = new BitSet(mk_tokenSet_82());

    private static final long[] mk_tokenSet_83() {
        long[] data = {-35181073553948672L, 8376319L, 137437133824L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_83 = new BitSet(mk_tokenSet_83());

    private static final long[] mk_tokenSet_84() {
        long[] data = {-34903992328786912L, 8376319L, 137437199360L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_84 = new BitSet(mk_tokenSet_84());

    private static final long[] mk_tokenSet_85() {
        long[] data = {0L, 3653632L, 136390641664L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_85 = new BitSet(mk_tokenSet_85());

    private static final long[] mk_tokenSet_86() {
        long[] data = {4575657221408423936L, 3653632L, 136390649856L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_86 = new BitSet(mk_tokenSet_86());

    private static final long[] mk_tokenSet_87() {
        long[] data = {0L, 0L, 7541622784L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_87 = new BitSet(mk_tokenSet_87());

    private static final long[] mk_tokenSet_88() {
        long[] data = {0L, 7847936L, 110620837888L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_88 = new BitSet(mk_tokenSet_88());

    private static final long[] mk_tokenSet_89() {
        long[] data = {0L, 1048576L, 5394139136L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_89 = new BitSet(mk_tokenSet_89());

    private static final long[] mk_tokenSet_90() {
        long[] data = {0L, 3653632L, 108473358336L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_90 = new BitSet(mk_tokenSet_90());

    private static final long[] mk_tokenSet_91() {
        long[] data = {4575657221408423936L, 0L, 68719476736L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_91 = new BitSet(mk_tokenSet_91());

    private static final long[] mk_tokenSet_92() {
        long[] data = {0L, 0L, 5393874944L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_92 = new BitSet(mk_tokenSet_92());

    private static final long[] mk_tokenSet_93() {
        long[] data = {4577909021222109184L, 0L, 74088185856L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_93 = new BitSet(mk_tokenSet_93());

    private static final long[] mk_tokenSet_94() {
        long[] data = {-4611686018427387904L, 3L, 68719476736L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_94 = new BitSet(mk_tokenSet_94());

    private static final long[] mk_tokenSet_95() {
        long[] data = {-4607182418800017408L, 3L, 74088185856L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_95 = new BitSet(mk_tokenSet_95());

    private static final long[] mk_tokenSet_96() {
        long[] data = {0L, 252L, 68719476736L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_96 = new BitSet(mk_tokenSet_96());

    private static final long[] mk_tokenSet_97() {
        long[] data = {9007199254740992L, 252L, 74088185856L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_97 = new BitSet(mk_tokenSet_97());

    private static final long[] mk_tokenSet_98() {
        long[] data = {0L, 3906L, 68719476736L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_98 = new BitSet(mk_tokenSet_98());

    private static final long[] mk_tokenSet_99() {
        long[] data = {18014398509481984L, 3906L, 74088185856L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_99 = new BitSet(mk_tokenSet_99());

    private static final long[] mk_tokenSet_100() {
        long[] data = {844424930131968L, 0L, 6304768L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_100 = new BitSet(mk_tokenSet_100());

    private static final long[] mk_tokenSet_101() {
        long[] data = {0L, 3653632L, 137372108800L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_101 = new BitSet(mk_tokenSet_101());

    private static final long[] mk_tokenSet_102() {
        long[] data = {4575657221408423936L, 3653632L, 137372116992L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_102 = new BitSet(mk_tokenSet_102());

    private static final long[] mk_tokenSet_103() {
        long[] data = {3298534883328L, 1687552L, 68719478784L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_103 = new BitSet(mk_tokenSet_103());

    private static final long[] mk_tokenSet_104() {
        long[] data = {-35181073553948672L, 8376319L, 108513213440L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_104 = new BitSet(mk_tokenSet_104());

    private static final long[] mk_tokenSet_105() {
        long[] data = {844424930131968L, 0L, 74119656448L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_105 = new BitSet(mk_tokenSet_105());

    private static final long[] mk_tokenSet_106() {
        long[] data = {2251799813685248L, 0L, 74088185856L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_106 = new BitSet(mk_tokenSet_106());

    private static final long[] mk_tokenSet_107() {
        long[] data = {4503599627370496L, 0L, 74088185856L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_107 = new BitSet(mk_tokenSet_107());

    private static final long[] mk_tokenSet_108() {
        long[] data = {4575657221408423936L, 0L, 74088185856L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_108 = new BitSet(mk_tokenSet_108());

    private static final long[] mk_tokenSet_109() {
        long[] data = {844424930131968L, 0L, 1105212416L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_109 = new BitSet(mk_tokenSet_109());

    private static final long[] mk_tokenSet_110() {
        long[] data = {0L, 0L, 1073741824L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_110 = new BitSet(mk_tokenSet_110());

    private static final long[] mk_tokenSet_111() {
        long[] data = {844424930131968L, 0L, 41840817152L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_111 = new BitSet(mk_tokenSet_111());

    private static final long[] mk_tokenSet_112() {
        long[] data = {844424930131968L, 0L, 68684362752L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_112 = new BitSet(mk_tokenSet_112());

    private static final long[] mk_tokenSet_113() {
        long[] data = {-34903992328786910L, 8376319L, 137437854720L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_113 = new BitSet(mk_tokenSet_113());

    private static final long[] mk_tokenSet_114() {
        long[] data = {0L, 0L, 2048L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_114 = new BitSet(mk_tokenSet_114());

    private static final long[] mk_tokenSet_115() {
        long[] data = {844424930131968L, 0L, 7547663360L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_115 = new BitSet(mk_tokenSet_115());

    private static final long[] mk_tokenSet_116() {
        long[] data = {-34903992328786912L, 8376319L, 110660762624L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_116 = new BitSet(mk_tokenSet_116());

    private static final long[] mk_tokenSet_117() {
        long[] data = {0L, 0L, 4096L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_117 = new BitSet(mk_tokenSet_117());

    private static final long[] mk_tokenSet_118() {
        long[] data = {0L, 0L, 524288L, 0L, 0L, 0L};
        return data;
    }
    public static final BitSet _tokenSet_118 = new BitSet(mk_tokenSet_118());
}
