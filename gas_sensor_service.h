/**
 * @brief 
 * 
 * @file gas_sensor_service.h
 * @author Blake Hildebrand
 * @date 2018-09-27
 */

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

#define GAS_SENSOR_UUID_BASE    {0x3d, 0x63, 0x13, 0xd5, 0xd7, 0xcf, 0x49, 0x16, 0xab, 0xf1, 0xa9, 0x6b, 0x47, 0x8f, 0x8b, 0xf3}
#define GAS_SENSOR_UUID         0x1450
#define HUMITEMP_CHAR_UUID      0x1451 /**< Humidity and temperature characteristic */

/**@brief   Macro for defining a ble_cus instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_GAS_SERVICE_DEF(_name)                                                                          \
static ble_gas_t _name;       

/**
 * @brief 
 * 
 */
typedef struct ble_gas_s ble_gas_t;

typedef enum
{
    BLE_GAS_EVT_NOTIFICATION_ENABLED,                             /**< Custom value notification enabled event. */
    BLE_GAS_EVT_NOTIFICATION_DISABLED,
    BLE_GAS_EVT_DISCONNECTED,
    BLE_GAS_EVT_CONNECTED
} ble_gas_evt_type_t;

typedef struct
{
    ble_gas_evt_type_t evt_type;                                  /**< Type of event. */
} ble_gas_evt_t;

/**@brief Custom Service event handler type. */
typedef void (*ble_gas_evt_handler_t) (ble_gas_t * p_cus, ble_gas_evt_t * p_evt);

/**
 * @brief 
 * 
 */
typedef struct
{
    ble_gas_evt_handler_t         evt_handler; 
    uint8_t                       initial_custom_value;           /**< Initial custom value */
    ble_srv_cccd_security_mode_t  custom_value_char_attr_md;     /**< Initial security level for Custom characteristics attribute */
} ble_gas_init_t;

/**
 * @brief 
 * 
 */
struct ble_gas_s
{
    ble_gas_evt_handler_t         evt_handler; 
    uint16_t                      service_handle;                 /**< Handle of Custom Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t      custom_value_handles;           /**< Handles related to the Custom Value characteristic. */
    uint16_t                      conn_handle;                    /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    uint8_t                       uuid_type; 
};

/**
 * @brief 
 * 
 * @param p_cus 
 * @param p_cus_init 
 * @return uint32_t 
 */
uint32_t ble_gas_init(ble_gas_t * p_gas_s, const ble_gas_init_t * p_gas_init);

void ble_gas_on_ble_evt( ble_evt_t * p_ble_evt, void * p_context);

uint32_t ble_gas_custom_value_update(ble_gas_t * p_cus, uint8_t custom_value);