package plexiljava.commons.xml;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.w3c.dom.Node;

public class XMLPlanParsing {

    public static Map<Node, List<Node>> qualifyNode(Node node, Map<Node, List<Node>> qualityMap) {
        int index = 0;
        Node child;
        List<Node> consolidatedChildren = new ArrayList<Node>();
        while( (child=node.getChildNodes().item(index)) != null ) {
            if( child.getChildNodes().getLength() == 1 && child.getFirstChild().getNodeName().equals("#text")) {
                if( !qualityMap.containsKey(node) ) {
                    qualityMap.put(node, new ArrayList<Node>());
                }
                qualityMap.get(node).add(child);
                consolidatedChildren.add(child);
                System.out.println("Consolidated " + child.getNodeName() + " in " + node.getNodeName());
            } else {
                qualifyNode(child, qualityMap);
            }
            index++;
        }
        for( Node consolidated : consolidatedChildren ) {
            node.removeChild(consolidated);
        }
        return qualityMap;
    }

    public static List<Node> getAttributes(Node node) {
        List<Node> attributes = new ArrayList<Node>();
        int index = 0;
        Node attr = null;
        while( (attr=node.getAttributes().item(index)) != null ) {
            attributes.add(attr);
            index++;
        }
        return attributes;
    }

    public static Map<String, String> getAttributeMap(Node node) {
        Map<String, String> attributeMap = new HashMap<String, String>();
        List<Node> attributes = getAttributes(node);
        for( Node attribute : attributes ) {
            attributeMap.put(attribute.getNodeName(), attribute.getNodeValue());
        }
        return attributeMap;
    }

}
