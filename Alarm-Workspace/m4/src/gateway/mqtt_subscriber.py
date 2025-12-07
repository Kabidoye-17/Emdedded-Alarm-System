from mqtt_client import MQTTClient

class MQTTSubscriber(MQTTClient):
    """MQTT Subscriber for receiving messages from topics"""

    def __init__(self):
        super().__init__()
        self.subscriptions = {}  # topic -> callback mapping
        self.client.on_message = self._on_message

    def _on_message(self, _client, _userdata, msg):
        """Handle incoming messages"""
        topic = msg.topic
        payload = msg.payload.decode('utf-8', errors='replace')

        print(f"Received message on topic '{topic}': {payload}")

        # Call registered callback for this topic
        if topic in self.subscriptions:
            try:
                self.subscriptions[topic](payload)
            except Exception as e:
                print(f"Error in callback for topic '{topic}': {e}")

    def _subscribe_topic(self, topic):
        """Helper to subscribe to a single topic"""
        self.client.subscribe(topic)
        print(f"Subscribed to topic: {topic}")

    def subscribe(self, topic, callback):
        """
        Subscribe to a topic with a callback

        Args:
            topic: MQTT topic to subscribe to
            callback: Function to call when message received (takes payload string)
        """
        self.subscriptions[topic] = callback
        if self.is_connected:
            self._subscribe_topic(topic)

    def on_connect_success(self):
        """Re-subscribe to all topics on connection"""
        for topic in self.subscriptions:
            self._subscribe_topic(topic)
