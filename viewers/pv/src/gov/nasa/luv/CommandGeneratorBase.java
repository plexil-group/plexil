/* Copyright (c) 2006-2015, Universities Space Research Association (USRA).
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
import java.util.Collection;

// Shared logic for classes implementing interface CommandGenerator

public class CommandGeneratorBase {
    protected String getPlan(Settings s)
        throws CommandGenerationException {
        File p = s.getPlanLocation();
        if (p == null)
            throw new CommandGenerationException("Plan not set");
        return p.getAbsoluteFile().toString();
    }

    public boolean checkPlanFile(Settings s)
        throws CommandGenerationException {
        File p = s.getPlanLocation();
        if (p == null)
            throw new CommandGenerationException("No plan file specified");
        if (!p.exists())
            throw new CommandGenerationException("Plan file " + p.toString() + " not found");
        if (!p.isFile())
            throw new CommandGenerationException("Plan file " + p.toString() + " is not a plain file");
        if (!p.canRead())
            throw new CommandGenerationException("Plan file " + p.toString() + " is not readable");
        return true;
    }

    protected String getScript(Settings s)
        throws CommandGenerationException {
        File f = s.getScriptLocation();
        if (f == null)
            throw new CommandGenerationException("Script not set");
        return f.getAbsoluteFile().toString();
    }

    protected boolean checkScriptFile(Settings s)
        throws CommandGenerationException {
        File scr = s.getScriptLocation();
        if (scr == null)
            throw new CommandGenerationException("No script file specified");
        if (!scr.exists())
            throw new CommandGenerationException("Script file " + scr.toString() + " not found");
        if (!scr.isFile())
            throw new CommandGenerationException("Script file " + scr.toString() + " is not a plain file");
        if (!scr.canRead())
            throw new CommandGenerationException("Script file " + scr.toString() + " is not readable");
        return true;
    }

    protected String getConfig(Settings s)
        throws CommandGenerationException {
        File f = s.getConfigLocation();
        if (f == null)
            throw new CommandGenerationException("Config not set");
        return f.getAbsoluteFile().toString();
    }

    protected boolean checkConfigFile(Settings s)
        throws CommandGenerationException {
        File c = s.getConfigLocation();
        if (c == null)
            throw new CommandGenerationException("No config file specified");
        if (!c.exists())
            throw new CommandGenerationException("Config file " + c.toString() + " not found");
        if (!c.isFile())
            throw new CommandGenerationException("Config file " + c.toString() + " is not a plain file");
        if (!c.canRead())
            throw new CommandGenerationException("Config file " + c.toString() + " is not readable");
        return true;
    }

    protected Collection<File> getLibraryDirs(Settings s)
        throws CommandGenerationException {
        return s.getLibDirs();
    }

    protected Collection<String> getLibraryNames(Settings s)
        throws CommandGenerationException {
        return s.getLibs();
    }

}
