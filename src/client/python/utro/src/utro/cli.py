import asyncio
import argparse
import readline
import atexit
import os
import time
import pathlib
import sys

from utro import Client

def prepare_autocomplete(url):
    with Client(url=url) as client:
        resp = client.execute("create table autocomplete(id integer primary key, line text, times integer)")
        
        resp = client.execute("create table stats(name text, value_int integer, value_text string)")
        
        if resp['success']:
            resp = client.execute("insert into stats(name, value_int) VALUES (?,?)", ["keys_pressed",0])

def get_key_presses(url):
    resp = Client(url).execute("select value_int from stats where name = ?",["keys_pressed"])
    return int(resp['rows'][0][0])

def update_key_presses(url,key_presses):
    with Client(url=url) as client:
        client.execute("update stats set value_int = value_int + ? where name = ?",[key_presses,"keys_pressed"]) 
    

def completer(text, state):
    options = [".verbose", ".exit",".repeat","python",".presses"]
    import subprocess
    process = subprocess.Popen(['bash', '-c', 'complete'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = process.communicate()
    for result in stdout.decode("utf-8").split("\n"):
        options.append(result)
    if pathlib.Path(os.getcwd()).joinpath(text.strip("./")).exists():
        for file in pathlib.Path(os.getcwd()).joinpath(text.strip("./")).glob("*"):
            options.append(text.lstrip("/") + file.name)
    for file in pathlib.Path(os.getcwd()).joinpath(text.lstrip("./")).glob("*"):
        options.append(text  + "/" + file.name)
    matches = [option for option in options if option.startswith(text)]
    return matches[state] if state < len(matches) else None

def is_query(line):
    line = line.lower().strip("\n").strip(' ')
    return line.startswith("insert") or line.startswith("create") or line.startswith("select") or line.startswith("show") or line.startswith("explain") or line.startswith("describe") or line.startswith("with") or line.startswith("update") or line.startswith("delete") or line.startswith("drop")
def main():

    history_file = os.path.expanduser("~/.utro_history")
    if os.path.exists(history_file):
        readline.read_history_file(history_file)
    atexit.register(readline.write_history_file, history_file)
    readline.set_completer(completer)
    readline.parse_and_bind("tab: complete")
    readline.parse_and_bind("Control-r: reverse-search-history")
    readline.parse_and_bind("Control-a: pony")
    readline.parse_and_bind("Control-l: clear-screen")
    parser = argparse.ArgumentParser(description="A sample CLI tool.")
    parser.add_argument("url", default="http://127.0.0.1:8888/", type=str, help="URL of backend server")
    parser.add_argument("--history", action="store_true", default="http://127.0.0.1:8888/", help="URL of backend server")
    parser.add_argument("--data")
    args = parser.parse_args()
    client = Client(args.url,keep_alive=True)
    if args.history and args.data:
        client.execute("create table history(id integer primary key autoincrement, data text)") 
        client.execute("insert into history(data) values(?)", [args.data])

    prepare_autocomplete(url=args.url)

    previous = None
    repeat = False 
    query_number = 0
    last_time = 0
    while True:
        try:
            if not repeat:
                line = input("sql> ")
            else:
                line = repeat 
            if not line:
                continue
            if not repeat:
                update_key_presses(url=args.url,key_presses=len(line))
            if line == ".python" or line == "python":
                line = sys.executable
            if line == ".presses":
                print("You did " + str(get_key_presses(url=args.url)) + " key presses")
                continue
            if line == ".time":
                print("Last command took f" + str(last_time) + "s")
                continue
            if line == ".exit":
                exit(0)
            if line == ".repeat":
                line = previous
                repeat = previous
            if line == ".schema":
                line = "select * from sqlite_schema"
            if line == ".verbose":
                client.verbose = not client.verbose
                print("verbose is now " + (client.verbose and "enabled" or "disabled"))
                previous = line 
                continue
            if not line:
                continue
    
            previous = line
            
            time_start = time.time()
            if not is_query(line):
                os.system(line)
                time_end = time.time()
                last_time = time_end - time_start
                continue
                
            
            resp = client.execute(line)
            query_number += 1
            time_end = time.time()
            last_time = time_end - time_start
            if type(resp) == str:
                print(resp)
                print(f"Query took f{time_end-time_start}s")
                continue
            if "error" in resp:
                print("error:",resp['error'])
                continue
            
            has_rows = len(resp.get('rows',[])) > 0
            if 'columns' in resp:
                column_index = 0
                for column in resp['columns']:
                    print(column,end="")
                    if client.verbose and has_rows:
                        print("<{}>".format(type(resp['rows'][0][column_index]).__name__,""),end="")
                    column_index += 1
                    print("\t",end="")
                print("")
            if 'rows' in resp:
                for row in resp['rows']:
                    for column in row:
                        print(column,end="\t")
                    print("")
            if 'rows' not in resp and 'columns' not in resp and resp.get('success'):
                print("success:",line)
            print(f"#{query_number} took {time_end-time_start}s")
            
        except KeyboardInterrupt:
            if repeat:
                repeat = None
                continue 
            break

    

if __name__ == "__main__":
    main()