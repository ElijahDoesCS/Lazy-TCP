# shutdown.sh
#!/bin/bash
kill -USR1 $(pidof tcp_less)
echo "Initiating shutdown ~ Pinging host..."
ping -c 1 -4 $(cat ./env/ip_req.txt)