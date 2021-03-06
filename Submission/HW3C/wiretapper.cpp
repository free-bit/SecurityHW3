/*
 * wiretapper.cpp
 *
 * Sniffer example of TCP/IP packet capture using libpcap.
 * 
 * Version 0.1.1 (2005-07-05)
 * Copyright (c) 2005 The Tcpdump Group
 *
 * This software is intended to be used as a practical example and 
 * demonstration of the libpcap library; available at:
 * http://www.tcpdump.org/
 *
 ****************************************************************************
 *
 * This software is a modification of Tim Carstens' "wiretapper.cpp"
 * demonstration source code, released as follows:
 * 
 * wiretapper.cpp
 * Copyright (c) 2002 Tim Carstens
 * 2002-01-07
 * Demonstration of using libpcap
 * timcarst -at- yahoo -dot- com
 * 
 * "wiretapper.cpp" is distributed under these terms:
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. The name "Tim Carstens" may not be used to endorse or promote
 *    products derived from this software without prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * <end of "wiretapper.cpp" terms>
 *
 * This software, "wiretapper.cpp", is a derivative work of "wiretapper.cpp" and is
 * covered by the following terms:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Because this is a derivative work, you must comply with the "wiretapper.cpp"
 *    terms reproduced above.
 * 2. Redistributions of source code must retain the Tcpdump Group copyright
 *    notice at the top of this source file, this list of conditions and the
 *    following disclaimer.
 * 3. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. The names "tcpdump" or "libpcap" may not be used to endorse or promote
 *    products derived from this software without prior written permission.
 *
 * THERE IS ABSOLUTELY NO WARRANTY FOR THIS PROGRAM.
 * BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY
 * FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN
 * OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES
 * PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED
 * OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS
 * TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE
 * PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,
 * REPAIR OR CORRECTION.
 * 
 * IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING
 * WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR
 * REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,
 * INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING
 * OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED
 * TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY
 * YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER
 * PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 * <end of "wiretapper.cpp" terms>
 * 
 ****************************************************************************
 *
 * Below is an excerpt from an email from Guy Harris on the tcpdump-workers
 * mail list when someone asked, "How do I get the length of the TCP
 * payload?" Guy Harris' slightly snipped response (edited by him to
 * speak of the IPv4 header length and TCP data offset without referring
 * to bitfield structure members) is reproduced below:
 * 
 * The Ethernet size is always 14 bytes.
 * 
 * <snip>...</snip>
 *
 * In fact, you *MUST* assume the Ethernet header is 14 bytes, *and*, if 
 * you're using structures, you must use structures where the members 
 * always have the same size on all platforms, because the sizes of the 
 * fields in Ethernet - and IP, and TCP, and... - headers are defined by 
 * the protocol specification, not by the way a particular platform's C 
 * compiler works.)
 *
 * The IP header size, in bytes, is the value of the IP header length,
 * as extracted from the "ip_vhl" field of "struct sniff_ip" with
 * the "IP_HL()" macro, times 4 ("times 4" because it's in units of
 * 4-byte words).  If that value is less than 20 - i.e., if the value
 * extracted with "IP_HL()" is less than 5 - you have a malformed
 * IP datagram.
 *
 * The TCP header size, in bytes, is the value of the TCP data offset,
 * as extracted from the "th_offx2" field of "struct sniff_tcp" with
 * the "TH_OFF()" macro, times 4 (for the same reason - 4-byte words).
 * If that value is less than 20 - i.e., if the value extracted with
 * "TH_OFF()" is less than 5 - you have a malformed TCP segment.
 *
 * So, to find the IP header in an Ethernet packet, look 14 bytes after 
 * the beginning of the packet data.  To find the TCP header, look 
 * "IP_HL(ip)*4" bytes after the beginning of the IP header.  To find the
 * TCP payload, look "TH_OFF(tcp)*4" bytes after the beginning of the TCP
 * header.
 * 
 * To find out how much payload there is:
 *
 * Take the IP *total* length field - "ip_len" in "struct sniff_ip" 
 * - and, first, check whether it's less than "IP_HL(ip)*4" (after
 * you've checked whether "IP_HL(ip)" is >= 5).  If it is, you have
 * a malformed IP datagram.
 *
 * Otherwise, subtract "IP_HL(ip)*4" from it; that gives you the length
 * of the TCP segment, including the TCP header.  If that's less than
 * "TH_OFF(tcp)*4" (after you've checked whether "TH_OFF(tcp)" is >= 5),
 * you have a malformed TCP segment.
 *
 * Otherwise, subtract "TH_OFF(tcp)*4" from it; that gives you the
 * length of the TCP payload.
 *
 * Note that you also need to make sure that you don't go past the end 
 * of the captured data in the packet - you might, for example, have a 
 * 15-byte Ethernet packet that claims to contain an IP datagram, but if 
 * it's 15 bytes, it has only one byte of Ethernet payload, which is too 
 * small for an IP header.  The length of the captured data is given in 
 * the "caplen" field in the "struct pcap_pkthdr"; it might be less than 
 * the length of the packet, if you're capturing with a snapshot length 
 * other than a value >= the maximum packet size.
 * <end of response>
 * 
 ****************************************************************************
 * 
 * Example compiler command-line for GCC:
 *   gcc -Wall -o wiretapper wiretapper.cpp -lpcap
 * 
 ****************************************************************************
 *
 * Code Comments
 *
 * This section contains additional information and explanations regarding
 * comments in the source code. It serves as documentaion and rationale
 * for why the code is written as it is without hindering readability, as it
 * might if it were placed along with the actual code inline. References in
 * the code appear as footnote notation (e.g. [1]).
 *
 * 1. Ethernet headers are always exactly 14 bytes, so we define this
 * explicitly with "#define". Since some compilers might pad structures to a
 * multiple of 4 bytes - some versions of GCC for ARM may do this -
 * "sizeof (struct sniff_ethernet)" isn't used.
 * 
 * 2. Check the link-layer type of the device that's being opened to make
 * sure it's Ethernet, since that's all we handle in this example. Other
 * link-layer types may have different length headers (see [1]).
 *
 * 3. This is the filter expression that tells libpcap which packets we're
 * interested in (i.e. which packets to capture). Since this source example
 * focuses on IP and TCP, we use the expression "ip", so we know we'll only
 * encounter IP packets. The capture filter syntax, along with some
 * examples, is documented in the tcpdump man page under "expression."
 * Below are a few simple examples:
 *
 * Expression     Description
 * ----------     -----------
 * ip         Capture all IP packets.
 * tcp          Capture only TCP packets.
 * tcp port 80      Capture only TCP packets with a port equal to 80.
 * ip host 10.1.2.3   Capture all IP packets to or from host 10.1.2.3.
 *
 ****************************************************************************
 *
 */

#define APP_NAME    "wiretapper"
#define APP_DESC    "Sniffer example using libpcap"
#define APP_COPYRIGHT "Copyright (c) 2005 The Tcpdump Group"
#define APP_DISCLAIMER  "THERE IS ABSOLUTELY NO WARRANTY FOR THIS PROGRAM."

#include <unistd.h>
#include <pcap.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <vector> 

using namespace std;

/* default snap length (maximum bytes per packet to capture) */
#define SNAP_LEN 1518

/* ethernet headers are always exactly 14 bytes [1] */
#define SIZE_ETHERNET 14

/* Ethernet addresses are 6 bytes */
#define ETHER_ADDR_LEN  6

/* Ethernet header */
struct sniff_ethernet {
        u_char  ether_dhost[ETHER_ADDR_LEN];    /* destination host address */
        u_char  ether_shost[ETHER_ADDR_LEN];    /* source host address */
        u_short ether_type;                     /* IP? ARP? RARP? etc */
};

/* IP header */
struct sniff_ip {
        u_char  ip_vhl;                 /* version << 4 | header length >> 2 */
        u_char  ip_tos;                 /* type of service */
        u_short ip_len;                 /* total length */
        u_short ip_id;                  /* identification */
        u_short ip_off;                 /* fragment offset field */
        #define IP_RF 0x8000            /* reserved fragment flag */
        #define IP_DF 0x4000            /* dont fragment flag */
        #define IP_MF 0x2000            /* more fragments flag */
        #define IP_OFFMASK 0x1fff       /* mask for fragmenting bits */
        u_char  ip_ttl;                 /* time to live */
        u_char  ip_p;                   /* protocol */
        u_short ip_sum;                 /* checksum */
        struct  in_addr ip_src,ip_dst;  /* source and dest address */
};
#define IP_HL(ip)               (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)                (((ip)->ip_vhl) >> 4)

/* TCP header */
typedef u_int tcp_seq;

struct sniff_tcp {
        u_short th_sport;               /* source port */
        u_short th_dport;               /* destination port */
        tcp_seq th_seq;                 /* sequence number */
        tcp_seq th_ack;                 /* acknowledgement number */
        u_char  th_offx2;               /* data offset, rsvd */
#define TH_OFF(th)      (((th)->th_offx2 & 0xf0) >> 4)
        u_char  th_flags;
        #define TH_FIN  0x01
        #define TH_SYN  0x02
        #define TH_RST  0x04
        #define TH_PUSH 0x08
        #define TH_ACK  0x10
        #define TH_URG  0x20
        #define TH_ECE  0x40
        #define TH_CWR  0x80
        #define TH_FLAGS        (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
        u_short th_win;                 /* window */
        u_short th_sum;                 /* checksum */
        u_short th_urp;                 /* urgent pointer */
};

void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);

void print_payload(const u_char *payload, int len);

void print_hex_ascii_line(const u_char *payload, int len, int offset);

void print_app_banner(void);

void print_app_usage(void);

/*
 * app name/banner
 */
void print_app_banner(void)
{

  printf("%s - %s\n", APP_NAME, APP_DESC);
  printf("%s\n", APP_COPYRIGHT);
  printf("%s\n", APP_DISCLAIMER);
  printf("\n");

return;
}

/*
 * print help text
 */
void print_app_usage(void)
{

  printf("Usage: %s [interface] [filter]\n", APP_NAME);
  printf("\n");
  printf("Options:\n");
  printf("=> [interface]\t Listen on given interface for packets.\n");
  printf("=> [filter]\t Filter expression for packets.\n");
  printf("\n");

return;
}

/*
 * print data in rows of 16 bytes: offset   hex   ascii
 *
 * 00000   47 45 54 20 2f 20 48 54  54 50 2f 31 2e 31 0d 0a   GET / HTTP/1.1..
 */
void print_hex_ascii_line(const u_char *payload, int len, int offset)
{

  int i;
  int gap;
  const u_char *ch;

  /* offset */
  printf("%05d   ", offset);
  
  /* hex */
  ch = payload;
  for(i = 0; i < len; i++) {
    printf("%02x ", *ch);
    ch++;
    /* print extra space after 8th byte for visual aid */
    if (i == 7)
      printf(" ");
  }
  /* print space to handle line less than 8 bytes */
  if (len < 8)
    printf(" ");
  
  /* fill hex gap with spaces if not full line */
  if (len < 16) {
    gap = 16 - len;
    for (i = 0; i < gap; i++) {
      printf("   ");
    }
  }
  printf("   ");
  
  /* ascii (if printable) */
  ch = payload;
  for(i = 0; i < len; i++) {
    if (isprint(*ch))
      printf("%c", *ch);
    else
      printf(".");
    ch++;
  }

  printf("\n");

return;
}

/*
 * print packet payload data (avoid printing binary data)
 */
void print_payload(const u_char *payload, int len)
{

  int len_rem = len;
  int line_width = 16;      /* number of bytes per line */
  int line_len;
  int offset = 0;         /* zero-based offset counter */
  const u_char *ch = payload;

  if (len <= 0)
    return;

  /* data fits on one line */
  if (len <= line_width) {
    print_hex_ascii_line(ch, len, offset);
    return;
  }

  /* data spans multiple lines */
  for ( ;; ) {
    /* compute current line length */
    line_len = line_width % len_rem;
    /* print line */
    print_hex_ascii_line(ch, line_len, offset);
    /* compute total remaining */
    len_rem = len_rem - line_len;
    /* shift pointer to remaining bytes to print */
    ch = ch + line_len;
    /* add offset */
    offset = offset + line_width;
    /* check if we have line width chars or less */
    if (len_rem <= line_width) {
      /* print last line and get out */
      print_hex_ascii_line(ch, len_rem, offset);
      break;
    }
  }

return;
}

/*
 * dissect/print packet
 */
void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{

  static int count = 1;                   /* packet counter */
  
  /* declare pointers to packet headers */
  const struct sniff_ethernet *ethernet;  /* The ethernet header [1] */
  const struct sniff_ip *ip;              /* The IP header */
  const struct sniff_tcp *tcp;            /* The TCP header */
  const unsigned char *payload;                    /* Packet payload */

  int size_ip;
  int size_tcp;
  int size_payload;
  
  printf("\nPacket number %d:\n", count);
  count++;
  
  /* define ethernet header */
  ethernet = (struct sniff_ethernet*)(packet);
  
  /* define/compute ip header offset */
  ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
  size_ip = IP_HL(ip)*4;
  if (size_ip < 20) {
    printf("*Invalid IP header length: %u bytes\n", size_ip);
    return;
  }

  /* print source and destination IP addresses */
  printf("Src IP: %s\n", inet_ntoa(ip->ip_src));
  printf("Dst IP: %s\n", inet_ntoa(ip->ip_dst));
  
  /* determine protocol */  
  switch(ip->ip_p) {
    case IPPROTO_TCP:
      printf("Protocol: TCP\n");
      break;
    case IPPROTO_UDP:
      printf("Protocol: UDP\n");
      return;
    case IPPROTO_ICMP:
      printf("Protocol: ICMP\n");
      return;
    case IPPROTO_IP:
      printf("Protocol: IP\n");
      return;
    default:
      printf("Protocol: Unknown\n");
      return;
  }
  
  /*
   *  OK, this packet is TCP.
   */
  
  /* define/compute tcp header offset */
  tcp = (struct sniff_tcp*)(packet + SIZE_ETHERNET + size_ip);
  size_tcp = TH_OFF(tcp)*4;
  if (size_tcp < 20) {
    printf("*Invalid TCP header length: %u bytes\n", size_tcp);
    return;
  }
  
  printf("Src port: %d\n", ntohs(tcp->th_sport));
  printf("Dst port: %d\n", ntohs(tcp->th_dport));
  
  /* define/compute tcp payload (segment) offset */
  payload = (u_char *)(packet + SIZE_ETHERNET + size_ip + size_tcp);
  
  /* compute tcp payload (segment) size */
  size_payload = ntohs(ip->ip_len) - (size_ip + size_tcp);
  
  /*
   * Print payload data; it might be binary, so don't just
   * treat it as a string.
   */
  if (size_payload > 0) {
    printf("Payload (%d bytes):\n", size_payload);
    print_payload(payload, size_payload);
  }
  else{
    printf("No payload!\n");
  }

return;
}

vector<string> parse_string_within_spaces(string str, int argc=1){
  vector<string> parsed_strs;
  string c;
  for(int i=argc; i>0; ){
    string parsed_str;
    bool parsing=false;

    for(int j=0; ; j++){
      
      c=str[j];

      if(!parsing && c==" "){
        continue;
      }
      else if(!parsing && c!=" "){
        parsing=true;
        parsed_str+=c;
      }
      else if(parsing && c!=" "){
        parsed_str+=c; 
      }
      else if(parsing && (c==" " || j==str.length()-1)){
        i--;
        if(i)
          str=str.substr(j+1, str.length()-j-1);
        break;
      }
    }
    parsed_strs.push_back(parsed_str);
  }
  return parsed_strs;
}

string build_filter(vector<string> &conditions){
  string filter;
  for(const auto & cond : conditions){
    filter+=cond+" and ";
  }
  if(filter.length()>=5)
    filter.resize(filter.size()-5);
  return filter;
}

int main(int argc, char **argv)
{

  string dev;                     /* capture device name */
  char errbuf[PCAP_ERRBUF_SIZE];  /* error buffer */
  pcap_t *handle;                 /* packet capture handle */

  string filter_exp;              /* filter expression [3] */
  struct bpf_program fp;          /* compiled filter program (expression) */
  bpf_u_int32 mask;               /* subnet mask */
  bpf_u_int32 net;                /* ip */
  int num_packets = 15;           /* number of packets to capture */

  //print_app_banner();

  auto uid = getuid();
  if (uid != 0){
    fprintf(stderr, "ERROR: Program requires root privileges!\n\n");
    exit(EXIT_FAILURE);
  }

  string arg_str;
  int len;
  /* gather and parse command line args */
  for(int i=1; i<argc; i++){
    arg_str+=" ";
    arg_str+=string(argv[i]);
  }
  len=arg_str.length();
  string flags[]={"--dev", "--tcp", "--icmp", "--src-ip", "--src-port", "--dst-ip", "--dst-port", "--sport-range", "--dport-range", "--count"};
  int i_dev            = arg_str.find(flags[0]),
      i_tcp            = arg_str.find(flags[1]),
      i_icmp           = arg_str.find(flags[2]),
      i_src_ip         = arg_str.find(flags[3]),
      i_src_port       = arg_str.find(flags[4]),
      i_dst_ip         = arg_str.find(flags[5]),
      i_dst_port       = arg_str.find(flags[6]),
      i_src_port_range = arg_str.find(flags[7]),
      i_dst_port_range = arg_str.find(flags[8]),
      i_count          = arg_str.find(flags[9]);

  string tcp, icmp,
         src_ip, src_port,
         dst_ip, dst_port,
         dst_port_srange, 
         dst_port_erange;

  if(i_dev!=string::npos){
    i_dev+=flags[0].length();
    dev=parse_string_within_spaces(arg_str.substr(i_dev, len-i_dev))[0];
  }
  /* find a capture device if not specified on command-line */
  else{
    dev = string(pcap_lookupdev(errbuf));
    if (dev.empty()) {
      fprintf(stderr, "Could not find default device: %s\n",
          errbuf);
      exit(EXIT_FAILURE);
    }
  }

  if(i_count!=string::npos){ 
    i_count+=flags[9].length();
    num_packets=stoi(parse_string_within_spaces(arg_str.substr(i_count, len-i_count))[0]);
  }

  string cond;
  vector<string> conditions;

  if(i_tcp!=string::npos)
    conditions.push_back("tcp");
  if(i_icmp!=string::npos)
    conditions.push_back("icmp");
  if(i_src_ip!=string::npos){
    i_src_ip+=flags[3].length();
    cond="src host "+parse_string_within_spaces(arg_str.substr(i_src_ip, len-i_src_ip))[0];
    conditions.push_back(cond);
  }
  if(i_src_port!=string::npos){
    i_src_port+=flags[4].length();
    cond="src port "+parse_string_within_spaces(arg_str.substr(i_src_port, len-i_src_port))[0];
    conditions.push_back(cond);
  }
  if(i_dst_ip!=string::npos){
    i_dst_ip+=flags[5].length();
    cond="dst host "+parse_string_within_spaces(arg_str.substr(i_dst_ip, len-i_dst_ip))[0];
    conditions.push_back(cond);
  }
  if(i_dst_port!=string::npos){
    i_dst_port+=flags[6].length();
    cond="dst port "+parse_string_within_spaces(arg_str.substr(i_dst_port, len-i_dst_port))[0];
    conditions.push_back(cond);
  }
  if(i_src_port_range!=string::npos){
    i_src_port_range+=flags[7].length();
    vector<string> args=parse_string_within_spaces(arg_str.substr(i_src_port_range, len-i_src_port_range), 2);
    cond="src portrange "+args[0]+"-"+args[1];
    conditions.push_back(cond);
  }
  if(i_dst_port_range!=string::npos){
    i_dst_port_range+=flags[8].length();
    vector<string> args=parse_string_within_spaces(arg_str.substr(i_dst_port_range, len-i_dst_port_range), 2);
    cond="dst portrange "+args[0]+"-"+args[1];
    conditions.push_back(cond);
  }

  filter_exp=build_filter(conditions);
  
  /* get network number and mask associated with capture device */
  if (pcap_lookupnet(dev.c_str(), &net, &mask, errbuf) == -1) {
    fprintf(stderr, "Could not get netmask for device %s: %s\n",
        dev.c_str(), errbuf);
    net = 0;
    mask = 0;
  }
  // pcap_lookupnet() is a function that, given the name of a device, returns one of its IPv4 network numbers and corresponding network mask 
  // ->(the network number is the IPv4 address ANDed with the network mask, so it contains only the network part of the address). 
  // ->This was essential because we needed to know the network mask in order to apply the filter.


  /* print capture info */
  printf("Device: %s\n", dev.c_str());
  printf("Number of packets: %d\n", num_packets);
  printf("Filter expression: %s\n", filter_exp.c_str());

  /* open capture device (in promiscuous mode) */
  // pcap_t *pcap_open_live(char *device, int snaplen, int promisc, int to_ms, char *ebuf)
  // "device" is the device that we specified in the previous section. 
  // "snaplen" is an integer which defines the maximum number of bytes to be captured by pcap. 
  // "promisc", when set to true, brings the interface into promiscuous mode 
  // ->(however, even if it is set to false, it is possible under specific cases for the interface to be in promiscuous mode, anyway). 
  // ->In standard, non-promiscuous sniffing, a host is sniffing only traffic that is directly related to it. Only traffic to, from, or routed through the host will be picked up by the sniffer. 
  // ->Promiscuous mode, on the other hand, sniffs all traffic on the wire. In a non-switched environment, this could be all network traffic.
  // "to_ms" is the read time out in milliseconds (a value of 0 means no time out; on at least some platforms, this means that you may wait until a sufficient number of packets arrive before seeing any packets, so you should use a non-zero timeout). 
  // "ebuf" is a string we can store any error messages within (as we did above with errbuf).
  // The function returns our session handler. 
  handle = pcap_open_live(dev.c_str(), SNAP_LEN, 1, 1000, errbuf);
  if (handle == NULL) {
    fprintf(stderr, "Could not open device %s: %s\n", dev.c_str(), errbuf);
    exit(EXIT_FAILURE);
  }
  // This code fragment opens the device stored in the strong "dev", 
  // tells it to read however many bytes are specified in BUFSIZ (which is defined in pcap.h). 
  // We are telling it to put the device into promiscuous mode, 
  // to sniff until an error occurs, and if there is an error, store it in the string errbuf; it uses that string to print an error message.


  /* make sure we're capturing on an Ethernet device [2] */
  if (pcap_datalink(handle) != DLT_EN10MB) {
    fprintf(stderr, "%s is not an Ethernet\n", dev.c_str());
    exit(EXIT_FAILURE);
  }

  /* compile the filter expression */
  if (pcap_compile(handle, &fp, filter_exp.c_str(), 0, net) == -1) {
    fprintf(stderr, "Could not parse filter %s: %s\n",
        filter_exp.c_str(), pcap_geterr(handle));
    exit(EXIT_FAILURE);
  }

  /* apply the compiled filter */
  if (pcap_setfilter(handle, &fp) == -1) {
    fprintf(stderr, "Could not install filter %s: %s\n",
        filter_exp.c_str(), pcap_geterr(handle));
    exit(EXIT_FAILURE);
  }

  /* now we can set our callback function */
  pcap_loop(handle, num_packets, got_packet, NULL);

  /* cleanup */
  pcap_freecode(&fp);
  pcap_close(handle);

  printf("\nCapture complete.\n");

  return 0;
}
/*
  //Determine link layer headers
  if (pcap_datalink(handle) != DLT_EN10MB) {
    fprintf(stderr, "Device %s doesn't provide Ethernet headers - not supported\n", dev);
    return(2);
  }
  //The pcap_datalink() routine returns a value indicating the type of link-layer headers

*/
// A callback function needs to be called every time a packet is sniffed that meets our filter requirements (if any filter exists, of course. If not, then all packets that are sniffed are sent to the callback.)
// The prototype for pcap_loop() is below:
// int pcap_loop(pcap_t *p, int cnt, pcap_handler callback, u_char *user)
// The first argument is our session handle. 
// Following that is an integer that tells pcap_loop() how many packets it should sniff for before returning (a negative value means it should sniff until an error occurs). 
// The third argument is the name of the callback function (just its identifier, no parentheses). 
// The last argument is useful in some applications, but many times is simply set as NULL. 
// Suppose we have arguments of our own that we wish to send to our callback function, in addition to the arguments that pcap_loop() sends. 
// ->This is where we do it. Obviously, you must typecast to a u_char pointer to ensure the results make it there correctly; as we will see later, pcap makes use of some very interesting means of passing information in the form of a u_char pointer. 
// ->After we show an example of how pcap does it, it should be obvious how to do it here. If not, consult your local C reference text, as an explanation of pointers is beyond the scope of this document. 
// ->pcap_dispatch() is almost identical in usage. The only difference between pcap_dispatch() and pcap_loop() is that pcap_dispatch() will only process the first batch of packets that it receives from the system, while pcap_loop() will continue processing packets or batches of packets until the count of packets runs out.

// void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);
// The first argument corresponds to the last argument of pcap_loop(). Whatever value is passed as the last argument to pcap_loop() is passed to the first argument of our callback function every time the function is called. 
// The second argument is the pcap header, which contains information about when the packet was sniffed, how large it is, etc. 
// The pcap_pkthdr structure is defined in pcap.h as:
// struct pcap_pkthdr {
//   struct timeval ts; /* time stamp */
//   bpf_u_int32 caplen; /* length of portion present */
//   bpf_u_int32 len; /* length this packet (off wire) */
// };
// The last argument is another pointer to a u_char, and it points to the first byte of a chunk of data containing the entire packet, as sniffed by pcap_loop(). 

