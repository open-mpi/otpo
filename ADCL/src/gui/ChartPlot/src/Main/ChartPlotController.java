package Main;

import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;

import javax.swing.JFrame;

import parser.OfflineParser;

import chartplot.Plot;

import communicationmanager.StartupServer;

public class ChartPlotController implements MouseListener, ActionListener
{
    public enum modes { online, offline};
    
	StartupServer _server;
	Plot _plot;
	OfflineParser _parser;
	Dimension _windowSize;
	boolean begin = false;

	modes onOffMode;
	int procNum = 0;
	
	public boolean getBegin() 
	{
		return begin;
	}	
	
	public int getNumberOfProcs()
	{
		if(onOffMode == modes.offline)
			return _parser.getNumberOfProcs(); 
		return 1; // to be done: else part for the online mode.
	}
	
	public modes getOnOffMode() 
	{
		return onOffMode;
	}

	public void toggleOnOffMode() 
	{
		if(this.onOffMode == modes.online)
		{
			this.onOffMode = modes.offline;
		}
		else
		{
			this.onOffMode = modes.online;
		}
	}
	
	public int getProcNum()
	{
		return procNum;
	}
	
	public void setProcNum(int procNumber)
	{
		procNum = procNumber;
	}

	public ChartPlotController(Dimension windowSize) 
	{

		_plot = new Plot(windowSize);
		_server = new StartupServer();
		_parser = new OfflineParser(procNum);
		
		_windowSize = windowSize;
		onOffMode = modes.online;
		
	}		
	
	public void mouseClicked(MouseEvent event)
	{		
		if(event.getComponent().getName() == "Disconnect")
		{
			_server.disconnect();
			System.out.println("disconnect");
		}
		else if(event.getComponent().getName() == "Start Over")
		{
			System.out.println("Start over");

			new Thread() 
	        {
				public void run() 
				{
					begin = false;
					restart();
					
				}
			}.start();
		}
	}

	public void mouseEntered(MouseEvent arg0) 
	{		
	}

	public void mouseExited(MouseEvent arg0) 
	{
	}

	public void mousePressed(MouseEvent arg0) 
	{
	}

	public void mouseReleased(MouseEvent arg0) 
	{		
	}

	public void actionPerformed(ActionEvent e) 
	{
		System.out.println(e.getActionCommand());
		if(e.getActionCommand() == "On/Off")
		{
			this.toggleOnOffMode();
		}
		else if(e.getActionCommand() == "Start")
		{
			new Thread() 
	        {
				public void run() 
				{
					begin = true;
					setup();
				}
			}.start();
		}
	}
	

	public void restart()
	{
		System.out.println("restarting");
	    _server.removeRecvListener(_plot);
	    _parser.removeRecvListener(_plot);
		_plot.startOver();
	    _server.disconnect();
	    _parser = null;
		
		System.gc();      
		
		_parser = new OfflineParser(procNum);
		setup();
	}

	private void setup() 
	{		
		if(begin == true)
		{
			if(getOnOffMode() == modes.online)
			{
				String serverArgs[] = {"-n","1","-p","20000"};
			    _server.addRecvListener(_plot); //define addrecvlistener in startupserver. put the generator code in it.
			    _server.start(serverArgs);
			}
			else
			{
				_parser.addRecvListener(_plot);
				_parser.startRead();
			}
		}		
		else
		{
			_plot.setOnOffButton(onOffMode);
			_plot.setVisible(true);
			_plot.paint(_plot.getGraphics());
		}
	}

	public void init() 
	{
		_plot.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);	   
		_plot.setController(this);

        setup();
	}
	
	
}
