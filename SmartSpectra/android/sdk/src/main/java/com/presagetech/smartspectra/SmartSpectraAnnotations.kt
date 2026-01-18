package com.presagetech.smartspectra

@RequiresOptIn(
    level = RequiresOptIn.Level.WARNING,
    message = "This is an experimental API. Use with caution. Do not use in production server"
)
@Target(
    AnnotationTarget.CLASS,
    AnnotationTarget.FUNCTION,
    AnnotationTarget.PROPERTY
)
@Retention(value = AnnotationRetention.BINARY)
internal annotation class ExperimentalFeature
