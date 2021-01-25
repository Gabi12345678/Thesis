package master;

import com.fasterxml.jackson.annotation.*;

import java.util.*;

class MultiSumAggregatorFactory extends MultiColumnAggregatorFactory {
	@JsonCreator
	public MultiSumAggregatorFactory(
		@JsonProperty("columns") List<String> columns,
		@JsonProperty("name") String outputColumnName) {
		super(outputColumnName, "__time", columns);
	}

	@Override
	public Object applyUdf(List<Long> timestamps, List<List<Double>> data) {
		Double total = 0.0;
		for (List<Double> line : data) {
			for (Double point : line) {
				total = total + point;
			}
		}
		return total;
	}

	@Override
	public byte udfCacheKey() { return 0x51; }
}
