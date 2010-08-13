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
 @see ChartType#LINE
 */
final class LineChartInstance extends FunctionInstance {

LineChartInstance(String description, BigDecimal[] argument, BigDecimal[] value, ChartStyle style) {
	super(description, argument, value, style);
}


void
renderFunction(BigDecimal[] xCoordinate, BigDecimal[] yCoordinate, BigDecimal xScalar, BigDecimal yScalar) {
	assert xCoordinate.length == yCoordinate.length;
	int i = xCoordinate.length - 1;

	ArrayList<BigDecimal> bufferX = new ArrayList<BigDecimal>(i);
	ArrayList<BigDecimal> bufferY = new ArrayList<BigDecimal>(i);
	BigDecimal oldX = xCoordinate[i];
	BigDecimal oldY = yCoordinate[i];

	if (oldY != null) {
		bufferX.add(oldX);
		bufferY.add(oldY);
	}

	while (--i >= 0) {
		BigDecimal x = xCoordinate[i];
		BigDecimal y = yCoordinate[i];

		if (y != null) {
			bufferX.add(x);
			bufferY.add(y);
			if (oldY == null)
				addTerminator(x, y, xScalar, yScalar);
		} else if (oldY != null) {
			addTerminator(oldX, oldY, xScalar, yScalar);
			polylinesX.add(toCoordinates(bufferX, xScalar));
			polylinesY.add(toCoordinates(bufferY, yScalar));
		}

		oldX = x;
		oldY = y;
	}

	if (oldY != null) {
		polylinesX.add(toCoordinates(bufferX, xScalar));
		polylinesY.add(toCoordinates(bufferY, yScalar));
	}
}


private static int[]
toCoordinates(ArrayList<BigDecimal> values, BigDecimal scalar) {
	int i = values.size();
	int[] array = new int[i];
	while (--i >= 0)
		array[i] = values.get(i).divide(scalar, 0, RoundingMode.HALF_UP).intValue();
	return array;
}


void
paintFunction(Graphics g) {
	int i = polylinesX.size();
	while (--i >= 0) {
		int[] x = polylinesX.get(i);
		int[] y = polylinesY.get(i);
		g.drawPolyline(x, y, x.length);
	}
}


private final ArrayList<int[]> polylinesX = new ArrayList<int[]>();
private final ArrayList<int[]> polylinesY = new ArrayList<int[]>();

}
