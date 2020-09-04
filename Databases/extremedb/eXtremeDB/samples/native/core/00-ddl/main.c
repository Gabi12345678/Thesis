#include <common.h>

int main( int argc, char ** argv ) {

    sample_os_initialize(DEFAULT);

    printf ( "Sample 'ddl' demonstrates the use of the eXtremeDB schema compiler.\n" );

    sample_pause_end("\n\nPress any key to continue . . . ");

    sample_os_shutdown();
    return 0;
}
