package plot;

/*
Copyright (c) 2007 Pascal S. de Kloe. All rights reserved.

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
import java.text.Format;
import javax.swing.*;


/**
 @author Pascal S. de Kloe
 */
@SuppressWarnings("serial")
final class ZoomSelection extends JComponent {

ZoomSelection(InteractiveGraph graph, Point start) {
	this.graph = graph;
	origin = start;
	current = start;
	graphHeight = graph.getHeight();
	format = graph.getXAxis().getFormat();
	setSize(graph.getSize());
	graph.setCursor(DRAG_EAST_CURSOR);
}


void
setMousePosition(Point position) {
	current = position;
}


/**
 * Gets the mathematical notation.
 */
String
getIntervalDescription() {
	GraphInstance render = graph.getRender();
	if (render == null)
		return "";
	BigDecimal a = render.getXValue(origin.x);
	BigDecimal b = render.getXValue(current.x);

	StringBuffer buffer = new StringBuffer(40);
	buffer.append('[');
	if (a.compareTo(b) < 0) {
		graph.setCursor(DRAG_EAST_CURSOR);
		buffer.append(format.format(a));
		buffer.append(", ");
		buffer.append(format.format(b));
	} else {
		graph.setCursor(DRAG_WEST_CURSOR);
		buffer.append(format.format(b));
		buffer.append(", ");
		buffer.append(format.format(a));
	}
	buffer.append(')');
	
	return buffer.toString();
}


boolean
apply() {
	GraphInstance render = graph.getRender();
	if (render == null)
		return false;
	final BigDecimal A = render.getXValue(origin.x);
	final BigDecimal B = render.getXValue(current.x);
	final int compare = A.compareTo(B);
	if (compare == 0)
		return false;

	graph.setEnabled(false);
	graph.setCursor(graph.BUSSY_CURSOR);
	new Thread() {

		public void run() {
			try {
				if (compare < 0)
					graph.setDomain(new GraphDomain(A, B));
				else
					graph.setDomain(new GraphDomain(B, A));
				graph.render();
				graph.repaint();
			} finally {
				graph.setEnabled(true);
				graph.setCursor(graph.DEFAULT_CURSOR);
			}
		}

	}.start();
	return true;
}


public void
paintComponent(Graphics g) {
	int x =	Math.min(origin.x, current.x);
	int y = 0;
	int width = Math.max(origin.x, current.x) - x;
	int height = graphHeight - 1;
	g.setColor(SELECTION_COLOR);
	g.drawRect(x, y, width, height);

	++x;
	++y;
	--width;
	--height;
	g.setColor(SELECTION_TRANSPARENT);
	g.fillRect(x, y, width, height);
}


private static final Cursor DRAG_EAST_CURSOR = new Cursor(Cursor.E_RESIZE_CURSOR);
private static final Cursor DRAG_WEST_CURSOR = new Cursor(Cursor.W_RESIZE_CURSOR);

private final Color SELECTION_COLOR = UIManager.getColor("textHighlight");
private final Color SELECTION_TRANSPARENT = new Color(SELECTION_COLOR.getRGB() & 0x00FFFFFF | 0x77000000, true);

private final InteractiveGraph graph;
private final int graphHeight;
private final Format format;
private final Point origin;
private Point current;

}
