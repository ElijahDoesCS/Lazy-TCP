# Goals
- Serve web content through from the TUN device
- Implement TCP state handling
- Modify makefile to add CAP_NET_ADMIN to output binary
- Cook up some test TCP packets on scapy
- Create an analog interface on verilog if ur trynna get crazy

## ICMP
- Just handles echo responses
- Could handle service unreachable
- Could parse pertinent options if necessary

### TODO
- Refactor so that IP packet holds a union of icmp and tcp header
- Refactor to clean up code, and implement threading
- Implement minimal TCP stack
- Reimplement the v_device ip binding without calling system, i.e. via a or hooked python run script
- Respond on certain error conditions with ICMP destination unreachable

### Run Script
sudo ./tcp_less $(cat ./env/ip.txt) $(cat ./env/tun.txt) vb

### Packet Generation Script
sudo python3 ./packet_cooking/gen.py

### Notes on TCP
- Have to support control bits for SYN, ACK, FIN, RST control bits
- Have to support MSS (maximum segment size) for negotation at handshake
- Have to implement padding options such as nop and end of op
- Have to handle illegal option lengths as well
- Have to implement an unsigned window size
- Implement TCP timestamps and timing mechanisms
- Implement a retransmission queue
- Not handling options
- Have to update state on empty ACKs

### To see the web content
- Go to http://10.8.0.1 or the ip that you configured