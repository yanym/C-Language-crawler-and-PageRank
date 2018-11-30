#include "crawler.h"
#include "threadpool.h"

#define d 256
#define MAX_Thread 1

QueueType* urlQueue;
map_t allUrlMap;   //所有处理过的url所构成hashmap
BF* bf;

static int sockfdCount = 0;
int count = 0;

FILE* outfile;
FILE* confile;

void initParse(char* strUrl)
{
	bf = bloom_create();
	bloom_add(&bf, strUrl);

    allUrlMap = hashmap_new();
    struct KEY_VALUE* kv = malloc(sizeof(struct KEY_VALUE));
    snprintf(kv->urlname, URLSIZE, "%s", strUrl);
    kv->urlid = count;
    hashmap_put(allUrlMap, kv->urlname, &kv->urlid);
}


void read_cb(evutil_socket_t fd, short what, void *arg)
{
    struct EVENT_ARG* evarg = (struct EVENT_ARG*)arg;
    map_t curUrlMap = hashmap_new(); //当前页面处理过的hashmap
    int i = 0, j = 0, n;
    int state = 0;
    char buf[BUFSIZE];
    char urlbuf[URLSIZE];
    char urlhttp[19]="http://news.qq.com/";
    char curChar;
    //Regex Pattern Start

    //const char *pattern = "(https?|file)://[-A-Za-z0-9+&@#/%?=~_|!:,.;]+[-A-Za-z0-9+&@#/%=~_|]";
    //This one
    //const char *pattern = "((http|https)://)(([a-zA-Z0-9._-]+.[a-zA-Z]{2,6})|([0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}))(:[0-9]{1,4})*(/[a-zA-Z0-9&%_./-~-]*)?";
    /*
    ((http|https)://): Protocol
    (([a-zA-Z0-9._-]+.[a-zA-Z]{2,6}): 字母host，前半部分（+号及以前）host、二级域名，后半部分表示一定有类似.cn、.com、.net的跟在后面
    ([0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}): IP host，可以进一步简化([0-9]{1,3}(\.[0-9]{1,3}){3})
    (:[0-9]{1,4})* : 端口
    (/[a-zA-Z0-9\&%_\./-~-]*)? : /path及后面内容，第一个/匹配host后紧跟的斜杠，host后可能有 / ，/ 后可能还有n多字符，当然，也可能没有了。最后四个字符/-~-有些问题，/的ASCII码为47，~的为126，（ASCII码表）这之间包括了数字、大小写字母（与前面的重复），还有些如<>=?{}，这些在正常的url中也会被编码的，不会出现在url中，中括号中的 - 如果不成组就表示 - 字符（一般放在中括号两边或加转义）

    容易理解，可以满足大部分需求，不能匹配url+锚点、ftp有user:pass@host的情况
    */

    //Regex Pattern End
    while (1)
    {
        memset(buf, 0x0, BUFSIZE);
        int ret = recv(fd, (void*)buf, BUFSIZE, 0);
        if (ret == 0)
        {
            hashmap_free(curUrlMap);
            event_free(evarg->ev_r);
            if (evarg != NULL)
            {
                free(evarg);
                evarg = NULL;
            }
            break;
        }
        else if (ret > 0)
        {
            if (evarg->firstPack == 1)
            {
                evarg->firstPack = 0;
                char *substr = "200 OK";
//--------------If used for Err Checking--------------
                //if (searchSubstr(substr, buf) == NULL)
		        if (strstr(buf, substr) == NULL) {
                    //printf("Not found 200 OK\n");
                    hashmap_free(curUrlMap);
                    event_free(evarg->ev_r);
                    if (evarg != NULL) {
                        free(evarg);
                        evarg = NULL;
                    }
                    break;
                } else { 

//--------------If used for Err Checking--------------
                    struct KEY_VALUE* kv_a = malloc(sizeof(struct KEY_VALUE));
                    snprintf(kv_a->urlname, URLSIZE, "%s", evarg->url);
                    kv_a->urlid = count;
                    int error1 = hashmap_put(allUrlMap, kv_a->urlname, &kv_a->urlid);
                    if (error1 != MAP_OK)
                    {
                        printf("hashmap_put error\n");
                    }                               
                    fprintf(outfile, "%s %d\n", kv_a->urlname, count++);
//--------------If used for Err Checking--------------
                }

            }

/*----------------------------Version 2: Regex Match Start----------------------------
                    char *bematch = buf;
                    char errbuf[1024];
                    char match[100];
                    regex_t reg;
                    int err,nm = 100;
                    regmatch_t pmatch[nm];

                    if(regcomp(&reg,pattern,REG_EXTENDED) < 0){
                        regerror(err,&reg,errbuf,sizeof(errbuf));
                        printf("err:%s\n",errbuf);
                    }
                    regexec(&reg,bematch,nm,pmatch,0); // err = 

                    for(int i=0;i<100 && pmatch[i].rm_so!=-1;i++){
                        int len = pmatch[i].rm_eo-pmatch[i].rm_so;
                        if(len){
                            memset(match,'\0',sizeof(match));
                            memcpy(match,bematch+pmatch[i].rm_so,len);
                            dealWithUrl(match, evarg->url, curUrlMap);
                           // printf("%s\n",match);
                        }
                    }
----------------------------Version 2: Regex Match End----------------------------*/



            
//----------------------------Version 1: DFA Match Start----------------------------
            int len = strlen(buf);
            for(i = 0; i < len; i++)
	        {
                curChar = buf[i];
                switch(state)
                {
                    case 0:if (curChar == '<'){
                                state = 1; break;
                            } else {
                                state = 0; j = 0; break;
                            }
                    case 1:if (curChar == 'a' || curChar == 'A'){
                                state = 2; break;
                            } else {
                                state = 0; j = 0; break;
                            }
                    case 2:if (curChar == 'h'){
                                state = 3; break;
                            } else if(curChar == '>'){
                                state = 0; j = 0; break;
                            } else {
                                state = 2; break;
                            }
                    case 3:if (curChar == 'r'){
                                state = 4; break;
                            } else if (curChar == '>') {
                                state = 0; j = 0; break;
                            } else {
                                state = 2; break;
                            }
                    case 4:if (curChar == 'e'){
                                state = 5; break;
                            }
                            else if (curChar == '>'){
                                state = 0; j = 0; break;
                            } else {
                                state = 2; break;
                            }
                    case 5:if (curChar == 'f') {
                                state = 6; break;
                            } else if (curChar == '>'){
                                state = 0; j = 0; break;
                            } else {
                                state = 2; break;
                            }
                    case 6:if (curChar == '='){
                                state = 7; break;
                            }
                            else if (curChar == '>'){
                                state = 0; j = 0; break;
                            } else {
                                state = 2; break;
                            }
                    case 7:if (curChar == '"') {
                                state = 10; break;
                            } else if (curChar == ' ') {
                                state = 7; break;
                            } else {
                                state = 0; j = 0; break;
                            }
                    case 10:if ((curChar=='"')||(curChar=='|')||(curChar=='>')||(curChar=='#')) {
                                state = 0; j = 0; break;
                            } else if (curChar == '/') {
                                state = 8;
                                urlbuf[j++] = curChar;
                                break;
                            } else {
                                state = 10;
                                urlbuf[j++] = curChar;
                                break;
                            }
                    case 8:if (curChar == '"'){
                                state = 9; break;
                            } else if (curChar == '>') {
                                state = 0; j = 0; break;
                            } else {
                                state = 8;
                                urlbuf[j++] = curChar;
                                break;
                            }
                    case 9:urlbuf[j] = '\0';     
                            state = 0;
                            for(n = 0;n <= 18; ++n){
                                if(urlbuf[n] != urlhttp[n]){
                                    break;
                                }
                            }
                            if(n == 19){
                                printf("%s\n", urlbuf);
                                dealWithUrl(urlbuf, evarg->url, curUrlMap);
                                memset(urlbuf, 0x0, sizeof(urlbuf));
                                state = 0;
                                j = 0;
                            }
                            break;
                }
            }
//----------------------------Version 1: DFA Match End----------------------------
            


        }   //if(ret > 0) End
        else
        {
            if ((ret < 0) && (errno == EAGAIN || \
                    errno == EWOULDBLOCK || errno == EINTR))
            {
                continue;
            }
            hashmap_free(curUrlMap);
            event_free(evarg->ev_r);
            if (evarg != NULL)
            {
                free(evarg);
                evarg = NULL;
            }
            break;
        }
	}//while(1)

    sockfdCount--;
    close(fd);
}

void write_cb(evutil_socket_t fd, short what, void *arg)
{
    struct EVENT_ARG* evarg = (struct EVENT_ARG*)arg;
    char* url = evarg->url;
    char* strHttpHead = HttpHeadCreate(url);
    int ret = send(fd, (void*)strHttpHead, strlen(strHttpHead) + 1, 0);
    if (strHttpHead != NULL)
    {
        free(strHttpHead);
        strHttpHead = NULL;
    }
    if (ret < 0)
	{
		printf("send HttpHead ERROR\n");
		close(fd);
		return;
    }
    else if (ret >= 0)
    {
        return;
    }
}


//void* initbase(char* ip, int port)
void* initbase(void* ip)
{	
    ip = (char*) ip;
    printf("----- Start Requesting -----\n\n");
    printf("IP passed to initbase(): %s \n\n", ip);
    struct event_base* base = event_base_new();
    while (urlQueue->length != 0)
    {
        while (urlQueue->length != 0 && sockfdCount < 100)
        {
            char* nextUrl = deQueue(urlQueue);

            evutil_socket_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            struct sockaddr_in serverAddr;
            struct event* ev_write = NULL;
            struct event* ev_read = NULL;
            serverAddr.sin_family = AF_INET;
            // serverAddr.sin_port = htons(port);
            serverAddr.sin_port = htons(80);
            serverAddr.sin_addr.s_addr = inet_addr(ip);
            
            int ret = connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
            if (ret < 0 && ret != EINPROGRESS)
            {
                printf("errono = %d\n", errno);
                continue;
            }
            else if (ret == 0)
            {
                sockfdCount++;
            }
            evutil_make_socket_nonblocking(sock);
            struct EVENT_ARG* evarg = (struct EVENT_ARG*)malloc(sizeof(struct EVENT_ARG));
            ev_write = event_new(base, sock, EV_WRITE, write_cb, (void*)evarg);
            ev_read = event_new(base, sock, EV_READ|EV_PERSIST, read_cb, (void*)evarg);
	        evarg->ev_r = ev_read;
            evarg->ev_w = ev_write;
            evarg->firstPack = 1;
            snprintf(evarg->url, URLSIZE, "%s", nextUrl);
            event_add(ev_write, 0);
            event_add(ev_read, 0);
            if (nextUrl != NULL)
            {
                free(nextUrl);
                nextUrl = NULL;
            }
        }
        event_base_dispatch(base);
	printf("\nTotal Numbers of Url:%d\n", count);
	if (count > 259) break;	//Ju ti shi 151259
    }
    event_base_free(base);
    printf("----- Finish Requesting -----\n");
    return 0;
}

void dealWithUrl(char* curUrl, char* fatherUrl, map_t curUrlMap)
{
    int ret = bloom_check(&bf, curUrl);

    if (strlen(curUrl) != 0 && ret == 0) //bloom not contain curUrl
    {
        char* newUrl = (char*)malloc(URLSIZE * sizeof(char));
        snprintf(newUrl, URLSIZE, "%s", curUrl);

        bloom_add(&bf, newUrl);                                    
        enQueue(urlQueue, (void*)newUrl);
                                 
        fprintf(confile, "%s->%s\n", fatherUrl, newUrl);
    }
    else if (strlen(curUrl) != 0 && ret == 1) //bloom contain curUrl
    {
        int* findNo;
        int* tmpNo;
        int error1 = hashmap_get(allUrlMap, curUrl, (void**)&findNo);
        int error2 = hashmap_get(curUrlMap, curUrl, (void**)&tmpNo);
        if (error1 == MAP_OK && error2 == MAP_MISSING)
        {
            struct KEY_VALUE* kv_c = malloc(sizeof(struct KEY_VALUE));
            snprintf(kv_c->urlname, URLSIZE, "%s", curUrl);
            kv_c->urlid = *findNo;
            hashmap_put(curUrlMap, kv_c->urlname, &kv_c->urlid);
            fprintf(confile, "%s->%s\n", fatherUrl, kv_c->urlname);
        }
    }
    return;
}

void* dealWithFile()
{
    fprintf(outfile, "\n");
    char line[LINESIZE];
    char strFather[URLSIZE];
    char strChild[URLSIZE];
    while (fgets(line, LINESIZE, confile) != NULL)
    {
        memset(strFather, 0x0, sizeof(strFather));
        memset(strChild, 0x0, sizeof(strChild));
        int ret = sscanf(line, "%[^-]->%s", strFather, strChild);
        if (ret < 0)
            continue;

        int* fatherId;
        int* childId;
        int error1 = hashmap_get(allUrlMap, strFather, (void**)&fatherId);
        if (error1 != MAP_OK)
        {
            continue;
        }
        int error2 = hashmap_get(allUrlMap, strChild, (void**)&childId);
        if (error2 == MAP_OK && *fatherId != *childId)
        {
            fprintf(outfile, "%d %d\n", *fatherId, *childId);
        }

    }
}


void runCrawler(char* ip, int port, char* filename)
{
    printf(">>> runCrawler\n");
    outfile = fopen(filename, "w+");
    confile = fopen("tmp.txt", "w+");

    char *first = (char*)malloc(URLSIZE * sizeof(char));
    snprintf(first, URLSIZE, "%s", "http://news.qq.com/");

    urlQueue = (QueueType*)malloc(sizeof(QueueType));
    initQueue(urlQueue, 2500000);
    enQueue(urlQueue, (void*)first);

    initParse(first);
    
    void* arg = NULL;

    pool_init (MAX_Thread);
    char* args = (char*) malloc(sizeof (char) * strlen(ip));
    args = ip;
    int i;
    for (i = 0; i < MAX_Thread; i++)
    {
        pool_add_worker (initbase, args);
    }
	printf("Sleep for a sec \n\n\n");
	sleep(1);	//This is important


	pool_destroy ();
	



    //initbase(ip, port);
    //initbase(ip);
    fclose(confile);
    confile = fopen("tmp.txt", "r");
    printf("----- Writing Relations into url.txt... -----\n");
    dealWithFile();
    printf("----- url.txt Generation Success ----- \n");
    hashmap_free(allUrlMap);
    fclose(outfile);
    //fclose(logfile);
    fclose(confile);

}

//----------------------------Version 3: Rabin-Karp Match Start----------------------------

char* searchSubstr(char *pat, char *txt)
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
                //printf("Pattern found at index %d\n", i);
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
    return result;
}
//----------------------------Version 3: Rabin-Karp Match End----------------------------
