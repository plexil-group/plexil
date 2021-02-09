package plexiljava.model;

import java.util.ArrayList;
import java.util.List;

import org.w3c.dom.Node;

import plexiljava.decompilation.Decompilable;
import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.main.Decompiler;

/**
 * A model that represents an element in the modified XML tree
 */
public class BaseModel implements Decompilable {
    protected BaseModel root;					// The BaseModel this one was built off of

    protected Node originalNode;				// The XML element node this one was built off of
    protected int order;						// The position of this node in reference to its siblings

    protected BaseModel parent;					// The parent node
    protected List<BaseModel> children;			// Child nodes that could potentially branch off
    protected List<QualityModel> qualities;		// Child nodes that are guaranteed to be leaf nodes
    protected List<BaseModel> attributes;		// XML attributes of this node

    /**
     * Gets the children of an XML node that doesn't have a set children size
     * @param node to get the children of
     * @return List of child nodes
     */
    public static List<Node> getChildren(Node node) {
        List<Node> children = new ArrayList<Node>();
        int index = 0;
        Node element;
        while( (element=node.getChildNodes().item(index)) != null ) {
            children.add(element);
            index++;
        }
        return children;
    }

    /**
     * Constructs a BaseModel and generates all of its properties based on an instance of a BaseModel
     * @param root BaseModel to construct based off of
     */
    public BaseModel(BaseModel root) {
        this.root = root;

        this.originalNode = root.getOriginalNode();
        this.order = root.getOrder();

        this.parent = root.getParent();
        children = new ArrayList<BaseModel>();
        qualities = new ArrayList<QualityModel>();
        attributes = new ArrayList<BaseModel>();

        if( originalNode.getAttributes() != null ) {
            int index = 0;
            Node attribute;
            while( (attribute=originalNode.getAttributes().item(index)) != null ) {
                attributes.add(new BaseModel(attribute, this, index));
                index++;
            }
        }

        addBranches();
    }

    /**
     * Constructs a BaseModel and generates all of its properties
     * @param node XML node to construct off of
     * @param parent node of this one
     * @param order relative to its siblings
     */
    public BaseModel(Node node, BaseModel parent, int order) {
        node.normalize();
        this.originalNode = node;
        this.order = order;

        this.parent = parent;
        children = new ArrayList<BaseModel>();
        qualities = new ArrayList<QualityModel>();
        attributes = new ArrayList<BaseModel>();

        if( originalNode.getAttributes() != null ) {
            int index = 0;
            Node attribute;
            while( (attribute=originalNode.getAttributes().item(index)) != null ) {
                attributes.add(new BaseModel(attribute, this, index));
                index++;
            }
        }

        addBranches();
        root = this;
    }

    /**
     * Internal function that separates children from qualities and adds them accordingly
     */
    protected void addBranches() {
        int index = 0;
        for( Node child : getChildren(originalNode) ) {
            if( child.getChildNodes().getLength() == 1 && child.getFirstChild().getNodeName().equals("#text") ) {
                qualities.add(new QualityModel(child, this, index));
            } else {
                children.add(new BaseModel(child, this, index));
            }
            index += 1;
        }
    }

    /**
     * 
     * @return Node XML node this one was built off of
     */
    public Node getOriginalNode() {
        return originalNode;
    }

    /**
     * 
     * @return BaseModel parent node of this one
     */
    public BaseModel getParent() {
        return parent;
    }

    /**
     * 
     * @return String value of the original XML name
     */
    public String getName() {
        return originalNode.getNodeName();
    }

    /**
     * 
     * @return String value of the original XML node
     */
    public String getValue() {
        return originalNode.getNodeValue();
    }

    /**
     * 
     * @return int depth in tree
     */
    public int getDepth() {
        if( parent == null ) {
            return 0;
        }
        return parent.getDepth() + 1;
    }

    /**
     * 
     * @return int order relative to siblings
     */
    public int getOrder() {
        return order;
    }

    /**
     * 
     * @return List of BaseModel children
     */
    public List<BaseModel> getChildren() {
        return new ArrayList<BaseModel>(children);
    }

    /**
     * 
     * @param c Class to filter for
     * @return List of BaseModel children of Class c
     */
    public List<BaseModel> getChildren(@SuppressWarnings("rawtypes") Class c) {
        List<BaseModel> matchingChildren = new ArrayList<BaseModel>();
        for( BaseModel child : children ) {
            if( c.isInstance(child)) {
                matchingChildren.add(child);
            }
        }
        return matchingChildren;
    }

    /**
     * 
     * @return List of QualityModel qualities
     */
    public List<QualityModel> getQualities() {
        return new ArrayList<QualityModel>(qualities);
    }

    /**
     * 
     * @return List of BaseModel attributes
     */
    public List<BaseModel> getAttributes() {
        return new ArrayList<BaseModel>(attributes);
    }

    /**
     * 
     * @param name String attribute to search for
     * @return BaseModel attribute with the given name, if it exists
     */
    public BaseModel getAttribute(String name) {
        for( BaseModel attribute : attributes ) {
            if( attribute.getName().equals(name) ) {
                return attribute;
            }
        }
        return null;
    }

    /**
     * 
     * @param c Class to filter for
     * @return BaseModel first child of the type Class c
     */
    public BaseModel getChild(@SuppressWarnings("rawtypes") Class c) {
        for( BaseModel child : children ) {
            if( c.isInstance(child) ) {
                return child;
            }
        }
        return null;
    }

    /**
     * 
     * @param name String quality to search for
     * @return QualityModel quality with the given name, if it exists
     */
    public QualityModel getQuality(String name) {
        for( QualityModel quality : qualities ) {
            if( quality.getName().equals(name) ) {
                return quality;
            }
        }
        return null;
    }

    /**
     * 
     * @param c Class to check children for
     * @return true if a child of Class c exists
     */
    public boolean hasChild(@SuppressWarnings("rawtypes") Class c) {
        return getChild(c) != null;
    }

    /**
     * 
     * @param name String to check qualities for
     * @return true if a quality with the given name exists
     */
    public boolean hasQuality(String name) {
        return getQuality(name) != null;
    }

    /**
     * 
     * @param name String to check attributes for
     * @return true if an attribute with the given name exists
     */
    public boolean hasAttribute(String name) {
        return getAttribute(name) != null;
    }

    /**
     * An exception that generates its message based on the class it is thrown from
     */
    @SuppressWarnings("serial")
    public class PatternRecognitionFailureException extends Exception {
        public PatternRecognitionFailureException(@SuppressWarnings("rawtypes") Class c, String lineNumber, String colNumber) {
            super("Unfamiliar pattern detected while generating " + c.getName() + " in file " + Decompiler.infileName + " at " + lineNumber + ":" + colNumber);
        }

        public PatternRecognitionFailureException(@SuppressWarnings("rawtypes") Class c) {
            super("Unfamiliar pattern detected while generating " + c.getName());
        }
    }

    /**
     * Throw an exception based on where in the file it is thrown, as well as based on the runtime parameters
     * @throws PatternRecognitionFailureException
     */
    public void throwPatternRecognitionFailureException() throws PatternRecognitionFailureException {
        if( hasAttribute("ColNo") && hasAttribute("LineNo") ) {
            throw new PatternRecognitionFailureException(this.getClass(), getAttribute("ColNo").getValue(), getAttribute("LineNo").getValue());
        } else {
            throw new PatternRecognitionFailureException(this.getClass());
        }
    }

    @Override
    public boolean verify() {
        return true;
    }

    @Override
    public final String decompile(int indentLevel) throws PatternRecognitionFailureException {
        if( !verify() && !Decompiler.UNSAFE ) {
            throwPatternRecognitionFailureException();
        }
        //try {
        return translate(indentLevel);
        //} catch(Exception e) {
        //	throwPatternRecognitionFailureException();
        //	return null;
        //}
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        dsb.append(getValue());
        return dsb.toString();
    }
}
