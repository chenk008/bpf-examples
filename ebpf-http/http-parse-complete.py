#!/usr/bin/python
#
#Bertrone Matteo - Polytechnic of Turin
#November 2015
#
#eBPF application that parses HTTP packets 
#and extracts (and prints on screen) the URL contained in the GET/POST request.
#
#eBPF program http_filter is used as SOCKET_FILTER attached to eth0 interface.
#only packet of type ip and tcp containing HTTP GET/POST are returned to userspace, others dropped
#
#python script uses bcc BPF Compiler Collection by iovisor (https://github.com/iovisor/bcc)
#and prints on stdout the first line of the HTTP GET/POST request containing the url

from __future__ import print_function
import atexit
import resource
from bcc import BPF
from ctypes import *
from struct import *
from sys import argv

import sys
import socket
import os
import struct
import binascii
import time
import traceback

CLEANUP_N_PACKETS  = 50       #run cleanup every CLEANUP_N_PACKETS packets received
MAX_URL_STRING_LEN = 8192     #max url string len (usually 8K)
MAX_AGE_SECONDS    = 30       #max age entry in bpf_sessions map

from signal import signal, SIGTERM

global target_interface
target_interface = "eth0"
global bpf

@atexit.register
def exit_handle():
  global bpf
  print('exit')
  if bpf is not None:
    print('clean xdp')
    bpf.remove_xdp(target_interface, 3)


# 使用signal捕获关闭信号，保证被kill时退出前执行
signal(SIGTERM, lambda signum, stack_frame: exit(1))

def getLocalIp():
  s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
  s.settimeout(0)
  try:
    s.connect(('10.254.254.254', 1))
    IP = s.getsockname()[0]
  except Exception:
    IP = '127.0.0.1'
  finally:
    s.close()
  return IP

#print str until CR+LF
def printUntilCRLF(str):
    for k in range (0,len(str)-1):
      if (str[k] == '\n'):
        if (str[k-1] == '\r'):
          print ("")
          return
      print ("%c" % (str[k]), end = "")
    print("")
    return  

#cleanup function
def cleanup(bpf_sessions):
    #get current time in seconds
    current_time = int(time.time())
    #looking for leaf having:
    #timestap  == 0        --> update with current timestamp
    #AGE > MAX_AGE_SECONDS --> delete item
    for key,leaf in bpf_sessions.items():
      try:
        current_leaf = bpf_sessions[key]
        #set timestamp if timestamp == 0
        if (current_leaf.timestamp == 0):
          bpf_sessions[key] = bpf_sessions.Leaf(current_time)
        else:
          #delete older entries
          if (current_time - current_leaf.timestamp > MAX_AGE_SECONDS):
            del bpf_sessions[key]
      except:
        print("cleanup exception.")
    return 

#args
def usage():
    print("USAGE: %s [-i <if_name>]" % argv[0])
    print("")
    print("Try '%s -h' for more options." % argv[0])
    exit()

#help
def help():
    print("USAGE: %s [-i <if_name>]" % argv[0])
    print("")
    print("optional arguments:")
    print("   -h                       print this help")
    print("   -i if_name               select interface if_name. Default is eth0")
    print("")
    print("examples:")
    print("    http-parse              # bind socket to eth0")
    print("    http-parse -i wlan0     # bind socket to wlan0")
    exit()

def main(argv):
  global target_interface
  global bpf
  #arguments

  if len(argv) == 2:
    if str(argv[1]) == '-h':
      help()
    else:
      usage()

  if len(argv) == 3:
    if str(argv[1]) == '-i':
      target_interface = argv[2]
    else:
      usage()

  if len(argv) > 3:
    usage()

  # save the original memlock limits
  memlock_limit = resource.getrlimit(resource.RLIMIT_MEMLOCK)

  # set a small RLIMIT_MEMLOCK limit
  resource.setrlimit(resource.RLIMIT_MEMLOCK, (4096, 4096))

  # initialize BPF - load source code from http-parse-complete.c
  # bpf = BPF(src_file = "http-parse-complete.c",debug = 0x4,cflags=["-I/usr/include"])
  bpf = BPF(src_file = "http-parse-complete.c",debug = 0,cflags=[""])

  xdp_fn = bpf.load_func("xdp_prog1", BPF.XDP, None)
  bpf.attach_xdp(target_interface, xdp_fn, 3)

  print ("binding socket to '%s'" % target_interface)
  ip = getLocalIp()
  print ("local ip to '%s'" % ip)

  #load eBPF program http_filter of type SOCKET_FILTER into the kernel eBPF vm
  #more info about eBPF program types
  #http://man7.org/linux/man-pages/man2/bpf.2.html
  function_http_filter = bpf.load_func("http_filter", BPF.SOCKET_FILTER)

  #create raw socket, bind it to interface
  #attach bpf program to socket created
  BPF.attach_raw_socket(function_http_filter, target_interface)

  #get file descriptor of the socket previously created inside BPF.attach_raw_socket
  socket_fd = function_http_filter.sock

  #create python socket object, from the file descriptor
  sock = socket.fromfd(socket_fd,socket.PF_PACKET,socket.SOCK_RAW,socket.IPPROTO_IP)
  #set it as blocking socket
  sock.setblocking(True)

  #get pointer to bpf map of type hash
  bpf_sessions = bpf.get_table("sessions")

  #packets counter
  packet_count = 0

  #dictionary containing association <key(ipsrc,ipdst,portsrc,portdst),payload_string>
  #if url is not entirely contained in only one packet, save the firt part of it in this local dict
  #when I find \r\n in a next pkt, append and print all the url
  local_dictionary = {}
  threshold_url = b"/latest/meta-data/region-id"
  threshold_cnt = 3

  while 1:
    #retrieve raw packet from socket
    packet_str = os.read(socket_fd,4096) #set packet length to max packet length on the interface
    packet_count += 1
    # print(packet_str)

    #DEBUG - print raw packet in hex format
    #packet_hex = toHex(packet_str)
    #print ("%s" % packet_hex)

    #convert packet into bytearray
    packet_bytearray = bytearray(packet_str)
    # print(packet_bytearray)
    
    #ethernet header length
    ETH_HLEN = 14 

    #IP HEADER
    #https://tools.ietf.org/html/rfc791
    # 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    # +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    # |Version|  IHL  |Type of Service|          Total Length         |
    # +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    #
    #IHL : Internet Header Length is the length of the internet header 
    #value to multiply * 4 byte
    #e.g. IHL = 5 ; IP Header Length = 5 * 4 byte = 20 byte
    #
    #Total length: This 16-bit field defines the entire packet size, 
    #including header and data, in bytes.

    #calculate packet total length
    total_length = packet_bytearray[ETH_HLEN + 2]               #load MSB
    total_length = total_length << 8                            #shift MSB
    total_length = total_length + packet_bytearray[ETH_HLEN+3]  #add LSB
    
    #calculate ip header length
    ip_header_length = packet_bytearray[ETH_HLEN]               #load Byte
    ip_header_length = ip_header_length & 0x0F                  #mask bits 0..3
    ip_header_length = ip_header_length << 2                    #shift to obtain length

    #retrieve ip source/dest
    ip_src_bytes = packet_str[ETH_HLEN+12:ETH_HLEN+16]                #ip source offset 12..15
    ip_dst_bytes = packet_str[ETH_HLEN+16:ETH_HLEN+20]                #ip dest   offset 16..19

    line4 = struct.unpack('>4s', ip_src_bytes)
    ip_src = socket.inet_ntoa(line4[0])
    # print("src_ip:{},{}".format(ip_src,ip_src == ip))
    need_filter=ip_src == ip

    line4 = struct.unpack('>4s', ip_dst_bytes)
    ip_dst = socket.inet_ntoa(line4[0])
    # print("ip_dst:{}".format(ip_dst))


    #TCP HEADER 
    #https://www.rfc-editor.org/rfc/rfc793.txt
    #  12              13              14              15  
    #  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 
    # +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    # |  Data |           |U|A|P|R|S|F|                               |
    # | Offset| Reserved  |R|C|S|S|Y|I|            Window             |
    # |       |           |G|K|H|T|N|N|                               |
    # +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    #
    #Data Offset: This indicates where the data begins.  
    #The TCP header is an integral number of 32 bits long.
    #value to multiply * 4 byte
    #e.g. DataOffset = 5 ; TCP Header Length = 5 * 4 byte = 20 byte

    #calculate tcp header length
    tcp_header_length = packet_bytearray[ETH_HLEN + ip_header_length + 12]  #load Byte
    tcp_header_length = tcp_header_length & 0xF0                            #mask bit 4..7
    tcp_header_length = tcp_header_length >> 2                              #SHR 4 ; SHL 2 -> SHR 2
    
    #retrieve port source/dest
    port_src_bytes = packet_str[ETH_HLEN+ip_header_length:ETH_HLEN+ip_header_length+2]
    port_dst_bytes = packet_str[ETH_HLEN+ip_header_length+2:ETH_HLEN+ip_header_length+4]
    

    #calculate payload offset
    payload_offset = ETH_HLEN + ip_header_length + tcp_header_length
    
    #payload_string contains only packet payload
    payload_string = packet_str[(payload_offset):(len(packet_bytearray))]

    
    #CR + LF (substring to find)
    crlf = b"\r\n"

    #current_Key contains ip source/dest and port source/map
    #useful for direct bpf_sessions map access
    current_Key = bpf_sessions.Key(int.from_bytes(ip_src_bytes, "big"),int.from_bytes(ip_dst_bytes, "big"),int.from_bytes(port_src_bytes, "big"),int.from_bytes(port_dst_bytes, "big"))

    # print("payload_string:{},{}".format(payload_string[:3], payload_string[:3] == b"GET"))

    if (ip_src != "100.100.100.200" and ip_dst!= "100.100.100.200"):
      continue

    localtime = time.asctime( time.localtime(time.time()) )
    print("{} header info, dst_ip:{},src_port:{},dst_ip:{}".format(localtime, ip_dst,int.from_bytes(port_src_bytes, "big"),int.from_bytes(ip_dst_bytes, "big")))
    # printUntilCRLF(payload_string)

    #looking for HTTP GET/POST request
    if ((payload_string[:3] == b"GET") or (payload_string[:4] == b"POST")   or (payload_string[:4] == b"HTTP") \
        or (payload_string[:3] == b"PUT") or (payload_string[:6] == b"DELETE") or (payload_string[:4] == b"HEAD") ):
      #match: HTTP GET/POST packet found
      # print("trigger payload_string:{}".format(payload_string))

      if (crlf in payload_string and threshold_url in payload_string):
        #url entirely contained in first packet -> print it all
        threshold_cnt -= 1

        #delete current_Key from bpf_sessions, url already printed. current session not useful anymore 
        if need_filter and threshold_cnt >= 0:
        #clean bpf_sessions & local_dictionary
          try:
            reverse_Key = bpf_sessions.Key(int.from_bytes(ip_dst_bytes, "big"),int.from_bytes(ip_src_bytes, "big"),int.from_bytes(port_dst_bytes, "big"),int.from_bytes(port_src_bytes, "big"))
            leaf = bpf_sessions.Leaf(2)
            bpf_sessions[reverse_Key]=leaf
            print ("inject error")
          except Exception as e:
            print ("error update map:{}".format(e))
            traceback.print_exc()
        else:
          try:
            del bpf_sessions[current_Key]
            # del local_dictionary[binascii.hexlify(current_Key)]
            threshold_cnt = 3
          except:
            print ("error deleting from map or dictionary")
            traceback.print_exc()
      else: 
        #url NOT entirely contained in first packet   
        #not found \r\n in payload. 
        #save current part of the payload_string in dictionary <key(ips,ipd,ports,portd),payload_string>
        local_dictionary[binascii.hexlify(current_Key)] = payload_string
    else:
      #NO match: HTTP GET/POST  NOT found

      #check if the packet belong to a session saved in bpf_sessions
      if (current_Key in bpf_sessions):
        #check id the packet belong to a session saved in local_dictionary 
        #(local_dictionary mantains HTTP GET/POST url not printed yet because splitted in N packets)
        if (binascii.hexlify(current_Key) in local_dictionary):
          #first part of the HTTP GET/POST url is already present in local dictionary (prev_payload_string)
          prev_payload_string = local_dictionary[binascii.hexlify(current_Key)]
          #looking for CR+LF in current packet. 
          if (crlf in payload_string):
            #last packet. containing last part of HTTP GET/POST url splitted in N packets.
            #append current payload
            prev_payload_string += payload_string
            #print HTTP GET/POST url 
            # printUntilCRLF(prev_payload_string)

            if need_filter:
            #clean bpf_sessions & local_dictionary
              try:
                reverse_Key = bpf_sessions.Key(int.from_bytes(port_src_bytes, "big"),int.from_bytes(port_dst_bytes, "big"),int.from_bytes(ip_src_bytes, "big"),int.from_bytes(ip_dst_bytes, "big"))
                bpf_sessions[reverse_Key]=1
                print("update map{}".format_map(ip_src))
              except:
                print ("error deleting from map or dictionary")
            else:
              try:
                del bpf_sessions[current_Key]
                del local_dictionary[binascii.hexlify(current_Key)]
              except:
                print ("error deleting from map or dictionary")
          else:
            #NOT last packet. containing part of HTTP GET/POST url splitted in N packets.
            #append current payload
            prev_payload_string += payload_string
            #check if not size exceeding (usually HTTP GET/POST url < 8K )
            if (len(prev_payload_string) > MAX_URL_STRING_LEN):
              print("url too long")
              try:
                del bpf_sessions[current_Key]
                del local_dictionary[binascii.hexlify(current_Key)]
              except:
                print ("error deleting from map or dict")
            #update dictionary
            local_dictionary[binascii.hexlify(current_Key)] = prev_payload_string  
        else:
          #first part of the HTTP GET/POST url is NOT present in local dictionary
          #bpf_sessions contains invalid entry -> delete it
          try:
            del bpf_sessions[current_Key]
          except:
            print ("error del bpf_session")

    #check if dirty entry are present in bpf_sessions
    if (((packet_count) % CLEANUP_N_PACKETS) == 0):
      cleanup(bpf_sessions)


if __name__ == "__main__":
    main(sys.argv)