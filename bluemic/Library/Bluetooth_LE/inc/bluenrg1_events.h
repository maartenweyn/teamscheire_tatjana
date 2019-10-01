#ifndef __BLUENRG1_EVENTS_H__
#define __BLUENRG1_EVENTS_H__


/**
  ******************************************************************************
  * @file    bluenrg1_events.h
  * @author  AMS - VMA RF Application team
  * @version V1.0.0
  * @date    21-Sept-2015
  * @brief   Header file for BlueNRG-1 Bluetooth Low Energy stack events callbacks
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  ******************************************************************************
  */
#include <stdint.h>
/**
 *@defgroup HCI_events HCI events
 *@brief HCI events layer.
 *@{
 */
/* HCI events */
/** Documentation for Handle_Packets_Pair_Entry_t */
typedef struct Handle_Packets_Pair_Entry_t_s {
  uint16_t Connection_Handle /** Connection_Handle */;
  uint16_t HC_Num_Of_Completed_Packets /** The number of HCI Data Packets that have been completed (transmitted
or flushed) for the associated Connection_Handle since the previous time
the event was returned.
Range for N: 0x0000-0xFFFF */;
} Handle_Packets_Pair_Entry_t;
/**
  * @brief The Disconnection Complete event occurs when a connection is terminated.
The status parameter indicates if the disconnection was successful or not. The
reason parameter indicates the reason for the disconnection if the disconnection
was successful. If the disconnection was not successful, the value of the
reason parameter can be ignored by the Host. For example, this can be the
case if the Host has issued the Disconnect command and there was a parameter
error, or the command was not presently allowed, or a Connection_Handle
that didn't correspond to a connection was given.
  * @param Status Error code
  * @param Connection_Handle Connection_Handle which was disconnected.
Range: 0x0000-0x0EFF (0x0F00 - 0x0FFF Reserved for future use)
  * @param Reason Reason for disconnection.
  * @retval None
*/
void hci_disconnection_complete_event(uint8_t Status,
                                      uint16_t Connection_Handle,
                                      uint8_t Reason);
/**
  * @brief The Encryption Change event is used to indicate that the change of the encryption
mode has been completed. The Connection_Handle will be a Connection_Handle
for an ACL connection. The Encryption_Enabled event parameter
specifies the new Encryption_Enabled parameter for the Connection_Handle
specified by the Connection_Handle event parameter. This event will occur on
both devices to notify the Hosts when Encryption has changed for the specified
Connection_Handle between two devices. Note: This event shall not be generated
if encryption is paused or resumed; during a role switch, for example.
The meaning of the Encryption_Enabled parameter depends on whether the
Host has indicated support for Secure Connections in the Secure_Connections_Host_Support
parameter. When Secure_Connections_Host_Support is
'disabled' or the Connection_Handle refers to an LE link, the Controller shall
only use Encryption_Enabled values 0x00 (OFF) and 0x01 (ON). When
Secure_Connections_Host_Support is 'enabled' and the Connection_Handle
refers to a BR/EDR link, the Controller shall set Encryption_Enabled to 0x00
when encryption is off, to 0x01 when encryption is on and using E0 and to 0x02
when encryption is on and using AES-CCM.
  * @param Status Error code
  * @param Connection_Handle Connection handle for which the command is given. 
Range: 0x0000-0x0EFF (0x0F00 - 0x0FFF Reserved for future use)
  * @param Encryption_Enabled - 0x00 Link Level Encryption is OFF
- 0x01 Link Level Encryption is ON with E0 for BR/EDR 
       Link Level Encryption is ON with AES-CCM for LE

- 0x02 Link Level Encryption is ON with AES-CCM for BR/EDR
- 0x03-0xFF Reserved.
  * @retval None
*/
void hci_encryption_change_event(uint8_t Status,
                                 uint16_t Connection_Handle,
                                 uint8_t Encryption_Enabled);
/**
  * @brief The Read Remote Version Information Complete event is used to indicate the
completion of the process obtaining the version information of the remote Controller
specified by the Connection_Handle event parameter. The Connection_Handle
shall be for an ACL connection.
The Version event parameter defines the specification version of the BR/EDR
or LE Controller. The Manufacturer_Name event parameter indicates the manufacturer
of the remote Controller. The Subversion event parameter is controlled
by the manufacturer and is implementation dependent. The Subversion
event parameter defines the various revisions that each version of the Bluetooth
hardware will go through as design processes change and errors are
fixed. This allows the software to determine what Bluetooth hardware is being
used and, if necessary, to work around various bugs in the hardware.
When the Connection_Handle is associated with a BR/EDR ACL-U logical link,
the Version event parameter shall be LMP VersNr parameter, the Manufacturer_Name
event parameter shall be the CompId parameter, and the Subversion
event parameter shall be the LMP SubVersNr parameter.
When the Connection_Handle is associated with an LE-U logical link, the Version
event parameter shall be Link Layer VersNr parameter, the Manufacturer_Name
event parameter shall be the CompId parameter, and the Subversion
event parameter shall be the SubVersNr parameter.
  * @param Status Error code
  * @param Connection_Handle Connection handle for which the command is given. 
Range: 0x0000-0x0EFF (0x0F00 - 0x0FFF Reserved for future use)
  * @param Version Version of the Current LMP in the remote Controller
  * @param Manufacturer_Name Manufacturer Name of the remote Controller
  * @param Subversion Subversion of the LMP in the remote Controller
  * @retval None
*/
void hci_read_remote_version_information_complete_event(uint8_t Status,
                                                        uint16_t Connection_Handle,
                                                        uint8_t Version,
                                                        uint16_t Manufacturer_Name,
                                                        uint16_t Subversion);
/**
  * @brief The Hardware Error event is used to indicate some implementation specific type of hardware failure for the controller. This event is used to notify the Host that a hardware failure has occurred in the Controller.
  * @param Hardware_Code Hardware Error Event code.
Error code 0 means that an error on the SPI bus has been detected,
most likely caused by incorrect SPI configuration on the external micro-controller.
Error code 0x01 and 0x02 are errors caused by a slow crystal startup and they are an indication that
the HS_STARTUP_TIME in the device configuration needs to be tuned.
After this event is recommended to hardware reset the device.
  * @retval None
*/
void hci_hardware_error_event(uint8_t Hardware_Code);
/**
  * @brief The Number Of Completed Packets event is used by the Controller to indicate
to the Host how many HCI Data Packets have been completed (transmitted or
flushed) for each Connection_Handle since the previous Number Of Completed
Packets event was sent to the Host. This means that the corresponding
buffer space has been freed in the Controller. Based on this information, and
the HC_Total_Num_ACL_Data_Packets and HC_Total_Num_Synchronous_-
Data_Packets return parameter of the Read_Buffer_Size command, the Host
can determine for which Connection_Handles the following HCI Data Packets
should be sent to the Controller. The Number Of Completed Packets event
must not be sent before the corresponding Connection Complete event. While
the Controller has HCI data packets in its buffer, it must keep sending the Number
Of Completed Packets event to the Host at least periodically, until it finally
reports that all the pending ACL Data Packets have been transmitted or
flushed.
  * @param Number_of_Handles The number of Connection_Handles and Num_HCI_Data_Packets parameters pairs contained in this event
  * @param Handle_Packets_Pair_Entry See @ref Handle_Packets_Pair_Entry_t
  * @retval None
*/
void hci_number_of_completed_packets_event(uint8_t Number_of_Handles,
                                           Handle_Packets_Pair_Entry_t Handle_Packets_Pair_Entry[]);
/**
  * @brief This event is used to indicate that the Controller's data buffers have been overflowed.
This can occur if the Host has sent more packets than allowed. The
Link_Type parameter is used to indicate that the overflow was caused by ACL data.
  * @param Link_Type 0x00 Synchronous Buffer Overflow (Voice Channels).
0x01 ACL Buffer Overflow (Data Channels).
0x02-0xFF Reserved for future use.
  * @retval None
*/
void hci_data_buffer_overflow_event(uint8_t Link_Type);
/**
  * @brief The Encryption Key Refresh Complete event is used to indicate to the Host
that the encryption key was refreshed on the given Connection_Handle any
time encryption is paused and then resumed.
If the Encryption Key Refresh Complete event was generated due to an
encryption pause and resume operation embedded within a change connection
link key procedure, the Encryption Key Refresh Complete event shall be sent
prior to the Change Connection Link Key Complete event.
If the Encryption Key Refresh Complete event was generated due to an
encryption pause and resume operation embedded within a role switch procedure,
the Encryption Key Refresh Complete event shall be sent prior to the
Role Change event.
  * @param Status Error code
  * @param Connection_Handle Connection handle for which the command is given. 
Range: 0x0000-0x0EFF (0x0F00 - 0x0FFF Reserved for future use)
  * @retval None
*/
void hci_encryption_key_refresh_complete_event(uint8_t Status,
                                               uint16_t Connection_Handle);
/**
 * @}
 */
/**
 *@defgroup HCI_LE_meta_events HCI LE meta events
 *@brief HCI LE meta events layer.
 *@{
 */
/* HCI LE meta events */
/** Documentation for Advertising_Report_t */
typedef struct Advertising_Report_t_s {
  uint8_t Event_Type /** 0x00 Connectable undirected advertising (ADV_IND).
0x01 Connectable directed advertising (ADV_DIRECT_IND)
0x02 Scannable undirected advertising (ADV_SCAN_IND)
0x03 Non connectable undirected advertising (ADV_NONCONN_IND)
0x04 Scan Response (SCAN_RSP)
0x05-0xFF Reserved for future use */;
  uint8_t Address_Type /** 0x00 Public Device Address
0x01 Random Device Address
0x02 - 0xFF Reserved for future use */;
  uint8_t Address[6] /** Public Device Address or Random Device Address of the device
to be connected */;
  uint8_t Length_Data /** 0x00 - 0x1F Length of the Data[i] field for each device which responded.
0x20 - 0xFF Reserved for future use. */;
  uint8_t *Data /** Length_Data[i] octets of advertising or scan response data formatted
as defined in [Vol 3] Part C, Section 8. */;
  uint8_t RSSI /** N Size: 1 Octet (signed integer)
Range: -127 <= N <= +20
Units: dBm
127 RSSI is not available
21 to 126 Reserved for future use */;
} Advertising_Report_t;
/**
  * @brief The LE Connection Complete event indicates to both of the Hosts forming the
connection that a new connection has been created. Upon the creation of the
connection a Connection_Handle shall be assigned by the Controller, and
passed to the Host in this event. If the connection establishment fails this event
shall be provided to the Host that had issued the LE_Create_Connection command.
This event indicates to the Host which issued a LE_Create_Connection
command and received a Command Status event if the connection
establishment failed or was successful.
The Master_Clock_Accuracy parameter is only valid for a slave. On a master,
this parameter shall be set to 0x00.
  * @param Status Error code
  * @param Connection_Handle Connection_Handle to be used to identify a connection between two
Bluetooth devices. 
Range: 0x0000-0x0EFF (0x0F00 - 0x0FFF Reserved for future use)
  * @param Role 0x00 Connection is master
0x01 Connection is slave
0x02-0xFF Reserved for future use
  * @param Peer_Address_Type The address type of the peer device:
0x00 Public Device Address
0x01 Random Device Address
0x02-0xFF Reserved for future use
  * @param Peer_Address Public Device Address or Random Device Address of the peer
device
  * @param Conn_Interval Connection interval used on this connection.
Range: 0x0006 to 0x0C80
Time = N * 1.25 msec
Time Range: 7.5 msec to 4000 msec.
  * @param Conn_Latency Slave latency for the connection in number of connection events.
Range: 0x0000 to 0x01F3
  * @param Supervision_Timeout N = 0xXXXX    Supervision timeout for the LE Link. (See [Vol 6] Part B, Section
4.5.2)
Range: 0x000A to 0x0C80
Time = N * 10 msec
Time Range: 100 msec to 32 seconds
0x0000 - 0x0009 and
0x0C81 - 0xFFFF Reserved for future use
  * @param Master_Clock_Accuracy 0x00 500 ppm
0x01 250 ppm
0x02 150 ppm
0x03 100 ppm
0x04 75 ppm
0x05 50 ppm
0x06 30 ppm
0x07 20 ppm
0x08-0xFF Reserved for future use
  * @retval None
*/
void hci_le_connection_complete_event(uint8_t Status,
                                      uint16_t Connection_Handle,
                                      uint8_t Role,
                                      uint8_t Peer_Address_Type,
                                      uint8_t Peer_Address[6],
                                      uint16_t Conn_Interval,
                                      uint16_t Conn_Latency,
                                      uint16_t Supervision_Timeout,
                                      uint8_t Master_Clock_Accuracy);
/**
  * @brief The LE Advertising Report event indicates that a Bluetooth device or multiple
Bluetooth devices have responded to an active scan or received some information
during a passive scan. The Controller may queue these advertising reports
and send information from multiple devices in one LE Advertising Report event.
  * @param Num_Reports 0x01 - 0x19 Number of responses in event.
  * @param Advertising_Report See @ref Advertising_Report_t
  * @retval None
*/
void hci_le_advertising_report_event(uint8_t Num_Reports,
                                     Advertising_Report_t Advertising_Report[]);
/**
  * @brief The LE Connection Update Complete event is used to indicate that the Controller
process to update the connection has completed.
On a slave, if no connection parameters are updated, then this event shall not be issued.
On a master, this event shall be issued if the Connection_Update command was sent.
  * @param Status Error code
  * @param Connection_Handle Connection_Handle to be used to identify a connection between two
Bluetooth devices.
Range: 0x0000-0x0EFF (0x0F00 - 0x0FFF Reserved for future use)
  * @param Conn_Interval Connection interval used on this connection.
Range: 0x0006 to 0x0C80
Time = N * 1.25 msec
Time Range: 7.5 msec to 4000 msec.
  * @param Conn_Latency Slave latency for the connection in number of connection events.
Range: 0x0000 to 0x01F3
  * @param Supervision_Timeout N = 0xXXXX    Supervision timeout for the LE Link. (See [Vol 6] Part B, Section
4.5.2)
Range: 0x000A to 0x0C80
Time = N * 10 msec
Time Range: 100 msec to 32 seconds
0x0000 - 0x0009 and
0x0C81 - 0xFFFF Reserved for future use
  * @retval None
*/
void hci_le_connection_update_complete_event(uint8_t Status,
                                             uint16_t Connection_Handle,
                                             uint16_t Conn_Interval,
                                             uint16_t Conn_Latency,
                                             uint16_t Supervision_Timeout);
/**
  * @brief The LE Read Remote Used Features Complete event is used to indicate the
completion of the process of the Controller obtaining the used features of the
remote Bluetooth device specified by the Connection_Handle event parameter.
  * @param Status Error code
  * @param Connection_Handle Connection_Handle to be used to identify a connection between two
Bluetooth devices. 
Range: 0x0000-0x0EFF (0x0F00 - 0x0FFF Reserved for future use)
  * @param LE_Features Bit Mask List of used LE features. For details see LE Link Layer specification.
  * @retval None
*/
void hci_le_read_remote_used_features_complete_event(uint8_t Status,
                                                     uint16_t Connection_Handle,
                                                     uint8_t LE_Features[8]);
/**
  * @brief The LE Long Term Key Request event indicates that the master device is
attempting to encrypt or re-encrypt the link and is requesting the Long Term
Key from the Host. (See [Vol 6] Part B, Section 5.1.3).
  * @param Connection_Handle Connection_Handle to be used to identify a connection between two
Bluetooth devices. 
Range: 0x0000-0x0EFF (0x0F00 - 0x0FFF Reserved for future use)
  * @param Random_Number 64-bit random number
  * @param Encrypted_Diversifier 16-bit encrypted diversifier
  * @retval None
*/
void hci_le_long_term_key_request_event(uint16_t Connection_Handle,
                                        uint8_t Random_Number[8],
                                        uint16_t Encrypted_Diversifier);
/**
 * @}
 */
/**
 *@defgroup ACI_GAP_events ACI GAP events
 *@brief ACI GAP events layer.
 *@{
 */
/* ACI GAP events */
/**
  * @brief This event is generated by the controller when the limited discoverable mode ends due to
timeout. The timeout is 180 seconds.
  * @retval None
*/
void aci_gap_limited_discoverable_event(void);
/**
  * @brief This event is generated when the pairing process has completed successfully or a pairing
procedure timeout has occurred or the pairing has failed. This is to notify the application that
we have paired with a remote device so that it can take further actions or to notify that a
timeout has occurred so that the upper layer can decide to disconnect the link.
  * @param Connection_Handle Connection handle on which the pairing procedure completed
  * @param Status Pairing status
  * @retval None
*/
void aci_gap_pairing_complete_event(uint16_t Connection_Handle,
                                    uint8_t Status);
/**
  * @brief This event is generated by the Security manager to the application when a passkey is
required for pairing. When this event is received, the application has to respond with the
@ref aci_gap_pass_key_resp command.
  * @param Connection_Handle Connection handle for which the passkey has been requested.
  * @retval None
*/
void aci_gap_pass_key_req_event(uint16_t Connection_Handle);
/**
  * @brief This event is generated by the Security manager to the application when the application has
set that authorization is required for reading/writing of attributes. This event will be
generated as soon as the pairing is complete. When this event is received,
@ref aci_gap_authorization_resp command should be used to respond by the application.
  * @param Connection_Handle Connection handle for which authorization has been requested.
  * @retval None
*/
void aci_gap_authorization_req_event(uint16_t Connection_Handle);
/**
  * @brief This event is generated when the slave security request is successfully sent to the master.
  * @retval None
*/
void aci_gap_slave_security_initiated_event(void);
/**
  * @brief This event is generated when a pairing request is issued in response to a slave security
request from a master which has previously bonded with the slave. When this event is
received, the upper layer has to issue the command @ref aci_gap_allow_rebond in order to
allow the slave to continue the pairing process with the master.
  * @retval None
*/
void aci_gap_bond_lost_event(void);
/**
  * @brief This event is sent by the GAP to the upper layers when a procedure previously started has
been terminated by the upper layer or has completed for any other reason
  * @param Procedure_Code Terminated procedure.
0x01 LIMITED_DISCOVERY_PROC,
0x02 GENERAL_DISCOVERY_PROC,
0x04 NAME_DISCOVERY_PROC,
0x08 AUTO_CONNECTION_ESTABLISHMENT_PROC,
0x10 GENERAL_CONNECTION_ESTABLISHMENT_PROC,
0x20 SELECTIVE_CONNECTION_ESTABLISHMENT_PROC,
0x40 DIRECT_CONNECTION_ESTABLISHMENT_PROC
0x80 OBSERVATION_PROC
  * @param Status Error code
  * @param Data_Length Length of Data in octets
  * @param Data Procedure Specific Data:
- For Name Discovery Procedure: the name of the peer device if the procedure completed successfully.
  * @retval None
*/
void aci_gap_proc_complete_event(uint8_t Procedure_Code,
                                 uint8_t Status,
                                 uint8_t Data_Length,
                                 uint8_t Data[]);
/**
  * @brief This event is sent only by a privacy enabled Peripheral. The event is sent to the
upper layers when the peripheral is unsuccessful in resolving the resolvable
address of the peer device after connecting to it.
  * @param Connection_Handle Connection handle for which the private address could not be
resolved with any of the stored IRK's.
  * @retval None
*/
void aci_gap_addr_not_resolved_event(uint16_t Connection_Handle);
/**
 * @}
 */
/**
 *@defgroup ACI_GATT_ATT_events ACI GATT/ATT events
 *@brief ACI GATT/ATT events layer.
 *@{
 */
/* ACI GATT/ATT events */
/** Documentation for Attribute_Group_Handle_Pair_t */
typedef struct Attribute_Group_Handle_Pair_t_s {
  uint16_t Found_Attribute_Handle /** Found Attribute handle */;
  uint16_t Group_End_Handle /** Group End handle */;
} Attribute_Group_Handle_Pair_t;
/** Documentation for Handle_Item_t */
typedef struct Handle_Item_t_s {
  uint16_t Handle /** Not available */;
} Handle_Item_t;
/**
  * @brief This event is generated to the application by the GATT server when a client modifies any
attribute on the server, as consequence of one of the following GATT procedures:
- write without response
- signed write without response
- write characteristic value
- write long characteristic value
- reliable write.
  * @param Connection_Handle The connection handle which modified the attribute.
  * @param Attr_Handle Handle of the attribute that was modified.
  * @param Offset Offset from which the write has been performed by the peer device.
  * @param Attr_Data_Length Length of Attr_Data in octets
  * @param Attr_Data The modified value
  * @retval None
*/
void aci_gatt_attribute_modified_event(uint16_t Connection_Handle,
                                       uint16_t Attr_Handle,
                                       uint16_t Offset,
                                       uint8_t Attr_Data_Length,
                                       uint8_t Attr_Data[]);
/**
  * @brief This event is generated by the client/server to the application on a GATT timeout (30
seconds). This is a critical event that should not happen during normal operating conditions. It is an indication of either a major disruption in the communication link or a mistake in the application which does not provide a reply to GATT procedures. After this event, the GATT channel is closed and no more GATT communication can be performed. The applications is exptected to issue an @ref aci_gap_terminate to disconnect from the peer device. It is important to leave an 100 ms blank window before sending the @ref aci_gap_terminate, since immediately after this event, system could save important information in non volatile memory.
  * @param Connection_Handle Connection handle on which the GATT procedure has timed out
  * @retval None
*/
void aci_gatt_proc_timeout_event(uint16_t Connection_Handle);
/**
  * @brief This event is generated in response to an Exchange MTU request. See
@ref aci_gatt_exchange_config.
  * @param Connection_Handle Connection handle related to the response
  * @param Server_RX_MTU Attribute server receive MTU size
  * @retval None
*/
void aci_att_exchange_mtu_resp_event(uint16_t Connection_Handle,
                                     uint16_t Server_RX_MTU);
/**
  * @brief This event is generated in response to a Find Information Request. See
@ref aci_att_find_info_req and Find Information Response in Bluetooth Core v4.1
spec.
  * @param Connection_Handle Connection handle related to the response
  * @param Format Format of the hanndle-uuid pairs
  * @param Event_Data_Length Length of Handle_UUID_Pair in octets
  * @param Handle_UUID_Pair A sequence of handle-uuid pairs. if format=1, each pair is:[2 octets for handle, 2 octets for UUIDs], if format=2, each pair is:[2 octets for handle, 16 octets for UUIDs]
  * @retval None
*/
void aci_att_find_info_resp_event(uint16_t Connection_Handle,
                                  uint8_t Format,
                                  uint8_t Event_Data_Length,
                                  uint8_t Handle_UUID_Pair[]);
/**
  * @brief This event is generated in response to a @ref aci_att_find_by_type_value_req
  * @param Connection_Handle Connection handle related to the response
  * @param Num_of_Handle_Pair Number of attribute, group handle pairs
  * @param Attribute_Group_Handle_Pair See @ref Attribute_Group_Handle_Pair_t
  * @retval None
*/
void aci_att_find_by_type_value_resp_event(uint16_t Connection_Handle,
                                           uint8_t Num_of_Handle_Pair,
                                           Attribute_Group_Handle_Pair_t Attribute_Group_Handle_Pair[]);
/**
  * @brief This event is generated in response to a @ref aci_att_read_by_type_req. See
@ref aci_gatt_find_included_services and @ref aci_gatt_disc_all_char_desc.
  * @param Connection_Handle Connection handle related to the response
  * @param Handle_Value_Pair_Length The size of each attribute handle-value pair
  * @param Data_Length Length of Handle_Value_Pair_Data in octets
  * @param Handle_Value_Pair_Data Attribute Data List as defined in Bluetooth Core v4.1 spec. A sequence of handle-value pairs: [2 octets for Attribute Handle, (Handle_Value_Pair_Length - 2 octets) for Attribute Value]
  * @retval None
*/
void aci_att_read_by_type_resp_event(uint16_t Connection_Handle,
                                     uint8_t Handle_Value_Pair_Length,
                                     uint8_t Data_Length,
                                     uint8_t Handle_Value_Pair_Data[]);
/**
  * @brief This event is generated in response to a Read Request. See @ref aci_gatt_read_char_value.
  * @param Connection_Handle Connection handle related to the response
  * @param Event_Data_Length Length of following data
  * @param Attribute_Value The value of the attribute.
  * @retval None
*/
void aci_att_read_resp_event(uint16_t Connection_Handle,
                             uint8_t Event_Data_Length,
                             uint8_t Attribute_Value[]);
/**
  * @brief This event is generated in response to a Read Request. See @ref aci_gatt_read_char_value.
  * @param Connection_Handle Connection handle related to the response
  * @param Event_Data_Length Length of following data
  * @param Attribute_Value Part of the attribute value.
  * @retval None
*/
void aci_att_read_blob_resp_event(uint16_t Connection_Handle,
                                  uint8_t Event_Data_Length,
                                  uint8_t Attribute_Value[]);
/**
  * @brief This event is generated in response to a Read Multiple Request.
  * @param Connection_Handle Connection handle related to the response
  * @param Event_Data_Length Length of following data
  * @param Set_Of_Values A set of two or more values.
A concatenation of attribute values for each of the attribute handles in the request in the order that they were requested.
  * @retval None
*/
void aci_att_read_multiple_resp_event(uint16_t Connection_Handle,
                                      uint8_t Event_Data_Length,
                                      uint8_t Set_Of_Values[]);
/**
  * @brief This event is generated in response to a Read By Group Type Request. See
@ref aci_gatt_disc_all_primary_services.
  * @param Connection_Handle Connection handle related to the response
  * @param Attribute_Data_Length The size of each attribute data
  * @param Data_Length Length of Attribute_Data_List in octets
  * @param Attribute_Data_List Attribute Data List as defined in Bluetooth Core v4.1 spec. A sequence of attribute handle, end group handle, attribute value tuples: [2 octets for Attribute Handle, 2 octets End Group Handle, (Attribute_Data_Length - 4 octets) for Attribute Value]
  * @retval None
*/
void aci_att_read_by_group_type_resp_event(uint16_t Connection_Handle,
                                           uint8_t Attribute_Data_Length,
                                           uint8_t Data_Length,
                                           uint8_t Attribute_Data_List[]);
/**
  * @brief This event is generated in response to a @ref aci_att_prepare_write_req.
  * @param Connection_Handle Connection handle related to the response
  * @param Attribute_Handle The handle of the attribute to be written
  * @param Offset The offset of the first octet to be written.
  * @param Part_Attribute_Value_Length Length of Part_Attribute_Value in octets
  * @param Part_Attribute_Value The value of the attribute to be written
  * @retval None
*/
void aci_att_prepare_write_resp_event(uint16_t Connection_Handle,
                                      uint16_t Attribute_Handle,
                                      uint16_t Offset,
                                      uint8_t Part_Attribute_Value_Length,
                                      uint8_t Part_Attribute_Value[]);
/**
  * @brief This event is generated in response to an Execute Write Request.
  * @param Connection_Handle Connection handle related to the response
  * @retval None
*/
void aci_att_exec_write_resp_event(uint16_t Connection_Handle);
/**
  * @brief This event is generated when an indication is received from the server.
  * @param Connection_Handle Connection handle related to the response
  * @param Attribute_Handle The handle of the attribute
  * @param Attribute_Value_Length Length of Attribute_Value in octets
  * @param Attribute_Value The current value of the attribute
  * @retval None
*/
void aci_gatt_indication_event(uint16_t Connection_Handle,
                               uint16_t Attribute_Handle,
                               uint8_t Attribute_Value_Length,
                               uint8_t Attribute_Value[]);
/**
  * @brief This event is generated when a notification is received from the server.
  * @param Connection_Handle Connection handle related to the response
  * @param Attribute_Handle The handle of the attribute
  * @param Attribute_Value_Length Length of Attribute_Value in octets
  * @param Attribute_Value The current value of the attribute
  * @retval None
*/
void aci_gatt_notification_event(uint16_t Connection_Handle,
                                 uint16_t Attribute_Handle,
                                 uint8_t Attribute_Value_Length,
                                 uint8_t Attribute_Value[]);
/**
  * @brief This event is generated when a GATT client procedure completes either with error or
successfully.
  * @param Connection_Handle Connection handle related to the response
  * @param Error_Code Indicates whether the procedure completed with an error or was successful
  * @retval None
*/
void aci_gatt_proc_complete_event(uint16_t Connection_Handle,
                                  uint8_t Error_Code);
/**
  * @brief This event is generated when an Error Response is received from the server. The error
response can be given by the server at the end of one of the GATT discovery procedures.
This does not mean that the procedure ended with an error, but this error event is part of the
procedure itself.
  * @param Connection_Handle Connection handle related to the response
  * @param Req_Opcode The request that generated this error response
  * @param Attribute_Handle The attribute handle that generated this error response
  * @param Error_Code The reason why the request has generated an error response (ATT error codes)
  * @retval None
*/
void aci_gatt_error_resp_event(uint16_t Connection_Handle,
                               uint8_t Req_Opcode,
                               uint16_t Attribute_Handle,
                               uint8_t Error_Code);
/**
  * @brief This event can be generated during a "Discover Characteristics By UUID" procedure or a
"Read using Characteristic UUID" procedure.
The attribute value will be a service declaration as defined in Bluetooth Core v4.1spec
(vol.3, Part G, ch. 3.3.1), when a "Discover Characteristics By UUID" has been started. It will
be the value of the Characteristic if a* "Read using Characteristic UUID" has been
performed.
  * @param Connection_Handle Connection handle related to the response
  * @param Attribute_Handle The handle of the attribute
  * @param Attribute_Value_Length Length of Attribute_Value in octets
  * @param Attribute_Value The attribute value will be a service declaration as defined in Bluetooth Core v4.0 spec
 (vol.3, Part G, ch. 3.3.1), when a "Discover Characteristics By UUID" has been started.
 It will be the value of the Characteristic if a "Read using Characteristic UUID" has been performed.
  * @retval None
*/
void aci_gatt_disc_read_char_by_uuid_resp_event(uint16_t Connection_Handle,
                                                uint16_t Attribute_Handle,
                                                uint8_t Attribute_Value_Length,
                                                uint8_t Attribute_Value[]);
/**
  * @brief This event is given to the application when a write request, write command or signed write
command is received by the server from the client. This event will be given to the application
only if the event bit for this event generation is set when the characteristic was added.
When this event is received, the application has to check whether the value being requested
for write can be allowed to be written and respond with the command @ref aci_gatt_write_resp.
The details of the parameters of the command can be found. Based on the response from
the application, the attribute value will be modified by the stack. If the write is rejected by the
application, then the value of the attribute will not be modified. In case of a write REQ, an
error response will be sent to the client, with the error code as specified by the application.
In case of write/signed write commands, no response is sent to the client but the attribute is
not modified.
  * @param Connection_Handle Handle of the connection on which there was the request to write the attribute
  * @param Attribute_Handle The handle of the attribute
  * @param Data_Length Length of Data field
  * @param Data The data that the client has requested to write
  * @retval None
*/
void aci_gatt_write_permit_req_event(uint16_t Connection_Handle,
                                     uint16_t Attribute_Handle,
                                     uint8_t Data_Length,
                                     uint8_t Data[]);
/**
  * @brief This event is given to the application when a read request or read blob request is received
by the server from the client. This event will be given to the application only if the event bit
for this event generation is set when the characteristic was added.
On receiving this event, the application can update the value of the handle if it desires and
when done, it has to send the @ref aci_gatt_allow_read command to indicate to the stack that it
can send the response to the client.
  * @param Connection_Handle Connection handle related to the response
  * @param Attribute_Handle The handle of the attribute
  * @param Offset Contains the offset from which the read has been requested
  * @retval None
*/
void aci_gatt_read_permit_req_event(uint16_t Connection_Handle,
                                    uint16_t Attribute_Handle,
                                    uint16_t Offset);
/**
  * @brief This event is given to the application when a read multiple request or read by type request is
received by the server from the client. This event will be given to the application only if the
event bit for this event generation is set when the characteristic was added.
On receiving this event, the application can update the values of the handles if it desires and
when done, it has to send the @ref aci_gatt_allow_read command to indicate to the stack that it
can send the response to the client.
  * @param Connection_Handle Handle of the connection which requested to read the attribute
  * @param Number_of_Handles Not available
  * @param Handle_Item See @ref Handle_Item_t
  * @retval None
*/
void aci_gatt_read_multi_permit_req_event(uint16_t Connection_Handle,
                                          uint8_t Number_of_Handles,
                                          Handle_Item_t Handle_Item[]);
/**
  * @brief Each time BLE FW stack raises the error code
BLE_STATUS_INSUFFICIENT_RESOURCES (0x64), the
@ref aci_gatt_tx_pool_available_event event is generated as soon as there are at least two
buffers available for notifications or write commands.
  * @param Connection_Handle Connection handle related to the request
  * @param Available_Buffers Number of buffers available
  * @retval None
*/
void aci_gatt_tx_pool_available_event(uint16_t Connection_Handle,
                                      uint16_t Available_Buffers);
/**
  * @brief This event is generated when the client has sent the confirmation to a previously sent indication
  * @param Connection_Handle Connection handle related to the event
  * @retval None
*/
void aci_gatt_server_confirmation_event(uint16_t Connection_Handle);
/**
  * @brief This event is given to the application when a prepare write request
is received by the server from the client. This event will be given to the application
only if the event bit for this event generation is set when the characteristic was added.
When this event is received, the application has to check whether the value being requested
for write can be allowed to be written and respond with the command @ref aci_gatt_write_resp.
Based on the response from the application, the attribute value will be modified by the stack.
If the write is rejected by the application, then the value of the attribute will not be modified
and an error response will be sent to the client, with the error code as specified by the application.
  * @param Connection_Handle Handle of the connection on which there was the request to write the attribute
  * @param Attribute_Handle The handle of the attribute
  * @param Offset The offset from which the prepare write has been requested
  * @param Data_Length Length of Data field
  * @param Data The data that the client has requested to write
  * @retval None
*/
void aci_gatt_prepare_write_permit_req_event(uint16_t Connection_Handle,
                                             uint16_t Attribute_Handle,
                                             uint16_t Offset,
                                             uint8_t Data_Length,
                                             uint8_t Data[]);
/**
 * @}
 */
/**
 *@defgroup ACI_L2CAP_events ACI L2CAP events
 *@brief ACI L2CAP events layer.
 *@{
 */
/* ACI L2CAP events */
/**
  * @brief This event is generated when the master responds to the connection update request packet
with a connection update response packet.
  * @param Connection_Handle Connection handle referring to the COS Channel where the Disconnection has been received.
  * @param Result 
  * @retval None
*/
void aci_l2cap_connection_update_resp_event(uint16_t Connection_Handle,
                                            uint16_t Result);
/**
  * @brief This event is generated when the master does not respond to the connection update
request packet with a connection update response packet or a command reject packet
within 30 seconds.
  * @param Connection_Handle Handle of the connection related to this
L2CAP procedure.
  * @param Data_Length Length of following data
  * @param Data Not available
  * @retval None
*/
void aci_l2cap_proc_timeout_event(uint16_t Connection_Handle,
                                  uint8_t Data_Length,
                                  uint8_t Data[]);
/**
  * @brief The event is given by the L2CAP layer when a connection update request is received from
the slave. The upper layer which receives this event has to respond by sending a
@ref aci_l2cap_connection_parameter_update_resp command.
  * @param Connection_Handle Handle of the connection related to this
L2CAP procedure.
  * @param Identifier This is the identifier which associate the request to the
response.
  * @param L2CAP_Length Length of the L2CAP connection update request.
  * @param Interval_Min N = 0xXXXX    Minimum value for the connection event interval. This shall be less
than or equal to Conn_Interval_Max.
Range: 0x0006 to 0x0C80
Time = N * 1.25 msec
Time Range: 7.5 msec to 4 seconds.
0x0000 - 0x0005 and 0x0C81 - 0xFFFF Reserved for future use
  * @param Interval_Max N = 0xXXXX    Maximum value for the connection event interval. This shall be
greater than or equal to Conn_Interval_Min.
Range: 0x0006 to 0x0C80
Time = N * 1.25 msec
Time Range: 7.5 msec to 4 seconds.
0x0000 - 0x0005 and 0x0C81 - 0xFFFF Reserved for future use
  * @param Slave_Latency Slave latency for the connection in number of connection events.
Range: 0x0000 to 0x01F3
  * @param Timeout_Multiplier Defines connection timeout parameter in the following manner: Timeout Multiplier * 10ms.
  * @retval None
*/
void aci_l2cap_connection_update_req_event(uint16_t Connection_Handle,
                                           uint8_t Identifier,
                                           uint16_t L2CAP_Length,
                                           uint16_t Interval_Min,
                                           uint16_t Interval_Max,
                                           uint16_t Slave_Latency,
                                           uint16_t Timeout_Multiplier);
/**
  * @brief This event is generated when the master responds to the connection update request packet
with a command reject packet.
  * @param Connection_Handle Connection handle referring to the COS Channel where the Disconnection has been received.
  * @param Identifier This is the identifier which associate the request to the
response.
  * @param Reason Reason
  * @param Data_Length Length of following data
  * @param Data Data field associated with Reason
  * @retval None
*/
void aci_l2cap_command_reject_event(uint16_t Connection_Handle,
                                    uint8_t Identifier,
                                    uint16_t Reason,
                                    uint8_t Data_Length,
                                    uint8_t Data[]);
/**
 * @}
 */
/**
 *@defgroup ACI_HAL_events ACI HAL events
 *@brief ACI HAL events layer.
 *@{
 */
/* ACI HAL events */
/**
  * @brief This event is generated when the device completes a radio activity and provide information when a new radio acitivity will be performed.
Informtation provided includes type of radio activity and absolute time in system ticks when a new radio acitivity is schedule, if any. Application can use this information to schedule user activities synchronous to selected radio activitities. A command @ref aci_hal_set_radio_activity_mask is provided to enable radio activity events of user interests, by default no events are enabled.
User should take into account that enablinng radio events in application with intense radio activity could lead to a fairly high rate of events generated.
Application use cases includes synchronizing notification with connection interval, switiching antenna at the end of advertising or performing flash erase operation while radio is idle.
  * @param Last_State Completed radio events
  * @param Next_State Incoming radio events
  * @param Next_State_SysTime 32bit absolute current time expressed in internal time units.
  * @retval None
*/
void aci_hal_end_of_radio_activity_event(uint8_t Last_State,
                                         uint8_t Next_State,
                                         uint32_t Next_State_SysTime);
/**
 * @}
 */
#endif /* __BLUENRG1_EVENTS_H__*/
