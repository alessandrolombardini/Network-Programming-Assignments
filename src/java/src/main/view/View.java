package main.view;

import java.net.URL;
import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Stage;
import main.controller.Controller;

public class View extends Application {

	private FXMLGuiController viewController ;
	private Controller controller;
	
	@Override
	public void start(Stage stage) throws Exception {

		/* Load scene and its controller */
		final FXMLLoader loader = new FXMLLoader();        
		final URL url = getClass().getClassLoader().getResource("main/res/fxml_gui.fxml");
        loader.setLocation(url);
        viewController = new FXMLGuiController(this);
        loader.setController(viewController);
		Parent root = loader.load();
        Scene scene = new Scene(root, 800, 600);
        /* Set values of stage */
        stage.setScene(scene);
        stage.setMinHeight(600);
		stage.setMinWidth(800);
		stage.setTitle("Java Serial App");
        stage.show();
        
        /*Start thread to manage the controller */
		new Thread(() -> {
        	controller = new Controller(this);
        }).start();
        
	}
	
	public void writeOut(final String arg) {
		viewController.appendOnTextArea(arg);
	}
	
	/* Methods usefull to handle events of elements of the scene */
	
    public void changeVelocitySelected(final int velocity) {
    	viewController.changeSpinnerSelection(velocity);
    }
    
    public void enabledManualMode() {
    	viewController.enableMovementsButtons();
    	viewController.enableAutoButton();
    	viewController.enableSingleButton();
    	viewController.disableManualButton();
    	viewController.enableSpeed();
    }
    
    public void enabledSingleMode() {
    	viewController.disableMovementsButtons();
    	viewController.enableAutoButton();
    	viewController.disableSingleButton();
    	viewController.enableManualButton();
    	viewController.enableSpeed();
    }
    
    public void enabledAutoMode() {
    	viewController.disableMovementsButtons();
    	viewController.disableAutoButton();
    	viewController.enableSingleButton();
    	viewController.enableManualButton();
    	viewController.enableSpeed();
    }
	    
	public void spinnerValueChanged(final String newValue) {
		controller.speedValueChanged(newValue);
	}
	
	public void rightButtonPressed() {
		controller.increaseServoPosition();
	}
	
	public void leftButtonPressed() {
		controller.decreaseServoPosition();
	}
	
	public void autoButtonPressed() {
		controller.changeModAuto();
	}
	
	public void manualButtonPressed() {
		controller.changeModManual();
	}
	
	public void singleButtonPressed() {
		controller.changeModSingle();
	}
	
	public String askUserValidation(final String[] values) {
		String result = viewController.askUserAnswer(values);
		return result;
		
	}
	
	public void disableSpeedMovements(){
		viewController.disableSpeed();
		viewController.disableMovementsButtons();
	}
	
	public void enableSpeed(){
		viewController.enableSpeed();
	}
	
	public void enableMovements() {
		viewController.enableMovementsButtons();
	}
	
	public void disableGUI(){
		viewController.disableSpeed();
		viewController.disableSingleButton();
		viewController.disableManualButton();
		viewController.disableAutoButton();
		viewController.disableMovementsButtons();
	}
}
