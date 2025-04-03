# chee kit best teammate 2

####  port 8080
- for autoindexing + hyperlinks

####  port 8090
- for custom index file (just another random file din rly do anything abt it lol)
- for the index part if cant find first index then continue, if cant find all then do autoindexing, if its off then throw error

#### port 8100
- for unallowed method


#### Test server_name
```bash
curl --resolve abc:8090:127.0.0.1 http://abc:8090/
curl http://127.0.0.1:8090/
```