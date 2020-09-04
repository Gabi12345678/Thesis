This sample demonstrates binary schema evolution (BSE) during the database save\load cycle.

A database backup file must first be created by running sample "18-backup-db". This sample 
creates file "backupdb.bak" with the dictionary generated from the schema file 
"samples\core\18-backup\db\schema.mco" and with the "BSE" option necessary for sample 
"18-backup-bse".  (Note line 91 of file "samples\core\18-backup\db\main.c" which causes
function "mco_db_save()" to save the database backup file "backupdb.bak" in BSE mode.)

After creating the backup file, run sample "18-backup-bse". It opens the database backup file 
"backupdb.bak" and loads the database image by calling function "mco_db_load()".  Note that 
the database schema for sample "18-backup-bse" is different from the schema for sample 
"18-backup-db"; but, because file "backupdb.bak" contains information for schema evolution, 
the function "mco_db_load()" loads this data successfully applying the necessary data conversion.

If the runtime option "MCO_RT_OPTION_DB_SAVE_BSE" was not set in sample "18-backup-db", no 
information about the dictionary would have been saved in the database backup file and
sample "18-backup-bse" would fail in function "mco_db_load()" with the error code 
"MCO_E_VERS_MISMATCH".
