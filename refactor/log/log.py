from scapy.layers.inet import IP, ICMP, TCP
from scapy.layers.inet6 import IPv6, ICMPv6ND_RS, ICMPv6ND_RA, ICMPv6ND_NS, ICMPv6ND_NA
import struct
import os
import sys
from datetime import datetime

MTU_SIZE = 1500

class Color:
    RESET = '\033[0m'
    BOLD = '\033[1m'
    RED = '\033[91m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    MAGENTA = '\033[95m'
    CYAN = '\033[96m'
    GRAY = '\033[90m'

def close(msg):
    print(f"\n\n{Color.YELLOW}[LOG]: Closing debugger...{Color.RESET}\n")
    sys.exit(msg)

def display_event(event_num, event_data):
    try:
        direction = event_data[0]
        timestamp_ns = struct.unpack('=Q', event_data[1:9])[0]
        tu_len = struct.unpack('=H', event_data[9:11])[0]
        if (tu_len == 0): 
            close("Server detached (Host sent shutdown)")

        packet_bytes = bytes(event_data[11:11+tu_len])
        pkt = IP(packet_bytes) if packet_bytes[0] >> 4 == 4 else IPv6(packet_bytes)
        
        dt = datetime.fromtimestamp(timestamp_ns / 1_000_000_000)
        time_str = dt.strftime('%H:%M:%S.%f')[:-3]
        
        dir_color, arrow, dir_text = (Color.GREEN, "←", "RECV") if direction == 1 else (Color.BLUE, "→", "SEND")
        
        print(f"\n{Color.BOLD}{Color.CYAN}{'='*80}{Color.RESET}")
        print(f"{Color.BOLD}Event #{event_num}{Color.RESET} | "
              f"{Color.GRAY}{time_str}{Color.RESET} | "
              f"{dir_color}{dir_text} {arrow}{Color.RESET} | "
              f"Length: {Color.MAGENTA}{tu_len}{Color.RESET} bytes")
        
        print(f"{Color.BOLD}{pkt.name}:{Color.RESET} "
              f"{pkt.src} → {pkt.dst} | "
              f"Protocol: {Color.YELLOW}{pkt.payload.name if pkt.payload else 'None'}{Color.RESET}")
        
        if ICMP in pkt:
            print(f"{Color.BOLD}ICMP:{Color.RESET} Type {pkt[ICMP].type} | Code {pkt[ICMP].code}")
        elif ICMPv6ND_RS in pkt:
            print(f"{Color.BOLD}ICMPv6:{Color.RESET} Router Solicitation")
        elif ICMPv6ND_RA in pkt:
            print(f"{Color.BOLD}ICMPv6:{Color.RESET} Router Advertisement")
        elif ICMPv6ND_NS in pkt:
            print(f"{Color.BOLD}ICMPv6:{Color.RESET} Neighbor Solicitation")
        elif ICMPv6ND_NA in pkt:
            print(f"{Color.BOLD}ICMPv6:{Color.RESET} Neighbor Advertisement")
        elif TCP in pkt:
            flags = pkt[TCP].flags
            print(f"{Color.BOLD}TCP:{Color.RESET} "
                  f"Port {pkt[TCP].sport} → {pkt[TCP].dport} | "
                  f"Flags: {Color.YELLOW}{flags}{Color.RESET}")
        else:
            for i in range(0, len(packet_bytes), 16):
                chunk = packet_bytes[i:i+16]
                hex_part = ' '.join(f'{b:02x}' for b in chunk)
                ascii_part = ''.join(chr(b) if 32 <= b < 127 else '.' for b in chunk)
                print(f"{Color.GRAY}{i:04x}  {hex_part:<47}  {ascii_part}{Color.RESET}")
                            
        print(f"{Color.GRAY}{pkt.summary()}{Color.RESET}")
        
    except Exception as e:
        print(f"{Color.RED}[ERROR]: {e}{Color.RESET}")

def main():
    with open("./env/fifo_end.txt") as f:
        fifo_path = f.read().strip()
    
    fd = os.open(fifo_path, os.O_RDONLY)
    event_size = 1 + 8 + 2 + MTU_SIZE
    event_num = 0
    
    try:
        while True:
            data = os.read(fd, event_size)
            if not data:
                break
            if len(data) == event_size:
                event_num += 1
                display_event(event_num, data)

        close("Server closed abruptly")
    except KeyboardInterrupt:
        close("Debug process recieved interrupt")
    finally:
        os.close(fd)

if __name__ == '__main__':
    main()