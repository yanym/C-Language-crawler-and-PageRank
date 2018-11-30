## Crawler:
``` bash
mkdir build
cd build
cmake ..
make
./crawler 123.151.137.18 80 url.txt
``` bash  
   
## PageRank:
``` bash
g++ -o pagerank pagerank.c
./pagerank url.txt top10.txt
``` bash
