package plot;

/*
Copyright (c) 2005 Pascal S. de Kloe. All rights reserved.

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
import java.awt.geom.AffineTransform;
import java.math.*;
import java.text.*;
import java.util.*;


/**
 @author Pascal S. de Kloe
 */
abstract class AxisInstance {

/**
 @param parent the source of this instance.
 @param min the lowest value in this dimension.
 @param max the highest value in this dimension.
 @param length the graphical length of this dimension. This can be negative to make {@link #GRAPHICS_SCALAR} negative since
 * graphical y-coordinates count down.
 */
AxisInstance(Axis parent, BigDecimal min, BigDecimal max, int length) 
{
	assert min.compareTo(max) < 0 && length != 0;
	axis = parent;

	boolean zigZag = needZigZag(min, max, parent.getZigZaginess());
	if (! zigZag) {
		if (max.signum() < 0)
			max = BigDecimal.ZERO;
		else if (min.signum() == 1)
			min = BigDecimal.ZERO;
	}

	int preferredSteps = parent.getPreferredSteps();
	indexValues = parent.getIndexScheme().getIndexValues(min, max, preferredSteps);
	MIN = indexValues.get(0);
	MAX = indexValues.get(indexValues.size() - 1);
	assert MIN.compareTo(min) <= 0 && MAX.compareTo(max) >= 0;
	if (preferredSteps == 0)
		indexValues.clear();

	if (! zigZag) {
		ZERO = BigDecimal.ZERO;
		GRAPHICS_SCALAR = getGraphicsScalar(MIN, MAX, length);
	} else {
		BigDecimal zigZagLength = MAX.subtract(MIN).movePointLeft(1);
		if (MAX.signum() < 0) {
			ZERO = MAX.add(zigZagLength);
			GRAPHICS_SCALAR = getGraphicsScalar(MIN, ZERO, length);
		} else {
			assert MIN.signum() == 1;
			ZERO = MIN.subtract(zigZagLength);
			GRAPHICS_SCALAR = getGraphicsScalar(ZERO, MAX, length);
		}
	}
}

private static boolean
needZigZag(BigDecimal min, BigDecimal max, BigDecimal zigZaginess) {
	assert min.compareTo(max) < 0;
	if (zigZaginess == null) return false;

	int signum = min.signum();
	if (signum < max.signum()) return false;
	assert signum == max.signum();
	
	BigDecimal range = signum < 0 ? min : max;
	BigDecimal start = signum < 0 ? max : min;
	BigDecimal unusedPart = start.divide(range, zigZaginess.scale(), RoundingMode.HALF_UP);
	assert unusedPart.signum() >= 0 && unusedPart.compareTo(BigDecimal.ONE) <= 0;

	return unusedPart.compareTo(zigZaginess) >= 0;
}


private static BigDecimal
getGraphicsScalar(BigDecimal min, BigDecimal max, int graphicalLength) {
	assert min.compareTo(max) < 0 && graphicalLength != 0;
	BigDecimal range = max.subtract(min);
	BigDecimal length = BigDecimal.valueOf(graphicalLength);
	BigDecimal scalar = range.divide(length, MathContext.DECIMAL32);
	assert scalar.signum() != 0;
	return scalar;
}


final void
paint(Graphics g) {
	Graphics2D g2 = (Graphics2D) g.create();

	// paint axis:
	Paint axisPaint = axis.getPaint();
	if (axisPaint != null)
		g2.setPaint(axisPaint);
	g2.setStroke(axis.getStroke());
	for (int i = axisRender.length; --i >= 0; g2.draw(axisRender[i]));

	if (nailRender == null) return;		// no axis index
	final int LABELS = nailRender.length;
	assert LABELS == labelText.length && LABELS == labelX.length && LABELS == labelY.length;

	// paint index nails:
	g2.setStroke(axis.getNailStroke());
	for (int i = LABELS; --i >= 0; g2.draw(nailRender[i]));

	// paint index labels:
	Paint labelPaint = axis.getLabelPaint();
	if (labelPaint != null)
		g2.setPaint(labelPaint);
	for (int i = LABELS; --i >= 0; g2.drawString(labelText[i], labelX[i], labelY[i]));

    if(axis.getClass() == YAxis.class)
        drawTextVertical(g, nailRender[LABELS-1].getBounds().x - 40, nailRender[(LABELS-1)/2].getBounds().y, -90 * java.lang.Math.PI/180, axis.toString());
    else
        g2.drawString(axis.toString(), nailRender[(LABELS-1)/2].getBounds().x, nailRender[LABELS-1].getBounds().y+40);

	g2.dispose();
}

private void drawTextVertical(Graphics g, double x, double y, double theta, String label)
{
     Graphics2D g2D = (Graphics2D)g;
    // Create a rotation transformation for the font.
    AffineTransform fontAT = new AffineTransform();
    // get the current font
    Font theFont = g2D.getFont();
    // Derive a new font using a rotatation transform
    fontAT.rotate(theta);
    Font theDerivedFont = theFont.deriveFont(fontAT);
    // set the derived font in the Graphics2D context
    g2D.setFont(theDerivedFont);
    // Render a string using the derived font
    g2D.drawString(label, (int)x, (int)y);
    // put the original font back
    g2D.setFont(theFont);
}

/**
 @param otherZero the graphical position of zero in the other dimension.
 */
final void
render(float otherZero, java.awt.FontMetrics fontMetrics) {
	axisRender = renderAxis(otherZero);
	renderIndex(otherZero);
	alignLabels(fontMetrics);
}


private void
renderIndex(float otherZero) {
	// Don't label zero:
	int i = indexValues.size();
	while (--i >= 0)
		if (indexValues.get(i).signum() == 0) {
			indexValues.remove(i);
			break;
		}

	i = indexValues.size();
	nailRender = new Shape[i];
	labelText = new String[i];
	labelX = new float[i];
	labelY = new float[i];

	Format format = axis.getFormat();
	while (--i >= 0)
		labelText[i] = format.format(indexValues.get(i));

	renderIndex(indexValues, nailRender, labelX, labelY, otherZero, axis.getNailLength());
}


private void
alignLabels(java.awt.FontMetrics fontMetrics) {
	float xOffset = axis.getLabelOffsetX();
	float yOffset = axis.getLabelOffsetY();
	if (yOffset != 0) {
		if (yOffset < 0)
			yOffset -= fontMetrics.getAscent();
		else
			yOffset += fontMetrics.getDescent();
	}

	int i = labelText.length;
	if (xOffset < 0) {
		while (--i >= 0) {
			labelX[i] += xOffset - fontMetrics.stringWidth(labelText[i]);
			labelY[i] -= yOffset;
		}
	} else {
		while (--i >= 0) {
			labelX[i] += xOffset;
			labelY[i] -= yOffset;
		}
	}
}


abstract Shape[]
renderAxis(float otherZero);


abstract void
renderIndex(ArrayList<BigDecimal> indexValues, Shape[] nailRender, float[] labelX, float[] labelY, float otherZero, float nailLength);


/** The lowest value on the axis. */
final BigDecimal MIN;

/** The highest value on the axis. */
final BigDecimal MAX;

/** The location of zero. This is zero unless there is a zig-zag. */
final BigDecimal ZERO;

/** The scalar to transform the graphical position to values in this dimension. */
final BigDecimal GRAPHICS_SCALAR;

private final Axis axis;
private final ArrayList<BigDecimal> indexValues;

// Render storage:
private Shape[] axisRender = null;	// axis line with possible zig-zag
private Shape[] nailRender = null;	// index nails
private String[] labelText = null;	// index label contents
private float[] labelX = null;		// index label horizontal positions
private float[] labelY = null;		// index label vertical positions

}
