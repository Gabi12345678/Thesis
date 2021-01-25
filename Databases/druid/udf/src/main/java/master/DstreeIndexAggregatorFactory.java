package master;

import com.fasterxml.jackson.annotation.*;

import java.util.*;
import java.util.stream.*;
import java.io.*;
import cn.edu.fudan.cs.dstree.dynamicsplit.*;

public class DstreeIndexAggregatorFactory extends MultiColumnAggregatorFactory {
	private String datafile, indexfile;
	private int tscount;

	@JsonCreator
	public DstreeIndexAggregatorFactory(
		@JsonProperty("name") String name,
		@JsonProperty("tscount") int tscount,
		@JsonProperty("datafile") String datafile,
		@JsonProperty("indexfile") String indexfile) {
		super(name, null, new ArrayList<String>());
		this.tscount = tscount;
		this.datafile = datafile;
		this.indexfile = indexfile;
	}

	@Override
	public Object applyUdf(List<Long> timestamps, List<List<Double>> data) {
		try {
			IndexBuilder.buildIndex(this.datafile, this.indexfile, 100, 1, 1000, this.tscount);
		} catch (IOException | ClassNotFoundException e) {
			StringWriter sw = new StringWriter();
			PrintWriter pw = new PrintWriter(sw);
			e.printStackTrace(pw);
			return sw.toString();
		}
		return "Index done";
	}

	@Override
	public byte udfCacheKey() { return (byte) 0x55; } 
}
