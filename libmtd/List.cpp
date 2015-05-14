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

#define USE_LIST_H
#include "matrix.h"

namespace matrix {

void ListHead::remove()
{
	prev->next = next;
	next->prev = prev;
	prev = next = this;
}

ListHead *ListHead::removeHead()
{
	ListHead *t = next;
	next->remove();
	return t;
}

/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
void ListHead::add(ListHead *item)
{
	item->prev = prev;
	item->next = this;
	prev->next = item;
	prev = item;
}

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
void ListHead::addHead(ListHead *item)
{
	item->prev = this;
	item->next = next;
	next->prev = item;
	next = item;
}

}

