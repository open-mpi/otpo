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
import java.math.*;
import java.text.*;


/**
 * Represents one axis/dimension.
 @author Pascal S. de Kloe
 @since 1.0
 */
public abstract class Axis {

Axis(String description, IndexScheme scheme, Format format, float labelOffsetX, float labelOffsetY) {
	if (scheme == null || format == null)
		throw new IllegalArgumentException();
	DESCRIPTION = description == null ? "" : description;
	INDEX_SCHEME = scheme;
	FORMAT = format;
	setLabelOffsetX(labelOffsetX);
	setLabelOffsetY(labelOffsetY);
}


/**
 @param min the lowest value in this dimension.
 @param max the highest value in this dimension.
 @param length the graphical length of this dimension.
 */
abstract AxisInstance
getInstance(BigDecimal min, BigDecimal max, int length);


/**
 * Sets the paint of the axis and the index nails.
 * The default is {@code null} which means that the {@link Graph#getForeground() foreground color} will be used.
 * Changes apply on the next {@link Graph#paint(Graphics) paint}.
 @since 1.0
 */
public final void
setPaint(Paint paint) {
	this.paint = paint;
}


/**
 * Gets the paint of the axis and the index nails.
 @since 1.0
 */
public final Paint
getPaint() {
	return paint;
}


/**
 * Sets the paint of the index labels.
 * The default is {@code null} which means that the {@link #setPaint(Paint) axis paint} will be used.
 * Changes apply on the next {@link Graph#paint(Graphics) paint}.
 @since 1.0
 */
public final void
setLabelPaint(Paint paint) {
	labelPaint = paint;
}


/**
 * Gets the paint of the index labels.
 @since 1.0
 */
public final Paint
getLabelPaint() {
	return labelPaint;
}


/**
 * Sets the stroke for the axis.
 * Changes apply on the next {@link Graph#paint(Graphics) paint}.
 @since 1.0
 */
public final void
setStroke(Stroke stroke) {
	if (stroke == null)
		throw new IllegalArgumentException();
	this.stroke = stroke;
}


/**
 * Gets the stroke for the axis.
 @since 1.0
 */
public final Stroke
getStroke() {
	return stroke;
}


/**
 * Sets the stroke for the index nails.
 * Changes apply on the next {@link Graph#paint(Graphics) paint}.
 @since 1.0
 */
public final void
setNailStroke(Stroke stroke) {
	if (stroke == null)
		throw new IllegalArgumentException();
	nailStroke = stroke;
}


/**
 * Gets the stroke for the index nails.
 @since 1.0
 */
public final Stroke
getNailStroke() {
	return nailStroke;
}


/**
 * Sets the lenght of the index nails.
 * The default is 6. Changes apply on the next {@link Graph#render() render}.
 @since 1.0
 */
public final void
setNailLength(float length) {
	if (Float.isNaN(length) || Float.isInfinite(length) || length < 0.0)
		throw new IllegalArgumentException();
	nailLength = length;
}


/**
 * Gets the lenght of the index nails.
 @since 1.0
 */
public final float
getNailLength() {
	return nailLength;
}


/**
 * Sets the amount of horizontal space between each index label and their position on the axis.
 @since 1.0
 */
public final void
setLabelOffsetX(float offset) {
	if (Float.isNaN(offset) || Float.isInfinite(offset))
		throw new IllegalArgumentException();
	labelOffsetX = offset;
}


/**
 * Gets the amount of horizontal space between each index label and their position on the axis.
 @since 1.0
 */
public final float
getLabelOffsetX() {
	return labelOffsetX;
}


/**
 * Sets the amount of vertical space between each index label and their position on the axis.
 @since 1.0
 */
public final void
setLabelOffsetY(float offset) {
	if (Float.isNaN(offset) || Float.isInfinite(offset))
		throw new IllegalArgumentException();
	labelOffsetY = offset;
}


/**
 * Gets the amount of vertical space between each index label and their position on the axis.
 @since 1.0
 */
public final float
getLabelOffsetY() {
	return labelOffsetY;
}


/**
 * Sets the preferred number of steps on the axis index.
 * There are two special cases. Zero will disable any labels and one will minimize the label usage to one at each end that is not
 * connected to another axis.
 * The default is 6. Changes apply on the next {@link Graph#render() render}.
 @throws IllegalArgumentException if {@code steps} is larger than 99.
 @since 1.0
 */
public final void
setPreferredSteps(int steps) {
	if (steps < 0 || steps >= 100)
		throw new IllegalArgumentException();
	preferredSteps = steps;
}


/**
 * Gets the preferred number of steps on the axis index.
 @since 1.0
 */
public final int
getPreferredSteps() {
	return preferredSteps;
}


/**
 * Sets the minimal part of unused space needed to make a zig-zag in the axis.
 * This feature is disabled by default. Changes apply on the next {@link Graph#render() render}.
 @param unusedPart use a value within interval [½, 1) or {@code null} to disable.
 @since 1.1
 */
public final void
setZigZaginess(BigDecimal unusedPart) {
	if (unusedPart != null && (unusedPart.compareTo(BigDecimal.valueOf(5, 1)) < 0 || unusedPart.compareTo(BigDecimal.ONE) >= 0))
		throw new IllegalArgumentException();
	zigZaginess = unusedPart;
}


/**
 * Gets the minimal part of unused space needed to make a zig-zag in the axis.
 @since 1.1
 */
public final BigDecimal
getZigZaginess() {
	return zigZaginess;
}


/**
 * Gets the short description as defined with the constructor.
 */
public final String
toString() {
	return DESCRIPTION;
}


final IndexScheme
getIndexScheme() {
	return INDEX_SCHEME;
}


final Format
getFormat() {
	return FORMAT;
}


private final String DESCRIPTION;
private final IndexScheme INDEX_SCHEME;
private final Format FORMAT;
private float labelOffsetX;
private float labelOffsetY;
private Paint paint = null;
private Paint labelPaint = null;
private Stroke stroke = new BasicStroke(2f);
private Stroke nailStroke = new BasicStroke(1f);
private float nailLength = 6;
private int preferredSteps = 6;
private BigDecimal zigZaginess = null;

}
