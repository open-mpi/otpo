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

import java.math.BigDecimal;


/**
 * Defines the boundaries from one or more coordinates.
 @author Pascal S. de Kloe
 */
final class CoordinateBoundary {

/**
 @param xMin the lowest x-coordinate.
 @param xMax the highest x-coordinate.
 @param yMin the lowest y-coordinate.
 @param yMax the highest y-coordinate.
 */
CoordinateBoundary(BigDecimal xMin, BigDecimal xMax, BigDecimal yMin, BigDecimal yMax) {
	assert xMin.compareTo(xMax) <= 0;
	assert yMin.compareTo(yMax) <= 0;
	X_MIN = xMin;
	X_MAX = xMax;
	Y_MIN = yMin;
	Y_MAX = yMax;
}


BigDecimal
getXMin() {
	return X_MIN;
}


BigDecimal
getXMax() {
	return X_MAX;
}


BigDecimal
getYMin() {
	return Y_MIN;
}


BigDecimal
getYMax() {
	return Y_MAX;
}


/**
 * Makes sure that the area is visible in a graph.
 */
CoordinateBoundary
getVisual() {
	BigDecimal xMin = X_MIN;
	BigDecimal xMax = X_MAX;
	BigDecimal yMin = Y_MIN;
	BigDecimal yMax = Y_MAX;

	if (xMin.compareTo(xMax) == 0) {
		int signum = xMin.signum();
		if (signum == 0) {
			xMin = BigDecimal.valueOf(-1);
			xMax = BigDecimal.ONE;
		} else {
			if (signum < 0)
				xMax = BigDecimal.ZERO;
			else
				xMin = BigDecimal.ZERO;
		}
	}

	if (yMin.compareTo(yMax) == 0) {
		int signum = yMin.signum();
		if (signum == 0) {
			yMin = BigDecimal.valueOf(-1L);
			yMax = BigDecimal.ONE;
		} else {
			if (signum < 0)
				yMax = BigDecimal.ZERO;
			else
				yMin = BigDecimal.ZERO;
		}
	}

	return new CoordinateBoundary(xMin, xMax, yMin, yMax);
}


private final BigDecimal X_MIN;
private final BigDecimal X_MAX;
private final BigDecimal Y_MIN;
private final BigDecimal Y_MAX;

}
