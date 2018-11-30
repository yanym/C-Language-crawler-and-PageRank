#ifndef CRAWLER_H
#define CRAWLER_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
//#include <pthread.h>

#include <fcntl.h>
#include <event.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/http.h>
#include <event2/dns.h>
                                                                                                     
#include <sys/types.h>
#include <regex.h>
#include <memory.h>


#include "common.h"
#include "queue.h"
#include "connserver.h"
#include "bloomfilter.h"
#include "hashmap.h"

//void BFS(Url_No* next, const int port);
void initParse(char* strUrl);
//void* initbase(char* ip, int port);
void* initbase(void* ip);
void dealWithUrl(char* curUrl, char* fatherUrl, map_t curUrlMap);
void* dealWithFile();
void runCrawler(char* ip, int port, char* filename);

void read_cb(evutil_socket_t fd, short what, void *arg);
void write_cb(evutil_socket_t fd, short what, void *arg);
void time_cb(evutil_socket_t fd, short what, void *arg);
char* searchSubstr(char *pat, char *txt);
#endif
