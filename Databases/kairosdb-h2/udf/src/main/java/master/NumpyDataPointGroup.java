package master;

import ch.qos.logback.classic.Logger;
import org.slf4j.LoggerFactory;
import jep.*;
import org.kairosdb.core.*;
import org.kairosdb.core.datastore.*;
import org.kairosdb.core.datapoints.*;
import org.kairosdb.core.groupby.*;
import java.util.*;
import java.lang.*;


public class NumpyDataPointGroup implements DataPointGroup {
	public static final Logger logger = (Logger) LoggerFactory.getLogger(KMeansAggregator.class);

	private String name;
	private ArrayList<Long> timestamps;
	private int lines, columns, indexIterator;
	private float[] data;
	private Set<String> dimTags;
	private DoubleDataPointFactory dataPointFactory;
	

	public NumpyDataPointGroup(DoubleDataPointFactory dataPointFactory, String name, NDArray<float[]> data, ArrayList<Long> timestamps) {
		this.lines = data.getDimensions()[0];
		this.columns = data.getDimensions()[1];
		this.data = data.getData();
		this.indexIterator = 0;
		this.timestamps = timestamps;
		this.name = name;
		this.dataPointFactory = dataPointFactory;
		this.dimTags = new HashSet<String> ();
		for (int i = 0; i < this.columns; ++i) {
			dimTags.add("dim" + Integer.toString(i));
		}
		while (this.timestamps.size() < this.lines) {
			int s = this.timestamps.size();
			this.timestamps.add(this.timestamps.get(s - 1) * 2 - this.timestamps.get(s - 2));
		}
	}

	@Override
	public String getName() {
		return this.name;
	}

	@Override
	public List<GroupByResult> getGroupByResult() {
		return new ArrayList<GroupByResult>();
	}

	@Override
	public void close() {}

	@Override
	public Set<String> getTagNames() {
		return Collections.singleton("dim");
	}

	@Override
	public Set<String> getTagValues(String tag) {
		if (tag.equals("dim")) {
			return dimTags;
		}
		return Collections.emptySet();
	}

	@Override
	public boolean hasNext() {
		return indexIterator != data.length;
	}

	@Override
	public DataPoint next() {
		DataPoint result = dataPointFactory.createDataPoint(timestamps.get(indexIterator / columns), data[indexIterator]);
		result.setDataPointGroup(new TagOnlyDataPointGroup(this.name, "dim" + Integer.toString(indexIterator % columns)));
		indexIterator++;
		return result;
	}

	private class TagOnlyDataPointGroup implements DataPointGroup {
		private String name, dimTag;

		public TagOnlyDataPointGroup(String name, String dimTag) {
			this.name = name;
			this.dimTag = dimTag;
		}

		@Override
		public String getName() {
			return this.name;
		}

		@Override
		public List<GroupByResult> getGroupByResult() {
			return new ArrayList<GroupByResult>();
		}

		@Override
		public void close() {}
		
		@Override
		public Set<String> getTagNames() {
			return Collections.singleton("dim");
		}

		@Override
		public Set<String> getTagValues(String tag) {
			if (tag.equals("dim")) {
				return Collections.singleton(this.dimTag);
			}
			return Collections.emptySet();
		}

		@Override
		public boolean hasNext() {
			return false;
		}

		@Override 
		public DataPoint next() {
			return null;
		}
	}
}
