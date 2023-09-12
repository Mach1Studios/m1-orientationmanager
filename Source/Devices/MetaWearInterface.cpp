//
//  m1-orientationmanager
//  Copyright © 2022 Mach1. All rights reserved.
//

#include "MetaWearInterface.h"

//==============================================================================
MetaWearInterface::MetaWearInterface()
{
}

MetaWearInterface::~MetaWearInterface()
{
}

//SimpleBLE::Peripheral* MetaWearInterface::get_peripheral_device(){
//    return this->peripheral;
//}

bool MetaWearInterface::set_peripheral_device(SimpleBLE::Peripheral& peripheral) {
    if (peripheral.address() == "UNKNOWN"){
        return false;
    } else {
        this->deviceInterface = new SimpleBLE::Safe::Peripheral(peripheral);
        return true;
    }
}

void MetaWearInterface::data_printer(void* context, const MblMwData* data) {
    // Print data as 2 digit hex values
    uint8_t* data_bytes = (uint8_t*) data->value;
    std::string bytes_str("[");
    char buffer[5];
    for (uint8_t i = 0; i < data->length; i++) {
        if (i) {
            bytes_str += ", ";
        }
        //sprintf(buffer, "0x%02x", data_bytes[i]);
        bytes_str += buffer;
    }
    bytes_str += "]";

    // Format time as YYYYMMDD-HH:MM:SS.LLL
    std::chrono::time_point<std::chrono::system_clock> then(std::chrono::milliseconds(data->epoch));
    auto time_c = std::chrono::system_clock::to_time_t(then);
    auto rem_ms= data->epoch % 1000;

    std::cout << "{timestamp: " << std::put_time(localtime(&time_c), "%Y%m%d-%T") << "." << rem_ms << ", "
        << "type_id: " << data->type_id << ", "
        << "bytes: " << bytes_str.c_str() << "}"
        << std::endl;
}

void MetaWearInterface::configure_sensor_fusion(MblMwMetaWearBoard* board) {
    // set fusion mode to ndof (n degress of freedom)
    mbl_mw_sensor_fusion_set_mode(board, MBL_MW_SENSOR_FUSION_MODE_NDOF);
    // set acceleration range to +/-16G, note accelerometer is configured here
    mbl_mw_sensor_fusion_set_acc_range(board, MBL_MW_SENSOR_FUSION_ACC_RANGE_16G);
    // set gyro range to 2000 DPS
    mbl_mw_sensor_fusion_set_gyro_range(board, MBL_MW_SENSOR_FUSION_GYRO_RANGE_2000DPS);
    // write changes to the board
    mbl_mw_sensor_fusion_write_config(board);
    
    // set the tx power as high as allowed
    if (mbl_mw_metawearboard_get_model(board) == MBL_MW_MODEL_METAMOTION_S){
        mbl_mw_settings_set_tx_power(board, 8);
    } else {
        mbl_mw_settings_set_tx_power(board, 4);
    }
}

void MetaWearInterface::get_current_power_status(MblMwMetaWearBoard* board) {
    auto power_status = mbl_mw_settings_get_power_status_data_signal(board);
    mbl_mw_datasignal_subscribe(power_status, this, [](void* context, const MblMwData* data) -> void {
        auto *wrapper = static_cast<MetaWearInterface *>(context);
        //std::cout << "Power Status: " << data << std::endl;
    });
    
    auto charge_status = mbl_mw_settings_get_charge_status_data_signal(board);
    mbl_mw_datasignal_subscribe(charge_status, this, [](void* context, const MblMwData* data) -> void {
        auto *wrapper = static_cast<MetaWearInterface *>(context);
        //std::cout << "Charge Status: " << data << std::endl;
    });
}

void MetaWearInterface::get_battery_percentage(MblMwMetaWearBoard* board) {
    auto battery_signal = mbl_mw_settings_get_battery_state_data_signal(board);
    if (battery_signal != NULL) { // seems to be inaccessible at first
        mbl_mw_datasignal_subscribe(battery_signal, this, [](void* context, const MblMwData* data) -> void {
            auto *wrapper = static_cast<MetaWearInterface *>(context);
            auto state = (MblMwBatteryState*) data->value;
            wrapper->battery_level = state->charge;
            //printf("{voltage: %dmV, charge: %d}\n", state->voltage, state->charge);
        });
        mbl_mw_datasignal_read(battery_signal);
    }
}

void MetaWearInterface::set_ad_name(MblMwMetaWearBoard* board) {
    const char* charName;
    if (mbl_mw_metawearboard_get_model(board) == MBL_MW_MODEL_METAMOTION_S){
        charName = "Mach1-MMS";
    } else if (mbl_mw_metawearboard_get_model(board) == MBL_MW_MODEL_METAMOTION_RL){
        charName = "Mach1-MMRL";
    } else if (mbl_mw_metawearboard_get_model(board) == MBL_MW_MODEL_METAWEAR_R){
        charName = "Mach1-MMR";
    } else {
        charName = "MetaWear";
    }
    size_t length = strlen(charName) + 1;
    const char* beg = charName;
    const char* end = charName + length;
    uint8_t* name = new uint8_t[length];
    size_t i = 0;
    for (; beg != end; ++beg, ++i){
        name[i] = (uint8_t)(*beg);
    }
    mbl_mw_settings_set_device_name(board, name, strlen(charName));
}

void MetaWearInterface::get_ad_name(MblMwMetaWearBoard* board){
    // This function is for calling the name via metamotion
    module_name = mbl_mw_metawearboard_get_model_name(board);
}

void MetaWearInterface::enable_fusion_sampling(MblMwMetaWearBoard* board) {
    // Write the config to the sensor
    configure_sensor_fusion(board);
    
    auto fusion_signal = mbl_mw_sensor_fusion_get_data_signal(board, MBL_MW_SENSOR_FUSION_DATA_EULER_ANGLE);
    mbl_mw_datasignal_subscribe(fusion_signal, this, [](void* context, const MblMwData* data) -> void {
        auto *wrapper = static_cast<MetaWearInterface *>(context);
        
        auto euler = (MblMwEulerAngles*)data->value;
        if (wrapper->bUseMagnoHeading){ // externally set use of magnometer to correct IMU or not
            wrapper->outputEuler[0] = euler->heading;
            wrapper->outputEuler[3] = euler->yaw;
        } else {
            wrapper->outputEuler[0] = euler->yaw;
            wrapper->outputEuler[3] = euler->heading;
        }
        wrapper->outputEuler[1] = euler->pitch;
        wrapper->outputEuler[2] = euler->roll;
        //printf("(%.3f, %.3f, %.3f)\n", euler->yaw, euler->pitch, euler->roll);
    });
    
    // Start
    mbl_mw_sensor_fusion_enable_data(board, MBL_MW_SENSOR_FUSION_DATA_EULER_ANGLE);
    mbl_mw_sensor_fusion_start(board);
    enable_led(board);
}

void MetaWearInterface::enable_led(MblMwMetaWearBoard* board) {
    MblMwLedPattern pattern;
    pattern = { 8, 0, 250, 250, 250, 5000, 0, 0 };
    mbl_mw_led_write_pattern(board, &pattern, MBL_MW_LED_COLOR_RED);
    mbl_mw_led_play(board);
}

void MetaWearInterface::disable_led(MblMwMetaWearBoard* board) {
    // stop the LED pattern from playing
    mbl_mw_led_stop_and_clear(board);
}

void MetaWearInterface::disable_fusion_sampling(MblMwMetaWearBoard* board) {
    auto fusion_signal = mbl_mw_sensor_fusion_get_data_signal(board, MBL_MW_SENSOR_FUSION_DATA_EULER_ANGLE);
    mbl_mw_datasignal_unsubscribe(fusion_signal);
    mbl_mw_sensor_fusion_stop(board);
}

void MetaWearInterface::calibration_mode(MblMwMetaWearBoard* board) {
    
}

void MetaWearInterface::resetOrientation() {
    for(int i=0; i< 3; i++){
        angle_shift[i] = 0;
    }
}

void MetaWearInterface::recenter() {
    float* swpAngle = getAngle();
    angle_shift[0] = angle_shift[0] - swpAngle[0];
    angle_shift[1] = angle_shift[1] - swpAngle[1];
    angle_shift[2] = angle_shift[2] - swpAngle[2];
}

float* MetaWearInterface::getAngle() {
    float* calculated_angle = new float[3];
    calculated_angle[0] = outputEuler[0] + angle_shift[0];
    calculated_angle[1] = outputEuler[1] + angle_shift[1];
    calculated_angle[2] = outputEuler[2] + angle_shift[2];
    return calculated_angle;
}

int MetaWearInterface::getBatteryLevel() {
    get_battery_percentage(board);
    return battery_level;
}

std::string HighLow2Uuid(const uint64_t high, const uint64_t low){
    uint8_t *u_h = (uint8_t *)&(high);
    uint8_t *u_l = (uint8_t *)&(low);

    char UUID[38];
    std::sprintf(UUID, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            u_h[7], u_h[6], u_h[5], u_h[4], u_h[3], u_h[2], u_h[1], u_h[0],
            u_l[7], u_l[6], u_l[5], u_l[4], u_l[3], u_l[2], u_l[1], u_l[0]
            );
    
    return std::string(UUID);
}

void MetaWearInterface::read_gatt_char(void *context, const void *caller, const MblMwGattChar *characteristic, MblMwFnIntVoidPtrArray handler) {
    auto *wrapper = static_cast<MetaWearInterface *>(context);
    
    if(wrapper->deviceInterface == nullptr) return;

    auto readByteArray = wrapper->deviceInterface->read(HighLow2Uuid(characteristic->service_uuid_high, characteristic->service_uuid_low), HighLow2Uuid(characteristic->uuid_high, characteristic->uuid_low)).value_or("UNKNOWN");
                                                     
    handler(caller, (uint8_t*)readByteArray.data(), readByteArray.length());
}

void MetaWearInterface::write_gatt_char(void *context, const void *caller, MblMwGattCharWriteType writeType, const MblMwGattChar *characteristic, const uint8_t *value, uint8_t length){
    auto *wrapper = static_cast<MetaWearInterface *>(context);

    if(wrapper->deviceInterface == nullptr) return;

    wrapper->deviceInterface->write_command(HighLow2Uuid(characteristic->service_uuid_high, characteristic->service_uuid_low), HighLow2Uuid(characteristic->uuid_high, characteristic->uuid_low), std::string((char*)value, int(length)));
}

void MetaWearInterface::enable_char_notify(void *context, const void *caller, const MblMwGattChar *characteristic, MblMwFnIntVoidPtrArray handler, MblMwFnVoidVoidPtrInt ready) {
    auto *wrapper = static_cast<MetaWearInterface *>(context);

    if(wrapper->deviceInterface == nullptr) return;

    wrapper->deviceInterface->notify(HighLow2Uuid(characteristic->service_uuid_high, characteristic->service_uuid_low), HighLow2Uuid(characteristic->uuid_high, characteristic->uuid_low), [&,handler,caller](SimpleBLE::ByteArray payload) {
            handler(caller,(uint8_t*)payload.data(),payload.length());
        });
    ready(caller, MBL_MW_STATUS_OK);
}

void MetaWearInterface::on_disconnect(void *context, const void *caller, MblMwFnVoidVoidPtrInt handler) {
    //auto *wrapper = static_cast<MetaWearInterface *>(context);
    handler(caller, 0);
}
