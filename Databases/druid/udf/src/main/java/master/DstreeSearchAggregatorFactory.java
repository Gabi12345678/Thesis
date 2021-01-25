package master;

import com.fasterxml.jackson.annotation.*;

import java.util.*;
import java.util.stream.*;
import java.io.*;
import cn.edu.fudan.cs.dstree.dynamicsplit.*;
import org.apache.druid.java.util.common.logger.Logger;

public class DstreeSearchAggregatorFactory extends MultiColumnAggregatorFactory {
	private static final Logger log = new Logger(DstreeSearchAggregatorFactory.class);

	private String indexfile;

	@JsonCreator
	public DstreeSearchAggregatorFactory(
                @JsonProperty("name") String name,
                @JsonProperty("timeField") String timeField,
                @JsonProperty("columns") List<String> columns,
		@JsonProperty("indexfile") String indexfile) {
                super(name, timeField, columns);
		this.indexfile = indexfile;
	}

	@Override
	public Object applyUdf(List<Long> timestamps, List<List<Double>> data) {
		int lines = timestamps.size();
                if(lines == 0){
                        return "{}";
                }
                int columns = data.get(0).size();
                double[][] input = new double[columns][lines];
                for(int i = 0; i < lines; i++){
                        for(int j = 0; j < columns; j++){
                                input[j][i] = data.get(i).get(j);
                        }
                }

		double[] distances = new double[0];
		try {
			distances = search(input, columns, lines);
		} catch (Exception e) {
			StringWriter sw = new StringWriter();
                        PrintWriter pw = new PrintWriter(sw);
                        e.printStackTrace(pw);
			log.warn("Error: %s", sw.toString());
			return sw.toString();
		}

		log.warn("Result is %s", Arrays.toString(distances));
               	return Arrays.toString(distances);
	}

        double[] search(double[][] data, int lines, int columns) throws IOException, ClassNotFoundException {
		log.warn("IndexFile is %s", this.indexfile);
                File file = new File(this.indexfile);
                Node newRoot = null;
                if (file.exists()) {
                        String indexFileName = this.indexfile + "\\" + "root.idx";
			log.warn("IndexFileName is %s", indexFileName);

			FileInputStream fis = new FileInputStream(indexFileName);
		        ObjectInputStream ios = new ObjectInputStream(fis);
		        newRoot = (Node) ios.readObject();
			log.warn("Finished reading node");

                } else {
			return new double[0];
		}
                int tsLength = columns;
                int totalTsCount = newRoot.getSize();
                int searchCount = lines;

		log.warn("Before times");

                IndexExactSearcher.totalTime.reset();
                IndexExactSearcher.totalTime.start();
                IndexExactSearcher.totalTime.suspend();
                IndexExactSearcher.ioTime.reset();
                IndexExactSearcher.ioTime.start();
                IndexExactSearcher.ioTime.suspend();
                IndexExactSearcher.approTime.reset();
                IndexExactSearcher.approTime.start();
                IndexExactSearcher.approTime.suspend();

		log.warn("Starting searching");
                double[] output = new double[lines];
                for (int c = 0; c < lines; ++c) {
                        double[] queryTs = data[c];
			log.warn("Query=%s", Arrays.toString(queryTs));
                        PqItem result = IndexExactSearcher.exactSearch(queryTs, newRoot);
                        output[c] = result.dist;
                }

                IndexExactSearcher.totalTime.stop();
                IndexExactSearcher.ioTime.stop();
                IndexExactSearcher.approTime.stop();
                return output;
        }

	@Override
	public byte udfCacheKey() { return (byte) 0x56; } 
}
