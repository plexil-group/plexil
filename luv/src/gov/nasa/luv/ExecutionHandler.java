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

import static gov.nasa.luv.Constants.*;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

import static java.lang.System.*;

public class ExecutionHandler {
    
    public ExecutionHandler() {}
    
    public void runExec() throws IOException
    {  
        File ue = new File(PROP_UE_EXEC);         
        File plan = getPlan();
        File script = getScript();

        Execution exec = new Execution(ue, plan, script, null, null);

        try
        {
            InputStream is = exec.start();
            InputStream es = exec.getErrorStream();
            while (exec.isRunning())
            {
                while (is.available() > 0)
                    out.write(is.read());
                while (es.available() > 0)
                    err.write(es.read());
            }
            while (is.available() > 0)
                out.write(is.read());
            while (es.available() > 0)
                err.write(es.read());
            out.flush();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    public File getPlan() 
    {        
        while (Luv.getLuv().model.getPlanName() == null && Luv.plan == null)
            Luv.fileHandler.choosePlan();
        
        if (Luv.getLuv().model.getPlanName() != null)
            Luv.plan = new File(Luv.getLuv().model.getPlanName());

        return Luv.plan;  
    }

    public File getScript() throws IOException 
    {
        if (Luv.getLuv().model.getScriptName() != null)
            Luv.script = new File(Luv.getLuv().model.getScriptName());

        if (Luv.script == null || !Luv.script.canRead())
        {      
            String path = PROP_RECENT_FILES + "/scripts/";
            String name =  Luv.plan.getName().replace(".plx", "-script.plx");
            Luv.script = new File(path + name);    

            if (!Luv.script.canRead())
                Luv.script = Luv.fileHandler.findScriptOrCreateEmptyOne(Luv.script.getName(), Luv.plan.getName(),path);
        }

        Luv.getLuv().model.addScriptName(Luv.script.getName());  

        return Luv.script;
    }
}
