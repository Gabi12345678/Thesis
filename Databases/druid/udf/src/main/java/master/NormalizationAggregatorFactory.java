package master;

import com.fasterxml.jackson.annotation.*;

import java.util.*;
import java.util.stream.*;
import java.lang.*;

public class NormalizationAggregatorFactory extends MultiColumnAggregatorFactory {
	@JsonCreator
	public NormalizationAggregatorFactory(
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
                float[][] input = new float[lines][columns];
                for(int i = 0; i < lines; i++){
                        for(int j = 0; j < columns; j++){
                                input[i][j] =(float)((double)data.get(i).get(j));
                        }
                }

                float[][] norm = ZScore.zScore(input);

		StringBuilder sb = new StringBuilder();
		sb.append("("); sb.append(lines); sb.append(", "); sb.append(columns); sb.append(") ");
		sb.append(Arrays.deepToString(norm));

                return sb.toString();
	}

	@Override
	public byte udfCacheKey() { return (byte) 0x53; } 
}
