import com.mcobject.extremedb.*;

public class Quote 
{ 
    @Indexable(unique=true)
    @Dimension(21)
    String symbol;
    
    @Sequence(type=Sequence.Type.UInt4, order=Sequence.Order.Ascending)
    OrderedSequence day;

    @Sequence(type=Sequence.Type.Float)
    UnorderedSequence low;
 
    @Sequence(type=Sequence.Type.Float)
    UnorderedSequence high;

    @Sequence(type=Sequence.Type.Float)
    UnorderedSequence open;

    @Sequence(type=Sequence.Type.Float)
    UnorderedSequence close;

    @Sequence(type=Sequence.Type.UInt4)
    UnorderedSequence volume;

    @Sequence(type=Sequence.Type.Char, elemSize=15)
    UnorderedSequence day_str;
};
