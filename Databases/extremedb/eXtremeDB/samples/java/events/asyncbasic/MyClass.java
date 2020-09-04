import com.mcobject.extremedb.Database;
import com.mcobject.extremedb.Event;
import com.mcobject.extremedb.Indexable;
import com.mcobject.extremedb.Persistent;
import com.mcobject.extremedb.Trigger;

@Persistent(autoid=true)
@Trigger({
		 @Event(name="NewEvent", type=Database.EventType.OnNew),
		 @Event(name="DeleteEvent", type=Database.EventType.OnDelete),
		 @Event(name="DeleteAllEvent", type=Database.EventType.OnDeleteAll),
		 @Event(name="CheckPointEvent", type=Database.EventType.OnCheckpoint),
		 @Event(name="UpdateEvent", type=Database.EventType.OnUpdate)
		 })
class MyClass
{
    @Indexable(type = Database.IndexType.BTree, unique = true)
    public int key;

    @Event(name="FieldUpdateEvent", type=Database.EventType.OnFieldUpdate)
    public int value;
}

