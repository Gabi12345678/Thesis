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
import org.apache.commons.math3.linear.*;

@FeatureComponent(
        name = "recov",
	description = "UDF for CD"
)
public class RecovAggregator implements Aggregator {
	public static final Logger logger = (Logger) LoggerFactory.getLogger(RecovAggregator.class);

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
	public RecovAggregator(DoubleDataPointFactory dataPointFactory) {
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

			if (lines * columns != dataPointList.size()) {
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

			double[][] data = new double[lines][columns];
			for (int i = 0; i < dataPointList.size(); ++i) {	
				DataPoint dp = dataPointList.get(i);
				String[] dimTags = dp.getDataPointGroup().getTagValues("dim").toArray(new String[0]);
				Integer indexLine = indexLines.get(dp.getTimestamp());
				Integer indexColumn = Integer.parseInt(dimTags[0].substring(3));
				data[indexLine][indexColumn] = (double) dp.getDoubleValue();
			}
		 	
			RealMatrix input = new Array2DRowRealMatrix( data );
			RealMatrix result = Recovdb.recovery(input, 3, 0.2, 10);
			
			logger.info("Applied Recovdb");

			cleanData();

			return new MatrixDataPointGroup(this.dataPointFactory, dataPointGroup.getName() + ".result", result.getData(), timestamps); 
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
