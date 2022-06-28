/*
 * Copyright (c) 2021, The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cstdlib>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include <vector>

#include <android-base/properties.h>
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#include <sys/sysinfo.h>

#include "property_service.h"
#include "vendor_init.h"

void property_override(char const prop[], char const value[], bool add = true) {
    prop_info *pi;

    pi = (prop_info *)__system_property_find(prop);
    if (pi)
        __system_property_update(pi, value, strlen(value));
    else if (add)
        __system_property_add(prop, strlen(prop), value, strlen(value));
}

void load_dalvik_properties() {
    struct sysinfo sys;

    sysinfo(&sys);
    if (sys.totalram > 7168ull * 1024 * 1024) {
        // 8GB & 12GB RAM
        property_override("dalvik.vm.heapstartsize", "32m");
        property_override("dalvik.vm.heapgrowthlimit", "512m");
        property_override("dalvik.vm.heapsize", "768m");
        property_override("dalvik.vm.heapmaxfree", "64m");
    } else {
        // from - phone-xhdpi-6144-dalvik-heap.mk
        property_override("dalvik.vm.heapstartsize", "16m");
        property_override("dalvik.vm.heapgrowthlimit", "256m");
        property_override("dalvik.vm.heapsize", "512m");
        property_override("dalvik.vm.heapmaxfree", "32m");
    }

    property_override("dalvik.vm.heaptargetutilization", "0.5");
    property_override("dalvik.vm.heapminfree", "8m");
}

std::vector<std::string> ro_props_default_source_order = {
        "", "bootimage.", "odm.", "product.", "system.", "system_ext.", "vendor.",
};

void set_ro_build_prop(const std::string& prop, const std::string& value) {
    for (const auto& source : ro_props_default_source_order) {
        auto prop_name = "ro." + source + "build." + prop;
        if (source == "")
            property_override(prop_name.c_str(), value.c_str());
        else
            property_override(prop_name.c_str(), value.c_str(), false);
    }
};

void set_ro_product_prop(const std::string& prop, const std::string& value) {
    for (const auto& source : ro_props_default_source_order) {
        auto prop_name = "ro.product." + source + prop;
        property_override(prop_name.c_str(), value.c_str(), false);
    }
};

void vendor_load_properties() {
    std::string region = android::base::GetProperty("ro.boot.hwc", "");
    std::string product = android::base::GetProperty("ro.boot.product.hardware.sku", "");

    std::string model;
    std::string device;
    std::string fingerprint;
    std::string description;
    std::string mod_device;

    if (region == "CN") {
        if (product == "pro") {
        model = "Redmi K30 Pro Zoom Edition";
        device = "lmipro";
        description = "qssi-user 12 RKQ1.211001.001 V13.0.4.0.SJKCNXM release-keys";
        mod_device = "lmipro";
        } else {
        model = "Redmi K30 Pro";
        device = "lmi";
        description = "qssi-user 12 RKQ1.211001.001 V13.0.4.0.SJKCNXM release-keys";
        }
    } else {
        model = "POCO F2 Pro";
        device = "lmi";
        description = "qssi-user 12 RKQ1.211001.001 V13.0.1.0.SJKMIXM release-keys";
    }

    set_ro_product_prop("device", device);
    set_ro_product_prop("model", model);
    property_override("ro.build.description", description.c_str());
    if (mod_device != "") {
        property_override("ro.product.mod_device", mod_device.c_str());
    }

    property_override("ro.boot.verifiedbootstate", "green");

    load_dalvik_properties();
}
