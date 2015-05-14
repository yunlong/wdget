/*
 * Copyright (C) 2000-2003 the xine project,
 *
 * This file is part of xine, a free video player.
 *
 * xine is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * xine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 *
 * abortable i/o helper functions
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __GNUC__
# include <inttypes.h>
#else
# include "stdwin.h"
#endif
#ifdef WIN32
#  include <winsock.h>
#else
# include <sys/time.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>

#include "xsysio.h"


/* private constants */
#define XIO_FILE_READ             0
#define XIO_FILE_WRITE            1
#define XIO_TCP_READ              2
#define XIO_TCP_WRITE             3
#define XIO_POLLING_INTERVAL      50000  /* usec */


#ifndef ENABLE_IPV6
static int
xio_tcp_connect_ipv4(const char *host, int port)
{
  struct hostent *h;
  int             i, s;
  
  h = gethostbyname(host);
  if (h == NULL) {
    xmessage(XMSG_UNKNOWN_HOST, "unable to resolve", host, NULL);
    return -1;
  }

  s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);  
  if (s == -1) {
    xmessage(XMSG_CONNECTION_REFUSED, "failed to create socket", strerror(errno), NULL);
    return -1;
  }

#ifndef WIN32
  if (fcntl (s, F_SETFL, fcntl (s, F_GETFL) | O_NONBLOCK) == -1) {
    xmessage(XMSG_CONNECTION_REFUSED, "can't put socket in non-blocking mode", strerror(errno), NULL);
    return -1;
  }
#else
  {
	unsigned long non_block = 1;
	int rc;

    rc = ioctlsocket(s, FIONBIO, &non_block);

    if (rc == SOCKET_ERROR) {
      xmessage(XMSG_CONNECTION_REFUSED, "can't put socket in non-blocking mode", strerror(errno), NULL);
	  return -1;
    }
  }
#endif

  for (i = 0; h->h_addr_list[i]; i++) {
    struct in_addr ia;
    struct sockaddr_in sin;
 
    memcpy (&ia, h->h_addr_list[i], 4);
    sin.sin_family = AF_INET;
    sin.sin_addr   = ia;
    sin.sin_port   = htons((uint16_t)port);
    
#ifndef WIN32
    if (connect(s, (struct sockaddr *)&sin, sizeof(sin))==-1 && errno != EINPROGRESS) {
#else
    if (connect(s, (struct sockaddr *)&sin, sizeof(sin))==-1 && WSAGetLastError() != WSAEWOULDBLOCK) {
#endif /* WIN32 */

      xmessage(XMSG_CONNECTION_REFUSED, strerror(errno), NULL);
      close(s);
      continue;
    }
    
    return s;
  }
  return -1;
}
#endif

/**
 * open a tcp connection
 *
 * @params:
 *   host          address of target
 *   port          port on target
 * @returns a socket descriptor or -1 if an error occured
 */
extern int
xio_tcp_connect(const char *host, int port)
{
#ifndef ENABLE_IPV6
    return xio_tcp_connect_ipv4(host, port);
#else

  int             s;
  struct addrinfo hints, *res, *tmpaddr;
  int error;
  char strport[16];
	
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = PF_UNSPEC; 

  snprintf(strport, sizeof(strport), "%d", port);

  xprintf(XVERBOSITY_DEBUG, "Resolving host '%s' at port '%s'\n", host, strport);

  error = getaddrinfo(host, strport, &hints, &res);

  if (error) {
    xmessage(XMSG_UNKNOWN_HOST, "unable to resolve", host, NULL);
    return -1;
  }

  tmpaddr = res;

  while (tmpaddr)
  {
      s = socket(tmpaddr->ai_family, SOCK_STREAM, IPPROTO_TCP);  
      if (s == -1) {
			xmessage(XMSG_CONNECTION_REFUSED, 
				   "failed to create socket", strerror(errno), NULL);
			tmpaddr = tmpaddr->ai_next;
			continue;
      }

      /**
       * Uncommenting nonblocking features due to IPv6 support.
       * Need to know if the connect failed, in order to try another
       * address (if available).  Error will be reported if no address
       * worked.
       */

#ifndef WIN32

    if (connect(s, tmpaddr->ai_addr, 
		tmpaddr->ai_addrlen)==-1 && errno != EINPROGRESS) {
	
#else
    if (connect(s, tmpaddr->ai_addr, 
		tmpaddr->ai_addrlen)==-1 && 
	WSAGetLastError() != WSAEWOULDBLOCK) {

      xprintf(XVERBOSITY_DEBUG, "io_helper: WSAGetLastError() = %d\n", WSAGetLastError());
#endif /* WIN32 */

      error = errno;
      close(s);
      tmpaddr = tmpaddr->ai_next;
      continue;
    } else {
    
      return s;
    }
  
    tmpaddr = tmpaddr->ai_next;
  }

  xmessage(XMSG_CONNECTION_REFUSED, strerror(error), NULL);
  
  return -1;
#endif
}


/**
 * Waits for a file descriptor/socket to change status. <BR>
 *
 * network input plugins should use this function in order to
 * not freeze the engine.
 *
 * @params :
 *   fd            file/socket descriptor
 *   state         XIO_READ_READY, XIO_WRITE_READY
 *   timeout_sec   timeout in seconds
 *
 * @return value :
 *   XIO_READY     the file descriptor is ready for cmd
 *   XIO_ERROR     an i/o error occured
 *   XIO_ABORTED   command aborted by an other thread
 *   XIO_TIMEOUT   the file descriptor is not ready after timeout_msec milliseconds
 */
extern int
xio_select(int fd, int state, int timeout_msec)
{
  fd_set fdset;
  fd_set *rset, *wset;
  struct timeval select_timeout;
  int timeout_usec, total_time_usec;
  int ret;
  
  timeout_usec = 1000 * timeout_msec;
  total_time_usec = 0;
  
  while (total_time_usec < timeout_usec)
  {
    FD_ZERO (&fdset);
    FD_SET  (fd, &fdset);
    
    select_timeout.tv_sec  = 0;
    select_timeout.tv_usec = XIO_POLLING_INTERVAL;
    
    rset = (state & XIO_READ_READY) ? &fdset : NULL;
    wset = (state & XIO_WRITE_READY) ? &fdset : NULL;
    ret = select (fd + 1, rset, wset, NULL, &select_timeout);
    
    if (ret == -1) {
      /* select error */
      return XIO_ERROR;
    } else if (ret == 1) {
      /* fd is ready */
      return XIO_READY;
    }
    
    /* select timeout
     *   aborts current read if action pending. otherwise xine
     *   cannot be stopped when no more data is available.
    if (stream && stream->demux_action_pending)
      return XIO_ABORTED;
     *
     */
    total_time_usec += XIO_POLLING_INTERVAL;
  }
  return XIO_TIMEOUT;
}


/**
 * wait for finish connection
 *
 * @params :
 *   fd            socket descriptor
 *   timeout_msec  timeout in milliseconds
 *
 * @return value :
 *   XIO_READY     host respond, the socket is ready for cmd
 *   XIO_ERROR     an i/o error occured
 *   XIO_ABORTED   command aborted by an other thread
 *   XIO_TIMEOUT   the file descriptor is not ready after timeout
 */
extern int
xio_tcp_connect_finish(int fd, int timeout_msec)
{
  int ret;
  
  ret = xio_select(fd, XIO_WRITE_READY, timeout_msec);

  /* find out, if connection is successfull */
  if (ret == XIO_READY)
  {
    int err;
    socklen_t len = sizeof(int);
    
    if ((getsockopt(fd, SOL_SOCKET, SO_ERROR, (void *)&err, &len)) == -1)
	{
      xmessage(XMSG_CONNECTION_REFUSED, _("failed to get status of socket"), strerror(errno), NULL);
      return XIO_ERROR;
    }
    if (err)
	{
      xmessage(XMSG_CONNECTION_REFUSED, strerror(errno), NULL);
      return XIO_ERROR;
    }
  }
  
  return ret;
}


static off_t
xio_rw_abort(int fd, int cmd, void *buf, off_t todo)
{
  off_t ret = -1;
  off_t total = 0;
  int sret;
  int state = 0;
  
  if ((cmd == XIO_TCP_READ) || (cmd == XIO_FILE_READ))
    state = XIO_READ_READY;
  else
    state = XIO_WRITE_READY;
  
  while (total < todo)
  {
    do {
      sret = xio_select(fd, state, 500); /* 500 ms */
    } while (sret == XIO_TIMEOUT);
    
    if (sret != XIO_READY)
      return -1;
    
    switch (cmd)
    {
      case XIO_FILE_READ:
        ret = read(fd, (char *)buf + total, todo - total);
        break;
      case XIO_FILE_WRITE:
        ret = write(fd, (char *)buf + total, todo - total);
        break;
      case XIO_TCP_READ:
        ret = recv(fd, (char *)buf + total, todo - total, 0);
        break;
      case XIO_TCP_WRITE:
        ret = send(fd, (char *)buf + total, todo - total, 0);
        break;
      default:
        assert(1);
		break;
    }
    /* check EOF */
    if (!ret)
      break;

    /* check errors */
    if (ret < 0)
    {
      /* non-blocking mode */
#ifndef WIN32
      if (errno == EAGAIN)
        continue;

      if (errno == EACCES) {
        xmessage(XMSG_PERMISSION_ERROR, NULL, NULL);
      } else if (errno == ENOENT) {
        xmessage(XMSG_FILE_NOT_FOUND, NULL, NULL);
      } else if (errno == ECONNREFUSED) {
        xmessage(XMSG_CONNECTION_REFUSED, NULL, NULL);
      } else {
        perror("io_helper: I/O error");
      }
#else
      if (WSAGetLastError() == WSAEWOULDBLOCK)
        continue;
      xprintf(XVERBOSITY_DEBUG, "io_helper: WSAGetLastError() = %d\n", WSAGetLastError());
#endif
      
      return ret;
    }
    total += ret;
  }
  return total;
}

/*
 * read from tcp socket checking demux_action_pending
 *
 * network input plugins should use this function in order to
 * not freeze the engine.
 *
 * aborts with zero if no data is available and *abort is set
 */
extern off_t
xio_tcp_read(int s, void *buf, off_t todo)
{
	return xio_rw_abort(s, XIO_TCP_READ, buf, todo);
}

/*
 * write to a tcp socket checking demux_action_pending
 *
 * network input plugins should use this function in order to
 * not freeze the engine.
 *
 * aborts with zero if no data is available and *abort is set
 */
extern off_t
xio_tcp_write(int s, void *buf, off_t todo)
{
	return xio_rw_abort(s, XIO_TCP_WRITE, buf, todo);
}

/*
 * read from a file descriptor checking demux_action_pending
 *
 * the fifo input plugin should use this function in order to
 * not freeze the engine.
 *
 * aborts with zero if no data is available and *abort is set
 */
extern off_t
xio_file_read(int s, void *buf, off_t todo)
{
	return xio_rw_abort(s, XIO_FILE_READ, buf, todo);
}

/*
 * write to a file descriptor checking demux_action_pending
 *
 * the fifo input plugin should use this function in order to
 * not freeze the engine.
 *
 * aborts with zero if *abort is set
 */
extern off_t
xio_file_write(int s, void *buf, off_t todo)
{
	return xio_rw_abort(s, XIO_FILE_WRITE, buf, todo);
}

/**
 * read a string from socket, return string length (same as strlen) <BR>
 * the string is always '\0' terminated but given buffer size is never exceeded <BR>
 * that is, _x_io_tcp_read_line(,,,X) <= (X-1) ; X > 0
 */
extern int
xio_tcp_read_line(int sock, char *str, int size)
{
  int i = 0;
  char c;
  off_t r;

  if( size <= 0 )
    return 0;

  while ((r = xio_rw_abort(sock, XIO_TCP_READ, &c, 1)) != -1) {
    if (c == '\r' || c == '\n')
      break;
    if (i+1 == size)
      break;

    str[i] = c;
    i++;
  }

  if (r != -1 && c == '\r')
    r = xio_rw_abort(sock, XIO_TCP_READ, &c, 1);

  str[i] = '\0';

  return (r != -1) ? i : (int)r;
}


