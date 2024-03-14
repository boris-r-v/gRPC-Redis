Multy thread server and clients


the server is waiting for Redis server instance on localhost:6379

client always use 5 threads

one thread server, one connetion to redis
 sync_create takes 10271ms to create 50k records
async_create takes 10247ms to create 50k records

four thread, four connection to redfis
 sync_create takes 2950ms to create 50k records
async_create takes 2961ms to create 50k records


4-thread, one connection to redis
 sync_create takes 5179ms to create 50k records
async_create takes 5150ms to create 50k records
:

2-thread, one connection to redis
 sync_create takes 4782ms to create 50k records
async_create takes 4825ms to create 50k records


