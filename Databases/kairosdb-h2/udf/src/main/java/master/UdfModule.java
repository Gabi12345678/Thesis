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
	}
}
