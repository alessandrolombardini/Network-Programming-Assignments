package main.controller;

import jssc.SerialPortList;
import main.model.ChannelWrapper;
import main.view.View;

public class Controller {
	
	private final View view;
	private ChannelWrapper seriale;
	private boolean isPowerDownMode = false;
	private boolean isManualEnabled = true;
	private boolean isArduinoReady = false;
	
	public Controller(final View view) {
		this.view = view;
		this.seriale = new ChannelWrapper(this);
		view.disableGUI(); /*Disable GUI until Arduino is ready*/
		final String[] portNames = SerialPortList.getPortNames(); /*Get names of active ports*/
		
		if (portNames.length == 0) { /*There isn't an active port in the system*/
			view.writeOut("There isn't an active port.");
		} else if (portNames.length == 1){ /*There is one active port in the system*/
			seriale.init(portNames[0]);
		} else { /*there are multiple active ports in the system -> ask user choice*/
			final String port = view.askUserValidation(portNames); /* Returns the only port name that we are going to use to connect the serial*/
			if (port == null) {
				view.writeOut("Selected port is not valid.");
			} else {
				seriale.init(port); /*Init of the model*/
			}
		}
		
	}
	
	public void printOnConsole(final String arg) {
		view.writeOut(arg);
	}
	
	public void evaluateMessageReceived(final String arg) {
		if(arg.length() >= 8) {
			if (arg.substring(0,7).equals("Speed: ")) {
				this.view.changeVelocitySelected(Integer.parseInt(arg.substring(7, 8)));
			} else if (arg.equals("Modality selected: SINGLE")) {
				if (isArduinoReady) {
					isManualEnabled = false;
					view.enabledSingleMode();
				}
				
			} else if (arg.equals("Modality selected: AUTO")) {
				if (isArduinoReady) {
					isManualEnabled = false;
					view.enabledAutoMode();
				}
				
			} else if (arg.equals("Modality selected: MANUAL")) {
				if (isArduinoReady) {
					isManualEnabled = true;
					view.enabledManualMode();
				}
				
			} else if (arg.equals("Going in power down mode...")) {
				isPowerDownMode = true;
				view.disableSpeedMovements();
			} else if (arg.equals("Wake up!")) {
				isPowerDownMode = false;
				view.enableSpeed();
				if (isManualEnabled)
					view.enableMovements();
			} else if (arg.equals("Pir ready")) {
				isArduinoReady = true;
				view.enabledManualMode();
			}
		}
	}
	
	/* Methods usefull to handle events of elements of the scene */
	
	public void speedValueChanged(final String newValue) {
		view.writeOut("[Sent] " + newValue);
		seriale.getChannel().sendMsg(newValue);
	}
	
	public void increaseServoPosition() {
		String message = "Servo: increase position";
		view.writeOut("[Sent] " + message);
		seriale.getChannel().sendMsg(message);
	}
	
	public void decreaseServoPosition() {
		String message = "Servo: decrease position";
		view.writeOut("[Sent] " + message);
		seriale.getChannel().sendMsg(message);
	}
	
	public void changeModAuto() {
		isManualEnabled = false;
		String message = "Modality selected: AUTO";
		view.writeOut("[Sent] " + message);
		view.enabledAutoMode();
		if (isPowerDownMode) {
			seriale.getChannel().sendMsg("!" + message);
		} else {
			seriale.getChannel().sendMsg(message);
		}
	}
	
	public void changeModSingle() {
		isManualEnabled = false;
		String message = "Modality selected: SINGLE";
		view.writeOut("[Sent] " + message);
		view.enabledSingleMode();
		if (isPowerDownMode) {
			seriale.getChannel().sendMsg("!" + message);
		} else {
			seriale.getChannel().sendMsg(message);
		}
	}
	
	public void changeModManual() {
		isManualEnabled = true;
		String message = "Modality selected: MANUAL";
		view.writeOut("[Sent] " + message);
		view.enabledManualMode();
		if (isPowerDownMode) {
			seriale.getChannel().sendMsg("!" + message);
		} else {
			seriale.getChannel().sendMsg(message);
		}
	}

}
