import paho.mqtt.client as mqtt
from config.config import mqtt as mqtt_config

class MQTTClient:
    """Base MQTT client for connection management"""

    def __init__(self):
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)
        self.broker = mqtt_config.broker
        self.port = mqtt_config.port
        self.keep_alive = mqtt_config.keep_alive
        self.username = mqtt_config.username
        self.password = mqtt_config.password
        self._is_connected = False

        # Set default callbacks
        self.client.on_connect = self._on_connect
        self.client.on_disconnect = self._on_disconnect

        if self.username:
            # Configure broker authentication when credentials are provided
            self.client.username_pw_set(self.username, self.password)

    def _on_connect(self, _client, _userdata, _flags, rc):
        """Default connection callback"""
        if rc == 0:
            self._is_connected = True
            print(f"Successfully connected to MQTT broker at {self.broker}:{self.port}")
            self.on_connect_success()
        else:
            self._is_connected = False
            print(f"Failed to connect to MQTT broker. Return code: {rc}")

    def _on_disconnect(self, _client, _userdata, rc):
        """Default disconnection callback"""
        self._is_connected = False
        if rc != 0:
            print(f"Unexpected disconnect from MQTT broker. Return code: {rc}")

    def on_connect_success(self):
        """Override this in subclasses for custom connection logic"""
        pass

    def connect(self):
        """Connect to MQTT broker"""
        print(f"Connecting to MQTT broker at {self.broker}:{self.port}...")
        self.client.connect(self.broker, self.port, keepalive=self.keep_alive)

    def disconnect(self):
        """Disconnect from MQTT broker"""
        self.client.disconnect()
        print("Disconnected from MQTT broker")

    def start(self):
        """Start the MQTT client loop"""
        self.connect()
        try:
            print("MQTT client running (Ctrl+C to stop)...")
            self.client.loop_forever()
        except KeyboardInterrupt:
            print("\nStopping MQTT client...")
        finally:
            self.disconnect()

    @property
    def is_connected(self):
        """Check if client is connected"""
        return self._is_connected
