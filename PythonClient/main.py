import socket
import json

def main():
    host = "127.0.0.1"
    port = 8080

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
        client_socket.connect((host, port))

        message = json.dumps({"operation":"Multiply", "arg1":30, "arg2":20})
        client_socket.sendall(message.encode())

        receivedData = client_socket.recv(1024)
        response = json.loads(receivedData.decode())

        print(f"Response from server: {response['response']}")

if __name__ == "__main__":
    main()