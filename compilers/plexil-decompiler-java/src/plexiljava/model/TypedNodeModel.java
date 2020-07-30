package plexiljava.model;

public class TypedNodeModel extends NodeModel {

	protected String type;
	
	public TypedNodeModel(BaseModel node) {
		super(node);
	}

	protected TypedNodeModel(BaseModel node, String type) {
		super(node);
		this.type = type;
	}
	
	public String getType() {
		return type;
	}
}
