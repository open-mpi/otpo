
package chartplot;

/**
 *
 * @author sarat
 */

import java.awt.*;
import java.util.*;
import java.math.*;
import javax.swing.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import Main.ChartPlotController;
import Main.ChartPlotController.modes;

import eventpackage.*;
import plot.*;
import utility.Utility;

@SuppressWarnings("serial")
public final class Plot extends JFrame implements CustomEventListener,ChangeListener
{
	class TabStatus
	{
		HashMap<Integer, Integer> funcNumToFuncId;
		public TabStatus(Graph tab, int iteration) 
		{			
			super();
			this.tab = tab;
			this.iteration = iteration;
			funcNumToFuncId = new HashMap<Integer, Integer>();
		}
		
		Graph tab;
		public void setTab(Graph tab) {
			this.tab = tab;
		}
		public Graph getTab() {
			return tab;
		}
		int iteration;
		public int getIteration() {
			return iteration;
		}
		public void setIteration(int iteration) {
			this.iteration = iteration;
		}
		
		public HashMap<Integer, Integer> getFuncIdToFuncNum() {
			return funcNumToFuncId;
		}		
		
		int functionNumber = 0;
		public int getFunctionNumber()
		{
			return functionNumber;
		}
		public void setFunctionNumber(int functionNumber) {
			this.functionNumber = functionNumber;
		}
		public void addFunctionId(int functionId) 
		{
			funcNumToFuncId.put(Integer.valueOf(functionId), Integer.valueOf(functionNumber));
			functionNumber++;
		}
		
		public boolean functionIdExists(int functionId)
		{
			return funcNumToFuncId.containsKey(functionId);
		}
		
		public void incrementIterator() 
		{
			iteration++;
		}
		
		public int getFuncNumFromId(int functionId) 
		{
			return funcNumToFuncId.get(functionId);
		}
	}

	Color color[]={Color.GREEN,Color.BLACK,Color.RED,Color.BLUE,Color.CYAN,Color.YELLOW,Color.GRAY,Color.PINK,Color.ORANGE,Color.WHITE,Color.MAGENTA,Color.LIGHT_GRAY};
    XAxis xAxis;
    YAxis yAxis;// make changes in axisinstance in the paint function
    private final ArrayList<Graph> graph;
    boolean init;
    private static final Cursor ACTION_CURSOR = new Cursor(Cursor.HAND_CURSOR);
    
    Utility util = Utility.getInstance();
    
    HashMap<Integer, TabStatus> idToTabStatus;
        
    JTabbedPane tab = null;
    ChartPlotController _controller;
    WelcomeScreen ws;
   
    Dimension _windowSize;

	public Plot(Dimension windowSize) 
	{
		xAxis = new XAxis("No. of Iterations");
		yAxis = new YAxis("Time Elapsed");
		graph = new ArrayList<Graph>();
		init = false;
		
		_windowSize = windowSize;
		setSize(_windowSize);
		setResizable(false);		
		idToTabStatus = new  HashMap<Integer, TabStatus>();
		
	}
	
	public void setController(ChartPlotController controller)
	{
		_controller = controller;
		addWelcomeScreen();
	}
	
    private void addWelcomeScreen()
    {
		ws = new WelcomeScreen(_windowSize, _controller);
		getContentPane().add(ws);	
	}
    
    public void setOnOffButton(modes onOff)
    {
    	if(onOff == modes.online)
    		ws.setModeToggle(true);
    	if(onOff == modes.offline)
    		ws.setModeToggle(false);
    }

	private void initComponent()
    {    	
        tab = new JTabbedPane();
        getContentPane().add(tab);
        
        for(int i=0;i<graph.size();i++)
        {
        	addComponentsToTab(i);
        }

		tab.setVisible(true);
    }

	private void addComponentsToTab(int i) 
	{
		Graph graphToAdd = idToTabStatus.get(Integer.valueOf(i)).getTab();
		JLabel disconnect = new JLabel();
		disconnect.setName("Disconnect");
		ImageIcon disconnectIcon = new ImageIcon(util.getImage("/icons/disconnect1.png").getScaledInstance(40, 40, Image.SCALE_SMOOTH));
		disconnect.setBounds(400,10,100,25);
		disconnect.setToolTipText("Disconnect");
		disconnect.setCursor(ACTION_CURSOR);
		disconnect.setIcon(disconnectIcon);
		disconnect.addMouseListener(_controller);
		
		JLabel startOver = new JLabel();
		startOver.setName("Start Over");
		ImageIcon startOverIcon = new ImageIcon(util.getImage("/icons/restart2.png").getScaledInstance(40, 40, Image.SCALE_SMOOTH));
		startOver.setToolTipText("Start Over");
		startOver.setCursor(ACTION_CURSOR);
		startOver.setIcon(startOverIcon);
		startOver.addMouseListener(_controller);
		startOver.setBounds(400,10,100,25);
		
		JComponent toolbar = ((InteractiveGraph)graphToAdd).getToolBar();
		if(_controller.getOnOffMode() == modes.online)
			toolbar.add(disconnect);
		toolbar.add(startOver);
		
		JToggleButton yLimit = new JToggleButton("Set Y");
		yLimit.addItemListener((InteractiveGraph)graphToAdd);
		toolbar.add(yLimit);
		
		if(_controller.getOnOffMode() == modes.offline)
		{
			SpinnerModel model = new SpinnerNumberModel(_controller.getProcNum(), 0, _controller.getNumberOfProcs() - 1, 1);
			JSpinner spin = new JSpinner(model);
			spin.addChangeListener(this);
			toolbar.add(spin);			
		}	
		
		graphToAdd.add(toolbar, BorderLayout.NORTH);

		Panel textPanel = new Panel();
		textPanel.setBounds(0,getHeight() - 120, getWidth(), 100);
		TextArea messageBox = new TextArea("", textPanel.getHeight()/16, getWidth()/8, 1);
		
		textPanel.add(messageBox);
		graphToAdd.add(textPanel,BorderLayout.SOUTH);
		
		Legend legend = new Legend();
		graphToAdd.add(legend,BorderLayout.EAST);

		tab.addTab("Tab "+i, graphToAdd);
	}

	private void addNewGraphToList() 
	{
		Insets padding = new Insets(100, 70, 200, 200);
		Graph newGraph = new InteractiveGraph(xAxis, yAxis);    		
		newGraph.getXAxis().setZigZaginess(BigDecimal.valueOf(7L, 1));
		newGraph.getYAxis().setZigZaginess(BigDecimal.valueOf(7L, 1));
		newGraph.setBackground(Color.WHITE);
		newGraph.setPadding(padding);
		graph.add(newGraph);
	}

	private void addFunctionToGraph(int i) 
	{
		ChartStyle style2 = new ChartStyle();
		style2.setPaint(color[0]);
		idToTabStatus.get(Integer.valueOf(i)).getTab().addFunction(new Function("Function "+i), style2);
	}

    public void addPointtoGraph(int messageId, double y)
    {
    	Integer iteration = idToTabStatus.get(messageId).getIteration();
    	BigDecimal xPoint = new BigDecimal(iteration.doubleValue());
    	BigDecimal yPoint = new BigDecimal(y);
    	Graph updateGraph = idToTabStatus.get(Integer.valueOf(messageId)).getTab();
    	
    	Object[] functions = updateGraph.getGraphFunctions();
    	Function graphFunc = (Function)functions[functions.length-1];
    	graphFunc.addPoint(xPoint, yPoint);    		   		

		idToTabStatus.get(messageId).incrementIterator();	
    }
    
	public void stateChanged(ChangeEvent arg) 
	{
		JSpinner spinner = (JSpinner)arg.getSource();
		Integer spinValue = (Integer)spinner.getValue();
		resetToNewProc(spinValue);
	}
    
    public void resetToNewProc(int spinValue)
    {
    	_controller.setProcNum(spinValue);
    	new Thread() 
        {
			public void run() 
			{
				_controller.restart();				
			}
		}.start();
    }
    
    public void recvReceived(EventObject event) 
    {
    	if(event.getClass().getSimpleName().contentEquals("PointsEvent"))  
    	{
    		PointsEvent ptEvent = (PointsEvent)event;
    		int messageId = ptEvent.get_id();
    		double yval = ptEvent.get_yval();
	    	newMessage(messageId);    	
	    	addPointtoGraph(messageId, yval);
    	}
    	else if(event.getClass().getSimpleName().contentEquals("MessageReceivedEvent"))
    	{
    		MessageReceivedEvent mrEvent = (MessageReceivedEvent)event;
    		int messageId = mrEvent.get_msgId();
    		String message = mrEvent.get_message();
    		newMessage(messageId);
    		showInMessageBox(messageId, message);
    	}
    	else if(event.getClass().getSimpleName().contentEquals("FunctionChangeEvent"))
    	{
    		FunctionChangeEvent fcEvent = (FunctionChangeEvent)event;
    		int messageId = fcEvent.get_msgId();
    		int funcId = fcEvent.get_funcId();
    		String objName = fcEvent.get_objectName();
    		newMessage(messageId);

    		functionChange(messageId, funcId, objName);    		
    	}
    	else if(event.getClass().getSimpleName().contentEquals("WinnerDecidedEvent"))
    	{
    		WinnerDecidedEvent wdEvent = (WinnerDecidedEvent)event;
    		int messageId = wdEvent.get_msgid();
    		int functionId = wdEvent.get_funcId();
    		int requestId = wdEvent.get_requestId();
    		String objectName = wdEvent.get_message();
    		objectName = objectName.substring(0, 8);
    		objectName = objectName.replaceAll("[^A-Za-z\\_\\s]", " ");
    		newMessage(messageId);
    		winnerDecided(messageId, functionId, requestId, objectName);    
    	}
		
		repaintApplication();
    	
    }

	private void newMessage(int messageId) 
	{
		if(!idToTabStatus.containsKey(Integer.valueOf(messageId)))
		{
			if(!init)
			{				
				ws.setVisible(false);
				initComponent();
				init = true;
			}
			addNewGraphToList();
			idToTabStatus.put(Integer.valueOf(messageId), new TabStatus(graph.get(graph.size() - 1),Integer.valueOf(1)));
			
			addFunctionToGraph(messageId);
			addComponentsToTab(messageId);					
		}
	}

	private void repaintApplication() 
	{
		for(Graph eachGraph : graph)
		{
			eachGraph.render();
			eachGraph.repaint();
		}
	}

	private void winnerDecided(int messageId, int functionId, int requestId, String objectName) 
	{		
		ChartStyle style2 = new ChartStyle();		
		TabStatus tabStatus = idToTabStatus.get(Integer.valueOf(messageId));
		style2.setPaint(color[tabStatus.getFuncNumFromId(functionId)]);
		
		Graph currGraph = tabStatus.getTab();		
		currGraph.addFunction(new Function("Function "+messageId), style2);	
		
		String winnerMsg = objectName+" "+requestId+" winner is "+functionId+" "+tabStatus.getTab().getGraphFunctions()[tabStatus.getFuncNumFromId(functionId)+1].toString();
		showInMessageBox(messageId, winnerMsg);
	}

	private void functionChange(int messageId, int functionId, String functionName) 
	{		
		if(!idToTabStatus.get(messageId).functionIdExists(functionId))
		{
			idToTabStatus.get(messageId).addFunctionId(functionId);
			int functionNum = idToTabStatus.get(messageId).getFunctionNumber();
			functionName = functionName.trim();
			ChartStyle style2 = new ChartStyle();
			style2.setPaint(color[functionNum-1]);
			Graph currGraph = idToTabStatus.get(Integer.valueOf(messageId)).getTab();
			
			currGraph.addFunction(new Function(functionName), style2);			
			((Legend)currGraph.getComponent(currGraph.getComponentCount()-1)).addToLegendList(color[functionNum-1],functionName);
		}
	}

	private void showInMessageBox(int id, String str) 
	{
		Panel panel = (Panel)idToTabStatus.get(id).getTab().getComponent(1);		
		TextArea textbox = (TextArea)panel.getComponent(0);
		textbox.append(str+"\n");
	}
	
	public void startOver()
	{
		Collection<TabStatus> list = idToTabStatus.values();
		
		for(TabStatus eachStatus : list )
		{			
			((InteractiveGraph)eachStatus.getTab()).resetToggle();
		}
		
		if(tab != null)
		{
			tab.setVisible(false);
			if(!_controller.getBegin())
				ws.setVisible(true);
			tab.removeAll();
			getContentPane().remove(tab);		
			tab = null;
		}
		
		for(TabStatus eachStatus : list )
		{			
			eachStatus.getFuncIdToFuncNum().clear();
			eachStatus = null;
		}
		if(idToTabStatus != null)
			idToTabStatus.clear();
		if(graph != null)
			graph.clear();
		init = false;
	}

}


