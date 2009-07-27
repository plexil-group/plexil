/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
*  All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the Universities Space Research Association nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
* TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

package gov.nasa.luv;

import java.util.ArrayList;
import static gov.nasa.luv.Constants.*;

/** 
 * The RegexModelFilter class provides methods to fileter a Plexil Model.
 */

public class RegexModelFilter extends AbstractModelFilter
{
    // list of elements to filter from view    
    private ArrayList<String> listOfRegex;

    /**
     * Constructs a RegexModelFilter with the specified value to 
     * enable or disable the filtering.
     * 
     * @param enabled the value that either enables or disables the filtering
     */
    public RegexModelFilter(boolean enabled)
    {
        super(enabled);
        listOfRegex = new ArrayList<String>();
    }

    /** {@inheritDoc} */
    public boolean isFiltered(Model model)
    {
        String type = model.getProperty(NODETYPE_ATTR, UNKNOWN);
        String value = model.getModelName();
        
        if (value == null && type.equals(UNKNOWN))
            return false;
        else
        {
            if (Luv.getLuv().getProperty(type).equals("HIDE"))
                return true;
            
            for (String regex : listOfRegex)
            {
                if (value.matches(regex))
                    return true;
            }
        }

        return false;
    }
    
    /**
     * Adds the specified element to filter the Plexil Model with.
     * 
     * @param regex the new element to be filtered
     */
    public void addRegex(String regex)
    {
        String list = Luv.getLuv().getProperties().getProperty(PROP_HIDE_SHOW_LIST, UNKNOWN);      
        
        if (list.equals(UNKNOWN) || list.equals(""))
            list = regex;
        else
            list += ", " + regex;
        
        Luv.getLuv().getProperties().setProperty(PROP_HIDE_SHOW_LIST, list);

        listOfRegex.add(formatRegex(regex));
        
        Luv.getLuv().getViewHandler().refreshRegexView();  
    }
    
    /**
     * Removes the specified filtering element from the Plexil Model.
     * 
     * @param regex the element to be removed
     */
    public void removeRegex(String regex)
    {
        String list = Luv.getLuv().getProperties().getProperty(PROP_HIDE_SHOW_LIST, UNKNOWN);     
        
        if (!list.equals(UNKNOWN))
        {
            String [] array = list.split(", ");
            list = "";
            for (int i = 0; i < array.length; i++)
            {
                if (!array[i].equals(regex) && !array[i].equals(""))
                    list += array[i] + ", ";
            }
        }
        Luv.getLuv().getProperties().setProperty(PROP_HIDE_SHOW_LIST, list);
        
        listOfRegex.remove(formatRegex(regex));
        
        Luv.getLuv().getViewHandler().refreshRegexView();  
    }
    
    /**
     * Updates the list of elements that filter the Model, which occurs when
     * the Luv application is opened and looks to the saved list of regex
     * in the Luv properties file.
     */
    public void updateRegexList()
    {
        String namelist = Luv.getLuv().getProperties().getProperty(PROP_HIDE_SHOW_LIST, UNKNOWN);
        
        if (!namelist.equals(UNKNOWN) && !namelist.equals(""))
        {
            String [] array = namelist.split(", ");
            for (int i = 0; i < array.length; i++)
            {
                listOfRegex.add(formatRegex(array[i]));
            }
        }
    }

    public void extendedPlexilView()
    {
        Luv.getLuv().setProperty(AUX, "HIDE");
        Luv.getLuv().setProperty(AUX_THEN, "HIDE");
        Luv.getLuv().setProperty(AUX_ELSE, "HIDE");
    }

    public void corePlexilView()
    {
        Luv.getLuv().setProperty(AUX, "SHOW");
        Luv.getLuv().setProperty(AUX_THEN, "SHOW");
        Luv.getLuv().setProperty(AUX_ELSE, "SHOW");
    }
    
    private String formatRegex(String regex)
    {  
        String formattedRegex = "";

        if (regex.endsWith("*"))
        {
            //*regex* --> .*regex.*
            if (regex.startsWith("*"))
            {
                formattedRegex = regex.substring(1, regex.length() - 1);
                formattedRegex = ".*" + formattedRegex + ".*";
            }
            //regex* --> ^regex.*
            else
            {
                formattedRegex = regex.substring(0, regex.length() - 1);
                formattedRegex = "^" + formattedRegex + ".*";
            }
        }
        //*regex --> .*regex
        else if (regex.startsWith("*"))
        {
            formattedRegex = regex.substring(1, regex.length());
            formattedRegex = ".*" + formattedRegex;
        }
        //regex --> regex
        else
            formattedRegex = regex;

        return formattedRegex;
    }
}

