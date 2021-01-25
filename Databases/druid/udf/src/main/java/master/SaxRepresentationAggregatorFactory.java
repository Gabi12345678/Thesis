package master;

import com.fasterxml.jackson.annotation.*;

import java.util.*;
import java.util.stream.*;

public class SaxRepresentationAggregatorFactory extends MultiColumnAggregatorFactory {
	@JsonCreator
	public SaxRepresentationAggregatorFactory(
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

		List<String> sax = SaxTransformation.saxrepresentation(input);
		return "[" + String.join(", ", sax) + "]";
	}

	@Override
	public byte udfCacheKey() { return (byte) 0x57; } 
}
