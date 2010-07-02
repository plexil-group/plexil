package gov.nasa.luv;
import java.io.IOException;
import java.io.File;
import static gov.nasa.luv.Constants.PORT_TEMP_FILE_LOCATION;

public class LuvTempFile {
	
	LuvTempFile() 
	{
		File temp_dir = new File(PORT_TEMP_FILE_LOCATION);
		File temp = new File(PORT_TEMP_FILE_LOCATION + "Luv_" + Luv.getLuv().getPort());		
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

	String fileName = PORT_TEMP_FILE_LOCATION + "Luv_" + Luv.getLuv().getPort();
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
	File temp = new File(PORT_TEMP_FILE_LOCATION + "Luv_" + port);
	return temp.exists();
}
}
