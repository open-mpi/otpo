package eventpackage;

import java.util.EventObject;

import communicationmanager.Header;


@SuppressWarnings("serial")
public class RecvEvent extends EventObject {
    private byte[] _data;
    Header _head;    
    
    public RecvEvent( Object source, byte[] data, Header head) 
    {
        super( source );
        _head = head;
        _data = data;
    }
    
    public Header get_header() {
		return _head;
	}

	public byte[] getRecvData(){return _data;}
    
}