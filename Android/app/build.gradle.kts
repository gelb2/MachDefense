plugins { id("com.android.application"); id("org.jetbrains.kotlin.android") }
val prepareAssets by tasks.registering(Exec::class) {
    workingDir(rootProject.projectDir)
    commandLine("python3", "tools/prepare_assets.py", layout.buildDirectory.dir("generated/gameAssets").get().asFile.absolutePath)
    inputs.files(listOf("Images", "Audio", "RawData", "Tables", "Generated").map { fileTree("../../Resources/$it") })
    outputs.dir(layout.buildDirectory.dir("generated/gameAssets"))
}
android {
    namespace = "com.gelb2.machdefense"
    compileSdk = 36
    ndkVersion = "26.3.11579264"
    defaultConfig {
        applicationId = "com.gelb2.machdefense.restored"
        minSdk = 26; targetSdk = 36; versionCode = 6; versionName = "0.6"
        ndk { abiFilters += listOf("arm64-v8a") }
        externalNativeBuild { cmake {
            arguments += listOf("-DANDROID_STL=c++_static")
            targets += "machdefense"
        } }
    }
    flavorDimensions += "edition"
    productFlavors {
        create("normal") {
            dimension = "edition"; buildConfigField("boolean", "CHEAT", "false")
            resValue("string", "app_name", "MachDefense")
            externalNativeBuild { cmake { arguments += "-DMD_VALIDATION=OFF" } }
        }
        create("cheat") {
            dimension = "edition"; applicationId = "com.gelb2.machdefense.validation"
            buildConfigField("boolean", "CHEAT", "true"); resValue("string", "app_name", "MD Validation")
            externalNativeBuild { cmake { arguments += "-DMD_VALIDATION=ON" } }
        }
    }
    buildTypes { getByName("release") { isMinifyEnabled = false } }
    buildFeatures { buildConfig = true }
    externalNativeBuild { cmake { path = file("../cmake/CMakeLists.txt"); version = "3.22.1" } }
    sourceSets.getByName("main").assets.srcDir(layout.buildDirectory.dir("generated/gameAssets"))
    compileOptions { sourceCompatibility = JavaVersion.VERSION_17; targetCompatibility = JavaVersion.VERSION_17 }
    packaging { jniLibs { useLegacyPackaging = false } }
    androidResources { noCompress += listOf("mp3", "wav") }
}
tasks.named("preBuild").configure { dependsOn(prepareAssets) }

kotlin { compilerOptions { jvmTarget.set(org.jetbrains.kotlin.gradle.dsl.JvmTarget.JVM_17) } }
