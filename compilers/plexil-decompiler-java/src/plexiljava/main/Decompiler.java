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
import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.BaseModel.PatternRecognitionFailureException;
import plexiljava.model.NodeModel;

public class Decompiler {

    public static Logger logger = Logger.getLogger(Decompiler.class.getName());
    public static boolean VERBOSE = false;
    public static boolean UNSAFE = true;

    public static String infileName = null;
    public static String outfileName = null;

    public static void main(String[] args) throws ParserConfigurationException, SAXException, IOException, PatternRecognitionFailureException {
        List<String> argList = new ArrayList<String>();
        for( String arg : args ) {
            if( arg.startsWith("-") ) {
                switch( arg.charAt(1) ) {
                case 'v':
                    VERBOSE = true;
                    break;
                case 's':
                    UNSAFE = false;
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
        if( argList.size() < 1 ) {
            logger.setLevel(Level.WARNING);
            logger.warning("usage: [options] file_to_decompile [output_file]\n  options:\n    -v\tPrint all debug statements\n    -s\tDo not decompile if errors exist");

            Scanner sc = new Scanner(System.in);
            logger.setLevel(Level.INFO);
            logger.info("file_to_decompile: ");
            infileName = sc.nextLine();
            logger.setLevel(Level.INFO);
            logger.info("output_file (blank for default): ");
            outfileName = sc.nextLine();
        }
        if( infileName == null ) {
            infileName = argList.get(0);
        }
        if( !infileName.endsWith(".plx") ) {
            logger.setLevel(Level.SEVERE);
            logger.severe("Unsupported file type (must be a .plx file)!");
            return;
        }
        if( outfileName == null || outfileName.equals("") ) {
            if( argList.size() > 1 ) {
                outfileName = argList.get(1);
            } else {
                outfileName = infileName.substring(0, infileName.length()-4) + ".ple";
            }
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
        try {
            XMLIO.deformatFile(infile);
            logger.setLevel(Level.INFO);
            logger.info("Decompiling " + infile + "...");
            infile = new File("_plexildformatted");
            Element rootNode = XMLIO.readToNode(infile);
            NodeModel planRoot = new NodeModel(new BaseModel(rootNode, null, 0));
            String decompilation = "";
            if( planRoot.getChildren().size() > 1 ) {
                for( BaseModel child : planRoot.getChildren() ) {
                    decompilation += child.decompile(0) + "\n";
                }
            } else {
                decompilation += planRoot.getChildren().get(0).decompile(0);
                //decompilation += (new NodeModel(new BaseModel(rootNode.getFirstChild(), null, 0))).decompile(0);
            }
            decompilation = decompilation.replaceAll("\\n[^\\n]*" + DecompilableStringBuilder.REFERENCE_IDENTIFIER + "[^\\n]*\\n", "\n");
            decompilation = decompilation.replace(" , ", ", ");
            XMLIO.writeToXML(outfileName, decompilation);
            infile.delete();
        } catch(PatternRecognitionFailureException e) {
            logger.setLevel(Level.SEVERE);
            logger.severe(e.getMessage());
        }
    }

}
