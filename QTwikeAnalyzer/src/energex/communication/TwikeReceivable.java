package energex.communication;

import java.util.HashSet;
import java.util.Set;

public class TwikeReceivable {
	Set<TwikeReceiver> setReceiver = new HashSet<TwikeReceiver>();
	
	public void addReceiver(TwikeReceiver receiver) {
		setReceiver.add(receiver);
	}
	
	public void deleteReceiver(TwikeReceiver receiver) {
		setReceiver.remove(receiver);
	}
	
	public void notifyReceiver(byte data) {
		for(TwikeReceiver receiver : setReceiver) {
			receiver.receiveData(data);
		}
	}
	
	public void deleteReceivers() {
		setReceiver.clear();
	}
}
