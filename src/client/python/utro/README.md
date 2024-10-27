# Utro client

## Usage async
```python
import asyncio
from utro import Client, AsyncClient

async def main(url):
    async with AsyncClient(url,verbose=False,keep_alive=True) as client:
        json_obj = await client.execute("select * from sqlite_schema")
        print("Found",json_obj["count"],"schema objects")
        json_obj = await client.execute("create table test (a,b,c,d)")
        print("Create table success:", json_obj["success"])
        json_obj = await client.execute(
            "insert into test (a,b,c,d) VALUES (?,?,?,?)",
            1,
            "2",
            3.0,
            True
        )        

asyncio.run(main())
```
## Usage sync
```python
import asyncio
from utro import Client

with Client(url,url,verbose=False) as client:
    json_obj = client.execute("select * from sqlite_schema")
    print("Found",json_obj["count"],"schema objects")
    json_obj = client.execute("create table test (a,b,c,d)")
    print("Create table success:", json_obj["success"])
    json_obj = client.execute(
        "insert into test (a,b,c,d) VALUES (?,?,?,?)",
        1,
        "2",
        3.0,
        True
    )        
```
