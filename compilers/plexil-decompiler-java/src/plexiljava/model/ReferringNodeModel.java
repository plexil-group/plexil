package plexiljava.model;

import plexiljava.decompilation.DecompilableStringBuilder;

public class ReferringNodeModel extends NodeModel {

    /**
     * Constructs a generic extension of the NodeModel that contains a reference to another node
     * @param node BaseModel to be built off of
     */
    public ReferringNodeModel(BaseModel node) {
        super(node);
    }

    /**
     * 
     * @return String referred node ID
     */
    public String getReference() {
        return hasQuality("NodeRef") ? getQuality("NodeRef").getValue() : getQuality("NodeId").getValue();
    }

    @Override
    public boolean verify() {
        return hasQuality("NodeRef") || hasQuality("NodeId");
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        dsb.addReference(getReference());
        return dsb.toString();
    }
}
