/* Copyright (c) 2006-2015, Universities Space Research Association (USRA).
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

package gov.nasa.luv;

import java.io.InterruptedIOException;
import org.xml.sax.Attributes;
import java.util.Stack;
import static gov.nasa.luv.Constants.*;

 /** 
  * The PlexilPlanHandler class provide methods for processing and translating a
  * SAX Plexil Plan written in XML. It Collects condition, local variable and
  * action information to be available for display in the Luv applcaition.
  */

public class PlexilPlanHandler
    extends AbstractDispatchableHandler {

    // Clients are expected to implement this class to receive the plan or library
    // when loading is complete.

    public interface PlanReceiver {
        public void newPlan(Model m);
        public void newLibrary(Model m);
    }

    //
    // Local constants
    //
          
    /** Represents the XML tag indicating the loaded file is a Plexil Script. */
    private static final String PLEXIL_SCRIPT     = "PLEXILScript";
    /** Represents the XML tag indicating the loaded file is a Plexil Library. */
    private static final String LIBRARY           = "Library";   
    /** Represents the XML tag in the Plexil Plan marking the Plexil Plan file name information. */
    private static final String PLAN_FILENAME = "PlanFilename";
    /** Represents the XML tag in the Plexil Plan marking the Plexil Script file name information. */
    private static final String SCRIPT_FILENAME = "ScriptFilename";

    /** Represents the DeclareVariable XML tag in the Plexil Plan. */
    private static final String DECL_VAR = "DeclareVariable";
    /** Represents the XML tag marking an array declaration indicator in the Plexil Plan. */
    private static final String DECL_ARRAY = "DeclareArray";

    /** A collection of all the possible XML tags that represent Plexil Plan properties. */
    private static final String[] PROPERTY_TAGS = 
    {
        NODE_ID,
        NODE_TYPE,
        NODE_STATE,
        NODE_OUTCOME,
        NODE_FAILURE_TYPE,

        VIEWER_BLOCKS,
        PLAN_FILENAME,
        SCRIPT_FILENAME,

        PLEXIL_PLAN,
        PLEXIL_SCRIPT,
        LIBRARY,

        NODETYPE_ATTR,
        NODE_ID,
        NODE_BODY,
        NODELIST,
        NAME,
        TYPE,
        MAXSIZE,
        INDEX,
        INT_VAL,
        REAL_VAL,
        STRING_VAL,
        BOOL_VAL,
        DECL_VAR,
        DECL_ARRAY,
        ARRAYELEMENT,
    };

    /** Represents the Resource XML tag in the Plexil Plan. */
    private static final String RESOURCE = "Resource";
    /** Represents the ResourceName XML tag in the Plexil Plan. */
    private static final String RESOURCE_NAME = "ResourceName";
    /** Represents the ResourcePriority XML tag in the Plexil Plan. */
    private static final String RESOURCE_PRIORITY = "ResourcePriority";
    /** Represents the ResourceLowerBound XML tag in the Plexil Plan. */
    private static final String RESOURCE_LOWER_BOUND = "ResourceLowerBound";
    /** Represents the ResourceUpperBound XML tag in the Plexil Plan. */
    private static final String RESOURCE_UPPER_BOUND = "ResourceUpperBound";
    /** Represents the ResourceReleaseAtTermination XML tag in the Plexil Plan. */
    private static final String RESOURCE_RELEASE_AT_TERM = "ResourceReleaseAtTermination";
    
    /** A collection of all the possible XML tags that represent Resource information. */  
    private static final String[] RESOURCE_TAGS =
    {
        RESOURCE_NAME,
        RESOURCE_PRIORITY, 
        RESOURCE_LOWER_BOUND, 
        RESOURCE_UPPER_BOUND,
        RESOURCE_RELEASE_AT_TERM,
    };

    /** Represents the Node Outcome Value XML tag in the Plexil Plan. */
    private static final String NODE_OUTCOME_VAL = NODE_OUTCOME + "Value";
    /** Represents the Node Failure Value XML tag in the Plexil Plan. */
    private static final String NODE_FAILURE_VAL = NODE_FAILURE + "Value";
    /** Represents the Node State Value XML tag in the Plexil Plan. */
    private static final String NODE_STATE_VAL = NODE_STATE + "Value";
    /** Represents the Node Command Handle Value XML tag in the Plexil Plan. */
    private static final String NODE_CMD_HANDLE_VAL = NODE_CMD_HANDLE + "Value";
    /** Represents the Node Timepoint Value XML tag in the Plexil Plan. */
    private static final String NODE_TIMEPOINT_VAL = NODE_TIMEPOINT + "Value";

    /** A collection of all the possible XML tags that represent Node State values. */   
    private static final String[] NODE_STATE_TAGS =
    {
        NODE_OUTCOME_VAL, 
        NODE_FAILURE_VAL, 
        NODE_STATE_VAL,  
        NODE_CMD_HANDLE_VAL,
    };

    /** Represents the ADD (addition) operator XML tag in the Plexil Plan. */
    private static final String ADD = "ADD";
    /** Represents the SUB (subtraction) operator XML tag in the Plexil Plan. */
    private static final String SUB = "SUB";
    /** Represents the MUL (multiplication) operator XML tag in the Plexil Plan. */
    private static final String MUL = "MUL";
    /** Represents the DIV (division) operator XML tag in the Plexil Plan. */
    private static final String DIV = "DIV";
    /** Represents the SQRT (square root) operator XML tag in the Plexil Plan. */
    private static final String SQRT = "SQRT";
    /** Represents the ABS (absolute) operator XML tag in the Plexil Plan. */
    private static final String ABS = "ABS";

    /** A collection of all the possible XML tags that represent mathematic operators. */  
    private static final String[] MATH_TAGS =
    {
        ADD,
        SUB, 
        MUL, 
        DIV,
        SQRT,
        ABS
    };

    /** Represents the EQ (equals) XML tag in the Plexil Plan. */
    private static final String EQ = "EQ";
    /** Represents the EQNumeric (numeric equals) XML tag in the Plexil Plan. */
    private static final String EQ_NUMERIC = "EQNumeric";
    /** Represents the EQString (string equals) XML tag in the Plexil Plan. */
    private static final String EQ_STRING = "EQString";
    /** Represents the EQBoolean (boolean equals) XML tag in the Plexil Plan. */
    private static final String EQ_BOOLEAN = "EQBoolean";
    /** Represents the EQBoolean (boolean equals) XML tag in the Plexil Plan. */
    private static final String EQ_TIME ="EQTime";
    /** Represents the EQInternal (internal equals) XML tag in the Plexil Plan. */
    private static final String EQ_INTERNAL = "EQInternal";
    /** Represents the NE (not equals) XML tag in the Plexil Plan. */
    private static final String NE = "NE";
    /** Represents the NENumeric (numeric not equals) XML tag in the Plexil Plan. */
    private static final String NE_NUMERIC = "NENumeric";
    /** Represents the NEString (string not equals) XML tag in the Plexil Plan. */
    private static final String NE_STRING = "NEString";
    /** Represents the NEBoolean (boolean not equals) XML tag in the Plexil Plan. */
    private static final String NE_BOOLEAN = "NEBoolean";
    /** Represents the NEInternal (internal not equals) XML tag in the Plexil Plan. */
    private static final String NE_INTERNAL = "NEInternal";
    /** Represents the LT (less than) operator XML tag in the Plexil Plan. */
    private static final String LT = "LT";
    /** Represents the LE (less than or equal) operator XML tag in the Plexil Plan. */
    private static final String LE = "LE";
    /** Represents the GT (greater than) operator XML tag in the Plexil Plan. */
    private static final String GT = "GT";
    /** Represents the GE (greater than or equal) operator XML tag in the Plexil Plan. */
    private static final String GE = "GE";
    
    /** A collection of all the possible XML tags that represent comparison operators. */  
    private static final String[] COMPARISON_TAGS =
    {
        LT,
        GT,
        LE,
        GE, 
        EQ, 
        EQ_NUMERIC,
        EQ_STRING, 
        EQ_BOOLEAN,
        EQ_TIME,
        EQ_INTERNAL,
        NE,
        NE_NUMERIC,
        NE_STRING,
        NE_BOOLEAN, 
        NE_INTERNAL,
    };

    /** Represents the LookupNow XML tag in the Plexil Plan. */
    private static final String LOOKUPNOW = "LookupNow";
    /** Represents the LookupOnChange XML tag in the Plexil Plan. */
    private static final String LOOKUPCHANGE = "LookupOnChange";
    
    /** A collection of all the possible XML tags that represent Plexil Plan Lookups. */  
    private static final String[] LOOKUP_TAGS =
    {
        LOOKUPNOW,
        LOOKUPCHANGE, 
    };

    /** Represents the RHS XML tag in the Plexil Plan. */
    private static final String RHS = "RHS";
    /** Represents the NumericRHS XML tag in the Plexil Plan. */
    private static final String NUMERIC_RHS = "NumericRHS";
    /** Represents the StringRHS XML tag in the Plexil Plan. */
    private static final String STRING_RHS = "StringRHS";
    /** Represents the BooleanRHS XML tag in the Plexil Plan. */
    private static final String BOOLEAN_RHS = "BooleanRHS";
    /** Represents the LookupRHS XML tag in the Plexil Plan. */
    private static final String LOOKUP_RHS = "LookupRHS";
    /** Represents the ArrayRHS XML tag in the Plexil Plan. */
    private static final String ARRAY_RHS = "ArrayRHS";
    
    /** A collection of all the possible XML tags that represent Returns. */  
    private static final String[] RETURN_TAGS =
    {
        RHS,
        NUMERIC_RHS, 
        STRING_RHS, 
        TIME_RHS,
        BOOLEAN_RHS,
        LOOKUP_RHS,
        ARRAY_RHS,
    };
      
    /** A collection of all the possible XML tags that represent Node Actions (Assignement, Command, etc). */  
    private static final String[] ACTION_TAGS = 
    {
        ASSN,
        COMMAND,
        UPDATE,
    };
      
    /** A collection of all the possible XML tags that represent variable declarations. */  
    private static final String[] VARIABLE_DECL_TAGS =
    {
        DECL_VAR,
        DECL_ARRAY,
    };

    private static int row_number;   

    //
    // Instance variables
    //

    // variables and flags for collecting condition, local variable 
    // and action information
    private Stack<String> nodeInfoHolder;
    private boolean recordIN;
    private boolean recordINOUT;
    private boolean properName;
    // flags for library information
    private boolean libraryNodeCall;
    private boolean loadingLibrary;
    private Stack<Node> nodeHolder;
    private Model topLevelNode;              

    private PlanReceiver receiver;
    
    /**
     * Constructs a PlexilPlanHandler.
     */
    public PlexilPlanHandler(PlanReceiver r)
    {
        super();
        
        row_number = 0;
        nodeInfoHolder = new Stack<String>();    
        nodeHolder = new Stack<Node>();
        nodeHolder.push(Model.getRoot());
        receiver = r;
    }
    
    /**
     * Resets row number to zero which indicates that a new Plexil Plan or Library
     * is being processed.
     */
    public static void resetRowNumber()
    {
        row_number = 0;
    }

    /** {@inheritDoc} */
    public void startDocument()
    {
        recordIN = recordINOUT = false;
        properName = false; 
        libraryNodeCall = false;
        loadingLibrary = false;
        topLevelNode = null;
    }

    /**
     * Handles the start of an XML element. Watches for XML tags that might represent
     * whether or not it is for a Plexil Model Node or Library or a property of a Plexil Model Node.
     * 
     * @param uri
     * @param tagName
     * @param qName
     * @param attributes
     */
    public void startElement(String uri, String tagName, String qName, Attributes attributes) {    
        // get the current node in the stack
        Node node = nodeHolder.peek();

        // error if loaded a script instead of plan
        if (tagName.equals(PLEXIL_SCRIPT)) {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(null, "ERROR: loaded script instead of plan");           
        }

        // if this SHOULD be a child node, make that happen
        else if (tagName.equals(NODE)) {
            node = startNode(node, tagName, attributes);
        }

        // starting to load a library def'n?
        else if (tagName.equals(PLEXIL_LIBRARY)) {
            loadingLibrary = true;
        }

        // if it's not a property we can ignore it
        else if (!isProperty(tagName)) {
            node = null;
        }

        // catch condition, local variable and action information
        catchStartTag(tagName);

        // push new node onto the stack
        nodeHolder.push(node);
    }

    /**
     * Handles the end of an XML element. Gathers the text in between the start
     * and end tag that could be for conditions, local variables or actions.
     * 
     * @param uri
     * @param tagName
     * @param qName
     */
    public void endElement(String uri, String tagName, String qName) {
        Node topNode = nodeHolder.peek();

        // get tweener text and put it in its place
        String text = getTweenerText();

        try {
            // catch condition, local variable and action information
            catchTweenerText(tagName, text);
        } 
        catch (InterruptedIOException ex) {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(ex, "ERROR: exception occurred while locating library");
        }

        // catch condition, local variable and action information
        catchEndTag(tagName);

        // assign node name and path to the appropriate node
        if (topNode != null && text != null) {
            topNode.setProperty(tagName, text);
            if (tagName.equals(NODE_ID))
                topNode.setNodeName(text);
        }

        // if node, assign the main attributes for display in luv viewer
        // name, type, state and outcome
        if (topNode != null && tagName.equals(topNode.getType()))
            topNode.setMainAttributesOfNode();

        // pop the node off the stack
        nodeHolder.pop();        
    }

    /** {@inheritDoc} */
    public void endDocument() {       
        // Notify client that plan or library is complete when appropriate
        if (receiver != null) {
            if (loadingLibrary)
                receiver.newLibrary(topLevelNode);
            else 
                receiver.newPlan(topLevelNode);
        }
    }

    private Node startNode(Node node, String tagName, Attributes attributes) {
        String nodeTypeAttr = attributes.getValue(NODETYPE_ATTR);
        if (node == Model.getRoot()) {
            // Starting a new plan
            topLevelNode = new Model(nodeTypeAttr, row_number++);
            node.addChild(topLevelNode);
            node = topLevelNode;
        }
        else {
            Node child = new Node(nodeTypeAttr, row_number++);
            node.addChild(child);
            node = child;
        }

        // add attributes for this node
        // NODETYPE_ATTR identify which icon should be used when displaying plan
        // AUX is nodes that should be hidden if in EPX view
        for (int i = 0; i < attributes.getLength(); ++i) {
            String qname = attributes.getQName(i);
            String value = attributes.getValue(i);
            if (qname.equals(EPX)) {
                // save plexil core view icon
                String nodeType = node.getProperty(NODETYPE_ATTR);
                if (nodeType.contains(THEN) || nodeType.contains(ELSE))
                    node.setProperty(NODETYPE_ATTR_PLX, nodeType.substring(THEN.length())); // ???
                else
                    node.setProperty(NODETYPE_ATTR_PLX, nodeType);

                // insert epx view icon
                if (value.equals(THEN))
                    node.setProperty(NODETYPE_ATTR, AUX_THEN);
                else if (value.equals(ELSE))
                    node.setProperty(NODETYPE_ATTR, AUX_ELSE);
                else if (node.getParent().getProperty(NODETYPE_ATTR) != null &&
                         node.getParent().getProperty(NODETYPE_ATTR).equals(AUX_THEN)) 
                    node.setProperty(NODETYPE_ATTR, THEN + value);                    
                else if (node.getParent().getProperty(NODETYPE_ATTR) != null &&
                         node.getParent().getProperty(NODETYPE_ATTR).equals(AUX_ELSE)) 
                    node.setProperty(NODETYPE_ATTR, ELSE + value);
                else 
                    node.setProperty(NODETYPE_ATTR, value);
                    
            }
            else {
                String parentType = node.getParent().getProperty(NODETYPE_ATTR);
                if (parentType != null &&
                    parentType.equals(AUX_THEN)) {
                    node.setProperty(qname, THEN + value);
                    node.setProperty(NODETYPE_ATTR_PLX, value);
                }
                else if (parentType != null &&
                         parentType.equals(AUX_ELSE)) {
                    node.setProperty(qname, ELSE + value);
                    node.setProperty(NODETYPE_ATTR_PLX, value);
                }
                else if (qname.equals(NODETYPE_ATTR)) {
                    node.setProperty(qname, value);   
                    node.setProperty(NODETYPE_ATTR_PLX, value);
                }
                else
                    node.setProperty(qname, value);                
            }
        }
        return node;
    }
    
    /**
     * If the start tag refers to a library, condition, local variable or action
     * this class will know how to store any following information. 
     * @param tagName the definition of an XML tag
     */
    private void catchStartTag(String tagName) {
        if (tagName.equals(LIBRARYNODECALL)) {
            libraryNodeCall = true;  
            nodeInfoHolder.clear();
        }
        else if (isCondition(tagName) || isAction(tagName) || isVariableDeclaration(tagName)) {
            nodeInfoHolder.clear();
        }
        recordStartTag(tagName);
    }

    /**
     * If the specified text is not null, it will store the text appropriately
     * based on what kinf XML tag it belongs to.
     * 
     * @param tagName the type of XML tag
     * @param text the text in between the start and end XML tags
     * @throws java.io.InterruptedIOException
     */
    private void catchTweenerText(String tagName, String text) throws InterruptedIOException
    {
        if (text != null) {            
            if (libraryNodeCall) {
                currentNode().setLibraryName(text);
                currentNode().setUnresolvedLibraryCall(true);
                FileHandler fh = Luv.getLuv().getFileHandler();
                boolean askAboutMissingLibs = !loadingLibrary && !fh.getStopSearchForMissingLibs();
                Model library = fh.searchForLibrary(text, askAboutMissingLibs); // checks if file has been updated

                if (library == null) 
                    topLevelNode.addMissingLibrary(text);
                else {
                    row_number = currentNode().getRowNumber();
                    library.setRowNumber(++row_number);
                    row_number++;
                    currentNode().linkLibrary(library);
                }
                libraryNodeCall = false;                
            }
            
            // if data is a string object and not a proper name, surround with quotes
            if (tagName.equals(STRING_VAL) && !properName && !text.equals(UNKNOWN))
                text = "\"" + text + "\"";

            nodeInfoHolder.push(text);
        }
    }

    /**
     * Waits for and stores the end XML tag.
     * 
     * @param tagName the XML tag
     */
    private void catchEndTag(String tagName) {  
        recordEndTag(tagName);
    }

    /**
     * Based on the specfied start XML tag, if it refers to condition, local variable 
     * or action information - it will be stored appropriately.
     * 
     * @param tagName the XML tag
     */
    private void recordStartTag(String tagName)
    {
        if (tagName.equals(ARGS) || tagName.equals(AND) || tagName.equals(OR) || tagName.equals(CONCAT)) {
            nodeInfoHolder.push(tagName);
        }       
        else if (isLookup(tagName)) {
            nodeInfoHolder.push(tagName);
            nodeInfoHolder.push(ARGS);
        }
        else if (tagName.equals(RESOURCE)) {
            saveActionInfoToNode(tagName);
        }
        else if (isVariableDeclaration(tagName)) {
            if (recordIN)           nodeInfoHolder.push("In");
            else if (recordINOUT)   nodeInfoHolder.push("InOut");
            else                    nodeInfoHolder.push("--");
        }
        else if (tagName.equals(NAME)) {
            properName = true;
        }
        else if (tagName.equals(IN)) {
            recordIN = true; 
        }
        else if (tagName.equals(INOUT)) {
            recordINOUT = true;
        }       
    }

    /**
     * Based on the specfied XML end tag, if it refers to condition, local variable 
     * or action information - it will be stored appropriately.
     * 
     * @param tagName the XML tag
     */
    private void recordEndTag(String tagName) {
        // complete condition info is ready to be saved to node
        if (isCondition(tagName)) {
            saveConditionInfoToNode(tagName);
        }
        
        // complete action info is ready to be saved to node
        else if (isAction(tagName) || isResource(tagName) || tagName.equals(LIBRARYNODECALL)) {
            saveActionInfoToNode(tagName);
        }
        
        // complete local variable info is ready to be saved to node
        else if (isVariableDeclaration(tagName)) {
            saveLocalVariableInfoToNode();
        }  
        
        // more data still needs to be collected before it can be saved to node
        else if (tagName.equals(ALIAS)
                 || isReturnValue(tagName)
                 || tagName.equals(PAIR)
                 || isComparisonOperator(tagName)
                 || isMathOperator(tagName)) {
                if (nodeInfoHolder.size() > 1) {
                    String second = nodeInfoHolder.pop();
                    String first = nodeInfoHolder.pop();
                    first += SEPARATOR + convertTagNameToLiteral(tagName) + SEPARATOR + second;
                    nodeInfoHolder.push(first);
                }
            }
        else if (tagName.equals(CONCAT) || tagName.equals(AND) || tagName.equals(OR)) {
            String elements = "";

            while (!nodeInfoHolder.isEmpty()) {
                if (!nodeInfoHolder.peek().equals(tagName)) {
                    if (elements.equals(""))
                        elements = nodeInfoHolder.pop() + SEPARATOR;
                    else
                        elements = nodeInfoHolder.pop() + SEPARATOR + convertTagNameToLiteral(tagName) + SEPARATOR + elements;
                }
                else {
                    nodeInfoHolder.pop();
                    break;
                }
            }

            if (!elements.equals("")) {
                nodeInfoHolder.push(elements);
            }
        }
        else if (tagName.equals(NOT) || tagName.equals(IS_KNOWN)) {
            String complete = convertTagNameToLiteral(tagName) + SEPARATOR + "(" + SEPARATOR + nodeInfoHolder.pop() + SEPARATOR + ")";
            nodeInfoHolder.push(complete);
        }
        else if (isNodeState(tagName)) {
            if (nodeInfoHolder.size() > 1) {
                String value = nodeInfoHolder.pop();
                String state = nodeInfoHolder.pop() + convertTagNameToLiteral(tagName);
                nodeInfoHolder.push(state);
                nodeInfoHolder.push(value);
            }
        }  
        else if (tagName.equals(ARRAYELEMENT)) {
            if (nodeInfoHolder.size() > 1) {
                String index = "[" + nodeInfoHolder.pop() + "]";
                String arrayName = nodeInfoHolder.pop() + index;
                nodeInfoHolder.push(arrayName);
            }          
        } 
        else if (tagName.equals(TYPE)) {
            nodeInfoHolder.push(INITIALVAL);
        }
        else if (tagName.equals(MAXSIZE)) {
            if (nodeInfoHolder.size() > 1) {
                String maxsize = "[" + nodeInfoHolder.pop() + "]";
                String initVal = nodeInfoHolder.pop();
                String type = nodeInfoHolder.pop();
                String arrayName = nodeInfoHolder.pop() + maxsize;
                nodeInfoHolder.push(arrayName);
                nodeInfoHolder.push(type);
                nodeInfoHolder.push(initVal);
            }
        }
        else if (tagName.equals(TIME_VAL)) {
            if (nodeInfoHolder.size() > 1) {
                String second = nodeInfoHolder.pop();
                String first = nodeInfoHolder.pop();
                first = "[" + SEPARATOR + first + "," + SEPARATOR + second + SEPARATOR + "]";
                nodeInfoHolder.push(first);
            }           
        }
        else if (tagName.equals(ARGS) || isLookup(tagName)) {
            String arguments = "";

            while (!nodeInfoHolder.isEmpty()) {
                if (!nodeInfoHolder.peek().equals(ARGS)) {
                    if (arguments.equals(""))
                        arguments = nodeInfoHolder.pop() + SEPARATOR;
                    else
                        arguments = nodeInfoHolder.pop() + "," + SEPARATOR + arguments;
                }
                else {
                    nodeInfoHolder.pop();
                    break;
                }
            }

            if (!arguments.equals("")) {
                arguments = nodeInfoHolder.pop() + SEPARATOR + "(" + SEPARATOR + arguments + ")";
                nodeInfoHolder.push(arguments);
            }
        }
        else if (tagName.equals(TIMEPOINT)) {
            if (nodeInfoHolder.size() > 2) {
                String timepoint = nodeInfoHolder.pop();
                String state = nodeInfoHolder.pop();
                String nodeid = nodeInfoHolder.pop();
                nodeid += "." + state + "." + timepoint;
                nodeInfoHolder.push(nodeid);
            }
        }
        else if (tagName.equals(NAME)) {
            properName = false;
        }
        else if (tagName.equals(IN)) {
            recordIN = false;
        }
        else if (tagName.equals(INOUT)) {
            recordINOUT = false;
        }
    } 
    
    /**
     * Sends the completed condition information to the current Plexil Node and
     * clears the holder for any future information.
     * 
     * @param tagName the XML tag that indicates the end of the condition information was reached
     */
    private void saveConditionInfoToNode(String tagName)
    {
        if (!nodeInfoHolder.isEmpty() && nodeInfoHolder.size() == 1)
            {
                int condition = getCondition(tagName);
                currentNode().addConditionInfo(condition, nodeInfoHolder.pop());
            }

        nodeInfoHolder.clear();
    }
    
    /**
     * Sends the completed action information to the current Plexil Node and
     * clears the holder for any future information.
     * 
     * @param tagName the XML tag that indicates the end of the action information was reached
     */
    private void saveActionInfoToNode(String tagName)
    {
        if (tagName.equals(RESOURCE))
            {
                currentNode().addActionInfo(RESOURCE + SEPARATOR + ":");  
            }
        else if (isResource(tagName))
            {
                String resource = convertTagNameToLiteral(tagName) + SEPARATOR + nodeInfoHolder.pop();
                currentNode().addActionInfo(resource);       
            }
        else if (tagName.equals(UPDATE))
            {
                String update = "";

                while (!nodeInfoHolder.isEmpty()) 
                    {
                        if (update.equals(""))
                            update = nodeInfoHolder.pop();
                        else
                            update = nodeInfoHolder.pop() + ", " + update;
                    }

                currentNode().addActionInfo(update);
            }
        else if (tagName.equals(LIBRARYNODECALL))
            {
                String arguments = "";

                while (nodeInfoHolder.size() > 1) 
                    {
                        if (arguments.equals(""))
                            arguments = nodeInfoHolder.pop() + SEPARATOR;
                        else
                            arguments = nodeInfoHolder.pop() + "," + SEPARATOR + arguments;               
                    }

                arguments = nodeInfoHolder.pop() + SEPARATOR + "(" + SEPARATOR + arguments + ")"; 
                currentNode().addActionInfo(arguments); 
            }
        else if (!nodeInfoHolder.isEmpty())
            {
                if (tagName.equals(COMMAND) && !nodeInfoHolder.peek().endsWith(")"))
                    {
                        String closeParens = nodeInfoHolder.pop() + SEPARATOR + "(" + SEPARATOR + ")";
                        nodeInfoHolder.push(closeParens);
                    }

                if (nodeInfoHolder.size() == 2)
                    {
                        String second = nodeInfoHolder.pop();
                        String first = nodeInfoHolder.pop();
                        first += SEPARATOR + "=" + SEPARATOR + second;
                        nodeInfoHolder.push(first);
                    }

                if (nodeInfoHolder.size() > 1)
                    currentNode().addActionInfo(tagName + SEPARATOR + ":" + SEPARATOR + nodeInfoHolder.pop());
                else
                    currentNode().addActionInfo(tagName + SEPARATOR + ":" + SEPARATOR + nodeInfoHolder.pop());                
            }      

        nodeInfoHolder.clear();
    }
    
    /**
     * Sends the completed local variable information to the current Plexil Node and
     * clears the holder for any future information.
     * 
     * @param tagName the XML tag that indicates the end of the local variable information was reached
     */
    private void saveLocalVariableInfoToNode()
    {
        if (!nodeInfoHolder.isEmpty())
            {
                if (nodeInfoHolder.size() == 4)
                    {
                        nodeInfoHolder.pop();
                        nodeInfoHolder.push(UNKNOWN);
                    }
                else
                    {
                        String elements = "";

                        while (!nodeInfoHolder.isEmpty()) 
                            {
                                if (!nodeInfoHolder.peek().equals(INITIALVAL))
                                    {
                                        if (elements.equals(""))
                                            elements = nodeInfoHolder.pop();
                                        else
                                            elements = nodeInfoHolder.pop() + ", " + elements;
                                    }
                                else
                                    {
                                        nodeInfoHolder.pop();
                                        break;
                                    }
                            }

                        if (!elements.equals(""))
                            {
                                nodeInfoHolder.push(elements);
                            }
                    }

                if (nodeInfoHolder.size() == 4)
                    {
                        currentNode().addVariableInfo(nodeInfoHolder);
                    }
            }

        nodeInfoHolder.clear();
    }

    /**
     * Returns the current Node being processed.
     * @return the current Node
     */
    private Node currentNode()
    {
        int i = nodeHolder.size() - 1;
        while (nodeHolder.elementAt(i) == null)
            i--; 
        return nodeHolder.elementAt(i);
    } 

    /**
     * Returns the top level node.
     * @return the top level node
     */
    public Model getPlan()
    {
        return topLevelNode;
    }

    private int getCondition(String tag)
    {
        int condition = -1;

        for (int i = 0; i < ALL_CONDITIONS.length; i++)
            {
                if (tag.equals(ALL_CONDITIONS[i]))
                    {
                        condition = i;
                        break;
                    }
            } 

        return condition;
    }

    private boolean isProperty(String tag)
    {
        for (String property: PROPERTY_TAGS)
            if (property.equalsIgnoreCase(tag)) 
                return true;

        return false;
    }

    private boolean isCondition(String tag)
    {
        for (String condition: ALL_CONDITIONS)
            if (condition.equalsIgnoreCase(tag))
                return true;

        return false;
    }

    private boolean isVariableDeclaration(String tag)
    {
        for (String varDecl: VARIABLE_DECL_TAGS)
            if (varDecl.equalsIgnoreCase(tag))
                return true;

        return false;
    }

    private boolean isAction(String tag)
    {
        for (String action: ACTION_TAGS)
            if (action.equalsIgnoreCase(tag))
                return true;

        return false;
    }   

    private boolean isNodeState(String tag)
    {
        for (String nodeState: NODE_STATE_TAGS)
            if (nodeState.equalsIgnoreCase(tag))
                return true;

        return false;
    }

    private boolean isReturnValue(String tag)
    {
        for (String returnValue: RETURN_TAGS)
            if (returnValue.equalsIgnoreCase(tag))
                return true;

        return false;
    }

    private boolean isResource(String tag)
    {
        for (String resource: RESOURCE_TAGS)
            if (resource.equalsIgnoreCase(tag))
                return true;

        return false;
    }

    private boolean isMathOperator(String tag)
    {
        for (String math: MATH_TAGS)
            if (math.equalsIgnoreCase(tag))
                return true;

        return false;
    }

    private boolean isLookup(String tag)
    {
        for (String lookup: LOOKUP_TAGS)
            if (lookup.equalsIgnoreCase(tag))
                return true;

        return false;
    }

    private boolean isComparisonOperator(String tag)
    {
        for (String comparison: COMPARISON_TAGS)
            if (comparison.equalsIgnoreCase(tag))
                return true;

        return false;
    } 

    private String convertTagNameToLiteral(String tag)
    {
        if (tag.equals(EQ) ||
            tag.equals(EQ_NUMERIC) || 
            tag.equals(EQ_STRING) || 
            tag.equals(EQ_BOOLEAN) ||
            tag.equals(EQ_TIME) ||
            tag.equals(EQ_INTERNAL))       
            return "==";
        else if (tag.equals(NE) ||
                 tag.equals(NE_NUMERIC) ||
                 tag.equals(NE_STRING) ||
                 tag.equals(NE_BOOLEAN) || 
                 tag.equals(NE_INTERNAL))  
            return "!=";
        else if (tag.equals(LT))                        return "<";
        else if (tag.equals(GT))                        return ">";
        else if (tag.equals(LE))                        return "<=";
        else if (tag.equals(GE))                        return ">=";
        else if (tag.equals(ADD) || tag.equals(CONCAT)) return "+"; 
        else if (tag.equals(SUB))                       return "-"; 
        else if (tag.equals(MUL))                       return "*"; 
        else if (tag.equals(DIV))                       return "/"; 
        else if (tag.equals(AND))                       return "&&"; 

        else if (tag.equals(ALIAS) || 
                 tag.equals(PAIR)  ||
                 isReturnValue(tag))                    return "=";       

        else if (tag.equals(OR))                        return "||";
        else if (tag.equals(NOT))                       return "!";
        else if (tag.equals(NODE_OUTCOME_VAL))          return ".outcome"; 
        else if (tag.equals(NODE_FAILURE_VAL))          return ".failure";               
        else if (tag.equals(NODE_STATE_VAL))            return ".state";    
        else if (tag.equals(NODE_TIMEPOINT_VAL))        return ".timepoint"; 
        else if (tag.equals(NODE_CMD_HANDLE_VAL))       return ".command_handle"; 

        else if (tag.equals(RESOURCE_NAME))             return "   Name" + SEPARATOR + "=";
        else if (tag.equals(RESOURCE_PRIORITY))         return "   Priority" + SEPARATOR + "=";
        else if (tag.equals(RESOURCE_LOWER_BOUND))      return "   LowerBound" + SEPARATOR + "=";
        else if (tag.equals(RESOURCE_UPPER_BOUND))      return "   UpperBound" + SEPARATOR + "=";
        else if (tag.equals(RESOURCE_RELEASE_AT_TERM))  return "   ReleaseAtTermination" + SEPARATOR + "=";
        else                                            return tag;
    }
}
