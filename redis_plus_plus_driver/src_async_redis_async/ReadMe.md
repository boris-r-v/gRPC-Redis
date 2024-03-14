Async gRPC and Redis server


the server is waiting for Redis server instance on localhost:6379

client always use 5 threads

CREATE NEW RECORDS
one thread server, 1 connetion to redis
sync_create takes 3180ms to create 50k records
WAS in syncRedis: 
 sync_create takes 10271ms to create 50k records

four thread, four connection to redfis
 sync_create takes 3398ms to create 50k records
async_create takes 3397ms to create 50k records 
WAS in syncRedis:
 sync_create takes 2950ms to create 50k records
async_create takes 2961ms to create 50k records

4-thread, 12 connection to redis
sync_create takes 3428ms to create 50k records


full async client and async gRPC and Redis

async_create2 takes 816ms to create 50k records
async_create2 takes 77ms to create 5k records
with 500k - laptop  stucked :)

