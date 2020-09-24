package plexiljava.decompilation;

import plexiljava.model.BaseModel.PatternRecognitionFailureException;

public interface Decompilable {

    /**
     * Preliminary check to ensure that the given pattern has all the components it needs
     * When run in safe mode, this function is checked to determine whether the decompiler should exit
     * Otherwise, a warning will be generated
     * @return true if all the components necessary for decompiling this node exist
     */
    public boolean verify();

    /**
     * Create a String representation of this node
     * This function should only be called internally, but is made public for use in very specific edge cases
     * @param indentLevel int number of indents to use before the lines in this node
     * @return String representation of this node
     * @throws PatternRecognitionFailureException if any of its children fail their verification checks
     */
    public String translate(int indentLevel) throws PatternRecognitionFailureException;

    /**
     * Creates a String representation of this node if verification passes or if not run in safe mode
     * This function should be the one being called when trying to get the decompiled representation of a node
     * This function should only be overridden if the associated decompilation pattern requires specific manipulations
     * @param indentLevel int number of indents to use before the lines in this node
     * @return String representation of this node
     * @throws PatternRecognitionFailureException if this node's verification fails
     */
    public String decompile(int indentLevel) throws PatternRecognitionFailureException; 

}
