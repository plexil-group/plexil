package gov.nasa.luv.runtime;

import java.util.Set;

import gov.nasa.luv.Model;

/**
 * 
 * @author Hector Fabio Cadavid Rengifo. hector.cadavid@escuelaing.edu.co
 *
 */
public abstract class AbstractPlexilExecutiveCommandGenerator {
	
	private String scriptPath;
	
	private Set<String> libFiles;
	
	public Set<String> getLibFiles() {
		return libFiles;
	}

	public void setLibFiles(Set<String> libFiles) {
		this.libFiles = libFiles;
	}


	private boolean breaksAllowed;
	
	private Model currentPlan;

	public Model getCurrentPlan() {
		return currentPlan;
	}

	public void setCurrentPlan(Model currentPlan) {
		this.currentPlan = currentPlan;
	}

	public boolean isBreaksAllowed() {
		return breaksAllowed;
	}

	public void setBreaksAllowed(boolean breaksAllowed) {
		this.breaksAllowed = breaksAllowed;
	}


	public String getScriptPath() {
		return scriptPath;
	}

	public void setScriptPath(String scriptPath) {
		this.scriptPath = scriptPath;
	}

	
	public abstract String generateCommandLine() throws ExecutiveCommandGenerationException;
	
}
