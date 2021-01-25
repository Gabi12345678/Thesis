package master;

import ch.qos.logback.classic.Logger;
import org.slf4j.LoggerFactory;
import org.kairosdb.plugin.*;
import org.kairosdb.core.datastore.*;
import org.kairosdb.core.datapoints.*;
import org.kairosdb.core.*;
import org.kairosdb.core.annotation.*;
import java.util.*;
import java.lang.*;
import java.io.*;
import com.google.inject.name.Named;
import javax.inject.Inject;

@FeatureComponent(
        name = "knn",
	description = "UDF for KNN."
)
public class KNNAggregator implements Aggregator {
	public static final Logger logger = (Logger) LoggerFactory.getLogger(KNNAggregator.class);

	private DoubleDataPointFactory dataPointFactory;
	private ArrayList<DataPoint> dataPointList;
	private HashSet<Long> uniqueTimestamps;

	@FeatureProperty( 
		name = "lines",
		label = "Lines",
		description = "Number of datapoints for each of the measurements."
	)
	private int lines;

	@FeatureProperty(
		name = "columns",
		label = "Columns",
		description = "Number of columns / measurements."
	)
	private int columns;

	@Inject
	public KNNAggregator(DoubleDataPointFactory dataPointFactory) {
		this.dataPointFactory = dataPointFactory;
		this.dataPointList = new ArrayList<DataPoint>();
		this.uniqueTimestamps = new HashSet<Long>();
	}

	
	public void setLines(int lines) {
		this.lines = lines;
	}

	public void setColumns(int columns) {
		this.columns = columns;
	}

	@Override
	public DataPointGroup aggregate(DataPointGroup dataPointGroup) {
			while (dataPointGroup.hasNext()) {
				DataPoint currentDataPoint = dataPointGroup.next();
				dataPointList.add(currentDataPoint);
				uniqueTimestamps.add(currentDataPoint.getTimestamp());
			}

			if (lines * ( 2 * columns + 1 ) != dataPointList.size()) {
				logger.info("Insuficient data. Got " + dataPointList.size() + ", but expected " + (lines * columns) + ". Waiting for more.");
				return dataPointGroup;
			}
			logger.info("All expected data has been received.");

			ArrayList<Long> timestamps = new ArrayList<Long>(uniqueTimestamps);
			Collections.sort(timestamps);
			HashMap<Long, Integer> indexLines = new HashMap<Long, Integer>();
			int index = 0;
			for (Long t : timestamps) {
				indexLines.put(t, index);
				index++;
			}

			Double[][] data = new Double[lines][columns];
			Double[][] label_data = new Double[lines][columns];
			Integer[] labels = new Integer[lines];
			for (int i = 0; i < dataPointList.size(); ++i) {	
				DataPoint dp = dataPointList.get(i);
				String[] dimTags = dp.getDataPointGroup().getTagValues("dim").toArray(new String[0]);
				Integer indexLine = indexLines.get(dp.getTimestamp());
				if (dimTags[0].startsWith("dim")) {
					Integer indexColumn = Integer.parseInt(dimTags[0].substring(3));
					data[indexLine][indexColumn] = (double) dp.getDoubleValue();
				} else if (dimTags[0].equals("label")) {
					labels[indexLine] = (int) dp.getDoubleValue();
				} else {
					Integer indexColumn = Integer.parseInt(dimTags[0].substring(1));
					label_data[indexLine][indexColumn] = (double) dp.getDoubleValue();
				}
			}
			List<List<Double>> matrix = new ArrayList<List<Double>>();
			for (int i = 0; i < lines; ++i) {
				matrix.add( Arrays.asList(data[i]) );
			}
			List<List<Double>> label_matrix = new ArrayList<List<Double>>();
			for (int i = 0; i < lines; ++i) {
				label_matrix.add( Arrays.asList(label_data[i]) );
			}
			
			List<Integer> result = KNN.knn(label_matrix, Arrays.asList(labels), matrix, 3);
			double[][] result_data = new double[result.size()][1];
			for (int i = 0; i < result.size(); ++i)
				result_data[i][0] = result.get(i);

			logger.info("Applied KNN");

			cleanData();
			return new MatrixDataPointGroup(this.dataPointFactory, dataPointGroup.getName() + ".result", result_data, timestamps); 
	}

	@Override
	public boolean canAggregate(String groupType) {
		return true;
	}

	@Override
	public String getAggregatedGroupType(String groupType) {
		return groupType;
	}

	private void cleanData() {
		this.dataPointList = new ArrayList<DataPoint>();
		this.uniqueTimestamps = new HashSet<Long>();
	}
}
