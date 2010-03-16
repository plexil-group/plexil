//original author Europa group
//modified for Plexil

package gov.nasa.luv;

public class DebugHistoryEntry
{
    protected Integer idx_;
    protected Integer step_;
    protected String towers_;
    protected String content_;
    protected String opHistory_;
    
    public DebugHistoryEntry(Integer idx,Integer step,String content,String opHistory)
    {
    	idx_ = idx;
    	step_ = step;
    	content_ = content;
    	opHistory_ = opHistory;
    }
    
    public Integer getIndex() { return idx_; }
    public Integer getStepNumber() { return step_; }
    public String getContent() { return content_; }
    public String getOperatorHistory() { return opHistory_; }	
}
