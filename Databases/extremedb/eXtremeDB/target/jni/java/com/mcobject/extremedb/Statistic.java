package com.mcobject.extremedb;

public class Statistic {	

    public static class ClassStat {
        public long ObjectsCount;
        public int VersionsCount;
        public int CorePageCount;
        public int BlobPages;
        public int SeqPages;
        public long CoreSpace;
    
        public ClassStat(long ObjectsCount, 
                    int VersionsCount, 
                    int CorePageCount, 
                    int BlobPages, 
                    long CoreSpace)
        {
        	this.ObjectsCount = ObjectsCount;
        	this.VersionsCount = VersionsCount;
        	this.CorePageCount = CorePageCount;
        	this.BlobPages = BlobPages;
        	this.CoreSpace = CoreSpace;
        }
    }


    /* define index characteristics */
    final static int MCO_IDXST_TYPE_MEM         = 0;      /* In-memory index */
    final static int MCO_IDXST_TYPE_DISK        = 1;      /* Persistent index */
    final static int MCO_IDXST_TYPE_MASK        = 1;      
    
    final static int MCO_IDXST_NATURE_BTREE     = 0;      /* binary tree index */
    final static int MCO_IDXST_NATURE_PTREE     = 2;      /* Patricia tree index*/
    final static int MCO_IDXST_NATURE_KDTREE    = 4;      /* KD-tree index */
    final static int MCO_IDXST_NATURE_HASH      = 6;      /* hash index */
    final static int MCO_IDXST_NATURE_META      = 8;      /* meta-index */
    final static int MCO_IDXST_NATURE_RTREE     = 10;     /* R-tree index */
    final static int MCO_IDXST_NATURE_MASK      = 14;
    
    final static int MCO_IDXST_FEATURE_UNIQUE   = 0x10;   /* Unique index */
    final static int MCO_IDXST_FEATURE_UDF      = 0x20;   /* User-defined index */
    final static int MCO_IDXST_FEATURE_THICK    = 0x40;   /* User-defined index */

    final static int MCO_IDXST_FUNCTION_REGULAR = 0;      /* Regular class index */
    final static int MCO_IDXST_FUNCTION_OID     = 0x100;   /* Object-ID meta-index */
    final static int MCO_IDXST_FUNCTION_AUTOOID = 0x200;   /* Auto object-ID meta-index */
    final static int MCO_IDXST_FUNCTION_AUTOID  = 0x400;   /* Auto object id meta-index*/
    final static int MCO_IDXST_FUNCTION_LIST    = 0x800;   /* List meta-index */
    final static int MCO_IDXST_FUNCTION_MASK    = 0xF00;

    public static class IndexStat 
    {      
        public short ClassCode;             /* class code that the index belongs */

        public int type; /* type, MCO_IDXST_ flags */
    
        public String Label;                 /* label of the index */
        public long  KeysCount;               /* number of keys in the index */
        public long  PagesCount;              /* number of pages used by the index */
    }

	
    public static class IndexStatTree extends IndexStat
    {
        public long LevelsCount;
        public long DuplicatesCount;
    }
    
    public static class IndexStatKDTree extends IndexStat
    {
        public long LevelsCount;
    }

    public static class IndexStatHash extends IndexStat
    {
        public long AvgChainLength;
        public long MaxChainLength;
    }

    public static class IndexStatMeta extends IndexStat
    {
        public long DiskPagesCount;
    }

    public static class DiskInfo
    {
        public long DataFileSize;
        public long LogFileSize;
        public long UsedDatabaseSize;
        
        public DiskInfo(long DataFileSize, long LogFileSize, long UsedDatabaseSize)
        {
        	this.DataFileSize = DataFileSize;
        	this.LogFileSize = LogFileSize;
        	this.UsedDatabaseSize = UsedDatabaseSize;
        }
    };

}
