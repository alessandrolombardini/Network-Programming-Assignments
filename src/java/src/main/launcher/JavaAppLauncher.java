package main.launcher;

import javafx.application.Application;
import main.view.View;

public class JavaAppLauncher {

	private JavaAppLauncher() {}
	
	/* Starting point of the application */
	public static void main(String[] args) throws Exception {
		Application.launch(View.class, args);
	}

}
