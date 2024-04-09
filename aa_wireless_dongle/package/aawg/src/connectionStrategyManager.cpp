#include <stdio.h>
#include <unistd.h>

#include "common.h"
#include "bluetoothHandler.h"
#include "proxyHandler.h"
#include "uevent.h"
#include "usb.h"
#include "ConnectionStrategyManager.h"

ConnectionStrategyManager& ConnectionStrategyManager::instance(){
    static ConnectionStrategyManager instance;
    return instance;
}

void ConnectionStrategyManager::init() {
    // Init does not actually do anything but provides an intuitive place to cause the constructor call earlier than first use.

}

ConnectionStrategyManager::ConnectionStrategyManager(){

}

void ConnectionStrategyManager::start(){
    ConnectionStrategy connectionStrategy = Config::instance()->getConnectionStrategy();
    Logger::instance()->info("ConnectionStrategyManager: Connection Strategy: %d\n", connectionStrategy);
    if (connectionStrategy == ConnectionStrategy::DONGLE_MODE) {
        dongleMode();
    }
    if (connectionStrategy == ConnectionStrategy::PHONE_FIRST) {
        phoneFirst();
    }
    if (connectionStrategy == ConnectionStrategy::USB_FIRST) {
        usbFirst();
    }
}

void ConnectionStrategyManager::dongleMode(){
    BluetoothHandler::instance().powerOn();

    while (true) {
        AAWProxy proxy;
        std::optional<std::thread> proxyThread = proxy.startServer(Config::instance()->getWifiInfo().port);

        if (!proxyThread) {
            Logger::instance()->error("ConnectionStrategyManager: Failed to start proxy server thread\n");
            return;
        }

        std::optional<std::thread> btConnectionThread = BluetoothHandler::instance().connectWithRetry();

        proxyThread->join();

        if (btConnectionThread) {
            BluetoothHandler::instance().stopConnectWithRetry();
            btConnectionThread->join();
        }

        UsbManager::instance().disableGadget();
    }

}

void ConnectionStrategyManager::phoneFirst(){
    while (true) {
        AAWProxy proxy;
        std::optional<std::thread> proxyThread = proxy.startServer(Config::instance()->getWifiInfo().port);

        if (!proxyThread) {
            Logger::instance()->error("ConnectionStrategyManager: Failed to start proxy server thread\n");
            return;
        }

        BluetoothHandler::instance().powerOn();
        std::optional<std::thread> btConnectionThread = BluetoothHandler::instance().connectWithRetry();
        proxyThread->join();

        if (btConnectionThread) {
            BluetoothHandler::instance().stopConnectWithRetry();
            btConnectionThread->join();
        }

        UsbManager::instance().disableGadget();
        sleep(2);
    }

}

void ConnectionStrategyManager::usbFirst(){
    while (true) {
        Logger::instance()->info("ConnectionStrategyManager: Waiting for the accessory to connect first\n");
        UsbManager::instance().enableDefaultAndWaitForAccessory();
        AAWProxy proxy;
        std::optional<std::thread> proxyThread = proxy.startServer(Config::instance()->getWifiInfo().port);

        if (!proxyThread) {
            Logger::instance()->error("ConnectionStrategyManager: Failed to start proxy server thread\n");
            return;
        }
        BluetoothHandler::instance().powerOn();
        std::optional<std::thread> btConnectionThread = BluetoothHandler::instance().connectWithRetry();
        proxyThread->join();

        if (btConnectionThread) {
            BluetoothHandler::instance().stopConnectWithRetry();
            btConnectionThread->join();
        }

        UsbManager::instance().disableGadget();
        sleep(2);
    }

}