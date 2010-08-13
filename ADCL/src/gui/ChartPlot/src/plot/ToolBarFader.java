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


/**
 @author Pascal S. de Kloe
 */
final class ToolBarFader implements MouseMotionListener, ActionListener {

ToolBarFader(ToolBar target) {
	toolBar = target;
	timer = new Timer(FADE_INTERVAL, this);
	timer.setInitialDelay(IDDLE_TIMEOUT);
	setIddleState();
}


public void
mouseDragged(MouseEvent e) {
	setIddleState();
}


public void
mouseMoved(MouseEvent e) {
	setIddleState();
}


/**
 * Shows the tool bar until IDDLE_TIMEOUT.
 */
private void
setIddleState() {
	timer.restart();
	if (fadeTrack != 0) {
		fadeTrack = 0;
		toolBar.setAlphaComposite((AlphaComposite) null);
		toolBar.setVisible(true);
		toolBar.repaint();
	}
}


/**
 * Fades the tool bar.
 */
public void
actionPerformed(ActionEvent e) {
	++fadeTrack;
	if (fadeTrack >= FADE_STEPS) {
		setVanishState();
		return;
	}
	float alpha = 1f - (float) fadeTrack / FADE_STEPS;
	toolBar.setAlphaComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, alpha));
	toolBar.repaint();
}


/**
 * Hides the tool bar.
 */
private void
setVanishState() {
	timer.stop();
	toolBar.setVisible(false);
}


private static final int IDDLE_TIMEOUT = 2000;
private static final int FADE_INTERVAL = 30;
private static final int FADE_STEPS = 10;
private int fadeTrack = 0;

private final ToolBar toolBar;
private final Timer timer;

}
