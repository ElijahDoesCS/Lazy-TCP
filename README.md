# Goals
- Parse the IP 
- Wrap the IP header in a structure and be able to print out the ip header contents.

- Handle ICMP echo requests

# functionality
provide the input an ip, a device name such as tun0, and any input in the third parameter enables verbose output

# Dump
=== Packet (84 bytes) ===
0000: 45 00 00 54 f1 7e 40 00  40 01 35 1a 0a 08 00 00  |E..T.~@.@.5.....|
0010: 0a 08 00 01 08 00 5b f2  00 0e 00 01 0b c0 76 69  |......[.......vi|
0020: 00 00 00 00 51 02 0a 00  00 00 00 00 10 11 12 13  |....Q...........|
0030: 14 15 16 17 18 19 1a 1b  1c 1d 1e 1f 20 21 22 23  |............ !"#|
0040: 24 25 26 27 28 29 2a 2b  2c 2d 2e 2f 30 31 32 33  |$%&'()*+,-./0123|
0050: 34 35 36 37                                       |4567|

Crafted an ICMP echo reply, sending...

=== Packet (84 bytes) ===
0000: 45 00 00 54 f1 7e 40 00  40 01 35 1a 0a 08 00 01  |E..T.~@.@.5.....|
0010: 0a 08 00 00 00 00 63 f2  00 0e 00 01 0b c0 76 69  |......c.......vi|
0020: 00 00 00 00 51 02 0a 00  00 00 00 00 10 11 12 13  |....Q...........|
0030: 14 15 16 17 18 19 1a 1b  1c 1d 1e 1f 20 21 22 23  |............ !"#|
0040: 24 25 26 27 28 29 2a 2b  2c 2d 2e 2f 30 31 32 33  |$%&'()*+,-./0123|
0050: 34 35 36 37                                       |4567|

================ IPv4 Header ================
 Version: 4 | IHL: 5 (20 bytes)
 ToS    : 0x00
 Length : 84 bytes
 ID     : 0xF17E (61822)
 Flags  : 0x2 (Res:0 DF:1 MF:0)
 Offset : 0
 TTL    : 64
 Proto  : 1 (0x01)
 Check  : 0x351A
 Source : 10.8.0.1
 Dest   : 10.8.0.0
=============================================
---------------- ICMP Header ----------------
 Type     : 0 (Echo Reply)
 Code     : 0
 Checksum : 0x63F2
 ID       : 14 (0x000E)
 Sequence : 1 (0x0001)
---------------------------------------------

=== Packet (48 bytes) ===
0000: 60 00 00 00 00 08 3a ff  fe 80 00 00 00 00 00 00  |`.....:.........|
0010: 0d e0 43 73 c3 4d 75 ba  ff 02 00 00 00 00 00 00  |..Cs.Mu.........|
0020: 00 00 00 00 00 00 00 02  85 00 f2 db 00 00 00 00  |................|

