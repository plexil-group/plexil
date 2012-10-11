package gov.nasa.luv;

import static gov.nasa.luv.Constants.PROP_ARRAY_MAX_CHARS;

import java.util.ArrayList;
import java.util.Iterator;

public class ArrayVariable extends Variable {

	private ArrayList<Variable> list;
	
	public ArrayVariable(String inOut, String name, String type, String value){
		super(inOut, name, type, value);
		String[] valueList = null;
    	int counter = 0;
    	if(!value.equals("UNKNOWN"))
    	{        		
    		valueList = value.replaceAll(",", "").split(" ");
    		counter = valueList.length;
    	}
    	String array_name = name.replaceFirst("\\[.*", "");
    	Integer index = Integer.parseInt(name.replaceAll("[^0-9]", ""));
    	list = new ArrayList<Variable>();
    	for(int i = 0; i < index; i++)
    	{
    		if(counter>0){
    			list.add(new Variable("--",array_name+"("+i+")",type,valueList[i] ));
    			counter--;
    		}
    		else
    			list.add(new Variable("--",array_name+"("+i+")",type,"U" ));
    	}    	
	}

	public void setVariable(Integer index, String value)
	{
		list.get(index).setValue(value);
	}
	
	public Variable getVariable(Integer index)
	{
		return list.get(index);
	}
	
	public String getBaseName()
	{
		return this.getName().replaceAll("[^a-zA-Z_0-9].*[^a-zA-Z_0-9]$", "");
	}
	
	public static String getBaseName(String input)
	{
		return input.replaceAll("[^a-zA-Z_0-9].*[^a-zA-Z_0-9]$", "");
	}

	public void setArrayIndexVariable(String input, String value)
	{
		String index = "";
		if(input.matches("[a-zA-Z_0-9]*[^a-zA-Z_0-9][0-9]*[^a-zA-Z_0-9]"))
			index = input.replaceAll("[^0-9]", "");
		else
		{
			String temp = input.replaceAll("^.*[^a-zA-Z_0-9]", "");
			index = temp.replaceAll("[^a-zA-Z_0-9]*$", "");
		}
		if(index.matches("[0-9]*") && !index.equals(""))					
			setVariable(Integer.parseInt(index), value);							
		updateList();
	}
	
	public int size()
	{
		return list.size();
	}
	
	public void updateList(){
		int length = 0;
		String arrayValue = "#(";
		Iterator<Variable> it = list.iterator();
		while(it.hasNext() && length < Luv.getLuv().getProperties().getInteger(PROP_ARRAY_MAX_CHARS))
		{			
			arrayValue+=it.next().getValue();
			if(it.hasNext())
			{				
				arrayValue+=", ";				
			}
			length = arrayValue.length();
		}
		if(length >= Luv.getLuv().getProperties().getInteger(PROP_ARRAY_MAX_CHARS))
			arrayValue+="...";
		arrayValue+=")";
		setValue(arrayValue);
	}
	
}
