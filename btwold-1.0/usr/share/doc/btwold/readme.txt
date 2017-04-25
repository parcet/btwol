Configuration
=============
Please specify system parameters in /etc/default/btwol: 
1. Phone bluetooth MAC adress in variable PHONE_BT_MAC;
2. PC Ethernet MAC address you would like to wake up in PC_WOL_MAC;
3. Change BTWOL_ENABLED to 1;

Please execute: 
    service btwol start

Daemon sends wakeonlan command once if specified phone is observed for 15 seconds.
Next wakeonlan command will be send out if phone is observed for 15 seconds, but before was invisible for 3 minutes.
Target computer should be configured to listen for wakeonlan packets.

TODO
==== 
* a lot, this is only POC