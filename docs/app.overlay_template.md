/ {
/*
  * @brief Device Tree Overlay for XIAO nRF54L15
  *
  * This file customizes the base board device tree to configure
  * peripherals for a specific application, including:
  * - User-defined ADC channels
  * - PWM-controlled LED
  * - Buttons and a relay
  * - E-paper display (UC8179) via SPI
  * - OLED display (SSD1306) via I2C
  *
  * To switch between the two displays, simply uncomment one and comment
  * out the other in the "chosen" node below.
  */

/************************************************************
  * Aliases for easy access to devices in application code
  ************************************************************/
aliases {
  pwm-led = &pwm0_led0;
  sw1 = &xiao_button0;
  relay0 = &xiao_relay0;
};

/************************************************************
  * Display selection (choose one if multiple)
  ************************************************************/
chosen {
  zephyr,display = &uc8179_7inch5_epaper_gdew075t7;
  zephyr,display = &ssd1306_128x64;
};

/************************************************************
  * PWM LED, Button, and Relay configuration
  ************************************************************/
pwm_leds {
  compatible = "pwm-leds";
  pwm0_led0: my_pwm_led {
    // PWM channel 0 on PWM instance 20
    // PWM_MSEC(20) sets a period of 20ms
    pwms = <&pwm20 0 PWM_MSEC(20) PWM_POLARITY_NORMAL>;
    status = "okay";
  };
};

buttons {
  compatible = "gpio-keys";
  xiao_button0: button_0 {
    // Connect to the XIAO nRF54L15 pin D1
    // GPIO_PULL_UP ensures the pin is high when the button is not pressed
    // GPIO_ACTIVE_LOW means the button press drives the pin low
    gpios = <&xiao_d 1 (GPIO_PULL_UP | GPIO_ACTIVE_LOW)>;
    zephyr,code = <INPUT_KEY_0>;
  };
};

relay {
  compatible = "gpio-leds";
  xiao_relay0: relay_0 {
    // Connect to the XIAO nRF54L15 pin D0
    gpios = <&xiao_d 0 GPIO_ACTIVE_HIGH>;
  };
};

/************************************************************
  * Local nodes that don't modify existing ones
  ************************************************************/
zephyr,user {
  io-channels = <&adc 0>, <&adc 1>, <&adc 2>, <&adc 3>,
          <&adc 4>, <&adc 5>, <&adc 6>, <&adc 7>;
};

// MIPI-DBI SPI interface for the E-paper display
mipi_dbi_xiao_epaper {
  compatible = "zephyr,mipi-dbi-spi";
  spi-dev = <&xiao_spi>;
  // D3 pin for Data/Command control
  dc-gpios = <&xiao_d 3 GPIO_ACTIVE_HIGH>;
  // D0 pin for Reset
  reset-gpios = <&xiao_d 0 GPIO_ACTIVE_LOW>;
  write-only;
  #address-cells = <1>;
  #size-cells = <0>;

  uc8179_7inch5_epaper_gdew075t7: uc8179@0 {
    compatible = "gooddisplay,gdew075t7", "ultrachip,uc8179";
    // Max SPI frequency for the display
    mipi-max-frequency = <4000000>;
    reg = <0>;
    width = <800>;
    height = <480>;
    // D2 pin for Busy signal from the display
    busy-gpios = <&xiao_d 2 GPIO_ACTIVE_LOW>;
    softstart = [17 17 17 17];
    full {
      pwr = [07 07 3f 3f];
      cdi = <07>;
      tcon = <0x22>;
    };
  };
};
};

/************************************************************
* Device fragments (modifying nodes from the base board DTS)
************************************************************/
// PWM instance 20
&pwm20 {
status = "okay";
pinctrl-0 = <&pwm20_default>;
pinctrl-1 = <&pwm20_sleep>;
pinctrl-names = "default", "sleep";
};

// GPIO pin control
&pinctrl {
pwm20_default: pwm20_default {
  group1 {
    // Configure PWM channel 0 on P1.04 pin (Pin D0)
    psels = <NRF_PSEL(PWM_OUT0, 1, 4)>;
  };
};

pwm20_sleep: pwm20_sleep {
  group1 {
    psels = <NRF_PSEL(PWM_OUT0, 1, 4)>;
    low-power-enable;
  };
};
};

// PDM instance 20 for DMIC
dmic_dev: &pdm20 {
status = "okay";
};

// Power configuration
&pdm_imu_pwr {
/delete-property/ regulator-boot-on;
};

// UART instance 20
&uart20 {
current-speed = <921600>;
};

// SPI peripheral
&xiao_spi {
status = "okay";
// D1 pin for Chip Select
cs-gpios = <&xiao_d 1 GPIO_ACTIVE_LOW>;
};

// I2C peripheral
&xiao_i2c {
status = "okay";
zephyr,concat-buf-size = <2048>;

ssd1306_128x64: ssd1306@3c {
  compatible = "solomon,ssd1306fb";
  reg = <0x3c>;
  width = <128>;
  height = <64>;
  segment-offset = <0>;
  page-offset = <0>;
  display-offset = <0>;
  multiplex-ratio = <63>;
  segment-remap;
  com-invdir;
  prechargep = <0x22>;
};
};