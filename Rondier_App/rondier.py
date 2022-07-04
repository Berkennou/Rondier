import socket

hote = "192.168.43.89"
port = 6565
size = 1024
format = "utf-8"

def main() : 
    
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect((hote, port))
    print("[Connected]")

    """Identification"""
    client.send("2".encode(format))

    
    data = client.recv(size).decode(format)
    print(f"[server] {data}")
    if data[0]=='D' : 
        client.send("OK I WILL GO\0".encode(format))

    while True : 
        data = client.recv(size).decode(format)
        print(f"[server]")
            
        if data[0]=='R' : 
            Rapport =input("Write Report : ")
            Rapport += "\0"
            client.send(Rapport.encode(format))
        data = client.recv(size).decode(format)
        
        
        if data[0]=='E' :
            break
    print (f"End of Round :) \n")
    
    client.close()        


    

    

if __name__ =="__main__" : 
    main()


