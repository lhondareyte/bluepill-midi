
#ifndef __USB_MIDI_H
#define __USB_MIDI_H

#include <stdint.h>
#include "usbd_def.h"

#ifdef __cplusplus
 extern "C" {
#endif

#define MIDI_DATA_FS_MAX_PACKET_SIZE	(0x40)

#define MIDI_OUT_PACKET_SIZE			MIDI_DATA_FS_MAX_PACKET_SIZE
#define MIDI_IN_PACKET_SIZE				MIDI_DATA_FS_MAX_PACKET_SIZE
#define USB_MIDI_BUFFER_MAX_SIZE		MIDI_DATA_FS_MAX_PACKET_SIZE
#define MIDI_DATA_IN_PACKET_SIZE		MIDI_DATA_FS_MAX_PACKET_SIZE
#define MIDI_DATA_OUT_PACKET_SIZE		MIDI_DATA_FS_MAX_PACKET_SIZE

#define USB_EP_IN_ADDR_1	(0x81)
#define USB_EP_OUT_ADDR_1	(0x01)

#define APP_RX_DATA_SIZE               				((MIDI_OUT_PACKET_SIZE) * 4) //2048->256
#define USB_MIDI_CONFIG_DESC_SIZE	(165u)

typedef struct
{
	uint8_t dummy;
}USBD_MIDI_HandleTypeDef;

typedef struct
{
	uint16_t (*RX)	(uint8_t *msg, uint16_t length);
//	uint16_t (*TX)	(uint8_t *msg, uint16_t length);
}USBD_MIDI_ItfTypeDef;

uint8_t  USBD_MIDI_RegisterInterface (USBD_HandleTypeDef   *pdev,
								USBD_MIDI_ItfTypeDef *fops);

extern USBD_MIDI_ItfTypeDef 	hUsbClassMidi_CB;
extern USBD_HandleTypeDef 		hUsbDeviceFS;
extern USBD_MIDI_HandleTypeDef 	hUsbClassHandleMidi;
extern USBD_ClassTypeDef  		hUsbClassMIDI;

extern uint8_t APP_Rx_Buffer   [APP_RX_DATA_SIZE];
extern uint32_t APP_Rx_ptr_in;
extern uint32_t APP_Rx_ptr_out;
extern uint32_t APP_Rx_length;
extern uint8_t  USB_Tx_State;

void USBD_MIDI_DumpRingBuffer	(void);

extern USBD_ClassTypeDef  USBD_MIDI;
#define USBD_MIDI_CLASS    &USBD_MIDI

#ifdef __cplusplus
}
#endif

#endif  /* __USB_MIDI_H */
