package plexiljava.model;

import java.util.ArrayList;
import java.util.List;

import org.w3c.dom.Node;

import plexiljava.decompilation.Decompilable;
import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.main.Decompiler;

public class BaseModel implements Decompilable {
	protected BaseModel root;
	
	protected Node originalNode;	
	protected int order;
	
	protected BaseModel parent;
	protected List<BaseModel> children;
	protected List<QualityModel> qualities;
	protected List<BaseModel> attributes;
	
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
	
	public Node getOriginalNode() {
		return originalNode;
	}
	
	public BaseModel getParent() {
		return parent;
	}
	
	public String getName() {
		return originalNode.getNodeName();
	}
	
	public String getValue() {
		return originalNode.getNodeValue();
	}
	
	public int getDepth() {
		if( parent == null ) {
			return 0;
		}
		return parent.getDepth() + 1;
	}

	public int getOrder() {
		return order;
	}
	
	public List<BaseModel> getChildren() {
		return new ArrayList<BaseModel>(children);
	}
	
	public List<BaseModel> getChildren(@SuppressWarnings("rawtypes") Class c) {
		List<BaseModel> matchingChildren = new ArrayList<BaseModel>();
		for( BaseModel child : children ) {
			if( c.isInstance(child)) {
				matchingChildren.add(child);
			}
		}
		return matchingChildren;
	}
	
	public List<QualityModel> getQualities() {
		return new ArrayList<QualityModel>(qualities);
	}
	
	public List<BaseModel> getAttributes() {
		return new ArrayList<BaseModel>(attributes);
	}
	
	public BaseModel getAttribute(String name) {
		for( BaseModel attribute : attributes ) {
			if( attribute.getName().equals(name) ) {
				return attribute;
			}
		}
		return null;
	}
	
	public BaseModel getChild(@SuppressWarnings("rawtypes") Class c) {
		for( BaseModel child : children ) {
			if( c.isInstance(child) ) {
				return child;
			}
		}
		return null;
	}
	
	public QualityModel getQuality(String name) {
		for( QualityModel quality : qualities ) {
			if( quality.getName().equals(name) ) {
				return quality;
			}
		}
		return null;
	}
	
	public boolean hasChild(@SuppressWarnings("rawtypes") Class c) {
		return getChild(c) != null;
	}
	
	public boolean hasQuality(String name) {
		return getQuality(name) != null;
	}
	
	public boolean hasAttribute(String name) {
		return getAttribute(name) != null;
	}
	
	@SuppressWarnings("serial")
	public class PatternRecognitionFailureException extends Exception {
		public PatternRecognitionFailureException(@SuppressWarnings("rawtypes") Class c, String lineNumber, String colNumber) {
			super("Unfamiliar pattern detected while generating " + c.getName() + " in file " + Decompiler.infileName + " at " + lineNumber + ":" + colNumber);
		}
		
		public PatternRecognitionFailureException(@SuppressWarnings("rawtypes") Class c) {
			super("Unfamiliar pattern detected while generating " + c.getName());
		}
	}
	
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
		if( !verify() && !Decompiler.FORCE ) {
			throwPatternRecognitionFailureException();
		}
		return translate(indentLevel);
	}
	
	@Override
	public String translate(int indentLevel) throws PatternRecognitionFailureException {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		dsb.append(getValue());
		return dsb.toString();
	}
}
