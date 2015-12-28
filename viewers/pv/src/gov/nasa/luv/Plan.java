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

// *** TODO: Resolve library load/update issues:
// - Don't update if plan is executing
// - Update implies plan reset
// - Update library means update transitive closure of callers

package gov.nasa.luv;

import java.io.File;
import java.util.Collection;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Properties;
import java.util.Set;
import java.util.Vector;

import static gov.nasa.luv.Constants.AppType;
import static gov.nasa.luv.Constants.AppType.*;

public class Plan
    extends Properties
    implements Cloneable {
    private AppType appType;
    private String name;
    private File planFile;
    private File scriptFile;
    private File configFile;
    private File debugFile;
    private Vector<File> libraryPath;
    private Set<File> libraryFiles;

    // Members below this line should not be serialized
    private Date planLastModified;
    private Node rootNode;
    private Map<String, Vector<LibraryCallNode> > libraryCalls;

    public Plan() {
        appType = NO_APP;
        name = null;
        planFile = null;
        scriptFile = null;
        configFile = null;
        debugFile = null;
        libraryPath = new Vector<File>();
        libraryFiles = new HashSet<File>();

        planLastModified = null;
        rootNode = null;
        libraryCalls = new HashMap<String, Vector<LibraryCallNode> >();
    }

    private Plan(Plan orig) {
        appType = orig.appType;
        name = orig.name;
        planFile = orig.planFile;
        scriptFile = orig.scriptFile;
        configFile = orig.configFile;
        debugFile = orig.debugFile;
        libraryPath = new Vector<File>(orig.libraryPath);
        libraryFiles = new HashSet<File>(orig.libraryFiles);

        planLastModified = orig.planLastModified;
        rootNode = orig.rootNode.clone();
        libraryCalls = new HashMap<String, Vector<LibraryCallNode> >();
    }

    public Plan clone() {
        Plan result = new Plan(this);
        return result;
    }

    /**
     * @brief Merge the settings in another plan with the current settings.
     * @param p Another Plan instance.
     * @return true if this instance was updated, false if not.
    */
    public boolean merge(Plan p) {
        if (p == this || p == null)
            return false;
        boolean modified = false;
        if (p.name != null && !p.name.equals(name)) {
            name = p.name;
            modified = true;
        }
        if (p.rootNode != null && !p.rootNode.equivalent(rootNode)) {
            rootNode = p.rootNode;
            libraryCalls = p.libraryCalls;
            modified = true;
        }
        if (p.appType != NO_APP) {
            appType = p.appType;
            modified = true;
        }
        if (!p.libraryPath.isEmpty() && !p.libraryPath.equals(libraryPath)) {
            libraryPath = p.libraryPath;
            modified = true;
        }
        if (!p.libraryFiles.isEmpty() && !p.libraryFiles.equals(libraryFiles)) {
            libraryFiles = p.libraryFiles;
            modified = true;
        }
        if (p.scriptFile != null) {
            scriptFile = p.scriptFile;
            modified = true;
        }
        if (p.configFile != null) {
            configFile = p.configFile;
            modified = true;
        }
        if (p.debugFile != null) {
            debugFile = p.debugFile;
            modified = true;
        }
        if (p.planLastModified != null
            && (planLastModified == null || planLastModified.before(p.planLastModified))) {
            planLastModified = p.planLastModified;
            modified = true;
        }
        return modified;
    }

    public int hashCode() {
        int result = super.hashCode();
        result = result * 31 + getClass().getName().hashCode();
        result = result * 31 + appType.ordinal();
        result = result * 31 +
            (name == null ? 0 : name.hashCode());
        result = result * 31 +
            (planFile == null ? 0 : planFile.hashCode());
        result = result * 31 +
            (scriptFile == null ? 0 : scriptFile.hashCode());
        result = result * 31 +
            (configFile == null ? 0 : configFile.hashCode());
        result = result * 31 +
            (rootNode == null ? 0 : rootNode.hashCode());
        return result;
    }

    public boolean equals(Object o) {
        if (!(o instanceof Plan))
            return false;

        Plan other = (Plan) o;
        if (other == this)
            return true; // identity

        if (appType != other.appType)
            return false;

        if (name == null) {
            if (other.name != null) {
                // System.out.println("Not equivalent because name is null and other plan's isn't");
                return false;
            }
        }
        else if (other.name == null) {
            // System.out.println("Not equivalent because other plan's name is null and this plan's isn't");
            return false;
        }
        else if (!name.equals(other.name)) {
            // System.out.println("Not equivalent because plan names differ");
            return false;
        }
        
        if (planFile == null) {
            if (other.planFile != null) {
                // System.out.println("Not equivalent because plan file is null and other plan's isn't");
                return false;
            }
        }
        else if (other.planFile == null) {
            // System.out.println("Not equivalent because other plan file is null and this plan's isn't");
            return false;
        }
        else if (!planFile.equals(other.planFile)) {
            // System.out.println("Not equivalent because plan files differ");
            return false;
        }

        if (scriptFile == null) {
            if (other.scriptFile != null) {
                // System.out.println("Not equivalent because script file is null and other plan's isn't");
                return false;
            }
        }
        else if (other.scriptFile == null) {
            // System.out.println("Not equivalent because other plan's script file is null and this plan's isn't");
            return false;
        }
        else if (!scriptFile.equals(other.scriptFile)) {
            // System.out.println("Not equivalent because script files differ");
            return false;
        }

        if (configFile == null) {
            if (other.configFile != null) {
                // System.out.println("Not equivalent because config file is null and other plan's isn't");
                return false;
            }
        }
        else if (other.configFile == null) {
            // System.out.println("Not equivalent because other plan's config file is null and this plan's isn't");
            return false;
        }
        else if (!configFile.equals(other.configFile)) {
            // System.out.println("Not equivalent because config files differ");
            return false;
        }

        if (debugFile == null) {
            if (other.debugFile != null) {
                // System.out.println("Not equivalent because debug file is null and other plan's isn't");
                return false;
            }
        }
        else if (other.debugFile == null) {
            // System.out.println("Not equivalent because other plan's debug file is null and this plan's isn't");
            return false;
        }
        else if (!debugFile.equals(other.debugFile)) {
            // System.out.println("Not equivalent because debug files differ");
            return false;
        }

        if (rootNode == null) {
            if (other.rootNode != null) {
                // System.out.println("Not equivalent because root node is null and other plan's isn't");
                return false;
            }
        }
        else if (other.rootNode == null) {
            // System.out.println("Not equivalent because other plan's root node is null and this plan's isn't");
            return false;
        }
        else if (!rootNode.equals(other.rootNode))
            return false;

        return super.equals(o);
    }

    public AppType getAppType() {
        return appType;
    }

    public void setAppType(AppType a) {
        appType = a;
    }

    public File getPlanFile() {
        return planFile;
    }

    public void setPlanFile(File f) {
        planFile = f;
    }

    public File getScriptFile() {
        return scriptFile;
    }

    public void setScriptFile(File f) {
        scriptFile = f;
    }

    public File getConfigFile() {
        return configFile;
    }

    public void setConfigFile(File f) {
        configFile = f;
    }

    public File getDebugFile() {
        return debugFile;
    }

    public void setDebugFile(File f) {
        debugFile = f;
    }

    public String getName() {
        if (name != null)
            return name;
        else if (rootNode != null)
            return (name = rootNode.getNodeName());
        else
            return null;
    }

    public void setName(String s) {
        name = s;
    }

    public Vector<File> getLibraryPath() {
        return libraryPath;
    }

    public void setLibraryPath(Collection<File> path) {
        libraryPath.clear();
        libraryPath.addAll(path);
    }

    public void addLibraryPath(File f) {
        libraryPath.add(f);
    }

    public Set<File> getLibraryFiles() {
        return libraryFiles;
    }

    public void setLibraryFiles(Collection<File> files) {
        libraryFiles.clear();
        libraryFiles.addAll(files);
    }

    public void addLibraryFile(File f) {
        libraryFiles.add(f);
    }

    public Node getRootNode() {
        return rootNode;
    }

    public void setRootNode(Node n) {
        rootNode = n;
    }

    public Date getLastModified() {
        return planLastModified;
    }

    public void setLastModified(Date d) {
        planLastModified = d;
    }

    public boolean hasLibraryCalls() {
        return !libraryCalls.isEmpty();
    }

    //* Returns the names of all Libraries called by this Plexil plan.
    public Collection<String> getLibraryNames() {
        return libraryCalls.keySet();
    }

    /**
     * Logs a reference to a library node, to be resolved later.
     * 
     * @param libName The name of the library
     * @param referrer The LibraryNodeCall node
     */
    public void addLibraryReference(String libName, LibraryCallNode referrer) {
        Vector<LibraryCallNode> referrers = libraryCalls.get(libName);
        if (referrers == null) {
            // First call to this library in this plan
            referrers = new Vector<LibraryCallNode>();
            libraryCalls.put(libName, referrers);
            // Tell model
            RootModel.addLibraryCaller(libName, this);
        }
        referrers.add(referrer);
    }

    /**
     * Notifies the plan that a library it depends on was loaded or updated.
     * 
     * @param libName The name of the found library
     * @param library The library itself, with all dependent library references resolved.
     */

    public void libraryLoaded(String libName, Plan library) {
        Vector<LibraryCallNode> referrers = libraryCalls.get(libName);
        if (referrers == null)
            return;

        for (LibraryCallNode n : referrers)
            n.linkLibrary(library);
    }

    /**
     * Attempt to link all libraries referenced, recursively.
     * @return True if all references were satisfied, false otherwise.
     * @note Library references within libraries are not resolved until the
     *       library is linked into an active plan.
     */

    public boolean resolveLibraryCalls() {
        if (libraryCalls.isEmpty())
            return true; // trivial case

        // Ensure explicitly named libraries are loaded
        RootModel.loadPlanLibraryFiles(this);

        boolean result = true;
        for (Map.Entry<String, Vector<LibraryCallNode> > e : libraryCalls.entrySet()) {
            Plan lib = RootModel.findLibrary(e.getKey(), this);
            if (lib == null) {
                result = false;
                continue;
            }

            // TODO: add library's file to our list

            // Found it
            // Clone it and try to resolve its library calls
            lib = lib.resolveDependentLibraries();
            if (lib == null) {
                result = false;
                continue;
            }

            // If we were successful, link it
            for (Node n : e.getValue())
                n.linkLibrary(lib);
        }
        return result;
    }

    public boolean hasUnresolvedLibraryCalls() {
        if (libraryCalls.isEmpty())
            return false;
        for (Vector<LibraryCallNode> callers : libraryCalls.values())
            for (LibraryCallNode caller : callers)
                if (caller.getChildCount() == 0)
                    return true;
        return false;
    }

    /**
     * Resolve any library dependencies in this library before linking into another plan
     * @param lib The library to be linked.
     * @return If successful, a copy of the library with all dependencies resolved;
     *         if not, null.
     */ 

    private Plan resolveDependentLibraries() {
        if (libraryCalls.isEmpty())
            return this;
        Plan lib = clone(); // always mount a scratch monkey
        if (lib.resolveLibraryCalls())
            return lib;
        else
            return null;
    }

    // Defer to root
    public void reset() {
        if (rootNode != null)
            rootNode.reset();
    }
        

}
