/* 
 * class hashTable
 * This class is in charge of making sure we don't crawl twice the same url
 */
#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__


#include "Uri.h"
using namespace htmlcxx;

#define HASH_SIZE   64000000
#define HASH_FILE   "hashtable.bak"

// Size of the arrays of Sites in main memory
#define SITE_NAME_LIST_SIZE     20000
#define SITE_IP_LIST_SIZE       10000

namespace matrix {

class HashTable 
{
private:
	ssize_t m_size;
  	char *	m_table;
  	char *	m_file;
public:
	HashTable(bool create);
  	~HashTable();

   /* save the hashTable in a file */
  	void saveToFile();

   /* test if this url is allready in the hashtable
   	* return true if it has been added
   	* return false if it has allready been seen
   	*/
  	bool test (Uri *U);

  	/* set a url as present in the hashtable*/
  	void set (Uri *U);

  	/* add a new url in the hashtable
  	 * return true if it has been added
   	 * return false if it has allready been seen
   	 */
  	bool testSet (Uri *U);
  
   /* set a page in the hashtable
   	* return false if it was already there
    * return true if it was not (ie it is new)
    */
  	bool testSet (char *doc);

	unsigned int hashCode (Uri * U);
	unsigned int siteHashCode (char * host);
};

}

#endif 
