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
    // ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 0;
    char_md.char_props.notify = 0; 
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL; 
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