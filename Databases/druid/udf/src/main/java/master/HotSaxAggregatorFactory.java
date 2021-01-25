package master;

import com.fasterxml.jackson.annotation.*;
import org.apache.commons.math3.linear.*;
import java.util.*;
import java.util.stream.*;

public class HotSaxAggregatorFactory extends MultiColumnAggregatorFactory {
	@JsonCreator
	public HotSaxAggregatorFactory(
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
		try {
		int columns = data.get(0).size();
		double[][] input = new double[lines][columns];
		for(int i = 0; i < lines; i++){
			for(int j = 0; j < columns; j++){
				input[i][j] = data.get(i).get(j);
			}
		}

		List<Discord> all_discords = new ArrayList<Discord>();
		for (int i = 0; i < columns; ++i) {
			List<Double> current_column = new ArrayList<Double>();
			for (int j = 0; j < lines; ++j) {
				current_column.add( input[j][i] );
			}
			List<Discord> current_discords = HotSax.find_discords_hotsax(current_column, 100, 2, 3, 3, 0.01);
			all_discords.addAll( current_discords );
		}
                StringBuilder sb = new StringBuilder();
                sb.append("("); sb.append(lines); sb.append(", "); sb.append(columns); sb.append(") [");
		for (int i = 0; i < all_discords.size(); ++i) {
			sb.append(all_discords.get(i).toString() + ", ");
		}
		sb.append("]");
	
		return sb.toString();
		} catch (java.lang.ArrayIndexOutOfBoundsException e) {
			java.io.StringWriter sw = new java.io.StringWriter();
			java.io.PrintWriter pw = new java.io.PrintWriter(sw);
			e.printStackTrace(pw);
			return sw.toString();
		}
	}

	@Override
	public byte udfCacheKey() { return (byte) 0x52; } 
}
