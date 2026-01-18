package com.presagetech.smartspectra

import android.content.Context
import android.util.Base64
import com.google.android.gms.tasks.Task
import com.google.android.gms.tasks.Tasks.await
import com.google.android.play.core.integrity.IntegrityManagerFactory
import com.google.android.play.core.integrity.IntegrityTokenRequest
import com.google.android.play.core.integrity.IntegrityTokenResponse
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import timber.log.Timber

internal enum class AuthError {
    CONFIGURATION_FAILED,
    CHALLENGE_FETCH_FAILED,
    PLAY_INTEGRITY_API_FAILED,
    TOKEN_FETCH_FAILED,
    BASE64_DECODE_FAILED
}

internal sealed class AuthResult {
    data class Success(val authToken: String) : AuthResult()
    data class Failure(val error: AuthError) : AuthResult()
}

internal class AuthHandler private constructor(private val context: Context, private val config: Map<String, Any?>) {
    // Create an instance of a manager.
    private val integrityManager = IntegrityManagerFactory.create(context.applicationContext)
    private val isOAuthEnabled: Boolean = config["oauth_enabled"] as? Boolean ?: false

    /**
     * Begins the App Attest flow used for OAuth authentication. Retries are
     * performed with exponential backoff when errors occur.
     */
    internal fun startAuthWorkflow() {
        // only proceed if oauth is true and auth token is expired already
        if (!(isOAuthEnabled && isAuthTokenExpired())) return

        CoroutineScope(Dispatchers.IO).launch {
            var attempt = 0
            val maxAttempts = 5
            var delay = 1000L // 1 second in milliseconds

            while (attempt < maxAttempts) {
                when (val result = AuthWorkflow()) {
                    is AuthResult.Success -> {
                        Timber.d("Successfully obtained auth token.")
                        SmartSpectraSdk.getInstance().setErrorMessage("")
                        return@launch
                    }
                    is AuthResult.Failure -> {
                        Timber.e("Failed to complete App Attest workflow: ${result.error}")
                        SmartSpectraSdk.getInstance().setErrorMessage("Authentication failed: ${result.error}")
                        attempt++
                        if (attempt < maxAttempts) {
                            Timber.d("Retrying in ${delay / 1000} seconds...")
                            kotlinx.coroutines.delay(delay)
                            delay *= 2 // Exponential backoff
                        } else {
                            Timber.e("Max retry attempts reached. Aborting.")
                        }
                    }
                }
            }
        }
    }

    /** Executes the network challenge/response portion of the auth flow. */
    private suspend fun AuthWorkflow(): AuthResult = withContext(Dispatchers.IO) {
        val clientId = config["client_id"] as? String ?: return@withContext AuthResult.Failure(AuthError.CONFIGURATION_FAILED)
        val sub = config["sub"] as? String ?: return@withContext AuthResult.Failure(AuthError.CONFIGURATION_FAILED)
        nativeConfigureAuthClient(clientId, sub, isOAuthEnabled)

        // Receive the nonce from the secure server.
        val challenge = fetchAuthChallenge() ?: return@withContext AuthResult.Failure(AuthError.CHALLENGE_FETCH_FAILED)
        val nonce: String
        try {
            val decodedBytes = Base64.decode(challenge, Base64.DEFAULT)
            nonce = Base64.encodeToString(decodedBytes, Base64.URL_SAFE or Base64.NO_WRAP)
        } catch (error: IllegalArgumentException) {
            Timber.e("Error decoding base64: ${error.localizedMessage}")
            return@withContext AuthResult.Failure(AuthError.BASE64_DECODE_FAILED)
        }

        Timber.d("Authentication Challenge Received from server")

        // Request the integrity token by providing a nonce.
        val integrityTokenResponse: Task<IntegrityTokenResponse> =
            integrityManager.requestIntegrityToken(
                IntegrityTokenRequest.builder()
                    .setCloudProjectNumber(914071078041)
                    .setNonce(nonce)
                    .build())

        // Await the result of the Task
        val integrityToken: String
        try {
            integrityToken = await(integrityTokenResponse).token()
            Timber.d("Received App attest integrity token response from play integrity api")
        } catch (e: Exception) {
            Timber.e("Error getting access token ${e.localizedMessage}")
            return@withContext AuthResult.Failure(AuthError.PLAY_INTEGRITY_API_FAILED)
        }

        // Process the token
        val accessToken = respondToAuthChallenge(integrityToken, context.packageName)
        if (accessToken == null) {
            Timber.e("Error getting access token ${integrityTokenResponse.exception?.localizedMessage}")
            return@withContext AuthResult.Failure(AuthError.TOKEN_FETCH_FAILED)
        }

        return@withContext AuthResult.Success(accessToken)
    }

    /**
     * Requests an authentication challenge from the Presage backend.
     */
    private fun fetchAuthChallenge(): String? {
        // Fetch the challenge from the server using native method
        var challenge: String? = null
        try {
            challenge = nativeFetchAuthChallenge()
        } catch (e: RuntimeException) {
            Timber.e("Error fetching auth challenge: ${e.localizedMessage}")
        }
        return challenge
    }

    /**
     * Sends the integrity token response back to the backend and returns an
     * access token if successful.
     */
    private fun respondToAuthChallenge(challengeResponse: String, packageName: String): String? {
        // Send the attestation object to the server and get the auth token using native method
        var accessToken: String? = null
        try {
            accessToken = nativeRespondToAuthChallenge(challengeResponse, packageName)
        } catch (e: RuntimeException) {
            Timber.e("Error responding to auth challenge: ${e.localizedMessage}")
        }
        return accessToken
    }

    /** Returns true if the stored auth token has expired. */
    internal fun isAuthTokenExpired(): Boolean {
        return nativeIsAuthTokenExpired()
    }

    /** Indicates whether OAuth authentication is active. */
    internal fun isOAuthEnabled(): Boolean {
        return isOAuthEnabled
    }

    private external fun nativeFetchAuthChallenge(): String
    private external fun nativeRespondToAuthChallenge(base64EncodedAnswer: String, packageName: String): String
    private external fun nativeConfigureAuthClient(clientId: String?, sub: String?, isOAuthEnabled: Boolean)
    private external fun nativeIsAuthTokenExpired(): Boolean

    internal companion object {
        @Volatile
        private var INSTANCE: AuthHandler? = null

        /**
         * Creates the singleton [AuthHandler] using the provided configuration.
         */
        fun initialize(context: Context, config: Map<String, Any?>) {
            synchronized(this) {
                if (INSTANCE == null) {
                    INSTANCE = AuthHandler(context.applicationContext, config)
                }
            }
        }

        /** Returns the previously [initialize]d instance. */
        fun getInstance(): AuthHandler =
            INSTANCE ?: throw IllegalStateException("AuthHandler is not initialized. Call initialize() first.")
    }
}
