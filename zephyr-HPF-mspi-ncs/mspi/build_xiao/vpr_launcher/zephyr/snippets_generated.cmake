# WARNING. THIS FILE IS AUTO-GENERATED. DO NOT MODIFY!
#
# This file contains build system settings derived from your snippets.
# Its contents are an implementation detail that should not be used outside
# of Zephyr's snippets CMake module.
#
# See the Snippets guide in the Zephyr documentation for more information.

###############################################################################
# Global information about all snippets.

# The name of every snippet that was discovered.
set(SNIPPET_NAMES "bt-ll-sw-split" "cdc-acm-console" "ci-shell" "diagnostic-logs" "hpf-gpio-icbmsg" "hpf-gpio-icmsg" "hpf-gpio-mbox" "hpf-mspi" "hw-flow-control" "matter-debug" "nordic-bt-rpc" "nordic-flpr" "nordic-flpr-xip" "nordic-log-stm" "nordic-log-stm-dict" "nordic-ppr" "nordic-ppr-xip" "nrf70-driver-debug" "nrf70-driver-verbose-debug" "nrf70-fw-patch-ext-flash" "nrf70-wifi" "nrf91-modem-trace-ext-flash" "nrf91-modem-trace-ram" "nrf91-modem-trace-rtt" "nrf91-modem-trace-uart" "nus-console" "power-consumption-tests" "ram-console" "rtt-console" "rtt-tracing" "serial-console" "tfm-enable-share-uart" "usbip-native-sim" "video-sw-generator" "wifi-credentials" "wifi-enterprise" "wifi-ip" "wifi-ipv4" "wifi-ipv6" "wpa-supplicant-debug" "xen_dom0" "zperf")
# The paths to all the snippet.yml files. One snippet
# can have multiple snippet.yml files.
set(SNIPPET_PATHS "C:/ncs/v3.1.0/nrf/snippets/ci-shell/snippet.yml" "C:/ncs/v3.1.0/nrf/snippets/hpf/gpio/icbmsg/snippet.yml" "C:/ncs/v3.1.0/nrf/snippets/hpf/gpio/icmsg/snippet.yml" "C:/ncs/v3.1.0/nrf/snippets/hpf/gpio/mbox/snippet.yml" "C:/ncs/v3.1.0/nrf/snippets/hpf/mspi/snippet.yml" "C:/ncs/v3.1.0/nrf/snippets/hw-flow-control/snippet.yml" "C:/ncs/v3.1.0/nrf/snippets/matter-debug/snippet.yml" "C:/ncs/v3.1.0/nrf/snippets/matter-diagnostic-logs/snippet.yml" "C:/ncs/v3.1.0/nrf/snippets/matter-power-consumption-tests/snippet.yml" "C:/ncs/v3.1.0/nrf/snippets/nordic-bt-rpc/snippet.yml" "C:/ncs/v3.1.0/nrf/snippets/nordic-flpr/snippet.yml" "C:/ncs/v3.1.0/nrf/snippets/nrf70-driver-debug/snippet.yml" "C:/ncs/v3.1.0/nrf/snippets/nrf70-driver-verbose-debug/snippet.yml" "C:/ncs/v3.1.0/nrf/snippets/nrf70-fw-patch-ext-flash/snippet.yml" "C:/ncs/v3.1.0/nrf/snippets/nrf70-wifi/snippet.yml" "C:/ncs/v3.1.0/nrf/snippets/nrf91-modem-trace-ext-flash/snippet.yml" "C:/ncs/v3.1.0/nrf/snippets/nrf91-modem-trace-ram/snippet.yml" "C:/ncs/v3.1.0/nrf/snippets/nrf91-modem-trace-rtt/snippet.yml" "C:/ncs/v3.1.0/nrf/snippets/nrf91-modem-trace-uart/snippet.yml" "C:/ncs/v3.1.0/nrf/snippets/tfm-enable-share-uart/snippet.yml" "C:/ncs/v3.1.0/nrf/snippets/wpa-supplicant-debug/snippet.yml" "C:/ncs/v3.1.0/nrf/snippets/zperf/snippet.yml" "C:/ncs/v3.1.0/zephyr/snippets/bt-ll-sw-split/snippet.yml" "C:/ncs/v3.1.0/zephyr/snippets/cdc-acm-console/snippet.yml" "C:/ncs/v3.1.0/zephyr/snippets/nordic-flpr-xip/snippet.yml" "C:/ncs/v3.1.0/zephyr/snippets/nordic-flpr/snippet.yml" "C:/ncs/v3.1.0/zephyr/snippets/nordic-log-stm-dict/snippet.yml" "C:/ncs/v3.1.0/zephyr/snippets/nordic-log-stm/snippet.yml" "C:/ncs/v3.1.0/zephyr/snippets/nordic-ppr-xip/snippet.yml" "C:/ncs/v3.1.0/zephyr/snippets/nordic-ppr/snippet.yml" "C:/ncs/v3.1.0/zephyr/snippets/nus-console/snippet.yml" "C:/ncs/v3.1.0/zephyr/snippets/ram-console/snippet.yml" "C:/ncs/v3.1.0/zephyr/snippets/rtt-console/snippet.yml" "C:/ncs/v3.1.0/zephyr/snippets/rtt-tracing/snippet.yml" "C:/ncs/v3.1.0/zephyr/snippets/serial-console/snippet.yml" "C:/ncs/v3.1.0/zephyr/snippets/usbip-native-sim/snippet.yml" "C:/ncs/v3.1.0/zephyr/snippets/video-sw-generator/snippet.yml" "C:/ncs/v3.1.0/zephyr/snippets/wifi-credentials/snippet.yml" "C:/ncs/v3.1.0/zephyr/snippets/wifi-enterprise/snippet.yml" "C:/ncs/v3.1.0/zephyr/snippets/wifi-ip/snippet.yml" "C:/ncs/v3.1.0/zephyr/snippets/wifi-ipv4/snippet.yml" "C:/ncs/v3.1.0/zephyr/snippets/wifi-ipv6/snippet.yml" "C:/ncs/v3.1.0/zephyr/snippets/xen_dom0/snippet.yml")

# Create variable scope for snippets build variables
zephyr_create_scope(snippets)

###############################################################################
# Snippet 'nordic-flpr'

# Common variable appends.
zephyr_set(EXTRA_DTC_OVERLAY_FILE "C:/ncs/v3.1.0/nrf/snippets/nordic-flpr/nordic-flpr.overlay" SCOPE snippets APPEND)
zephyr_set(EXTRA_DTC_OVERLAY_FILE "C:/ncs/v3.1.0/zephyr/snippets/nordic-flpr/nordic-flpr.overlay" SCOPE snippets APPEND)
# Appends for board regular expression '.*/nrf54lm20a/cpuapp'
if("${BOARD}${BOARD_QUALIFIERS}" MATCHES "^.*/nrf54lm20a/cpuapp$")
  zephyr_set(EXTRA_DTC_OVERLAY_FILE "C:/ncs/v3.1.0/nrf/snippets/nordic-flpr/soc/nrf54lm20a_cpuapp.overlay" SCOPE snippets APPEND)
  zephyr_set(EXTRA_DTC_OVERLAY_FILE "C:/ncs/v3.1.0/zephyr/snippets/nordic-flpr/soc/nrf54lm20a_cpuapp.overlay" SCOPE snippets APPEND)
endif()
# Appends for board regular expression '.*/nrf54lv10a/cpuapp'
if("${BOARD}${BOARD_QUALIFIERS}" MATCHES "^.*/nrf54lv10a/cpuapp$")
  zephyr_set(EXTRA_DTC_OVERLAY_FILE "C:/ncs/v3.1.0/nrf/snippets/nordic-flpr/soc/nrf54lv10a_cpuapp.overlay" SCOPE snippets APPEND)
endif()
# Appends for board regular expression '.*/nrf54l15/cpuapp'
if("${BOARD}${BOARD_QUALIFIERS}" MATCHES "^.*/nrf54l15/cpuapp$")
  zephyr_set(EXTRA_DTC_OVERLAY_FILE "C:/ncs/v3.1.0/zephyr/snippets/nordic-flpr/soc/nrf54l15_cpuapp.overlay" SCOPE snippets APPEND)
endif()
# Appends for board regular expression '.*/nrf54h20/cpuapp'
if("${BOARD}${BOARD_QUALIFIERS}" MATCHES "^.*/nrf54h20/cpuapp$")
  zephyr_set(EXTRA_DTC_OVERLAY_FILE "C:/ncs/v3.1.0/zephyr/snippets/nordic-flpr/soc/nrf54h20_cpuapp.overlay" SCOPE snippets APPEND)
endif()
# Appends for board regular expression '.*/nrf54l09/cpuapp'
if("${BOARD}${BOARD_QUALIFIERS}" MATCHES "^.*/nrf54l09/cpuapp$")
  zephyr_set(EXTRA_DTC_OVERLAY_FILE "C:/ncs/v3.1.0/zephyr/snippets/nordic-flpr/soc/nrf54l09_cpuapp.overlay" SCOPE snippets APPEND)
endif()
# Appends for board regular expression '.*/nrf54l20/cpuapp'
if("${BOARD}${BOARD_QUALIFIERS}" MATCHES "^.*/nrf54l20/cpuapp$")
  zephyr_set(EXTRA_DTC_OVERLAY_FILE "C:/ncs/v3.1.0/zephyr/snippets/nordic-flpr/soc/nrf54l20_cpuapp.overlay" SCOPE snippets APPEND)
endif()

