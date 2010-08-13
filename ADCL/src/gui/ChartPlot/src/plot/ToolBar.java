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
import java.awt.event.*;
import javax.swing.*;
import utility.*;


/**
 @author Pascal S. de Kloe
 */
@SuppressWarnings("serial")
final class ToolBar extends JToolBar implements ComponentListener {

ToolBar(Graph source) {
	graph = source;
	abortZoom.setVisible(false);
	setDoubleBuffered(true);

	setLayout(new FlowLayout(FlowLayout.TRAILING));
	add(interval);
	add(abortZoom);
	add(export);
	add(print);

	interval.setOpaque(true);
	int rgb = interval.getBackground().getRGB();
	rgb &= 0x00FFFFFF;	// strip alpha
	rgb |= 0xC0000000;	// make transparent
	interval.setBackground(new Color(rgb, true));

	interval.setToolTipText("Zoom interval");
	abortZoom.setToolTipText("Abort Zoom");
	export.setToolTipText("Export...");
	print.setToolTipText("Print...");
	abortZoom.setCursor(ACTION_CURSOR);
	export.setCursor(ACTION_CURSOR);
	print.setCursor(ACTION_CURSOR);

	graph.addComponentListener(this);
	abortZoom.addMouseListener(new MouseListener() {

		public void mouseClicked(MouseEvent event) {
			int modifiers = event.getModifiers();
			if ((modifiers & InputEvent.BUTTON1_MASK) != 0) {
				currentInterval = null;
				interval.setText(null);
				abortZoom.setVisible(false);

				graph.setEnabled(false);
				graph.setCursor(InteractiveGraph.BUSSY_CURSOR);
				new Thread() {

					@Override
					public void run() {
						try {
							graph.setDomain(new GraphDomain(null, null));
							graph.render();
							graph.repaint();
						} finally {
							graph.setEnabled(true);
							graph.setCursor(InteractiveGraph.DEFAULT_CURSOR);
						}
					}

				}.start();
			}
		}


		// Unused methods from the MouseListener interface:
		public void mousePressed(MouseEvent event) { }
		public void mouseReleased(MouseEvent event) { }
		public void mouseEntered(MouseEvent event) { }
		public void mouseExited(MouseEvent event) { }

	});


	export.addMouseListener(new ExportListener(graph));

	print.addMouseListener(new MouseListener() {

		public void mouseClicked(MouseEvent event) {
			int modifiers = event.getModifiers();
			if ((modifiers & InputEvent.BUTTON1_MASK) != 0) {
				print.setEnabled(false);
				new Thread() {

					@Override
					public void run() {
						try {
							RenderUtils.print(graph);
						} finally {
							print.setEnabled(true);
						}
					}

				}.start();
			}
		}


		// Unused interface methods:
		public void mousePressed(MouseEvent event) { }
		public void mouseReleased(MouseEvent event) { }
		public void mouseEntered(MouseEvent event) { }
		public void mouseExited(MouseEvent event) { }

	});
}


/**
 * Sets the transparency of the tool bar.
 * Use {@code null} to disable this feature.
 */
void
setAlphaComposite(AlphaComposite composite) {
	transparency = composite;
}


@Override
public void
paint(Graphics g) {
	if (transparency != null) {
		Graphics2D g2 = (Graphics2D) g;
		g2.setComposite(transparency);
	}
	super.paint(g);
}


/**
 * Doesn't print this component.
 */
@Override
public void
print(Graphics g) {
}


void
setZoomPending(String intervalDescription) {
	if (intervalDescription == null)
		interval.setText(currentInterval);
	else
		interval.setText(intervalDescription);
}


void
setZoom(String intervalDescription) {
	currentInterval = intervalDescription;
	interval.setText(intervalDescription);
	abortZoom.setVisible(true);
}


public void
componentResized(ComponentEvent event) {
	setAppropriateIcons();
}


// Unused interface methods:
public void componentMoved(ComponentEvent event) { };
public void componentShown(ComponentEvent event) { };
public void componentHidden(ComponentEvent event) { };


private void
setAppropriateIcons() {
	int size = (graph.getWidth() + graph.getHeight()) / 40 + 2;
	abortZoom.setIcon(new ImageIcon(ABORT_ZOOM_IMAGE.getScaledInstance(size, size, Image.SCALE_SMOOTH)));
	export.setIcon(new ImageIcon(EXPORT_IMAGE.getScaledInstance(size, size, Image.SCALE_SMOOTH)));
	print.setIcon(new ImageIcon(PRINT_IMAGE.getScaledInstance(size, size, Image.SCALE_SMOOTH)));
}


private static final Cursor ACTION_CURSOR = new Cursor(Cursor.HAND_CURSOR);
private Utility util = Utility.getInstance();
private final Image ABORT_ZOOM_IMAGE = util.getImage("/icons/fit-width.gif");
private final Image EXPORT_IMAGE = util.getImage("/icons/export.gif");
private final Image PRINT_IMAGE = util.getImage("/icons/print.gif");

private final Graph graph;
private final JLabel interval = new JLabel();
private final JLabel abortZoom = new JLabel();
private final JLabel export = new JLabel();
private final JLabel print = new JLabel();

private AlphaComposite transparency = null;
private String currentInterval = null;

}
