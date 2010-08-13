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
import java.util.*;


/**
 @author Pascal S. de Kloe
 */
final class GraphInstance {

/**
 @param xOffset the graphical x-coordinate of the zero point.
 @param yOffset the graphical y-coordinate of the zero point.
 */
GraphInstance(AxisInstance xAxis, AxisInstance yAxis, ArrayList<FunctionInstance> functions, int xOffset, int yOffset) {
	this.xAxis = xAxis;
	this.yAxis = yAxis;
	this.functions = functions;
	X_OFFSET = xOffset;
	Y_OFFSET = yOffset;
}


void
paint(Graphics g) {
	g = g.create();
	g.translate(X_OFFSET , Y_OFFSET);

	xAxis.paint(g);
	yAxis.paint(g);
	int i = functions.size();
	while (--i >= 0)
		functions.get(i).paint(g);

	g.dispose();
}


ArrayList<FunctionInstance>
getFunctionInstances() {
	return functions;
}


/**
 @see RenderUtils#getXValue(Graph, Point2D)
 */
BigDecimal
getXValue(int position) {
	BigDecimal offset = BigDecimal.valueOf(position - X_OFFSET);
	return offset.multiply(xAxis.GRAPHICS_SCALAR, MathContext.DECIMAL32);
}


/**
 @see RenderUtils#getYValue(Graph, Point2D)
 */
BigDecimal
getYValue(int position) {
	BigDecimal offset = BigDecimal.valueOf(position - Y_OFFSET);
	return offset.multiply(yAxis.GRAPHICS_SCALAR, MathContext.DECIMAL32);
}


private final ArrayList<FunctionInstance> functions;
private final AxisInstance xAxis;
private final AxisInstance yAxis;
private final int X_OFFSET;
private final int Y_OFFSET;

}
