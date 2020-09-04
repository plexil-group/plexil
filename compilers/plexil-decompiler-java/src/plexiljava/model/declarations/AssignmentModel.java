package plexiljava.model.declarations;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class AssignmentModel extends NodeModel {

    public AssignmentModel(BaseModel node) {
        super(node);
    }

    @Override
    public boolean verify() {
        return children.size() == 2 || !qualities.isEmpty();
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        if( children.size() == 2 ) {
            dsb.append(children.get(0).decompile(0), " = ");
            if( children.get(1).getName().equals("StringValue") && !children.get(1).decompile(0).startsWith("\"") ) {
                dsb.append("\"");
            }
            dsb.append(children.get(1).decompile(0));
            if( children.get(1).getName().equals("StringValue") && !children.get(1).decompile(0).endsWith("\"") ) {
                dsb.append("\"");
            }
        } else {
            dsb.append(qualities.get(0).getValue(), " = ");
            if( children.get(0).getName().equals("StringValue") && !children.get(0).decompile(0).startsWith("\"") ) {
                dsb.append("\"");
            }
            dsb.append(children.get(0).decompile(0));
            if( children.get(0).getName().equals("StringValue") && !children.get(0).decompile(0).endsWith("\"") ) {
                dsb.append("\"");
            }
        }
        if( indentLevel != 0 ) {
            dsb.append(";");
        }
        return dsb.toString();
    }

}
