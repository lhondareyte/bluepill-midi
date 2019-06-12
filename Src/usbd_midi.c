
#include "usbd_midi.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"

#include "usbd_def.h"
#include "usbd_core.h"

static uint8_t  USBD_MIDI_Init (USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_MIDI_DeInit (USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_MIDI_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_MIDI_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  *USBD_MIDI_GetCfgDesc (uint16_t *length);
static uint8_t  *USBD_MIDI_GetDeviceQualifierDesc (uint16_t *length);

uint32_t APP_Rx_ptr_in  = 0;
uint32_t APP_Rx_ptr_out = 0;
uint32_t APP_Rx_length  = 0;
uint8_t  USB_Tx_State = 0;

__ALIGN_BEGIN uint8_t USB_Rx_Buffer[MIDI_OUT_PACKET_SIZE] __ALIGN_END ;
__ALIGN_BEGIN uint8_t APP_Rx_Buffer[APP_RX_DATA_SIZE] __ALIGN_END ;

USBD_HandleTypeDef *pInstance = NULL;

USBD_ClassTypeDef  hUsbClassMIDI =
{
  USBD_MIDI_Init,
  USBD_MIDI_DeInit,
  NULL,
  NULL,
  NULL,
  USBD_MIDI_DataIn,
  USBD_MIDI_DataOut,
  NULL,
  NULL,
  NULL,
  USBD_MIDI_GetCfgDesc,
  USBD_MIDI_GetCfgDesc, 
  USBD_MIDI_GetCfgDesc,
  USBD_MIDI_GetDeviceQualifierDesc,
};

/* USB MIDI device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_MIDI_CfgDesc[USB_MIDI_CONFIG_DESC_SIZE] __ALIGN_END =
//		const uint8 CYCODE USBMIDI_1_DEVICE0_CONFIGURATION0_DESCR[165u] =
{
	/*  Config Descriptor Length               */ 0x09u,
	/*  DescriptorType: CONFIG                 */ 0x02u,
	/*  wTotalLength                           */ 0xA5u, 0x00u,
	/*  bNumInterfaces                         */ 0x02u,
	/*  bConfigurationValue                    */ 0x01u,
	/*  iConfiguration                         */ 0x01u,
	/*  bmAttributes                           */ 0x80u,
	/*  bMaxPower                              */ 0x32u,
	/*********************************************************************
	* AudioControl Interface Descriptor
	*********************************************************************/
	/*  Interface Descriptor Length            */ 0x09u,
	/*  DescriptorType: INTERFACE              */ 0x04u,
	/*  bInterfaceNumber                       */ 0x00u,
	/*  bAlternateSetting                      */ 0x00u,
	/*  bNumEndpoints                          */ 0x00u,
	/*  bInterfaceClass                        */ 0x01u,
	/*  bInterfaceSubClass                     */ 0x01u,
	/*  bInterfaceProtocol                     */ 0x00u,
	/*  iInterface                             */ 0x03u,
	/*********************************************************************
	* AC Header Descriptor
	*********************************************************************/
	/*  AC Header Descriptor Length            */ 0x09u,
	/*  DescriptorType: MIDI                  */ 0x24u,
	/*  bDescriptorSubtype                     */ 0x01u,
	/*  bcdADC                                 */ 0x00u, 0x01u,
	/*  wTotalLength                           */ 0x09u, 0x00u,
	/*  bInCollection                          */ 0x01u,
	/*  baInterfaceNr                          */ 0x01u,
	/*********************************************************************
	* MIDIStreaming Interface Descriptor
	*********************************************************************/
	/*  Interface Descriptor Length            */ 0x09u,
	/*  DescriptorType: INTERFACE              */ 0x04u,
	/*  bInterfaceNumber                       */ 0x01u,
	/*  bAlternateSetting                      */ 0x00u,
	/*  bNumEndpoints                          */ 0x02u,
	/*  bInterfaceClass                        */ 0x01u,
	/*  bInterfaceSubClass                     */ 0x03u,
	/*  bInterfaceProtocol                     */ 0x00u,
	/*  iInterface                             */ 0x04u,
	/*********************************************************************
	* MS Header Descriptor
	*********************************************************************/
	/*  MS Header Descriptor Length            */ 0x07u,
	/*  DescriptorType: MIDI                  */ 0x24u,
	/*  bDescriptorSubtype                     */ 0x01u,
	/*  bcdADC                                 */ 0x00u, 0x01u,
	/*  wTotalLength                           */ 0x61u, 0x00u,
	/*********************************************************************
	* MIDI IN Jack Descriptor
	*********************************************************************/
	/*  MIDI IN Jack Descriptor Length         */ 0x06u,
	/*  DescriptorType: MIDI                  */ 0x24u,
	/*  bDescriptorSubtype                     */ 0x02u,
	/*  bJackType                              */ 0x01u,
	/*  bJackID                                */ 0x01u,
	/*  iJack                                  */ 0x05u,
	/*********************************************************************
	* MIDI IN Jack Descriptor
	*********************************************************************/
	/*  MIDI IN Jack Descriptor Length         */ 0x06u,
	/*  DescriptorType: MIDI                  */ 0x24u,
	/*  bDescriptorSubtype                     */ 0x02u,
	/*  bJackType                              */ 0x02u,
	/*  bJackID                                */ 0x02u,
	/*  iJack                                  */ 0x05u,
	/*********************************************************************
	* MIDI OUT Jack Descriptor
	*********************************************************************/
	/*  MIDI OUT Jack Descriptor Length        */ 0x09u,
	/*  DescriptorType: MIDI                  */ 0x24u,
	/*  bDescriptorSubtype                     */ 0x03u,
	/*  bJackType                              */ 0x01u,
	/*  bJackID                                */ 0x03u,
	/*  bNrInputPins                           */ 0x01u,
	/*  baSourceID                             */ 0x02u,
	/*  baSourceID                             */ 0x01u,
	/*  iJack                                  */ 0x06u,
	/*********************************************************************
	* MIDI OUT Jack Descriptor
	*********************************************************************/
	/*  MIDI OUT Jack Descriptor Length        */ 0x09u,
	/*  DescriptorType: MIDI                  */ 0x24u,
	/*  bDescriptorSubtype                     */ 0x03u,
	/*  bJackType                              */ 0x02u,
	/*  bJackID                                */ 0x04u,
	/*  bNrInputPins                           */ 0x01u,
	/*  baSourceID                             */ 0x01u,
	/*  baSourceID                             */ 0x01u,
	/*  iJack                                  */ 0x06u,
	/*********************************************************************
	* MIDI IN Jack Descriptor
	*********************************************************************/
	/*  MIDI IN Jack Descriptor Length         */ 0x06u,
	/*  DescriptorType: MIDI                  */ 0x24u,
	/*  bDescriptorSubtype                     */ 0x02u,
	/*  bJackType                              */ 0x01u,
	/*  bJackID                                */ 0x05u,
	/*  iJack                                  */ 0x07u,
	/*********************************************************************
	* MIDI IN Jack Descriptor
	*********************************************************************/
	/*  MIDI IN Jack Descriptor Length         */ 0x06u,
	/*  DescriptorType: MIDI                  */ 0x24u,
	/*  bDescriptorSubtype                     */ 0x02u,
	/*  bJackType                              */ 0x02u,
	/*  bJackID                                */ 0x06u,
	/*  iJack                                  */ 0x07u,
	/*********************************************************************
	* MIDI OUT Jack Descriptor
	*********************************************************************/
	/*  MIDI OUT Jack Descriptor Length        */ 0x09u,
	/*  DescriptorType: MIDI                  */ 0x24u,
	/*  bDescriptorSubtype                     */ 0x03u,
	/*  bJackType                              */ 0x01u,
	/*  bJackID                                */ 0x07u,
	/*  bNrInputPins                           */ 0x01u,
	/*  baSourceID                             */ 0x06u,
	/*  baSourceID                             */ 0x01u,
	/*  iJack                                  */ 0x08u,
	/*********************************************************************
	* MIDI OUT Jack Descriptor
	*********************************************************************/
	/*  MIDI OUT Jack Descriptor Length        */ 0x09u,
	/*  DescriptorType: MIDI                  */ 0x24u,
	/*  bDescriptorSubtype                     */ 0x03u,
	/*  bJackType                              */ 0x02u,
	/*  bJackID                                */ 0x08u,
	/*  bNrInputPins                           */ 0x01u,
	/*  baSourceID                             */ 0x05u,
	/*  baSourceID                             */ 0x01u,
	/*  iJack                                  */ 0x08u,
	/*********************************************************************
	* MIDI IN Jack Descriptor
	*********************************************************************/
	/*  MIDI IN Jack Descriptor Length         */ 0x06u,
	/*  DescriptorType: MIDI                  */ 0x24u,
	/*  bDescriptorSubtype                     */ 0x02u,
	/*  bJackType                              */ 0x01u,
	/*  bJackID                                */ 0x09u,
	/*  iJack                                  */ 0x09u,
	/*********************************************************************
	* MIDI IN Jack Descriptor
	*********************************************************************/
	/*  MIDI IN Jack Descriptor Length         */ 0x06u,
	/*  DescriptorType: MIDI                  */ 0x24u,
	/*  bDescriptorSubtype                     */ 0x02u,
	/*  bJackType                              */ 0x02u,
	/*  bJackID                                */ 0x0Au,
	/*  iJack                                  */ 0x09u,
	/*********************************************************************
	* MIDI OUT Jack Descriptor
	*********************************************************************/
	/*  MIDI OUT Jack Descriptor Length        */ 0x09u,
	/*  DescriptorType: MIDI                  */ 0x24u,
	/*  bDescriptorSubtype                     */ 0x03u,
	/*  bJackType                              */ 0x01u,
	/*  bJackID                                */ 0x0Bu,
	/*  bNrInputPins                           */ 0x01u,
	/*  baSourceID                             */ 0x0Au,
	/*  baSourceID                             */ 0x01u,
	/*  iJack                                  */ 0x0Au,
	/*********************************************************************
	* MIDI OUT Jack Descriptor
	*********************************************************************/
	/*  MIDI OUT Jack Descriptor Length        */ 0x09u,
	/*  DescriptorType: MIDI                  */ 0x24u,
	/*  bDescriptorSubtype                     */ 0x03u,
	/*  bJackType                              */ 0x02u,
	/*  bJackID                                */ 0x0Cu,
	/*  bNrInputPins                           */ 0x01u,
	/*  baSourceID                             */ 0x09u,
	/*  baSourceID                             */ 0x01u,
	/*  iJack                                  */ 0x0Au,
	/*********************************************************************
	* Endpoint Descriptor
	*********************************************************************/
	/*  Endpoint Descriptor Length             */ 0x09u,
	/*  DescriptorType: ENDPOINT               */ 0x05u,
	/*  bEndpointAddress                       */ 0x81u,
	/*  bmAttributes                           */ 0x02u,
	/*  wMaxPacketSize                         */ 0x40u, 0x00u,
	/*  bInterval                              */ 0x00u,
	/*  bRefresh                               */ 0x00u,
	/*  bSynchAddress                          */ 0x00u,
	/*********************************************************************
	* MS Bulk Data Endpoint Descriptor
	*********************************************************************/
	/*  Endpoint Descriptor Length             */ 0x07u,
	/*  DescriptorType: CS_ENDPOINT            */ 0x25u,
	/*  bDescriptorSubtype                     */ 0x01u,
	/*  bNumEmbMIDIJack                        */ 0x03u,
	/*  baAssocJackID                          */ 0x03u, 0x07u, 0x0Bu,
	/*********************************************************************
	* Endpoint Descriptor
	*********************************************************************/
	/*  Endpoint Descriptor Length             */ 0x09u,
	/*  DescriptorType: ENDPOINT               */ 0x05u,
	/*  bEndpointAddress                       */ 0x01u,
	/*  bmAttributes                           */ 0x02u,
	/*  wMaxPacketSize                         */ 0x40u, 0x00u,
	/*  bInterval                              */ 0x00u,
	/*  bRefresh                               */ 0x00u,
	/*  bSynchAddress                          */ 0x00u,
	/*********************************************************************
	* MS Bulk Data Endpoint Descriptor
	*********************************************************************/
	/*  Endpoint Descriptor Length             */ 0x07u,
	/*  DescriptorType: CS_ENDPOINT            */ 0x25u,
	/*  bDescriptorSubtype                     */ 0x01u,
	/*  bNumEmbMIDIJack                        */ 0x03u,
	/*  baAssocJackID                          */ 0x01u, 0x05u, 0x09u
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_MIDI_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END=
{
	USB_LEN_DEV_QUALIFIER_DESC,
	USB_DESC_TYPE_DEVICE_QUALIFIER,
	0x00,
	0x02,
	0x00,
	0x00,
	0x00,
	0x40,
	0x01,
	0x00,
};


static uint8_t  USBD_MIDI_Init (USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	pInstance = pdev;
	USBD_LL_OpenEP(&hUsbDeviceFS, USB_EP_OUT_ADDR_1, USBD_EP_TYPE_BULK, MIDI_OUT_PACKET_SIZE);
	USBD_LL_OpenEP(&hUsbDeviceFS, USB_EP_IN_ADDR_1,  USBD_EP_TYPE_BULK, MIDI_OUT_PACKET_SIZE);
	USBD_LL_PrepareReceive(pdev, USB_EP_OUT_ADDR_1, (uint8_t*)(USB_Rx_Buffer), MIDI_OUT_PACKET_SIZE);
	return USBD_OK;
}

static uint8_t  USBD_MIDI_DeInit (USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
	pInstance = NULL;
	USBD_LL_CloseEP(pdev, USB_EP_OUT_ADDR_1);
	USBD_LL_CloseEP(pdev, USB_EP_IN_ADDR_1);
	return USBD_OK;
}

static uint8_t  USBD_MIDI_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
	USB_Tx_State = USB_Tx_State ? 0: 1;
	return USBD_OK;
}

static uint8_t  USBD_MIDI_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
	uint16_t USB_Rx_Cnt;

	USBD_MIDI_ItfTypeDef *pmidi;
	pmidi = (USBD_MIDI_ItfTypeDef *)(pdev->pUserData);

	USB_Rx_Cnt = ((PCD_HandleTypeDef*)pdev->pData)->OUT_ep[epnum].xfer_count;

	pmidi->RX((uint8_t *)&USB_Rx_Buffer, USB_Rx_Cnt);

	USBD_LL_PrepareReceive(pdev,USB_EP_OUT_ADDR_1,(uint8_t*)(USB_Rx_Buffer), MIDI_OUT_PACKET_SIZE);
	return USBD_OK;
}

static uint8_t MIDI_TX_Buffer  [MIDI_USB_RING_SIZE*MIDI_USB_MSG_SIZE]  = {0};
void USBD_MIDI_DumpRingBuffer ()
{
	uint16_t delta = MIDI_Message_Ring_Dump(MIDI_TX_Buffer);
	if (delta)
	{
	    USB_Tx_State = 1;
	    USBD_LL_Transmit (pInstance, USB_EP_IN_ADDR_1,
	    		MIDI_TX_Buffer, delta);
	}
}
/*
void USBD_MIDI_SendPacket (){
  uint16_t USB_Tx_ptr;
  uint16_t USB_Tx_length;

  if(USB_Tx_State != 1){
    if (APP_Rx_ptr_out == APP_RX_DATA_SIZE){
      APP_Rx_ptr_out = 0;
    }

    if(APP_Rx_ptr_out == APP_Rx_ptr_in){
      USB_Tx_State = 0;
      return;
    }

    if(APP_Rx_ptr_out > APP_Rx_ptr_in){
      APP_Rx_length = APP_RX_DATA_SIZE - APP_Rx_ptr_out;
    }else{
      APP_Rx_length = APP_Rx_ptr_in - APP_Rx_ptr_out;
    }

    if (APP_Rx_length > MIDI_DATA_IN_PACKET_SIZE){
      USB_Tx_ptr = APP_Rx_ptr_out;
      USB_Tx_length = MIDI_DATA_IN_PACKET_SIZE;
      APP_Rx_ptr_out += MIDI_DATA_IN_PACKET_SIZE;
      APP_Rx_length -= MIDI_DATA_IN_PACKET_SIZE;
    }else{
      USB_Tx_ptr = APP_Rx_ptr_out;
      USB_Tx_length = APP_Rx_length;
      APP_Rx_ptr_out += APP_Rx_length;
      APP_Rx_length = 0;
    }
    USB_Tx_State = 1;
    USBD_LL_Transmit (pInstance, USB_EP_IN_ADDR_1,(uint8_t*)&APP_Rx_Buffer[USB_Tx_ptr],USB_Tx_length);
  }
}
*/
// -----------------------------------------------------------------------------
static uint8_t  *USBD_MIDI_GetCfgDesc (uint16_t *length)
{
	*length = sizeof (USBD_MIDI_CfgDesc);
	return USBD_MIDI_CfgDesc;
}

static uint8_t  *USBD_MIDI_GetDeviceQualifierDesc (uint16_t *length)
{
  *length = sizeof (USBD_MIDI_DeviceQualifierDesc);
  return USBD_MIDI_DeviceQualifierDesc;
}

inline uint8_t  USBD_MIDI_RegisterInterface  (USBD_HandleTypeDef   *pdev,
                                        USBD_MIDI_ItfTypeDef *fops)
{
	if(fops != NULL)
	{
		pdev->pUserData= fops;
	}
	return 0;
}

