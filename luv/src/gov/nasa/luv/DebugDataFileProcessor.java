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

import java.io.File;
import java.io.FileNotFoundException;
import java.util.HashMap;
import java.util.Scanner;

import static gov.nasa.luv.Constants.*;

public class DebugDataFileProcessor 
{
    private HashMap<String, String> lines = new HashMap<String, String>();
    private int countFirstTier = 0;
    private int countSecondTier = 0;
    private int countThirdTier = 0;
    
    private void processDebugFlags() throws FileNotFoundException 
    {
        try
        {
            Scanner scanner = new Scanner(new File(DEBUG_FLAG_DAT_FILE));
            try 
            {
                while (scanner.hasNextLine())
                {               
                    String line = scanner.nextLine().trim();                
                    assignTiersToLines(line);
                }             
            }
            finally 
            {
                scanner.close();
            }
        }
        catch (FileNotFoundException ex)
        {
            Luv.getLuv().displayErrorMessage(ex, "ERROR: " + DEBUG_FLAG_DAT_FILE + " not found");
        }         
    }
    
    private void assignTiersToLines(String line)
    {
        String[] array = line.split(":");
        
        if (array.length == 2 && !line.endsWith(":"))   
        {
            lines.put("A" + countFirstTier, line);
            countFirstTier++; 
            countSecondTier = 0;
        }
        else if (array.length == 2)   
        {
            if (noParent("A", array[1]))
            {
                lines.put("A" + countFirstTier, line);
                countFirstTier++;
                countSecondTier = 0;
            }
            else
            {
                lines.put("A" + (countFirstTier-1) + "B" + countSecondTier, line);
                countSecondTier++;
                countThirdTier = 0;
            }
        }
        else if (array.length == 3)                      
        {
            if (noParent("A" + (countFirstTier-1) + "B", array[1]))
            {
                lines.put("A" + (countFirstTier-1) + "B" + countSecondTier, line);
                countSecondTier++;
                countThirdTier = 0;
            }
            else
            {
                lines.put("A" + (countFirstTier-1) + "B" + (countSecondTier-1) + "C" + countThirdTier, line);
                countThirdTier++;
            }
        }
    }
    
    private boolean noParent(String parent, String word)
    {
        word = ":" + word + ":";
        
        for (int i = 0; i < lines.size(); i++)
        {
            String test = lines.get(parent + i);
            if (test != null && word.startsWith(test))
                return false;
        }
        
        return true;
    }
    
    public HashMap<String, String> getLines()
    {
        return lines;
    }
       
    public void run() throws FileNotFoundException
    {
        processDebugFlags();
    }
}

