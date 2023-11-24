// Control Requests
#define INITIATE_ABORT_BULK_OUT 1
#define CHECK_ABORT_BULK_OUT_STATUS 2
#define INITIATE_ABORT_BULK_IN 3
#define CHECK_ABORT_BULK_IN_STATUS 4
#define INITIATE_CLEAR 5
#define CHECK_CLEAR_STATUS 6
#define GET_CAPABILITIES 7
#define INDICATOR_PULSE 64
#define READ_STATUS_BYTE 128
#define REN_CONTROL 160
#define GO_TO_LOCAL 161
#define LOCAL_LOCKOUT 162

// Statuses
#define STATUS_SUCCESS 1
#define STATUS_PENDING 2
#define STATUS_FAILED 128
#define STATUS_TRANSFER_NOT_IN_PROGRESS 129
#define STATUS_SPLIT_NOT_IN_PROGRESS 130
#define STATUS_SPLIT_IN_PROGRESS 131

// USBTMC Interface Capabilities
#define USBTMC_IFC_CAPABILITY_LISTEN_ONLY 1
#define USBTMC_IFC_CAPABILITY_TALK_ONLY 2
#define USBTMC_IFC_CAPABILITY_INDICATOR_PULSE 4

// USBTMC Device Capabilities
#define USBTMC_DEV_CAPABILITY_TERMCHAR_SET 1

// USB488 Interface Capabilities
#define USB488_IFC_CAPABILITY_TRIGGER_ACCEPTED 1
#define USB488_IFC_CAPABILITY_REN_ACCEPTED 2
#define USB488_IFC_CAPABILITY_IS_USB4882 4

// USB488 Device Capabilities
#define USB488_DEV_CAPABILITY_DT1_CAPABLE 1
#define USB488_DEV_CAPABILITY_RL1_CAPABLE 2
#define USB488_DEV_CAPABILITY_SR1_CAPABLE 4
#define USB488_DEV_CAPABILITY_FULL_SCPI_CAPABLE 8

// Capabilities message format
struct usb_tmc_req_capabilities {
  uint8_t bUSBTMC_status;
  uint8_t bRsv1;
  uint16_t wUSBTMCInterfaceVersion;
  uint8_t bInterfaceCapabilities;
  uint8_t bDeviceCapabilities;
  uint8_t bRsv2[6];
  uint16_t wUSB488InterfaceVersion;
  uint8_t bUSB488InterfaceCapabilities;
  uint8_t bUSB488DeviceCapabilities;
  uint8_t bRsv3[8];
};

// USBTMC message header format
struct usb_tmc_msg_header {
  uint8_t bMsgID;
  uint8_t bTag;
  uint8_t bTagInverse;
  uint8_t bRsv;
};

// USBTMC DEV_DEP_MSG_OUT format
struct usb_tmc_msg_dev_dep_msg_out {
  uint32_t wTransferSize;
  uint8_t bmTransferAttributes;
  uint8_t bRsv[3];
};

// USBTMC REQUEST_DEV_DEP_MSG_IN format
struct usb_tmc_msg_request_dev_dep_msg_in {
  uint32_t dwTransferSize;
  uint8_t bTransferAttributes;
  uint8_t TermChar;
  uint8_t bRsv[2];
};

// USBTMC DEV_DEP_MSG_IN format
struct usb_tmc_msg_dev_dep_msg_in {
  uint32_t dwTransferSize;
  uint8_t bTransferAttributes;
  uint8_t bRsv[3];
};

// USBTMC message types
#define USBTMC_MSGID_DEV_DEP_MSG_OUT 1
#define USBTMC_MSGID_REQUEST_DEV_DEP_MSG_IN 2
#define USBTMC_MSGID_DEV_DEP_MSG_IN 2
#define USBTMC_MSGID_VENDOR_SPECIFIC_OUT 126
#define USBTMC_MSGID_REQUEST_VENDOR_SPECIFIC_IN 127
#define USBTMC_MSGID_VENDOR_SPECIFIC_IN 127

// USBTMC transfer attributes
#define USBTMC_EOM 1
#define USBTMC_TERMCHAR 2
#define USBTMC_TERMCHAR_ENABLE 2

// USB488 message types
#define USB488_MSGID_TRIGGER 128

