package plexiljava.model.operations;

import java.util.ArrayList;
import java.util.List;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.QualityModel;
import plexiljava.model.TypedNodeModel;

public class OperatorModel extends TypedNodeModel {
	
	public OperatorModel(BaseModel node) {
		super(node, "Default");
	}
	
	protected OperatorModel(BaseModel node, String type) {
		super(node, type);
	}
	
	@Override
	public boolean verify() {
		return children.size() + qualities.size() > 1;
	}
	
	@Override
	public String translate(int indentLevel) throws PatternRecognitionFailureException {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);

		List<BaseModel> iterChildren = new ArrayList<BaseModel>(children);
		List<QualityModel> iterQualities = new ArrayList<QualityModel>(qualities);
		
		while( !iterChildren.isEmpty() || !iterQualities.isEmpty() ) {
			BaseModel nextChild = null;
			QualityModel nextQuality = null;
			if( !iterChildren.isEmpty() ) {
				nextChild = iterChildren.get(0);
			}
			if( !iterQualities.isEmpty() ) {
				nextQuality = iterQualities.get(0);
			}
			if( nextChild != null && nextQuality != null ) {
				if( nextChild.getOrder() < nextQuality.getOrder() ) {
					dsb.append(nextChild.decompile(0), " ", type, " ");
					iterChildren.remove(0);
				} else {
					dsb.append(nextQuality.getValue(), " ", type, " ");
					iterQualities.remove(0);
				}
			} else if( nextChild != null ) {
				dsb.append(nextChild.decompile(0), " ", type, " ");
				iterChildren.remove(0);
			} else {
				dsb.append(nextQuality.getValue(), " ", type, " ");
				iterQualities.remove(0);
			}
		}
		dsb.sb.delete(dsb.sb.length() - (type.length() + 2), dsb.sb.length());
		return dsb.toString();
		
		/*
		if( children.size() == 2 ) {
			dsb.append(children.get(0).decompile(0), " ", type, " ", children.get(1).decompile(0));
		} else if( children.size() == 1 ) {
			if( children.get(0).getOrder() < qualities.get(0).getOrder() ) {
				dsb.append(children.get(0).decompile(0), " ", type, " ", qualities.get(0).getValue());
			} else {
				dsb.append(qualities.get(0).getValue(), " ", type, " ", children.get(0).decompile(0));
			}
		} else {
			dsb.append(qualities.get(0).getValue(), " ", type, " ", qualities.get(1).getValue());
		}
		
		return dsb.toString();
		*/
	}

}
