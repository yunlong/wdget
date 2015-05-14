#define USE_URL_H
#define USE_HASHTABLE_H

#include "matrix.h"

namespace matrix {

HashTable::HashTable (bool create) 
{
	m_size = HASH_SIZE / 8;
	m_table = new char[m_size];
  	if (create) 
	{
		for (ssize_t i = 0; i < m_size; i++) 
	  		m_table[i] = 0;
  	} 
	else 
	{
		int fd = open("hashtable.bak", O_RDONLY);
		if (fd < 0) 
		{
	  		cerr << "Cannot find hashtable.bak, restart from scratch\n";
      		for (ssize_t i = 0; i < m_size; i++) 
        		m_table[i] = 0;
		} 
		else 
		{
      		ssize_t len = 0;
      		while (len < m_size) 
			{
        		ssize_t BytesRead = read(fd, m_table + len, m_size - len);
        		if (BytesRead <= 0) 
				{
          			cerr << "Cannot read hashtable.bak : " << strerror(errno) << endl;
          			exit(1);
        		} 
				else 
				{
          			len += BytesRead;
        		}
      		}
      		close(fd);
    	}
  	}
}

HashTable::~HashTable () 
{
	delete [] m_table;
}

/* save the hashTable in a file */
void HashTable::saveToFile() 
{
	rename("hashtable.bak", "hashtable.old");
  	int fd = creat("hashtable.bak", 00600);
  	if (fd >= 0) 
	{
		int count = HASH_SIZE / 8;
	    int pos = 0;
  		while (pos < count ) 
		{
    		int i = write(fd, m_table + pos, count - pos);
    		if (i == -1) 
			{
      			switch (errno) 
				{
      			case EINTR:
        			break;
      			default:
        			pos = count + 1;
        			perror("Problem in ecrireBuff");
        			break;
      			}
    		} 
			else 
			{
      			pos += i;
    		}
  		}
		close(fd);
  	}
  	unlink("hashtable.old");
}

/* test if this url is allready in the hashtable
 * return true if it has been added
 * return false if it has allready been seen
 */
bool HashTable::test (Uri *U) 
{
	unsigned int code = hashCode(U);
  	unsigned int pos = code / 8;
  	unsigned int bits = 1 << (code % 8);
  	return m_table[pos] & bits;
}

/* set a url as present in the hashtable */
void HashTable::set (Uri *U) 
{
	unsigned int code = hashCode(U);
  	unsigned int pos = code / 8;
  	unsigned int bits = 1 << (code % 8);
  	m_table[pos] |= bits;
}

/* add a new url in the hashtable
 * return true if it has been added
 * return false if it has allready been seen
 */
bool HashTable::testSet (Uri *U) 
{
	unsigned int code = hashCode(U);
  	unsigned int pos = code / 8;
  	unsigned int bits = 1 << (code % 8);
  	int res = m_table[pos] & bits;
  	m_table[pos] |= bits;
  	return !res;
}

/* set a page in the hashtable 
 * return false if it was already there
 * return true if it was not (ie it is new)
 */
bool HashTable :: testSet (char *doc) 
{
	unsigned int code = 0;
  	char c;
  	for (unsigned int i = 0; (c = doc[i] ) != 0; i++) 
	{
    	if (c > 'A' && c < 'z')
      		code = (code * 23 + c) % m_size;
  	}
  	unsigned int pos = code / 8;
  	unsigned int bits = 1 << (code % 8);
  	int res = m_table[pos] & bits;
  	m_table[pos] |= bits;
  	return !res;
}

/* return a hashcode for this url */
unsigned int HashTable::hashCode (Uri * U) 
{
	const char * host = U->hostname().c_str(); 
	unsigned int h = U->port();
//	char * file = U->get_file(); 
	const char * file = U->path().c_str(); 
  	unsigned int i = 0;
  	while (host[i] != 0) 
	{
    	h = 31 * h + host[i];
    	i++;
  	}
  	i=0;
  	while (file[i] != 0) 
	{
    	h = 31 * h + file[i];
    	i++;
  	}
  	return h % HASH_SIZE;
}

/* small functions used later */
unsigned int HashTable :: siteHashCode (char *host) 
{
	unsigned int h = 0;
  	unsigned int i = 0;
  	while (host[i] != 0) 
	{
    	h = 37 * h + host[i];
    	i++;
  	}
  	return h % SITE_NAME_LIST_SIZE;
}

}
