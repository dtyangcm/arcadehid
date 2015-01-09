/**
  ******************************************************************************
  * @file    usb_endp.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Endpoint routines
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "usb_hw_config.h"
#include "usb_lib.h"
#include "usb_istr.h"
#include "usb_conf.h"

#ifdef CONFIG_ARCHID_VCD
#include "usb_desc.h"
#include "usb_pwr.h"

/* Interval between sending IN packets in frame number (1 frame = 1ms) */
#define VCOMPORT_IN_FRAME_INTERVAL             5

uint8_t USB_Rx_Buffer[VIRTUAL_COM_PORT_DATA_SIZE];
extern ringbuf tx_rb;
extern uint8_t USB_Tx_State;

#endif

void EP1_IN_Callback(void)
{
  /* Set the transfer complete token to inform upper layer that the current 
  transfer has been complete */
  kb_tx_complete = 1;
}

void EP2_IN_Callback(void)
{
  /* Set the transfer complete token to inform upper layer that the current
  transfer has been complete */
  mouse_tx_complete = 1;
}

#ifdef CONFIG_ARCHID_VCD

void EP5_IN_Callback (void)
{
  if (USB_Tx_State == 1)
  {
    u8_t *buf;
    int avail = ringbuf_available_linear(&tx_rb, &buf);
    if (avail  == 0) {
      USB_Tx_State = 0;
      return;
    }

    if (avail > VIRTUAL_COM_PORT_DATA_SIZE)
    {
      avail = VIRTUAL_COM_PORT_DATA_SIZE;
    }

    UserToPMABufferCopy(buf, ENDP5_TXADDR, avail);
    ringbuf_get(&tx_rb, 0, avail);
    SetEPTxCount(ENDP5, avail);
    SetEPTxValid(ENDP5);
  }
}

void EP4_OUT_Callback(void)
{
  uint16_t USB_Rx_Cnt;

  /* Get the received data buffer and update the counter */
  USB_Rx_Cnt = USB_SIL_Read(EP4_OUT, USB_Rx_Buffer);

  /* USB data will be immediately processed, this allow next USB traffic being
  NAKed till the end of the USART Xfer */

  ringbuf_put(&rx_rb, USB_Rx_Buffer, USB_Rx_Cnt);

  /* Enable the receive of data on EP4 */
  SetEPRxValid(ENDP4);

  if (rx_cb != NULL) {
    rx_cb(ringbuf_available(&rx_rb), NULL);
  }
}

/*******************************************************************************
* Function Name  : SOF_Callback / INTR_SOFINTR_Callback
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SOF_Callback(void)
{
  static uint32_t FrameCount = 0;

  if(bDeviceState == CONFIGURED)
  {
    if (FrameCount++ == VCOMPORT_IN_FRAME_INTERVAL)
    {
      /* Reset the frame counter */
      FrameCount = 0;

      /* Check the data to be sent through IN pipe */
      Handle_USBAsynchXfer();
    }
  }
}

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

