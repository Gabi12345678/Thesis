package master;

import com.google.inject.AbstractModule;
import com.google.inject.Singleton;

public class UdfModule extends AbstractModule
{
	@Override
	protected void configure()
	{
		bind(KMeansAggregator.class);
		bind(CDAggregator.class);
		bind(RecovAggregator.class);
		bind(HotSaxAggregator.class);
		bind(ZScoreAggregator.class);
		bind(ScreenAggregator.class);
		bind(DstreeIndexAggregator.class);
		bind(DstreeSearchAggregator.class);
		bind(SaxRepresentationAggregator.class);
		bind(KNNAggregator.class);
	}
}
