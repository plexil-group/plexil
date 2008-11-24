 /* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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
 import java.util.ArrayList;
 import org.xml.sax.Attributes;
 import java.util.Stack;

 import static gov.nasa.luv.Constants.*;

 /** SAX PlexilPlan XML handler */

 public class PlexilPlanHandler extends AbstractDispatchableHandler
 {
     private boolean recordVariable = true;
     private String variableHolder = VAR + ":";  // : is used as a place holder in between variable elements, (see Model.java addVariableinfo())
     
     private boolean recordArray = true;
     private String arrayHolder = ARRAY + ":";
            
     private boolean 
	 recordCondition = false, 
	 recordEQ = false, 
	 recordNE = false, 
	 recordConditionArray = false, 
	 lookupChange = false, 
	 lonelyValue = false, 
	 recordTime = false, 
	 tolerance = false,
	 lookupNow = false,
	 lonelyVariable = false,
	 recordNodeTimepoint = false,
	 lessThan = false,
	 greaterThan = false,
	 lessThanEqual = false,
	 greaterThanEqual = false;
     private String save = "";  
     private String conditionEquation = "";
     private ArrayList<String> equationHolder = new ArrayList<String>();
     private ArrayList<String> lookupArguments = new ArrayList<String>(); 

     private boolean libraryNodeCall = false;

     private Stack<Model> stack = new Stack<Model>();

     private Model topLevelNode = null;

     public PlexilPlanHandler()
     {
	 super();
	 stack.push(Model.getRoot());
     }

     /** Handle start of an XML document */

     public void startDocument()
     {
	 resetFlags();
     }

     /** Handle start of an XML element. */

     public void startElement(String uri, String tagName, 
			      String qName, Attributes attributes)
     {
	 // get the current node in the stack

	 Model node = stack.peek();
         
         // if loaded a script instead of plan
         
         if (tagName.equals("PLEXILScript"))
         {
            try 
            {
                throw new Exception();
            } 
            catch (Exception ex) 
            {
                Luv.getLuv().displayErrorMessage(null, "ERROR: loaded script instead of plan");
            }
         }

	 // if this SHOULD be a child node, make that happen

	 if (tagName.equals(NODE)) 
         {
	     Model child = new Model(tagName);

	     // if there is a parent, add child to it
	     // *** There should ALWAYS be a parent!
	     node.addChild(child);

	     // if the parent is the root node, save this as a (the?) top level node
	     if (node.isRoot()) 
             {
		 topLevelNode = child;
	     }

	     // this child is now the current node

	     node = child;

	     // add attributes for this child

	     for (int i = 0; i < attributes.getLength(); ++i) 
             {
		 child.setProperty(attributes.getQName(i),
				   attributes.getValue(i));
	     }

	 }

	 // if it's not a property we can ignore it

	 else if (!isProperty(tagName)) 
         {
	     node = null;
	 }

	 // check if tagName indicates that we will need to store variable 
	 // and condition info before executing

	 catchStartTag(tagName);

	 // push new node onto the stack

	 stack.push(node);
     }

     /** Handle end of an XML element. */

     public void endElement(String uri, String tagName, String qName)
     {
	 Model topNode = stack.peek();
	 Model nodeToUpdate = getNodeToUpdate();    

	 // check if tagName indicates that we are finishing storage of info
	 // about a certain variable or condition

	 catchEndTag(nodeToUpdate, tagName);

	 // get tweener text and put it in its place

	 String text = getTweenerText();

	 try 
         {
             assignTweenerText(nodeToUpdate, tagName, text);
         } 
	 catch (InterruptedIOException ex) 
         {
             Luv.getLuv().displayErrorMessage(ex, "ERROR: exception occurred while locating library");
         }

	 // assign model name and path to the appropriate model

	 if (topNode != null && text != null) 
         {
	     topNode.setProperty(tagName, text);
	     if (tagName.equals(NODE_ID)) 
             {
		 topNode.setModelName(text);
		 topNode.setPathToNode();
	     }
	}

	// if node, assign the main attributes for display in luv viewer
	// name, type, state and outcome

	if (topNode != null && tagName.equals(topNode.getType()))
            topNode.setMainAttributesOfNode();

	// pop the node off the stack

	stack.pop();        

	// Make sure Luv instance is notified if this was plan or library
	if (tagName.equals(PLEXIL_PLAN))
	    Luv.getLuv().handleNewPlan(topLevelNode);
	else if (tagName.equals(PLEXIL_LIBRARY))
	    Luv.getLuv().handleNewLibrary(topLevelNode);
    }

    /** Handle end of document event. */

    public void endDocument()
    {
    }
      
    public void catchStartTag(String tagName)
    {
	if (tagName.equals(LIBRARYNODECALL))
        {
            libraryNodeCall = true;
        }
	else if (tagName.contains(CONDITION))
        {
            recordCondition = true;     
            recordEQ = recordNE = recordConditionArray = lookupChange = 
            lonelyValue = lonelyVariable = recordTime = tolerance = lookupNow =
            lessThan = greaterThan = lessThanEqual = greaterThanEqual = false;
            save = conditionEquation = "";
            equationHolder.clear();
            lookupArguments.clear();
        } 
        else if (tagName.equals(DECL_VAR))
        {
            recordVariable = true;
            variableHolder =  VAR + ":";  // : is used as a place holder in between variable elements, (see Model.java addVariableinfo())
        }
        else if (tagName.equals(DECL_ARRAY))
        {
            recordArray = true;
            arrayHolder =  ARRAY + ":";
        }

	if (recordCondition)
	    recordStartConditionInfo(tagName);
    }
      
    public void assignTweenerText(Model nodeToUpdate, String tagName, String text) 
	throws InterruptedIOException
    {
	if (text != null) {
	    if (libraryNodeCall) {
		Model library = 
		    Luv.getLuv().findLibraryNode(text,
						 !Luv.getLuv().getFileHandler().getStopSearchForMissingLibs());
		if (library == null) {
		    topLevelNode.addMissingLibrary(text);
		}
		else {
		    topLevelNode.addLibraryName(library.getPlanName());
		    // Inherit the libraries called by this one
		    for (String libfile: library.getLibraryNames())
			topLevelNode.addLibraryName(libfile);
		    for (String libnode: library.getMissingLibraries())
			topLevelNode.addMissingLibrary(libnode);
		    nodeToUpdate.addChild((Model) library.clone());
		}
		libraryNodeCall = false;
	    }

	    if (recordCondition)
		recordMiddleConditionInfo(tagName, text);
            
            if (recordVariable)
                variableHolder += text + ":";   // : is used as a place holder in between variable elements, (see Model.java addVariableinfo())
            
            if (recordArray)
                arrayHolder += text + ":";
             
	}
	else if (tagName.equals(NODEREF))
	    conditionEquation += nodeToUpdate.getProperty(NODE_ID);
    }
      
    public void catchEndTag(Model nodeToUpdate, String tagName)
    {        
	if (recordCondition)
        {
            if (tagName.equals(ADD)) {
                tagName = "+";
            }
            if (tagName.equals(SUB)) {
                tagName = "-";
            }
            if (tagName.equals(MUL)) {
                tagName = "*";
            }
            if (tagName.equals(DIV)) {
                tagName = "/";
            }
            conditionEquation = conditionEquation.replace("PlaceHolder", tagName);

            recordEndConditionInfo(tagName);
        }
        
        if (tagName.contains(DECL_VAR) && recordVariable)
            nodeToUpdate.addVariableInfo(variableHolder);
        
        if (tagName.contains(DECL_ARRAY) && recordArray)
            nodeToUpdate.addVariableInfo(arrayHolder);
    }
      
    public Model getNodeToUpdate()
    {
	int i = stack.size() - 1;
	while (stack.elementAt(i) == null)
	    i--; 
	return stack.elementAt(i);
    } 
      
    public void recordStartConditionInfo(String tagName)
    { 
	int name = getTagName(tagName);
         
	switch (name)
	    {
	    case TOLERANCE_NUM: 
		tolerance = true; 
		break;
	    case LT_NUM: 
		lessThan = true;
		break;
	    case GT_NUM: 
		greaterThan = true; 
		break;
	    case LE_NUM: 
		lessThanEqual = true; 
		break;
	    case GE_NUM: 
		greaterThanEqual = true; 
		break;
	    case EQ_NUM: 
		equationHolder.add("PlaceHolder");
		recordEQ = true;
		break;
	    case NE_NUM:
		equationHolder.add("PlaceHolder");
		recordNE = true;
		break;
	    case TIME_NUM:
		if (recordEQ)
		    conditionEquation += " == [";  
		else
		    conditionEquation += " != [";
		recordTime = true;
		break;
	    case ARRAYELEMENT_NUM:
		recordConditionArray = true;             
		if (!recordEQ)
		    equationHolder.add("PlaceHolder");
		break;
	    case NOT_NUM: conditionEquation = "!("; break;
	    case LOOKUP_NUM:
		if (conditionEquation.length() > 0 && !conditionEquation.equals("!("))
                {
                    if (recordEQ)
                        conditionEquation += " == ";
                    else
                        conditionEquation += " != ";
                }

		if (tagName.contains(LOOKUPNOW))
                {
                    conditionEquation += LOOKUPNOW;
                    lookupNow = true;
                }
		else if (tagName.equals(LOOKUPCHANGE)) 
                {
                    conditionEquation += LOOKUPCHANGE;
                    lookupChange = true;
                }   
                else if (tagName.equals(LOOKUPFREQ))
                {
                    conditionEquation += LOOKUPFREQ;
                    lookupChange = true;
                } 
                
		lookupArguments = new ArrayList<String>();
		conditionEquation += "(";
		break;                
	    }        
    }
      
    public void recordMiddleConditionInfo(String tagName, String text)
    {
	if (tagName.contains(VAR) || tagName.equals(NAME) || tagName.equals(NODEREF) || tagName.equals(NODE_ID) || tagName.equals(STATE_NAME))
	    {
		lonelyValue = false;
                 
		if (lookupChange || lookupNow)
		    lookupArguments.add(text);
		else
		    {
			if (conditionEquation.length() == 0)
			    {
				lonelyVariable = true;
				conditionEquation += text;
			    }
			else if (conditionEquation.equals("!("))
			    conditionEquation += text;
			else
			    {
				if (recordEQ)
				    conditionEquation += " == " + text;
				else
				    conditionEquation += " != " + text;

				lonelyVariable = false;
			    }
		    }
	    }
	else if (tagName.equals(STRING_VAL))
	    {
		lonelyVariable = false;
		if (lookupChange || lookupNow)
		    {
			text = "\"" + text + "\"";
			lookupArguments.add(text);
		    }
		else if (conditionEquation.length() == 0 && equationHolder.size() > 0)
		    {
			int last = equationHolder.size() - 1;
			conditionEquation = equationHolder.get(last);
			conditionEquation += " == \"" + text + "\"";
			equationHolder.set(last, conditionEquation);
			conditionEquation = "";
		    }
		else
		    conditionEquation += " == \"" + text + "\"";
	    }
	else if (tagName.contains(VAL))
	    {
		lonelyVariable = false;
                 
		if (tolerance)
		    {
			lookupArguments.add(text);
		    }
		else if (recordNodeTimepoint)
		    {
			lonelyVariable = true;
			conditionEquation += "." + text;
		    }
		else if (recordConditionArray)
		    {
			conditionEquation += "[" + text + "]";
			recordConditionArray = false;
		    }
		else if (conditionEquation.length() > 0 && !recordTime)
		    {
			if (recordNE)
			    conditionEquation += " != " + text;
			else if (recordEQ)
			    conditionEquation += " == " + text;
			else if (lessThan)
			    conditionEquation += " < " + text;
			else if (greaterThan)
			    conditionEquation += " > " + text;
			else if (lessThanEqual)
			    conditionEquation += " <= " + text;
			else if (greaterThanEqual)
			    conditionEquation += " >= " + text;
			else
			    conditionEquation += " PlaceHolder " + text;
		    }
		else if (recordTime)
		    {
			conditionEquation += ", " + text;
		    }
		else
		    {
			lonelyValue = true;
			conditionEquation = text;
		    }
	    } 
	else if (tagName.equals(TIMEPOINT))
	    {
		lonelyVariable = true;
		conditionEquation += "." + text;
	    }                
    }
      
    public void recordEndConditionInfo(String tagName)
    {
          
	int name = getTagName(tagName);
         
	switch (name)
	    {
	    case NODE_OUTCOME_NUM:     conditionEquation += ".outcome"; break;
	    case NODE_FAILURE_NUM:     conditionEquation += ".failure"; break;               
	    case NODE_STATE_NUM:       conditionEquation += ".state"; break;   
	    case NODE_TIMEPOINT_NUM:   conditionEquation += ".timepoint"; break; 
	    case NODE_CMD_HANDLE_NUM:  conditionEquation += ".command_handle"; break; 
	    case TIME_NUM: 
		conditionEquation += "]";  
		conditionEquation = conditionEquation.replace("[, ", "[");
		equationHolder.add(conditionEquation);
		conditionEquation = "";          
		recordTime = false;
		break;
	    case IS_KNOWN_NUM: 
		if (!equationHolder.isEmpty())
		    {
			updateEquation(" is known");
		    }
		else
		    {
			conditionEquation += " is known";
			equationHolder.add(conditionEquation);
			conditionEquation = "";
		    }
		break;       
	    case LOOKUP_NUM:
		String args = "";
                 
		if (lookupArguments.size() > 1)
		    {
			if (lookupNow)
			    {
				args = lookupArguments.get(0);
				args += "(" + lookupArguments.get(1) + ")";
			    }
			else if (lookupChange)
			    {
				if (!tolerance)
				    {
					args = lookupArguments.get(1);
					args += "," + lookupArguments.get(0);
				    }
				else
				    {
					args = lookupArguments.get(0);
					args += "," + lookupArguments.get(1);
					tolerance = false;
				    }
			    }
		    }
		else if (lookupArguments.size() == 1)
		    args = lookupArguments.get(0);

		conditionEquation += args + ")";

		equationHolder.add(conditionEquation);
		conditionEquation = "";
		lookupArguments.clear();

		lookupChange = false;
		lookupNow = false;
		break;
	    case EQ_NUM:                  
	    case NE_NUM:
	    case LT_NUM:                  
	    case GT_NUM:
	    case LE_NUM:                  
	    case GE_NUM:
		recordEquation(tagName);
		break;
	    case ARRAYELEMENT_NUM:
		if (!recordEQ)
		    recordEquation(tagName);
		break;
	    case NOT_NUM: 
		if (!equationHolder.isEmpty())
		    {
			updateEquation(")");
		    }
		else
		    {
			conditionEquation += ")";
			equationHolder.add(conditionEquation);
			conditionEquation = "";
		    }
		break;
	    case AND_NUM:
	    case OR_NUM: 
		for (int i = equationHolder.size() - 1; i >= 0; i--)
		    {
			if (equationHolder.get(i).equals("PlaceHolder"))
			    {
				equationHolder.set(i, tagName);
				break;
			    }
		    }

		save = tagName;
		break;
	    case CONDITION_NUM: 
		recordCondition = false;
		int condition = -1;

		for (int i = 0; i < ALL_CONDITIONS.length; i++)
		    {
			if (tagName.equals(ALL_CONDITIONS[i]))
			    {
				condition = i;
				break;
			    }
		    } 
                 
		if (conditionEquation.length() > 0)
		    {
			if (equationHolder.size() > 1)
			    {
				if (equationHolder.get(0).equals(OR) || equationHolder.get(0).equals(AND))
				    {
					equationHolder.add(equationHolder.get(0));
				    }
			    }
			equationHolder.add(conditionEquation);
			conditionEquation = "";
		    }

		if (equationHolder.size() > 1)
		    {
			if (equationHolder.get(0).equals(OR) || equationHolder.get(0).equals(AND) || equationHolder.get(0).equals("PlaceHolder"))
			    equationHolder.remove(0);
		    }

		if (equationHolder.contains("PlaceHolder"))
		    {
			for (int i = equationHolder.size() - 1; i >= 0; i--)
			    {
				if (equationHolder.get(i).equals("PlaceHolder"))
				    {
					equationHolder.set(i, save);
				    }
			    }
		    }

		getNodeToUpdate().addConditionInfo(condition, equationHolder);

		equationHolder = new ArrayList<String>();
		break;
	    case NODE_TIMEPOINT_VAL_NUM:
		recordNodeTimepoint = false;
		break;
	    }        
    }  
      
    public void recordEquation(String tagName)
    {
	if (conditionEquation.length() > 0) {

	    if (lonelyValue || lonelyVariable) {

		if (!equationHolder.isEmpty()) {

		    int lastIndex = equationHolder.size() - 1;

		    if (!conditionEquation.contains("=")) {
			if (tagName.contains(EQ))
			    conditionEquation = " == " + conditionEquation;
			else if (tagName.contains(NE))
			    conditionEquation = " != " + conditionEquation;

			String replace = equationHolder.get(lastIndex) + conditionEquation;
			equationHolder.set(lastIndex,replace);
		    }
		    else {

			equationHolder.set(lastIndex,conditionEquation);
		    }
		}	      
		else
		    equationHolder.add(conditionEquation);

		lonelyValue = lonelyVariable = false;
	      
	    }
	    else           
		equationHolder.add(conditionEquation);

	    conditionEquation = "";
	}
                              
	if (tagName.contains(EQ))
	    recordEQ = false;
	else if (tagName.contains(NE))
	    recordNE = false;
	else if (tagName.contains(LT))
	    lessThan = false;
	else if (tagName.contains(GT))
	    greaterThan = false;
	else if (tagName.contains(LE))
	    lessThanEqual = false;
	else if (tagName.contains(GE))
	    greaterThanEqual = false;
    }
      
    public void updateEquation(String update)
    {
	int lastIndex = equationHolder.size() - 1;
	String replace = equationHolder.get(lastIndex) + update;
	equationHolder.set(lastIndex,replace);
	conditionEquation = "";
    }

    public Model getPlan()
    {
	return topLevelNode;
    }

    public void resetFlags()
    {
        recordVariable = false;
        variableHolder =VAR + ":";  // : is used as a place holder in between variable elements, (see Model.java addVariableinfo())
        recordArray = false;
        arrayHolder = ARRAY + ":";
        
	recordCondition = false; 
	recordEQ = false; 
	recordNE = false; 
	recordConditionArray = false; 
	lookupChange = false; 
	lonelyValue = false; 
	recordTime = false; 
	tolerance = false; 
	lookupNow = false; 
	lonelyVariable = false; 
	recordNodeTimepoint = false;
	lessThan  = false;
	greaterThan = false; 
	lessThanEqual = false;
	greaterThanEqual = false;
	save = "";  
	conditionEquation = "";
	equationHolder = new ArrayList<String>();
	lookupArguments = new ArrayList<String>(); 

	libraryNodeCall = false;
    }

    public static boolean isProperty(String tag)
    {
	for (String propertyTag: PROPERTY_TAGS)
            if (propertyTag.equalsIgnoreCase(tag))
		return true;
         
	return false;
    }


}
