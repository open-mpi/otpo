package Main;
import java.awt.Dimension;

public class Main
{
	public static void main(String[] args) 
	{	
	    Dimension windowSize = new Dimension(820,720);
	    initialize(windowSize);	                
	}

	private static void initialize(Dimension windowSize) 
	{    		
		ChartPlotController controller = new ChartPlotController(windowSize);
		controller.init();    
	}
}
