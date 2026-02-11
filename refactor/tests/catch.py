from scapy.layers.inet import IP, TCP
from scapy.sendrecv import send, sr1
from scapy.packet import Raw
from scapy.all import hexdump

def dump_packet(label, pkt):
    print("\n" + "=" * 70)
    print(label)
    print("=" * 70)

    print("\n[Summary]")
    print(pkt.summary())

    print("\n[Decoded headers]")
    pkt.show()

    print("\n[Raw bytes on wire]")
    hexdump(pkt)

    if pkt.haslayer(Raw):
        print("\n[Payload]")
        try:
            print(pkt[Raw].load.decode("utf-8", errors="replace"))
        except Exception:
            print(pkt[Raw].load)

    print("=" * 70)


class TCPClient:
    def __init__(self, src_ip, dst_ip, src_port=12345, dst_port=8080):
        self.src_ip = src_ip
        self.dst_ip = dst_ip
        self.src_port = src_port
        self.dst_port = dst_port
        self.my_seq = 1000
        self.their_seq = 0
        self.state = "CLOSED"

    def handshake(self):
        print("\n" + "=" * 60)
        print("STARTING HANDSHAKE")
        print("=" * 60)

        # 1. SYN
        syn = IP(src=self.src_ip, dst=self.dst_ip) / TCP(
            sport=self.src_port,
            dport=self.dst_port,
            flags="S",
            seq=self.my_seq
        )

        dump_packet("OUTGOING SYN", syn)
        response = sr1(syn, timeout=2, verbose=0)

        if not response:
            print("ERROR: No SYN-ACK received")
            return False

        dump_packet("INCOMING SYN-ACK", response)

        if response[TCP].flags != 0x12:
            print(f"ERROR: Expected SYN-ACK, got flags={response[TCP].flags}")
            return False

        self.their_seq = response[TCP].seq + 1
        self.my_seq = response[TCP].ack

        # 3. ACK
        ack = IP(src=self.src_ip, dst=self.dst_ip) / TCP(
            sport=self.src_port,
            dport=self.dst_port,
            flags="A",
            seq=self.my_seq,
            ack=self.their_seq
        )

        dump_packet("OUTGOING ACK", ack)
        send(ack, verbose=0)

        self.state = "ESTABLISHED"
        print("[OK] Connection ESTABLISHED")
        return True

    def send_data(self, data):
        if self.state != "ESTABLISHED":
            print(f"ERROR: Cannot send, state is {self.state}")
            return False

        pkt = IP(src=self.src_ip, dst=self.dst_ip) / TCP(
            sport=self.src_port,
            dport=self.dst_port,
            flags="PA",
            seq=self.my_seq,
            ack=self.their_seq
        ) / data

        dump_packet("OUTGOING DATA", pkt)
        response = sr1(pkt, timeout=2, verbose=0)

        if not response:
            print("WARNING: No response")
            return False

        dump_packet("INCOMING RESPONSE", response)

        # Advance our send sequence
        self.my_seq += len(data)

        # If peer sent data
        if response.haslayer(Raw):
            their_data = response[Raw].load
            self.their_seq += len(their_data)

        return True

    def close(self):
        if self.state != "ESTABLISHED":
            print(f"Cannot close, state is {self.state}")
            return False

        fin = IP(src=self.src_ip, dst=self.dst_ip) / TCP(
            sport=self.src_port,
            dport=self.dst_port,
            flags="FA",
            seq=self.my_seq,
            ack=self.their_seq
        )

        dump_packet("OUTGOING FIN", fin)
        response = sr1(fin, timeout=2, verbose=0)

        if not response:
            print("ERROR: No FIN-ACK received")
            return False

        dump_packet("INCOMING FIN-ACK", response)

        self.their_seq = response[TCP].seq + 1

        final_ack = IP(src=self.src_ip, dst=self.dst_ip) / TCP(
            sport=self.src_port,
            dport=self.dst_port,
            flags="A",
            seq=self.my_seq + 1,
            ack=self.their_seq
        )

        dump_packet("OUTGOING FINAL ACK", final_ack)
        send(final_ack, verbose=0)

        self.state = "CLOSED"
        print("[OK] Connection CLOSED")
        return True

    def menu(self):
        while True:
            print("\n" + "=" * 60)
            print(f"STATE: {self.state}")
            print("=" * 60)
            print("1) Handshake")
            print("2) Send data")
            print("3) Close")
            print("4) Exit")

            choice = input("\nChoice: ").strip()

            if choice == "1":
                self.handshake()
            elif choice == "2":
                data = input("Data to send: ").strip()
                self.send_data(data.encode())
            elif choice == "3":
                self.close()
            elif choice == "4":
                if self.state == "ESTABLISHED":
                    self.close()
                break
            else:
                print("Invalid choice")


def get_env():
    with open("./env/ip_req.txt", "r") as f:
        ip_to = f.read().strip()
    with open("./env/ip_end.txt", "r") as f:
        ip_from = f.read().strip()
    print(f"IPs: {ip_from} -> {ip_to}")
    return ip_from, ip_to


if __name__ == "__main__":
    ip_from, ip_to = get_env()
    client = TCPClient(ip_from, ip_to)
    client.menu()