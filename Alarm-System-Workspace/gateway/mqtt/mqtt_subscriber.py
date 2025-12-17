from mqtt.mqtt_client import MQTTClient

class MQTTSubscriber(MQTTClient):
    """MQTT Subscriber for receiving messages from a single topic"""

    def __init__(self):
        super().__init__()
        self.topic = None
        self.callback = None
        self.client.on_message = self._on_message

    def _on_message(self, _client, _userdata, msg):
        """Handle incoming messages"""
        payload = msg.payload.decode('utf-8', errors='replace')

        print(f"Received message on topic '{msg.topic}': {payload}")

        # Call registered callback
        if self.callback:
            try:
                self.callback(payload)
            except Exception as e:
                print(f"Error in callback for topic '{msg.topic}': {e}")

    def subscribe(self, topic, callback):
        """
        Subscribe to a topic with a callback

        Args:
            topic: MQTT topic to subscribe to
            callback: Function to call when message received (takes payload string)
        """
        self.topic = topic
        self.callback = callback
        if self.is_connected:
            self.client.subscribe(topic)
            print(f"Subscribed to topic: {topic}")

    def on_connect_success(self):
        """Re-subscribe to topic on connection"""
        if self.topic:
            self.client.subscribe(self.topic)
            print(f"Subscribed to topic: {self.topic}")
