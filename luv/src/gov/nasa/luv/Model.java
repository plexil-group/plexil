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
      /** the type (usuall the XML tag) which identifies what kind of
       * thing this model represents.  All other features of the model
       * are stored in properties and children. */

      private String type = "<type undefined>";

      /** property change listners registered for this model */ 

      private Vector<ChangeListener> changeListeners = 
         new Vector<ChangeListener>();

      /** parent of this node */

      private Model parent;

      /** children of this node */

      private Vector<Model> children = new Vector<Model>();

      /** An array of model tailors to customize those which need it. */

      private Tailor[] tailors = 
      {
         new NodeTailor(),
         new NameFromProperty(NODE_OUTCOME_VAR, NODE_ID),
         new NameFromProperty(NODE_STATE_VAR, NODE_ID),
         new NameFromKeyProperty(STATE_VAL),
         new NameFromKeyProperty(STATE_NAME),
         new NameFromKeyProperty(INT_VAL),
         new NameFromKeyProperty(REAL_VAL),
         new NameFromKeyProperty(STRING_VAL),
         new BooleanFromKeyProperty(BOOL_VAL),
         new NameFromKeyProperty(TIME_VAL),
         new NameFromKeyProperty(BLOB_VAL),
         new NameFromKeyProperty(NODE_OUTCOME_VAL),
         new NameFromKeyProperty(NODE_FAILURE_VAL),
         new NameFromKeyProperty(NODE_STATE_VAL),
         new NameFromProperty(NODE_TIMEPOINT_VAL, NODE_ID),
         new CommandTailor(COMMAND, COMMAND_NAME),
         new DeclareTailor(DECL_VAR),
         new DeclareArrayTailor(DECL_ARRAY),
         new ArrayElementTailor(ARRAYELEMENT),
      };
      
      public HashMap<String, ArrayList> declVarMap = new HashMap<String, ArrayList>();
      
      public ArrayList<String> declNameVarList = new ArrayList<String>();
      public ArrayList<String> declTypeVarList = new ArrayList<String>();
      public ArrayList<String> declValueVarList = new ArrayList<String>();

      /** Table of model tailors to customize them as needed. */

      private HashMap<String, Tailor> makerMap = new HashMap<String, Tailor>()
      {
         {
            for (Tailor tailor: tailors)
               put(tailor.getKey(), tailor);
         }
      };

      /** Construct a Model.
       *
       * @param type string representation of the of object this is modeling
       */

      public Model(String type)
      {
         this.type = type;
      }

      /** Clone a model.
       *
       * @return the clone of this model
       */

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

      /** Accesor for model type.
       *
       * @return the type of the model
       */

      public final String getType()
      {
         return type;
      }

      /** Signel that a new plan has been installed under this model. */

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
         assert isChildType(child.type);
         children.add(child);
         child.setParent(this);
      }
      
      public void addLocalVariableName(String name, String value)
      {
          if (value == null)
              ;
          else if (name.equals(NAME))
          {
              declNameVarList.add(value);
              declVarMap.put(name, declNameVarList);              
          }
          else if (name.equals(TYPE))
          {
              declTypeVarList.add(value);
              declVarMap.put(name, declTypeVarList); 
          }
          else
          {
              declValueVarList.add(value); 
              declVarMap.put(VAL, declValueVarList); 
          }
      }

      /** Add a parent node to this node. 
       *
       * @param parent node of which this is a child
       */

      public void setParent(Model parent)
      {
         this.parent = parent;
      }

      /** Clear parent node of this node.  This makes this node a root
       * node. */

      public void clearParent()
      {
         this.parent = null;
      }

      /** Test if a given model is the root of a tree.
       *
       * @return true if this model is the root of a model tree
       */

      public boolean isRoot()
      {
         return (parent == null || parent.getType().equals("dummy"));
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
         setProperty(PLAN_FILENAME, planName);
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
         return getProperty(PLAN_FILENAME);
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
            child.clearParent();
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
               child.clearParent();
               children.remove(child);
               return child;
            }
         return null;
      }


      public void removeChild(Model child)
      {
         children.remove(child);
         child.setParent(null);
      }

      /** Test of two models have the same name. 
       *
       * @param other the model to test names with this one
       * @return true if this and the other model have the same name
       */

      public boolean hasSameName(Model other)
      {
         return getProperty(MODEL_NAME).equals(other.getProperty(MODEL_NAME));
      }

      public Object setProperty(String key, String value)
      {
         if (value == null)
         {
            System.err.println("key: " + key);
            System.err.println("type: " + type);
         }

         Object result = super.setProperty(key, value);

         if (value != result && value.equals(result) == false)
            for (ChangeListener cl: changeListeners)
               cl.propertyChange(this, key);
         
         return result;
      } 

      public static boolean isChildType(String type)
      {
         for (String childTag: CHILD_TAGS)
            if (childTag.equalsIgnoreCase(type))
               return true;
         return false;
      }

      public static boolean isProperty(String tag)
      {
         for (String propertyTag: PROPERTY_TAGS)
            if (propertyTag.equalsIgnoreCase(tag))
               return true;
         
         return false;
      }

      public String toString()
      {
         return display();
      }

      public String display()
      {
         String name = getProperty(MODEL_NAME);
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
      
      /** 
       * Tailor this model based on it's type.
       */

      public void tailor()
      {
         Tailor tailor = makerMap.get(type);
         if (tailor != null)
            tailor.tailor(this);
      }

      /**
       * Base class from which model tailors are derrived.
       */

      public static abstract class Tailor
      {
            String key;

            public Tailor(String key)
            {
               this.key = key;
            }

            String getKey()
            {
               return key;
            }
            
            abstract void tailor(Model model);
      }

      /**
       * Map a property into the model name.
       */

      public static class NameFromProperty extends Tailor
      {
            String propertyName;

            public NameFromProperty(String key, String propertyName)
            {
               super(key);
               this.propertyName = propertyName;
            }

            void tailor(Model model)
            {
               model.setProperty(MODEL_NAME, model.getProperty(propertyName));
            }
      }

      /**
       * Talor a "node" node;
       */

      public static class NodeTailor extends Tailor
      {
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

            String propertyName;

            public NodeTailor()
            {
               super(NODE);
            }
            
            void tailor(Model model)
            {
               String rawType = model.getProperty(NODETYPE_ATTR);
               String type = rawType != null ? typeLut.get(rawType) : null;
               if (type == null)
                  type = rawType;

               model.setProperty(MODEL_NAME, model.getProperty(NODE_ID));
               model.setProperty(MODEL_TYPE, type);
               model.setProperty(MODEL_OUTCOME, "UNKNOWN");
               model.setProperty(MODEL_STATE, "INACTIVE");

               // if this is a library node, get the library node name

               if (model.getProperty(NODETYPE_ATTR).equals(LIBRARYNODECALL))
               {
                  // find and remove the library call sub structure
                  
                  Model libcall = model.removeChild(LIBRARYNODECALL);
                  
                  // extract the node id from the removed lib call
                  // structure and stick it in those node under our own
                  // name
                  
                  model.setProperty(MODEL_LIBRARY_CALL_ID,
                                    libcall.getProperty(NODE_ID));
               }
            }
      };
      
      /**
       * Map model type property into the model name.
       */

      public static class NameFromKeyProperty extends NameFromProperty
      {
            public NameFromKeyProperty(String key)
            {
               super(key, key);
            }
      }

      /**
       * Collects and converts a boolean value. 
       */

      public static class BooleanProperty extends NameFromProperty
      {
            public BooleanProperty(String key, String propertyName)
            {
               super(key, propertyName);
            }

            void tailor(Model model)
            {
               String value = model.getProperty(propertyName);
               if (value.equals("0"))
                  value = TRUE;
               else if (value.equals("1"))
                  value = FALSE;
               model.setProperty(MODEL_NAME, value.toUpperCase());
            }
      }

      /**
       * Collects and converts a boolean value. 
       */

      public static class BooleanFromKeyProperty extends BooleanProperty
      {
            public BooleanFromKeyProperty(String key)
            {
               super(key, key);
            }
      }

      /**
       * Tailor a command node.
       */

      public static class CommandTailor extends NameFromProperty
      {
            public CommandTailor(String key, String propertyName)
            {
               super(key, propertyName);
            }

            void tailor(Model model)
            {
               StringBuffer name = new StringBuffer(
                  "Command: " + model.getProperty(propertyName) + "(");


               name.append(")");
               model.setProperty(MODEL_NAME, name.toString());
            }
      };

      /**
       * Tailor a variable declaration.
       */

      public static class DeclareTailor extends Tailor
      {
            public DeclareTailor(String key)
            {
               super(key);
            }

            void tailor(Model model)
            {
               String name =  model.getProperty(TYPE) +
                  " " + model.getProperty(NAME);
               model.setProperty(MODEL_NAME, name);
            }
      };

      /**
       * Tailor an array variable declaration.
       */

      public static class DeclareArrayTailor extends Tailor
      {
            public DeclareArrayTailor(String key)
            {
               super(key);
            }

            void tailor(Model model)
            {
               String name = 
                  model.getProperty(TYPE) + "[" + 
                  model.getProperty(MAXSIZE) + "] " +
                  model.getProperty(NAME);
               model.setProperty(MODEL_NAME, name);
            }
      };

      /**
       * Tailor an array element declaration.
       */

      public static class ArrayElementTailor extends Tailor
      {
            public ArrayElementTailor(String key)
            {
               super(key);
            }

            void tailor(Model model)
            {
               Model index = model.findChild(INT_VAL);
               String indexStr;
               if (index != null)
                  indexStr = index.getProperty(INT_VAL);
               else 
                  indexStr = model.findChild(INT_VAR).getProperty(INT_VAR);

               String name = 
                  model.findChild(STRING_VAL).getProperty(STRING_VAL) + "[" + 
                  indexStr + "] ";
               model.removeChild(STRING_VAL);
               model.removeChild(INT_VAL);
               model.setProperty(MODEL_NAME, name);
            }
      };

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

      /** Add breakpoint to this model. The breakpoint is only tested
       * when the model detects a change.
       *
       * @param breakPoint the specific breakpoint for this model.
       */

      /** A listener which is signaled when a the model is changed in
       * some way.
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

      /** An adapter which is signaled when a the model is changed in
       * some way.
       */

      public static class ChangeAdapter extends ChangeListener
      {
            @Override public void propertyChange(Model model, String property)
            {
            }

            @Override public void planCleared(Model model)
            {
            }

            @Override public void planChanged(Model model)
            {
            }

            @Override public void planNameAdded(Model model, String planName)
            {
            }
            
            @Override public void scriptNameAdded(Model model, String scriptName)
            {
            }

            @Override public void libraryNameAdded(Model model, String libraryName)
            {
            }
      }
}
