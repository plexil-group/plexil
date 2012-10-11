package gov.nasa.luv;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.File;
import java.io.InputStreamReader;

import static gov.nasa.luv.Constants.PORT_MAX;
import static gov.nasa.luv.Constants.PORT_MIN;
import static gov.nasa.luv.Constants.PORT_TEMP_FILE_LOCATION;
import static gov.nasa.luv.Constants.PORT_CHECK_UTIL_RHEL5;
import static gov.nasa.luv.Constants.PORT_CHECK_UTIL_UNIX;

public class LuvTempFile {
	
	LuvTempFile() 
	{
		File temp_dir = new File(PORT_TEMP_FILE_LOCATION);
		File temp = new File(PORT_TEMP_FILE_LOCATION + ".Luv_" + Luv.getLuv().getPort());		
		try{
		  if(!temp_dir.exists())
			  throw new IllegalArgumentException("Undefined temporary file directory" + temp_dir.getName()); 
		  if(temp.exists())
			  throw new IllegalArgumentException("Port in use");
		  else{
			  temp.createNewFile();
			  temp.deleteOnExit();
		  }
		  
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public static void deleteTempFile()
	{
		LuvTempFile.deleteTempFile(Luv.getLuv().getPort());
	}
		
	public static void deleteTempFile(int port)
	{
	
		String fileName = PORT_TEMP_FILE_LOCATION + ".Luv_" + port;
		File f = new File(fileName);
		
		if (!f.exists())
			return;
		
		if(!f.canWrite())
			throw new IllegalArgumentException("Delete: write protected: " + fileName);
		
		boolean success = f.delete();
		
		if(!success)
			throw new IllegalArgumentException("Delete: deletion failed");
	}
	
	public static boolean checkPort(int port)
	{
		File temp = new File(PORT_TEMP_FILE_LOCATION + ".Luv_" + port);
		return temp.exists();
	}
	
	public static void cleanupPorts()
	{
		for(int i = PORT_MIN; i < PORT_MAX + 1; i++)
				if(LuvTempFile.checkPort(i))						
					LuvTempFile.lookupPort(i);
	}
	
	private static void lookupPort(int port)// throws IOException
	{		
		System.out.println(System.getProperty("os.name"));
    	if(System.getProperty("os.name").contains("Linux"))    	
    		(new Thread(new PortHandler(PORT_CHECK_UTIL_RHEL5, port))).start();    	
    	if(System.getProperty("os.name").contains("Mac"))    	
    		(new Thread(new PortHandler(PORT_CHECK_UTIL_UNIX, port))).start();
	}
		
}

class PortHandler extends ExecutionHandler implements Runnable {
    private Runtime runtime;    
    private Process process; 
	private String command;
	private int port;
	
	public PortHandler() {}	
	
	public PortHandler(final String command, int port)
      {      	   
		this.port = port;
		this.command = command;
		  process = null;		  
	                 
      }
	
	 public void run()
     {
		 try
	      {
	          runtime = Runtime.getRuntime();
	          process = runtime.exec(this.command);                      	                      	                                            
	          checkOutput(process);
	      }
	      catch(Exception e)
	      {
	          Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while executing plan");
	      }         
     }
	
	private void checkOutput(Process ue_process) throws IOException
  	{
  		String line;  		
  		BufferedReader is = new BufferedReader(new InputStreamReader(ue_process.getInputStream()));
  		while ((line = is.readLine()) != null)
  		{
  			if(line.contains(":"+port))
  			{  				  				
  				is.close();
  				return;
  			}  			
  		}
  		LuvTempFile.deleteTempFile(port);
  	}
  	
}