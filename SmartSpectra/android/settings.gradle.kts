pluginManagement {
    repositories {
        google()
        mavenCentral()
        gradlePluginPortal()
    }
}
dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        mavenCentral()
        maven( url ="https://jitpack.io")
        maven( url = "https://central.sonatype.com/repository/maven-snapshots/")
        mavenLocal()
    }
}

rootProject.name = "SmartSpectra"
include(":sdk")
include(":samples")
include(":samples:demo-app")
include(":samples:smartspectra-trials")
