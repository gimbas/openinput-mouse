#JLinkExe -device STM32F103C8 -if swd -JTAGConf -1,-1 -speed auto -CommandFile stm32f1.jlink # bluepill
#JLinkExe -device STM32F103T6 -if swd -JTAGConf -1,-1 -speed auto -CommandFile stm32f1.jlink # rival310
JLinkExe -device EFM32GG12B810F1024 -if swd -JTAGConf -1,-1 -speed auto -CommandFile efm32gg12.jlink # sltb009a
