#include <dynamixel_ros_library.h>
#include <ros/ros.h>

dynamixel::PortHandler *myPortHandler = nullptr;
dynamixel::PacketHandler *myPacketHandler = nullptr;

std::map<int, std::string> dynamixelMotor::DMXL_MODELS = {
    // XW SERIES
    {1180, "XW540-T140"},
    {1170, "XW540-T260"},
    {1280, "XW430-T200"},
    {1270, "XW430-T333"},

    // XD SERIES
    {1111, "XD540-T150"},
    {1101, "XD540-T270"},
    {1011, "XD430-T210"},
    {1001, "XD430-T350"},

    // XH SERIES
    {1110, "XH540-W150"},
    {1100, "XH540-W270"},
    {1150, "XH540-V150"},
    {1140, "XH540-V270"},
    {1010, "XH430-W210"},
    {1000, "XH430-W350"},
    {1050, "XH430-V210"},
    {1040, "XH430-V350"},

    // XM SERIES
    {1130, "XM540-W150"},
    {1120, "XM540-W270"},
    {1030, "XM430-W210"},
    {1020, "XM430-W350"},

    // XC SERIES
    {1160, "2XC430-W250"},
    {1070, "XC430-W150"},
    {1080, "XC430-W240"},
    {1220, "XC330-T288"},
    {1210, "XC330-T181"},
    {1230, "XC330-M181"},
    {1240, "XC330-M288"},

    // XL SERIES
    {1060, "XL430-W250"},
    {1190, "XL330-M077"},
    {1200, "XL330-M288"},
    {350, "XL320"}
};

// Compatibility: XW, XD430, XH430, XM430,
std::map<std::string, int> dynamixelMotor::ADDR_DMXL22 = {
    // EEPROM AREA
    {"MODEL_NUMBER", 0},
    {"MODEL_INFORMATION", 2},
    {"FIRMWARE_VERSION", 6},
    {"ID", 7},
    {"BAUDRATE", 8},
    {"RETURN_DELAY_TIME", 9},
    {"DRIVE_MODE", 10},
    {"OPERATING_MODE", 11},
    {"SECONDARY_ID", 12},
    {"PROTOCOL_TYPE", 13},
    {"HOMING_OFFSET", 20},
    {"MOVING_THRESHOLD", 24},
    {"TEMPERATURE_LIMIT", 31},
    {"MAX_VOLTAGE_LIMIT", 32},
    {"MIN_VOLTAGE_LIMIT", 34},
    {"PWM_LIMIT", 36},
    {"CURRENT_LIMIT", 38},
    {"VELOCITY_LIMIT", 44},
    {"MAX_POSITION_LIMIT", 48},
    {"MIN_POSITION_LIMIT", 52},
    {"STARTUP_CONFIGURATION", 60},
    {"SHUTDOWN", 63}
};

// Compatibility: XD540, XH540, XM540, 
std::map<std::string, int> dynamixelMotor::ADDR_DMXL25 = {
    // EEPROM AREA
    {"MODEL_NUMBER", 0},
    {"MODEL_INFORMATION", 2},
    {"FIRMWARE_VERSION", 6},
    {"ID", 7},
    {"BAUDRATE", 8},
    {"RETURN_DELAY_TIME", 9},
    {"DRIVE_MODE", 10},
    {"OPERATING_MODE", 11},
    {"SECONDARY_ID", 12},
    {"PROTOCOL_TYPE", 13},
    {"HOMING_OFFSET", 20},
    {"MOVING_THRESHOLD", 24},
    {"TEMPERATURE_LIMIT", 31},
    {"MAX_VOLTAGE_LIMIT", 32},
    {"MIN_VOLTAGE_LIMIT", 34},
    {"PWM_LIMIT", 36},
    {"CURRENT_LIMIT", 38},
    {"VELOCITY_LIMIT", 44},
    {"MAX_POSITION_LIMIT", 48},
    {"MIN_POSITION_LIMIT", 52},
    {"EXTERNAL_PORT_MODE_1", 56},
    {"EXTERNAL_PORT_MODE_2", 57},
    {"EXTERNAL_PORT_MODE_3", 58},
    {"STARTUP_CONFIGURATION", 60},
    {"SHUTDOWN", 63}
};


dynamixelMotor::dynamixelMotor(std::string IDENTIFICATOR, int ID)
{
    this->IDENTIFICATOR = IDENTIFICATOR;
    this->ID = ID;
}

dynamixelMotor::~dynamixelMotor()
{
    
}

bool dynamixelMotor::iniComm(char* PORT_NAME, float PROTOCOL_VERSION, int BAUDRATE)
{
    myPortHandler = dynamixel::PortHandler::getPortHandler(PORT_NAME);
    myPacketHandler = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);

    if (!myPortHandler->openPort()) 
    {
        ROS_ERROR("Failed to open the port: %s !", PORT_NAME);
        return false;
    } else if (!myPortHandler->setBaudRate(BAUDRATE))  
    {
        ROS_ERROR("Failed to set the baudrate: %d !", BAUDRATE);
        return false;
    } else
    {
        ROS_INFO("Initialization success");
        return true;
    }
}

void dynamixelMotor::setControlTable()
{
    uint16_t *model_number = new uint16_t[1];
    uint8_t dxl_error;

    // Checking the DMXL model (adress 0 in all DMXLS)
    int dxl_comm_result = myPacketHandler->read2ByteTxRx(myPortHandler, this->ID, 0, model_number, &dxl_error);

    if(dxl_comm_result != COMM_SUCCESS)
    {
        ROS_ERROR("Error reading the model number of DMXL: %d",this->ID);
        ROS_ERROR("Error code: %d", dxl_error);
    } else {
        this->MODEL = static_cast<int>(*model_number); 

        switch(this->MODEL)
        {
            case 1180:
            case 1170:
            case 1280:
            case 1270:
            case 1011:
            case 1001:
            case 1010:
            case 1000:
            case 1050:
            case 1040:
            case 1030:
            case 1020:
            case 1160:
            case 1070:
            case 1080:
            case 1220:
            case 1210:
            case 1230:
            case 1240:
            case 1060:
            case 1190:
            case 1200:
            case 350:
                this->CONTROL_TABLE = ADDR_DMXL22;
            break;

            case 1111: 
            case 1101:
            case 1110:
            case 1100:
            case 1150:
            case 1140:
            case 1130:
            case 1120:
                this->CONTROL_TABLE = ADDR_DMXL25;
            break;

            // IN CASE OF UNRECOGNIZED MODEL NUMBER
            default:
                this->MODEL = 0;
            break;
        }

        ROS_INFO("Control table set for: %s", dynamixelMotor::DMXL_MODELS[this->MODEL].c_str());
    }
}

int dynamixelMotor::getID()
{
    return this->ID;
}

void dynamixelMotor::setID(int NEW_ID)
{
    int dxl_comm_result = 0;
    uint8_t dxl_error = 0;
    uint8_t *dato = new uint8_t[1];

    dxl_comm_result = myPacketHandler->write1ByteTxRx(myPortHandler,this->ID, this->CONTROL_TABLE["ID"], NEW_ID, &dxl_error);

    if (dxl_comm_result != COMM_SUCCESS)    
    {
        ROS_ERROR("Failed to change Dynamixels ID");
        ROS_ERROR("Error code: %d", dxl_error);
    }

    this->ID = NEW_ID;
}

std::string dynamixelMotor::getModel()
{
    return dynamixelMotor::DMXL_MODELS[this->MODEL];
}

int dynamixelMotor::getBaudrate()
{
    uint8_t dxl_error = 0;
    uint8_t *data = new uint8_t[1];
    int baudrate;
    bool max_baudrate_45M = (this->MODEL == 1180 || this->MODEL == 1170 || this->MODEL == 1280 || this->MODEL == 1270 || 
                             this->MODEL == 1111 || this->MODEL == 1101 || this->MODEL == 1011 || this->MODEL == 1001 ||
                             this->MODEL == 1110 || this->MODEL == 1100 || this->MODEL == 1150 || this->MODEL == 1140 ||
                             this->MODEL == 1010 || this->MODEL == 1000 || this->MODEL == 1050 || this->MODEL == 1040 ||
                             this->MODEL == 1130 || this->MODEL == 1120 || this->MODEL == 1030 || this->MODEL == 1020 ||
                             this->MODEL == 1160 || this->MODEL == 1070 || this->MODEL == 1080 || this->MODEL == 1060);

    bool max_baudrate_4M = (this->MODEL == 1220 || this->MODEL == 1210 || this->MODEL == 1230 || this->MODEL == 1240 || this->MODEL == 1190 || this->MODEL == 1200);
    bool max_baudrate_1M = this->MODEL == 350;
    int dxl_comm_result = myPacketHandler->read1ByteTxRx(myPortHandler, this->ID, this->CONTROL_TABLE["BAUDRATE"], data, &dxl_error);
        
    if(dxl_comm_result != COMM_SUCCESS)
    {
        ROS_ERROR("Failed to read from the LED for Dynamixel ID %d", this->ID);
        ROS_ERROR("Error code: %d", dxl_error);
        return 0;
    }

    if(max_baudrate_45M)
    {
        switch(static_cast<int>(*data))
        {
            case 0:
                baudrate = 9600;
            break;

            case 1:
                baudrate = 57600;
            break;

            case 2:
                baudrate = 115200;
            break;

            case 3:
                baudrate = 1e6;
            break;

            case 4:
                baudrate = 2e6;
            break;

            case 5:
                baudrate = 3e6;
            break;

            case 6:
                baudrate = 4e6;
            break;

            case 7:
                baudrate = 4.5e6;
            break;

            default:
                baudrate = 0;
            break;
        }
    } else if(max_baudrate_4M)
    {
        switch (static_cast<int>(*data))
        {
            case 0:
                baudrate = 9600;
            break;

            case 1:
                baudrate = 57600;
            break;

            case 2:
                baudrate = 115200;
            break;

            case 3:
                baudrate = 1e6;
            break;

            case 4:
                baudrate = 2e6;
            break;

            case 5:
                baudrate = 3e6;
            break;

            case 6:
                baudrate = 4e6;
            break;
        
            default:
                baudrate = 0;
            break;
        }
    } else if(max_baudrate_1M)
    {
        switch (static_cast<int>(*data))
        {
            case 0:
                baudrate = 9600;
            break;
        
            case 1:
                baudrate = 57600;
            break;

            case 2:
                baudrate = 115200;
            break;

            case 3:
                baudrate = 1e6;
            break;

            default:
                baudrate = 0;
            break;
        }
    }
    
    ROS_INFO("Getting baudrate for Dynamixel ID: %d",this->ID);
    ROS_INFO("Current baudrate is: %d bps",baudrate);
    return baudrate;
}
