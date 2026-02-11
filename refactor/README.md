# TODO
- Implemeent my TCP refactor and build out timing mechanisms
    - Additionally implement a hash table for the TCBs
    - Idle connection sweep
    - Retransmission timeout
    - Buffered sending and recieving
    - Implement MSS option
- Handle IP options?

# Usage
- Start the server:
    ./tcp_less $(cat ./env/ip_end.txt) $(cat ./env/tun.txt) (Optional third argument for verbose output)
    ./tcp_less $(cat ./env/ip_end.txt) $(cat ./env/tun.txt) vb
- Kill the server instance:
    ./shutudown.sh

# Sample requests
- ping the host
    - ping -4 -c 1 10.8.0.2
- Curl the host
    - 

# Problems
- If we request a teardown while we're waiting for this to connect, we get into deadlock until the debugger starts. 
  At which point, after sending a request the debugger immediately gets its shutdown


// Get the  flags
    // Get the connection ID and see if we have a TCB for it
    // If the connection is null
        // If we get a rst, drop it
        // If we get just a SYN we create a new connection
        // Otherwise we send a reset ourselves
    // If the connection is open
        
        // If we got a reset, remove if seq no is valid
        // If we got a syn, resend our syn/ack
        // If it's an ack, move to established if ack is valid
        // 