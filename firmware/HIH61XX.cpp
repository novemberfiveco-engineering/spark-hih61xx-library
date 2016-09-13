#include "HIH61XX.h"

HIH61XX::HIH61XX(uint8_t address, TwoWire& MyWire, uint8_t powerPin): a(address), p(powerPin), w(MyWire), f(0), h(0), t(0) {
    if (p < 255) {
        digitalWrite(p, LOW);
        pinMode(p, OUTPUT);
    }
}

uint8_t HIH61XX::start() {
    if (p < 255) {
        digitalWrite(p, HIGH);
    }
    f |= RunningFlag;
    return setError(0);
}

uint8_t HIH61XX::stop() {
    if (p < 255) {
        digitalWrite(p, LOW);
    }
    f &= ~RunningFlag;
    return setError(0);
}

uint8_t HIH61XX::update() {
    if (!isRunning()) {
        return setError(NotRunningError);
    }

    uint8_t x, y, s;
    int iter;
    int available;

    w.beginTransmission(a);
    int azer = w.endTransmission();
    if (azer == 0) {
        for(iter = 0; iter<10000; iter++) {
            delay(10);

            w.requestFrom(a, (uint8_t) 4);
            w.write(byte(0));
            delay(100);
            available = w.available();
            //Serial.println("available: " + (String)available);
            if (available == 4) {
                x = w.read();
                //Serial.print("x: ");
                //Serial.println(x, HEX);
                y = w.read();
                //Serial.print("y: ");
                //Serial.println(y, HEX);
                s = x >> 6;
                //Serial.print("s: ");
                //Serial.println(s, HEX);

                switch (s) {
                    default:
                        if(s == 1){
                            Serial.println("This Data may be stale");
                        }
                        h = (((uint16_t)(x & 0x3f)) << 8) | y;
                        x = w.read();
                        y = w.read();
                        t = ((((uint16_t) x) << 8) | y) >> 2;
                        w.endTransmission();
                        //Serial.println(h, HEX);
                        //Serial.println(t, HEX);
                        return setError(0);

                    case 2:
                        w.endTransmission();
                        return setError(CommandModeError);
                }
            } else {
                return setError(CommunicationError);
            }
        }
        //else
        return setError(LoopTimeout);
    } else {
        Serial.print("...");
        Serial.println(azer);
        return setError(ConnectionError);
    }
}

uint8_t HIH61XX::commandRequest(Stream & stream) {
    if (stream.available()) {
        return commandProcess(stream, stream.read());
    }
    return commandReply(stream, 255);
}

uint8_t HIH61XX::commandProcess(Stream & stream, uint8_t command) {
    switch (command) {
        //  get humidity
        case 'h':
            return commandReply(stream, 0, humidity());

            //  get temperature
        case 't':
            return commandReply(stream, 0, temperature());

            //  get i2c address
        case 'a':
            return commandReply(stream, 0, address());

            //  get power pin
        case 'p':
            return commandReply(stream, 0, powerPin());

            //  update
        case 'u':
            return commandReply(stream, update());

            //  start
        case '1':
            return commandReply(stream, start());

            //  stop
        case '0':
            return commandReply(stream, stop());
    }

    return commandReply(stream, 254);
}
