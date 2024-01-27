

/*
USBTMC Class
Original VID/PID: 0xF4ED/0xEE3A
Vendor: Siglent
Model: SAG1021
Serial: SAG1IAAC6R0048
VISA Resource name: USB0::0xF4ED::0xEE3A::SAG1IAAC6R0048::0::INSTR

SDS2504XPlus
VID/PID: 0xF4ED/0x1011
Vendor: Siglent
Model: SDS2504XPlus
Serial: SDS2PEEC6R0295
VISA Resource name: USB0::0xF4EC::0x1011::SDS2PEEC6R0295::INSTR



*/

#include <usb.h>
#include <fx2lib.h>
#include <fx2delay.h>
#include <fx2usb.h>
//#include <usbcdc.h>
#include <ctype.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "usb_tmc.h"
#include "FX2LPSerial.h"


#define BUSY_LED_PORT IOE
#define BUSY_LED_CTRL OEE
#define BUSY_LED_PIN  0x40

/* Device Descriptor */
usb_desc_device_c usb_device = {
  .bLength              = sizeof(struct usb_desc_device),
  .bDescriptorType      = USB_DESC_DEVICE,
  .bcdUSB               = 0x0200,
  .bDeviceClass         = USB_DEV_CLASS_PER_INTERFACE,
  .bDeviceSubClass      = USB_DEV_SUBCLASS_PER_INTERFACE,
  .bDeviceProtocol      = USB_DEV_PROTOCOL_PER_INTERFACE,
  .bMaxPacketSize0      = 64,
  .idVendor             = 0x1337,
  .idProduct            = 0x1021,
  .bcdDevice            = 0x0000,
  .iManufacturer        = 1,
  .iProduct             = 2,
  .iSerialNumber        = 3,
  .bNumConfigurations   = 1,
};

/* Interface Descriptor*/
usb_desc_interface_c usb_iface_tmc = {
  .bLength              = sizeof(struct usb_desc_interface),
  .bDescriptorType      = USB_DESC_INTERFACE,
  .bInterfaceNumber     = 0,
  .bAlternateSetting    = 0,
  .bNumEndpoints        = 2,
  .bInterfaceClass      = USB_IFACE_CLASS_APP_SPECIFIC,
  .bInterfaceSubClass   = 0x03, // USBTMC
  .bInterfaceProtocol   = 0x01, // USBTMC USB488
  .iInterface           = 0,
};

usb_desc_endpoint_c usb_endpoint_ep2_out = {
  .bLength              = sizeof(struct usb_desc_endpoint),
  .bDescriptorType      = USB_DESC_ENDPOINT,
  .bEndpointAddress     = 2,
  .bmAttributes         = USB_XFER_BULK,
  .wMaxPacketSize       = 64,
  .bInterval            = 0,
};

usb_desc_endpoint_c usb_endpoint_ep6_in = {
  .bLength              = sizeof(struct usb_desc_endpoint),
  .bDescriptorType      = USB_DESC_ENDPOINT,
  .bEndpointAddress     = 6|USB_DIR_IN,
  .bmAttributes         = USB_XFER_BULK,
  .wMaxPacketSize       = 64,
  .bInterval            = 0,
};

usb_configuration_c usb_config = {
  {
    .bLength              = sizeof(struct usb_desc_configuration),
    .bDescriptorType      = USB_DESC_CONFIGURATION,
    .bNumInterfaces       = 1,
    .bConfigurationValue  = 1,
    .iConfiguration       = 0,
    .bmAttributes         = USB_ATTR_RESERVED_1,
    .bMaxPower            = 250,
  },
  
  {
    { .interface = &usb_iface_tmc },
    { .endpoint  = &usb_endpoint_ep2_out },
    { .endpoint  = &usb_endpoint_ep6_in },
    { 0 }
  }
};

usb_configuration_set_c usb_configs[] = {
  &usb_config,
};

usb_ascii_string_c usb_strings[] = {
  [0] = "dr@drlectro.com",
  [1] = "doctored SAG1021I",
  [2] = "000000",
};

usb_descriptor_set_c usb_descriptor_set = {
  .device           = &usb_device,
  .config_count     = ARRAYSIZE(usb_configs),
  .configs          = usb_configs,
  .string_count     = ARRAYSIZE(usb_strings),
  .strings          = usb_strings,
};



void toggle_busy_led() {
  BUSY_LED_PORT ^= BUSY_LED_PIN;
}

void set_busy_led(bool on) {
  if(on)
    BUSY_LED_PORT &= ~BUSY_LED_PIN; // Active low.
  else
    BUSY_LED_PORT |= BUSY_LED_PIN;
}

void init_busy_led() {
  BUSY_LED_CTRL |= BUSY_LED_PIN;
  set_busy_led(false);
}

// Handle USB setup requests.
void handle_usb_setup(__xdata struct usb_req_setup *req) {

  // Handle GET_CAPABILITIES request.
  if(req->bmRequestType == (USB_RECIP_IFACE|USB_TYPE_CLASS|USB_DIR_IN) &&
     req->bRequest == GET_CAPABILITIES &&
     req->wIndex == 0 && req->wLength == 24) {
    __xdata struct usb_tmc_req_capabilities *capabilities =
      (__xdata struct usb_tmc_req_capabilities *)EP0BUF;

    capabilities->bUSBTMC_status = STATUS_SUCCESS;
    capabilities->bRsv1 = 0;
    capabilities->wUSBTMCInterfaceVersion = 0x0100;
    capabilities->bInterfaceCapabilities = 0;
    capabilities->bDeviceCapabilities = USBTMC_DEV_CAPABILITY_TERMCHAR_SET;
    memset(capabilities->bRsv2, 0, sizeof(capabilities->bRsv2));
    
    capabilities->wUSB488InterfaceVersion = 0x0100;
    capabilities->bUSB488InterfaceCapabilities = USB488_IFC_CAPABILITY_TRIGGER_ACCEPTED |
                                                 USB488_IFC_CAPABILITY_REN_ACCEPTED |
                                                 USB488_IFC_CAPABILITY_IS_USB4882;
    capabilities->bUSB488DeviceCapabilities = USB488_DEV_CAPABILITY_DT1_CAPABLE |
                                              USB488_DEV_CAPABILITY_RL1_CAPABLE |
                                              USB488_DEV_CAPABILITY_SR1_CAPABLE |
                                              USB488_DEV_CAPABILITY_FULL_SCPI_CAPABLE;
    memset(capabilities->bRsv3, 0, sizeof(capabilities->bRsv3));

    SETUP_EP0_BUF(sizeof(struct usb_tmc_req_capabilities));
    return;
  }

  // Handdle REN_CONTROL request.
  if(req->bmRequestType == (USB_RECIP_IFACE|USB_TYPE_CLASS|USB_DIR_IN) &&
     req->bRequest == REN_CONTROL &&
     req->wIndex == 0 && req->wLength == 1) {
    EP0BUF[0] = 0x01;
    SETUP_EP0_BUF(1);
    return;
  }


  STALL_EP0();
}

uint16_t get_FPGA_register(uint8_t reg){
  uint16_t value = 0;

  // Initialize bus
  OEB = 0xFF; // D0-D7
  OED = 0xFF; // D8-D15
  OEC = 0x07; // AS, DS, WR
  IOC = 0x00;

  // Set Address
  IOB = reg; 
  IOD = 0x00;
  IOC = 1; 
  IOC = 0;

  // Perform read
  OEB = 0;
  OED = 0;
  IOC = 2; // DS

  value = IOD << 8;
  value |= IOB;

  // Relase bus
  IOC = 0;

  return value;
}

void set_FPGA_register(uint8_t reg, uint16_t value){
  // Initialize bus
  OEB = 0xFF; // D0-D7
  OED = 0xFF; // D8-D15
  OEC = 0x07; // AS, DS, WR
  IOC = 0x00;
  

  // Set Address
  IOB = reg; 
  IOD = 0x00;
  IOC = 1; 
  IOC = 0;

  // Perform write
  IOD = value >> 8;
  IOB = value & 0xFF;
  IOC = 4; // WR
  IOC = 6; // DS

  // Release bus
  IOC = 0;
  OEB = 0;
  OED = 0;
}

// Convert a decimal, octal or hex formatted text string to an integer.
int strtoi(char *s){
  int i = 0;
  int l = 0;
  int base = 10;

  while (isspace(*s)) s++; // Skip leading whitespace

  if(*s == '0') {
    s++;
    if(*s == 'x' || *s == 'X') {
      s++;
      base = 16;
    } else {
      base = 8;
    }
  }

  while(*s) {
    l = i;
    i *= base;
    if (base == 8){
      if(*s >= '0' && *s <= '7')
        i += *s - '0';
      else 
        return l; // Terminate on invalid character
    }
    else if (base == 10){
      if(*s >= '0' && *s <= '9')
        i += *s - '0';
      else 
        return l; // Terminate on invalid character
    }
    else if (base == 16){
      if(*s >= '0' && *s <= '9')
        i += *s - '0';
      else if(*s >= 'a' && *s <= 'f')
        i += *s - 'a' + 10;
      else if(*s >= 'A' && *s <= 'F')
        i += *s - 'A' + 10;
      else 
        return l; // Terminate on invalid character
    }
    else 
      return 0; // Should never get here
    
    s++;
  }
  return i;
}



/*
 SDS2504X / EasyWaveX Reference  messages and responses

 *IDN?  "Siglent Technologies,SDS2504X Plus,SDS2PEEC6R0295,5.3.1.3.9R10."
 PROD? "PROD MODEL,SDS2000X+,BAND,5MHZ."
 IDN-SGLT-AWG?  "Siglent Technologies, , , , "

 C1:WVDT FREQ,1000,AMPL,2,OFST,0,PHASE,0,WVNM,wave1,LENGTH,32768,WAVEDATA,...
 C1:ARWV NAME,wave1


*/

/*
 * Get the next parameter and value from a SCPI command string.
 * Returns the total length of the parameter and value, or -1 if we reached the end of the string.
 * 
 */
int getScpiParam(__xdata char *cmd, __xdata char **param, __xdata char **value){

  *param = cmd;
  *value = NULL;
  int n = 0;

  while (*cmd != 0)
  {
    if (*cmd == ','){
      *cmd = 0;
      if (*value == 0)
        *value = cmd+1;
      else  
        return n;
      *value = cmd+1;
    }
    cmd++;
    n++;
  } 

  if (*value == NULL)
    return -1;
  else
    return n;
}
void int16_to_hexstr(uint16_t value, char *str){
  uint8_t nibble;
  for (int i = 0; i < 4; i++){
    nibble = (value >> (12 - i*4)) & 0xF;
    if (nibble < 10)
      *str++ = nibble + '0';
    else
      *str++ = nibble - 10 + 'A';
  }
  *str = 0;
}

uint16_t msg_length = 0;

uint16_t process_scpi_command(__xdata char *cmd, __xdata uint16_t len, __xdata uint32_t xfer_size, char *resp){

  __xdata uint16_t resp_len = 0;
  __xdata uint32_t bytes_processed = len - 12; // Header is 12 bytes 
  uint16_t length = 0;

  __xdata char* start = cmd;
  __xdata char* param = NULL;
  __xdata char* value = NULL;

  int r;  

  *(cmd + bytes_processed) = 0; // Null terminate the block

  FX2LPSerial_XmitString("\r\nP: ");
  FX2LPSerial_XmitHex4(len);
  FX2LPSerial_XmitChar(' ');
  FX2LPSerial_XmitHex4(xfer_size);

  if(strncmp(cmd, "*IDN?", 5) == 0){
    // It seem EasyWaveX only cares about the contents of the model field, and SAG1021I is not a valid model.
    // The rest can be anything.
    strcpy(resp, "DrElectro,SAG1021,SN0,R0");
    resp_len = strlen(resp);
  }

  else if(strncmp(cmd, "PROD?", 5) == 0){
    // It seem EasyWaveX doesn't much care about the contents of each field, just that they are present.
    strcpy(resp, "PROD MODEL,SAG1021I,-,-");
    resp_len = strlen(resp);
  }

  else if(strncmp(cmd, "IDN-SGLT-AWG?", 12) == 0){
    strcpy(resp, "Siglent Technologies, , , , ");
    resp_len = strlen(resp);
  }

  /*
   * C1:WVDT FREQ,1000,AMPL,2,OFST,0,PHASE,0,WVNM,wave1,LENGTH,32768,WAVEDATA,...
   */
  else if(strncmp(cmd, "C1:WVDT ", 8) == 0){
    
    cmd += 8;

    do{
      r = getScpiParam(cmd, &param, &value);
      
      // We ran out of characters, so wait for more.
      if (r == -1){

        r = bytes_processed - (cmd - start);  // Number of bytes left in the buffer
        xmemcpy(scratch, cmd, r); // Copy the remaining bytes to the start of the scratch buffer

        EP2BCL = 0; // rearm EP2OUT
        while((EP2CS & _EMPTY));
        length = (EP2BCH << 8) | EP2BCL;

        /*
        FX2LPSerial_XmitString("\r\n more: ");
        FX2LPSerial_XmitString(" 0x");
        FX2LPSerial_XmitHex2(r);
        FX2LPSerial_XmitString(" 0x");
        FX2LPSerial_XmitHex2(length);
        */

        bytes_processed += length;

        cmd = (__xdata char *)scratch;
        xmemcpy(cmd + r, EP2FIFOBUF, length);
        //FX2LPSerial_XmitString("\r\n cmd: ");
        //FX2LPSerial_XmitString(cmd);

        length += r;
        *(cmd + length) = 0; // Null terminate the block

        r = getScpiParam(cmd, &param, &value);
      }

      cmd += r+1;

      FX2LPSerial_XmitString("\r\n pv: ");
      FX2LPSerial_XmitString(param);
      FX2LPSerial_XmitChar(' ');
      FX2LPSerial_XmitString(value);
      //FX2LPSerial_XmitChar(' 0x');
      //FX2LPSerial_XmitHex2(r);
    }
    while (r > 0 && strncmp(cmd, "WAVEDATA", 8) != 0);

    if (r > 0){
      cmd += 9;
      FX2LPSerial_XmitString("\r\n WAVEDATA: ");

      FX2LPSerial_XmitHex4(*cmd);
      FX2LPSerial_XmitHex4(*cmd+2);
    }

    FX2LPSerial_XmitString("\n");

    while (bytes_processed < xfer_size){
  
      EP2BCL = 0; // rearm EP2OUT
      while((EP2CS & _EMPTY));
      length = (EP2BCH << 8) | EP2BCL;

      bytes_processed += length;

      FX2LPSerial_XmitString("\r\n r: ");
      FX2LPSerial_XmitHex4(length);
      FX2LPSerial_XmitChar(' ');
      FX2LPSerial_XmitHex4(bytes_processed);

      for (int i = 0; i < length; i+=2){
        //set_FPGA_register(10, EP2FIFOBUF[i]);
      }

    }

    //sprintf (dbg, "cmd=%s tok=%s\r\n", cmd, token);
    //FX2LPSerial_XmitString(dbg);

    //token = strchr(token, ' ')+1;
    //token = strchr(token, ' ')+1;

  }


  else if(strncmp(cmd, "SET ", 4) == 0){
    char * token = strtok(cmd, " ");
    token = strtok(NULL, ",");

    uint16_t reg = strtoi(token);
    token = strtok(NULL, ",");
    uint16_t value = strtoi(token);

    set_FPGA_register(reg, value);

  }

  else if(strncmp(cmd, "GET ", 4) == 0){
    char * token = strchr(cmd, ' ')+1;

    uint16_t reg = strtoi(token);
    uint16_t value = get_FPGA_register(reg);

    int16_to_hexstr(value, resp);
    resp_len = strlen(resp);
  }

  else {
    //sprintf (dbg, "\r\nUnknown command=%s", cmd);
    //FX2LPSerial_XmitString(dbg);
  }

  if (bytes_processed < xfer_size){
    //sprintf (dbg, "\r\np %d x %d", bytes_processed, xfer_size);
    //FX2LPSerial_XmitString(dbg);
  }

  return resp_len;
}

uint16_t process_usb_tmc_msg(__xdata struct usb_tmc_msg_header *msg, uint16_t len) {

  uint16_t resp_length = 0;
  //char * dbg = (__xdata char *)scratch+256;

  // Handle USBTMC_MSGID_DEV_DEP_MSG_OUT.
  if(msg->bMsgID == USBTMC_MSGID_DEV_DEP_MSG_OUT) {
    __xdata struct usb_tmc_msg_dev_dep_msg_out *msg_hdr = (__xdata struct usb_tmc_msg_dev_dep_msg_out *)(msg+1);
   
  
    /*FX2LPSerial_XmitString("\r\nR: ");
    FX2LPSerial_XmitHex4(len);
    FX2LPSerial_XmitChar(' ');
    FX2LPSerial_XmitHex8(msg_hdr->wTransferSize);
    FX2LPSerial_XmitChar(' ');
    FX2LPSerial_XmitHex2(msg_hdr->bmTransferAttributes);
    FX2LPSerial_XmitChar(' ');*/


    msg_length = process_scpi_command((__xdata char *)(msg_hdr+1), len, msg_hdr->wTransferSize, (__xdata char *)scratch+64);
    return 0;
  }

  // Handle USBTMC_MSGID_REQUEST_DEV_DEP_MSG_IN.
  else if(msg->bMsgID == USBTMC_MSGID_REQUEST_DEV_DEP_MSG_IN) {

    __xdata struct usb_tmc_msg_header *msg_hdr = (__xdata struct usb_tmc_msg_header *)scratch;
    __xdata struct usb_tmc_msg_dev_dep_msg_in *resp_hdr = (__xdata struct usb_tmc_msg_dev_dep_msg_in *)(msg_hdr+1);
    __xdata uint8_t *resp_data = (uint8_t *)(resp_hdr+1);

    msg_hdr->bMsgID = USBTMC_MSGID_DEV_DEP_MSG_IN;
    msg_hdr->bTag = msg->bTag;
    msg_hdr->bTagInverse = ~msg->bTag;
    msg_hdr->bRsv = 0;

    resp_hdr->dwTransferSize = msg_length;
    resp_hdr->bTransferAttributes = USBTMC_EOM;
    resp_hdr->bRsv[0] = 0;
    resp_hdr->bRsv[1] = 0;
    resp_hdr->bRsv[2] = 0;

    xmemcpy(resp_data, scratch+64, msg_length);
    resp_length = sizeof(struct usb_tmc_msg_header) + sizeof(struct usb_tmc_msg_dev_dep_msg_in) + msg_length;
    msg_length = 0;
    return resp_length;
  }

  else {
    //FX2LPSerial_XmitString("\r\nUnknown Message ID: ");
    //FX2LPSerial_XmitHex8(msg->bMsgID);
  }

  return 0;
}


volatile bool pending_ep6_in;

void isr_IBN() __interrupt {
  pending_ep6_in = true;
  CLEAR_USB_IRQ();
  NAKIRQ = _IBN;
  IBNIRQ = _IBNI_EP6;
}

int main() {

  // Initialize the UART 
  // This also sets the CPU clock to 48MHz
  FX2LPSerial_Init();
  FX2LPSerial_XmitString("Doctored SAG1021i !\r\n");

  // Run core at 48 MHz fCLK.
  // CPUCS = _CLKSPD1;

  // Use newest chip features.
  REVCTL = _ENH_PKT|_DYN_OUT;

  // NAK all transfers.
  SYNCDELAY;
  FIFORESET = _NAKALL;

  // EP1IN is configured as INTERRUPT IN. (but presently disabled)
  EP1INCFG &= ~_VALID; // = _VALID|_TYPE1|_TYPE0;
  // EP1OUT is not used.
  EP1OUTCFG &= ~_VALID;

  // EP2 is configured as 512-byte double buffed BULK OUT.
  EP2CFG  =  _VALID|_TYPE1|_BUF1;
  EP2CS   = 0;
  // EP6 is configured as 512-byte double buffed BULK IN.
  EP6CFG  =  _VALID|_DIR|_TYPE1|_BUF1;
  EP6CS   = 0;
  // EP4/8 are not used.
  EP4CFG &= ~_VALID;
  EP8CFG &= ~_VALID;

  // Enable IN-BULK-NAK interrupt for EP6.
  IBNIE = _IBNI_EP6;
  NAKIE = _IBN;

  // Reset and prime EP2, and reset EP6.
  SYNCDELAY;
  FIFORESET = _NAKALL|2;
  SYNCDELAY;
  OUTPKTEND = _SKIP|2;
  SYNCDELAY;
  OUTPKTEND = _SKIP|2;
  SYNCDELAY;
  FIFORESET = _NAKALL|6;
  SYNCDELAY;
  FIFORESET = 0;

  init_busy_led();

  // Re-enumerate, to make sure our descriptors are picked up correctly.
  usb_init(/*disconnect=*/true);

  
  uint16_t length = 0;
  uint16_t resp_length = 0;
  //uint8_t cmd_buf[16];
  //uint8_t cmd_idx = 0;

  while(1) {
    
      // On receive data from host
      if(length == 0 && !(EP2CS & _EMPTY)) {
        length = (EP2BCH << 8) | EP2BCL;
        
        //FX2LPSerial_XmitString("\r\nR: ");
        //FX2LPSerial_XmitHex4(length);
        //FX2LPSerial_XmitChar(' ');
        //FX2LPSerial_XmitHex4(EP2CS);

        //EP2FIFOBUF[length] = 0; // Null terminate the request.
        resp_length = process_usb_tmc_msg((__xdata struct usb_tmc_msg_header *)EP2FIFOBUF, length);
        EP2BCL = 0; // rearm EP2OUT

        length = 0;
      }

      // If we have something to send to the host, send it.
      if(resp_length != 0 && pending_ep6_in) {
        xmemcpy(EP6FIFOBUF, scratch, resp_length);
        EP6BCH = resp_length >> 8;
        SYNCDELAY;
        EP6BCL = resp_length;

        resp_length = 0;
        pending_ep6_in = false;
      }
  }
}
