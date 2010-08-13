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

import java.util.*;
import java.math.*;


/**
 * A collection of points to represent a function.
 @author Pascal S. de Kloe
 @since 1.0
 */
@SuppressWarnings("serial")
public class Function implements java.io.Serializable {

/**
 @param description a short description of the function.
 @since 1.0
 */
public
Function(String description) {
	DESCRIPTION = description == null ? "" : description;
}


/**
 * Defines a ordered pair (x, f(x)).
 @param x the argument.
 @param y the value at {@code x} or {@code null} if f({@code x}) is not defined.
 @since 1.0
 */
public final void
addPoint(BigDecimal x, BigDecimal y) {
	if (x == null)
		throw new IllegalArgumentException();
	synchronized (map) {
		map.put(x, y);
	}
}


final FunctionInstance
getInstance(GraphDomain domain, ChartStyle style) {
	BigDecimal from = domain.FROM;
	BigDecimal until = domain.UNTIL;
	if (from == null) {
		if (until == null)
			return getInstance(map, style);
		return getInstance(map.headMap(until), style);
	}
	if (until == null)
		return getInstance(map.tailMap(from), style);
	return getInstance(map.subMap(from, until), style);
}


private FunctionInstance
getInstance(SortedMap<BigDecimal,BigDecimal> submap, ChartStyle style) {
	Collection<BigDecimal> arguments = submap.keySet();
	Collection<BigDecimal> values = submap.values();
	BigDecimal[] x, y;
	synchronized (map) {
		int size = submap.size();
		x = new BigDecimal[size];
		y = new BigDecimal[size];
		arguments.toArray(x);
		values.toArray(y);
	}
	return style.getType().getInstance(DESCRIPTION, x, y, style);
}


/**
 * Gets a short description of the fuction.
 @since 1.0
 */
public String
toString() {
	return DESCRIPTION;
}


private final String DESCRIPTION;
private final SortedMap<BigDecimal,BigDecimal> map = new TreeMap<BigDecimal,BigDecimal>();

}
