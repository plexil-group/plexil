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
import java.io.InterruptedIOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Properties;
import java.util.Set;
import java.util.Stack;
import java.util.Vector;

import static gov.nasa.luv.Constants.*;

/**
 * The Model class represents a model of a Plexil Plan.
 */

public class Model
    extends Node
    implements Cloneable {

    //
    // Private constants
    //

    private static Model TheRootModel;
    private File planFile;
    private File scriptFile;
    private File configFile;
    private LinkedHashSet<File> libraryFiles;
    private LinkedHashSet<String> missingLibraryNodes; // set of library node names
    
    /**
     * Constructs a Plexil Model with the specified type (usually the XML tag) 
     * which identifies what kind of thing this model represents.  All other 
     * features of the model are stored in properties and children.
     * 
     * @param type the type of model or node this Model is
     */
    // *** FIXME: replace this with specialized EmptyModel subclass? ***
    public Model()
    {
        super(null, -1);
        commonInit();
    }
    
    /**
     * Constructs a Plexil Model with the specified type (usually the XML tag) 
     * which identifies what kind of thing this model represents and the row
     * number this Model lies on in the Luv application viewer. All other 
     * features of the model are stored in properties and children.
     * 
     * @param type the type of model or node this Model is
     * @param row the row number this Model is on in the viewer
     */
    public Model(String type, int row)
    {
        super(type, row);
        commonInit();
    }

    private void commonInit() {
        planFile = null;
        scriptFile = null;
        configFile = null;
        libraryFiles = new LinkedHashSet<File>();
        missingLibraryNodes = new LinkedHashSet<String>();
    }


    /**
     * Copy Constructor
     * @param node
     */
    public Model(Model orig) {
        super(orig);
        planFile = orig.planFile;
        scriptFile = orig.scriptFile;
        configFile = orig.configFile;
        libraryFiles = orig.libraryFiles;
        missingLibraryNodes =
            new LinkedHashSet<String>(orig.missingLibraryNodes);
    }

    public Model clone() {
        return new Model(this);
    }

    /** Returns the full path of the Plexil Plan of this Plexil Model/node.
     *  @return the full path of the Plexil Plan of this Plexil Model/node */
    public File                       getPlanFile()       { return planFile; }
    /** Returns the full path of the Plexil Script of this Plexil Model/node.
     *  @return the full path of the Plexil Script of this Plexil Model/node */
    public File                       getScriptFile()     { return scriptFile; }
    /** Returns the full path of the Plexil config file of this Plexil Model/node.
     *  @return the full path of the Plexil config file of this Plexil Model/node */
    public File                       getConfigFile()     { return scriptFile; }

    /** Returns the Set of Library Names for this Plexil Model/node.
     *  @return the Set of Library Names for this Plexil Model/node */
    public Set<File>                  getLibraryFiles()           { return libraryFiles; }
    /** Returns the Set of missing Libraries for this Plexil Model/node.
     *  @return the Set of missing Libraries for this Plexil Model/node */
    public Set<String>                  getMissingLibraries()       { return missingLibraryNodes; }
    
    /**
     * Returns the Plexil Plan name without the path.
     * @return the Plexil Plan name
     */
    public String getPlanName()
    {
        if (planFile == null)
            return null;
        return planFile.getName();
    }
    
    /**
     * Returns the Plexil Script name without the path.
     * @return the Plexil Script name
     */
    public String getScriptName()
    {
        if (scriptFile == null)
            return null;
        return scriptFile.getName();
    }
    
    /**
     * Returns the distinguished root node of the Plexil model.
     * @return the root Model
     * @note The root Model is special -
     *       its children are all Models, each representing a plan or library.
     */
    public static Model getRoot()
    {
        if (TheRootModel == null) {
            TheRootModel = new Model();
            TheRootModel.setNodeName("_The_Root_Model_");
        }
        return TheRootModel;
    }

    /**
     * Specifies the Plexil Plan name for this Model.
     * @param planFile the Plexil Plan name
     */
    public void setPlanFile(File name) {
        planFile = name;
        for (ChangeListener l : changeListeners)
            l.planFileAdded(this, name);
    }
      
    /**
     * Specifies the Plexil Script name for this Model.
     * @param scriptFile the Plexil Plan name
     */
    public void setScriptFile(File f)
    {
        scriptFile = f;
        for (ChangeListener l : changeListeners)
            l.scriptFileAdded(this, f);
    }

    public void setConfigFile(File f) {
        configFile = f;
    }

    /**
     * Specifies the Plexil Library name for this Model.
     * @param libraryName the Plexil Library name
     */
    public void addLibraryFile(File f)
    {
        if (libraryFiles.add(f)) {
            for (ChangeListener l : changeListeners)
                l.libraryFileAdded(this, f);
        }
    }

    /**
     * Notifies the top level node that a library was not found and adds the
     * library to the list of missing libraries.
     * 
     * @param nodeName the name of the missing library
     */
    public void addMissingLibrary(String nodeName)
    {
        missingLibraryNodes.add(nodeName);
    }

    /**
     * Notifies the top level node that a library was found and removes the
     * library from the list of missing libraries.
     * 
     * @param nodeName the name of the found library
     */
    public boolean missingLibraryFound(String nodeName)
    {
        return missingLibraryNodes.remove(nodeName);
    }
    
    /**
     * Compares this Model with the specified Node to see if they are the same.
     * @param other the node to compare with
     * @return whether or not the two are equivalent
     */
    @Override
    public boolean equivalent(Node other) {
        if (!(other instanceof Model))
            return false;
        if (!super.equivalent(other))
            return false;

        Model otherModel = (Model) other;
        if (libraryFiles.size() != otherModel.libraryFiles.size())
            return false;

        Iterator itr1 = libraryFiles.iterator();
        Iterator itr2 = otherModel.libraryFiles.iterator();
        while (itr1.hasNext() && itr2.hasNext())
            if (!itr1.next().equals(itr2.next()))
                return false;
        
        return true;
    }

    /**
     * Signals that a new Plexil Plan has been installed under this Model.
     */
    public void planChanged() {
        for (ChangeListener l : changeListeners)
            l.planChanged(this);
    }

}
