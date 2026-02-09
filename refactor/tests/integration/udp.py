from scapy.all import IP, UDP, send

send(
    IP(dst="10.8.0.2") /
    UDP(dport=9999, sport=12345)
)