/*
*    Created by: Andrew O'Shei
* 
*    Lookup BLE Appearance Based on ID
* 
*/

#include "ble_appearance.h"

const uint16_t idLookUp[295] = {
    0x0000,0x0040,0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,
    0x0088,0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,0x00C0,0x00C1,
    0x00C2,0x0100,0x0140,0x0180,0x01C0,0x0200,0x0240,0x0280,0x02C0,0x0300,
    0x0301,0x0340,0x0341,0x0380,0x0381,0x0382,0x03C0,0x03C1,0x03C2,0x03C3,
    0x03C4,0x03C5,0x03C6,0x03C7,0x03C8,0x0400,0x0440,0x0441,0x0442,0x0443,
    0x0480,0x0481,0x0482,0x0483,0x0484,0x0485,0x04C0,0x04C1,0x04C2,0x04C3,
    0x04C4,0x04C5,0x04C6,0x04C7,0x04C8,0x04C9,0x04CA,0x04CB,0x04CC,0x0500,
    0x0501,0x0502,0x0503,0x0540,0x0541,0x0542,0x0543,0x0544,0x0545,0x0546,
    0x0547,0x0548,0x0549,0x054A,0x054B,0x054C,0x054D,0x054E,0x054F,0x0550,
    0x0551,0x0552,0x0553,0x0554,0x0555,0x0556,0x0557,0x0558,0x0559,0x0580,
    0x0581,0x0582,0x0583,0x0584,0x0585,0x0586,0x0587,0x0588,0x0589,0x058A,
    0x058B,0x058C,0x058D,0x058E,0x058F,0x0590,0x0591,0x0592,0x0593,0x0594,
    0x0595,0x0596,0x0597,0x0598,0x0599,0x05C0,0x05C1,0x05C2,0x05C3,0x05C4,
    0x05C5,0x05C6,0x0600,0x0601,0x0602,0x0603,0x0604,0x0605,0x0606,0x0607,
    0x0608,0x0609,0x060A,0x060B,0x0640,0x0680,0x06C0,0x06C1,0x06C2,0x06C3,
    0x06C4,0x06C5,0x06C6,0x06C7,0x0700,0x0701,0x0702,0x0703,0x0704,0x0705,
    0x0706,0x0707,0x0708,0x0709,0x0740,0x0741,0x0742,0x0743,0x0744,0x0745,
    0x0780,0x0781,0x0782,0x0783,0x0784,0x0785,0x0786,0x0787,0x0788,0x0789,
    0x07C0,0x07C1,0x07C2,0x07C3,0x07C4,0x07C5,0x07C6,0x0800,0x0801,0x0802,
    0x0803,0x0804,0x0805,0x0806,0x0840,0x0841,0x0842,0x0843,0x0844,0x0845,
    0x0880,0x0881,0x0882,0x0883,0x0884,0x0885,0x0886,0x0887,0x0888,0x0889,
    0x08C0,0x08C1,0x08C2,0x08C3,0x08C4,0x08C5,0x08C6,0x08C7,0x08C8,0x08C9,
    0x08CA,0x08CB,0x08CC,0x08CD,0x08CE,0x08CF,0x0900,0x0901,0x0902,0x0903,
    0x0904,0x0905,0x0906,0x0907,0x0908,0x0909,0x090A,0x090B,0x090C,0x090D,
    0x090E,0x090F,0x0940,0x0941,0x0942,0x0943,0x0944,0x0980,0x0981,0x0982,
    0x0983,0x0984,0x09C0,0x09C1,0x09C2,0x09C3,0x09C4,0x09C5,0x09C6,0x09C7,
    0x09C8,0x09C9,0x09CA,0x0A00,0x0A01,0x0A02,0x0A03,0x0A40,0x0A41,0x0A42,
    0x0A43,0x0A80,0x0A81,0x0A82,0x0AC0,0x0AC1,0x0AC2,0x0C40,0x0C41,0x0C42,
    0x0C80,0x0CC0,0x0CC1,0x0CC2,0x0D00,0x0D40,0x0D41,0x0D44,0x0D48,0x0D80,
    0x1440,0x1441,0x1442,0x1443,0x1444
};


const String bleAppearance[295][2] = {
    // Category                 // Sub-Category             // Value
    {"Unknown",                 "Generic"},                 // 0x0000
    {"Phone",                   "Generic"},                 // 0x0040
    {"Computer",                "Generic"},                 // 0x0080
    {"Computer",                "Desktop Workstation"},     // 0x0081
    {"Computer",                "Server-Class Computer"},   // 0x0082
    {"Computer",                "Laptop"},                  // 0x0083
    {"Computer",                "Handheld PDA"},            // 0x0084
    {"Computer",                "Palm-size PDA"},           // 0x0085
    {"Computer",                "Wearable Computer"},       // 0x0086
    {"Computer",                "Tablet"},                  // 0x0087
    {"Computer",                "Docking Station"},         // 0x0088
    {"Computer",                "All in One"},              // 0x0089
    {"Computer",                "Blade Server"},            // 0x008A
    {"Computer",                "Convertible"},             // 0x008B
    {"Computer",                "Detachable"},              // 0x008C
    {"Computer",                "IoT Gateway"},             // 0x008D
    {"Computer",                "Mini PC"},                 // 0x008E
    {"Computer",                "Stick PC"},                // 0x008F
    {"Watch",                   "Generic"},                 // 0x00C0
    {"Watch",                   "Sports Watch"},            // 0x00C1
    {"Watch",                   "Smart Watch"},             // 0x00C2
    {"Clock",                   "Generic"},                 // 0x0100
    {"Display",                 "Generic"},                 // 0x0140
    {"Remote Control",          "Generic"},                 // 0x0180
    {"Eye-Glasses",             "Generic"},                 // 0x01C0
    {"Tag",                     "Generic"},                 // 0x0200
    {"Keyring",                 "Generic"},                 // 0x0240
    {"Media Player",            "Generic"},                 // 0x0280
    {"Barcode Scanner",         "Generic"},                 // 0x02C0
    {"Thermometer",             "Generic"},                 // 0x0300
    {"Thermometer",             "Ear Thermometer"},         // 0x0301
    {"Heart Rate Sensor",       "Generic"},                 // 0x0340
    {"Heart Rate Sensor",       "Heart Rate Belt"},         // 0x0341
    {"Blood Pressure",          "Generic"},                 // 0x0380
    {"Blood Pressure",          "Arm Blood Pressure"},      // 0x0381
    {"Blood Pressure",          "Wrist Blood Pressure"},    // 0x0382
    {"Human Interface Device",  "Generic"},                 // 0x03C0
    {"Human Interface Device",  "Keyboard"},                // 0x03C1
    {"Human Interface Device",  "Mouse"},                   // 0x03C2
    {"Human Interface Device",  "Joystick"},                // 0x03C3
    {"Human Interface Device",  "Gamepad"},                 // 0x03C4
    {"Human Interface Device",  "Digitizer Tablet"},        // 0x03C5
    {"Human Interface Device",  "Card Reader"},             // 0x03C6
    {"Human Interface Device",  "Digital Pen"},             // 0x03C7
    {"Human Interface Device",  "Barcode Scanner"},         // 0x03C8
    {"Glucose Meter",           "Generic"},                 // 0x0400
    {"Running Walking Sensor",  "Generic"},                 // 0x0440
    {"Running Walking Sensor",  "In-Shoe"},                 // 0x0441
    {"Running Walking Sensor",  "On-Shoe"},                 // 0x0442
    {"Running Walking Sensor",  "On-Hip"},                  // 0x0443
    {"Cycling",                 "Generic"},                 // 0x0480
    {"Cycling",                 "Cycling Computer"},        // 0x0481
    {"Cycling",                 "Speed Sensor"},            // 0x0482
    {"Cycling",                 "Cadence Sensor"},          // 0x0483
    {"Cycling",                 "Power Sensor"},            // 0x0484
    {"Cycling",                 "Speed and Cadence"},       // 0x0485
    {"Control Device",          "Generic"},                 // 0x04C0
    {"Control Device",          "Switch"},                  // 0x04C1
    {"Control Device",          "Multi-Switch"},            // 0x04C2
    {"Control Device",          "Button"},                  // 0x04C3
    {"Control Device",          "Slider"},                  // 0x04C4
    {"Control Device",          "Rotary Switch"},           // 0x04C5
    {"Control Device",          "Touch Panel"},             // 0x04C6
    {"Control Device",          "Single Switch"},           // 0x04C7
    {"Control Device",          "Double Switch"},           // 0x04C8
    {"Control Device",          "Triple Switch"},           // 0x04C9
    {"Control Device",          "Battery Switch"},          // 0x04CA
    {"Control Device",          "Energy Harvesting Switch"},// 0x04CB
    {"Control Device",          "Push Button"},             // 0x04CC   
    {"Network Device",          "Generic"},                 // 0x0500
    {"Network Device",          "Access Point"},            // 0x0501
    {"Network Device",          "Mesh Device"},             // 0x0502
    {"Network Device",          "Mesh Network Proxy"},      // 0x0503
    {"Sensor",                  "Generic"},                 // 0x0540
    {"Sensor",                  "Motion"},                  // 0x0541
    {"Sensor",                  "Air Quality"},             // 0x0542
    {"Sensor",                  "Temperature"},             // 0x0543
    {"Sensor",                  "Humidity"},                // 0x0544
    {"Sensor",                  "Leak"},                    // 0x0545
    {"Sensor",                  "Smoke"},                   // 0x0546
    {"Sensor",                  "Occupancy"},               // 0x0547
    {"Sensor",                  "Contact"},                 // 0x0548
    {"Sensor",                  "Carbon Monoxide"},         // 0x0549
    {"Sensor",                  "Carbon Dioxide"},          // 0x054A
    {"Sensor",                  "Ambient Light"},           // 0x054B
    {"Sensor",                  "Energy"},                  // 0x054C
    {"Sensor",                  "Color Light"},             // 0x054D
    {"Sensor",                  "Rain"},                    // 0x054E
    {"Sensor",                  "Fire"},                    // 0x054F
    {"Sensor",                  "Wind"},                    // 0x0550
    {"Sensor",                  "Proximity"},               // 0x0551
    {"Sensor",                  "Multi-Sensor"},            // 0x0552
    {"Sensor",                  "Flash Mounted"},           // 0x0553
    {"Sensor",                  "Ceiling Mounted"},         // 0x0554
    {"Sensor",                  "Wall Mounted"},            // 0x0555
    {"Sensor",                  "Multisensor"},             // 0x0556
    {"Sensor",                  "Energy Meter"},            // 0x0557
    {"Sensor",                  "Flame Detector"},          // 0x0558
    {"Sensor",                  "Vehicle Tire Pressure"},   // 0x0559
    {"Light Fixtures",          "Generic"},                 // 0x0580
    {"Light Fixtures",          "Wall Light"},              // 0x0581
    {"Light Fixtures",          "Ceiling Light"},           // 0x0582
    {"Light Fixtures",          "Floor Light"},             // 0x0583
    {"Light Fixtures",          "Cabinet Light"},           // 0x0584
    {"Light Fixtures",          "Desk Light"},              // 0x0585
    {"Light Fixtures",          "Troffer Light"},           // 0x0586
    {"Light Fixtures",          "Pendant Light"},           // 0x0587
    {"Light Fixtures",          "In-Ground Light"},         // 0x0588
    {"Light Fixtures",          "Flood Light"},             // 0x0589
    {"Light Fixtures",          "Underwater Light"},        // 0x058A
    {"Light Fixtures",          "Bollard Light"},           // 0x058B
    {"Light Fixtures",          "Pathway Light"},           // 0x058C
    {"Light Fixtures",          "Garden Light"},            // 0x058D
    {"Light Fixtures",          "Pole-Top Light"},          // 0x058E
    {"Light Fixtures",          "Spotlight"},               // 0x058F
    {"Light Fixtures",          "Linear Light"},            // 0x0590
    {"Light Fixtures",          "Street Light"},            // 0x0591
    {"Light Fixtures",          "Shelves Light"},           // 0x0592
    {"Light Fixtures",          "Bay Light"},               // 0x0593
    {"Light Fixtures",          "Emergency Exit"},          // 0x0594
    {"Light Fixtures",          "Light Controller"},        // 0x0595
    {"Light Fixtures",          "Light Driver"},            // 0x0596
    {"Light Fixtures",          "Bulb"},                    // 0x0597
    {"Light Fixtures",          "Low-Bay Light"},           // 0x0598
    {"Light Fixtures",          "High-Bay Light"},          // 0x0599
    {"Fan",                     "Generic Fan"},             // 0x05C0
    {"Fan",                     "Ceiling Fan"},             // 0x05C1
    {"Fan",                     "Axial Fan"},               // 0x05C2
    {"Fan",                     "Exhaust Fan"},             // 0x05C3
    {"Fan",                     "Pedestal Fan"},            // 0x05C4
    {"Fan",                     "Desk Fan"},                // 0x05C5
    {"Fan",                     "Wall Fan"},                // 0x05C6
    {"HVAC",                    "Generic"},                 // 0x0600
    {"HVAC",                    "Thermostat"},              // 0x0601
    {"HVAC",                    "Humidifier"},              // 0x0602
    {"HVAC",                    "De­humidifier"},           // 0x0603
    {"HVAC",                    "Heater"},                  // 0x0604
    {"HVAC",                    "Radiator"},                // 0x0605
    {"HVAC",                    "Boiler"},                  // 0x0606
    {"HVAC",                    "Heat Pump"},               // 0x0607
    {"HVAC",                    "Infrared Heater"},         // 0x0608
    {"HVAC",                    "Radiant Panel Heater"},    // 0x0609
    {"HVAC",                    "Fan Heater"},              // 0x060A
    {"HVAC",                    "Air Curtain"},             // 0x060B
    {"Air Conditioning",        "Generic"},                 // 0x0640
    {"Humidifier",              "Generic"},                 // 0x0680
    {"Heating",                 "Generic"},                 // 0x06C0
    {"Heating",                 "Radiator"},                // 0x06C1
    {"Heating",                 "Boiler"},                  // 0x06C2
    {"Heating",                 "Heat Pump"},               // 0x06C3
    {"Heating",                 "Infrared Heater"},         // 0x06C4
    {"Heating",                 "Radiant Panel Heater"},    // 0x06C5
    {"Heating",                 "Fan Heater"},              // 0x06C6
    {"Heating",                 "Air Curtain"},             // 0x06C7
    {"Access Control",          "Generic"},                 // 0x0700
    {"Access Control",          "Access Door"},             // 0x0701
    {"Access Control",          "Garage Door"},             // 0x0702
    {"Access Control",          "Emergency Exit Door"},     // 0x0703
    {"Access Control",          "Access Lock"},             // 0x0704
    {"Access Control",          "Elevator"},                // 0x0705
    {"Access Control",          "Window"},                  // 0x0706
    {"Access Control",          "Entrance Gate"},           // 0x0707
    {"Access Control",          "Door Lock"},               // 0x0708
    {"Access Control",          "Locker"},                  // 0x0709
    {"Motorized Device",        "Generic"},                 // 0x0740
    {"Motorized Device",        "Motorized Gate"},          // 0x0741
    {"Motorized Device",        "Awning"},                  // 0x0742
    {"Motorized Device",        "Blinds or Shades"},        // 0x0743
    {"Motorized Device",        "Curtains"},                // 0x0744
    {"Motorized Device",        "Screen"},                  // 0x0745
    {"Power Device",            "Generic"},                 // 0x0780
    {"Power Device",            "Power Outlet"},            // 0x0781
    {"Power Device",            "Power Strip"},             // 0x0782
    {"Power Device",            "Plug"},                    // 0x0783
    {"Power Device",            "Power Supply"},            // 0x0784
    {"Power Device",            "LED Driver"},              // 0x0785
    {"Power Device",            "Fluorescent Lamp Gear"},   // 0x0786
    {"Power Device",            "HID Lamp Gear"},           // 0x0787
    {"Power Device",            "Charge Case"},             // 0x0788
    {"Power Device",            "Power Bank"},              // 0x0789
    {"Light Source",            "Generic"},                 // 0x07C0
    {"Light Source",            "Incandescent Bulb"},       // 0x07C1
    {"Light Source",            "LED Lamp"},                // 0x07C2
    {"Light Source",            "HID Lamp"},                // 0x07C3
    {"Light Source",            "Fluorescent Lamp"},        // 0x07C4
    {"Light Source",            "LED Array"},               // 0x07C5
    {"Light Source",            "Multi-Color LED Array"},   // 0x07C6
    {"Window Covering",         "Generic"},                 // 0x0800
    {"Window Covering",         "Window Shades"},           // 0x0801
    {"Window Covering",         "Window Blinds"},           // 0x0802
    {"Window Covering",         "Window Awning"},           // 0x0803
    {"Window Covering",         "Window Curtain"},          // 0x0804
    {"Window Covering",         "Exterior Shutter"},        // 0x0805
    {"Window Covering",         "Exterior Screen"},         // 0x0806
    {"Audio Sink",              "Generic"},                 // 0x0840
    {"Audio Sink",              "Standalone Speaker"},      // 0x0841
    {"Audio Sink",              "Soundbar"},                // 0x0842
    {"Audio Sink",              "Bookshelf Speaker"},       // 0x0843
    {"Audio Sink",              "Standmounted Speaker"},    // 0x0844
    {"Audio Sink",              "Speakerphone"},            // 0x0845
    {"Audio Source",            "Generic"},                 // 0x0880
    {"Audio Source",            "Microphone"},              // 0x0881
    {"Audio Source",            "Alarm"},                   // 0x0882
    {"Audio Source",            "Bell"},                    // 0x0883
    {"Audio Source",            "Horn"},                    // 0x0884
    {"Audio Source",            "Broadcasting Device"},     // 0x0885
    {"Audio Source",            "Service Desk"},            // 0x0886
    {"Audio Source",            "Kiosk"},                   // 0x0887
    {"Audio Source",            "Broadcasting Room"},       // 0x0888
    {"Audio Source",            "Auditorium"},              // 0x0889
    {"Motorized Vehicle",       "Generic"},                 // 0x08C0
    {"Motorized Vehicle",       "Car"},                     // 0x08C1
    {"Motorized Vehicle",       "Large Goods Vehicle"},     // 0x08C2
    {"Motorized Vehicle",       "2­Wheeled Vehicle"},       // 0x08C3
    {"Motorized Vehicle",       "Motorbike"},               // 0x08C4
    {"Motorized Vehicle",       "Scooter"},                 // 0x08C5
    {"Motorized Vehicle",       "Moped"},                   // 0x08C6
    {"Motorized Vehicle",       "3­Wheeled Vehicle"},       // 0x08C7
    {"Motorized Vehicle",       "Light Vehicle"},           // 0x08C8
    {"Motorized Vehicle",       "Quad Bike"},               // 0x08C9
    {"Motorized Vehicle",       "Minibus"},                 // 0x08CA
    {"Motorized Vehicle",       "Bus"},                     // 0x08CB
    {"Motorized Vehicle",       "Trolley"},                 // 0x08CC
    {"Motorized Vehicle",       "Agricultural Vehicle"},    // 0x08CD
    {"Motorized Vehicle",       "Camper or Caravan"},       // 0x08CE
    {"Motorized Vehicle",       "Recreational Vehicle"},    // 0x08CF
    {"Domestic Appliance",      "Generic"},                 // 0x0900
    {"Domestic Appliance",      "Refrigerator"},            // 0x0901
    {"Domestic Appliance",      "Freezer"},                 // 0x0902
    {"Domestic Appliance",      "Oven"},                    // 0x0903
    {"Domestic Appliance",      "Microwave"},               // 0x0904
    {"Domestic Appliance",      "Toaster"},                 // 0x0905
    {"Domestic Appliance",      "Washing Machine"},         // 0x0906
    {"Domestic Appliance",      "Dryer"},                   // 0x0907
    {"Domestic Appliance",      "Coffee maker"},            // 0x0908
    {"Domestic Appliance",      "Clothes iron"},            // 0x0909
    {"Domestic Appliance",      "Curling iron"},            // 0x090A
    {"Domestic Appliance",      "Hair dryer"},              // 0x090B
    {"Domestic Appliance",      "Vacuum cleaner"},          // 0x090C
    {"Domestic Appliance",      "Robotic vacuum cleaner"},  // 0x090D
    {"Domestic Appliance",      "Rice cooker"},             // 0x090E
    {"Domestic Appliance",      "Clothes steamer"},         // 0x090F
    {"Wearable Audio Device",   "Generic"},                 // 0x0940
    {"Wearable Audio Device",   "Earbud"},                  // 0x0941
    {"Wearable Audio Device",   "Headset"},                 // 0x0942
    {"Wearable Audio Device",   "Headphones"},              // 0x0943
    {"Wearable Audio Device",   "Neck Band"},               // 0x0944
    {"Aircraft",                "Generic"},                 // 0x0980
    {"Aircraft",                "Light Aircraft"},          // 0x0981
    {"Aircraft",                "Microlight"},              // 0x0982
    {"Aircraft",                "Paraglider"},              // 0x0983
    {"Aircraft",                "Large Passenger Aircraft"},// 0x0984
    {"AV Equipment",            "Generic"},                 // 0x09C0
    {"AV Equipment",            "Amplifier"},               // 0x09C1
    {"AV Equipment",            "Receiver"},                // 0x09C2
    {"AV Equipment",            "Radio"},                   // 0x09C3
    {"AV Equipment",            "Tuner"},                   // 0x09C4
    {"AV Equipment",            "Turntable"},               // 0x09C5
    {"AV Equipment",            "CD Player"},               // 0x09C6
    {"AV Equipment",            "DVD Player"},              // 0x09C7
    {"AV Equipment",            "Bluray Player"},           // 0x09C8
    {"AV Equipment",            "Optical Disc Player"},     // 0x09C9
    {"AV Equipment",            "Set­Top Box"},             // 0x09CA
    {"Display Equipment",       "Generic"},                 // 0x0A00
    {"Display Equipment",       "Television"},              // 0x0A01
    {"Display Equipment",       "Monitor"},                 // 0x0A02
    {"Display Equipment",       "Projector"},               // 0x0A03
    {"Hearing aid",             "Generic"},                 // 0x0A40
    {"Hearing aid",             "In­ear hearing aid"},      // 0x0A41
    {"Hearing aid",             "Behind­ear hearing aid"},  // 0x0A42
    {"Hearing aid",             "Cochlear Implant"},        // 0x0A43
    {"Gaming",                  "Generic"},                 // 0x0A80
    {"Gaming",                  "Home Video Game Console"}, // 0x0A81
    {"Gaming",                  "Handheld console"},        // 0x0A82
    {"Signage",                 "Generic"},                 // 0x0AC0
    {"Signage",                 "Digital Signage"},         // 0x0AC1
    {"Signage",                 "Electronic Label"},        // 0x0AC2
    {"Pulse Oximeter",          "Generic"},                 // 0x0C40
    {"Pulse Oximeter",          "Fingertip"},               // 0x0C41
    {"Pulse Oximeter",          "Wrist Worn"},              // 0x0C42
    {"Weight Scale",            "Generic"},                 // 0x0C80
    {"Personal Mobility Device","Generic"},                 // 0x0CC0
    {"Personal Mobility Device","Powered Wheelchair"},      // 0x0CC1
    {"Personal Mobility Device","Mobility Scooter"},        // 0x0CC2
    {"Continuous Glucose Monitor","Generic"},               // 0x0D00
    {"Insulin Pump",            "Generic"},                 // 0x0D40
    {"Insulin Pump",            "Durable Pump"},            // 0x0D41
    {"Insulin Pump",            "Patch Pump"},              // 0x0D44
    {"Insulin Pump",            "Insulin Pen"},             // 0x0D48
    {"Medication Delivery",     "Generic"},                 // 0x0D80
    {"Outdoor Sports Activity", "Generic"},                 // 0x1440
    {"Outdoor Sports Activity", "Location Display"},        // 0x1441
    {"Outdoor Sports Activity", "Navigation Display"},      //0x1442
    {"Outdoor Sports Activity", "Location Pod"},            // 0x1443
    {"Outdoor Sports Activity", "Navigation Pod"}           // 0x1444
};

String getBLEAppearance(uint16_t id) {
    bool found = false;
    int index = 0;
    for (index; index < 295; index++) {
        if (id == idLookUp[index]) {
            found = true;
            break;
        }
    }
    if (!found) {
        return "Unknown Appearance";
    } else {
        return bleAppearance[index][0] + ": " + bleAppearance[index][1];
    }
}
