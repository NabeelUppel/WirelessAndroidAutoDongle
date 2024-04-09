class ConnectionStrategyManager {
public:
    static ConnectionStrategyManager& instance();
    void init();
    void start();

private:
    void dongleMode();
    void phoneFirst();
    void usbFirst();

};