package plexiljava.model;

public class ReferringNodeModel extends NodeModel {

	public ReferringNodeModel(BaseModel node) {
		super(node);
	}

	public String getReference() {
		return getQuality("NodeRef").getValue();
	}
	
	@Override
	public String decompile(int indentLevel) {
		return indent(indentLevel) + "@" + getReference();
	}
}
