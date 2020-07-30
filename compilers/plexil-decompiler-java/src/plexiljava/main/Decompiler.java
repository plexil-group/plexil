package plexiljava.main;

import java.io.IOException;

import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Element;
import org.xml.sax.SAXException;

import plexiljava.commons.xml.XMLIO;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class Decompiler {

	public static final String INFILE = "examples/basic/AddArray.plx";
	public static final String OUTFILE = "example.ple";
	
	public static void main(String[] args) throws ParserConfigurationException, SAXException, IOException {
		Element rootNode = XMLIO.readToNode(INFILE);
		NodeModel planRoot = new NodeModel(new BaseModel(rootNode, null, 0));
		String decompilation = "";
		if( planRoot.getChildren().size() > 1 ) {
			for( BaseModel child : planRoot.getChildren() ) {
				decompilation += child.decompile(0) + "\n";
			}
			//decompilation += (new NodeModel(new BaseModel(rootNode.getLastChild(), null, 0))).decompile(0);
		} else {
			decompilation += (new NodeModel(new BaseModel(rootNode.getFirstChild(), null, 0))).decompile(0);
		}
		XMLIO.writeToXML(OUTFILE, decompilation);
	}

}
