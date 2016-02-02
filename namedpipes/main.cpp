#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

const char* in_fifo_name = "in.fifo";
const char* out_fifo_name = "out.fifo";


void createFIFO( const char* name  )
{
	if( mkfifo( name, 0666 ) == -1 ) {
		printf( "error creating in FIFO: %s\n", name );
	}
}

int main()
{
	createFIFO( in_fifo_name );
	createFIFO( out_fifo_name );
	int to_read = open( in_fifo_name, O_RDONLY );
	int to_write = open( out_fifo_name, O_WRONLY );
	if( to_read == -1 || to_write == -1 ) {
		printf( "Can't open fifo %d %d\n", to_read, to_write );
	}
	size_t len = 0;
	char buffer[4096];
	while( (len = read( to_read, buffer, sizeof( buffer ) ) ) > 0 ) {
		size_t len2 = write( to_write, buffer, len );
		if( len2 < len ) {
			printf( "Write error" );
		}
	}
	return 0;
}
