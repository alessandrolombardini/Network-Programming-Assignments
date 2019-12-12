package main.model;

public enum Speed {
	Speed_1("Speed: 1", 1), 
	Speed_2("Speed: 2", 2),
	Speed_3("Speed: 3", 3),
	Speed_4("Speed: 4", 4),
	Speed_5("Speed: 5", 5),
	Speed_6("Speed: 6", 6),
	Speed_7("Speed: 7", 7),
	Speed_8("Speed: 8", 8);
	
	private String name;
	private int value;
	private Speed(final String name, final int value) {
		this.name = name;
		this.value = value;
	}
	
	public String getName() {
		return this.name;
	}
	
	public int getValue() {
		return this.value;
	}
	
	public static Speed getSpeedFromValue(final int value) {
		Speed ret = null;
		switch (value) {
			case 1: 
				ret = Speed_1;
				break;
			case 2: 
				ret = Speed_2;
				break;
			case 3: 
				ret = Speed_3;
				break;
			case 4: 
				ret = Speed_4;
				break;
			case 5: 
				ret = Speed_5;
				break;
			case 6: 
				ret = Speed_6;
				break;
			case 7: 
				ret = Speed_7;
				break;
			case 8: 
				ret = Speed_8;
				break;
		}
		return ret;
	}
}
