package plexiljava.model.lookups;
import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.tokens.ArgumentsModel;
import plexiljava.model.tokens.NameModel;
import plexiljava.model.tokens.ToleranceModel;

public class LookupOnChangeModel extends LookupModel {

    public LookupOnChangeModel(BaseModel node) {
        super(node, "Change");
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);

        dsb.append("Lookup (", getChild(NameModel.class).decompile(0));
        if( hasChild(ArgumentsModel.class) ) {
            dsb.append("(", getChild(ArgumentsModel.class).decompile(0), ")");
        }
        if( hasChild(ToleranceModel.class) ) {
            dsb.append(", ", getChild(ToleranceModel.class).decompile(0));
        }
        dsb.append(")");

        return dsb.toString();
    }
}
