#!/bin/bash
IP_OTHER=$1
IP_BLOCK=$2
#PASS='dees'

#Obtain root privileges
(( EUID != 0 )) && exec sudo -- "$0" "$@"

#Enable ufw
ufw enable

#Change input policy from 'DROP' to 'ACCEPT'
sed -i '/^DEFAULT_INPUT_POLICY=/s/DROP/ACCEPT/' /etc/default/ufw

echo 'Which firewall setting to apply?(1|2|3|4|5|all)'
read input

if [ "$input" = "1" ] || [ "$input" = "all" ]
then
	#Deny outgoing traffic from this machine to other machine by telnet (port 23)
	ufw deny out proto tcp to $IP_OTHER port 23
fi

if [ "$input" = "2" ] || [ "$input" = "all" ]
then
	#Deny incoming traffic from other machine to this machine by telnet (port 23)
	ufw deny in proto tcp from $IP_OTHER to any port 23
fi

if [ "$input" = "3" ] || [ "$input" = "all" ]
then
	#Deny outgoing traffic to facebook
	ufw deny out from any to $IP_BLOCK
fi

if [ "$input" = "4" ] || [ "$input" = "all" ]
then
	#Deny outgoing traffic through port number: 443
	ufw deny out 443/tcp
fi

if [ "$input" = "5" ] || [ "$input" = "all" ]
then
	#Deny all incoming and outgoing ICMP packets
	sed -i '/ufw-before-\(input\|output\|forward\).*icmp/s/ACCEPT/DROP/g' /etc/ufw/before.rules
	sed -i '/ufw6-before-\(input\|output\|forward\).*icmpv6/s/ACCEPT/DROP/g' /etc/ufw/before6.rules

	#Deny incoming ping packets only
	#sed -i '/ufw-before-input -p icmp --icmp-type echo-request/s/ACCEPT/DROP/g' /etc/ufw/before.rules
	#Deny forwarding ping packets only
	#sed -i '/ufw-before-forward -p icmp --icmp-type echo-request/s/ACCEPT/DROP/g' /etc/ufw/before.rules

	#Restart for some changes to take place
	ufw disable
	ufw enable
fi

#Print rules
ufw status numbered