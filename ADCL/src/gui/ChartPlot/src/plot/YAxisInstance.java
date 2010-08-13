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
final class YAxisInstance extends AxisInstance {

YAxisInstance(YAxis parent, BigDecimal min, BigDecimal max, int length) {
	super(parent, min, max, - length);
}


/**
 @param x the graphical position of the x-axis.
 */
Shape[]
renderAxis(float x) {
	float min = MIN.divide(GRAPHICS_SCALAR, MathContext.DECIMAL32).floatValue();
	float max = MAX.divide(GRAPHICS_SCALAR, MathContext.DECIMAL32).floatValue();
	Line2D axisLine = new Line2D.Float(x, min, x, max);

	if (ZERO.signum() == 0)	// no zig-zag
		return new Shape[] {axisLine};

	float zero = ZERO.divide(GRAPHICS_SCALAR, MathContext.DECIMAL32).floatValue();
	float length = (ZERO.signum() < 0 ? max : min) - zero;
	Shape[] render = new Shape[9];
	render[0] = axisLine;

	float y1 = zero;
	float y2 = y1 + 0.326f * length;
	render[1] = new Line2D.Float(x, y1, x, y2);
	float y3 = y2 + length / 35f;
	float x1 = x - length / 6f;
	render[2] = new Line2D.Float(x, y2, x1, y3);
	float y4 = y3 + length / 16f;
	float x2 = x + length / 4.5f;
	render[3] = new Line2D.Float(x1, y3, x2, y4);
	float y5 = y4 + length / 12f;
	float x3 = x - length / 3.5f;
	render[4] = new Line2D.Float(x2, y4, x3, y5);
	float y6 = y5 + length / 12f;
	render[5] = new Line2D.Float(x3, y5, x2, y6);
	float y7 = y6 + length / 16f;
	render[6] = new Line2D.Float(x2, y6, x1, y7);
	float y8 = y7 + length / 35f;
	render[7] = new Line2D.Float(x1, y7, x, y8);
	float y9 = y8 + 0.326f * length;
	render[8] = new Line2D.Float(x, y8, x, y9);

	return render;
}


/**
 @param nailRender gets filled with the index nails.
 @param labelX gets filled with the graphical vertical positions of the index labels.
 @param labelY gets filled with the graphical horizontal positions of the index labels.
 @param x the graphical position of the x-axis.
 @param nailLength the graphical length of the index nails.
 */
void
renderIndex(ArrayList<BigDecimal> indexValues, Shape[] nailRender, float[] labelX, float[] labelY, float x, float nailLength) {
	float halfNailLength = nailLength / 2f;
	int i = nailRender.length;
	assert i == indexValues.size() && i == labelX.length && i == labelY.length;
	while (--i >= 0) {
		float y = indexValues.get(i).divide(GRAPHICS_SCALAR, MathContext.DECIMAL32).floatValue();
		nailRender[i] = new Line2D.Float(x - halfNailLength, y, x + halfNailLength, y);
		labelX[i] = x;
		labelY[i] = y;
	}
}

}
