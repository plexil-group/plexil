package plexiljava.model;

import org.w3c.dom.Node;

public class QualityModel extends BaseModel {

	public QualityModel(Node node, BaseModel parent, int order) {
		super(node, parent, order);
	}

	@Override
	public String getValue() {
		return children.get(0).getValue();
	}
	
	@Override
	public String decompile(int indentLevel) {
		return indent(indentLevel) + children.get(0).getValue();
	}
	
}
