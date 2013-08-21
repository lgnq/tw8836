#ifndef _OUTPUTCTRL_H_
#define	_OUTPUTCTRL_H_

void	LedBackLight( BYTE on );
void	BlackScreen( BYTE on );

//void PowerUpPanelWithDefault(void);
void LedPowerUp(void);
void DCDCOn(void);
void BT656OutputEnable(BYTE fOn, BYTE clear_port);
void OutputEnablePin(BYTE fFPDataPin, BYTE fOutputPin);

#endif
