package master;

import com.fasterxml.jackson.annotation.*;
import org.apache.commons.math3.linear.*;
import java.util.*;
import java.util.stream.*;

public class RecovAggregatorFactory extends MultiColumnAggregatorFactory {
	@JsonCreator
	public RecovAggregatorFactory(
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

		RealMatrix recovrez = Recovdb.recovery( new Array2DRowRealMatrix(input), 3, 0.2, 10);
		double[][] r = recovrez.getData();

		StringBuilder sb = new StringBuilder();
                sb.append("("); sb.append(lines); sb.append(", "); sb.append(columns); sb.append(") ");
                sb.append(Arrays.deepToString(r));
		return sb.toString();
	}

	@Override
	public byte udfCacheKey() { return (byte) 0x52; } 
}
