package main.model;

import jssc.SerialPortException;
import main.controller.Controller;

public class ChannelWrapper {
	
	private CommChannel channel;
	private Controller controller;

	public ChannelWrapper(final Controller controller) {
		/* Find valid serial port */
		this.controller = controller;
	} 
	
	public void init (String port) {
		boolean validPortFounded = false;
		channel = new SerialCommChannel(port,9600);
		try {
			channel.init();
			validPortFounded = true;
			//controller.printOnConsole("Use the serial port " + portNames[i]);		
		} catch (SerialPortException ex) {
			/* Invalid port */
			controller.printOnConsole("The Serial Port choosen (" + port + ") is not answering.");
		}
		
		/* If a valid port is founded */
		if (validPortFounded) {
			try {
				/* Attesa necessaria per fare in modo che Arduino completi il reboot */
				controller.printOnConsole("Waiting Arduino for rebooting...");		
				Thread.sleep(4000);
				controller.printOnConsole("Ready.");		
			} catch (Exception ex2) {
				ex2.printStackTrace();
			}
					
			/* Avvio un thread dedicato alla stampa dei messaggi provenienti dalla porta */
			new Thread(() -> {
				while (true){
					try {
						if (channel.isMsgAvailable()) {
							final String msg = channel.receiveMsg();
							controller.evaluateMessageReceived(msg);
							controller.printOnConsole("[Received] "+ msg);	
						}
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}).start();
		}
	}
	
	public CommChannel getChannel() {
		return this.channel;
	}

}
