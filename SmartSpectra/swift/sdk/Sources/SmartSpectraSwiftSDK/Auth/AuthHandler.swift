//
//  AppAttestHandler.swift
//  SmartSpectraSwiftSDK
//
//  Created by Ashraful Islam on 12/30/24.
//

import Foundation
import DeviceCheck
import CryptoKit
import PresagePreprocessing

/// Errors that may occur during authentication.
internal enum AuthError: Error {
    case configurationFailed
    case notSupported
    case keyGenerationFailed
    case challengeFetchFailed
    case attestationFailed
    case bundleIdFetchFailed
    case tokenFetchFailed
}

/// Result of an authentication attempt.
internal enum AuthResult {
    case success(String)
    case failure(Error)
}

internal class AuthHandler {
    static let shared = AuthHandler()
    private let service = DCAppAttestService.shared
    private let keychainHelper = KeychainHelper.shared
    private var plistData: [String: Any]?
    /// Indicates whether OAuth authentication is configured and enabled.
    public var isOauthEnabled: Bool {
        guard let plistData = plistData else { return false }
        return plistData["IS_OAUTH_ENABLED"] as? Bool ?? false
    }
    private let authRunner = SerialTaskRunner()

    private init() {
        plistData = readPlist()
    }

    /// Begins the authentication workflow if an access token is expired.
    ///
    /// This method manages retries with exponential backoff and invokes the
    /// provided completion handler upon success or failure.
    /// - Parameter completion: Optional closure called with an error on failure
    ///   or `nil` on success.
    internal func startAuthWorkflow(completion: ((Error?) -> Void)? = nil) {
        Task {
            await authRunner.enqueue { [weak self] in
                guard let self = self else { completion?(nil); return }

                guard self.isAuthTokenExpired(), self.isOauthEnabled else {
                    completion?(nil); return
                }
                var attempt = 0
                let maxAttempts = 5
                var delay: UInt64 = 1_000_000_000 // 1 second in nanoseconds

                while attempt < maxAttempts {
                    let result = await self.AuthWorkflow()
                    switch result {
                        case .success(_):
                            print("Successfully obtained access token from server.")
                            completion?(nil)
                            return
                        case .failure(let error):
                            print("Failed to complete App Authentication workflow: \(error)")
                            attempt += 1
                            if attempt < maxAttempts {
                                try? await Task.sleep(nanoseconds: delay)
                                delay *= 2 // Exponential backoff
                            } else {
                                print("Max retry attempts reached. Aborting.")
                                completion?(error)
                                return
                            }
                    }
                }
            }
        }
    }

    /// Performs the App Attest authentication handshake with the server.
    private func AuthWorkflow() async -> AuthResult {

        // Check for pre-conditions, if plist data is not available, this method should not be called
        guard let plistData = plistData else { return .failure(AuthError.configurationFailed) }
        guard configureAuthClient(with: plistData) else { return .failure(AuthError.configurationFailed) }
        guard service.isSupported else { return .failure(AuthError.notSupported) }

        // Simultaneously fetch server challenge and generate/get key ID
        async let challenge = fetchAuthChallenge()
        async let keyId = getAppAttestKeyId()

        guard let fetchedChallenge = await challenge else { return .failure(AuthError.challengeFetchFailed) }
        guard let fetchedKeyId = await keyId else { return .failure(AuthError.keyGenerationFailed) }

        //TODO: use service.generateAssertion(keyId!, clientDataHash: challengeHash) once the server is ready and send assertion object instead after first time
        guard let attestationObject = await attestKey(keyId: fetchedKeyId, challenge: fetchedChallenge) else { return .failure(AuthError.attestationFailed) }

        let challengeResponse = fetchedKeyId + ":" + attestationObject.base64EncodedString()
        guard let bundleID = Bundle.main.bundleIdentifier else { return .failure(AuthError.bundleIdFetchFailed) }

        guard let authToken = respondToAuthChallenge(with: challengeResponse, for: bundleID) else { return .failure(AuthError.tokenFetchFailed) }

        return .success(authToken)
    }

    /// Retrieves an existing App Attest key or generates a new one.
    private func getAppAttestKeyId() async -> String? {
        // Try to use existing key if available
        if let existingKeyId = try? keychainHelper.retrieveKeyId() {
            print("Found existing App Attest key, reusing the same key.")
            return existingKeyId
        }

        // Generate a new key if none exists
        do {
            let newKeyId = try await service.generateKey()
            self.storeKeyId(newKeyId)
            print("New App Attest key generated.")
            return newKeyId
        } catch {
            if (error as? DCError)?.code == .invalidKey {
                print("Invalid key detected, removing from keychain")
                try? keychainHelper.deleteKeyId()

                // Retry generating the key once after deletion
                do {
                    let retryKeyId = try await service.generateKey()
                    self.storeKeyId(retryKeyId)
                    print("Retry successful: New App Attest key generated.")
                    return retryKeyId
                } catch {
                    print("Retrying key generation failed: \(error.localizedDescription)")
                }
            } else {
                print("Error generating App Attest key: \(error.localizedDescription)")
            }
        }
        return nil
    }

    /// Generates an attestation object for the specified challenge.
    /// - Parameters:
    ///   - keyId: Identifier returned from App Attest.
    ///   - challenge: Server-provided challenge string.
    private func attestKey(keyId: String, challenge: String) async -> Data? {
        guard let challengeHash = Data(base64Encoded: challenge) else {
            print("Invalid challenge format")
            return nil
        }

        do {
            let attestation = try await service.attestKey(keyId, clientDataHash: challengeHash)
            return attestation
        } catch {
            if let nsError = error as NSError?, nsError.domain == "com.apple.devicecheck.error" {
                switch nsError.code {
                    case 2: // invalidInput
                        print("Invalid input detected during attestation. Possible malformed challenge or keyId.")
                        // Consider deleting the keyId if it's suspected to be invalid
                        try? keychainHelper.deleteKeyId()
                    case 3: // invalidKey
                        print("Invalid key detected during attestation, removing from keychain")
                        try? keychainHelper.deleteKeyId()
                    default:
                        print("Unhandled DeviceCheck error: \(nsError.code)")
                }
            } else {
                print("Error during key attestation: \(error.localizedDescription)")
            }
        }
        return nil
    }

    /// Persists the generated key identifier to the keychain.
    private func storeKeyId(_ keyId: String) {
        do {
            try keychainHelper.saveKeyId(keyId)
            print("Key ID stored successfully.")
        } catch {
            print("Error storing Key ID in Keychain: \(error)")
        }
    }

    /// Loads the OAuth configuration plist bundled with the host app.
    private func readPlist() -> [String: Any]? {
        guard let path = Bundle.main.path(forResource: "PresageService-Info", ofType: "plist") else {
            print("PresageService-Info.plist not found. OAuth authentication will be disabled. Using API key authentication instead.")
            return nil
        }
        guard let plistData = NSDictionary(contentsOfFile: path) as? [String: Any] else {
            print("Error: Failed to load PresageService-Info.plist. OAuth authentication will be disabled.")
            return nil
        }
        return plistData
    }

    /// Configures `PresagePreprocessing` with OAuth values from the plist.
    private func configureAuthClient(with plistData: [String: Any]) -> Bool {
        PresagePreprocessing.configureAuthClient(with: plistData)
        return true
    }

    /// Requests a challenge string from the authentication server.
    private func fetchAuthChallenge() async -> String? {
        guard let challenge = PresagePreprocessing.fetchAuthChallenge() else {
            print("Error fetching server challenge.")
            return nil
        }
        return challenge
    }

    /// Exchanges an attestation response for an authentication token.
    private func respondToAuthChallenge(with challengeResponse: String, for bundleID: String) -> String? {
        guard let token = PresagePreprocessing.respondToAuthChallenge(with: challengeResponse, for: bundleID) else {
            print("Error getting auth token")
            return nil
        }
        return token
    }

    /// Indicates whether the cached access token is expired.
    internal func isAuthTokenExpired() -> Bool {
        return PresagePreprocessing.isAuthTokenExpired()
    }
}