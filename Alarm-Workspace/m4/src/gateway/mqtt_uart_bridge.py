from mqtt_subscriber import MQTTSubscriber
from uart_bridge import UARTBridge
from config.config import topics

class MQTTToUARTBridge:
    """Bridge that forwards MQTT messages to UART"""

    def __init__(self):
        self.uart = UARTBridge()
        self.subscriber = MQTTSubscriber()


    def on_command_received(self, payload):
        """Handle incoming MQTT messages and forward to UART"""
        print(f"Command received: {payload}")
        self.uart.send(payload)

    def start(self):
        """Start the MQTT-UART bridge"""
        try:
            # Subscribe to command topic
            self.subscriber.subscribe(topics.command, self.on_command_received)

            # Start the MQTT client (blocking)
            self.subscriber.start()
        finally:
            # Cleanup
            self.uart.close()

def main():
    bridge = MQTTToUARTBridge()
    bridge.start()

if __name__ == "__main__":
    main()