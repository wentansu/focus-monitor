package com.presagetech.smartspectra

import android.content.Context
import android.content.res.XmlResourceParser
import androidx.startup.Initializer
import timber.log.Timber

// Initializes SmartSpectraSdk at app startup
@Suppress("unused")
internal class SmartSpectraInitializer : Initializer<SmartSpectraSdk> {
    /**
     * Initializes [SmartSpectraSdk] and its authentication flow when the host
     * application starts.
     */
    override fun create(context: Context): SmartSpectraSdk {
        // Initialize Timber for logging
        if (Timber.forest().isEmpty()) {
            Timber.plant(Timber.DebugTree())
        }
        SmartSpectraSdk.initialize(context)

        val parsedConfig = parsePresageServicesXml(context)
        //Kick off authentication
        AuthHandler.initialize(context, parsedConfig)
        AuthHandler.getInstance().startAuthWorkflow()
        return SmartSpectraSdk.getInstance()
    }
    /** No library dependencies are required. */
    override fun dependencies(): List<Class<out Initializer<*>>> {
        // No dependencies on other libraries.
        return emptyList()
    }

    /**
     * Parses the optional `presage_services.xml` configuration file bundled in
     * the host application.
     *
     * @return a map of configuration keys used to initialize authentication.
     */
    private fun parsePresageServicesXml(context: Context): Map<String, Any?> {
        val resources = context.applicationContext.resources
        val resourceId = resources.getIdentifier("presage_services", "xml", context.packageName)
        if (resourceId == 0) {
            Timber.e("Resource not found")
            return emptyMap()
        }

        val xmlResourceParser = resources.getXml(resourceId)
        val result = mutableMapOf<String, Any?>()

        while (xmlResourceParser.eventType != XmlResourceParser.END_DOCUMENT) {
            if (xmlResourceParser.eventType == XmlResourceParser.START_TAG && xmlResourceParser.name == "string") {
                val name = xmlResourceParser.getAttributeValue(null, "name")
                xmlResourceParser.next() // Move to the text content
                when (name) {
                    "client_id" -> result["client_id"] = xmlResourceParser.text
                    "sub" -> result["sub"] = xmlResourceParser.text
                    "oauth_enabled" -> result["oauth_enabled"] = xmlResourceParser.text.toBoolean()
                    "config_version" -> result["config_version"] = xmlResourceParser.text
                }
            }
            xmlResourceParser.next()
        }

        return result
    }
}
