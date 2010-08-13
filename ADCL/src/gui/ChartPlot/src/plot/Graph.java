package plot;

/*
Copyright (c) 2005, 2006, 2007 Pascal S. de Kloe. All rights reserved.

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
import java.awt.print.*;
import java.math.*;
import java.util.*;
import java.text.*;
import javax.swing.*;


/**
 * The graph of one or more functions on a Cartesian plane.
 @author Pascal S. de Kloe
 @since 1.0
 */
@SuppressWarnings("serial")
public class Graph extends JComponent implements Printable {

/**
 @since 1.0
 */
public
Graph() {
	this(new XAxis(), new YAxis());
}


/**
 @since 1.0
 */
public
Graph(XAxis xAxis, YAxis yAxis) {
	this(xAxis, yAxis, new Insets(30, 60, 30, 60));
}


/**
 @since 1.0
 */
public
Graph(XAxis xAxis, YAxis yAxis, Insets padding) {
	if (xAxis == null || yAxis == null || padding == null)
		throw new IllegalArgumentException();
	this.xAxis = xAxis;
	this.yAxis = yAxis;
	setPadding(padding);
}


/**
 @since 1.0
 */
public final XAxis
getXAxis() {
	return xAxis;
}


/**
 @since 1.0
 */
public final YAxis
getYAxis() {
	return yAxis;
}


/**
 * Adds a function to the graph.
 * Changes apply on the next {@link #render()}.
 @since 1.0
 */
public final void
addFunction(Function function, ChartStyle style) {
	if (function == null || style == null)
		throw new IllegalArgumentException();
	synchronized (functions) {
		functions.put(function, style);
	}
}

/**
 * Removes a function from the graph.
 * Changes apply on the next {@link #render()}.
 @since 1.0
 */
public final void
removeFunction(Function function) {
	if (function == null)
		throw new IllegalArgumentException();
	synchronized (functions) {
		functions.remove(function);
	}
}


/**
 * Sets the amount of space between the border and the axis.
 * Note that the index labels are not affected. Changes apply on the next {@link #render()}.
 @since 1.0
 */
public final void
setPadding(Insets padding) {
	if (padding == null)
		throw new IllegalArgumentException();
	this.padding = padding;

	Insets border = getInsets();
	int minWidth = border.right + padding.right + border.left + padding.left;
	int minHeight = border.top + padding.top + border.bottom + padding.bottom;
	setMinimumSize(new Dimension(minWidth, minHeight));
}


/**
 * Gets the amount of space between the border and the axis.
 * Note that the index labels are not affected.
 @since 1.5
 */
public final Insets
getPadding() {
	return (Insets) padding.clone();
}


public void
paintComponent(Graphics g) {
	Graphics2D g2 = (Graphics2D) g.create();
	try {
		if (isOpaque()) {
			g2.setBackground(getBackground());
			g2.clearRect(0, 0, getWidth(), getHeight());
		}

		GraphInstance instance = render;	// thread-safe
		if (instance == null) return;

		g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		g2.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
		instance.paint(g2);
	} finally {
		g2.dispose();
	}
}


/**
 * Prints the graph on a single page.
 * The graph is scaled to as big as possible while remaining the aspect ratio.
 @since 1.1
 */
public int
print(Graphics g, PageFormat pageFormat, int pageIndex) {
	if (pageIndex > 0)
		return Printable.NO_SUCH_PAGE;

	int width = getWidth();
	int height = getHeight();
	Dimension minimumSize = getMinimumSize();
	if (width < minimumSize.getWidth() || height < minimumSize.getHeight())
		return Printable.NO_SUCH_PAGE;

	Paper paper = pageFormat.getPaper();
	double horizontalScalar = paper.getImageableWidth() / width;
	double verticalScalar = paper.getImageableHeight() / height;
	double scalar = Math.min(horizontalScalar, verticalScalar);

	Graphics2D g2 = (Graphics2D) g.create();
	g2.translate(paper.getImageableX(), paper.getImageableY());
	g2.scale(scalar, scalar);
	print(g2);

	return Printable.PAGE_EXISTS;
}


/**
 * Limits the domain for the graph.
 * Changes apply on the next {@link #render()}.
 @since 1.1
 */
public final void
setDomain(GraphDomain domain) {
	if (domain == null)
		throw new IllegalArgumentException();
	this.domain = domain;
}


/**
 * Renders the entire graph.
 * Use {@link #repaint()} to make the result visible.
 @since 1.0
 */
public final void
render() {
	// Graphical geometry:
	Insets border = getInsets();
	Insets padding = this.padding;	// thread-safe
	int right = border.right + padding.right;
	int top = border.top + padding.top;
	int left = border.left + padding.left;
	int bottom = border.bottom + padding.bottom;
	int width = getWidth() - left - right;
	int height = getHeight() - top - bottom;

	if (width > 0 && height > 0) {
		ArrayList<FunctionInstance> functionInstances = new ArrayList<FunctionInstance>(functions.size());
		CoordinateBoundary boundary = buildFunctionInstances(functionInstances);
		if (boundary != null) {
			boundary = boundary.getVisual();
			AxisInstance xAxisInstance = xAxis.getInstance(boundary.getXMin(), boundary.getXMax(), width);
			AxisInstance yAxisInstance = yAxis.getInstance(boundary.getYMin(), boundary.getYMax(), height);
			render = getRender(xAxisInstance, yAxisInstance, functionInstances, left, top);
			return;
		}
	}
	render = null;
	return;
}


private CoordinateBoundary
buildFunctionInstances(ArrayList<FunctionInstance> instances) {
	ArrayList<Function> currentFunctions;
	synchronized (functions) {
		currentFunctions = new ArrayList<Function>(functions.keySet());
	}

	GraphDomain currentDomain = domain;	// thread-safe
	int i = currentFunctions.size();
	while (--i >= 0) {
		Function f = currentFunctions.get(i);
		ChartStyle style = functions.get(f);
		if (style != null) {
			FunctionInstance fi = f.getInstance(currentDomain, style);
			if (fi.getCoordinateBoundary() != null)
				instances.add(fi);
		}
	}

	i = instances.size();
	if (--i < 0) return null;

	CoordinateBoundary boundary = instances.get(i).getCoordinateBoundary();
	BigDecimal xMin = boundary.getXMin();
	BigDecimal xMax = boundary.getXMax();
	BigDecimal yMin = boundary.getYMin();
	BigDecimal yMax = boundary.getYMax();

	while (--i >= 0) {
		boundary = instances.get(i).getCoordinateBoundary();
		xMin = xMin.min(boundary.getXMin());
		xMax = xMax.max(boundary.getXMax());
		yMin = yMin.min(boundary.getYMin());
		yMax = yMax.max(boundary.getYMax());
	}
	return new CoordinateBoundary(xMin, xMax, yMin, yMax);
}


private GraphInstance
getRender(AxisInstance xAxis, AxisInstance yAxis, ArrayList<FunctionInstance> functions, int xOffset, int yOffset) {
	BigDecimal xScalar = xAxis.GRAPHICS_SCALAR;
	BigDecimal yScalar = yAxis.GRAPHICS_SCALAR;
	int zeroPointX = xAxis.ZERO.divide(xScalar, MathContext.DECIMAL32).intValue();
	int zeroPointY = yAxis.ZERO.divide(yScalar, MathContext.DECIMAL32).intValue();
	xOffset -= Math.min(zeroPointX, xAxis.MIN.divide(xScalar, MathContext.DECIMAL32).intValue());
	yOffset -= Math.min(zeroPointY, yAxis.MAX.divide(yScalar, MathContext.DECIMAL32).intValue());

	FontMetrics fontMetrics = getFontMetrics(getFont());
	xAxis.render(zeroPointY, fontMetrics);
	yAxis.render(zeroPointX, fontMetrics);

	Format yFormat = this.yAxis.getFormat();
	int i = functions.size();
	while (--i >= 0)
		functions.get(i).render(xScalar, yScalar, yFormat, fontMetrics);

	return new GraphInstance(xAxis, yAxis, functions, xOffset, yOffset);
}


/**
 * Gets the current render instance of the graph.
 */
final GraphInstance
getRender()
{
	return render;
}

public Object[] getGraphFunctions()
{
    return functions.keySet().toArray();
}

private final LinkedHashMap<Function, ChartStyle> functions = new LinkedHashMap<Function, ChartStyle>();
private final XAxis xAxis;
private final YAxis yAxis;
private Insets padding;
private GraphDomain domain = new GraphDomain(null, null);
private GraphInstance render = null;

public void setAdjustmentForFunction(Function graphFunc,BigDecimal xMinAdjust,BigDecimal xMaxAdjust,BigDecimal yMinAdjust,BigDecimal yMaxAdjust) 
{
	ChartStyle style = functions.get(graphFunc);
	if(style != null)
	{
		FunctionInstance funcInst = style.getType().getFunctionInstance();
		if(funcInst != null)		
			funcInst.setAdjustment(xMinAdjust, xMaxAdjust, yMinAdjust, yMaxAdjust);	
	}		
	render();
	repaint();
}

public void resetAdjustment(Function graphFunc)
{
	ChartStyle style = functions.get(graphFunc);
	if(style != null)
	{
		FunctionInstance funcInst = style.getType().getFunctionInstance();
		if(funcInst != null)		
			funcInst.resetAdjustment();
	}		
	render();
	repaint();	
}

}
