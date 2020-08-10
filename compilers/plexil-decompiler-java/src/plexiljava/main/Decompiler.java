package plexiljava.main;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Element;
import org.xml.sax.SAXException;

import plexiljava.commons.xml.XMLIO;
import plexiljava.model.BaseModel;
import plexiljava.model.BaseModel.PatternRecognitionFailureException;
import plexiljava.model.NodeModel;

public class Decompiler {
	
	public static Logger logger = Logger.getLogger(Decompiler.class.getName());
	public static boolean VERBOSE = false;
	public static boolean FORCE = false;
	
	public static String infileName = null;
	public static String outfileName = null;
	
	public static void main(String[] args) throws ParserConfigurationException, SAXException, IOException, PatternRecognitionFailureException {
		List<String> argList = new ArrayList<String>();
		for( String arg : args ) {
			if( arg.startsWith("-") ) {
				switch( arg.charAt(1) ) {
					case 'd':
						VERBOSE = true;
						break;
					case 'f':
						FORCE = true;
						break;
					default:
						logger.setLevel(Level.WARNING);
						logger.warning("Unrecognized flag: " + arg + "\nOperation aborted.");
						return;
				}
			} else {
				argList.add(arg);
			}
		}
		if( args.length < 2 ) {
			logger.setLevel(Level.WARNING);
			logger.warning("usage: [options] file_to_decompile output_file\n  options:\n    -d\tPrint debug statements\n    -f\tForce decompile with errors\nWould you like to input these filepaths manually? (y/n): ");
			Scanner sc = new Scanner(System.in);
			String response = sc.nextLine();
			if( !(response.startsWith("y") || response.startsWith("Y")) ) {
				logger.setLevel(Level.INFO);
				logger.info("Operation aborted.");
				return; 
			}
			logger.setLevel(Level.INFO);
			logger.info("file_to_decompile: ");
			infileName = sc.nextLine();
			logger.setLevel(Level.INFO);
			logger.info("output_file: ");
			outfileName = sc.nextLine();
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
				return;
			}
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
				return;
			}
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
