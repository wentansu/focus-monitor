plugins {
    id("com.android.application") version "8.5.2" apply false
    id("com.android.library") version "8.5.2" apply false
    id("org.jetbrains.kotlin.android") version "1.9.0" apply false
    id("org.jetbrains.dokka") version "1.9.20" apply false
    id("io.github.gradle-nexus.publish-plugin") version "1.3.0"
}

group = "com.presagetech"

nexusPublishing {
    // We are using "Publishing By Using the Portal OSSRH Staging API"
    // more details: https://central.sonatype.org/publish/publish-portal-ossrh-staging-api/#configuration
    repositories {
        sonatype {
            nexusUrl.set(uri("https://ossrh-staging-api.central.sonatype.com/service/local/"))
            snapshotRepositoryUrl.set(uri("https://central.sonatype.com/repository/maven-snapshots/"))
            // Credentials should be set via environment variables or gradle.properties
            // ORG_GRADLE_PROJECT_sonatypeUsername and ORG_GRADLE_PROJECT_sonatypePassword
        }
    }
}
