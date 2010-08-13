package plot;

/*
Copyright (c) 2005, 2006 Pascal S. de Kloe. All rights reserved.

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
import java.math.*;
import java.text.*;
import java.util.*;


/**
 @author Pascal S. de Kloe
 @see Function#getInstance(GraphDomain, ChartStyle)
 */
abstract class FunctionInstance {

/**
 @param description a short description of the function.
 @param argument the arguments in ascending order.
 @param value the corresponding values.
 */
	
	static BigDecimal _xMinAdjust = new BigDecimal(0);
	static BigDecimal _xMaxAdjust = new BigDecimal(0);
	static BigDecimal _yMinAdjust = new BigDecimal(0);
	static BigDecimal _yMaxAdjust = new BigDecimal(0);
	static boolean adjust = false;
	
FunctionInstance(String description, BigDecimal[] argument, BigDecimal[] value, ChartStyle style) {
	DESCRIPTION = description;
	X_COORDINATE = argument;
	Y_COORDINATE = value;
	this.style = style;
	COORDINATE_BOUNDARY = getCoordinateBoundary(argument, value);
}


/**
 @see #getCoordinateBoundary()
 */
private static CoordinateBoundary
getCoordinateBoundary(BigDecimal[] argument, BigDecimal[] value) {
	int i = argument.length;
	while (--i >= 0) {
		BigDecimal y = value[i];
		if (y != null) {
			BigDecimal yMin = y;
			BigDecimal yMax = y;
			while (--i >= 0) {
				y = value[i];
				if (y != null) {
					if (y.compareTo(yMin) < 0)
						yMin = y;
					else if (yMax.compareTo(y) < 0)
						yMax = y;
				}
			}
			BigDecimal xMin = argument[0];
			BigDecimal xMax = argument[argument.length - 1];
			if(!adjust)
				return new CoordinateBoundary(xMin.add(_xMinAdjust), xMax.add(_xMaxAdjust), yMin.add(_yMinAdjust), yMax.add(_yMaxAdjust));
			else
			{
				BigDecimal zero = new BigDecimal(0);
				return new CoordinateBoundary(_xMinAdjust.equals(zero)?xMin:_xMinAdjust, _xMaxAdjust.equals(zero)?xMax:_xMaxAdjust, _yMinAdjust.equals(zero)?yMin:_yMinAdjust, _yMaxAdjust.equals(zero)?yMax:_yMaxAdjust);
			}
		}
	}
	return null;
}

public void setAdjustment(BigDecimal xMinAdjust,BigDecimal xMaxAdjust,BigDecimal yMinAdjust,BigDecimal yMaxAdjust)
{
	_xMinAdjust = xMinAdjust;
	_xMaxAdjust = xMaxAdjust;
	_yMinAdjust = yMinAdjust;
	_yMaxAdjust = yMaxAdjust;
	adjust = true;
	
}

public void resetAdjustment()
{
	adjust = false;
}

public final String
toString() {
	return DESCRIPTION;
}


final BigDecimal[]
getXCoordinates() {
	return X_COORDINATE;
}


final BigDecimal[]
getYCoordinates() {
	return Y_COORDINATE;
}


/**
 * Gets the boundaries from all coordinates.
 @return {@code null} if there are no coordinates.
 */
final CoordinateBoundary
getCoordinateBoundary() {
	return COORDINATE_BOUNDARY;
}


/**
 @param xScalar transforms graphical horizontal positions to x-coordinates.
 @param yScalar transforms graphical vertical positions to y-coordinates.
 @param yFormat the format for values from the y-axis.
 */
final void
render(BigDecimal xScalar, BigDecimal yScalar, Format yFormat, FontMetrics fontMetrics) {
	renderFunction(X_COORDINATE, Y_COORDINATE, xScalar, yScalar);
	if (style.getLowerLimitEnabled())
		lowerLimit = new LowerLimitInstance(COORDINATE_BOUNDARY, xScalar, yScalar, yFormat, fontMetrics);
	if (style.getUpperLimitEnabled())
		upperLimit = new UpperLimitInstance(COORDINATE_BOUNDARY, xScalar, yScalar, yFormat, fontMetrics);
}


/**
 @param xCoordinate one or more arguments in ascending order.
 @param yCoordinate the corresponding values.
 @param xScalar transforms graphical horizontal positions to x-coordinates.
 @param yScalar transforms graphical vertical positions to y-coordinates.
 */
abstract void
renderFunction(BigDecimal[] xCoordinate, BigDecimal[] yCoordinate, BigDecimal xScalar, BigDecimal yScalar);


/**
 * Defines a terminator at coordinate ({@code x}, {@code y}).
 */
final void
addTerminator(BigDecimal x, BigDecimal y, BigDecimal xScalar, BigDecimal yScalar) {
	TerminatorFactory tf = style.getTerminatorFactory();
	if (tf == null) return;
	terminators.add(tf.getTerminator(x, y, xScalar, yScalar));
}


final void
paint(Graphics g) {
	Graphics2D g2 = (Graphics2D) g.create();
	Paint functionPaint = style.getPaint();
	if (functionPaint != null)
		g2.setPaint(functionPaint);

	paintFunction(g2);
	paintTerminators(g2);

	Paint lowerLimitPaint = null;
	if (lowerLimit != null) {
		lowerLimitPaint = style.getLowerLimitPaint();
		if (lowerLimitPaint != null)
			g2.setPaint(lowerLimitPaint);
		lowerLimit.paint(g2);
	}
	if (upperLimit != null) {
		Paint upperLimitPaint = style.getUpperLimitPaint();
		if (upperLimitPaint != null)
			g2.setPaint(upperLimitPaint);
		else if (lowerLimitPaint != null)
			g2.setPaint(functionPaint);
		upperLimit.paint(g2);
	}

	g2.dispose();
}


abstract void
paintFunction(Graphics g);


private void
paintTerminators(Graphics2D g2) {
	int i = terminators.size();
	while (--i >= 0)
		g2.draw(terminators.get(i));
}


private final String DESCRIPTION;
private final BigDecimal[] X_COORDINATE;
private final BigDecimal[] Y_COORDINATE;
private final CoordinateBoundary COORDINATE_BOUNDARY;
private final ChartStyle style;
private final ArrayList<Shape> terminators = new ArrayList<Shape>();

private LowerLimitInstance lowerLimit = null;
private UpperLimitInstance upperLimit = null;

}
