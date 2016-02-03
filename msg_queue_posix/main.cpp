#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mqueue.h>


int main()
{
	mqd_t mq = mq_open( "/test.mq", O_CREAT | O_RDONLY, 0666, 0 );

	char buffer[4096];
	memset(  buffer, 0, 4096 );
	size_t size = mq_receive( mq, buffer, 4096, 0 );


	int fd = open( "/home/box/message.txt", O_CREAT | O_WRONLY | O_TRUNC, 0666  );
	if( fd < 0 ) {
		printf( "Cant open out file" );
	}
	write( fd, buffer, size );
	close( fd );
	mq_unlink( "/test.mq" );
	return 0;
}
