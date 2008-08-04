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

import java.awt.Color;
import java.awt.Point;
import java.awt.Dimension;
import java.awt.Rectangle;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;

import static gov.nasa.luv.Constants.*;

/**
 * This is an extention of the stock Properties class which provides
 * automatic property persistance.
 */

public class Properties extends java.util.Properties
{
      /** file for persistant storage of properties */

      File file;

      /** 
       * Constrcut a properties object, reading properties if they
       * exist.
       *
       * @param filename name of properties file to read.
       */

      public Properties(String filename)
      {
         try
         {
            file = new File(filename);
            if (file.exists())
               load(new FileInputStream(file));
         }
         catch (Exception e)
         {
            e.printStackTrace();
         }
      }

      /**
       * Sets the value of a property in the property database, and
       * updates the state of the properties file.
       *
       * @param name  the name of the property
       * @param value the string value of the property
       */

      public Object setProperty(String name, String value)
      {
         Object oldValue = super.setProperty(name, value);
         try
         {
            store(new FileOutputStream(file), null);
         }
         catch (Exception e)
         {
            e.printStackTrace();
         }
         return oldValue;
      }

      /** Test that a given property exists.
       *
       * @return true if the property appears in this set of properties
       */

      public boolean exists(String name)
      {
         return getProperty(name) != null;
      }

      /**
       * Define a int property.  If the property does not exist, it will be
       * created with the specified default value.
       *
       * @param name         the name of the property
       * @param defaultValue the value that this property will take, if
       * it does not already appear in the properties list.
       */

      public void define(String name, int defaultValue)
      {
            String stringValue = getProperty(name);
            if (stringValue == null)
               set(name, defaultValue);
      }

      /**
       * Define a boolean property.  If the property does not exist, it will be
       * created with the specified default value.
       *
       * @param name         the name of the property
       * @param defaultValue the value that this property will take, if
       * it does not already appear in the properties list.
       */

      public void define(String name, boolean defaultValue)
      {
            String stringValue = getProperty(name);
            if (stringValue == null)
               set(name, defaultValue);
      }

      /**
       * Define a double property.  If the property does not exist, it will be
       * created with the specified default value.
       *
       * @param name         the name of the property
       * @param defaultValue the value that this property will take, if
       * it does not already appear in the properties list.
       */

      public void define(String name, double defaultValue)
      {
            String stringValue = getProperty(name);
            if (stringValue == null)
               set(name, defaultValue);
      }

      /**
       * Define a String property.  If the property does not exist, it will be
       * created with the specified default value.
       *
       * @param name         the name of the property
       * @param defaultValue the value that this property will take, if
       * it does not already appear in the properties list.
       */

      public void define(String name, String defaultValue)
      {
            String stringValue = getProperty(name);
            if (stringValue == null)
               set(name, defaultValue);
      }

      /**
       * Define a Color property.  If the property does not exist, it will be
       * created with the specified default value.
       *
       * @param name         the name of the property
       * @param defaultValue the value that this property will take, if
       * it does not already appear in the properties list.
       */

      public void define(String name, Color defaultValue)
      {
            String stringValue = getProperty(name);
            if (stringValue == null)
               set(name, defaultValue);
      }

      /**
       * Define a Point property.  If the property does not exist, it will be
       * created with the specified default value.
       *
       * @param name         the name of the property
       * @param defaultValue the value that this property will take, if
       * it does not already appear in the properties list.
       */

      public void define(String name, Point defaultValue)
      {
            String stringValue = getProperty(name);
            if (stringValue == null)
               set(name, defaultValue);
      }

      /**
       * Define a Dimension property.  If the property does not exist, it will be
       * created with the specified default value.
       *
       * @param name         the name of the property
       * @param defaultValue the value that this property will take, if
       * it does not already appear in the properties list.
       */

      public void define(String name, Dimension defaultValue)
      {
            String stringValue = getProperty(name);
            if (stringValue == null)
               set(name, defaultValue);
      }

      /**
       * Define a Rectangle property.  If the property does not exist, it will be
       * created with the specified default value.
       *
       * @param name         the name of the property
       * @param defaultValue the value that this property will take, if
       * it does not already appear in the properties list.
       */

      public void define(String name, Rectangle defaultValue)
      {
            String stringValue = getProperty(name);
            if (stringValue == null)
               set(name, defaultValue);
      }

      /**
       * Set a int property.  If the property does not exist, it will be
       * created with the specified value.
       *
       * @param name  the name of the property
       * @param value the value that this property will take
       */

      public void set(String name, int value)
      {
         setProperty(name, "" + value);
      }

      /**
       * Set a boolean property.  If the property does not exist, it will be
       * created with the specified value.
       *
       * @param name  the name of the property
       * @param value the value that this property will take
       */

      public void set(String name, boolean value)
      {
         setProperty(name, value ? TRUE : FALSE);
      }

      /**
       * Set a double property.  If the property does not exist, it will be
       * created with the specified value.
       *
       * @param name the name of the property
       * @param value the value that this property will take
       */

      public void set(String name, double value)
      {
         setProperty(name, "" + value);
      }

      /**
       * Set a String property.  If the property does not exist, it will be
       * created with the specified value.
       *
       * @param name  the name of the property
       * @param value the value that this property will take
       */

      public void set(String name, String value)
      {
         setProperty(name, value);
      }

      /**
       * Set a Color property.  If the property does not exist, it will be
       * created with the specified value.
       *
       * @param name  the name of the property
       * @param value the value that this property will take
       */

      public void set(String name, Color value)
      {
         setProperty(name, 
                     (int)value.getRed()   + ", " +
                     (int)value.getGreen() + ", " +
                     (int)value.getBlue()  + ", " +
                     (int)value.getAlpha());
      }

      /**
       * Set a Point property.  If the property does not exist, it will be
       * created with the specified value.
       *
       * @param name  the name of the property
       * @param value the value that this property will take
       */

      public void set(String name, Point value)
      {
         setProperty(name, 
                     (int)value.getX() + ", " +
                     (int)value.getY());
      }

      /**
       * Set a Dimension property.  If the property does not exist, it will be
       * created with the specified value.
       *
       * @param name  the name of the property
       * @param value the value that this property will take
       */

      public void set(String name, Dimension value)
      {
         setProperty(name, (int)value.getWidth() + ", " + (int)value.getHeight());
      }

      /**
       * Set a Rectangle property.  If the property does not exist, it will be
       * created with the specified value.
       *
       * @param name  the name of the property
       * @param value the value that this property will take
       */

      public void set(String name, Rectangle value)
      {
         setProperty(name, 
                     (int)value.getX()     + ", " +
                     (int)value.getY()     + ", " +
                     (int)value.getWidth() + ", " +
                     (int)value.getHeight());
      }

      /**
       * Get an integer propery value.
       *
       * @return the integer value of propery if it exists, otherwise it
       * throws an exception.
       */

      public int getInteger(String name)
      {
         return Integer.valueOf(getProperty(name));
      }

      /**
       * Get an boolean propery value.
       *
       * @return the boolean value of propery if it exists, otherwise it
       * throws an exception.
       */

      public boolean getBoolean(String name)
      {
         return Boolean.valueOf(getProperty(name));
      }

      /**
       * Get a double propery value.
       *
       * @return the double value of propery if it exists, otherwise it
       * throws an exception.
       */

      public double getDouble(String name)
      {
         return Double.valueOf(getProperty(name));
      }

      /**
       * Get an String propery value.
       *
       * @return the String value of propery if it exists, otherwise it
       * throws an exception.
       */

      public String getString(String name)
      {
         return getProperty(name);
      }

      /**
       * Get a Color propery value.
       *
       * @return the Color value of propery if it exists, otherwise it
       * throws an exception.
       */

      public Color getColor(String name)
      {
         String[] values = getProperty(name).split(",");
         return new Color(
            Integer.valueOf(values[0].trim()), 
            Integer.valueOf(values[1].trim()),
            Integer.valueOf(values[2].trim()), 
            Integer.valueOf(values[3].trim()));
      }

      /**
       * Get a Point propery value.
       *
       * @return the Point value of propery if it exists, otherwise it
       * throws an exception.
       */

      public Point getPoint(String name)
      {
         String[] values = getProperty(name).split(",");
         return new Point(
            Integer.valueOf(values[0].trim()), 
            Integer.valueOf(values[1].trim()));
      }

      /**
       * Get a Dimension propery value.
       *
       * @return the Dimension value of propery if it exists, otherwise it
       * throws an exception.
       */

      public Dimension getDimension(String name)
      {
         String[] values = getProperty(name).split(",");
         return new Dimension(
            Integer.valueOf(values[0].trim()), 
            Integer.valueOf(values[1].trim()));
      }

      /**
       * Get a Rectangle propery value.
       *
       * @return the Rectangle value of propery if it exists, otherwise it
       * throws an exception.
       */

      public Rectangle getRectangle(String name)
      {
         String[] values = getProperty(name).split(",");
         return new Rectangle(
            Integer.valueOf(values[0].trim()), 
            Integer.valueOf(values[1].trim()),
            Integer.valueOf(values[2].trim()), 
            Integer.valueOf(values[3].trim()));
      }
}
