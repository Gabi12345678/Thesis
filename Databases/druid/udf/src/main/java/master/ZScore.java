package master;

import java.lang.Math;

public class ZScore {
	public static float[][] zScore(float[][] datapoints){
		int columns = datapoints[0].length;
		int lines = datapoints.length;
		float[][] result = new float[lines][columns];
		for (int j = 0; j < columns; j++) {
			float avg = (float) 0.0;
			for (int i = 0; i < lines; i++) {
				avg = avg + datapoints[i][j];
			}
			avg = avg / lines;
			float stdev = (float) 0.0;
			for (int i = 0; i < lines; i++) {
				stdev = stdev + (datapoints[i][j] - avg) * (datapoints[i][j] - avg);
			}
			stdev = (float) Math.sqrt(stdev / lines);
			for (int i = 0; i < lines; i++) {
				result[i][j] = (datapoints[i][j] - avg) / stdev;
			}
		}
		return result;
	}
    
};
