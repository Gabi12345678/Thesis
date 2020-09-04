package master;

import master.NumpyDataPointGroup;
import ch.qos.logback.classic.Logger;
import org.slf4j.LoggerFactory;
import org.kairosdb.plugin.*;
import org.kairosdb.core.datastore.*;
import org.kairosdb.core.datapoints.*;
import org.kairosdb.core.*;
import org.kairosdb.core.annotation.*;
import jep.*;
import java.util.*;
import java.lang.*;
import java.io.*;
import com.google.inject.name.Named;
import javax.inject.Inject;

@FeatureComponent(
	name = "hotsax",
	description = "UDF for Hot Sax."
)
public class HotSaxAggregator implements Aggregator {
	public static final Logger logger = (Logger) LoggerFactory.getLogger(HotSaxAggregator.class);

	private String pythonHotSaxImplementation;
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
	public HotSaxAggregator(@Named("kairosdb.udf.sax.implementation") String pythonHotSaxImplementation,
				DoubleDataPointFactory dataPointFactory) {
		this.pythonHotSaxImplementation = pythonHotSaxImplementation;
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
		try {
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

			float[] data = new float[lines * columns];
			for (int i = 0; i < dataPointList.size(); ++i) {
                                DataPoint dp = dataPointList.get(i);
 				String[] dimTags = dp.getDataPointGroup().getTagValues("dim").toArray(new String[0]);
				Integer indexLine = indexLines.get(dp.getTimestamp());
				Integer indexColumn = Integer.parseInt(dimTags[0].substring(3));
				data[indexLine * columns + indexColumn] = (float) dp.getDoubleValue();

			}
			NDArray<float[]> numpyArray = new NDArray<float[]>(data, lines, columns);

			SharedInterpreter interp = new SharedInterpreter();
			interp.exec("import sys");
			interp.exec("sys.path.append('" + pythonHotSaxImplementation + "')");
			interp.exec("import hotsax");
			interp.exec("import numpy as np");
			interp.set("data", numpyArray);
			interp.exec("discords = hotsax.hotsax(data)");
			interp.exec("discords = np.array(discords).astype(np.float32)");
			NDArray<float[]> discords = interp.getValue("discords", numpyArray.getClass());
			interp.close();
			logger.info("Applied HotSax");

			cleanData();

			return new NumpyDataPointGroup(this.dataPointFactory, dataPointGroup.getName() + ".result", discords, timestamps);
		} catch (JepException e){
			StringWriter w = new StringWriter();
			e.printStackTrace(new PrintWriter(w));
			logger.info(w.toString());
		}
		cleanData();
		return dataPointGroup;
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

