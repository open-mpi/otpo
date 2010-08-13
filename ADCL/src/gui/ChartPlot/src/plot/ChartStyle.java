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


/**
 * Defines the style for one or more functions.
 @author Pascal S. de Kloe
 @since 1.0
 */
public final class ChartStyle {

public
ChartStyle() {
}


/**
 * Sets the chart type.
 * The default is {@link ChartType#LINE}.
 * Changes apply on the next {@link Graph#render() render}.
 @since 1.0
 */
public void
setType(ChartType type) {
	if (type == null)
		throw new IllegalArgumentException();
	chartType = type;
}


/**
 * Gets the chart type.
 @since 1.0
 */
public ChartType
getType() {
	return chartType;
}


/**
 * Sets the gap indicator factory.
 * The default is {@code null} which disables this feature.
 * Changes apply on the next {@link Graph#render() render}.
 @since 1.0
 */
public void
setTerminatorFactory(TerminatorFactory factory) {
	terminatorFactory = factory;
}


/**
 * Gets the gap indicator factory.
 @since 1.0
 */
public TerminatorFactory
getTerminatorFactory() {
	return terminatorFactory;
}


/**
 * Sets the paint for the chart.
 * The default is {@code null} which makes it the {@link Graph#getForeground foreground color}..
 * Changes apply on the next {@link Graph#paint(Graphics) paint}.
 @since 1.0
 */
public void
setPaint(Paint paint) {
	this.paint = paint;
}


/**
 * Gets the paint for the chart.
 @since 1.0
 */
public Paint
getPaint() {
	return paint;
}


/**
 * Sets the paint for the {@link #setLowerLimitEnabled(boolean) limit}.
 * The default is {@code null} which makes it the same as the {@link #setPaint(Paint) chart paint}.
 * Changes apply on the next {@link Graph#paint(Graphics) paint}.
 @since 1.4
 */
public void
setLowerLimitPaint(Paint paint) {
	lowerLimitPaint = paint;
}


/**
 * Gets the paint for the {@link #setLowerLimitEnabled(boolean) limit}.
 @since 1.4
 */
public Paint
getLowerLimitPaint() {
	return lowerLimitPaint;
}


/**
 * Sets the paint for the {@link #setUpperLimitEnabled(boolean) limit}.
 * The default is {@code null} which makes it the same as the {@link #setPaint(Paint) chart paint}.
 * Changes apply on the next {@link Graph#paint(Graphics) paint}.
 @since 1.4
 */
public void
setUpperLimitPaint(Paint paint) {
	upperLimitPaint = paint;
}


/**
 * Gets the paint for the {@link #setUpperLimitEnabled(boolean) limit}.
 @since 1.4
 */
public Paint
getUpperLimitPaint() {
	return upperLimitPaint;
}


/**
 * Sets whether the the smallest value in the range is shown as a horizontal line on the graph.
 * This feature is disabled by default.
 * Changes apply on the next {@link Graph#render() render}.
 @since 1.4
 @see Graph#setDomain(GraphDomain) Restrictions in the domain can effect the range.
 */
public void
setLowerLimitEnabled(boolean enabled) {
	lowerLimitEnabled = enabled;
}


/**
 * Gets whether the the smallest value in the range is shown as a horizontal line on the graph.
 @since 1.4
 */
public boolean
getLowerLimitEnabled() {
	return lowerLimitEnabled;
}


/**
 * Sets whether the the largest value in the range is shown as a horizontal line on the graph.
 * This feature is disabled by default.
 * Changes apply on the next {@link Graph#render() render}.
 @since 1.4
 @see Graph#setDomain(GraphDomain) Restrictions in the domain can effect the range.
 */
public void
setUpperLimitEnabled(boolean enabled) {
	upperLimitEnabled = enabled;
}


/**
 * Gets whether the the largest value in the range is shown as a horizontal line on the graph.
 @since 1.4
 */
public boolean
getUpperLimitEnabled() {
	return upperLimitEnabled;
}


private ChartType chartType = ChartType.LINE;
private TerminatorFactory terminatorFactory = null;
private Paint paint = null;
private Paint upperLimitPaint = null;
private Paint lowerLimitPaint = null;
private boolean lowerLimitEnabled = false;
private boolean upperLimitEnabled = false;

}
