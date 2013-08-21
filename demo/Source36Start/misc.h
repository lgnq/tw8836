#ifndef __MISC_H__
#define __MISC_H__

#if defined(SUPPORT_I2CCMD_SLAVE_V1)
extern bit F_i2ccmd_exec;				/*!< I2CCMD flag */
#define I2CCMD_CHECK	0x20
#define I2CCMD_EXEC		0x10
#endif

BYTE InitSystemForChipTest(BYTE fPowerUpBoot);
void Init_I2CCMD_Slave(void);
BYTE I2CCMD_exec_main(void);

#endif //__MISC_H__
