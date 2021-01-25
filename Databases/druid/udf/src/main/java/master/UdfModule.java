package master;

import com.fasterxml.jackson.databind.Module;
import com.fasterxml.jackson.databind.jsontype.NamedType;
import com.fasterxml.jackson.databind.module.SimpleModule;
import com.google.inject.Binder;
import org.apache.druid.initialization.DruidModule;

import java.util.Collections;
import java.util.List;

public class UdfModule implements DruidModule {
	@Override
	public List<? extends Module> getJacksonModules() {
		return Collections.singletonList( 
			new SimpleModule("UdfModule").registerSubtypes( 
				new NamedType(StringAggregatorFactory.class, "stringify"),
				new NamedType(MultiSumAggregatorFactory.class, "multisum"),
				new NamedType(KMeansAggregatorFactory.class, "kmeans"), 
				new NamedType(NormalizationAggregatorFactory.class, "znormalization"),
				new NamedType(CDAggregatorFactory.class, "cd"),
				new NamedType(RecovAggregatorFactory.class, "recov"),
				new NamedType(HotSaxAggregatorFactory.class, "anomalydet"),
				new NamedType(ScreenAggregatorFactory.class, "screen"),
				new NamedType(DstreeIndexAggregatorFactory.class, "dstreeindex"),
				new NamedType(DstreeSearchAggregatorFactory.class, "dstreesearch"),
				new NamedType(SaxRepresentationAggregatorFactory.class, "saxrepresentation"),
				new NamedType(KNNAggregatorFactory.class, "knn")
			));
	}

	@Override
	public void configure(Binder binder) {}
}
