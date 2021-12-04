#ifndef CMD_H
#define CMD_H


#define CMD_SYS_RUN      0x0001 /**< System running Command (enable all clocks,
                                     and go to active state) */
#define CMD_STANDBY      0x0002 /**< Standby Command (gate off clocks, and go to
                                     standby state) */
#define CMD_SLEEP        0x0003 /**< Sleep Command (disable all clocks, and go
                                     to sleep state) */
#define CMD_REG_RD       0x0010 /**< Read Register Command */
#define CMD_REG_WR       0x0011 /**< Write Register Command */
#define CMD_MEM_BST_RD_T 0x0012 /**< Memory Burst Read Trigger Command (This
                                     command will trigger internal FIFO to read
                                     data from memory.)*/
#define CMD_MEM_BST_RD_S 0x0013 /**< Memory Burst Read Start Command (This is
                                     only a data read command. It will read data
                                     from internal FIFO. So, this command should
                                     be issued after MEM_BST_RD_T command) */
#define CMD_MEM_BST_WR   0x0014 /**< Memory Burst Write Command */
#define CMD_MEM_BST_END  0x0015 /**< End Memory Burst Cycle */
#define CMD_LD_IMG       0x0020 /**< Load Full Image Command (AEG[15:0] see
                                     Register 0x200) (Write Data Number equals
                                     to full display size) */
#define CMD_LD_IMG_AREA  0x0021 /**< Load Partial Image Command (AEG[15:0] see
                                     Register 0x200) (Write Data Number equals
                                     to partial display size according to width
                                     and height) */
#define CMD_LD_IMG_END   0x0022 /**< End Load Image Cycle */
#define CMD_LD_JPEG      0x0030
#define CMD_LD_JPEG_AREA 0x0031
#define CMD_LD_JPEG_END  0x0032

#endif
