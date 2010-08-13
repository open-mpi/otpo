package plot;

/*
Copyright (c) 2007 Pascal S. de Kloe. All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products derived
   from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

import java.awt.*;
import java.awt.event.*;
import java.math.BigDecimal;


/**
 * Extends the graph with GUI interaction.
 @author Pascal S. de Kloe
 @since 1.6
 */
@SuppressWarnings("serial")
public class InteractiveGraph extends Graph implements MouseListener, MouseMotionListener,ItemListener {

/**
 @since 1.6
 */
public
InteractiveGraph() {
	super();
	init();
}


/**
 @since 1.6
 */
public
InteractiveGraph(XAxis xAxis, YAxis yAxis) {
	super(xAxis, yAxis);
	init();
}

/**
 @since 1.6
 */
public
InteractiveGraph(XAxis xAxis, YAxis yAxis, Insets padding) {
	super(xAxis, yAxis, padding);
	init();
}


private void
init() {
	setLayout(new BorderLayout());		
	
	setCursor(DEFAULT_CURSOR);
	setToolTipText("");	// enable tool tips
	addMouseListener(this);
	addMouseMotionListener(this);
	//addMouseMotionListener(new ToolBarFader(toolBar));
}


public void
paintChildren(Graphics g) {
	if (zoomSelection != null)
		zoomSelection.paint(g);
	super.paintChildren(g);
}


public String
getToolTipText(MouseEvent event) {
	GraphInstance render = getRender();
	if (render == null)
		return "";
	Point mousePosition = event.getPoint();
	StringBuffer buffer = new StringBuffer(80);
	buffer.append('(');
	buffer.append(getXAxis().getFormat().format(render.getXValue(mousePosition.x)));
	buffer.append(", ");
	buffer.append(getYAxis().getFormat().format(render.getYValue(mousePosition.y)));
	buffer.append(')');
	return buffer.toString();
}


public void
mousePressed(MouseEvent event) {
	if(!ytoggle)
	{
	int modifiers = event.getModifiers();
	if ((modifiers & InputEvent.BUTTON1_MASK) != 0)
		zoomSelection = new ZoomSelection(this, event.getPoint());
	}
}


public void
mouseDragged(MouseEvent event) 
{
	if(!ytoggle)
	{
		int modifiers = event.getModifiers();
		if ((modifiers & InputEvent.BUTTON1_MASK) != 0) {
			if (zoomSelection != null) {
				zoomSelection.setMousePosition(event.getPoint());
				toolBar.setZoomPending(zoomSelection.getIntervalDescription());
				repaint(30L);
			}
		}
	}	
}


public void
mouseReleased(MouseEvent event) {
	if(!ytoggle)
	{
		int modifiers = event.getModifiers();
		if ((modifiers & InputEvent.BUTTON1_MASK) != 0) {
			if (zoomSelection != null) {
				boolean zoomed = zoomSelection.apply();
				if (zoomed)
					toolBar.setZoom(zoomSelection.getIntervalDescription());
				else
					toolBar.setZoomPending(null);
				zoomSelection = null;
				repaint(30L);
			}
		}		
	}	
}


// Unused interface methods:
public void mouseClicked(MouseEvent event) 
{ 
	if(ytoggle)
	{
		Object[] functions = getGraphFunctions();
    	Function graphFunc = (Function)functions[functions.length-1];
    	
		BigDecimal yLimit = getRender().getYValue(event.getPoint().y);
		setAdjustmentForFunction(graphFunc, new BigDecimal(0),new BigDecimal(0),new BigDecimal(0),yLimit);
		System.out.println("mouse clicked at "+getYAxis().getFormat().format(yLimit));
	}
}

public void mouseEntered(MouseEvent event) { }
public void mouseMoved(MouseEvent event) { }
public void mouseExited(MouseEvent event) { }


static final Cursor DEFAULT_CURSOR = new Cursor(Cursor.CROSSHAIR_CURSOR);
static final Cursor BUSSY_CURSOR = new Cursor(Cursor.WAIT_CURSOR);

private final ToolBar toolBar = new ToolBar(this);

public ToolBar getToolBar() 
{
	return toolBar;
}


private ZoomSelection zoomSelection = null;
private boolean ytoggle = false;



@Override
public void itemStateChanged(ItemEvent e) 
{
		int state = e.getStateChange();
		if (state == ItemEvent.SELECTED) 
		{
			ytoggle = true;
		} 
		else
		{
			resetToggle();	
			
		}		
}


public void resetToggle() {
	ytoggle = false;
	Object[] functions = getGraphFunctions();
	Function graphFunc = (Function)functions[functions.length-1];
	resetAdjustment(graphFunc);
	
}

}
