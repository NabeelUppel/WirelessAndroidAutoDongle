#include <stdio.h>
#include <unistd.h>

#include "common.h"
#include "bluetoothHandler.h"
#include "connectionStrategyManager.h"
#include "proxyHandler.h"
#include "uevent.h"
#include "usb.h"


int main(void) {
    Logger::instance()->info("AA Wireless Dongle\n");

    // Global init
    std::optional<std::thread> ueventThread =  UeventMonitor::instance().start();
    UsbManager::instance().init();
    BluetoothHandler::instance().init();
    ConnectionStrategyManager::instance().init();

    ConnectionStrategy::instance().start();
    ueventThread->join();

    return 0;
}
