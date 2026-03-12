# Lazy TCP
- A userspace TCP implementation for web content hosting. 

## TODO & Problems
    - Idle connection sweep
    - Retransmission timeout
    - Close wait timeout
    - Buffered sending and recieving
    - MSS option
    - Push the TCB structure to the debug interface
    - Increment the IP identification field
    - Handle the fragment bit
    - Negotiate an MSS
    - Window scaling
    - SACK
    - Timestamps
    - Start rejecting connections when the number of a TCPs reaches some maximum
    - Timeouts on idle closed state blocks
    - Tests
        - Unit
        - Integration

## Working on
- Handle data sends
- Handle resets and the fin ack sequence

## Usage
- Start the server:
    ./tcp_less $(cat ./env/ip_end.txt) $(cat ./env/tun.txt) vb <--- (Optional third argument for verbose output)

- Kill the server instance:
    ./shutudown.sh

- Start the debug interface
    ./log/log.py

- Build Script (Compiles the binary with privileges for creating the interface)
    make ; make cap

- To keep the kernel from injecting resets on the default port address when running locally (requires root)
    Client side: 
        sudo iptables -A OUTPUT -p tcp --sport $(cat ./env/client_port.txt) --tcp-flags RST RST -j DROP
    Server side: 
        sudo iptables -A OUTPUT -p tcp --sport $(cat ./env/host_port.txt) --tcp-flags RST RST -j DROP

## Sample requests
- ping the host
    - ping -4 -c 1 10.8.0.2

- Curl the host
    - curl http://10.8.0.1:8080/

## Quirks
- If we request a teardown while we're waiting for this to connect, we get into deadlock until the debugger starts. 
  At which point, after sending a request the debugger immediately gets its shutdown
