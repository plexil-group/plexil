package plexiljava.model;

import org.w3c.dom.Node;

import plexiljava.decompilation.DecompilableStringBuilder;

public class QualityModel extends BaseModel {

    /**
     * Constructs a generic extension of the BaseModel that has only one associated value
     * @param node XML node to construct off of
     * @param parent node of this one
     * @param order relative to its siblings
     */
    public QualityModel(Node node, BaseModel parent, int order) {
        super(node, parent, order);
    }

    /**
     * @return String associated value of this quality
     */
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
