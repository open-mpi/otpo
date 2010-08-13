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
import java.util.*;
import java.math.*;


/**
 @author Pascal S. de Kloe
 @see ChartType#STEP
 */
final class StepChartInstance extends FunctionInstance {

StepChartInstance(String description, BigDecimal[] argument, BigDecimal[] value, ChartStyle style) {
	super(description, argument, value, style);
}


void
renderFunction(BigDecimal[] xCoordinate, BigDecimal[] yCoordinate, BigDecimal xScalar, BigDecimal yScalar) {
	assert xCoordinate.length == yCoordinate.length;
	final int COORDINATES = xCoordinate.length;
	ArrayList<BigDecimal> render = new ArrayList<BigDecimal>();

	BigDecimal oldX = xCoordinate[0];
	BigDecimal oldY = yCoordinate[0];

	for (int i = 1; i < COORDINATES; ++i) {
		BigDecimal x = xCoordinate[i];
		BigDecimal y = yCoordinate[i];

		if (y != null) {
			if (oldY != null) {
				if (y.compareTo(oldY) == 0)
					continue;
				render.add(oldX);
				render.add(x);
				render.add(oldY);
			} else
				addTerminator(x, y, xScalar, yScalar);
		} else if (oldY != null) {
			BigDecimal previousX = xCoordinate[i - 1];
			addTerminator(previousX, oldY, xScalar, yScalar);
			render.add(oldX);
			render.add(previousX);
			render.add(oldY);
		}

		oldX = x;
		oldY = y;
	}

	if (oldY != null && oldX != xCoordinate[COORDINATES - 1]) {
		render.add(oldX);
		render.add(xCoordinate[COORDINATES - 1]);
		render.add(oldY);
	}

	plot = toArray(render, xScalar, yScalar);
}


private static int[]
toArray(ArrayList<BigDecimal> render, BigDecimal xScalar, BigDecimal yScalar) {
	int i = render.size();
	int[] array = new int[i];
	assert i % 3 == 0;
	while (i != 0) {
		--i;
		array[i] = render.get(i).divide(yScalar, 0, RoundingMode.HALF_UP).intValue();
		--i;
		array[i] = render.get(i).divide(xScalar, 0, RoundingMode.HALF_UP).intValue();
		--i;
		array[i] = render.get(i).divide(xScalar, 0, RoundingMode.HALF_UP).intValue();
	}
	return array;
}


void
paintFunction(Graphics g) {
	int i = plot.length;
	assert i % 3 == 0;
	while (i != 0) {
		int y = plot[--i];
		int x2 = plot[--i];
		int x1 = plot[--i];
		g.drawLine(x1, y, x2, y);
	}
}


private int[] plot = null;

}
