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

uint32_t ble_cus_init(ble_gas_t * p_gas_s, const ble_gas_init_t * p_gas_init)
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
}