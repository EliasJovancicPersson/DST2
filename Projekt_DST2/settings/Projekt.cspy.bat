@REM This batch file has been generated by the IAR Embedded Workbench
@REM C-SPY Debugger, as an aid to preparing a command line for running
@REM the cspybat command line utility using the appropriate settings.
@REM
@REM You can launch cspybat by typing the name of this batch file followed
@REM by the name of the debug file (usually an ELF/DWARF or UBROF file).
@REM Note that this file is generated every time a new debug session
@REM is initialized, so you may want to move or rename the file before
@REM making changes.
@REM 


"C:\Program Files (x86)\IAR Systems\Embedded Workbench 6.0\common\bin\cspybat" "C:\Program Files (x86)\IAR Systems\Embedded Workbench 6.0\arm\bin\armproc.dll" "C:\Program Files (x86)\IAR Systems\Embedded Workbench 6.0\arm\bin\armjlink.dll"  %1 --plugin "C:\Program Files (x86)\IAR Systems\Embedded Workbench 6.0\arm\bin\armbat.dll" --macro "C:\Users\Elijov23\Desktop\Projekt_DST2\arduino_due_x_sram.mac" --backend -B "--endian=little" "--cpu=Cortex-M3" "--fpu=None" "-p" "C:\Program Files (x86)\IAR Systems\Embedded Workbench 6.0\arm\CONFIG\debugger\Atmel\iosam3x8e.svd" "--semihosting" "--device=SAM3X8E" "--drv_communication=USB0" "--jlink_speed=auto" "--jlink_initial_speed=32" "--jlink_reset_strategy=0,0" "--drv_catch_exceptions=0x000" "--drv_swo_clock_setup=72000000,0,2000000" 


