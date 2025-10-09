.. _zephyr_sdcard_ncs:

SD Card Sample for XIAO nRF54L15
##################################

Overview
********

This sample demonstrates how to use the SD card interface with the XIAO nRF54L15
board and expansion base. It uses the SPI interface to communicate with an SD card
and demonstrates basic filesystem operations including:

* Initializing and mounting an SD card
* Reading SD card information (size, sector count)
* Listing directory contents
* Creating files and directories (optional)
* Unmounting the SD card

Requirements
************

* XIAO nRF54L15 development board
* XIAO Expansion Base with SD card slot
* Micro SD card (formatted as FAT32 or exFAT)
* nRF Connect SDK v3.1.0 or later

Hardware Setup
**************

The SD card is connected via the XIAO expansion base using SPI:

* **MOSI**: D10 (xiao_spi)
* **MISO**: D9 (xiao_spi)
* **SCK**: D8 (xiao_spi)
* **CS**: D2 (GPIO)

Insert a formatted SD card (FAT32 or exFAT) into the SD card slot on the
XIAO expansion base before powering on the device.

Building and Running
********************

This application can be built and flashed as follows:

.. code-block:: console

   west build -b xiao_nrf54l15/nrf54l15/cpuapp
   west flash

After flashing, the application will:

1. Initialize the SD card
2. Display SD card information (size, sector count)
3. Mount the filesystem
4. List the contents of the root directory
5. Optionally create sample files and directories
6. Unmount the SD card

Sample Output
*************

.. code-block:: console

   *** Booting nRF Connect SDK v3.1.0 ***
   [00:00:00.123,456] <inf> main: XIAO nRF54L15 SD Card Sample
   [00:00:00.123,456] <inf> main: ==================================
   [00:00:00.123,456] <inf> main: Initializing SD card...
   [00:00:00.234,567] <inf> main: Block count 15523840
   [00:00:00.234,567] <inf> main: Sector size 512 bytes
   [00:00:00.234,567] <inf> main: Memory Size(MB) 7580
   [00:00:00.234,567] <inf> main: Mounting disk...
   [00:00:00.345,678] <inf> main: Disk mounted successfully at /SD:
   [00:00:00.345,678] <inf> main: Testing unmount/remount...
   [00:00:00.456,789] <inf> main: Unmount/remount test successful
   [00:00:00.456,789] <inf> main: 
   Listing dir /SD: ...
   [00:00:00.567,890] <inf> main: [DIR ] DCIM
   [00:00:00.567,890] <inf> main: [FILE] test.txt (size = 1024)
   [00:00:00.567,890] <inf> main: Total entries: 2
   [00:00:00.678,901] <inf> main: Unmounting disk...
   [00:00:00.678,901] <inf> main: Sample completed. System will idle.

Configuration Options
*********************

The following configuration options are available:

* ``CONFIG_FS_SAMPLE_CREATE_SOME_ENTRIES``: Enable to create sample files and
  directories on the SD card (default: y)
* ``CONFIG_FS_FATFS_EXFAT``: Enable exFAT support for larger file sizes and
  longer filenames (default: y)
* ``CONFIG_FS_FATFS_MAX_LFN``: Maximum length for long filenames (default: 255)

Troubleshooting
***************

If the sample fails to initialize the SD card:

1. Ensure the SD card is properly inserted
2. Verify the SD card is formatted as FAT32 or exFAT
3. Check that all SPI connections are correct on the expansion base
4. Try a different SD card (some cards may not be compatible)
5. Reduce the SPI frequency in the overlay file if experiencing communication issues

If the filesystem fails to mount:

1. Format the SD card as FAT32 on a computer
2. Ensure the SD card is not write-protected
3. Check the logs for specific error codes

References
**********

* `Zephyr File System API <https://docs.zephyrproject.org/latest/services/storage/file_systems.html>`_
* `Zephyr SDHC SPI Slot Driver <https://docs.zephyrproject.org/latest/hardware/peripherals/sdhc.html>`_
* `FAT Filesystem (ELM) <https://docs.zephyrproject.org/latest/services/storage/file_systems.html#fat-filesystem>`_
