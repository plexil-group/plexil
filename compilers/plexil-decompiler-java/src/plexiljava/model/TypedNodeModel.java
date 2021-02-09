package plexiljava.model;

public class TypedNodeModel extends NodeModel {

    protected String type;	// The varying factor in question

    /**
     * Construct a generic extension of the NodeModel that follows a template with only one varying factor
     * This constructor is meant to be overridden and call the protected constructor with a hard-coded type
     * @param node BaseModel to be built off of
     */
    public TypedNodeModel(BaseModel node) {
        super(node);
    }

    /**
     * Construct a generic extension of the NodeModel that follows a template with only one varying factor
     * @param node BaseModel to be built off of
     * @param type String that determiens this node's type
     */
    protected TypedNodeModel(BaseModel node, String type) {
        super(node);
        this.type = type;
    }

    /**
     * 
     * @return String type of this node
     */
    public String getType() {
        return type;
    }
}
