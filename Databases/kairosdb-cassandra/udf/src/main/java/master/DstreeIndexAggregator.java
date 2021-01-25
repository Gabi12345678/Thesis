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
        name = "dstreeindex",
	description = "UDF for Dstree exact search"
)
public class DstreeIndexAggregator implements Aggregator {
	public static final Logger logger = (Logger) LoggerFactory.getLogger(DstreeSearchAggregator.class);

	private DoubleDataPointFactory dataPointFactory;

	@FeatureProperty(
		name = "datafile",
		label = "Datafile",
		description = "Path of the data file"
	)
	private String datafile;

	@FeatureProperty(
		name = "indexfile",
		label = "Indexfile",
		description = "Path of the index file"
	)
	private String indexfile;

	@FeatureProperty(
		name = "tscount",
		label = "Tscount",
		description = "Number of timeseries in the data"
	)
	private int tscount;

	@Inject
	public DstreeIndexAggregator(DoubleDataPointFactory dataPointFactory) {
		this.dataPointFactory = dataPointFactory;
	}

	
	public void setDatafile(String datafile) {
		this.datafile = datafile;
	}

	public void setIndexfile(String indexfile) {
		this.indexfile = indexfile;
	}

	public void setTscount(int tscount) {
		this.tscount = tscount;
	}

	@Override
	public DataPointGroup aggregate(DataPointGroup dataPointGroup) {
			try {
				IndexBuilder.buildIndex(this.datafile, this.indexfile, 100, 1, 1000, this.tscount);
			} catch (IOException e) { 
				StringWriter sw = new StringWriter();
				PrintWriter pw = new PrintWriter(sw);
				e.printStackTrace(pw);
				logger.info("IOException: ----- " + sw.toString());
			} catch (ClassNotFoundException e) {
				logger.info("ClassNotFoundException");
			}

			logger.info("Applied Dstree Index");

			return new MatrixDataPointGroup(this.dataPointFactory, dataPointGroup.getName() + ".result", new double[0][0], new ArrayList<Long>()); 
	}

	@Override
	public boolean canAggregate(String groupType) {
		return true;
	}

	@Override
	public String getAggregatedGroupType(String groupType) {
		return groupType;
	}
}
