/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package gov.nasa.luv;

import static gov.nasa.luv.Constants.*;

import java.util.Vector;
import javax.swing.JOptionPane;

public class LibraryHandler 
{

      /** set of active libraries */

      Vector<Model> libraries = new Vector<Model>();
      
      public LibraryHandler() {}

      /** Link a plan with it's provided libraries 
       *
       * @param node current node of model being linked
       * @param libraries the collected libraries which will linked into
       * the plan
       *
       * @return false if all linking cancled by user, this does NOT mean that the 
       */

      public boolean link(Model node, Vector<Model> libraries) 
         throws LinkCanceledException
      {
         // if this is a library node call, link that node
         
         String type = node.getProperty(NODETYPE_ATTR);
         if (type != null && type.equals(LIBRARYNODECALL))
         {
            boolean retry = true;
            String callName = node.getProperty(MODEL_LIBRARY_CALL_ID);

            do
            {
               // look through the libraries for match

               for (Model library: libraries)
               {
                  String libName = library.getProperty(NODE_ID);
                  
                  if (callName.equals(libName))
                  {
                     node.addChild((Model)library.clone());
                     retry = false;
                     break;
                  }
               }

               // if we didn't make the link, ask user for library

               if (retry)
               {
                  // option

                  Object[] options = 
                     {
                        "load this library",
                        "do not load this library",
                        "stop asking about libraries",
                     };

                  // show the options

                  int result = JOptionPane.showOptionDialog(
                     Luv.luvViewerHandler,
                     "Missing library which contains \"" + callName +
                     "\".  Would you like to load this library?",
                     "Load the library?",
                     JOptionPane.YES_NO_CANCEL_OPTION,
                     JOptionPane.WARNING_MESSAGE,
                     null,
                     options,
                     options[0]);

                  // process the results

                  switch (result)
                  {
                     // try to load the library and retry the link

                     case 0:
                        Luv.getLuv().fileHandler.chooseLibrary();
                        break;
                        
                        // if the user doesn't want to find this library
                        // go on with link but don't retry to like this
                        // one

                     case 1:
                        return false;
                        
                     // if the user doesn't want to load any libraries,
                     // halt the link operation now
                     
                     case 2:
                        throw new LinkCanceledException(callName);
                  }
               }
            }
            while (retry);
         }

         // if this is node, traverse into any children it might have,
         // note that this could be a node could have newly minted
         // children from the linking action above

         boolean success = true;
         for (Model child: node.getChildren())
            if (!link(child, libraries))
               success = false;

         return success;
      }    
}
