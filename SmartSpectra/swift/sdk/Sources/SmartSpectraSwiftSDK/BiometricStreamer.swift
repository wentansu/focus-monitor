import Foundation
import Combine
import Network

/// UDP client for streaming biometric data to SAM (Solace Agent Mesh) gateway
@available(macOS 10.15, iOS 13.0, watchOS 6.0, tvOS 13.0, *)
public class BiometricStreamer: NSObject, ObservableObject {
    
    // MARK: - Properties
    
    private var connection: NWConnection?
    private let queue = DispatchQueue(label: "com.smartspectra.biometricstreamer")
    
    // Default config matching the updated requirement
    private let defaultHost = "10.194.124.2"
    private let defaultPort: UInt16 = 5005
    
    @Published public var isConnected: Bool = false
    @Published public var lastMessage: String = ""
    @Published public var connectionError: String?
    
    /// Session ID for tracking the current streaming session
    public var sessionId: String = UUID().uuidString
    
    /// Counter for tracking sent messages
    @Published public var messagesSentCount: Int = 0
    
    /// Counter for tracking received messages
    @Published public var messagesReceivedCount: Int = 0
    
    /// Last sent data summary (for UI display)
    @Published public var lastSentSummary: String = ""
    
    // MARK: - Initialization
    
    public override init() {
        super.init()
    }
    
    // MARK: - Connection Management
    
    /// Connect to the UDP server using the default configuration
    public func connect() {
        connect(host: defaultHost, port: defaultPort)
    }
    
    /// Connect to a UDP server with a custom URL string
    /// - Parameter urlString: The URL string (e.g., "udp://10.194.124.2:5005")
    public func connect(urlString: String) {
        guard let url = URL(string: urlString),
              let host = url.host,
              let port = url.port else {
            print("[BiometricStreamer] Invalid URL: \(urlString)")
            DispatchQueue.main.async {
                self.connectionError = "Invalid URL: \(urlString)"
            }
            return
        }
        
        connect(host: host, port: UInt16(port))
    }
    
    /// Connect to a UDP server
    /// - Parameters:
    ///   - host: The hostname or IP address
    ///   - port: The port number
    public func connect(host: String, port: UInt16) {
        // Clean up any existing connection
        disconnect()
        
        print("[BiometricStreamer] 🔌 Attempting to connect to UDP: \(host):\(port)")
        
        let nwHost = NWEndpoint.Host(host)
        let nwPort = NWEndpoint.Port(rawValue: port)!
        
        // Create the connection with UDP parameters
        connection = NWConnection(host: nwHost, port: nwPort, using: .udp)
        
        // Handle state updates
        connection?.stateUpdateHandler = { [weak self] state in
            self?.handleStateUpdate(state)
        }
        
        // Start the connection
        connection?.start(queue: queue)
        
        // Start receiving messages (if any)
        receiveMessage()
    }
    
    private func handleStateUpdate(_ state: NWConnection.State) {
        switch state {
        case .ready:
            print("[BiometricStreamer] UDP connection ready")
            DispatchQueue.main.async {
                self.isConnected = true
                self.connectionError = nil
            }
        case .failed(let error):
            print("[BiometricStreamer] Connection failed: \(error.localizedDescription)")
            DispatchQueue.main.async {
                self.isConnected = false
                self.connectionError = error.localizedDescription
            }
        case .cancelled:
            print("[BiometricStreamer] Connection cancelled")
            DispatchQueue.main.async {
                self.isConnected = false
            }
        case .waiting(let error):
            print("[BiometricStreamer] Connection waiting: \(error.localizedDescription)")
        case .preparing:
            print("[BiometricStreamer] Connection preparing...")
        case .setup:
            print("[BiometricStreamer] Connection setup...")
        @unknown default:
            break
        }
    }
    
    /// Disconnect from the server
    public func disconnect() {
        connection?.cancel()
        connection = nil
        
        DispatchQueue.main.async {
            self.isConnected = false
        }
        
        print("[BiometricStreamer] Disconnected")
    }
    
    // MARK: - Sending Data
    
    /// Send raw string message
    /// - Parameter message: The string message to send
    public func send(message: String) {
        guard let data = message.data(using: .utf8) else { return }
        send(data: data)
    }
    
    /// Send raw data
    /// - Parameter data: The data to send
    private func send(data: Data) {
        guard let connection = connection else { return }
        
        connection.send(content: data, completion: .contentProcessed { [weak self] error in
            if let error = error {
                print("[BiometricStreamer] ❌ Send error: \(error.localizedDescription)")
            } else {
                DispatchQueue.main.async {
                    self?.messagesSentCount += 1
                }
            }
        })
    }
    
    /// Send a dictionary as JSON
    /// - Parameter data: Dictionary to serialize and send
    public func send(data: [String: Any]) {
        guard let connection = connection else { return }
        
        guard let jsonData = try? JSONSerialization.data(withJSONObject: data),
              let _ = String(data: jsonData, encoding: .utf8) else {
            print("[BiometricStreamer] ❌ Failed to serialize data to JSON")
            return
        }
        
        // Send as data
        connection.send(content: jsonData, completion: .contentProcessed { [weak self] error in
            guard let self = self else { return }
            if let error = error {
                print("[BiometricStreamer] ❌ Send error: \(error.localizedDescription)")
            } else {
                DispatchQueue.main.async {
                    self.messagesSentCount += 1
                    
                    // Create summary of what was sent
                    if let dataType = (data["data"] as? [String: Any])?["dataType"] as? String {
                        self.lastSentSummary = "\(dataType) #\(self.messagesSentCount)"
                    } else if let type = (data["metadata"] as? [String: Any])?["type"] as? String {
                        self.lastSentSummary = "\(type) #\(self.messagesSentCount)"
                    }
                }
            }
        })
    }
    
    /// Send biometric metrics data with metadata for SAM orchestrator
    /// - Parameters:
    ///   - content: The content/message to send (e.g., metrics JSON)
    ///   - type: The type of biometric data (e.g., "heartrate", "breathing", "face")
    ///   - additionalMetadata: Any additional metadata to include
    public func sendBiometricData(content: String, type: String, additionalMetadata: [String: Any] = [:]) {
        var metadata: [String: Any] = [
            "source": "iOS",
            "type": type,
            "timestamp": ISO8601DateFormatter().string(from: Date())
        ]
        // Merge additional metadata
        for (key, value) in additionalMetadata {
            metadata[key] = value
        }
        
        let payload: [String: Any] = [
            "id": UUID().uuidString,
            "content": content,
            "session_id": sessionId,
            "metadata": metadata
        ]
        
        send(data: payload)
    }
    
    /// Get statistics about the streaming session
    public func getStats() -> [String: Any] {
        return [
            "sessionId": sessionId,
            "isConnected": isConnected,
            "messagesSent": messagesSentCount,
            "messagesReceived": messagesReceivedCount,
            "lastSent": lastSentSummary,
            "lastReceived": lastMessage,
            "error": connectionError ?? "none"
        ]
    }
    
    /// Reset counters (call when starting a new session)
    public func resetStats() {
        DispatchQueue.main.async {
            self.messagesSentCount = 0
            self.messagesReceivedCount = 0
            self.lastSentSummary = ""
            self.lastMessage = ""
            self.connectionError = nil
        }
    }
    
    // MARK: - Receiving Messages
    
    private func receiveMessage() {
        // Prepare to receive a message
        connection?.receiveMessage { [weak self] (data, context, isComplete, error) in
            guard let self = self else { return }
            
            if let error = error {
                print("[BiometricStreamer] ❌ Receive error: \(error.localizedDescription)")
                DispatchQueue.main.async {
                    self.connectionError = error.localizedDescription
                }
                return
            }
            
            if let data = data, !data.isEmpty {
                DispatchQueue.main.async {
                    self.messagesReceivedCount += 1
                }
                
                if let text = String(data: data, encoding: .utf8) {
                    DispatchQueue.main.async {
                        self.lastMessage = text
                    }
                    self.handleReceivedMessage(text)
                }
            }
            
            // Continue receiving if connection is still alive
            if self.connection != nil && self.connection?.state != .cancelled {
                self.receiveMessage()
            }
        }
    }
    
    /// Handle received messages from the server
    /// - Parameter message: The received message string
    private func handleReceivedMessage(_ message: String) {
        // Parse JSON response if needed
        if let data = message.data(using: .utf8),
           let json = try? JSONSerialization.jsonObject(with: data) as? [String: Any] {
            // Handle structured response
            if let response = json["response"] as? String {
                print("[BiometricStreamer] Server response: \(response)")
            }
        }
    }
}
