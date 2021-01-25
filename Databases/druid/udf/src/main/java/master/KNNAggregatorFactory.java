package master;

import com.fasterxml.jackson.annotation.*;

import java.util.*;
import java.util.stream.*;

public class KNNAggregatorFactory extends MultiColumnAggregatorFactory {
	@JsonCreator
	public KNNAggregatorFactory(
		@JsonProperty("name") String name,
		@JsonProperty("timeField") String timeField,
		@JsonProperty("unlabelColumns") List<String> unlabelColumns,
		@JsonProperty("label") String label,
		@JsonProperty("labelColumns") List<String> labelColumns) {
		super(name, timeField, merge(unlabelColumns, labelColumns, label) );
	}

	private static List<String> merge(List<String> unlabelColumns, List<String> labelColumns, String label) {
		List<String> columns = new ArrayList<String>(unlabelColumns);
		columns.addAll(labelColumns);
		columns.add(label);
		return columns;
	}

	@Override
	public Object applyUdf(List<Long> timestamps, List<List<Double>> data) {
		int lines = timestamps.size();
		if(lines == 0){
			return "{}";
		}

		int columns = data.get(0).size() / 2;

		List<List<Double>> unlabelMatrix = new ArrayList<List<Double>>();
		List<List<Double>> labelMatrix = new ArrayList<List<Double>>();
		List<Integer> labels = new ArrayList<Integer>(); 
		for (int i = 0; i < lines; ++i) {
			unlabelMatrix.add( data.get(i).subList(0, columns) );
			labelMatrix.add( data.get(i).subList(columns, 2 * columns));
			labels.add( data.get(i).get(2 * columns).intValue() );
		}

		List<Integer> knn = KNN.knn(labelMatrix, labels, unlabelMatrix, 3);

		StringBuilder sb = new StringBuilder();
		sb.append("("); sb.append(lines); sb.append(", "); sb.append(columns); sb.append(") ");
		sb.append( knn.stream().map( n -> String.valueOf(n) ).collect(Collectors.joining(", ", "[", "]")) );
		return sb.toString(); 
	}

	@Override
	public byte udfCacheKey() { return (byte) 0x5E; } 
}
