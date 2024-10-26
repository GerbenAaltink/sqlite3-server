import asyncio
import argparse

from utro import Client

def main():
    while True:
        client = Client("http://127.0.0.1:8888")
        try:
            line = input("sql> ")
            if not line:
                continue
        
            resp = client.execute(line)
            if type(resp) == str:
                print(resp)
                continue
            if "error" in resp:
                print("error:",resp['error'])
                continue
            if not len(resp['columns']) == 1:
                for column in resp['columns']:
                    print(column,end="\t")
                print("")
            for row in resp['rows']:
                for column in row:
                    print(column,end="\t")
                print("")
        except KeyboardInterrupt:
            print("Exiting")
            break 

    parser = argparse.ArgumentParser(description="A sample CLI tool.")
    parser.add_argument("name", type=str, help="Your name")
    args = parser.parse_args()
    print(f"Hello, {args.name}!")

if __name__ == "__main__":
    main()