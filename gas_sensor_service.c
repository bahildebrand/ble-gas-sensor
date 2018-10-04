/**
 * @brief 
 * 
 * @file gas_sensor_service.c
 * @author Blake Hildebrand
 * @date 2018-09-27
 */

#include "sdk_common.h"
#include "ble_srv_common.h"
#include "gas_sensor_service.h"
#include <string.h>
#include "nrf_gpio.h"
#include "boards.h"
#include "nrf_log.h"

/**
 * @brief 
 * 
 * @param p_cus 
 * @param p_cus_init 
 * @return uint32_t 
 */
static uint32_t custom_value_char_add(ble_gas_t * p_gas, const ble_gas_init_t * p_cus_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on Cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 0;
    char_md.char_props.notify = 0; 
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.char_props.notify = 1;  
    char_md.p_cccd_md         = &cccd_md; 
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_gas->uuid_type;
    ble_uuid.uuid = HUMITEMP_CHAR_UUID;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint8_t);

    err_code = sd_ble_gatts_characteristic_add(p_gas->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_gas->custom_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**
 * @brief 
 * 
 * @param p_gas 
 * @param p_ble_evt 
 */
static void on_connect(ble_gas_t * p_gas, ble_evt_t const * p_ble_evt)
{
    ble_gas_evt_t evt;

    p_gas->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

    evt.evt_type = BLE_GAS_EVT_CONNECTED;

    p_gas->evt_handler(p_gas, &evt);
}

/**
 * @brief 
 * 
 * @param p_gas 
 * @param p_ble_evt 
 */
static void on_disconnect(ble_gas_t * p_gas, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_gas->conn_handle = BLE_CONN_HANDLE_INVALID;
}

/**
 * @brief 
 * 
 * @param p_gas 
 * @param p_ble_evt 
 */
static void on_write(ble_gas_t * p_gas, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    
    // Check if the handle passed with the event matches the Custom Value Characteristic handle.
    if (p_evt_write->handle == p_gas->custom_value_handles.value_handle)
    {
        // Put specific task here. 
    }

    if ((p_evt_write->handle == p_gas->custom_value_handles.cccd_handle)
        && (p_evt_write->len == 2)
       )
    {

        // CCCD written, call application event handler
        if (p_gas->evt_handler != NULL)
        {
            ble_gas_evt_t evt;

            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                evt.evt_type = BLE_GAS_EVT_NOTIFICATION_ENABLED;
            }
            else
            {
                evt.evt_type = BLE_GAS_EVT_NOTIFICATION_DISABLED;
            }
            // Call the application event handler.
            p_gas->evt_handler(p_gas, &evt);
        }
    }

}

/**
 * @brief 
 * 
 * @param p_gas_s 
 * @param p_gas_init 
 * @return uint32_t 
 */
uint32_t ble_gas_init(ble_gas_t * p_gas_s, const ble_gas_init_t * p_gas_init)
{
    uint32_t        err_code;
    ble_uuid_t      ble_uuid;
    ble_uuid128_t   base_uuid = {GAS_SENSOR_UUID_BASE};

    if (p_gas_s == NULL || p_gas_init == NULL)
    {
        return NRF_ERROR_NULL;
    }

    // Initialize service structure
    p_gas_s->conn_handle = BLE_CONN_HANDLE_INVALID;
    p_gas_s->evt_handler               = p_gas_init->evt_handler;
    p_gas_s->conn_handle               = BLE_CONN_HANDLE_INVALID;

    // Add Custom Service UUID
    err_code =  sd_ble_uuid_vs_add(&base_uuid, &p_gas_s->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_gas_s->uuid_type;
    ble_uuid.uuid = GAS_SENSOR_UUID;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_gas_s->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return custom_value_char_add(p_gas_s, p_gas_init);
}

void ble_gas_on_ble_evt( ble_evt_t * p_ble_evt, void * p_context)
{
    ble_gas_t * p_gas = (ble_gas_t *) p_context;
    
    if (p_gas == NULL || p_ble_evt == NULL)
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_gas, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_gas, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_gas, p_ble_evt);
           break;

        default:
            // No implementation needed.
            break;
    }
}

uint32_t ble_gas_custom_value_update(ble_gas_t * p_gas, uint8_t custom_value)
{
    uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;
    
    if (p_gas == NULL)
    {
        return NRF_ERROR_NULL;
    }

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = sizeof(uint8_t);
    gatts_value.offset  = 0;
    gatts_value.p_value = &custom_value;

    // Update database.
    err_code = sd_ble_gatts_value_set(p_gas->conn_handle,
                                        p_gas->custom_value_handles.value_handle,
                                        &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    if ((p_gas->conn_handle != BLE_CONN_HANDLE_INVALID)) 
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_gas->custom_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_gas->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}
