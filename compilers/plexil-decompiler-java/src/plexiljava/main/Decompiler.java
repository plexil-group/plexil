package plexiljava.main;

import java.io.File;
import java.io.IOException;
import java.util.Scanner;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Element;
import org.xml.sax.SAXException;

import plexiljava.commons.xml.XMLIO;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class Decompiler {
	
	public static Logger logger = Logger.getLogger(Decompiler.class.getName());
	
	public static void main(String[] args) throws ParserConfigurationException, SAXException, IOException {
		String infileName = null;
		String outfileName = null;
		if( args.length < 2 ) {
			logger.setLevel(Level.WARNING);
			logger.warning("usage: file_to_decompile output_file\nWould you like to input these filepaths manually? (y/n): ");
			Scanner sc = new Scanner(System.in);
			String response = sc.nextLine();
			if( !(response.startsWith("y") || response.startsWith("Y")) ) {
				logger.setLevel(Level.INFO);
				logger.info("Operation aborted.");
				sc.close();
				return;
			}
			logger.setLevel(Level.INFO);
			logger.info("file_to_decompile: ");
			infileName = sc.nextLine();
			logger.setLevel(Level.INFO);
			logger.info("output_file: ");
			outfileName = sc.nextLine();
			sc.close();
		}
		if( infileName == null ) {
			infileName = args[0];
		}
		if( outfileName == null ) {
			outfileName = args[1];
		}
		if( infileName.equals(outfileName) ) {
			logger.setLevel(Level.WARNING);
			logger.warning("The output file will replace the contents of the input file.\nAre you sure you wish to continue? (y/n): ");
			Scanner sc = new Scanner(System.in);
			String response = sc.nextLine();
			if( !(response.startsWith("y") || response.startsWith("Y")) ) {
				logger.setLevel(Level.INFO);
				logger.info("Operation aborted.");
				sc.close();
				return;
			}
			sc.close();
		}
		File infile = new File(infileName);
		if( !infile.exists() ) {
			logger.setLevel(Level.SEVERE);
			logger.severe("Infile doesn't exist!");
			return;
		}
		File outfile = new File(outfileName);
		if( outfile.exists() ) {
			logger.setLevel(Level.WARNING);
			logger.warning("Outfile already exists; this operation will override the existing outfile.\nAre you sure you wish to continue? (y/n): ");
			Scanner sc = new Scanner(System.in);
			String response = sc.nextLine();
			if( !(response.startsWith("y") || response.startsWith("Y")) ) {
				logger.setLevel(Level.INFO);
				logger.info("Operation aborted.");
				sc.close();
				return;
			}
			sc.close();
		}
		Element rootNode = XMLIO.readToNode(infile);
		NodeModel planRoot = new NodeModel(new BaseModel(rootNode, null, 0));
		String decompilation = "";
		if( planRoot.getChildren().size() > 1 ) {
			for( BaseModel child : planRoot.getChildren() ) {
				decompilation += child.decompile(0) + "\n";
			}
		} else {
			decompilation += (new NodeModel(new BaseModel(rootNode.getFirstChild(), null, 0))).decompile(0);
		}
		XMLIO.writeToXML(outfileName, decompilation);
		logger.setLevel(Level.INFO);
		logger.info("Operation completed.");
	}

}
