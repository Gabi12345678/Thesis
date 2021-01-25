package master;

import org.apache.commons.lang3.time.StopWatch;
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
import cn.edu.fudan.cs.dstree.*;
import cn.edu.fudan.cs.dstree.dynamicsplit.*;
import cn.edu.fudan.cs.dstree.util.*;

@FeatureComponent(
        name = "dstreesearch",
	description = "UDF for Dstree exact search"
)
public class DstreeSearchAggregator implements Aggregator {
	public static final Logger logger = (Logger) LoggerFactory.getLogger(DstreeSearchAggregator.class);

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

	@FeatureProperty(
		name = "indexfile",
		label = "Indexfile",
		description = "Path of the index file"
	)
	private String indexfile;

	@Inject
	public DstreeSearchAggregator(DoubleDataPointFactory dataPointFactory) {
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

	public void setIndexfile(String indexfile) {
		this.indexfile = indexfile;
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

			double[][] data = new double[columns][lines];
			for (int i = 0; i < dataPointList.size(); ++i) {	
				DataPoint dp = dataPointList.get(i);
				String[] dimTags = dp.getDataPointGroup().getTagValues("dim").toArray(new String[0]);
				Integer indexLine = indexLines.get(dp.getTimestamp());
				Integer indexColumn = Integer.parseInt(dimTags[0].substring(3));
				data[indexColumn][indexLine] = (double) dp.getDoubleValue();
			}
		 	
			double[][] result = new double[0][0];
			try {
				result = search(data);
			} catch (IOException e) { 
				StringWriter sw = new StringWriter();
				PrintWriter pw = new PrintWriter(sw);
				e.printStackTrace(pw);
				logger.info("IOException: ----- " + sw.toString());
			} catch (ClassNotFoundException e) {
				logger.info("ClassNotFoundException");
			}

			logger.info("Applied Dstree Exact Search");

			cleanData();

			return new MatrixDataPointGroup(this.dataPointFactory, dataPointGroup.getName() + ".result", result, timestamps); 
	}

	double[][] search(double[][] data) throws IOException, ClassNotFoundException {
		File file = new File(this.indexfile);
		Node newRoot = null;
		logger.info("IndexPath = " + this.indexfile);
		if (file.exists()) {
			String indexFileName = this.indexfile + "\\" + "root.idx";
			logger.info("reading file " + indexFileName);
			newRoot = Node.loadFromFile(indexFileName);
			logger.info("root file: " + newRoot.getFileName());
		} else {
			logger.info("file does not exists");
		}
		int tsLength = lines;
		int totalTsCount = newRoot.getSize();
		int searchCount = columns;

	        IndexExactSearcher.totalTime.reset();
	        IndexExactSearcher.totalTime.start();
	        IndexExactSearcher.totalTime.suspend();
	        IndexExactSearcher.ioTime.reset();
	        IndexExactSearcher.ioTime.start();
	        IndexExactSearcher.ioTime.suspend();
	        IndexExactSearcher.approTime.reset();
	        IndexExactSearcher.approTime.start();
	        IndexExactSearcher.approTime.suspend();	

		double[][] output = new double[columns][1];
		for (int c = 0; c < columns; ++c) {
			double[] queryTs = data[c];
			PqItem result = IndexExactSearcher.exactSearch(queryTs, newRoot);
			output[c][0] = result.dist;
		}

	        IndexExactSearcher.totalTime.stop();
	        IndexExactSearcher.ioTime.stop();
	        IndexExactSearcher.approTime.stop();	
		return output;
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
