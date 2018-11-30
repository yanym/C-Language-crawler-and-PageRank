#include "connserver.h"
#include <stdio.h>
#include <string.h>

#define d 256

#define SIZE_BUFFER 45920
#define SIZE_URL 512
char* HttpHeadCreate(const char* strUrl)
{
	if (strUrl == NULL) return NULL;

	char* strHttpHead = (char*)malloc(SIZE_BUFFER);
	memset(strHttpHead, 0x0, SIZE_BUFFER);
	char* strHost = getHostAddrFromUrl(strUrl);
	char* strParam = getParamFromUrl(strUrl);
	strcat(strHttpHead, "GET /");
	strcat(strHttpHead, strParam);
	strcat(strHttpHead, " HTTP/1.1\r\nAccept: */*\r\nAccept-Language: cn\r\nUser-Agent: Mozilla/4.0\r\nHost: ");
	strcat(strHttpHead, strHost);
	strcat(strHttpHead, "\r\nCache-Control: no-cache\r\nConnection: Keep-Alive\r\n\r\n\r\n");
	free(strParam);
	free(strHost);
	strParam = NULL;
	strHost = NULL;
	return strHttpHead;
}


char* getParamFromUrl(const char* strUrl)
{
	char url[SIZE_URL];
	memset(url, 0x0, sizeof(url));
	int i = 0;
	snprintf(url, SIZE_URL, "%s", strUrl);
	int iPos = -1;

	char* strAddr = search("http://", url);
	if (strAddr == NULL) 
	{
		strAddr = search("https://", url);
		if (strAddr != NULL) 
		{
			strAddr += 8;
		}
	}
	else {
		strAddr += 7;
	}

	if (strAddr == NULL) {
		strAddr = url;
	}
	char *strParam = (char*)malloc(strlen(strAddr) + 1);
	memset(strParam, 0x0, strlen(strAddr) + 1);
	
	for (i = 0; i < strlen(strAddr) + 1 + 1; ++i) 
	{
		if (strAddr[i] == '/') 
		{
			iPos = i;
			break;
		}
	}
	if (iPos == -1) 
	{
		strcpy(strParam, "");
	}
	else 
	{
		strcpy(strParam, strAddr + iPos + 1);
	}
	return strParam;
}

char* getHostAddrFromUrl(const char* strUrl)
{
	char url[SIZE_URL];
	memset(url, 0x0, sizeof(url));
	snprintf(url, SIZE_URL, "%s", strUrl);
	char* strAddr = search("http://", url);
	if (strAddr == NULL) 
	{
		strAddr = search("https://", url);
		if (strAddr != NULL) 
		{
			strAddr += 8;
		}
	}
	else {
		strAddr += 7;
	}

	if (strAddr == NULL) 
	{
		strAddr = url;
	}

	char *strHostAddr = (char*)malloc(strlen(strAddr) + 1);
	memset(strHostAddr, 0x0, strlen(strAddr) + 1);
	int i = 0;
	for (i = 0; i < strlen(strAddr) + 1; ++i) 
	{
		if (strAddr[i] == '/') 
		{
			break;
		}
		else 
		{
			strHostAddr[i] = strAddr[i];
		}
	}

	return strHostAddr;
}
//----------------------------Version 3: Rabin-Karp Match Start----------------------------
char* search(char *pat, char *txt)
{	
	int q = 11;
	int position;
    int m = strlen(pat);
    int n = strlen(txt);int DONOTCHANGE = n;
    int i, j;
    int p = 0;        //hash value for pattern
    int t = 0;        //hash value for txt
    int h = 1;

    for(i = 0; i < m - 1; i++) {
        h = (h * d) % q;
    }

    for(i = 0; i < m; i++) {
        p = (d * p + pat[i]) % q;
        t = (d * t + txt[i]) % q;
    }

    for(i = 0; i <= n - m; i++) {
        if(p == t) {
			for(j = 0; j < m; j++) {
                if(txt[i + j] != pat[j]) {
                    break;
                }
            }
            if(j == m) {
                //printf("Connect: found at %d\n", i);
                position = i;
            }
        }

        if(i < n - m) {
            t = (d * ( t - txt[i] * h) + txt[i + m]) % q;
            //mightnegative t
            if(t < 0) {
                t = t + q;
            }
        }
    }
    int COPYlength = (DONOTCHANGE - position);
    char *result = (char*)malloc(COPYlength);
	memset(result, 0x0, COPYlength);
	strncpy(result, txt + position, COPYlength);
	//printf("conncet server result: %s\n\n",result);
	return result;
}
//----------------------------Version 3: Rabin-Karp Match End----------------------------

