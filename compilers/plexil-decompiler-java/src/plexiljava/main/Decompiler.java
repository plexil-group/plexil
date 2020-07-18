package plexiljava.main;

import java.io.IOException;

import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Element;
import org.xml.sax.SAXException;

import plexiljava.commons.xml.XMLIO;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class Decompiler {

	public static final String INFILE = "ArrayVariation.plx";
	public static final String OUTFILE = "ArrayVariation.ple";
	
	public static void main(String[] args) throws ParserConfigurationException, SAXException, IOException {
		Element rootNode = XMLIO.readToNode(INFILE);
		NodeModel plan = new NodeModel(new BaseModel(rootNode.getFirstChild(), null, 0));
		String decompilation = plan.decompile(0);
		XMLIO.writeToXML(OUTFILE, decompilation);
	}

}
