package plexiljava.main;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Scanner;

import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import plexiljava.commons.xml.XMLIO;
import plexiljava.commons.xml.XMLPlanParsing;

public class XMLNavigator {

    // TODO: B/DFS Locate, Selected View

    public static final String FILENAME = "example.plx";
    public static final String HELPDOC = "cd	Change Directory\nls	List Children\nattr	List Attributes";

    public static String getNodeString(Node node) {
        return node.getNodeName() + ":\t" + node.getNodeValue();
    }

    public static List<Node> getNodeChildren(Node node) {
        List<Node> nodeChildren = new ArrayList<Node>();
        int index = 0;
        Node child = null;
        while( (child=children.item(index)) != null ) {
            nodeChildren.add(child);
            index++;
        }
        return nodeChildren;
    }

    public static void cdUp() {
        if( path.size() < 2 ) {
            System.out.println("Already at root directory!");
        } else {
            children = path.get(path.size()-2).getChildNodes();
            path.remove(path.size()-1);
        }
    }

    public static void cdDown(int targetIndex) {
        try {
            Node child = getNodeChildren(path.get(path.size()-1)).get(targetIndex);
            path.add(child);
            children = child.getChildNodes();
        } catch( IndexOutOfBoundsException e ) {
            System.out.println("Invalid target!");
        }
    }

    public static boolean handleCommand(String command) {
        if( command.equals("exit") ) {
            return false;
        } else if( command.startsWith("cd ") ) {
            String target = command.substring(3);
            if( target.equals("..") ) {
                cdUp();
            } else {
                try {
                    int targetIndex = Integer.parseInt(target);
                    cdDown(targetIndex);
                } catch( NumberFormatException e ) {
                    System.out.println("Target must be an index or '..'");
                }
            }
        } else if( command.equals("ls") ) {
            List<Node> nodeChildren = getNodeChildren(path.get(path.size()-1));
            for( int f=0; f<nodeChildren.size(); f++ ) {
                Node child = nodeChildren.get(f);
                System.out.println(f + "\t" + child.getNodeName());
            }
        } else if( command.equals("attr") ) {
            List<Node> attributes = XMLPlanParsing.getAttributes(path.get(path.size()-1));
            for( int f=0; f<attributes.size(); f++ ) {
                Node attribute = attributes.get(f);
                System.out.println(getNodeString(attribute));
            }
        } else if( command.equals("qual") ) {
            if( qualityMap.containsKey(path.get(path.size()-1)) ) {
                List<Node> qualities = qualityMap.get(path.get(path.size()-1));
                for( Node quality : qualities ) {
                    System.out.println(quality.getNodeName() + ":\t" + quality.getFirstChild().getNodeValue());
                }
            }
        } else {
            System.out.println("Unrecognized command.");
        }
        System.out.print(getPath());
        return true;
    }

    public static String getPath() {
        String pathString = "~/";
        for( Node node : path ) {
            pathString += node.getNodeName() + "/";
        }
        pathString += ": ";
        return pathString;
    }

    public static List<Node> path = new ArrayList<Node>();
    public static NodeList children;
    public static Map<Node, List<Node>> qualityMap = new HashMap<Node, List<Node>>();

    public static void main(String[] args) throws SAXException, IOException, ParserConfigurationException {
        File origFile = new File(FILENAME);
        XMLIO.deformatFile(origFile);
        File infile = new File("_plexildformatted");
        Element plan = XMLIO.readToNode(infile);
        plan.normalize();
        XMLPlanParsing.qualifyNode(plan, qualityMap);
        children = plan.getChildNodes();
        path.add(plan);

        Scanner sc = new Scanner(System.in);
        System.out.print(getPath());
        while( handleCommand(sc.nextLine()) ) {}
        sc.close();
        infile.delete();
    }

}
