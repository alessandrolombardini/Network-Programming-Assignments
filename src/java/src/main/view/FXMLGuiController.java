package main.view;

import java.util.Arrays;
import java.util.List;
import java.util.NoSuchElementException;
import java.util.concurrent.Semaphore;

import javafx.application.Platform;
import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;
import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.fxml.FXML;
import javafx.scene.control.Button;
import javafx.scene.control.ChoiceDialog;
import javafx.scene.control.Spinner;
import javafx.scene.control.SpinnerValueFactory;
import javafx.scene.control.TextArea;
import main.model.Speed;

public class FXMLGuiController {
		
	    @FXML 
	    private TextArea console;
	    
	    @FXML
	    private Button sinistra;

	    @FXML
	    private Button destra;
	    
	    @FXML
	    private Button start;
	    
	    @FXML
	    private Spinner<Speed> spinner;
	    
	    @FXML
	    private Button AUTOButton;
	    
	    @FXML
	    private Button SINGLEButton;
	    
	    @FXML
	    private Button MANUALButton;
	    
		private final View view;
		private boolean dontSendMessage = false;
		private ChoiceDialog<String> requestDialog;
	    
	    public FXMLGuiController(final View view) {
			this.view = view;
		}
	    
	    public void initialize() {
	    	destra.setText("----->");
	    	sinistra.setText("<-----");
	    	console.setEditable(false);
	    	console.setWrapText(true);
	    	/* Insert values in the spinner element and set a listener on it */
	    	List<Speed> speed = Arrays.asList(Speed.values());
	    	ObservableList<Speed> spinner_values = FXCollections.observableArrayList(speed);
	    	SpinnerValueFactory<Speed> valueFactory = new SpinnerValueFactory.ListSpinnerValueFactory<Speed>(spinner_values);
	    	spinner.setValueFactory(valueFactory);
	    	spinner.valueProperty().addListener(new ChangeListener<Speed>() {
	            @Override
	            public void changed(ObservableValue<? extends Speed> observable, Speed oldValue, Speed newValue) { 
	            	if (dontSendMessage) {
	            		dontSendMessage = false;
	            	} else {
	            		view.spinnerValueChanged(newValue.getName());
	            	}
	            }
	        });
	    	requestDialog = new ChoiceDialog<String>();
	    }   
	    
	    public String askUserAnswer(final String[] values) {
	    	final StringProperty result = new SimpleStringProperty();
	    	Semaphore semaphore = new Semaphore (0);
	    	Platform.runLater(()-> {
	    		List<String> dialogData = Arrays.asList(values);
	    		requestDialog.getItems().addAll(dialogData);
	    		requestDialog.setTitle("Question");
	    		requestDialog.setHeaderText("There are multiple ports awaiable.");
	    		requestDialog.setContentText("Select wich one to use:");
	    		try{
	    			result.set(requestDialog.showAndWait().get());
	    		} catch (NoSuchElementException e) {
	    			result.set(null);
	    		}
	    		semaphore.release();
	    	});
	    	try {
	    		semaphore.acquire();
	    	}catch (Exception e) {
	    		System.out.println("Problem with semaphore.");
	    	}
	    	return result.get();
	    }
	    
	    public void appendOnTextArea(final String arg) {
	    	Platform.runLater(() -> console.appendText(arg + "\n")); // This call does auto-scroll the TextArea
	    	//Platform.runLater(() -> console.setText(console.getText() + arg + "\n")); // This call doesn't auto-scroll the TextArea
	    }
	    
	    public void changeSpinnerSelection(final int velocity) {
	    	dontSendMessage = true;
	    	Platform.runLater(() -> spinner.getValueFactory().setValue(Speed.getSpeedFromValue(velocity)));
	    }
	    
	    /* Methods usefull to handle events of elements of the scene */
	    
	    @FXML
	    public void handleAutoButton() {
	    	Platform.runLater(() -> view.autoButtonPressed());
	    }
	    
	    @FXML
	    public void handleSingleButton() {
	    	Platform.runLater(() -> view.singleButtonPressed());
	    }
	    
	    @FXML
	    public void handleManualButton() {
	    	Platform.runLater(() -> view.manualButtonPressed());
	    }
	    
	    @FXML
	    public void handleLeftButton() {
	    	Platform.runLater(() -> view.leftButtonPressed());
	    }
	    
	    @FXML
	    public void handleRightButton() {
	    	Platform.runLater(() -> view.rightButtonPressed());
	    }
	    
    	public void disableMovementsButtons() {
    		Platform.runLater(() -> { this.destra.setDisable(true);
    								  this.sinistra.setDisable(true); });
    	}
    	
    	public void enableMovementsButtons() {
    		Platform.runLater(() -> { this.destra.setDisable(false);
    								  this.sinistra.setDisable(false); });
    	}
    	
    	public void enableAutoButton() {
    		Platform.runLater(() ->this.AUTOButton.setDisable(false));
    	}
    	
    	public void disableAutoButton() {
    		Platform.runLater(() -> this.AUTOButton.setDisable(true));
    	}
    	
    	public void enableSingleButton() {
    		Platform.runLater(() -> this.SINGLEButton.setDisable(false));
    	}
    	
    	public void disableSingleButton() {
    		Platform.runLater(() -> this.SINGLEButton.setDisable(true));
    	}
    	
    	public void enableManualButton() {
    		Platform.runLater(() ->this.MANUALButton.setDisable(false));
    	}
    	
    	public void disableManualButton() {
    		Platform.runLater(() -> this.MANUALButton.setDisable(true));
    	}
    	
    	public void enableSpeed() {
    		Platform.runLater(() -> this.spinner.setDisable(false));
    	}
    	
    	public void disableSpeed() {
    		Platform.runLater(() -> this.spinner.setDisable(true));
    	}
}
