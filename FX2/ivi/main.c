

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
#include <usbcdc.h>
#include <ctype.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "usb_tmc.h"


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

// Convert a hex digit to an integer.
int hextoi(char *s){
  int i = 0;
  while(*s) {
    i <<= 4;
    if(*s >= '0' && *s <= '9')
      i |= *s - '0';
    else if(*s >= 'a' && *s <= 'f')
      i |= *s - 'a' + 10;
    else if(*s >= 'A' && *s <= 'F')
      i |= *s - 'A' + 10;
    else
      return 0;
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


uint16_t msg_length = 0;

uint16_t process_scpi_command(char *cmd, char *resp){
  //char *token = strtok(cmd, " ");

  char *token = cmd;

  if(strncmp(token, "*IDN?", 5) == 0){
    // It seem EasyWaveX only cares about the contents of the model field, and SAG1021I is not a valid model.
    // The rest can be anything.
    sprintf(resp, "DrElectro,SAG1021,SN0,R0");
    return strlen(resp);
  }

  if(strncmp(token, "PROD?", 5) == 0){
    // It seem EasyWaveX doesn't much care about the contents of each field, just that they are present.
    sprintf(resp, "PROD MODEL,SAG1021I,-,-");
    return strlen(resp);
  }

  if(strncmp(token, "IDN-SGLT-AWG?", 12) == 0){
    sprintf(resp, "Siglent Technologies, , , , ");
    return strlen(resp);
  }

  return 0;
}

uint16_t process_usb_tmc_msg(__xdata struct usb_tmc_msg_header *msg) {

  uint16_t resp_length = 0;

  // Handle USBTMC_MSGID_DEV_DEP_MSG_OUT.
  if(msg->bMsgID == USBTMC_MSGID_DEV_DEP_MSG_OUT) {
    __xdata struct usb_tmc_msg_dev_dep_msg_out *msg_hdr = (__xdata struct usb_tmc_msg_dev_dep_msg_out *)(msg+1);
    //toggle_busy_led();
    msg_length = process_scpi_command((__xdata char *)(msg_hdr+1), (__xdata char *)scratch+64);
    return 0;
  }

  // Handle USBTMC_MSGID_REQUEST_DEV_DEP_MSG_IN.
  if(msg->bMsgID == USBTMC_MSGID_REQUEST_DEV_DEP_MSG_IN) {

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

  return 0;
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


volatile bool pending_ep6_in;

void isr_IBN() __interrupt {
  pending_ep6_in = true;
  CLEAR_USB_IRQ();
  NAKIRQ = _IBN;
  IBNIRQ = _IBNI_EP6;
}

int main() {
  // Run core at 48 MHz fCLK.
  CPUCS = _CLKSPD1;

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
        //xmemcpy(scratch, EP2FIFOBUF, length);
        resp_length = process_usb_tmc_msg((__xdata struct usb_tmc_msg_header *)EP2FIFOBUF);
        EP2BCL = 0;

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