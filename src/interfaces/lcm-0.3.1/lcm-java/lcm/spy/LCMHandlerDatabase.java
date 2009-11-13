package lcm.spy;

import java.util.*;

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.table.*;
import javax.swing.tree.*;
import java.awt.*;
import java.awt.event.*;

import java.io.*;
import java.util.*;
import java.util.jar.*;
import java.util.zip.*;

import lcm.util.*;

import java.lang.reflect.*;

import lcm.lcm.*;

/** Searches classpath for objects that implement LCSpyPlugin using reflection. **/
class LCMHandlerDatabase
{
    HashMap<Long, Class> classes = new HashMap<Long, Class>();

    public LCMHandlerDatabase()
    {
	ClassDiscoverer.findClasses(new MyClassVisitor());
	System.out.println("Found "+classes.size()+" LCM types");
    }

    class MyClassVisitor implements ClassDiscoverer.ClassVisitor
    {
	public void classFound(String jar, Class cls)
	{
	    Field[] fields = cls.getFields();

	    try {
		for (Field f : fields) {
		    if (f.getName().equals("LCM_FINGERPRINT")) {
			// it's a static member, we don't need an instance
			long fingerprint = f.getLong(null);
			classes.put(fingerprint, cls);
			// System.out.printf("%016x : %s\n", fingerprint, cls);

			break;
		    }
		}	
	    } catch (IllegalAccessException ex) {
		System.out.println("Bad LCM Type? "+ex);
	    }
	}
    }

    public Class getClassByFingerprint(long fingerprint)
    {
	return classes.get(fingerprint);
    }
}
