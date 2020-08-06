package plexiljava.model.declarations;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.QualityModel;

public class InitialValueModel extends NodeModel {

	public InitialValueModel(BaseModel node) {
		super(node);
	}

	@Override
	public String decompile(int indentLevel) {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		
		if( !children.isEmpty() ) {
			dsb.append(children.get(0).decompile(0));
		} else {
			for( QualityModel quality : qualities ) {
				dsb.append(quality.getValue(), " ");
			}
			dsb.sb.deleteCharAt(dsb.sb.length()-1);
		}
		
		return dsb.toString();
	}
	
}
