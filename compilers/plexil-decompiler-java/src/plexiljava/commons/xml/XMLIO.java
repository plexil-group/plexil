package plexiljava.commons.xml;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.SAXException;

public class XMLIO {

    public static void deformatFile(File infile) {
        StringBuffer unformatted = new StringBuffer();
        try {
            FileReader fr = new FileReader(infile);
            BufferedReader br = new BufferedReader(fr);
            String line;
            while((line=br.readLine())!=null) {
                line = line.trim();
                unformatted.append(line);
            }
            br.close();
            fr.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        try {
            FileWriter fw = new FileWriter("_plexildformatted");
            fw.write(unformatted.toString());
            fw.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static Element readToNode(File infile) throws ParserConfigurationException, SAXException, IOException {
        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
        DocumentBuilder db = dbf.newDocumentBuilder();
        Document doc = db.parse(infile);

        return doc.getDocumentElement();
    }

    public static void writeToXML(String filename, String content) {
        try {
            FileWriter fw = new FileWriter(filename);
            fw.write(content);
            fw.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

}
