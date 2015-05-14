
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <math.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

#include "MySQLDB.h"

int main(int ac, char** av)
{

	char SQL[1024];
    MYSQL_RES * resultset;
    MYSQL_ROW row;
	MySQLDB * mysqldb = new MySQLDB();
	if(mysqldb->execQuery("SET NAMES utf8") == true) {}

	string testsql = "SELECT item_id, good_title FROM jd_goods WHERE item_id=%d";

	ifstream ifs("360Buy_Ratingsimilarity.sim");
    string line;
    while( getline(ifs, line) )
    {
    	if( line.size() != 0 ) 
        {
        	int pos1 = line.find_first_of(':');
	        string srcid = line.substr(0, pos1);
    	    int pos2 = line.find_first_of(' ', pos1 + 1); 
			
			string filename = "data/" + srcid + ".txt";

		 	cout << "Processing file: " << filename << endl;
			ofstream outfile( filename.c_str() );

			int item1 = atoi( srcid.c_str() ); 
			memset(SQL, 0, 1024);
			sprintf(SQL, testsql.c_str(), item1 );

			if(mysqldb->execQuery(SQL) == true)
    		{
			    resultset = mysqldb->storeResultSet();
			    if(resultset != NULL && mysqldb->getRowsNum(resultset) > 0)
        		{
		        	while((row = mysqldb->fetchRow(resultset))!= NULL)
        		    {
                	//	cout << row[0] << '\t' << row[1] << endl;
                		outfile << row[0] << '\t' << row[1] << endl << endl;
            		}
        		}
			}
        	mysqldb->freeRecordSet(resultset);

       	 	while (pos2 != string::npos)
        	{
        		string dstid = line.substr(pos1 + 1, pos2 - pos1 - 1 );
    	    	int pos3 = line.find_first_of(';', pos2 + 1); 
        		string simval = line.substr(pos2 + 1, pos3 - pos2 - 1 );
				
				int item2 = atoi( dstid.c_str() ); 
				double similarity = atof( simval.c_str() );
	            //cout << item1 << ':' << item2 << ':' << similarity << endl;

			    memset(SQL, 0, 1024);
			    sprintf(SQL, testsql.c_str(), item2 );

			    if(mysqldb->execQuery(SQL) == true)
    			{
			        resultset = mysqldb->storeResultSet();
			        if(resultset != NULL && mysqldb->getRowsNum(resultset) > 0)
        			{
		            	while((row = mysqldb->fetchRow(resultset))!= NULL)
        		    	{
                			//cout << row[0] << '\t' << row[1] << endl;
                			outfile << row[0] << '\t' << similarity << '\t' <<  row[1] << endl;
            			}
        			}
				}
        		mysqldb->freeRecordSet(resultset);

       		    pos1 = pos3;   
       	     	pos2 = line.find_first_of(' ', pos1 + 1); 
        	}
		
  			outfile.close(); 
		}
    }

    ifs.close();
	
	delete mysqldb;
}

