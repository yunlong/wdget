/*
 *  dget - download get for linux by yunlong.lee
 *  @author           YunLong.Lee    <yunlong.lee@163.com>
 *  @version          0.5v
 *
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * 
 */

#define USE_SOCKET_H
#define USE_POLLER_H

#include "matrix.h"

#define NEVENT 32000
#define MAX_EPOLL_TIMEOUT (35*60*1000)

namespace matrix {

bool TPoller :: start(void)
{
    int m_nevents = NEVENT;
	struct rlimit r1;
    if (getrlimit(RLIMIT_NOFILE, &r1) == 0 && r1.rlim_cur != RLIM_INFINITY)
	{
        /*
         * Solaris is somewhat retarded - it's important to drop
         * backwards compatibility when making changes.  So, don't
         * dare to put rl.rlim_cur here.
         */
		m_nevents = r1.rlim_cur - 1;
    }

    m_epfd = epoll_create(m_nevents);

	if(m_epfd == -1)
		return false;

    m_pevents = new TPollEvent[m_nevents];
	return true;
}

void TPoller :: stop(void)
{
	close(m_epfd);
	m_epfd = -1;

	if(m_pevents != NULL) 
		delete[] m_pevents;
}

void TPoller::addSocket( TSocket * skp )
{
	struct epoll_event ee;
	ee.events = 0;
	
	if(skp->getMaskRead())
	{
		ee.events = ee.events | EPOLLIN;
	}
	
	if(skp->getMaskWrite())
	{
		ee.events = ee.events | EPOLLOUT;
	}

	ee.data.ptr = skp;
	epoll_ctl(m_epfd, EPOLL_CTL_ADD, skp->getHandle(), &ee);
	
}

void TPoller :: delSocket( TSocket * skp )
{
	epoll_ctl(m_epfd, EPOLL_CTL_DEL, skp->getHandle(), NULL);
}

void TPoller :: modSocket( TSocket * skp )
{
	struct epoll_event ee;
	ee.events = 0;
	
	if(skp->getMaskRead())
	{
		ee.events = ee.events | EPOLLIN;
	}

	if(skp->getMaskWrite())
	{
		ee.events = ee.events | EPOLLOUT;
	}

	ee.data.ptr = skp;

	epoll_ctl(m_epfd, EPOLL_CTL_MOD, skp->getHandle(), &ee);	
}

TPollEvent * TPoller :: getPollEvents(void)
{
	return m_pevents;
}

int TPoller :: poll( int timeout )
{
    if (timeout > MAX_EPOLL_TIMEOUT) 
	{
        /* Linux kernels can wait forever if the timeout is too big;*/
        timeout = MAX_EPOLL_TIMEOUT;
    }

	int	rc = epoll_wait(m_epfd, m_pevents, m_nevents, timeout);
	return rc;
}

}

