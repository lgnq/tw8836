#ifndef __MAIN__
#define __MAIN__


#define SYS_MODE_NORMAL		0
#define SYS_MODE_NOINIT		1
#define SYS_MODE_RCD		2
extern BYTE CurrSystemMode;

#define SW_INTR_VIDEO		0x80
#define SW_INTR_EXTERN		0x40

#define EXT_I2C_REQ_ANSWER	0x20
#define EXT_I2C_DONE		0x10
#define EXT_I2C_ACK1		0xA1
#define EXT_I2C_ACK2		0xA2
#define EXT_I2C_NAK2		0xB2	
		
void InitCore(BYTE fPowerUpBoot);
void StartVideoInput(void);


void TaskSetGrid(BYTE onoff);
BYTE TaskGetGrid(void);
void TaskSetGridCmd(BYTE cmd);
BYTE TaskGetGridCmd(void);

#define TASK_CMD_DONE			0
#define TASK_CMD_WAIT_VIDEO		1
#define TASK_CMD_WAIT_MODE		2
#define TASK_CMD_RUN			3
#define TASK_CMD_RUN_FORCE		4

void TaskNoSignal_setCmd(BYTE cmd);
BYTE TaskNoSignal_getCmd(void);

void Interrupt_enableVideoDetect(BYTE fOn);
void Interrupt_enableSyncDetect(BYTE fOn);

void LinkCheckAndSetInput(void);
extern BYTE (*CheckAndSetInput)(void);
BYTE CheckAndSetUnknown(void);
BYTE IsBackDrivePortOn(void);

#if defined(MODEL_TW8835_SLAVE) && defined(SUPPORT_I2CCMD_TEST)
void test_set_i2c_slave_compare_page(BYTE test_page);
#endif

void SystemPowerSave(void);
void SystemPowerResume(void);

#endif	// __MAIN__
