# Read from the file descriptor that describes events
#!/usr/bin/env python3
import sys

def main():
    fifo_path = "./tcp_less_log.fifo"
    
    try:
        with open(fifo_path, 'rb') as fifo:
            print(f"Listening on {fifo_path}...")
            
            while True:
                # Read data from FIFO
                data = fifo.read(1024)  # Read in chunks
                
                if not data:
                    # EOF - writer closed
                    print("FIFO closed by writer")
                    break
                
                # For now, just print raw bytes
                print(f"Received {len(data)} bytes: {data[:20]}...")  # First 20 bytes
                
    except KeyboardInterrupt:
        print("\nShutting down logger")
    except FileNotFoundError:
        print(f"Error: {fifo_path} not found. Make sure C program creates it first.")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == '__main__':
    main()