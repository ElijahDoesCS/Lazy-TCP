from scapy.layers.inet import IP, TCP
from scapy.sendrecv import send

tcp_flags = {
    "SYN": "S",
    "ACK": "A",
    "FIN": "F",
    "PSH+ACK": "PA",
    "RST": "R",
    "SYN+ACK": "SA",
    "FIN+ACK": "FA"
}

def get_env():
    with open("./env/ip_end.txt", "r") as f:
        ip_to = f.read().strip()
    with open("./env/ip_src.txt", "r") as f:
        ip_from = f.read().strip()

    print(f"Chosen IPs: src={ip_from} dst={ip_to}")
    return ip_from, ip_to

def generate_packet(ip_from, ip_to, flag):
    ip = IP(src=ip_from, dst=ip_to)
    tcp = TCP(
        sport=12345,
        dport=8080,
        flags=flag,
        seq=1000,
        ack=0      # correct default
    )
    return ip / tcp

def select_flag():
    print("Available flags:")
    for k in tcp_flags:
        print(" ", k)

    selection = input("Select flag: ").strip().upper()

    if selection not in tcp_flags:
        raise ValueError("Invalid TCP flag selection")

    print(f"Generating packet: {selection}")
    return tcp_flags[selection]

ip_from, ip_to = get_env()
flag = select_flag()
pkt = generate_packet(ip_from, ip_to, flag)
send(pkt, count=1)
