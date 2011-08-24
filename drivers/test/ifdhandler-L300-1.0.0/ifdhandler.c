/*****************************************************************
/
/ File   :   ifdhandler.c
/ Author :   David Corcoran <corcoran@linuxnet.com>
/ Date   :   June 15, 2000
/ Purpose:   This provides reader specific low-level calls.
/            See http://www.linuxnet.com for more information.
/ License:   See file LICENSE
/
******************************************************************/

#include <pcscdefines.h>
#include <ifdhandler.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <plat_ioctl.h>

// For debug Macro
#ifndef PCSC_DEBUG
#define PCSC_DEBUG      1
#endif

#ifdef PCSC_DEBUG
#define print           printf
#else
#define print
#endif
// debug end

#define APDU_SIZE               4
#define MAX_APDU                512
#define NUM_SLOTS               1
#define SCARD_PROTOCOL_T0       0

/* local data */
static DEVICE_CAPABILITIES device_data = 
{
        "GHL SYSTEM",           /* Vendor_Name          */
        "GHL    ",              /* IFD_Type             */
        1,                      /* IFD_Version          */
        "L350",                 /* IFD_Serial           */
        0,                      /* IFD_Channel_ID       */
        1,                      /* Asynch_Supported T=0 */ 
        0,                      /* Default_Clock        */
        0,                      /* Max_Clock            */
        0,                      /* Default_Data_Rate    */
        0,                      /* Max_Data_Rate        */
        0,                      /* Max_IFSD             */
        0,                      /* Synch_Supported T=1  */
        1,                      /* Power_Mgmt           */
        0,                      /* Card_Auth_Devices    */
        0,                      /* User_Auth_Device     */
        0                       /* Mechanics_Supported  */
};

struct smartport_t 
{
        int fd; /* This is the file descriptor to be used for the device */
        DEVICE_CAPABILITIES device ; 
        PROTOCOL_OPTIONS protocol; 
};   
struct smartport_t SmartCard;

static ATRField ATRBuf; /* the buffer to receive ATR response from smart card */

RESPONSECODE IFDHCreateChannel ( DWORD Lun, DWORD Channel ) {

  /* Lun - Logical Unit Number, use this for multiple card slots 
     or multiple readers. 0x XXXX YYYY -  XXXX multiple readers,
     YYYY multiple slots. The resource manager will set these 
     automatically.  By default the resource manager loads a new
     instance of the driver so if your reader does not have more than
     one smartcard slot then ignore the Lun in all the functions.
     Future versions of PC/SC might support loading multiple readers
     through one instance of the driver in which XXXX would be important
     to implement if you want this.
  */
  
  /* Channel - Channel ID.  This is denoted by the following:
     0x000001 - /dev/pcsc/1
     0x000002 - /dev/pcsc/2
     0x000003 - /dev/pcsc/3
     
     USB readers may choose to ignore this parameter and query 
     the bus for the particular reader.
  */

  /* This function is required to open a communications channel to the 
     port listed by Channel.  For example, the first serial reader on COM1 would
     link to /dev/pcsc/1 which would be a sym link to /dev/ttyS0 on some machines
     This is used to help with intermachine independance.
     
     Once the channel is opened the reader must be in a state in which it is possible
     to query IFDHICCPresence() for card status.
 
     returns:

     IFD_SUCCESS
     IFD_COMMUNICATION_ERROR
  */

    print("\n**In handler driver call IFDHCreateChannel: Lun=%X Channel=%X\n", Lun, Channel);

    SmartCard.device = device_data;
    SmartCard.protocol.Protocol_Type = SCARD_PROTOCOL_T0;

    if(SmartCard.fd <= 0)
    {
      SmartCard.fd = open("/dev/sam", O_RDWR);
      if(SmartCard.fd < 0)
      {
        return IFD_COMMUNICATION_ERROR;
      }
    }

    return IFD_SUCCESS;
}

RESPONSECODE IFDHCloseChannel ( DWORD Lun ) {
  
  /* This function should close the reader communication channel
     for the particular reader.  Prior to closing the communication channel
     the reader should make sure the card is powered down and the terminal
     is also powered down.

     returns:

     IFD_SUCCESS
     IFD_COMMUNICATION_ERROR     
  */
  
    print("\n**In handler driver call IFDHCloseChannel Lun: %X\n", Lun);
    
    if(SmartCard.fd >= 0)
    {
        close(SmartCard.fd);
        SmartCard.fd= -1;
    } 
    
    return IFD_SUCCESS; 
}

RESPONSECODE IFDHGetCapabilities ( DWORD Lun, DWORD Tag, PDWORD Length, PUCHAR Value ) 
{
  
  /* This function should get the slot/card capabilities for a particular
     slot/card specified by Lun.  Again, if you have only 1 card slot and don't mind
     loading a new driver for each reader then ignore Lun.

     Tag - the tag for the information requested
         example: TAG_IFD_ATR - return the Atr and it's size (required).
         these tags are defined in ifdhandler.h

     Length - the length of the returned data
     Value  - the value of the data

     returns:
     
     IFD_SUCCESS
     IFD_ERROR_TAG
  */

    RESPONSECODE lRetVal;
    DWORD HighNibble;
    DWORD LowNibble;

    print("\n**In handler driver call IFDHGetCapabilities: Lun=%X, Tag=%08X\n", Lun, Tag);

    if (Value == 0)
    {
        print("**Null pointer exception\n"); 
        return IFD_ERROR_TAG;
    }

    Tag &= 0xFFFF;
    
    HighNibble = Tag >> 8;
    LowNibble = Tag - (HighNibble << 8);    /* Shift left and subtract. */
    lRetVal = IFD_SUCCESS;

    if (HighNibble == 0x0F)
    {
        switch (LowNibble)
        {
            case 0xAE:  // Get slots count
                *Length = 1;
                *(char *)Value = NUM_SLOTS;
                print("**Return Slots count: %d", *(char *)Value);
                break;
            default: 
             print("** Doesn't support HighNibble: %02x LowNibble: %02x\n", HighNibble, LowNibble);
                return IFD_ERROR_TAG;
        }
    }
    else if(HighNibble == 0x02)
    {
        switch (LowNibble)      // Get Protocol type
        {
            case 0x01:
                *Length = sizeof(DWORD);
                *(DWORD *) Value = SmartCard.protocol.Protocol_Type;
                printf("**Return T=0 protocol Support");
                break;

            default:
             printf("** Doesn't support HighNibble: %lx LowNibble: %lx\n", HighNibble, LowNibble);
                return IFD_ERROR_TAG;
        }
    }
    else if( HighNibble == 0x03 ) 
    {
        switch (LowNibble)      // Get Protocol type
        {
            case 0x03:
                memset((void *)(&ATRBuf), 0, sizeof(ATRField));
                if(ioctl(SmartCard.fd, SAM_CARD_STATUS, (unsigned long)(&ATRBuf)))
                {
                    print("**Get ATR error\n");
                    lRetVal = IFD_COMMUNICATION_ERROR;
                    return IFD_ERROR_TAG;
                } 

                int i = 0;
                
                *Length = ATRBuf.len;
                for(i=0; i<ATRBuf.len; i++)
                    Value[i] = ATRBuf.FieldBuf[i];

                printf("Return ATR Value: ");
                for(i=0; i<*Length; i++)
                    printf("%02x ", Value[i]);
                printf("\n");
                break;

            default:
             printf("** Doesn't support HighNibble: %lx LowNibble: %lx\n", HighNibble, LowNibble);
                return IFD_ERROR_TAG;
        }
    }
    else if( HighNibble == 0x01 ) 
    {
        switch(LowNibble) 
        {
            case 0x00:
                *Length = strlen(SmartCard.device.Vendor_Name);
                strcpy(Value,SmartCard.device.Vendor_Name);
                printf("Return Vendor Name: %s\n", Value);
                break;

            case 0x01:
                *Length = strlen(SmartCard.device.IFD_Type);
                strcpy(Value,SmartCard.device.IFD_Type);
                break; 
            
            case 0x02: 
                *Length = sizeof(SmartCard.device.IFD_Version);
                *(DWORD *) Value = SmartCard.device.IFD_Version;
                printf("**Return IFD_Version: %lu \n", *(DWORD *)Value);
                break;

            case 0x03: 
                *Length = strlen(SmartCard.device.IFD_Serial);
                strcpy(Value,SmartCard.device.IFD_Serial);
                break; 
            
            case 0x80:
                *Length=sizeof(char);
                *(char *)Value = NUM_SLOTS;
                break;

            default:
             printf("** Doesn't support HighNibble: %lx LowNibble: %lx\n", HighNibble, LowNibble);
                return IFD_ERROR_TAG;
        }
    }
    else
    {
         printf("** Doesn't support HighNibble: %lx\n", HighNibble);
         return IFD_ERROR_TAG;
    }

    print("\n");
    return lRetVal;
}

RESPONSECODE IFDHSetCapabilities ( DWORD Lun, DWORD Tag, 
			       DWORD Length, PUCHAR Value ) {

  /* This function should set the slot/card capabilities for a particular
     slot/card specified by Lun.  Again, if you have only 1 card slot and don't mind
     loading a new driver for each reader then ignore Lun.

     Tag - the tag for the information needing set

     Length - the length of the returned data
     Value  - the value of the data

     returns:
     
     IFD_SUCCESS
     IFD_ERROR_TAG
     IFD_ERROR_SET_FAILURE
     IFD_ERROR_VALUE_READ_ONLY
  */

    print("\n**In handler driver call IFDHSetCapabilities: Lun=%X, Tag=%08X\n\n", Lun, Tag);

    return IFD_SUCCESS;
}

RESPONSECODE IFDHSetProtocolParameters ( DWORD Lun, DWORD Protocol, 
				   UCHAR Flags, UCHAR PTS1,
				   UCHAR PTS2, UCHAR PTS3) {

  /* This function should set the PTS of a particular card/slot using
     the three PTS parameters sent

     Protocol  - 0 .... 14  T=0 .... T=14
     Flags     - Logical OR of possible values:
     IFD_NEGOTIATE_PTS1 IFD_NEGOTIATE_PTS2 IFD_NEGOTIATE_PTS3
     to determine which PTS values to negotiate.
     PTS1,PTS2,PTS3 - PTS Values.

     returns:

     IFD_SUCCESS
     IFD_ERROR_PTS_FAILURE
     IFD_COMMUNICATION_ERROR
     IFD_PROTOCOL_NOT_SUPPORTED
  */
  print("\n**In handler driver call IFDHSetProtocolParameters: %X %X %X %X %X %X\n", Lun, Protocol, Flags, PTS1, PTS2, PTS3);
    print("\n");
    return IFD_PROTOCOL_NOT_SUPPORTED;

}


RESPONSECODE IFDHPowerICC ( DWORD Lun, DWORD Action, 
			    PUCHAR Atr, PDWORD AtrLength ) {

  /* This function controls the power and reset signals of the smartcard reader
     at the particular reader/slot specified by Lun.

     Action - Action to be taken on the card.

     IFD_POWER_UP - Power and reset the card if not done so 
     (store the ATR and return it and it's length).
 
     IFD_POWER_DOWN - Power down the card if not done already 
     (Atr/AtrLength should be zero'd)
 
    IFD_RESET - Perform a quick reset on the card.  If the card is not powered
     power up the card.  (Store and return the Atr/Length)

     Atr - Answer to Reset of the card.  The driver is responsible for caching
     this value in case IFDHGetCapabilities is called requesting the ATR and it's
     length.  This should not exceed MAX_ATR_SIZE.

     AtrLength - Length of the Atr.  This should not exceed MAX_ATR_SIZE.

     Notes:

     Memory cards without an ATR should return IFD_SUCCESS on reset
     but the Atr should be zero'd and the length should be zero

     Reset errors should return zero for the AtrLength and return 
     IFD_ERROR_POWER_ACTION.

     returns:

     IFD_SUCCESS
     IFD_ERROR_POWER_ACTION
     IFD_COMMUNICATION_ERROR
     IFD_NOT_SUPPORTED
  */

    int         i = 0;
    RESPONSECODE lRetVal = IFD_SUCCESS;
    *AtrLength = 0;     // set it default as error

    print("\n**In handler driver call IFDHPowerICC with cmd=%ld\n", Action); 

    if(Atr == 0) 
    {
      print("**Null pointer Exception \n");
      lRetVal = IFD_ERROR_POWER_ACTION;
    }

  if(Action == IFD_POWER_DOWN) 
  {
      print("**ioctl on SAM_POWER_OFF in handler\n");
      if(ioctl(SmartCard.fd, SAM_POWER_OFF, 0) < 0) 
      {
          print("**Set IFD_POWER_OFF error\n");
          lRetVal = IFD_COMMUNICATION_ERROR;
          goto ret;
      }

      print("**Set IFD_POWER_OFF ok\n");
      goto ret;
  }
  else if(Action==IFD_POWER_UP) 
  {
      print("**Card SAM_POWER_UP in handler\n");
#if 1
      if(ioctl(SmartCard.fd, SAM_POWER_UP, 0)) 
      {
          print("**Set IFD_POWER_UP ERROR\n");
          lRetVal = IFD_COMMUNICATION_ERROR;
          goto ret;
      }
#endif

      memset((void *)(&ATRBuf), 0, sizeof(ATRField));
      if(ioctl(SmartCard.fd, SAM_CARD_STATUS, (unsigned long)(&ATRBuf)))
      {
          print("**Get ATR error\n");
          lRetVal = IFD_COMMUNICATION_ERROR;
          goto ret;
      }

      print("**Get ATR Length: %d\n", ATRBuf.len);
      *AtrLength = ATRBuf.len;
      for(i=0; i<ATRBuf.len; i++)
          Atr[i] = ATRBuf.FieldBuf[i];

  }
  else if(Action==IFD_RESET) 
  {
      print("**Card RESET\n");
//      return IFD_NOT_SUPPORTED; // Do nothing

#if 1
      if(ioctl(SmartCard.fd, SAM_CARD_RESET, 0)) 
      {
          print("**IFD_RESET (ioctl SAM_CARD_RESET) ERROR\n");
          lRetVal = IFD_COMMUNICATION_ERROR;
          goto ret;
      }
#endif
      printf("ioctl on SAM_CARD_RESET OK\n");

      memset((void *)(&ATRBuf), 0, sizeof(ATRField));
      if(ioctl(SmartCard.fd, SAM_CARD_STATUS, (unsigned long)(&ATRBuf)))
      {
          print("**Get ATR error\n");
          lRetVal = IFD_COMMUNICATION_ERROR;
          goto ret;
      }

      printf("ioctl on SAM_CARD_STATUS OK\n");

      *AtrLength = ATRBuf.len;
      print("**Get ATR[%d]:  ", ATRBuf.len);
      for(i=0; i<ATRBuf.len; i++)
      {
          print("%02x ", ATRBuf.FieldBuf[i]);
          Atr[i] = ATRBuf.FieldBuf[i];
      }
      print("\n");

  }

ret:
  print("** In handler driver IFDHPowerICC exit code: %lu\n\n", lRetVal);
  return lRetVal;
}

RESPONSECODE IFDHTransmitToICC_T0 ( int Lun,PUCHAR TxBuffer, DWORD TxLength,
                                    PUCHAR RxBuffer, PDWORD RxLength)
{
    RESPONSECODE rc = IFD_SUCCESS; 
    int         size = 0;
    
    if(RxBuffer == 0 ) 
    { 
        print("**RxBuffer null\n");
        return IFD_SUCCESS;
    }

    if(TxLength < 4 ) 
    { 
        RxBuffer[0]=67;
        RxBuffer[1]=0;
        *RxLength=2;
        print("**APDU Too Small \n");
        return IFD_COMMUNICATION_ERROR;
    } 
   
    if( TxLength > 5 )
    {
        if(!((TxLength == (TxBuffer[4]+5)) || (TxLength == (TxBuffer[4]+6))))
        {
       print("**LC(%d) field doesn't match with # of bytes to send(%d)\n",TxBuffer[5],TxLength-4); 
            RxBuffer[0]=67; 
            RxBuffer[1]=0;
            *RxLength=2;
            return IFD_COMMUNICATION_ERROR;
        }
    }

    size = write(SmartCard.fd, TxBuffer, TxLength);
    print("**Write %lu bytes APDU to SAM card with return %d\n", TxLength, size); 

//    if(write(SmartCard.fd, TxBuffer, TxLength) < 0)
    if(size < 0)
    {
        print("**Write APDU to SAM card failure.\n"); 
        *RxLength = 0;
        rc = IFD_COMMUNICATION_ERROR;
        goto RET;
    }


#if 1
    memset(RxBuffer, 0, *RxLength);
    size = read(SmartCard.fd, RxBuffer, *RxLength);
    if(size < 0)
    {
        print("**Get Reply For APDU from SAM card failure.\n"); 
        *RxLength = 0;
        rc = IFD_COMMUNICATION_ERROR;
    }
    else
    {
        *RxLength = size;
        print("**Get Reply %d bytes from SAM card: ", *RxLength);
        int m;
        for(m=0; m<size; m++)
            print("%02x ", RxBuffer[m]);
        print("\n");
    }
#endif

RET:
    return rc;
}


RESPONSECODE IFDHTransmitToICC ( DWORD Lun, SCARD_IO_HEADER SendPci, 
                                 PUCHAR TxBuffer, DWORD TxLength, 
				 PUCHAR RxBuffer, PDWORD RxLength, 
				 PSCARD_IO_HEADER RecvPci ) 
{
  /* This function performs an APDU exchange with the card/slot specified by
     Lun.  The driver is responsible for performing any protocol specific exchanges
     such as T=0/1 ... differences.  Calling this function will abstract all protocol
     differences.

     SendPci
     Protocol - 0, 1, .... 14
     Length   - Not used.

     TxBuffer - Transmit APDU example (0x00 0xA4 0x00 0x00 0x02 0x3F 0x00)
     TxLength - Length of this buffer.
     RxBuffer - Receive APDU example (0x61 0x14)
     RxLength - Length of the received APDU.  This function will be passed
     the size of the buffer of RxBuffer and this function is responsible for
     setting this to the length of the received APDU.  This should be ZERO
     on all errors.  The resource manager will take responsibility of zeroing
     out any temporary APDU buffers for security reasons.
  
     RecvPci
     Protocol - 0, 1, .... 14
     Length   - Not used.

     Notes:
     The driver is responsible for knowing what type of card it has.  If the current
     slot/card contains a memory card then this command should ignore the Protocol
     and use the MCT style commands for support for these style cards and transmit 
     them appropriately.  If your reader does not support memory cards or you don't
     want to then ignore this.

     RxLength should be set to zero on error.

     returns:
     
     IFD_SUCCESS
     IFD_COMMUNICATION_ERROR
     IFD_RESPONSE_TIMEOUT
     IFD_ICC_NOT_PRESENT
     IFD_PROTOCOL_NOT_SUPPORTED
  */


print("\n**In handler driver call IFDHTransmitToICC **\n");
    RESPONSECODE rc = IFD_SUCCESS;

#if 0
    if (ioctl(SmartCard.fd, SAM_CARD_RESET, 0))
    {
        printf("ioctl SAM_CARD_RESET failed\n"); 
        return  IFD_ICC_NOT_PRESENT;
    }
#endif


print("**Get APDU command %lu bytes: ", TxLength);
long j=0;
for(j=0; j<TxLength; j++)
    print("%02x ", TxBuffer[j]);
print("\n");

#if 1
    if ( SendPci.Protocol == SCARD_PROTOCOL_T0 ) 
    {
        rc = IFDHTransmitToICC_T0(Lun,TxBuffer,TxLength,RxBuffer,RxLength );
        memcpy(RecvPci,&SendPci,sizeof(RecvPci));
    }
    else
    {
        print("**Unsupported  protocol \n");  
        rc = IFD_PROTOCOL_NOT_SUPPORTED;
        memcpy(RecvPci,&SendPci,sizeof(RecvPci));
    }
#endif


print("**exit handler driver normally **\n\n");
    return rc;
}


RESPONSECODE IFDHControl ( DWORD Lun, PUCHAR TxBuffer, 
			 DWORD TxLength, PUCHAR RxBuffer, 
			 PDWORD RxLength ) {

  /* This function performs a data exchange with the reader (not the card)
     specified by Lun.  Here XXXX will only be used.
     It is responsible for abstracting functionality such as PIN pads,
     biometrics, LCD panels, etc.  You should follow the MCT, CTBCS 
     specifications for a list of accepted commands to implement.

     TxBuffer - Transmit data
     TxLength - Length of this buffer.
     RxBuffer - Receive data
     RxLength - Length of the received data.  This function will be passed
     the length of the buffer RxBuffer and it must set this to the length
     of the received data.

     Notes:
     RxLength should be zero on error.
  */
  print("\n**In handler driver call  IFDHControl return zero\n");
  print("\n");
  return 0;

}


RESPONSECODE IFDHICCPresence( DWORD Lun ) {

  /* This function returns the status of the card inserted in the 
     reader/slot specified by Lun.  It will return either:

     returns:
     IFD_ICC_PRESENT
     IFD_ICC_NOT_PRESENT
     IFD_COMMUNICATION_ERROR
  */ 

   int ret = -1;
   ret = ioctl(SmartCard.fd, SAM_PRESENT_DETECT, 0);
   if(CARD_PRESENT == ret)
           return IFD_ICC_PRESENT;
   else if(CARD_NOT_PRESENT == ret)
           return IFD_ICC_NOT_PRESENT;
   else
           return IFD_COMMUNICATION_ERROR;
}
