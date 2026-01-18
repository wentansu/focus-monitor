plugins {
    id("com.android.library")
    id("org.jetbrains.kotlin.android")
    id("org.jetbrains.dokka")
    id("maven-publish")
    id("signing")
    id("kotlin-parcelize")
    id("org.cyclonedx.bom") version "1.8.2"
}

android {
    namespace = "com.presagetech.smartspectra"
    compileSdk = 34

    defaultConfig {
        minSdk = 26

        ndk {
            abiFilters.add("armeabi-v7a")
            abiFilters.add("arm64-v8a")
        }

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        testInstrumentationRunnerArguments["physiologyApiKey"] = System.getenv("PHYSIOLOGY_API_KEY") ?: ""
        consumerProguardFiles("consumer-rules.pro")
    }
    sourceSets.getByName("main") {
        jniLibs {
            srcDirs("jni/")
        }
    }
    buildFeatures {
        buildConfig = false
    }

    buildTypes {
        debug {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android.txt"),
                "proguard-rules-debug.pro"
            )
            isJniDebuggable = true
        }
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }
    kotlinOptions {
        jvmTarget = "1.8"
    }
    buildToolsVersion = "34.0.0"
    ndkVersion = "26.1.10909125"
    publishing {
        singleVariant("release") {
            withJavadocJar()
        }
    }

}

dependencies {
    implementation("androidx.core:core-ktx:1.13.1")
    implementation("androidx.appcompat:appcompat:1.7.0")
    implementation("com.google.android.material:material:1.12.0")
    implementation("androidx.constraintlayout:constraintlayout:2.2.0")
    implementation("androidx.lifecycle:lifecycle-livedata-ktx:2.8.7")
    implementation("androidx.lifecycle:lifecycle-viewmodel-ktx:2.8.7")

    implementation("com.jakewharton.timber:timber:5.0.1")
    implementation("androidx.test:core-ktx:1.6.1")
    implementation("androidx.startup:startup-runtime:1.2.0")

    val camerax_version = "1.4.0"
    implementation("androidx.camera:camera-core:${camerax_version}")
    implementation("androidx.camera:camera-camera2:${camerax_version}")
    implementation("androidx.camera:camera-lifecycle:${camerax_version}")
    implementation("androidx.camera:camera-view:${camerax_version}")

    // mediapipe dependencies
    implementation("com.google.flogger:flogger:0.6")
    implementation("com.google.flogger:flogger-system-backend:0.6")
    implementation("com.google.code.findbugs:jsr305:3.0.2")
    implementation("com.google.guava:guava:27.0.1-android")
    implementation("org.opencv:opencv:4.10.0")

    //protobuf dependencies
    api("com.google.protobuf:protobuf-javalite:4.26.1")

    //integrity api dependencies
    implementation("com.google.android.play:integrity:1.4.0")

    // mediapipe java library - contains JNI bindings for native libraries
    implementation(files("libs/classes.jar"))

    // charting
    implementation ("com.github.PhilJay:MPAndroidChart:v3.1.0")

    // test dependencies
    testImplementation("junit:junit:4.13.2")
    testImplementation("org.mockito:mockito-core:5.4.0")
    androidTestImplementation("org.mockito:mockito-android:4.0.0")
    testImplementation ("androidx.arch.core:core-testing:2.2.0")
    androidTestImplementation("androidx.test.ext:junit:1.2.1")
    androidTestImplementation("androidx.test:runner:1.6.2")
    androidTestImplementation("androidx.test.espresso:espresso-core:3.6.1")
    androidTestImplementation("org.mockito:mockito-core:5.4.0")
}

tasks.withType<org.jetbrains.dokka.gradle.DokkaTask>().configureEach {
    dokkaSourceSets {
        named("main") {
            displayName.set("SmartSpectra Android SDK")
            moduleName.set("SmartSpectra Android SDK")

            // Include outer README overview
            includes.from("../README.md")

            // Include main SDK packages
            perPackageOption {
                matchingRegex.set("com\\.presagetech\\.smartspectra")
                includeNonPublic.set(false)
            }

            // Include protobuf package for documentation
            perPackageOption {
                matchingRegex.set("com\\.presage\\.physiology\\.proto")
                includeNonPublic.set(false)
            }

            suppressInheritedMembers.set(true)
            suppressObviousFunctions.set(true)
            suppressGeneratedFiles.set(true)
        }
    }

    // Copy images to the documentation output directory
    doLast {
        val dokkaOutputDir = file("build/dokka/html")
        val mediaOutputDir = file("$dokkaOutputDir/media")

        // Copy media images
        val androidMediaDir = file("../media")
        if (androidMediaDir.exists()) {
            copy {
                from(androidMediaDir)
                into(mediaOutputDir)
            }
        }
    }
}

task<Javadoc>("androidJavadoc") {
    exclude("**/R.html", "**/R.*.html", "**/index.html")
    options.encoding = "UTF-8"
}

signing {
    useInMemoryPgpKeys(System.getenv("SIGNING_KEY_ID"), System.getenv("SIGNING_KEY"), System.getenv("SIGNING_PASSWORD"))
    sign(publishing.publications)
}


publishing {
    publications {
        create<MavenPublication>("release") {
            afterEvaluate {
                from(components["release"])
            }
            groupId = "com.presagetech"
            artifactId = "smartspectra"
            version = project.version.toString()
            println("Publishing com.presagetech:smartspectra:${project.version}")
            pom {
                name.set("Presage Physiology SDK")
                description.set("Heart and respiration rate measurement by Presage Technologies")
                url.set("https://physiology.presagetech.com/")
                licenses {
                    license {
                        name.set("GNU LESSER GENERAL PUBLIC LICENSE v3.0")
                        url.set("https://www.gnu.org/licenses/lgpl-3.0.html")
                    }
                }
                developers {
                    developer {
                        id.set("presagetech")
                        name.set("Presage Technologies")
                        email.set("support@presagetech.com")
                    }
                }
                scm {
                    connection.set("scm:git:git://github.com/Presage-Security/SmartSpectra.git")
                    developerConnection.set("scm:git:ssh://github.com:Presage-Security/SmartSpectra.git")
                    url.set("https://github.com/Presage-Security/SmartSpectra")
                }
            }
        }
    }
}


// CycloneDX SBOM Configuration
tasks.named<org.cyclonedx.gradle.CycloneDxTask>("cyclonedxBom") {
    setIncludeConfigs(listOf("releaseRuntimeClasspath", "releaseCompileClasspath"))
    setSkipConfigs(listOf(".*Test.*", ".*AndroidTest.*", ".*test.*", ".*debug.*"))
    setSchemaVersion("1.4")
    setIncludeLicenseText(false)
    setOutputFormat("json")
    setOutputName("smartspectra-android-sdk-bom")
    setProjectType("library")
    setIncludeBomSerialNumber(true)
    setDestination(file("${layout.buildDirectory.get()}/reports/bom"))
}

// Task to generate XML format SBOM for SDK
tasks.register<org.cyclonedx.gradle.CycloneDxTask>("cyclonedxBomXml") {
    setIncludeConfigs(listOf("releaseRuntimeClasspath", "releaseCompileClasspath"))
    setSkipConfigs(listOf(".*Test.*", ".*AndroidTest.*", ".*test.*", ".*debug.*"))
    setSchemaVersion("1.4")
    setIncludeLicenseText(false)
    setOutputFormat("xml")
    setOutputName("smartspectra-android-sdk-bom")
    setProjectType("library")
    setIncludeBomSerialNumber(true)
    setDestination(file("${layout.buildDirectory.get()}/reports/bom"))
}

// Task to generate both JSON and XML formats
tasks.register("generateAllSBOMFormats") {
    group = "verification"
    description = "Generate SBOM in both JSON and XML formats"
    dependsOn("cyclonedxBom", "cyclonedxBomXml")
}
