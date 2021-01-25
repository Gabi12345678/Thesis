package master;

import com.fasterxml.jackson.annotation.*;
import org.apache.commons.math3.linear.*;
import java.util.*;
import java.util.stream.*;
import cn.edu.thu.screen.Screen;
import cn.edu.thu.screen.entity.*;

public class ScreenAggregatorFactory extends MultiColumnAggregatorFactory {
	@JsonCreator
	public ScreenAggregatorFactory(
		@JsonProperty("name") String name,
		@JsonProperty("timeField") String timeField,
		@JsonProperty("columns") List<String> columns) {
		super(name, timeField, columns);
	}

	@Override
	public Object applyUdf(List<Long> timestamps, List<List<Double>> data) {
		int lines = timestamps.size();
		if(lines == 0){
			return "{}";
		}
		int columns = data.get(0).size();
		double[][] input = new double[lines][columns];
		for(int i = 0; i < lines; i++){
			for(int j = 0; j < columns; j++){
				input[i][j] = data.get(i).get(j);
			}
		}

		double[][] output = new double[lines][columns];
		for (int j = 0; j < columns; ++j) {
			ArrayList<TimePoint> timePoints = new ArrayList<TimePoint>();
			for (int i = 0; i < lines; ++i) {
				timePoints.add( new TimePoint(timestamps.get(i), input[i][j]));
			}
			Screen screenAlg = new Screen(new TimeSeries(timePoints), 0.0001, -0.0001, 300000);
			ArrayList<TimePoint> ts = screenAlg.mainScreen().getTimeseries();
			for(int i = 0; i < lines; ++i) {
                        	TimePoint p = ts.get(i);
                                if (p.isModified()) {
                                	output[i][j] = p.getModify();
                                } else {
                                	output[i][j] = p.getValue();
                                }
                       }
		}

                StringBuilder sb = new StringBuilder();
                sb.append("("); sb.append(lines); sb.append(", "); sb.append(columns); sb.append(") ");
                sb.append(Arrays.deepToString(output));

                return sb.toString();
	}

	@Override
	public byte udfCacheKey() { return (byte) 0x52; } 
}
