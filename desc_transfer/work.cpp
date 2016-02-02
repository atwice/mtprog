#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

int set_nonblock( int fd )
{
	int flags;
#ifdef O_NONBLOCK
	if( -1 == (flags = fcntl(fd, F_GETFL, 0 ))) {
		flags = 0;
	}
	return fcntl( fd, F_SETFL, flags | O_NONBLOCK );
#else
	flags = 1;
	return ioctl( fd, FIONBIO, &flags );
#endif
}

void send_to_second_line( int sec_line_sock, int fd )
{
	char buffer[] = "empty";

	struct iovec iov;
	iov.iov_base = buffer;
	iov.iov_len = 6;

	struct msghdr msg;
	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	union {
		struct cmsghdr cmsghdr;
		char control[CMSG_SPACE( sizeof( int ) )];
	} cmsgu;
	struct cmsghdr* cmsg;

	if( fd == -1 ) {
		printf( "Err sending socket to child process\n" );
		return;
	}
	msg.msg_control = cmsgu.control;
	msg.msg_controllen = sizeof( cmsgu.control );	
	cmsg = CMSG_FIRSTHDR( &msg );
	cmsg->cmsg_len = CMSG_LEN( sizeof( int ) );
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	*((int*) CMSG_DATA(cmsg) ) = fd;

	size_t size = sendmsg( sec_line_sock, &msg, 0 );
	if( size < 0 ) {
		printf( "Error sending msg with socket" );
	}
}

int recv_sock_from_first_line( int parent_socket )
{
	char buffer[6];
	struct iovec iov;
	iov.iov_base = buffer;
	iov.iov_len = 6;

	struct msghdr msg;
	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	union {
		struct cmsghdr cmsghdr;
		char control[CMSG_SPACE( sizeof( int ) )];
	} cmsgu;
	struct cmsghdr* cmsg;

	msg.msg_control = cmsgu.control;
	msg.msg_controllen = sizeof( cmsgu.control );	
	
	size_t size = recvmsg( parent_socket, &msg, 0 );
	if( size < 0 ) {
		printf( "Error receiving socket from first line\n" );
		return -1;
	}

	cmsg = CMSG_FIRSTHDR( &msg );
	if( cmsg && cmsg->cmsg_len == CMSG_LEN( sizeof(int) ) ) {
		if( cmsg->cmsg_level != SOL_SOCKET 
			|| cmsg->cmsg_type != SCM_RIGHTS )
		{
			printf( "Error recv socket from first lin 2\n" );
			return -1;
		}
		return *((int*) CMSG_DATA(cmsg));
	} else {
		printf( "Error receiving 3\n" );
		return -1;
	}
}

void first_line( int child_socket )
{
	int out_socket = socket( AF_INET, SOCK_STREAM, 0 );
	if( out_socket <= 0 ) {
		printf( "Cant create out socket\n" );
		return;
	}
	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons( 8080 );
	sa.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
	bind( out_socket, (sockaddr*)&sa, sizeof( sockaddr_in ) );	

	listen( out_socket, SOMAXCONN );
	
	while( true ) {
		int slave_socket = accept( out_socket, 0, 0 );
		char reply[100];
		size_t len = sprintf( reply, "Hello from first line. Your fd=%d",slave_socket );
		send( slave_socket, reply, len+1, 0 );
		send_to_second_line( child_socket, slave_socket );
	}
}

void second_line( int to_read )
{
	char buffer[4];
	int size = 0;
	while( true ) {
		int slave_socket = recv_sock_from_first_line( to_read );
		do {
			size = recv( slave_socket, buffer, sizeof( buffer ), MSG_NOSIGNAL );
			send(slave_socket, buffer, size, MSG_NOSIGNAL );
		} while( size > 0 );
	}		
}

int main()
{
	int sock_pair[2];
	int err = socketpair( AF_UNIX, SOCK_STREAM, 0, sock_pair );
	if( err != 0 ) {
		printf( "Error creating Unix Sockets\n" );
		return -1;
	}

	int child_pid = fork();
	if( child_pid ) {
		close( sock_pair[1] );
		first_line( sock_pair[0] );
		waitpid( child_pid, 0, 0 );
	} else {
		close( sock_pair[0] );
		second_line( sock_pair[1] );
	}
	return 0;	
}
