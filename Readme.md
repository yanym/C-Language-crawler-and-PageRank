## Crawler:
``` bash
mkdir build
cd build
cmake ..
make
./crawler 123.151.137.18 80 url.txt
```  
   
## PageRank:
``` bash
g++ -o pagerank pagerank.c
./pagerank url.txt top10.txt
``` 
### Crawler instruction
We have two programs. The first is Crawler, and the other is PageRank.  
For Crawler, we get all URLs and filter them by BloomFilter. All URLs stored in url.txt. 
#### Note:
- You can see the Index for each URL and you will find many index pairs in the 2nd half of url.txt. That is, <Original Website Index, Out Website index>
- 123.151.137.18 80 is news.qq.com. If you want to modify it, you also need to check some fixed length variables defined in the program. You should also modify those parameters by analizing the URL. 

### PageRank instruction
If we have url.txt, exec PageRank to check the the ranking.  