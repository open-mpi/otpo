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
import java.awt.geom.*;
import java.math.*;


/**
 * The factory produces shapes used to indicate a gap in the function line, i.e. mark nonexistence.
 * This default implementation uses circles.
 @author Pascal S. de Kloe
 @since 1.0
 */
public class TerminatorFactory {

/**
 @since 1.0
 */
public
TerminatorFactory() {
	this(4.0);
}


/**
 * Exists for backwards compatibility. Use {@link #TerminatorFactory(double)} instead.
 @param radius the radius of the terminator shape.
 @since 1.0
 */
public
TerminatorFactory(float radius) {
	this((double) radius);
}


/**
 @param radius the radius of the terminator shape.
 @since 1.5
 */
public
TerminatorFactory(double radius) {
	if (radius >= 0.0)
		RADIUS = radius;
	else	// catches not-a-number too
		throw new IllegalArgumentException();
}


/**
 * Produces a terminator.
 @param argument the x-coordinate.
 @param value the y-coordinate.
 @param xScalar transforms horizontal graphical positions to x-coordinates.
 @param yScalar transforms vertical graphical positions to y-coordinates.
 @return a terminator on position ({@code argument} / {@code xScalar}, {@code value} / {@code yScalar}).
 @since 1.0
 */
protected Shape
getTerminator(BigDecimal argument, BigDecimal value, BigDecimal xScalar, BigDecimal yScalar) {
	double x = argument.divide(xScalar, MathContext.DECIMAL64).doubleValue();
	double y = value.divide(yScalar, MathContext.DECIMAL64).doubleValue();
	x -= RADIUS;
	y -= RADIUS;
	double d = 2 * RADIUS;
	return new Ellipse2D.Double(x, y, d, d);
}


private final double RADIUS;

}
