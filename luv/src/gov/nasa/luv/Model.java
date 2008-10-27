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
import java.util.Iterator;
import java.util.Vector;
import java.util.HashMap;
import java.util.Properties;
import java.util.Map.Entry;

import static gov.nasa.luv.Constants.*;

/**
 * The model of a Plexil plan.
 */

public class Model extends Properties
{
      /** the type (usually the XML tag) which identifies what kind of
       * thing this model represents.  All other features of the model
       * are stored in properties and children. */

      private String type = "<type undefined>";
      
      private String path = "";

    private String modelName = "";

      /** property change listeners registered for this model */ 

      private Vector<ChangeListener> changeListeners = new Vector<ChangeListener>();

      /** parent of this node */

      private Model parent;

      /** children of this node */

      private Vector<Model> children = new Vector<Model>();
      
      // condition info holders
      
      public HashMap<Integer, ArrayList> conditionMap = new HashMap<Integer, ArrayList>();
      
      static HashMap<String, String> typeLut = new HashMap<String, String>()
        {
           {
              put(NODELIST,        "node-list");
              put(COMMAND,         "command-node");
              put(ASSN,            "assignment-node");
              put(EMPTY,           "empty-node");
              put(FUNCCALL,        "function-call-node");
              put(UPDATE,          "update-node");
              put(LIBRARYNODECALL, "library-node");
           }
        };

    private static Model TheRootModel = null;

      /** Construct a Model.
       *
       * @param type string representation of the of object this is modeling
       */

      public Model(String type)
      {
         this.type = type;
      }
      
      public void setPath()
      {
          Model node = this;
          
          while (node.parent != null && !node.parent.isRoot())
          {
              path += "--->" + node.parent.modelName;
              node = node.parent;
          }
          
          path = modelName + path;
      }
      
      
      public String getPath()
      {
          return path;
      }

    public String getModelName()
    {
	return modelName;
    }

    public void setModelName(String name)
    {
	modelName = name;
    }

    public static Model getRoot()
    {
	if (TheRootModel == null) {
	    TheRootModel = new Model("dummy");
	    TheRootModel.setModelName("_The_Root_Model_");
	}
	return TheRootModel;
    }

    /** Clone a model.
       *
       * @return the clone of this model
       */

    // *** Is this complete?!? ***

      public Object clone()
      {
         Model clone = (Model)super.clone();

         // deeply clone the children of this model

         clone.children = new Vector<Model>();
         for (Model child: children)
             clone.addChild((Model)child.clone());

         // return the copy, is this ethical? :)
         
         return clone;
      }

    // Model-to-model comparison

    public boolean equivalent(Model other)
    {
	if (other == null)
	    return false;
	if (other == this)
	    return true;
	if (!modelName.equals(other.modelName))
	    return false;
	if (!type.equals(other.type))
	    return false;
	if (!path.equals(other.path))
	    return false;
	if (!super.equals(other)) // compare properties
	    return false;
	if (!childrenEquivalent(other))
	    return false;
	return true;
    }

    private boolean childrenEquivalent(Model other)
    {
	if (children.isEmpty()) {
	    if (other.children.isEmpty())
		return true;
	    else
		return false;
	}
	else if (other.children.isEmpty())
	    return false;
	else if (children.size() != other.children.size())
	    return false;
	for (int i = 0; i < children.size(); i++) {
	    if (!children.get(i).equivalent(other.children.get(i)))
		return false;
	}
	return true;
    }

      /** Accesor for model type.
       *
       * @return the type of the model
       */

      public final String getType()
      {
         return type;
      }

      /** Signal that a new plan has been installed under this model. */

      public void planChanged()
      {
         for (int i = 0; i < changeListeners.size(); ++i)
            changeListeners.get(i).planChanged(this);
      }

      /** Accesor for model children.
       *
       * @return the children of the model
       */

      public final Vector<Model> getChildren()
      {
         return children;
      }

      /** Create and add a child to this model.
       *
       * @param child child to add to this model
       */

      public void addChild(Model child)
      {
	  assert child.isNode();

	  children.add(child);
	  child.setParent(this);
      }
      
      public void addConditionInfo(int condition, ArrayList<String> equationHolder)
      { 
          if (!equationHolder.isEmpty())
              conditionMap.put(condition, equationHolder);
      }

      /** Add a parent node to this node. 
       *
       * @param parent node of which this is a child
       */

      public void setParent(Model newParent)
      {
         parent = newParent;
      }

      /** Test if a given model is the root of a tree.
       *
       * @return true if this model is the root of a model tree
       */

      public boolean isRoot()
      {
         return this == getRoot(); 
      }

      /** Get the parent node to this node. 
       *
       * @return the parent of this node or null if it is a root node
       */

      public Model getParent()
      {
         return parent;
      }

      /** Clear node of all children and properties. */

      public void clear()
      {
         removeChildren();
         super.clear();
         for (ChangeListener cl: changeListeners)
            cl.planCleared(this);
         setProperty(LIBRARY_COUNT, "0");
      }

      /** Specify the plan file name.
       *
       * @param planName name of plan file
       */

      public void addPlanName(String planName)
      {
         setProperty(FILENAME_ATTR, planName);
         for (int i = 0; i < changeListeners.size(); ++i)
            changeListeners.get(i).planNameAdded(this, planName);
      }
      
      /** Specify the script file name.
       *
       * @param scriptName name of script file
       */

      public void addScriptName(String scriptName)
      {
         setProperty(SCRIPT_FILENAME, scriptName);
         for (int i = 0; i < changeListeners.size(); ++i)
            changeListeners.get(i).scriptNameAdded(this, scriptName);
      }

      /** Get plan name recorded in this model.
       *
       * @return name of plan, which might be null
       */

      public String getPlanName()
      {
         return getProperty(FILENAME_ATTR);
      }

      /** Get script name recorded in this model.
       *
       * @return name of script, which might be null
       */

      public String getScriptName()
      {
         return getProperty(SCRIPT_FILENAME);
      }

      /** Specify a library name.
       *
       * @param libraryName name of library file
       */

      public void addLibraryName(String libraryName)
      {
         String countStr = getProperty(LIBRARY_COUNT);
         int count = (countStr != null)
            ? Integer.valueOf(countStr)
            : 0;

         setProperty(LIBRARY_FILENAME + "-" + ++count, libraryName);
         setProperty(LIBRARY_COUNT, "" + count);

         for (int i = 0; i < changeListeners.size(); ++i)
            changeListeners.get(i).libraryNameAdded(this, libraryName);
      }

      /** Get a list of library names recorded in this model.
       *
       * @return a vector of library names, which might be empty
       */

      public Vector<String> getLibraryNames()
      {
         Vector<String> libraries = new Vector<String>();
         String libName;
         int i = 1;

         while ((libName = getProperty(LIBRARY_FILENAME + "-" + i++)) != null)
            libraries.add(libName);

         return libraries;
      }

      /** Join all data from the two models together.
       *
       * @param other other node to join data into this one
       */

      public void join(Model other)
      {
         changeListeners.addAll(other.changeListeners);
         children.addAll(other.children);
         putAll(other);
      }

      /** Clear node of all children and properties. */

      public void removeChildren()
      {
         for (Model child: children)
            child.parent = null;
         children.clear();
      }
     
      public Model getChild(int i)
      {
         return children.get(i);
      }

      public int getChildCount()
      {
         return children.size();
      }
      
      public Model findChildByName(String name)
      {
          for (Model child: children)
              if (child.modelName.equals(name))
                  return child;
          return null;
      }

      /** Find all children of a given type. 
       *
       * @return a list of children of a given type.
       */

      public Vector<Model> findChildren(String type)
      {
         Vector<Model> matches = new Vector<Model>();
         for (Model child: children)
            if (child.type.equalsIgnoreCase(type))
               matches.add(child);
         return matches;
      }

      public Model findChild(String type)
      {
         for (Model child: children)
            if (child.type.equalsIgnoreCase(type))
               return child;
         return null;
      }

      public Model findChild(String property, String value)
      {
         for (Model child: children)
         {
            String prop = child.getProperty(property);
            if (prop != null && prop.equals(value))
               return child;
         }
         return null;
      }

      public Model removeChild(String type)
      {
         for (Model child: children)
            if (child.type.equalsIgnoreCase(type))
            {
               removeChild(child);
               return child;
            }
         return null;
      }


      public boolean removeChild(Model child)
      {
	  // Can't use Vector.remove(Object) because it uses equals(),
	  // and two models are often equal without being identical!
	  boolean removed = false;
	  for (Iterator<Model> it = children.iterator(); it.hasNext(); ) {
	      if (it.next() == child) {
		  it.remove();
		  child.parent = null;
		  removed = true;
		  break;
	      }
	  }
	  return removed;
      }

      /** Test whether two models have the same name. 
       *
       * @param other the model to test names with this one
       * @return true if this and the other model have the same name
       */

      public boolean hasSameName(Model other)
      {
         return modelName.equals(other.modelName);
      }

      public Object setProperty(String key, String value)
      {
         if (value == null)
         {
            System.err.println("key: " + key);
            System.err.println("type: " + type);
         }

         Object result = super.setProperty(key, value);

         if (value != result && value.equals(result) == false) {
            for (ChangeListener cl: changeListeners)
               cl.propertyChange(this, key);
	 }
         
         return result;
      } 

      public boolean isNode()
      {
         return type.equals(NODE);
      }

      public String toString()
      {
         return display();
      }

      public String display()
      {
         String name = getModelName();
         if (name == null)
            name = type;
         StringBuffer s = new StringBuffer(name);
         
         s.append("(");
         for (Entry<Object, Object> property: entrySet())
            s.append(" " + property.getKey() + " = " + property.getValue());
         s.append(")");
         
         if (children.size() > 0)
         {
            s.append("[");
            for (Model child: children)
            {
               s.append(child.toString());
               if (child != children.lastElement())
                  s.append(", ");
            }
            s.append("]");
         }
         return s.toString();
      }                        
            
      void setMainAttributesOfNode()
      {
         String rawType = getProperty(NODETYPE_ATTR);
         String polishedtype = rawType != null ? typeLut.get(rawType) : null;
         if (polishedtype == null)
            polishedtype = rawType;

         modelName = getProperty(NODE_ID);
         setProperty(MODEL_TYPE, polishedtype);
         setProperty(MODEL_OUTCOME, "UNKNOWN");
         setProperty(MODEL_STATE, "INACTIVE");
      }
            
      /** Add a property change listener to this model. 
       *
       * @param listener the listener which will be added to the set of
       * listeners which will be signaled when a property change occurs
       */

      public void addChangeListener(ChangeListener listener)
      {
         changeListeners.add(listener);
      }

      /** Remove a property change listener frome this model. 
       *
       * @param listener the listener which will be removed to the set
       * of listeners which will be signaled when a property change
       * occurs
       */

      public void removeChangeListener(ChangeListener listener)
      {
         changeListeners.remove(listener);
      }

      /** A listener which is signaled when a the model is changed in some way.
       */

      public abstract static class ChangeListener
      {
            abstract public void propertyChange(Model model, String property);

            abstract public void planCleared(Model model);

            abstract public void planChanged(Model model);

            abstract public void planNameAdded(Model model, String planName);
            
            abstract public void scriptNameAdded(Model model, String scriptName);

            abstract public void libraryNameAdded(Model model, String libraryName);
      }

      /** An adapter which is signaled when the model is changed in some way.
       */

      public static class ChangeAdapter extends ChangeListener
      {
            public void propertyChange(Model model, String property)
            {
            }

            public void planCleared(Model model)
            {
            }

            public void planChanged(Model model)
            {
            }

            public void planNameAdded(Model model, String planName)
            {
            }
            
            public void scriptNameAdded(Model model, String scriptName)
            {
            }

            public void libraryNameAdded(Model model, String libraryName)
            {
            }
      }
}
