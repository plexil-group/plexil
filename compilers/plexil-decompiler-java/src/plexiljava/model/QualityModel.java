package plexiljava.model;

import org.w3c.dom.Node;

import plexiljava.decompilation.DecompilableStringBuilder;

public class QualityModel extends BaseModel {

	public QualityModel(Node node, BaseModel parent, int order) {
		super(node, parent, order);
	}

	@Override
	public String getValue() {
		return children.get(0).getValue();
	}
	
	@Override
	public boolean verify() {
		return !children.isEmpty();
	}
	
	@Override
	public String translate(int indentLevel) throws PatternRecognitionFailureException {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		dsb.append(children.get(0).getValue());
		return dsb.toString();
	}
	
}
