package master;

import com.google.common.base.Preconditions;
import com.fasterxml.jackson.annotation.*;

import java.util.*;
import java.util.stream.*;

public class StringAggregatorFactory extends MultiColumnAggregatorFactory {
	@JsonCreator
	public StringAggregatorFactory(
		@JsonProperty("name") String name,
		@JsonProperty("timeField") String timeField,
		@JsonProperty("columns") List<String> columns) {
		super(name, timeField, columns);
		Preconditions.checkNotNull(name, "Expected non-null name");
		Preconditions.checkNotNull(timeField, "Expected non-null timeField");
		Preconditions.checkNotNull(columns, "Expected non-null columns");
		Preconditions.checkState(columns.size() > 0, "Expected non-empty columns");
	}

	@Override
	public Object applyUdf(List<Long> timestamps, List<List<Double>> data) {
		String result = "{ ";
		for (int i = 0; i < timestamps.size(); ++i) {
			result = result + timestamps.get(i) + ": [";
			result = result + String.join(", ", data.get(i).stream().map( v -> Double.toString(v) ).collect(Collectors.toList()));
			result = result + "], ";
		}
		result = result + "}";
		return result; 
	}

	@Override
	public byte udfCacheKey() { return (byte) 0x50; } 
}
