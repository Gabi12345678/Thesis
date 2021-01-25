package master;

import org.apache.druid.segment.*;
import org.apache.druid.query.aggregation.*;

import java.util.*;
import java.sql.*;

public class MultiColumnAggregator implements Aggregator {
	public static Comparator COMPARATOR = new Comparator<Map<Long, List<Double>>>() {
		@Override
		public int compare(Map<Long, List<Double>> lhs, Map<Long, List<Double>> rhs) {
			List<Long> lhsKeys = new ArrayList<Long>(lhs.keySet());
			List<Long> rhsKeys = new ArrayList<Long>(rhs.keySet());
			if (lhsKeys.size() < rhsKeys.size()) return -1;
			if (lhsKeys.size() > rhsKeys.size()) return 1;
			Collections.sort(lhsKeys);
			Collections.sort(rhsKeys);
			for (int i = 0; i < lhsKeys.size(); ++i) {
				if (lhsKeys.get(i) < rhsKeys.get(i)) return -1;
				if (lhsKeys.get(i) > rhsKeys.get(i)) return 1;
			}
			return 0;
		}
	};

	public static Map<Long, List<Double>> combine(Map<Long, List<Double>> lhs, Map<Long, List<Double>> rhs) {
		Map<Long, List<Double>> result = new HashMap<Long, List<Double>>();
		for (Map.Entry<Long, List<Double>> entry : lhs.entrySet()) {
			result.putIfAbsent( entry.getKey(), entry.getValue() );
		}
		for (Map.Entry<Long, List<Double>> entry : rhs.entrySet()) {
			result.putIfAbsent( entry.getKey(), entry.getValue());
		}
		return result;
	}

	private Map<Long, List<Double>> values;
	private BaseObjectColumnValueSelector timestampSelector;
	private List<BaseDoubleColumnValueSelector> columnSelectors;

	public MultiColumnAggregator(
		BaseObjectColumnValueSelector timestampSelector,
		List<BaseDoubleColumnValueSelector> columnSelectors) {
		this.values = new HashMap<Long, List<Double>>();
		this.timestampSelector = timestampSelector;
		this.columnSelectors = columnSelectors;
	}

	@Override
	public void aggregate() {
		Long timestamp = (long) 0;
		if (timestampSelector != null) {
			timestamp = convertTimestamp( timestampSelector.getObject() );
		}
		List<Double> columnValues = new ArrayList<Double>();
		for (BaseDoubleColumnValueSelector selector : columnSelectors) {
			columnValues.add( selector.getDouble() );
		}
		values.put(timestamp, columnValues);		
	}

	@Override
	public Object get() {
		return values;
	}

	@Override
	public float getFloat() {
		return (float) 0;
	}

	@Override
	public double getDouble() {
		return (double) 0;
	}

	@Override
	public long getLong() {
		return (long) 0;
	}

	@Override
	public void close() {}

	public static Long convertTimestamp(Object input) {
		if (input instanceof Number) {
			return ((Number) input).longValue();
		} else if (input instanceof Timestamp) {
			return ((Timestamp) input).getTime();
		}
		return null;
	}
}

