import com.mcobject.extremedb.*;


@Persistent(autoid=true)
@Trigger({
@Event(name="NewEvent", type=Database.EventType.OnNew),
@Event(name="DeleteEvent", type=Database.EventType.OnDelete),
@Event(name="DeleteAllEvent", type=Database.EventType.OnDeleteAll),
@Event(name="CheckPointEvent", type=Database.EventType.OnCheckpoint)
})
public class MyClass
{
    @Indexable(type = Database.IndexType.BTree, unique = false)
    public int key;

    @Event(name="FieldUpdateEvent", type=Database.EventType.OnFieldUpdate)
    public int value;
}
