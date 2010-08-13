package plot;

/*
Copyright (c) 2006 Pascal S. de Kloe. All rights reserved.

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
 */
final class UpperLimitInstance {

UpperLimitInstance(CoordinateBoundary boundary, BigDecimal xScalar, BigDecimal yScalar, Format yFormat, FontMetrics fontMetrics) {
	X_MIN = boundary.getXMin().divide(xScalar, 0, RoundingMode.HALF_UP).intValue();
	X_MAX = boundary.getXMax().divide(xScalar, 0, RoundingMode.HALF_UP).intValue();
	int x = -X_MIN > X_MAX ? X_MIN : X_MAX;

	BigDecimal limit = boundary.getYMax();
	LABEL = yFormat.format(limit);
	int y = limit.divide(yScalar, 0, RoundingMode.HALF_UP).intValue();
	Y = y;

	ComponentOrientation orientation = ComponentOrientation.getOrientation(Locale.getDefault());
	y -= fontMetrics.getMaxDescent();
	if (orientation.isHorizontal()) {
		if (x < 0) {
			if (! orientation.isLeftToRight())
				x += fontMetrics.stringWidth(LABEL);
		} else {
			if (orientation.isLeftToRight())
				x -= fontMetrics.stringWidth(LABEL);
		}
	} else
		y -= fontMetrics.stringWidth(LABEL);

	LABEL_X = x;
	LABEL_Y = y;
}


void
paint(Graphics g) {
	g.drawLine(X_MIN, Y, X_MAX, Y);
	g.drawString(LABEL, LABEL_X, LABEL_Y);
}


private final int X_MIN;
private final int X_MAX;
private final int Y;
private final int LABEL_X;
private final int LABEL_Y;
private final String LABEL;

}
