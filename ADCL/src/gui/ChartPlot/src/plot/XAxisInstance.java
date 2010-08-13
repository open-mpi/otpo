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
import java.awt.geom.*;
import java.math.*;
import java.util.*;


/**
 @author Pascal S. de Kloe
 */
final class XAxisInstance extends AxisInstance {

XAxisInstance(XAxis parent, BigDecimal min, BigDecimal max, int length) {
	super(parent, min, max, length);
}


/**
 @param y the graphical position of the y-axis.
 */
Shape[]
renderAxis(float y) {
	float min = MIN.divide(GRAPHICS_SCALAR, MathContext.DECIMAL32).floatValue();
	float max = MAX.divide(GRAPHICS_SCALAR, MathContext.DECIMAL32).floatValue();
	Line2D axisLine = new Line2D.Float(min, y, max, y);

	if (ZERO.signum() == 0)	// no zig-zag
		return new Shape[] {axisLine};

	float zero = ZERO.divide(GRAPHICS_SCALAR, MathContext.DECIMAL32).floatValue();
	float length = (ZERO.signum() < 0 ? max : min) - zero;
	Shape[] render = new Shape[9];
	render[0] = axisLine;

	float x1 = zero;
	float x2 = x1 + 0.326f * length;
	render[1] = new Line2D.Float(x1, y, x2, y);
	float x3 = x2 + length / 35f;
	float y1 = y - length / 6f;
	render[2] = new Line2D.Float(x2, y, x3, y1);
	float x4 = x3 + length / 16f;
	float y2 = y + length / 4.5f;
	render[3] = new Line2D.Float(x3, y1, x4, y2);
	float x5 = x4 + length / 12f;
	float y3 = y - length / 3.5f;
	render[4] = new Line2D.Float(x4, y2, x5, y3);
	float x6 = x5 + length / 12f;
	render[5] = new Line2D.Float(x5, y3, x6, y2);
	float x7 = x6 + length / 16f;
	render[6] = new Line2D.Float(x6, y2, x7, y1);
	float x8 = x7 + length / 35f;
	render[7] = new Line2D.Float(x7, y1, x8, y);
	float x9 = x8 + 0.326f * length;
	render[8] = new Line2D.Float(x8, y, x9, y);

	return render;
}


/**
 @param nailRender gets filled with the index nails.
 @param labelX gets filled with the graphical vertical positions of the index labels.
 @param labelY gets filled with the graphical horizontal positions of the index labels.
 @param y the graphical position of the y-axis.
 @param nailLength the graphical length of the index nails.
 */
void
renderIndex(ArrayList<BigDecimal> indexValues, Shape[] nailRender, float[] labelX, float[] labelY, float y, float nailLength) {
	float halfNailLength = nailLength / 2f;
	int i = nailRender.length;
	assert i == indexValues.size() && i == labelX.length && i == labelY.length;
	while (--i >= 0) {
		float x = indexValues.get(i).divide(GRAPHICS_SCALAR, MathContext.DECIMAL32).floatValue();
		nailRender[i] = new Line2D.Float(x, y - halfNailLength, x, y + halfNailLength);
		labelX[i] = x;
		labelY[i] = y;
	}
}

}
