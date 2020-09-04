/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <commonSQL.h>
#include <mcosql.h>
#include "initializedb.h"

using namespace McoSql;

int main(int argc, char* argv[])
{
    char buf[MAX_LINE_LENGTH];
    char sql[MAX_LINE_LENGTH * 2];
    McoSqlEngine engine;

    sample_os_initialize(DEFAULT|DISK);

    if (argc < 2)
    {
        fprintf(stderr, "Import data from CSV files\n""Usage: initialize {table.csv}\n");
        sample_os_shutdown();
        return 1;
    }

    #ifdef EXTREMEDB_LICENSE_KEY
    engine.license(EXTREMEDB_LICENSE_KEY);
    #endif
    engine.open("initializedb", initializedb_get_dictionary(), DATABASE_SIZE, PAGE_SIZE);

    for (int i = 1; i < argc; i++)
    {
        char* fileName = argv[i];
        FILE* f = fopen(fileName, "r");
        if (f == NULL)
        {
            fprintf(stderr, "Failed to open file %s\n", fileName);
            continue;
        }
        char* suf = strrchr(fileName, '.');
        if (suf != NULL)
        {
            *suf = '\0';
        }
        char* beg = strrchr(fileName, '/');
        if (beg == NULL)
        {
            beg = strrchr(fileName, '\\');
        }
        char* tableName = (beg != NULL) ? beg + 1: fileName;
        if (fgets(buf, sizeof(buf), f))
        {
            int len = (int)strlen(buf);
            while (--len >= 0 && (buf[len] == '\n' ||buf[len] == '\r' || buf[len] == ','));
            buf[len + 1] = '\0';
            char* values = sql + sprintf(sql, "insert into %s (%s) values ", tableName, buf);
            while (fgets(buf, (int)sizeof(buf), f))
            {
                len = (int)strlen(buf);
                if (len > 1)
                {
                    buf[len - 1] = '\0';
					if( buf[len - 2] == '\r')
					{
						buf[len - 2] = '\0';
					}
                    char* dst = values;
                    char* src = buf;
                    char sep = '(';
                    while (*src != '\0')
                    {
                        char* comma = strchr(src, ',');
                        if (comma != NULL)
                        {
                            *comma = '\0';
                        }
                        else
                        {
                            comma = src + strlen(src);
                        }
                        dst += sprintf(dst, "%c'%s'", sep, src);
                        sep = ',';
                        src = comma + 1;
                    }
                    *dst++ = ')';
                    *dst = '\0';
                    engine.executeStatement(sql);
                }
            }
        }
        fclose(f);
        printf("Imported table %s\n", tableName);
    }
#if (defined(_VXWORKS) && defined(_RTP))
    printf("\nProgram complete\n");
#else
    printf("\nPress Enter to exit");
    getchar();
#endif	
    engine.close();
    sample_os_shutdown();
    return 0;
}
