package master;

import com.fasterxml.jackson.annotation.*;
import org.apache.druid.segment.*;
import org.apache.druid.query.aggregation.*;
import org.apache.druid.java.util.common.*;
import org.apache.druid.segment.column.*;

import java.util.*;
import java.nio.*;
import javax.annotation.*;
import java.util.stream.*;

public abstract class MultiColumnAggregatorFactory extends AggregatorFactory {
	private String name;
	private String timestampFieldName;
	private List<String> columnFieldNames;

	public MultiColumnAggregatorFactory(
		String name,
		String timestampFieldName,
		List<String> columnFieldNames
	) {
		this.name = name;
		this.timestampFieldName = timestampFieldName;
		this.columnFieldNames = columnFieldNames;
	}

	@Override
	public Aggregator factorize(ColumnSelectorFactory metricFactory) {
		BaseObjectColumnValueSelector timestampSelector = null;
		if (timestampFieldName != null) {
			timestampSelector = metricFactory.makeColumnValueSelector( timestampFieldName );
		}
		List<BaseDoubleColumnValueSelector> columnSelectors = new ArrayList<BaseDoubleColumnValueSelector>();
		for (String fieldName : columnFieldNames) {
			columnSelectors.add( metricFactory.makeColumnValueSelector( fieldName) );
		}
		return new MultiColumnAggregator( timestampSelector, columnSelectors );
	}

	@Override
	public BufferAggregator factorizeBuffered(ColumnSelectorFactory metricFactory) {
		return null;
	}

	@Override
	public Comparator getComparator() {
		return MultiColumnAggregator.COMPARATOR;
	}


	@Override
	public Object combine(@Nullable Object lhs, @Nullable Object rhs) {
		Map<Long, List<Double>> l = Collections.emptyMap();
		if (lhs != null) {
			l = (Map<Long, List<Double>>) lhs;
		}
		Map<Long, List<Double>> r = Collections.emptyMap();
		if (rhs != null) {
			r = (Map<Long, List<Double>>) rhs;
		}
		return MultiColumnAggregator.combine( l, r );
	}

	@Override
	public AggregatorFactory getCombiningFactory() {
		return this;
	}

	// Omit implementation since this should not be used in our types of queries.
	@Override
	public List<AggregatorFactory> getRequiredColumns() {
		return Collections.emptyList();
	}
	@Override
	public Object deserialize(Object object) {
		return object;
	}

	@Override
	public String getName() {
		return name;
	}

	@Override
	public List<String> requiredFields() {
		List<String> result = new ArrayList<String>( columnFieldNames );
		if (timestampFieldName != null) {
			result.add(timestampFieldName);
		}
		return result;
	}

	@Override
	public int getMaxIntermediateSize() {
		return Long.BYTES * 10000000;
	}

	// Omit
	@Override
	public String getTypeName() {
		return "tsmap";
	}

	@Nullable
	@Override
	public Object finalizeComputation(Object object) {
		if (object == null) return null;
		Map<Long, List<Double>> timeseries = (Map<Long, List<Double>>) object;
		List<Long> timestamps = new ArrayList<Long>( timeseries.keySet() );
		Collections.sort(timestamps);
		List<List<Double>> data = new ArrayList<List<Double>>();
		for (Long time : timestamps) {
			data.add( timeseries.get(time) );
		}
		return applyUdf(timestamps, data);
	}

	@Override
	public byte[] getCacheKey() {
		int total = 1;
		for (String fieldName : columnFieldNames) {
			total += StringUtils.toUtf8(fieldName).length;
		}
		ByteBuffer bytes = ByteBuffer.allocate(total);
		bytes = bytes.put( udfCacheKey() );
		for (String fieldName : columnFieldNames) {
			bytes = bytes.put(StringUtils.toUtf8(fieldName));
		}
		return bytes.array();
	}

	public abstract Object applyUdf(List<Long> timestamps, List<List<Double>> data);

	public abstract byte udfCacheKey();
}
