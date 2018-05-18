
SLIPSerial {
        var portName;
        var action;
        var port, inputThread;

        *new { arg portName, baudRate=230400;
                Platform.case(
                        \linux, {
                                SerialPort.devicePattern = "/dev/tty[A,U]*"
                        },
                        \osx, {
                                SerialPort.devicePattern = "/dev/tty.usb*"
                        },
                        \windows, {
                                SerialPort.devicePattern = nil
                        },
                );
                // If the portname is not specified, select the first matching port.
                // This is platform dependant, but not thoroughly tested.
                portName = portName ? SerialPort.devices[0];

                ^super.newCopyArgs(portName).init(baudRate);
        }

        init { arg br;
                port = SerialPort(
                        port:portName,
                        baudrate:br,
                        databits:8,
                        stopbit:true,
                        parity:nil,
                        crtscts:false,
                        xonxoff:false,
                        exclusive:false
                );

                inputThread = fork {
                        // The SLIP packets are encoded using special characters:
                        // end = 8r300 (2r11000000 or 0xc0 or 192)
                        // esc = 8r333 (2r11011011 or 0xdb or 219)
                        // esc_end = 8r334 (2r011011100 or 0xdc or 220)
                        // esc_esc = 8r335 (2r011011101 or 0xdd or 221)

                        var serialByte, buffer;
                        var maxPacketSize = 1024;
                        var slipEND = 8r300;
                        var slipESC = 8r333;
                        var slipESC_END = 8r334;
                        var slipESC_ESC = 8r335;
                        buffer = Int8Array(maxSize:maxPacketSize);
                        {
                                serialByte = port.read;
                                serialByte.switch(
                                        slipEND, {
                                                action.value(buffer);
                                                buffer = Int8Array(maxSize:maxPacketSize);
                                        },
                                        slipESC, {
                                                serialByte = port.read;
                                                serialByte.switch(
                                                        slipESC_END, { buffer.add(slipEND) },
                                                        slipESC_ESC, { buffer.add(slipESC) },
                                                        {"SLIP encoding error.".warn; buffer.postln; }
                                                )
                                        },
                                        { buffer.add(serialByte); }
                                );
                        }.loop
                };

                CmdPeriod.doOnce(this);
                ShutDown.add({this.close});

        }

        close {
                inputThread.stop;
                port.close;
        }

        cmdPeriod {
                this.close;
        }
} 