import aiohttp 
import asyncio 
from bench import Bench

class Client:

    def __init__(self, url):
        self.url = url

    async def __aenter__(self):
        self.client = aiohttp.ClientSession()
        return self

    async def execute(self, query, params=None):
        result = None;
        async with self.client.post(self.url, json=dict(query=query,params=params or [])) as response:
             response = await response.json()
        print(response)
        return response;

    async def __aexit__(self, exc_type, exc, tb):
        await self.client.close()

async def do_call(session, arg):
    async with session.post('http://127.0.0.1:8888',json={"arg":arg}) as response:
            print(await response.text())
    
async def process_batch(worker_nr):
     async with aiohttp.ClientSession() as session:
        client = Client("http://127.0.0.1:8888")
        async with client:
            await client.execute("drop table test");
            await client.execute("create table test (arg1 integer, arg2 integer, arg3 text, arg4 integer)");
            tasks = []
            for i in range(5000):
                tasks.append(client.execute("insert into test (arg1,arg2,arg3,arg4) VALUES (?,?,?,?)", [worker_nr, i, f"w:{worker_nr}i:{i}", i*3]))
            tasks.append(client.execute("select * from test where arg1=? and arg2=? and arg3=? and arg4=?",  [worker_nr, i, f"w:{worker_nr}i:{i}", i*3]))
            await asyncio.gather(*tasks)

async def main():
    tasks = [
        process_batch(i) for i in range(1)
    ]
    await asyncio.gather(*tasks)
   

def dummy(*args, **kwargs):
     pass 

print = dummy 

with Bench("VOlledige apllicatie 1"):
    asyncio.run(main())

