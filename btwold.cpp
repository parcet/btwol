#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <cstdlib>
#include <ctime>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>

#include <time.h>
#include <unistd.h>

#define LEVEL_FATAL (0)
#define LEVEL_ERROR (1)
#define LEVEL_WARN  (2)
#define LEVEL_NOTE  (3)
#define LEVEL_DEBUG (4)
#define DEBUGLOG(LEVEL1, ARGS...) if(LEVEL1 <= debuglevel) printf(ARGS)

int debuglevel = LEVEL_ERROR;

void dump(const char* msg, const void* p1, int len)
{
    const char* c1 = (char*)p1;
    
    printf("%s\n", msg);   
    for (int i = 0; i<len; i++)
    {
        printf("#%d addr:%08x: %d %c\n", i, &c1[i], c1[i], c1[i]>' '? c1[i]:'.');
    }
    printf("\n");
}

void phone_enter(char** mac, int macs_count)
{
    for (int i = 0; i<macs_count; i++)    
    {    
        char s[128];
        sprintf(s, "wakeonlan %s &", mac[i]);
        DEBUGLOG(LEVEL_WARN, "%s %s\n", __FUNCTION__, s);
        system(s);
    } 
}

void phone_leave()
{
    DEBUGLOG(LEVEL_WARN, "%s\n", __FUNCTION__);
}
    

void usage ()
{
    printf("usage: phone_bt_addr sinal_threshold interval_short_ms interval_long_ms WOL_MAC_1 [WOL_MAC_2 [...]]\n");
    exit(1);
}

int open_device(const char* device_addr)
{
    bdaddr_t bdaddr;

    memset(&bdaddr, 0, sizeof(bdaddr));

    str2ba(device_addr, &bdaddr);

    int dev_id = hci_get_route(&bdaddr);

    if (dev_id < 0)
    {
        perror("No bluetooth adapters found found.");
        return -1;
    }

    int dd = hci_open_dev(dev_id);

    if (dd < 0)
    {
        perror("Failed to open device");
    }

    return dd;
}

void close_device(int dd)
{
    hci_close_dev(dd);
}

bool cmd_read_rssi(int dd, const char *my_phone_bt_addr, int8_t *rssi)
{
    const int    to     = 10000;
    const int    toc    = to;
    const int    tor    = to;
    const int    tod    = to;
    bdaddr_t bdaddr;
    uint16_t     handle = 0;
    uint8_t      role   = 0x01;
    unsigned int ptype  = HCI_DM1 | HCI_DM3 | HCI_DM5 | HCI_DH1 | HCI_DH3 | HCI_DH5;
    uint8_t      reason = HCI_OE_USER_ENDED_CONNECTION;
    bool         result = false;

    DEBUGLOG(LEVEL_DEBUG, "Addr:[%s]\n", my_phone_bt_addr);

    memset(&bdaddr, 0, sizeof(bdaddr));

    str2ba(my_phone_bt_addr, &bdaddr);

    if (hci_create_connection(dd, &bdaddr, htobs(ptype), htobs(0x0000), role, &handle, toc) >= 0)
    {
        if (hci_read_rssi(dd, htobs(handle), rssi, tor) >= 0)
        {
            result = true;
        }
        else
        {
            DEBUGLOG(LEVEL_WARN, "Read RSSI failed:%s", strerror(errno));
        }

        if (hci_disconnect(dd, htobs(handle), reason, tod) < 0)
        {
            DEBUGLOG(LEVEL_WARN, "Disconnect failed:%s", strerror(errno));
        }
    }
    else
    {
        DEBUGLOG(LEVEL_WARN, "Can't create connection:%s", strerror(errno));
    }

    DEBUGLOG(LEVEL_DEBUG, "RSSI: %d\n", *rssi);

    return result;
}


int main(int argc, const char* argv[])
{
    int interval_short   = 0;
    int interval_long    = 0;
    int signal_threshold = 0;
    const char* debuglevelenv = getenv("BTWOL_DEBUG_LEVEL");
    
    
    if (debuglevelenv  != NULL)
        debuglevel = atol(debuglevelenv);
    DEBUGLOG(LEVEL_FATAL, "debuglevel=%d\n", debuglevel);
    
    if (argc < 6)
        usage();

    interval_short   = atol(argv[3]);
    interval_long    = atol(argv[4]);
    signal_threshold = atol(argv[2]);

    if (interval_short == 0 || interval_long == 0 || signal_threshold == 0)
        usage();

    typedef char* mystring;
    int       counter                 = 0;
    int8_t    rssi                    = 0;
    int       interval                = interval_short;
    int       phone_visible_counter   = 0;
    int       phone_invisible_counter = 0;
    bool      phone_visible           = false;
    mystring* macs                    = NULL;
    int       macs_count              = argc -5;

    macs = (mystring*)malloc(macs_count * sizeof(mystring));
    
    if (macs == NULL)
        return 1;
        
    DEBUGLOG(LEVEL_WARN, "BT Phone mac   = %s\n", argv[1]);    
    for(int i = 0; i<macs_count;  i++)
    {
        
        macs[i] = (mystring)malloc((strlen(argv[5 +i]) +1) * sizeof(char));
    
        if (macs[i] == NULL)
            return 1;
       
        memcpy(macs[i], argv[5 +i], strlen(argv[5 +i]) +1);
        DEBUGLOG(LEVEL_WARN, "WOL PC macs[%d] = %s\n", i, macs[i]);
    }
    
    int dd = open_device(NULL);
        
    while (true)
    {     
        if (!cmd_read_rssi(dd, argv[1], &rssi))
        {
            rssi = -100;
        }
    
        if (rssi > signal_threshold)
        {
            phone_visible_counter++;
            phone_invisible_counter = 0;
        }
        else
        {
            phone_visible_counter = 0;
            phone_invisible_counter++;
        }
        
        DEBUGLOG(LEVEL_NOTE, "RSSI: %i visible:%d invisible:%d\n", rssi, phone_visible_counter, phone_invisible_counter);
        
        if (phone_visible_counter >= 3)
        {
            //phone visible stable
            if (phone_visible)
            {
            }
            else
            {
                interval = interval_long;
                phone_enter(macs, macs_count);
            }
            phone_visible = true;
        }   
        
        if (phone_invisible_counter >= 3)
        {
            //phone invisible stable
            if (phone_visible)
            {
                interval = interval_short;
                phone_leave();
            }
            else
            {                 
            }
            phone_visible = false;
        }   
                
        usleep(1000 * interval);            
    }
    close_device(dd);
    
    return 0;
}
