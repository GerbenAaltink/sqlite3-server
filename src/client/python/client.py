from utro import AsyncClient as Client
import asyncio 
from bench import Bench
#src/client/python/.venv/lib/python3.12/site-packages/aiohttp/helpers.py




async def process_batch(worker_nr,amount):
    str = "a" * 2000
    async with Client("http://retoor2:8888",keep_alive=True) as client:
        #with Bench("Delete table\n"):
        #    print(await client.execute("drop table test"))
        with Bench("Create table\n"):
            print(await client.execute("create table test (arg1 integer, arg2 integer, arg3 text, arg4 float, str text)"));
        tasks = []
        with Bench("Generate tasks\n"):
            for i in range(amount):
                tasks.append(client.execute("insert into test (arg1,arg2,arg3,arg4,str) VALUES (?,?,?,?,?)", [worker_nr, i, f"w:{worker_nr}i:{i}", 1.0*(i*2),str]))
        with Bench("Execute tasks\n"):
            await asyncio.gather(*tasks)

        with Bench("Tijd ophalen alle records\n"):
            result = await client.execute("select * from test")
            print(result)
            print(f"There are {result['count']} records\n")
        

async def main():
    tasks = [
        process_batch(i,80000) for i in range(1)
    ]
    await asyncio.gather(*tasks)
   

def dummy(*args, **kwargs):
     pass 


with Bench("VOlledige apllicatie 1"):
    asyncio.run(main())

