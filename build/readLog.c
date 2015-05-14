#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct
{
	off_t BlkSize;
	off_t RemoteStartPos;
 	off_t RemoteEndPos;
	off_t RemoteBytesReceived;
	off_t LocalStartPos;			
} ThreadRec;

typedef struct 
{
	off_t FileSize;
	unsigned int UrlAddrOff;		/* URL String offset		     */
	unsigned int UrlAddrSize;		/* URL String Length		     */
	unsigned int ThreadRecOff;		/* ThreadRec table entry size    */
	unsigned int ThreadRecSize; 	/* ThreadRec table entry size    */
	unsigned int ThreadRecNum;		/* ThreadRec table entry count   */
}LogHeader;


int main(int ac ,char * * av)
{
  	int i = 0;
  	char buffer[255];

 	unsigned int offset, nConn, Len, ThreadRecOff, UrlAddrOff;
 	char * UrlAddr;
 	off_t BytesPerConn;
   	off_t BytesLeft;
   	off_t FileSize;
   	
   	
	LogHeader loghdr;
 	ThreadRec tdrec;	
	FILE *fp = NULL;
 	
 	if(ac < 2)
 	{
 		fprintf(stdout, "%s\n", "Param < 2");	
 		return -1;
 	}
 	
	    	
    fp = fopen(av[1], "rb");
		
	if(fread(&loghdr, sizeof(LogHeader), 1, fp) != 1)
	{
		fclose(fp);
		return -1;
    }
    
	FileSize = loghdr.FileSize;
    nConn= loghdr.ThreadRecNum;   
    UrlAddrOff = loghdr.UrlAddrOff;
    Len = loghdr.UrlAddrSize;
    fseek(fp, UrlAddrOff, SEEK_SET);
    UrlAddr = (char *)malloc(Len + 1);
    if(fread(UrlAddr, Len, 1, fp) != 1 )
    {
    	fprintf(stdout, "%s\n", av[1]);	
    	fclose(fp);
        return -1;
    }
    
    	 	    	
	ThreadRecOff = loghdr.ThreadRecOff;
	
	fprintf(stdout, "\nLogFileHeader : [ FileSize is %lld | ThreadRecNum is %u "
						"| UrlAddrOff is %u  | UrlAddrSize is %u "
						"| ThreadRecOff is %u  ]\n\n", 
						FileSize, nConn, UrlAddrOff, Len, ThreadRecOff);
	
	fprintf(stdout, "%s\n\n", UrlAddr);

	free(UrlAddr);
		 				
	fseek(fp, ThreadRecOff, SEEK_SET);
	for(i = 0; i < nConn; i++)
	{		
		memset(&tdrec, 0, sizeof(ThreadRec));
		if(fread(&tdrec, sizeof(ThreadRec), 1, fp) != 1)
		{
			fclose(fp);
			return -1;	
		}
		
		fprintf(stdout, "ThreadRec : [ BlockSize is %lld | RemoteStartPos is %lld "
						"| RemoteEndPos is %lld  | RemoteBytesReceived is %lld "
						"| LocalStartPos is %lld  ]\n\n", 
						tdrec.BlkSize,
	 	 				tdrec.RemoteStartPos,
 	 	 				tdrec.RemoteEndPos,
	 	 				tdrec.RemoteBytesReceived,
		 				tdrec.LocalStartPos);
	}
		
	
	fclose(fp);
		
    return 0;	
}

