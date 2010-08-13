package eventpackage;

import java.util.ArrayList;
import java.util.EventObject;
import java.util.Iterator;
import java.util.List;

import communicationmanager.Header;


	
public class EventGenerator 
{
		byte[] recvdata;
		Header _head;
	    private List _listeners = new ArrayList();
	    
	    public synchronized void dataReceived(byte[] data, Header head) 
	    {
	       recvdata = data;
	       _head = head;
	       _fireRecvEvent();
	    }
	    
	    public synchronized void addRecvListener( CustomEventListener l ) {
	        _listeners.add( l );
	    }
	    
	    public synchronized void removeRecvListener( CustomEventListener l ) {
	        _listeners.remove( l );
	    }
	    
	    public synchronized void _firePointsEvent(int id, double yval ) 
	    {
	        PointsEvent event = new PointsEvent( this, id, yval);
	        callHandler(event);
	    }
	    
	    public synchronized void _fireFunctionChangeEvent(int msgId, int funcId, String message ) 
	    {
	        FunctionChangeEvent event = new FunctionChangeEvent( this, msgId, funcId, message);
	        callHandler(event);
	    }
	    
	    public synchronized void _fireWinnerDecidedEvent(int msgId, int requestId, int funcId, String message ) 
	    {
	        WinnerDecidedEvent event = new WinnerDecidedEvent( this, msgId, requestId, funcId, message);
	        callHandler(event);
	    }
	    
	    public synchronized void _fireMessageReceivedEvent(int msgId, String message ) 
	    {
	        MessageReceivedEvent event = new MessageReceivedEvent( this, msgId, message);
	        callHandler(event);
	    }
	    
		private void callHandler(EventObject event) {
			Iterator listeners = _listeners.iterator();
	        while( listeners.hasNext() ) 
	        {
	            ( (CustomEventListener) listeners.next() ).recvReceived( event );
	        }
		}
	     
	    private synchronized void _fireRecvEvent() 
	    {
	        RecvEvent event = new RecvEvent( this, recvdata, _head);
	        Iterator listeners = _listeners.iterator();
	        while( listeners.hasNext() ) 
	        {
	            ( (CustomEventListener) listeners.next() ).recvReceived( event );
	        }
	    }
}
