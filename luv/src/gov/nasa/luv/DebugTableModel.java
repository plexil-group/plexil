//original author Europa group
//modified for Plexil

package gov.nasa.luv;

import java.util.List;
import javax.swing.JTable;
import javax.swing.table.TableModel;
import javax.swing.table.AbstractTableModel;

public class DebugTableModel
    extends AbstractTableModel
{
	protected Debug data_;
	protected int maxDataset_;
    
	public DebugTableModel(Debug db)
	{
		data_ = db;
	}
	
	public int getColumnCount()
	{
		return data_.getMsgList().size();
	}
	
	public int getRowCount()
	{
		return maxDataset_;
	}
	
	public Object getValueAt(int rowIndex, int columnIndex)
	{
		List debugList = data_.getMsgList().get(columnIndex);
		if(debugList.size() > rowIndex)
			return debugList.get(rowIndex);
		else
			return "";
	}

}

